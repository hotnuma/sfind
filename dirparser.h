#ifndef DIRPARSER_H
#define DIRPARSER_H

#include <cstringlist.h>

typedef struct _DirParser DirParser;

struct _DirParser
{
    CStringList *pathlist;

    CStringList *incl;
    CStringList *excl;

    bool all;
};

DirParser* parser_new();
void parser_free(DirParser *parser);

#define DirParserAuto GC_CLEANUP(_freeDirParser) DirParser
GC_UNUSED static inline void _freeDirParser(DirParser **parser)
{
    parser_free(*parser);
}

void parser_sort(DirParser *parser);

bool parser_run(DirParser *parser, const char *path);

#endif // DIRPARSER_H


