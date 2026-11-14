
#ifndef ZEROSH_HISTORY_H
#define ZEROSH_HISTORY_H

#include <stddef.h>

#define HISTORY_MAX 100
#define HISTORY_LINE_SIZE 256

void history_add(const char *command);

size_t history_count(void);

const char *history_get(size_t index);

#endif
