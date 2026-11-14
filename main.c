#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void execute_command(char *argv[])
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return;
    }

    else if (pid == 0)
    {
        execvp(argv[0], argv);

        perror(argv[0]);
        _exit(127);
    }

    else
    {
        waitpid(pid, NULL, 0);
    }
}

int main(void)
{

    char buff[256];
    char *argv[32];
    char cwd[1024];

    while (1)
    {
        int argc = 0;

        getcwd(cwd, sizeof(cwd));

        printf("%s>", cwd);

        char *input = fgets(buff, sizeof(buff), stdin);

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

        char *token = strtok(buff, " \t\n");

        while (token != NULL && argc < 31)
        {
            argv[argc] = token;
            argc++;

            token = strtok(NULL, " \t\n");
        }

        argv[argc] = NULL;

        // This handles empty input / Enter

        if (argc == 0)
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
            printf("Internal commands: \n");
            printf(" mkdir\n ls\n rmdir\n cat\n");
            printf("External commands: \n");
            printf(" git\n python\n node\n npm\n");
        }

        // Command: exit

        else if (strcmp(argv[0], "exit") == 0)
        {
            break;
        }

        else
        {
            execute_command(argv);
        }
    }

    return 0;
}