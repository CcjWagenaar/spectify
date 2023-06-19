PROG=overwrite overwrite_arch
OBJS=overwrite.o overwrite_arch.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

overwrite.o: overwrite.c
overwrite_arch.o: overwrite_arch.c

clean:
	rm -f ${PROG} ${OBJS}