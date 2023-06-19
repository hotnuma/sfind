#include "dirparser.h"

#include <time.h>
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

bool _get_size(uint64_t *result, const char *sizestr)
{

    return true;
}

bool _get_date(uint64_t *result, const char *datestr)
{
    struct tm tm = {0};

    if (strptime(datestr, "%Y/%m/%d", &tm) == NULL)
        return false;

    tm.tm_isdst = -1;

    *result = mktime(&tm);

    return true;
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

        // file time ----------------------------------------------------------

        else if (strcmp(part, "-at") == 0)
        {
            parser_set(parser, DP_ATIME);
        }

        else if (strcmp(part, "-p") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n]);
        }

        else if (strcmp(part, "-sgt") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_size(&parser->s1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();
            }
        }

        else if (strcmp(part, "-slt") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_size(&parser->s1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();
            }
        }

        else if (strcmp(part, "-eq") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->t1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser->t2 = parser->t1 + 86399; // + 23h 59m 59s
            }
        }

        else if (strcmp(part, "-from") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->t1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();
            }
        }

        else if (strcmp(part, "-to") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->t2, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser->t2 += 86399; // date + 23:59:59
            }
        }

        // execute command ----------------------------------------------------

        else if (strcmp(part, "-exec") == 0)
        {
            argmode = true;
        }

        else if (argmode)
        {
            parser_args_append(parser, part);
        }

        // pattern ------------------------------------------------------------

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


