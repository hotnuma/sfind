#ifndef DIRPARSER_H
#define DIRPARSER_H

#include <clist.h>
#include <cstringlist.h>
#include <cfileinfo.h>

typedef struct _DirParser DirParser;

#define DP_NONE      0
#define DP_ALL      (1 << 0)
#define DP_SINGLE   (1 << 1)

//#define DP_OTHER  (1 << 2)
//#define DP_OTHER  (1 << 3)

struct _DirParser
{
    CStringList *pathlist;

    int flags;
    CStringList *excl;
    CStringList *incl;

    CFileInfo *info;
    uint64_t t1;
    uint64_t t2;

    CList *args;
    CList *argsreal;
    int childstatus;
};

DirParser* parser_new();
void parser_free(DirParser *parser);

#define DirParserAuto GC_CLEANUP(_freeDirParser) DirParser
GC_UNUSED static inline void _freeDirParser(DirParser **parser)
{
    parser_free(*parser);
}

void parser_set(DirParser *parser, int flag);
bool parser_is_set(DirParser *parser, int flag);
bool parser_get_date(const char *datestr, uint64_t *result);
void parser_args_append(DirParser *parser, const char *arg);
void parser_args_terminate(DirParser *parser);

bool parser_run(DirParser *parser, const char *path);

#endif // DIRPARSER_H


