#include "entry.h"

#include <stdlib.h>
#include <print.h>

static void _entry_setkey(Entry *entry);

Entry* entry_new()
{
    Entry *entry = malloc(sizeof(Entry));

    entry->path = cstr_new_size(128);
    entry->sortkey = cstr_new_size(256);

    return entry;
}

void entry_free(Entry *entry)
{
    if (!entry)
        return;

    cstr_free(entry->path);
    cstr_free(entry->sortkey);

    free(entry);
}

void entry_setpath(Entry *entry, const char *filepath)
{
    cstr_copy(entry->path, filepath);
    _entry_setkey(entry);

    //print("%s", c_str(entry->sortkey));
}

void _entry_setkey(Entry *entry)
{
    CString *result = entry->sortkey;
    cstr_clear(result);

    const char *p = c_str(entry->path);
    bool start = false;

    while (*p)
    {
        if (*p == '/')
        {
            start = true;
        }
        else if (start == true)
        {
            if (strchr(p, '/') != NULL)
                cstr_append_c(result, '0');
            else
                cstr_append_c(result, '1');

            start = false;
        }

        cstr_append_c(result, *p);

        ++p;
    }

    CStringAuto *temp = cstr_new_size(256);
    cstr_xfrm(temp, c_str(entry->sortkey));
    cstr_swap(entry->sortkey, temp);
}


