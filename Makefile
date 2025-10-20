#!/usr/bin/make -f

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = stairs
OBJ_STAIRS = stairs.o stairs_main.o

all: TARGET

$(TARGET): $(OBJ_STAIRS)
	$(CC) $(CFLAGS) -o $@ $^

stairs.o: stairs.c stairs.h
	$(CC) $(CFLAGS) -c stairs.c -o stairs.o

stairs_main.o: stairs_main.c stairs.h
	$(CC) $(CFLAGS) -c stairs_main.c -o stairs_main.o


.PHONY: clean

clean:
	rm -f $(OBJ_STAIRS) $(TARGET).exe