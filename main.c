#include <stdio.h>
#include <string.h>

int main(void)
{

    char buff[256];

    printf("Welcome to ZeroSH....\n");

    while (1)
    {

        printf("zerosh>");

        fgets(buff, sizeof(buff), stdin);

        char *input = strtok(buff, " \t\n");

        if (input == NULL)
        {
            continue;
        }

        else if (strcmp(input, "exit") == 0)
        {
            printf("Bye\n");
            break;
        }

        else if (strcmp(input, "test") == 0)
        {
            printf("This is a test\n");
        }

        else if (strcmp(input, "about") == 0)
        {
            printf("This is ZeroSH a shell written in C\n");
        }

        else if (strcmp(input, "help") == 0)
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