# Linux-Shell

A customized Linux shell implemented by C.

The shell accepts user commands and then executes each command in a separate process. The shell provides the user a prompt at which the next command is entered.

One technique for implementing a shell interface is to have the parent process first read what the user enters on the command line and then create a separate child process that performs the command. Unless otherwise specified, the parent process waits for the child to exit before continuing. However, UNIX shells typically also allow the child process to run in the background - or concurrently - as well by specifying the ampersand (&) at the end of the command. The separate child process is created using the `fork()` system call and the user's command is executed by using one of the system calls in the `exec()` family.

## Install

Install the repository.

```shell
$ make
$ ./shell
```

## Usage

### Basic Shell

To implement a shell interface, the parent process first calls `read_command()` to reads a full command from the user and tokenizes it into separate arguments. Those tokens can be passed directly to `execvp()` in the child process.

Unless otherwise specified, the parent process waits for the child to exit before continuing. However, UNIX shells typically also allow the child process to run in the background - or concurrently - as well by specifying the ampersand (&) at the end of the command. In such circumstance, `read_command()` will set the `in_background` parameter to true

The `main()` function first calls `read_command()`, which .  If the user enters an "&" as the final argument, `read_command()` will set the `in_background` parameter to true

1. `Fork()` a child process
2. Child process invokes `execvp()` using results in token array.
3. If in_background is false, parent waits for child to finish. Otherwise, parent loops back to `read_command()` again immediately.

```shell
/home/alex_wang_10/repos/Linux-Shell$ ls
makefile  README.md  shell  shell.c  shell.o
/home/alex_wang_10/repos/Linux-Shell$
```

```shell
/home/alex_wang_10/repos/Linux-Shell$ ls &
/home/alex_wang_10/repos/Linux-Shell$ makefile  README.md  shell  shell.c  shell.o
```

### Internal Commands

Internal commands are built-in features of the shell itself, as opposed to a separate program that is executed. The commands listed below are implemented.

* `exit`: Exit the shell program.
* `pwd`: Display the current working directory.
* `cd`: Change the current working directory.
* `help`: Display help information on internal commands.

```shell
/home/alex_wang_10/repos/Linux-Shell$ cd ~
/home/alex_wang_10$ cd -
/home/alex_wang_10/repos/Linux-Shell$ cd 
/home/alex_wang_10$ cd repos/Linux-Shell
/home/alex_wang_10/repos/Linux-Shell$
```

### History Feature

Shell allows the user access up to 10 most recently entered commands.

* Command "!n" runs command number n.
* Command "!!" runs the previous command.

```shell
/home/alex_wang_10/repos$ history
23        history
22        touch a.c
21        echo Hello!
20        man pthread_create
19        ls
18        cd repos
17        cd
16        cd ~
15        cd -
14        pwd
/home/alex_wang_10/repos$
```

```shell
alex_wang_10@DESKTOP-B47DDOK:~/repos/Linux-Shell$ ./shell
/home/alex_wang_10/repos/Linux-Shell$ echo Hello!
Hello!
/home/alex_wang_10/repos/Linux-Shell$ !!
echo Hello!
Hello!
/home/alex_wang_10/repos/Linux-Shell$ ls
makefile  README.md  shell  shell.c  shell.o
/home/alex_wang_10/repos/Linux-Shell$ !2
ls
makefile  README.md  shell  shell.c  shell.o
```

### Signals

Change your shell program to display the help information when the user presses ctrl-c (which is the SIGINT signal).

```shell
/home/alex_wang_10/repos/Linux-Shell$ ^C
A simple Linux Shell, version 1.0.1-release
These shell commands are defined internally.
cd [dir]         Change the current working directory.
exit             Exit the shell program.
help [command]   Display help information on internal commands.
pwd              Display the current working directory.
/home/alex_wang_10/repos/Linux-Shell$
```

