#include <stdlib.h>
#include <stdio.h>
#include <fnmatch.h>

int main()
{
    const char *pattern = "bla";
    const char *filepath = "bla";

    if (fnmatch(pattern, filepath, 0) != FNM_NOMATCH)
        printf("match\n");
    else
        printf("no match\n");

    return EXIT_SUCCESS;
}


