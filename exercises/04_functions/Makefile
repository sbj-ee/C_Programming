CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
TARGET = functions
SRC    = functions.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)
