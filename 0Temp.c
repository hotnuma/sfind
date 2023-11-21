
#if 0

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


