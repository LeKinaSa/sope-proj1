# SOPE - Project 1, Group T2G04
First project of the Operating Systems curricular unit.

* Clara Martins   (up201806528)
* Daniel Monteiro (up201806185)
* Gonçalo Pascoal (up201806332)

## Implemented features
* Command line argument parsing and invalid input handling
* Every required command line argument is implemented
* Interruption of the program with `SIGINT`, sending `SIGSTOP` to child processes and `SIGTERM` or `SIGCONT` depending on user input
* Complete logging of `CREATE`, `EXIT`, `RECV_PIPE`, `SEND_PIPE` and `ENTRY` type information
* Partial logging of `RECV_SIGNAL` and `SEND_SIGNAL` information (see below)

## Features that don't work
* `SIGSTOP` might not be logged on some occasions

## Relevant details
* The log file can become quite large when the program is run a lot of times
* We used `fork` in conjunction with `exec` to recursively call `simpledu`, providing a modified `argv` (with changes to the path and, if defined, `--max-depth`)
