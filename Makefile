PROG=spv1
OBJS=spv1.o gen_array.o time_mem_load.o
CFLAGS=-g
LOADLIBES=-lm

all: ${PROG}

spv: spv1.o gen_array.o time_and_flush.o

gen_array.o: gen_array.c
time_and_flush.o: time_and_flush.c
spv1.o: spv1.c

clean: 
	rm -f ${PROG} ${OBJS}