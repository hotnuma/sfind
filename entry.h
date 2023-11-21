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

#endif // ENTRY_H


