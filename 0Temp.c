
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


