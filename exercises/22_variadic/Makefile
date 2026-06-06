CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
TARGET = variadic
SRC    = variadic.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	rm -f $(TARGET)
