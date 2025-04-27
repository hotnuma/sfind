
#if 0

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

// ----------------------------------------------------------------------------

#include <libmacros.h>

static int _pathparse(const char *str)
{
    const char *last = NULL;
    const char *p = str;

    while (*p)
    {
        if (*p == '/')
            last = p;

        ++p;
    }

    if (last)
        return (last - str);

    return 0;
}

int path_cmp_2(const char *s1, const char *s2)
{
    int n1 = _pathparse(s1);
    int n2 = _pathparse(s2);

    int result = strncmp(s1, s2, MIN(n1, n2));

    if (result != 0)
        return result;

    return strcasecmp(s1+n1, s2+n2);
}

#endif


