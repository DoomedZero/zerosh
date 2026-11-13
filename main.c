#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{

    char buff[256];
    char *argv[32];
    char cwd[1024];

    printf("Welcome to ZeroSH....\n");

    while (1)
    {
        int argc = 0;

        getcwd(cwd, sizeof(cwd));

        printf("%s>", cwd);

        char *input = fgets(buff, sizeof(buff), stdin);

        char *token = strtok(buff, " \t\n");

        while (token != NULL && argc < 31)
        {
            argv[argc] = token;
            argc++;

            token = strtok(NULL, " \t\n");
        }

        argv[argc] = NULL;

        // This handles  EOF / Ctrl + D

        if (input == NULL)
        {
            printf("\n");
            break;
        }

        // This handles empty input / Enter

        else if (argc == 0)
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
                getcwd(cwd, sizeof(cwd));
                perror("cd");
            }
        }

        else if (strcmp(argv[0], "exit") == 0)
        {
            printf("Bye\n");
            break;
        }

        else if (strcmp(argv[0], "test") == 0)
        {
            printf("This is a test\n");
        }

        else if (strcmp(argv[0], "about") == 0)
        {
            printf("This is ZeroSH a shell written in C\n");
        }

        else if (strcmp(argv[0], "help") == 0)
        {
            printf("Commands:\n 1. about\n 2. test\n 3. exit\n");
        }

        else
        {
            printf("Command not found type help for information\n");
        }
    }

    return 0;
}