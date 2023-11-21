#include "entry.h"

#include <stdlib.h>

Entry* entry_new()
{
    Entry *entry = malloc(sizeof(Entry));

    entry->path = cstr_new_size(128);
    entry->sortkey = cstr_new_size(256);

    return entry;
}

void entry_free(Entry* entry)
{
    if (!entry)
        return;

    cstr_free(entry->path);
    cstr_free(entry->sortkey);

    free(entry);
}


