CC = gcc
CFLAGS = -pedantic -Wall -Wextra -pthread -lm -std=c17
DCFLAGS = -g -pedantic -Wall -Wextra -pthread -lm -std=c17 -DDEBUG_PRINT

TARGET = ipk25chat-client

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
