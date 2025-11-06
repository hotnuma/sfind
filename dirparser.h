#ifndef DIRPARSER_H
#define DIRPARSER_H

#include <clist.h>
#include <cstringlist.h>
#include <cfileinfo.h>

typedef struct _DirParser DirParser;

#define DP_NONE     0
#define DP_CHDIR    (1 << 0)
#define DP_HIDDEN   (1 << 1)
#define DP_NOSUB    (1 << 2)
#define DP_ATIME    (1 << 3)
#define DP_SIZEGT   (1 << 4)
#define DP_SIZELT   (1 << 5)

struct _DirParser
{
    CList *pathlist;

    int flags;
    CStringList *exclude;
    CStringList *include;

    CFileInfo *info;
    uint64_t time1;
    uint64_t time2;
    uint64_t size;

    CList *args;
    CList *argscmd;
    int childstatus;
};

DirParser* parser_new();
void parser_free(DirParser *parser);

#define DirParserAuto GC_CLEANUP(_freeDirParser) DirParser
GC_UNUSED static inline void _freeDirParser(DirParser **parser)
{
    parser_free(*parser);
}

void parser_set(DirParser *parser, int flags);
void parser_uset(DirParser *parser, int flags);
bool parser_is_set(DirParser *parser, int flags);
bool parser_set_timenow(DirParser *parser, const char *timestr);

void parser_args_append(DirParser *parser, const char *arg);
bool parser_args_terminate(DirParser *parser);

bool parser_run(DirParser *parser, const char *path);

#endif // DIRPARSER_H


