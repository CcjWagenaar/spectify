PROG=uninit_read uninit_read2 uninit_read3 test
OBJS=uninit_read.o uninit_read2.o uninit_read3.o test.o gen_array.o time_and_flush.o print_results.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

gen_array.o: ../lib/gen_array.c
time_and_flush.o: ../lib/time_and_flush.c
print_results.o: ../lib/print_results.c
uninit_read.o: uninit_read.c gen_array.o time_and_flush.o print_results.o
uninit_read2.o: uninit_read2.o gen_array.o time_and_flush.o print_results.o
uninit_read3.o: uninit_read3.o gen_array.o time_and_flush.o print_results.o
test.o: test.c

clean:
	rm -f ${PROG} ${OBJS}