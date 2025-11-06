#include "dirparser.h"
#include "entry.h"

#include <cdirparser.h>
#include <dirent.h>
#include <libpath.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <time.h>
#include <sys/wait.h>
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

DirParser* parser_new()
{
    DirParser *parser = (DirParser *) calloc(1, sizeof(DirParser));
    parser->pathlist = clist_new(128, (CDeleteFunc) entry_free);

    return parser;
}

void parser_free(DirParser *parser)
{
    clist_free(parser->pathlist);

    cstrlist_free(parser->exclude);
    cstrlist_free(parser->include);
    clist_free(parser->args);
    clist_free(parser->argscmd);

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

    if (strcmp(end, "s") == 0)
    {
        //fprintf(stderr, "sec = %lu\n", val);
        scale = 1;
    }
    else if (strcmp(end, "min") == 0)
    {
        //fprintf(stderr, "min = %lu\n", val);
        scale = 60;
    }
    else if (strcmp(end, "h") == 0)
    {
        //fprintf(stderr, "hour = %lu\n", val);
        scale = 3600;
    }
    else if (strcmp(end, "d") == 0)
    {
        //fprintf(stderr, "day = %lu\n", val);
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

bool parser_args_terminate(DirParser *parser)
{
    // format args list

    if (!parser->args)
        return true;

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

    if (!parser->argscmd)
        parser->argscmd = clist_new(size, (CDeleteFunc) free);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->args, i);

        clist_append(parser->argscmd, arg ? strdup(arg) : NULL);
    }

    return true;
}

// parse ----------------------------------------------------------------------

bool parser_run(DirParser *parser, const char *dirpath)
{
    CDirParserAuto *dir = cdirparser_new();
    cdirparser_setmatch(dir, (CDirParserMatch) _matchfunc, parser);

    int flags = CDP_FILES;

    if (parser_is_set(parser, DP_CHDIR))
    {
        if (chdir(dirpath) != 0)
            return false;

        dirpath = ".";
    }

    if (!parser_is_set(parser, DP_NOSUB))
        flags |= CDP_SUBDIRS;

    if (!cdirparser_open(dir, dirpath, flags))
        return false;

    CStringAuto *filepath = cstr_new_size(256);
    int type;

    while (cdirparser_read(dir, filepath, &type))
    {
        if (type != DT_REG)
            continue;

        if (!_parser_match(parser, c_str(filepath)))
            continue;

        Entry *entry = entry_new();
        entry_setpath(entry, c_str(filepath));

        clist_append(parser->pathlist, entry);
    }

    _parser_sort(parser);

    int size = clist_size(parser->pathlist);

    for (int i = 0; i < size; ++i)
    {
        Entry *entry = (Entry*) clist_at(parser->pathlist, i);

        // execute or print

        if (parser->args)
            _parser_exec(parser, c_str(entry->path));
        else
            print("%s", c_str(entry->path));
    }

    return true;
}

static bool _matchfunc(const char *dir, const char *item,
                       int type, DirParser *parser)
{
    (void) dir;
    (void) type;

    if (!parser_is_set(parser, DP_HIDDEN) && item[0] == '.')
        return false;

    if (parser->exclude == NULL)
        return true;

    int size = cstrlist_size(parser->exclude);
    for (int i = 0; i < size; ++i)
    {
        CString *pattern = cstrlist_at(parser->exclude, i);
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
    if (!parser->include)
        return true;

    // match file names only
    const char *fname = path_sep(filepath);
    if (fname)
        ++fname;
    else
        fname = filepath;

    // patterns
    int size = cstrlist_size(parser->include);
    for (int i = 0; i < size; ++i)
    {
        CString *pattern = cstrlist_at(parser->include, i);
        if (fnmatch(c_str(pattern), fname, 0) != FNM_NOMATCH)
            return true;
    }

    return false;
}

void _parser_sort(DirParser *parser)
{
    clist_sort(parser->pathlist, (CCompareFunc) _compare);
}

static int _compare(void *entry1, void *entry2)
{
    Entry *e1 = *((Entry **) entry1);
    Entry *e2 = *((Entry **) entry2);

    const char *s1 = c_str(e1->sortkey);
    const char *s2 = c_str(e2->sortkey);

    return strcmp(s1, s2);
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

    char **argv = (char**) clist_data(parser->argscmd);

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

    size = clist_size(parser->argscmd);

    for (int i = 0; i < size; ++i)
    {
        const char *arg = (const char*) clist_at(parser->argscmd, i);

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
            fprintf(stderr, "interupt\n");

            return false; // failed
        }
    }

    return true;
}


