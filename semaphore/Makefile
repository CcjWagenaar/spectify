PROG=semaphore semaphore_arch test
OBJS=semaphore.o semaphore_arch.o test.o
CFLAGS=-g -O0
LOADLIBES=-lm -lpthread

all: ${PROG}

semaphore.o: semaphore.c
semaphore_arch.o: semaphore_arch.c
test.o: test.c

clean:
	rm -f ${PROG} ${OBJS}