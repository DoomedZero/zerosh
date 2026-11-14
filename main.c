#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>

void handle_sigint(int signo)
{
    (void)signo;
}

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

int read_line(char *buffer, size_t size)
{
    struct termios original, input_mode;

    size_t len = 0;
    int result = 1;
    int escape_state = 0;

    if (size == 0)
    {
        errno = EINVAL;
        return -1;
    }

    // Save original terminal settings
    if (tcgetattr(STDIN_FILENO, &original) == -1)
    {
        return -1;
    }

    input_mode = original;

    // Disable canonical input and automatic echo
    input_mode.c_lflag &= ~(ICANON | ECHO);

    input_mode.c_cc[VMIN] = 1;
    input_mode.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &input_mode) == -1)
    {
        return -1;
    }

    buffer[0] = '\0';

    while (1)
    {
        char ch;

        ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);

        if (bytes_read == -1)
        {
            if (errno == EINTR)
            {
                len = 0;
                result = 2;
                break;
            }

            result = -1;
            break;
        }

        if (bytes_read == 0)
        {
            result = (len == 0) ? 0 : 1;
            break;
        }

        // Ignore terminal escape sequences for now
        if (escape_state == 1)
        {
            escape_state = (ch == '[') ? 2 : 0;
            continue;
        }

        if (escape_state == 2)
        {
            escape_state = 0;
            continue;
        }

        if (ch == 27)
        {
            escape_state = 1;
            continue;
        }

        // Ctrl+D
        if (ch == 4 && len == 0)
        {
            putchar('\n');
            result = 0;
            break;
        }

        // Enter
        if (ch == '\n' || ch == '\r')
        {
            putchar('\n');
            result = 1;
            break;
        }

        // Backspace
        if (ch == 127 || ch == 8)
        {
            if (len > 0)
            {
                len--;
                buffer[len] = '\0';

                printf("\b \b");
                fflush(stdout);
            }

            continue;
        }

        // Normal characters
        if (ch >= 32 && ch <= 126)
        {
            if (len < size - 1)
            {
                buffer[len++] = ch;
                buffer[len] = '\0';

                putchar(ch);
                fflush(stdout);
            }
            else
            {
                putchar('\a');
                fflush(stdout);
            }
        }
    }

    // Restore original terminal settings
    int saved_errno = errno;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &original) == -1)
    {
        return -1;
    }

    if (result == -1)
    {
        errno = saved_errno;
        return -1;
    }

    if (result == 2)
    {
        printf("^C\n");
    }

    buffer[len] = '\0';

    return result;
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

        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s> ", cwd);
        }
        else
        {
            perror("getcwd");
            printf("zerosh> ");
        }
        fflush(stdout);

        int input_result = read_line(buff, sizeof(buff));

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