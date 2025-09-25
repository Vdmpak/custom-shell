CC = gcc
CFLAGS = -Wall -Wextra
TARGET = ../bin/customshell

$(TARGET): customshell.c utility.c customshell.h
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
