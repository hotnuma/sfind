#include "pathcmp.h"

#include <string.h>

static int _count(const char *str)
{
    int ret = 0;

    while (*str)
    {
        if (*str == '/')
            ++ret;

        ++str;
    }

    return ret;
}

int path_cmp(const char *s1, const char *s2)
{
    int n1 = _count(s1);
    int n2 = _count(s2);

    if (n1 != n2)
        return n2 - n1;

    return strcasecmp(s1, s2);
}


