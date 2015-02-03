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

