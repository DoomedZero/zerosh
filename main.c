#include <stdio.h>
#include <string.h>

int main(void)
{

    char input[10];

    printf("Welcome to ZeroSH....\n");

    while (1)
    {

        printf("zerosh>");

        if (scanf("%9s", input) != 1)
        {
            break;
        }

        if (strcmp(input, "exit") == 0)
        {
            printf("Bye\n");
            break;
        }

        if (strcmp(input, "test") == 0)
        {
            printf("This is a test\n");
        }
        
    }
    

    return 0;
}