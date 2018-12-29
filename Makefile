CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g
OUT=prog
SAMPLES=$(wildcard samples/*)
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)


$(OUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

test: $(OUT)
	./$(OUT) $(SAMPLES)

clean:
	rm $(OBJS) $(OUT)

.PHONY: clean test

