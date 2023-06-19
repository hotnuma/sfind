
#if 0
        else if (strcmp(part, "-ps") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n], 1);
        }

        else if (strcmp(part, "-pm") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n], 60);
        }

        else if (strcmp(part, "-ph") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n], 3600);
        }

        else if (strcmp(part, "-pd") == 0)
        {
            if (++n >= argc)
                return EXIT_FAILURE;

            parser_set_timenow(parser, argv[n], 86400);
        }
#endif

#if 0
bool parser_set_timenow_old(DirParser *parser, const char *timestr, int scale);

bool parser_set_timenow_old(DirParser *parser, const char *timestr, int scale)
{
    unsigned long val = strtoul(timestr, NULL, 10);

    if (val < 1)
        return false;

    if (!parser->info)
        parser->info = cfileinfo_new();

    parser->t2 = time(NULL);
    parser->t1 = parser->t2 - (val * scale) - 1;
    parser->t2 += 3600;

    return true;
}
#endif

#if 0
CString *directory;

cstr_free(parser->directory);

if (!parser->directory)
    parser->directory = cstr_new_size(256);

if (strcmp(dirpath, ".") == 0)
{
    if (getcwd(cstr_data(parser->directory), 256) == NULL)
        return false;

    cstr_terminate(parser->directory, -1);
}
else
{
    cstr_copy(parser->directory, dirpath);
}

char *currdir = c_str(parser->directory);

if (chdir(currdir) != 0)
    exit(EXIT_FAILURE);
#endif


