CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

VALGRIND = valgrind --leak-check=full --error-exitcode=1

# Directories that manage their own build (have a local Makefile)
_MANAGED     := $(shell find exercises topics -mindepth 2 -maxdepth 2 -name 'Makefile' \
                         -exec dirname {} \; 2>/dev/null)
_EXCL        := $(foreach d,$(_MANAGED),-not -path '$(d)/*.c')

# Single-file exercises: one .c per directory, built by this Makefile
SRCS := $(shell find exercises topics $(_EXCL) -name '*.c')
BINS := $(SRCS:.c=)

.PHONY: all clean valgrind

all: $(BINS)
	@for d in $(_MANAGED); do $(MAKE) -C $$d all; done

# Pattern rule: compile any standalone .c to a binary alongside it
%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lm

valgrind: all
	@for bin in $(BINS); do \
		echo "--- $$bin ---"; \
		$(VALGRIND) $$bin 2>&1 | grep -E "ERROR SUMMARY|no leaks"; \
	done
	@for d in $(_MANAGED); do $(MAKE) -C $$d valgrind 2>/dev/null || true; done

clean:
	@find exercises topics -type f ! -name '*.c' ! -name '*.h' ! -name 'Makefile' -delete
	@for d in $(_MANAGED); do $(MAKE) -C $$d clean; done
