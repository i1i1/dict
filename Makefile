CC=gcc
CFLAGS=-Wall -g -Wextra
RM=rm -rf
OUT=test
SAMPLES=$(wildcard samples/*)

all: build

build: vector/vector.o
	$(CC) $(CFLAGS) -o $(OUT) *.c vector/vector.o

vector/vector.o:
	make -C vector

test: all
	./$(OUT) $(SAMPLES)

debug: CFLAGS+=-DDEBUG_ON
debug: build

clean:
	$(RM) *.o $(OUT)

.PHONY: debug build clean all

