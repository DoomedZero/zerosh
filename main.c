#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int execute_command(char *argv[])
{
    pid_t pid = fork();

    if (pid == -1) // Error creating process
    {
        perror("fork");
        return 1;
    }

    else if (pid == 0) // Child process
    {
        execvp(argv[0], argv);

        perror(argv[0]);
        _exit(127);
    }

    else // Parent process
    {
        int status; // Status is used to store encoded termination information
        pid_t result;

        do
        {
            result = waitpid(pid, &status, 0);
        } 
        while (result == -1 && errno == EINTR);

        if (result == -1)
        {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            return 128 + WTERMSIG(status);
        }

        return 1;
    }
}

int main(void)
{

    char buff[256];      // Input buff to be tokenized
    char *argv[32];      // Tokens
    char cwd[1024];      // Current Working Directory
    int last_status = 0; // Debugging exit status of child processes

    while (1)
    {
        int argc = 0; // Number of input tokens

        getcwd(cwd, sizeof(cwd));

        printf("%s>", cwd);

        char *input = fgets(buff, sizeof(buff), stdin); // Taking Buff

        // Check input result before parsing

        if (input == NULL)
        {
            if (feof(stdin))
            {
                printf("\n");
                break;
            }
            else if (ferror(stdin))
            {
                perror("fgets");
                break;
            }
        }

        char *token = strtok(buff, " \t\n"); // Tokenization

        // Stacking tokens

        while (token != NULL && argc < 31)
        {
            argv[argc] = token;
            argc++;

            token = strtok(NULL, " \t\n");
        }

        argv[argc] = NULL; // Termination of stacked tokens

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