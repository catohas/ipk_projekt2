CC = gcc
CFLAGS = -g -pedantic -Wall -Wextra -pthread -O2 -std=c11
# CFLAGS = -g -pedantic -Wall -Wextra -pthread -Werror

TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
