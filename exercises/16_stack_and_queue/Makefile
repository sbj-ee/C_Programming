CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
TARGET = stack_queue
SRC    = stack_queue.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)
