PROG=uninit_read uninit_read_arch
OBJS=uninit_read.o uninit_read_arch
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

uninit_read.o: uninit_read.c
uninit_read_arch.o: uninit_read_arch.c

clean:
	rm -f ${PROG} ${OBJS}