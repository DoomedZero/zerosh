
#define _POSIX_C_SOURCE 200809L

#include "execute.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

int execute_command(char *argv[])
{
    pid_t pid = fork();

    if (pid == -1) // Error creating process
    {
        perror("fork");
        return 1;
    }

    else if (pid == 0) // Child Process
    {
        // Restores Default Ctrl + C behaviour

        struct sigaction default_action = {0};

        default_action.sa_handler = SIG_DFL;
        sigemptyset(&default_action.sa_mask);

        if (sigaction(SIGINT, &default_action, NULL) == -1)
        {
            perror("sigaction");
            _exit(126);
        }

        // Execute commands

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
        } while (result == -1 && errno == EINTR);

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
            if (WTERMSIG(status) == SIGINT)
            {
                putchar('\n');
                fflush(stdout);
            }

            return 128 + WTERMSIG(status);
        }

        return 1;
    }
}