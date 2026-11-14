
#include "history.h"

#include <ctype.h>
#include <string.h>

// Private history storage
static char commands[HISTORY_MAX][HISTORY_LINE_SIZE];
static size_t count = 0;

// Store a new command
void history_add(const char *command)
{
    if (command == NULL)
    {
        return;
    }

    // Ignore empty or whitespace-only commands
    const unsigned char *p = (const unsigned char *)command;

    while (*p != '\0' && isspace(*p))
    {
        p++;
    }

    if (*p == '\0')
    {
        return;
    }

    // If full, discard the oldest command
    if (count == HISTORY_MAX)
    {
        memmove(commands[0],
                commands[1],
                (HISTORY_MAX - 1) * sizeof(commands[0]));

        count--;
    }

    // Copy the command into history
    size_t len = strlen(command);

    if (len >= HISTORY_LINE_SIZE)
    {
        len = HISTORY_LINE_SIZE - 1;
    }

    memcpy(commands[count], command, len);
    commands[count][len] = '\0';

    count++;
}

// Return number of stored commands
size_t history_count(void)
{
    return count;
}

// Retrieve a command by index
const char *history_get(size_t index)
{
    if (index >= count)
    {
        return NULL;
    }

    return commands[index];
}
