PROG=lock
OBJS=lock.o gen_array.o time_and_flush.o print_results.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

gen_array.o: ../lib/gen_array.c
time_and_flush.o: ../lib/time_and_flush.c
print_results.o: ../lib/print_results.c
lock.o: lock.c gen_array.o time_and_flush.o print_results.o

clean:
	rm -f ${PROG} ${OBJS}