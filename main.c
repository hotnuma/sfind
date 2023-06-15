#include "dirparser.h"

#include <print.h>

static int _app_exit(bool usage, int ret)
{
    if (usage)
    {
        print("*** usage :");
        print("\tsfind dirpath");
    }

    return ret;
}

int main(int argc, char **argv)
{
    CStringAuto *dirpath = cstr_new_size(256);
    DirParserAuto *parser = parser_new();

    if (argc < 2)
    {
        return _app_exit(true, EXIT_FAILURE);
    }

    cstr_copy(dirpath, argv[1]);

    int n = 2;

    while (n < argc)
    {
        const char *part = argv[n];

        if (strcmp(part, "-x") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (!parser->excl)
                parser->excl = cstrlist_new_size(12);

            cstrlist_split(parser->excl, argv[n], ",", false, true);
        }
        else if (strcmp(part, "-a") == 0)
        {
            parser_set(parser, DP_ALL);
        }
        else
        {
            if (!parser->incl)
                parser->incl = cstrlist_new_size(12);

            cstrlist_split(parser->incl, part, ",", false, true);
        }

        ++n;
    }

    parser_run(parser, c_str(dirpath));

    return EXIT_SUCCESS;
}


