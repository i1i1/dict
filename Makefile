CC=gcc
CFLAGS=-Wall
RM=rm -rf
OUT=test

all: build

build: 
	$(CC) $(CFLAGS) -o $(OUT) *.c

debug: CFLAGS+=-DDEBUG_ON
debug: build

clean:
	$(RM) *.o $(OUT)

