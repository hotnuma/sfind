#include "dirparser.h"

#include "pathcmp.h"
#include <cdirparser.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <time.h>

#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include <print.h>

#if defined O_LARGEFILE
#define OPENFLAGS (O_RDONLY | O_LARGEFILE)
#else
#define OPENFLAGS (O_RDONLY)
#endif

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
    cstrlist_free(parser->pathlist);

    cstrlist_free(parser->excl);
    cstrlist_free(parser->incl);
    clist_free(parser->args);
    clist_free(parser->argsreal);

    free(parser);
}

// params ---------------------------------------------------------------------

void parser_set(DirParser *parser, int flags)
{
    parser->flags |= flags;
}

void parser_uset(DirParser *parser, int flags)
{
    parser->flags &= ~(flags);
}

bool parser_is_set(DirParser *parser, int flags)
{
    return ((parser->flags & flags) == flags);
}

bool parser_set_timenow(DirParser *parser, const char *timestr)
{
    int scale = 0;

    char *end;
    unsigned long val = strtoul(timestr, &end, 10);

    if (!end)
        return false;

    if (strcmp(end, " se") == 0)
    {
        print("sec = %lu", val);
        scale = 1;
    }
    else if (strcmp(end, " mi") == 0)
    {
        print("min = %lu", val);
        scale = 60;
    }
    else if (strcmp(end, " ho") == 0)
    {
        print("hour = %lu", val);
        scale = 3600;
    }
    else if (strcmp(end, " da") == 0)
    {
        print("day = %lu", val);
        scale = 86400;
    }
    else
    {
        return false;
    }

    if (val < 1)
        return false;

    if (!parser->info)
        parser->info = cfileinfo_new();

    parser->time2 = time(NULL);
    parser->time1 = parser->time2 - (val * scale) - 1;
    parser->time2 += 3600;

    return true;
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

    // append a default {} if needed

    bool found = false;
    int size = clist_size(parser->args);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->args, i);

        if (arg && strcmp(arg, "{}") == 0)
        {
            found = true;
            break;
        }
    }

    if (!found)
        clist_append(parser->args, strdup("{}"));

    // null terminate and duplicate the list

    clist_append(parser->args, NULL);

    size = clist_size(parser->args);

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
    CDirParserAuto *dir = cdirparser_new();
    cdirparser_setmatch(dir, (CDirParserMatch)_matchfunc, parser);

    int flags = CDP_FILES;

    if (!parser_is_set(parser, DP_SINGLE))
        flags |= CDP_SUBDIRS;

    if (!cdirparser_open(dir, dirpath, flags))
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
            print("%s", c_str(path));
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
    if (parser->info && cfileinfo_read(parser->info, filepath))
    {
        uint64_t ftime;
        uint64_t fsize;

        if (parser_is_set(parser, DP_ATIME))
            ftime = cfileinfo_atime(parser->info);
        else
            ftime = cfileinfo_mtime(parser->info);

        if (parser->time1 > 0 && ftime < parser->time1)
            return false;

        if (parser->time2 > 0 && ftime > parser->time2)
            return false;

        if (parser_is_set(parser, DP_SIZEGT))
        {
            fsize = cfileinfo_size(parser->info);
            if (fsize < parser->size)
                return false;
        }
        else if (parser_is_set(parser, DP_SIZELT))
        {
            fsize = cfileinfo_size(parser->info);
            if (fsize > parser->size)
                return false;
        }
    }

    // all files
    if (parser->incl == NULL)
        return true;

    // patterns
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

// exec -----------------------------------------------------------------------

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

    size = clist_size(parser->argsreal);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->argsreal, i);

        if (!arg)
            break;
    }

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

        execvp(argv[0], argv);

        _exit(1);
    }

    while (waitpid(child_pid, &(parser->childstatus), 0) == (pid_t) -1)
    {
        if (errno != EINTR)
        {
            print("interupt");

            return false; // fail
        }
    }

    return true;
}


