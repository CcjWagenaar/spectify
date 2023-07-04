PROG=overflow overflow_arch
OBJS=overflow.o overflow_arch.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

overflow.o: overflow.c
overflow_arch.o: overflow_arch.c

clean:
	rm -f ${PROG} ${OBJS}