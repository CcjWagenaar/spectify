PROG=bcb bcb_arch
OBJS=bcb.o bcb_arch.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

bcb.o: bcb.c
bcb_arch.o: bcb_arch.c

clean: 
	rm -f ${PROG} ${OBJS}