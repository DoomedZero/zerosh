
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "execute.h"
#include "input.h"
#include "history.h"

#define INPUT_SIZE 256
#define MAX_ARGS 32
#define CWD_SIZE 1024
#define PROMPT_SIZE 1100

void handle_sigint(int signo)
{
    (void)signo;
}

int main(void)
{
    char buff[INPUT_SIZE];       // Input buffer
    char *argv[MAX_ARGS];        // Argument pointers
    char cwd[CWD_SIZE];          // Current working directory

    int last_status = 0;

    // Configure SIGINT handling
    struct sigaction sa = {0};

    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    // Load persistent command history
    if (history_load() == -1)
    {
        perror("history_load");
    }

    while (1)
    {
        size_t argc = 0;
        char prompt[PROMPT_SIZE];

        // Construct current working directory prompt
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            int prompt_result = snprintf(
                prompt, sizeof(prompt), "%s> ", cwd
            );

            if (prompt_result < 0 ||
                (size_t)prompt_result >= sizeof(prompt))
            {
                fprintf(stderr, "zerosh: prompt is too long\n");
                return 1;
            }
        }
        else
        {
            perror("getcwd");
            snprintf(prompt, sizeof(prompt), "zerosh> ");
        }

        printf("%s", prompt);
        fflush(stdout);

        // Read user input
        int input_result = read_line(prompt, buff, sizeof(buff));

        // Handle EOF / Ctrl+D
        if (input_result == 0)
        {
            break;
        }

        // Handle input errors
        if (input_result == -1)
        {
            perror("read_line");
            break;
        }

        // Handle Ctrl+C
        if (input_result == 2)
        {
            continue;
        }

        // Store command before tokenization
        if (history_add(buff) == -1)
        {
            perror("history_add");
        }

        // Tokenization
        char *token = strtok(buff, " \t\n");

        while (token != NULL && argc < MAX_ARGS - 1)
        {
            argv[argc] = token;
            argc++;

            token = strtok(NULL, " \t\n");
        }

        // Reject commands exceeding argument capacity
        if (token != NULL)
        {
            fprintf(
                stderr,
                "zerosh: too many arguments (maximum %d)\n",
                MAX_ARGS - 1
            );

            continue;
        }

        // Terminate argument array
        argv[argc] = NULL;

        // Ignore empty input
        if (argc == 0)
        {
            continue;
        }

        // Built-in: cd
        else if (strcmp(argv[0], "cd") == 0)
        {
            if (argc != 2)
            {
                printf("Usage: cd <directory>\n");
            }
            else if (chdir(argv[1]) == -1)
            {
                perror("cd");
            }
        }

        // Built-in: pwd
        else if (strcmp(argv[0], "pwd") == 0)
        {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("%s\n", cwd);
            }
            else
            {
                perror("pwd");
            }
        }

        // Built-in: help
        else if (strcmp(argv[0], "help") == 0)
        {
            printf("Built-in commands:\n");
            printf("  exit\n");
            printf("  help\n");
            printf("  cd\n");
            printf("  pwd\n");
            printf("  status\n");

            printf("\nExternal commands:\n");
            printf("  ls, mkdir, rmdir, cat, git, gcc, etc.\n");
        }

        // Built-in: exit
        else if (strcmp(argv[0], "exit") == 0)
        {
            break;
        }

        // Debugging: last child exit status
        else if (strcmp(argv[0], "status") == 0)
        {
            printf("Last child exit status: %d\n", last_status);
        }

        // External commands
        else
        {
            last_status = execute_command(argv);
        }
    }

    return 0;
}
