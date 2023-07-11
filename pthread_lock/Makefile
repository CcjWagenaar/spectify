PROG=lock lock2 lock_arch test
OBJS=lock.o lock2.o lock_arch.o test.o
CFLAGS=-g -O0
LOADLIBES=-lm -lpthread

all: ${PROG}

lock.o: lock.c
lock2.o: lock2.c
lock_arch.o: lock_arch.c
test.o: test.c

clean:
	rm -f ${PROG} ${OBJS}