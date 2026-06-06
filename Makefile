CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

# Find all .c files under exercises/ and topics/
SRCS := $(shell find exercises topics -name '*.c')
BINS := $(SRCS:.c=)

.PHONY: all clean

all: $(BINS)

# Pattern rule: compile any .c to a binary alongside it
%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	@find exercises topics -type f ! -name '*.c' -delete
