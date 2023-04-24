PROG=spv2
OBJS=spv2.o gen_array.o time_mem_load.o
CFLAGS=-g
LOADLIBES=-lm

all: ${PROG}

spv: spv2.o gen_array.o time_and_flush.o

gen_array.o: gen_array.c
time_and_flush.o: time_and_flush.c
spv2.o: spv2.c

clean: 
	rm -f ${PROG} ${OBJS}