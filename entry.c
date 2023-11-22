#include "entry.h"

#include <stdlib.h>
#include <print.h>

static void _entry_setkey(Entry *entry);
static bool _is_dir(const char *path);

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
            if (_is_dir(p))
                cstr_append_c(result, '0');
            else
                cstr_append_c(result, '1');

            start = false;
        }

        cstr_append_c(result, *p);

        ++p;
    }

    p = c_str(entry->sortkey);
    int len = strxfrm(NULL, p, 0);

    CStringAuto *temp = cstr_new_size(len + 1);
    strxfrm(cstr_data(temp), p, len);
    cstr_terminate(temp, len);

    cstr_swap(entry->sortkey, temp);
}

bool _is_dir(const char *path)
{
    while (*path)
    {
        if (*path == '/')
            return true;

        ++path;
    }

    return false;
}


