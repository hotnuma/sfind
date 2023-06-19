#include "dirparser.h"

#include <print.h>

static int _app_exit(bool usage, int ret)
{
    if (usage)
    {
        print("*** usage :");
        print("sfind /home/me/mydir");
        print("sfind . -a");
        print("sfind . -s");
        print("sfind . \"*.c\"");
        print("sfind . -x \"not_this\" \"*.c\"");
        print("sfind . -x \"exclude,dirs\" \"*.h,*.c\"");
        print("sfind . \"*.c\" -exec ls -la {}");
        print("sfind . \"*.c\" -exec ls -la");
        print("sfind . -from \"2023/06/11\" -to \"2023/06/13\"");
        print("sfind . -eq \"2023/06/12\"");
    }

    return ret;
}

int main(int argc, char **argv)
{
    DirParserAuto *parser = parser_new();
    CStringAuto *dirpath = cstr_new_size(256);

    if (argc < 2)
    {
        return _app_exit(true, EXIT_FAILURE);
    }

    cstr_copy(dirpath, argv[1]);

    int n = 2;
    bool argmode = false;

    while (n < argc)
    {
        const char *part = argv[n];

        if (strcmp(part, "-a") == 0)
        {
            parser_set(parser, DP_ALL);
        }

        else if (strcmp(part, "-s") == 0)
        {
            parser_set(parser, DP_SINGLE);
        }

        else if (strcmp(part, "-x") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (!parser->excl)
                parser->excl = cstrlist_new_size(12);

            cstrlist_split(parser->excl, argv[n], ",", false, true);
        }

        else if (strcmp(part, "-eq") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (parser_get_date(argv[n], &parser->t1))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser->t2 = parser->t1 + 86399;
            }
        }

        else if (strcmp(part, "-from") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (parser_get_date(argv[n], &parser->t1))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();
            }
        }

        else if (strcmp(part, "-to") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (parser_get_date(argv[n], &parser->t2))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser->t2 += 86399; // date + 23:59:59
            }
        }

        else if (strcmp(part, "-exec") == 0)
        {
            argmode = true;
        }

        else if (argmode)
        {
            parser_args_append(parser, part);
        }

        else
        {
            if (!parser->incl)
                parser->incl = cstrlist_new_size(12);

            cstrlist_split(parser->incl, part, ",", false, true);
        }

        ++n;
    }

    parser_args_terminate(parser);

    parser_run(parser, c_str(dirpath));

    return EXIT_SUCCESS;
}


