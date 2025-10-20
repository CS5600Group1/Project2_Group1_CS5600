#!/usr/bin/make -f

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
LDFLAGS = -pthread -lrt

OBJ_STAIRS = stairs.o stairs_main.o

TARGET = stairs
PLATFORM ?= unix

ifeq ($(PLATFORM), unix)
CCFLAGS = $(CC) $(CFLAGS) $(LDFLAGS)
else ifeq ($(PLATFORM), win)
CCFLAGS = $(CC) $(CFLAGS)
endif

all: $(TARGET)

$(TARGET): $(OBJ_STAIRS)
	$(CCFLAGS) -o $@ $^

stairs.o: stairs.c stairs.h
	$(CC) $(CFLAGS) -c stairs.c -o stairs.o

stairs_main.o: stairs_main.c stairs.h
	$(CC) $(CFLAGS) -c stairs_main.c -o stairs_main.o

.PHONY: clean all

clean:
	rm -f $(OBJ_STAIRS) $(TARGET)