
* basic search
    
    `sfind /my/dir \"*.h,*.c\"`
    
    `sfind . \"*.c\"`
    
* show hidden files
    
    `sfind . -a \"*.c\"`

* single dir, no subdirs
    
    `sfind . -s \"*.c\"`

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

    `sfind . -p \"120 se\"`

* file size
    
    `sfind . -zlt \"10 m\" \"*.c\"`
    
    `sfind . -zgt \"10 k\" \"*.c\"`


