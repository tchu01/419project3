CC=gcc
TARGET=tsp
CFLAGS=-Wall -g -std=c99 -O2 -D_GNU_SOURCE

default: tsp.c
	$(CC) -o $(TARGET) $(CFLAGS) tsp.c

clean:
	rm -f tsp
