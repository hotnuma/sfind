#ifndef ENTRY_H
#define ENTRY_H

#include <cstring.h>

typedef struct _Entry Entry;

struct _Entry
{
    CString *path;
    CString *sortkey;
};

Entry* entry_new();
void entry_free();

void entry_setpath(Entry *entry, const char *filepath);

#endif // ENTRY_H


