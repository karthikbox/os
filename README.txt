Path functionality
command our sbush supports: set PATH <give a path>
case1: set PATH $PATH:/usr/bin/abc -> this command appends /usr/bin/abc to the existing path
case2: set PATH /usr/bin/abc:$PATH -> this command prepends /usr/bin/abc to the existing path
case3: set PATH /usr/bin/abc -> this command removes the existing path and sets to /usr/bin/abc

PS1 functionality
command our sbush supports: set PS1 "<give a word>"
example: set PS1 "sbush$ " -> this command changes the prompt to sbush$ 
example: set PS1 sbush$ -> this command throws an error

Pipe functionality
command our sbush supports: <command1> [args]|<command2> [args][|<command3> [args]] (No leading spaces/trailing spaces
example: ls|head|tail
example: /bin/ls|head|tail
example: ls -al|head -3|tail -1

EXECUTE BINARY functionality
syntax 1:
<filepath>
our shell first tries to execute this file as is, if it fails, it tries to look in the PATH directories for file.
example: sbush rootfs/bin/sbush -> executes the binary 
example: sbush ls -> tries to execute the command as is, it fails if there is no file named ls in the current directory, then it looks for in all the PATH directories

EXECUTE SCRIPT functionality
syntax:
sbush <filepath>
example: sbush /rootfs/test.dat -> executes the commands in the file test.dat.
NOTE: file CAN BE ANY extension.
