CC = gcc
CFLAGS = -pedantic -Wall -Wextra -O2 -std=c11
DCFLAGS = -g -pedantic -Wall -Wextra -O2 -std=c11 -DDEBUG_PRINT

TARGET = main

SRCDIR = src
BUILDDIR = $(SRCDIR)/build

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

MAKEFLAGS += -j

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = $(DCFLAGS)
debug: $(TARGET)

clean:
	rm -f $(TARGET) $(BUILDDIR)/*.o

rebuild: clean all

.PHONY: all clean rebuild debug
