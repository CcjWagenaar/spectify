PROG=use_after_free use_after_free_arch test
OBJS=use_after_free.o use_after_free_arch.o test.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

use_after_free.o: use_after_free.c
use_after_free_arch.o: use_after_free_arch.c
test.o: test.c

clean:
	rm -f ${PROG} ${OBJS}