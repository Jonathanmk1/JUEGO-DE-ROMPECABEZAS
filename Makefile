# Makefile simple para compilar el rompecabezas Kinect en C
CC=gcc
CFLAGS=-Wall -O2
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
OUT=rompecabezas

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -f src/*.o $(OUT)
