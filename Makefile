CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

# Find all .c files under exercises/ and topics/
SRCS := $(shell find exercises topics -name '*.c')
BINS := $(SRCS:.c=)

VALGRIND = valgrind --leak-check=full --error-exitcode=1

.PHONY: all clean valgrind

all: $(BINS)

# Pattern rule: compile any .c to a binary alongside it
%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lm

valgrind: all
	@for bin in $(BINS); do \
		echo "--- $$bin ---"; \
		$(VALGRIND) $$bin 2>&1 | grep -E "ERROR SUMMARY|no leaks"; \
	done

clean:
	@find exercises topics -type f ! -name '*.c' -delete
