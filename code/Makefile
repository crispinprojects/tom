# Makefile for Talking Output Machine (TOM)

CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c99 
INCLUDES = -Isrc
SRCDIR = src
TARGET = tom

# Source files
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
