#include "dirparser.h"

#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <print.h>

static void error_exit(const char *msg)
{
    if (!msg || *msg == '\0')
    {
        msg = "an error occurred";
    }

    printf("*** %s\nabort...\n", msg);

    exit(EXIT_FAILURE);
}

static void usage_exit()
{
    print("*** usage :");
    print("sfind .");
    print("sfind . \"*.c\"");
    print("sfind /my/dir \"*.h,*.c\"");
    print("*** change directory :");
    print("sfind /my/dir -c");
    print("*** include hidden files :");
    print("sfind . -h \"*.c\"");
    print("*** no sub dirs :");
    print("sfind . -n \"*.c\"");
    print("*** exclude :");
    print("sfind . -x \"dir1\" \"*.c\"");
    print("sfind . -x \"dir1,dir2\" \"*.c\"");
    print("*** file time :");
    print("sfind . -from \"2023/06/11\" -to \"2023/06/13\"");
    print("sfind . -eq \"2023/06/12\"");
    print("*** past duration :");
    print("sfind . -p 60s");
    print("sfind . -p 60min");
    print("*** execute :");
    print("sfind . \"*.c\" -exec ls -la {}");
    print("sfind . \"*.c\" -exec ls -la");

    exit(EXIT_FAILURE);
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
        usage_exit();

    cstr_copy(dirpath, argv[1]);

    int n = 2;
    bool argmode = false;

    while (n < argc)
    {
        // change directory
        if (strcmp(argv[n], "-c") == 0)
        {
            parser_set(parser, DP_CHDIR);
        }

        // show hidden
        else if (strcmp(argv[n], "-h") == 0)
        {
            parser_set(parser, DP_HIDDEN);
        }

        // no sub dirs
        else if (strcmp(argv[n], "-n") == 0)
        {
            parser_set(parser, DP_NOSUB);
        }

        // exclude
        else if (strcmp(argv[n], "-x") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (!parser->exclude)
                parser->exclude = cstrlist_new_size(12);

            cstrlist_split(parser->exclude, argv[n], ",", false, true);
        }

        // file time ----------------------------------------------------------

        // access time
        else if (strcmp(argv[n], "-at") == 0)
        {
            parser_set(parser, DP_ATIME);
        }

        else if (strcmp(argv[n], "-from") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            if (_get_date(&parser->time1, argv[n]))
            {
                if (!parser->info)
                    parser->info = cfileinfo_new();
            }
        }

        else if (strcmp(argv[n], "-to") == 0)
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

        else if (strcmp(argv[n], "-eq") == 0)
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

        // past duration
        else if (strcmp(argv[n], "-p") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n]);
        }

        // file size ----------------------------------------------------------

        // greater
        else if (strcmp(argv[n], "-sgt") == 0)
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

        // lesser
        else if (strcmp(argv[n], "-slt") == 0)
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

        // execute command ----------------------------------------------------

        else if (strcmp(argv[n], "-exec") == 0)
        {
            argmode = true;
        }

        else if (argmode)
        {
            parser_args_append(parser, argv[n]);
        }

        // search pattern -----------------------------------------------------

        else
        {
            char *opt = argv[n];

            if (opt[0] == '-')
                error_exit("invalid option");

            if (!parser->include)
                parser->include = cstrlist_new_size(16);

            cstrlist_split(parser->include, argv[n], ",", false, true);
        }

        ++n;
    }

    if (!parser_args_terminate(parser))
        return EXIT_FAILURE;

    parser_run(parser, c_str(dirpath));

    return EXIT_SUCCESS;
}


