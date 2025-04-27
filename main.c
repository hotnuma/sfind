#include "dirparser.h"

#include <locale.h>
#include <time.h>
#include <print.h>

static int _app_exit(bool usage, int ret)
{
    if (usage)
    {
        print("*** usage :");
        print("sfind /my/dir \"*.h,*.c\"");
        print("sfind . \"*.c\"");
        print("sfind . -a \"*.c\"");
        print("sfind . -s \"*.c\"");
        print("sfind . -x \"onedir\" \"*.c\"");
        print("sfind . -x \"onedir,twodir\" \"*.c\"");
        print("sfind . -from \"2023/06/11\" -to \"2023/06/13\"");
        print("sfind . -eq \"2023/06/12\"");
        print("sfind . \"*.c\" -exec ls -la {}");
        print("sfind . \"*.c\" -exec ls -la");
    }

    return ret;
}

bool _get_size(uint64_t *result, const char *sizestr)
{
    char *end;
    double val = strtod(sizestr, &end);

    if (!end)
        return false;

    if (strcmp(end, "K") == 0)
    {
        val *= 1024;

        //print("k = %u", (uint64_t) val);
    }
    else if (strcmp(end, "M") == 0)
    {
        val *= 1024 * 1024;

        //print("m = %u", (uint64_t) val);
    }
    else if (strcmp(end, "G") == 0)
    {
        val *= 1024 * 1024 * 1024;

        //print("g = %u", (uint64_t) val);
    }

    *result = (uint64_t) val;

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
    setlocale(LC_ALL, "");

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

            if (!parser->exclude)
                parser->exclude = cstrlist_new_size(12);

            cstrlist_split(parser->exclude, argv[n], ",", false, true);
        }

        // file time ----------------------------------------------------------

        else if (strcmp(part, "-from") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->time1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();
            }
        }

        else if (strcmp(part, "-to") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->time2, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser->time2 += 86399; // date + 23:59:59
            }
        }

        else if (strcmp(part, "-at") == 0)
        {
            parser_set(parser, DP_ATIME);
        }

        else if (strcmp(part, "-eq") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->time1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser->time2 = parser->time1 + 86399; // + 23h 59m 59s
            }
        }

        else if (strcmp(part, "-p") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n]);
        }

        // file size ----------------------------------------------------------

        else if (strcmp(part, "-zlt") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_size(&parser->size, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser_uset(parser, DP_SIZEGT);
                parser_set(parser, DP_SIZELT);
            }
        }

        else if (strcmp(part, "-zgt") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_size(&parser->size, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();

                parser_uset(parser, DP_SIZELT);
                parser_set(parser, DP_SIZEGT);
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

        // search pattern -----------------------------------------------------

        else
        {
            if (!parser->include)
                parser->include = cstrlist_new_size(12);

            cstrlist_split(parser->include, part, ",", false, true);
        }

        ++n;
    }

    if (!parser_args_terminate(parser))
        return EXIT_FAILURE;

    parser_run(parser, c_str(dirpath));

    return EXIT_SUCCESS;
}


