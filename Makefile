
CC = gcc

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g

SOURCES = main.c execute.c input.c history.c
TARGET = zerosh

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

release:
	$(CC) -std=c17 -O2 -Wall -Wextra -Wpedantic $(SOURCES) -o $(TARGET)

.PHONY: all clean release
