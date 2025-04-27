
#### Examples

* basic search
    
    `sfind /my/dir "*.h,*.c"`
    
    `sfind . "*.c"`
    
* show all
    
    `sfind . -a "*.c"`

* single dir
    
    `sfind . -s "*.c"`

* exclude directory list

    `sfind . -x "onedir" "*.c"`
    
    `sfind . -x "onedir,twodir" "*.c"`

* file time
    
    `sfind . -from "2023/06/11" -to "2023/06/13"`
    
    `sfind . -at -from "2023/06/11" -to "2023/06/13"`

    `sfind . -eq "2023/06/12"`

    `sfind . -p 60s`
    
    `sfind . -p 30min`

* file size
    
    `sfind . -zlt 10M "*.c"`
    
    `sfind . -zgt 10K "*.c"`

* execute command

    `sfind . "*.c" -exec ls -la`
    
    `sfind . "*.c" -exec ls -la {}`


#### Sorting
    
The default sorting order is like so :

./a/a.txt
./b/a/a.txt
./b/a.txt
./c/a.txt
./a.txt

With special chars :

./a.txt
./à.txt
./b.txt


