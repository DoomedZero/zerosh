
#define _POSIX_C_SOURCE 200809L

#include "input.h"

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include "history.h"

static void display_line(
    const char *prompt,
    const char *buffer)
{
    printf("\r\033[2K%s%s", prompt, buffer);
    fflush(stdout);
}

int read_line(const char *prompt, char *buffer, size_t size)
{
    struct termios original, input_mode;

    size_t len = 0;
    int result = 1;
    int escape_state = 0;
    int up_detected = 0;
    int down_detected = 0;
    size_t history_index = history_count();

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

    // Disable canonical mode and automatic echo
    // Keep ISIG enabled for Ctrl+C handling
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

        // First byte: ESC
        if (escape_state == 1)
        {
            escape_state = (ch == '[') ? 2 : 0;
            continue;
        }

        // Third byte: identify arrow key
        if (escape_state == 2)
        {
            escape_state = 0;

            // UP ARROW
            if (ch == 'A')
            {
                if (history_index > 0)
                {
                    history_index--;

                    const char *command = history_get(history_index);

                    if (command != NULL)
                    {
                        size_t command_len = strlen(command);

                        if (command_len >= size)
                        {
                            command_len = size - 1;
                        }

                        memcpy(buffer, command, command_len);
                        buffer[command_len] = '\0';

                        len = command_len;

                        display_line(prompt, buffer);
                    }
                }
            }

            // DOWN ARROW
            else if (ch == 'B')
            {
                if (history_index < history_count())
                {
                    history_index++;

                    if (history_index == history_count())
                    {
                        // Return to an empty input line
                        buffer[0] = '\0';
                        len = 0;
                    }
                    else
                    {
                        const char *command = history_get(history_index);

                        if (command != NULL)
                        {
                            size_t command_len = strlen(command);

                            if (command_len >= size)
                            {
                                command_len = size - 1;
                            }

                            memcpy(buffer, command, command_len);
                            buffer[command_len] = '\0';

                            len = command_len;
                        }
                    }

                    display_line(prompt, buffer);
                }
            }

            continue;
        }

        // Detect ESC
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

            if (up_detected)
            {
                printf("[DEBUG] Up arrow detected\n");
            }

            if (down_detected)
            {
                printf("[DEBUG] Down arrow detected\n");
            }

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

        // Normal printable characters
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
