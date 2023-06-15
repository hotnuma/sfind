#ifndef DIRPARSER_H
#define DIRPARSER_H

#include <cstringlist.h>

typedef struct _DirParser DirParser;

#define DP_NONE      0
#define DP_ALL      (1 << 0)

//#define DP_OTHER  (1 << 1)
//#define DP_OTHER  (1 << 2)
//#define DP_OTHER  (1 << 3)

struct _DirParser
{
    CStringList *pathlist;

    int flags;
    CStringList *excl;
    CStringList *incl;
    uint64_t t1;
    uint64_t t2;
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
void parser_sort(DirParser *parser);
bool parser_run(DirParser *parser, const char *path);

#endif // DIRPARSER_H


