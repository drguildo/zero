CFLAGS = -g -Wall -Wextra
CC = cc

all: zero

util.o: util.c util.h
	${CC} ${CFLAGS} -c util.c

main.o: main.c
	${CC} ${CFLAGS} -c main.c

zero: main.o util.o
	${CC} -o zero main.o util.o

clean:
	rm -f zero main.o util.o

.PHONY: all clean
