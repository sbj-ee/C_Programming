CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g

TARGET = dl_demo
PLUGIN = plugin.so

.PHONY: all clean

all: $(PLUGIN) $(TARGET)

$(PLUGIN): plugin.c plugin.h
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@ -lm

$(TARGET): dl_demo.c plugin.h
	$(CC) $(CFLAGS) $< -o $@ -ldl

clean:
	rm -f $(TARGET) $(PLUGIN)
