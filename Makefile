TARGET=partsp
CC=icc
SRC=tsp.cpp
CFLAGS= -g -w -lstdc++ -qopenmp

default: tsp.c
	$(CC) -o $(TARGET) $(CFLAGS) $(SRC)

clean:
	rm -f tsp
