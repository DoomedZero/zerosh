#include <stdio.h>
#include <string.h>

int main(void)
{

    char buff[256];
    char *argv[32];

    printf("Welcome to ZeroSH....\n");

    while (1)
    {
        int argc = 0;

        printf("zerosh>");

        fgets(buff, sizeof(buff), stdin);

        char *token = strtok(buff, " \t\n");

        while (token != NULL && argc < 31)
        {
            argv[argc] = token;
            argc++;

            token = strtok(NULL, " \t\n");
        }

        argv[argc] = NULL;

        if (argc == 0)
        {
            continue;
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

        printf("Argument Count: %d \n", argc);
        printf("Arguement 1 : %s \n", argv[0]);
        printf("Arguement 2 : %s \n", argv[1]);
    }

    return 0;
}