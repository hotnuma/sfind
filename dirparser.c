#include "dirparser.h"

#include "pathcmp.h"
#include <cdirparser.h>
#include <fnmatch.h>
#include <stdio.h>

#include <print.h>

static bool _matchfunc(const char *dir, const char *item,
                       int type, DirParser *parser);
static int _compare(void *entry1, void *entry2);
static bool _parser_match(DirParser *parser, const char *filepath);

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

    free(parser);
}

void parser_set(DirParser *parser, int flag)
{
    parser->flags |= flag;
}

bool parser_is_set(DirParser *parser, int flag)
{
    return ((parser->flags & flag) != 0);
}

void parser_sort(DirParser *parser)
{
    cstrlist_sort_func(parser->pathlist, (CCompareFunc)_compare);
}

static int _compare(void *entry1, void *entry2)
{
    CString *e1 = *((CString **)entry1);
    CString *e2 = *((CString **)entry2);

    return path_cmp(c_str(e1), c_str(e2));
}

bool parser_run(DirParser *parser, const char *dirpath)
{
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

    parser_sort(parser);

    int size = cstrlist_size(parser->pathlist);
    for (int i = 0; i < size; ++i)
    {
        CString *path = cstrlist_at(parser->pathlist, i);

        printf("%s\n", c_str(path));
    }

    return true;
}

static bool _matchfunc(const char *dir, const char *item,
                       int type, DirParser *parser)
{
    (void)dir;
    (void)type;

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


