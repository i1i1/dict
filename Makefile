CC=gcc -std=c89
CFLAGS=-Wall -Wextra -pedantic -g
OUT=prog
LIB=libdict
SAMPLES=$(wildcard samples/*)
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

all: $(OUT) $(LIB)

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(LIB): dict.o primes.o
	ar rcs $@.a $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

test: $(OUT)
	./$(OUT) $(SAMPLES)

clean:
	rm -f $(OBJS) $(OUT) $(LIB).a

.PHONY: all clean test

