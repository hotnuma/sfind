
* basic search
    
    `sfind /home/me/mydir`
    
    `sfind /home/me/mydir` \"*.c\"`
    
    `sfind /home/me/mydir` \"*.h,*.c\"`
    
    `sfind . \"*.c\"`
    
* show hidden files
    
    `sfind . -a`

* single dir, no subdirs
    
    `sfind . -s`

* exclude directory list

    `sfind . -x \"onedir\" \"*.c\"`
    
    `sfind . -x \"onedir,twodir\" \"*.c\"`

* execute command

    `sfind . \"*.c\" -exec ls -la`
    
    `sfind . \"*.c\" -exec ls -la {}`

* file time
    
    `sfind . -from \"2023/06/11\" -to \"2023/06/13\"`
    
    `sfind . -at -from \"2023/06/11\" -to \"2023/06/13\"`

    `sfind . -eq \"2023/06/12\"`

* file size
    
    `sfind . -zlt \"10 m\" \"*.c\"`
    
    `sfind . -zgt \"10 k\" \"*.c\"`

<!--
else if (strcmp(part, "-p") == 0)
{
    if (++n >= argc)
        return EXIT_FAILURE;

    parser_set_timenow(parser, argv[n]);
}
-->
