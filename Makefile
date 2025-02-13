CC = gcc
CFLAGS = -g -std=c11 -pedantic -Wall -Wextra
# CFLAGS = -g -std=c11 -pedantic -Wall -Wextra -Werror

TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
