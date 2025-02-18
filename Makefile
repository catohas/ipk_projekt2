CC = gcc
CFLAGS = -g -pedantic -Wall -Wextra -pthread -O2 -std=c11
DCFLAGS = -g -pedantic -Wall -Wextra -pthread -O2 -std=c11 -DDEBUG_PRINT
TARGET = main

SRCDIR = src

SOURCES = $(wildcard $(SRCDIR)/*.c)

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = $(DCFLAGS)
debug: clean $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

rebuild: clean all

.PHONY: all clean rebuild debug
