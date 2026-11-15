
#define _POSIX_C_SOURCE 200809L

#include "history.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define HISTORY_PATH_SIZE 4096

// Private history storage
static char commands[HISTORY_MAX][HISTORY_LINE_SIZE];
static size_t count = 0;


// Construct ~/.zerosh_history
static int get_history_path(char *path, size_t size)
{
    const char *home = getenv("HOME");

    if (home == NULL || home[0] == '\0')
    {
        errno = ENOENT;
        return -1;
    }

    int result = snprintf(
        path,
        size,
        "%s/.zerosh_history",
        home
    );

    if (result < 0 || (size_t)result >= size)
    {
        errno = ENAMETOOLONG;
        return -1;
    }

    return 0;
}


// Store a command in memory
static void store_command(const char *command)
{
    if (command == NULL)
    {
        return;
    }

    // Ignore whitespace-only commands
    const unsigned char *p =
        (const unsigned char *)command;

    while (*p != '\0' && isspace(*p))
    {
        p++;
    }

    if (*p == '\0')
    {
        return;
    }

    // Remove oldest entry if history is full
    if (count == HISTORY_MAX)
    {
        memmove(
            commands[0],
            commands[1],
            (HISTORY_MAX - 1) * sizeof(commands[0])
        );

        count--;
    }

    size_t len = strlen(command);

    if (len >= HISTORY_LINE_SIZE)
    {
        len = HISTORY_LINE_SIZE - 1;
    }

    memcpy(commands[count], command, len);

    commands[count][len] = '\0';

    count++;
}


// Load history from disk
int history_load(void)
{
    char path[HISTORY_PATH_SIZE];

    count = 0;

    if (get_history_path(path, sizeof(path)) == -1)
    {
        return -1;
    }

    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        // No history file is normal on first launch
        if (errno == ENOENT)
        {
            return 0;
        }

        return -1;
    }

    char line[HISTORY_LINE_SIZE + 2];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t len = strlen(line);

        if (len > 0 && line[len - 1] == '\n')
        {
            line[--len] = '\0';
        }
        else if (!feof(file))
        {
            // Discard an unexpectedly long history entry
            int ch;

            while ((ch = fgetc(file)) != '\n' && ch != EOF)
            {
                /* discard */
            }

            continue;
        }

        // Handle possible Windows-style CRLF
        if (len > 0 && line[len - 1] == '\r')
        {
            line[--len] = '\0';
        }

        store_command(line);
    }

    if (ferror(file))
    {
        int saved_errno = errno;
        fclose(file);
        errno = saved_errno;
        return -1;
    }

    if (fclose(file) == EOF)
    {
        return -1;
    }

    return 0;
}


// Add command to memory and disk
int history_add(const char *command)
{
    if (command == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Ignore empty or whitespace-only commands
    const unsigned char *p =
        (const unsigned char *)command;

    while (*p != '\0' && isspace(*p))
    {
        p++;
    }

    if (*p == '\0')
    {
        return 0;
    }

    // First save in memory
    store_command(command);

    char path[HISTORY_PATH_SIZE];

    if (get_history_path(path, sizeof(path)) == -1)
    {
        return -1;
    }

    // Open file in append mode, creating it if necessary
    int fd = open(
        path,
        O_WRONLY | O_CREAT | O_APPEND,
        S_IRUSR | S_IWUSR
    );

    if (fd == -1)
    {
        return -1;
    }

    // Prepare command followed by newline
    char line[HISTORY_LINE_SIZE];

    size_t len = strlen(command);

    if (len >= HISTORY_LINE_SIZE)
    {
        len = HISTORY_LINE_SIZE - 1;
    }

    memcpy(line, command, len);

    line[len] = '\n';

    size_t total = len + 1;
    size_t written = 0;

    // Handle partial writes and interruptions
    while (written < total)
    {
        ssize_t result = write(
            fd,
            line + written,
            total - written
        );

        if (result == -1 && errno == EINTR)
        {
            continue;
        }

        if (result <= 0)
        {
            int saved_errno = (result == 0) ? EIO : errno;
            close(fd);
            errno = saved_errno;
            return -1;
        }

        written += (size_t)result;
    }

    if (close(fd) == -1)
    {
        return -1;
    }

    return 0;
}


// Return number of commands
size_t history_count(void)
{
    return count;
}


// Retrieve command by index
const char *history_get(size_t index)
{
    if (index >= count)
    {
        return NULL;
    }

    return commands[index];
}
