
#if 0

// could it be faster ?

static inline char* _get_fname(char *filepath)
{
    if (!filepath || *filepath == '\0')
        return NULL;

    char *end = filepath + strlen(filepath);

    for (char *p = end; p >= filepath; ++p)
    {
        if (*p == '/')
            return p + 1;
    }

    return NULL;
}


// prepend "*/" if needed

CStringAuto *temp = cstr_new_size(64);

int size = cstrlist_size(parser->include);

for (int i = 0; i < size; ++i)
{
    CString *pattern = cstrlist_at(parser->include, i);

    if (!cstr_contains(pattern, "/", true)
        && !cstr_contains(pattern, "?", true)
        && !cstr_contains(pattern, "*", true))
    {
        cstr_copy(temp, "*/");
        cstr_append(temp, c_str(pattern));
        cstr_swap(pattern, temp);
    }
}

#endif


