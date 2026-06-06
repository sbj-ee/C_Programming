CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

TARGET  = multi_file
OBJS    = main.o vec2.o stats.o

.PHONY: all clean valgrind

all: $(TARGET)

# Link all object files into the final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compile each .c to a .o; headers listed as dependencies so a header
# change triggers a recompile of every .c that includes it.
main.o:  main.c  vec2.h stats.h
	$(CC) $(CFLAGS) -c $<

vec2.o:  vec2.c  vec2.h
	$(CC) $(CFLAGS) -c $<

stats.o: stats.c stats.h
	$(CC) $(CFLAGS) -c $<

valgrind: all
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
