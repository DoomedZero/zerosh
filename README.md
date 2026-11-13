# ZeroSH

**A minimal Unix-like command-line shell written in C.**

ZeroSH is a small systems programming project focused on understanding how command-line shells interact with the Linux operating system.

## Features

- [x] Basic interactive prompt
- [x] Command-line input handling
- [x] Command parsing and argument handling
- [x] External command execution
- [x] Process creation using `fork()`
- [x] Program execution using `execvp()`
- [x] Process management using `waitpid()`
- [x] Built-in `cd` command
- [x] Built-in `exit` command
- [ ] Built-in `help` command
- [ ] Error handling

## Technology

- **Language:** C
- **Standard:** C17
- **Compiler:** GCC
- **Platform:** GNU/Linux (Developement on Fedora and Arch)

## Compilation

Clone the repository and navigate to the project directory.

Compile using:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g main.c -o zerosh
```

## Execution

```bash
./zerosh
```

## Current Status

ZeroSH is under active development. The current prototype has a basic interactive prompt and input loop. Command parsing and operating-system process integration are planned.


## Learning Objectives

- C programming and memory fundamentals
- String and input handling
- Process creation and management
- POSIX system calls
- Linux command execution
- Debugging and error handling

---

**ZeroSH**