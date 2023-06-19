#include "dirparser.h"

#include "pathcmp.h"
#include <cdirparser.h>
#include <fnmatch.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>

#include <libapp.h>

#include <print.h>

static bool _matchfunc(const char *dir, const char *item,
                       int type, DirParser *parser);
static bool _parser_match(DirParser *parser, const char *filepath);
static void _parser_sort(DirParser *parser);
static int _compare(void *entry1, void *entry2);
static bool _parser_exec(DirParser *parser, const char *filepath);

// allocate -------------------------------------------------------------------

DirParser *parser_new()
{
    DirParser *parser = (DirParser *)calloc(1, sizeof(DirParser));
    parser->pathlist = cstrlist_new_size(128);

    return parser;
}

void parser_free(DirParser *parser)
{
    cstr_free(parser->directory);
    cstrlist_free(parser->pathlist);

    cstrlist_free(parser->excl);
    cstrlist_free(parser->incl);
    clist_free(parser->args);
    clist_free(parser->argsreal);

    free(parser);
}

// params ---------------------------------------------------------------------

void parser_set(DirParser *parser, int flag)
{
    parser->flags |= flag;
}

bool parser_is_set(DirParser *parser, int flag)
{
    return ((parser->flags & flag) != 0);
}

void parser_args_append(DirParser *parser, const char *arg)
{
    if (!parser->args)
        parser->args = clist_new(24, (CDeleteFunc) free);

    clist_append(parser->args, strdup(arg));
}

void parser_args_terminate(DirParser *parser)
{
    if (!parser->args)
        return;

    clist_append(parser->args, NULL);

    int size = clist_size(parser->args);

    if (!parser->argsreal)
        parser->argsreal = clist_new(size, (CDeleteFunc) free);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->args, i);

        clist_append(parser->argsreal, arg ? strdup(arg) : NULL);
    }
}

// parse ----------------------------------------------------------------------

bool parser_run(DirParser *parser, const char *dirpath)
{
    if (!parser->directory)
        parser->directory = cstr_new_size(256);

    if (strcmp(dirpath, ".") == 0)
    {
        if (getcwd(cstr_data(parser->directory), 256) == NULL)
            return false;

        cstr_terminate(parser->directory, -1);
    }
    else
    {
        cstr_copy(parser->directory, dirpath);
    }

    CDirParserAuto *dir = cdirparser_new();
    cdirparser_setmatch(dir, (CDirParserMatch)_matchfunc, parser);

    if (!cdirparser_open(dir, dirpath, CDP_FILES | CDP_SUBDIRS))
        return false;

    CStringAuto *filepath = cstr_new_size(256);

    while (cdirparser_read(dir, filepath, NULL))
    {
        if (!_parser_match(parser, c_str(filepath)))
            continue;

        cstrlist_append_len(parser->pathlist, c_str(filepath), cstr_size(filepath));
    }

    _parser_sort(parser);

    int size = cstrlist_size(parser->pathlist);
    for (int i = 0; i < size; ++i)
    {
        CString *path = cstrlist_at(parser->pathlist, i);

        if (parser->args)
            _parser_exec(parser, c_str(path));
        else
            printf("%s\n", c_str(path));
    }

    return true;
}

static bool _matchfunc(const char *dir, const char *item,
                       int type, DirParser *parser)
{
    (void) dir;
    (void) type;

    if (!parser_is_set(parser, DP_ALL) && item[0] == '.')
        return false;

    if (parser->excl == NULL)
        return true;

    int size = cstrlist_size(parser->excl);
    for (int i = 0; i < size; ++i)
    {
        CString *pattern = cstrlist_at(parser->excl, i);
        if (fnmatch(c_str(pattern), item, 0) != FNM_NOMATCH)
            return false;
    }

    return true;
}

static bool _parser_match(DirParser *parser, const char *filepath)
{
    if (parser->incl == NULL)
        return true;

    int size = cstrlist_size(parser->incl);
    for (int i = 0; i < size; ++i)
    {
        CString *pattern = cstrlist_at(parser->incl, i);
        if (fnmatch(c_str(pattern), filepath, 0) != FNM_NOMATCH)
            return true;
    }

    return false;
}

void _parser_sort(DirParser *parser)
{
    cstrlist_sort_func(parser->pathlist, (CCompareFunc)_compare);
}

static int _compare(void *entry1, void *entry2)
{
    CString *e1 = *((CString **)entry1);
    CString *e2 = *((CString **)entry2);

    return path_cmp(c_str(e1), c_str(e2));
}

#if defined O_LARGEFILE
#define OPENFLAGS (O_RDONLY | O_LARGEFILE)
#else
#define OPENFLAGS (O_RDONLY)
#endif

static bool prep_child_for_exec(bool close_stdin)
{
    bool ret = true;

    if (close_stdin)
    {
        if (close(0) < 0)
        {
            ret = false;
        }
        else
        {
            open("/dev/null", OPENFLAGS);
        }
    }

    return ret;
}

static bool _parser_exec(DirParser *parser, const char *filepath)
{
    if (!parser->args)
        return false;

    char *currdir = c_str(parser->directory);

    char **argv = (char**) clist_data(parser->argsreal);

    int size = clist_size(parser->args);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->args, i);

        if (!arg)
            break;

        if (strcmp(arg, "{}") == 0)
        {
            free(argv[i]);

            argv[i] = strdup(filepath);
        }
    }

    //printf("exec :");

    size = clist_size(parser->argsreal);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->argsreal, i);

        if (!arg)
            break;

        //printf(" %s", arg);
    }

    //printf("\n");

    // make sure output of command doesn't get mixed with global output.
    fflush(stdout);
    fflush(stderr);

    // make sure to listen for the kids.
    static bool first_time = true;

    if (first_time)
    {
        first_time = false;
        signal(SIGCHLD, SIG_DFL);
    }

    pid_t child_pid = fork();

    if (child_pid == -1)
        return false;

    if (child_pid == 0)
    {
        // we are the child

        if (!prep_child_for_exec(true))
        {
            _exit(1);
        }

        if (chdir(currdir) != 0)
            exit(EXIT_FAILURE);

        //printf("execute : %s\n", argv[0]);

        execvp(argv[0], argv);

        printf("failed\n");

        _exit(1);
    }

    while (waitpid(child_pid, &(parser->childstatus), 0) == (pid_t) -1)
    {
        if (errno != EINTR)
        {
            printf("interupt\n");

            return false; // fail
        }
    }

    return true;
}


