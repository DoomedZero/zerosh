
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "execute.h"
#include "input.h"
#include "history.h"

void handle_sigint(int signo)
{
    (void)signo;
}

int main(void)
{

    char buff[256];      // Input buff to be tokenized
    char *argv[32];      // Tokens
    char cwd[1024];      // Current Working Directory
    int last_status = 0; // Debugging exit status of child processes

    struct sigaction sa = {0}; // Stores signal config

    sa.sa_handler = handle_sigint; // Specifies handler function
    sigemptyset(&sa.sa_mask);      // Starts empty
    sa.sa_flags = 0;               // Does not request automatic restart of interrupted system calls

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    while (1)
    {
        int argc = 0; // Number of input tokens (Resets every loop)

        char prompt[1100];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            snprintf(prompt, sizeof(prompt), "%s> ", cwd);
        }
        else
        {
            snprintf(prompt, sizeof(prompt), "zerosh> ");
        }

        printf("%s", prompt);
        fflush(stdout);

        int input_result = read_line(prompt, buff, sizeof(buff));

        if (input_result == 0)
        {
            break;
        }

        if (input_result == -1)
        {
            perror("read_line");
            break;
        }

        if (input_result == 2)
        {
            continue;
        }

        // Add buff to history

        history_add(buff);

        char *token = strtok(buff, " \t\n"); // Tokenization

        // Stacking tokens

        while (token != NULL && argc < 31)
        {
            argv[argc] = token;
            argc++;

            token = strtok(NULL, " \t\n");
        }
        argv[argc] = NULL; // Termination of stacked tokens

        // REPL: Starts

        if (argc == 0) // Empty input / Enter
        {
            continue;
        }

        // Command: cd

        else if (strcmp(argv[0], "cd") == 0)
        {
            if (argc != 2)
            {
                printf("Usage: cd <directory> \n");
            }
            else if (chdir(argv[1]) == -1)
            {
                perror("cd");
            }
        }

        // Command: pwd

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

        // Command: help

        else if (strcmp(argv[0], "help") == 0)
        {
            printf("Built in commands: \n");
            printf(" exit\n help\n cd\n pwd\n");
            printf("Forked commands: \n");
            printf(" mkdir\n ls\n rmdir\n cat\n");
        }

        // Command: exit

        else if (strcmp(argv[0], "exit") == 0)
        {
            break;
        }

        // Command: status

        else if (strcmp(argv[0], "status") == 0)
        {
            printf("Last exit status: %d\n", last_status);
        }

        // Forked Commands

        else
        {
            last_status = execute_command(argv);
        }
    }

    return 0;
}
