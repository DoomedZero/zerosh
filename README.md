# ZeroSH

**A minimal Unix-like command-line shell written in C.**

ZeroSH is a small systems programming project to explore, POSIX, GNU/LINUX Shell, and how these commands work under the hood.
This project was started me thinking- "How hard it can be ?!" and my state right now is
"HOW HARD IT CAN BE!!?? T.T"

## Terra Reviewers

- My C expierence helped me till adding FORK functionality in ZeroSH after that I had to use AI as assistant/tutor as I was out of ideas and code that it would need.
- AI was used as a tutor/assistant in website, portion of project contain directly copied code snippets and functions.

## Features

1. Display an interactive prompt.
2. Read and parse user commands.
3. Support built-in commands.
4. Manage child processes.
5. Execute external Linux programs.
6. Handle errors and terminate gracefully.
7. Compartmentalization of ZeroSH
8. command history using arrow keys
9. Command editing using arrow keys

## Technology

- **Language:** C
- **Standard:** C17
- **Compiler:** GCC
- **Platform:** GNU/Linux (Developement on Fedora and Arch)

## Compilation

Clone the repository and navigate to the project directory.

Compile using:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g main.c execute.c input.c history.c -o zerosh
```

## Execution

```bash
./zerosh
```

## How to use?
This is a normal shell, You can try running commands you know
or try run help, and you will get list of commands you can try

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
