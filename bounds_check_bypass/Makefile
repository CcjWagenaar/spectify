PROG=bcb bcb_arch
OBJS=bcb.o bcb_arch.o gen_array.o time_and_flush.o print_results.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

#gen_array.o: ../lib/gen_array.c
#time_and_flush.o: ../lib/time_and_flush.c
#print_results.o: ../lib/print_results.c
bcb.o: bcb.c
bcb_arch.o: bcb_arch.c

clean: 
	rm -f ${PROG} ${OBJS}