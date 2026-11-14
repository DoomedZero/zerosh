
#define _POSIX_C_SOURCE 200809L

#include "input.h"
#include "history.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>

static void redraw_line(
    const char *prompt,
    const char *buffer,
    size_t cursor)
{
    size_t len = strlen(buffer);

    printf("\r\033[2K%s%s", prompt, buffer);

    if (cursor < len)
    {
        printf("\033[%zuD", len - cursor);
    }

    fflush(stdout);
}

static void replace_buffer(
    char *buffer,
    size_t size,
    size_t *len,
    size_t *cursor,
    const char *text)
{
    size_t copy_len = strlen(text);

    if (copy_len >= size)
    {
        copy_len = size - 1;
    }

    memcpy(buffer, text, copy_len);

    buffer[copy_len] = '\0';

    *len = copy_len;
    *cursor = copy_len;
}

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
    size_t cursor = 0;

    int result = 1;
    int escape_state = 0;

    size_t history_index = history_count();

    int browsing_history = 0;

    if (size == 0)
    {
        errno = EINVAL;
        return -1;
    }

    char *draft = malloc(size);

    if (draft == NULL)
    {
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

        // ESC received
        if (escape_state == 1)
        {
            escape_state = (ch == '[') ? 2 : 0;
            continue;
        }

        // Identify the key after ESC [
        if (escape_state == 2)
        {
            escape_state = 0;

            // UP ARROW
            if (ch == 'A')
            {
                size_t count = history_count();

                if (count > 0)
                {
                    // Save current input before browsing
                    if (!browsing_history)
                    {
                        memcpy(draft, buffer, len + 1);

                        history_index = count;
                        browsing_history = 1;
                    }

                    if (history_index > 0)
                    {
                        history_index--;

                        const char *command = history_get(history_index);

                        if (command != NULL)
                        {
                            replace_buffer(
                                buffer, size, &len, &cursor, command);

                            redraw_line(prompt, buffer, cursor);
                        }
                    }
                }
            }

            // DOWN ARROW
            else if (ch == 'B')
            {
                if (browsing_history)
                {
                    size_t count = history_count();

                    if (history_index + 1 < count)
                    {
                        history_index++;

                        const char *command = history_get(history_index);

                        if (command != NULL)
                        {
                            replace_buffer(
                                buffer, size, &len, &cursor, command);
                        }
                    }
                    else
                    {
                        history_index = count;

                        // Restore the input typed before pressing UP
                        replace_buffer(
                            buffer, size, &len, &cursor, draft);

                        browsing_history = 0;
                    }

                    redraw_line(prompt, buffer, cursor);
                }
            }

            // RIGHT ARROW
            else if (ch == 'C')
            {
                if (cursor < len)
                {
                    cursor++;
                    redraw_line(prompt, buffer, cursor);
                }
            }

            // LEFT ARROW
            else if (ch == 'D')
            {
                if (cursor > 0)
                {
                    cursor--;
                    redraw_line(prompt, buffer, cursor);
                }
            }

            // HOME
            else if (ch == 'H')
            {
                cursor = 0;
                redraw_line(prompt, buffer, cursor);
            }

            // END
            else if (ch == 'F')
            {
                cursor = len;
                redraw_line(prompt, buffer, cursor);
            }

            // Delete sequence begins with ESC [ 3 ~
            else if (ch == '3')
            {
                escape_state = 3;
            }

            continue;
        }

        // Complete Delete sequence
        if (escape_state == 3)
        {
            escape_state = 0;

            if (ch == '~' && cursor < len)
            {
                memmove(
                    buffer + cursor,
                    buffer + cursor + 1,
                    len - cursor);

                len--;

                browsing_history = 0;
                history_index = history_count();

                redraw_line(prompt, buffer, cursor);
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
            result = 1;
            break;
        }

        // Backspace
        if (ch == 127 || ch == 8)
        {
            if (cursor > 0)
            {
                memmove(
                    buffer + cursor - 1,
                    buffer + cursor,
                    len - cursor + 1);

                cursor--;
                len--;

                browsing_history = 0;
                history_index = history_count();

                redraw_line(prompt, buffer, cursor);
            }

            continue;
        }

        // Normal printable characters
        if (ch >= 32 && ch <= 126)
        {
            if (len < size - 1)
            {
                memmove(
                    buffer + cursor + 1,
                    buffer + cursor,
                    len - cursor + 1);

                buffer[cursor] = ch;

                cursor++;
                len++;

                browsing_history = 0;
                history_index = history_count();

                redraw_line(prompt, buffer, cursor);
            }
            else
            {
                putchar('\a');
                fflush(stdout);
            }

            continue;
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
        free(draft);
        errno = saved_errno;
        return -1;
    }

    if (result == 2)
    {
        printf("^C\n");
    }

    buffer[len] = '\0';
    free(draft);

    return result;
}
