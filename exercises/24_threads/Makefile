CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g -pthread

TARGET  = threads

.PHONY: all clean valgrind

all: $(TARGET)

$(TARGET): threads.c
	$(CC) $(CFLAGS) -o $@ $<

valgrind: all
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

clean:
	rm -f $(TARGET)
