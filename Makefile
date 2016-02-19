CC=gcc
TARGET=tsp
CFLAGS=-Wall -g -std=c99 -O2 -D_GNU_SOURCE -lm
SRC=tsp.c
ifdef CPP
CC=g++
SRC=tsp.cpp
CFLAGS= -g -lstdc++
endif

default: tsp.c
	$(CC) -o $(TARGET) $(CFLAGS) $(SRC)

clean:
	rm -f tsp
