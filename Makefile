PROG=spv1
OBJS=spv1.o gen_array.o time_and_flush.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

#spv: spv1.0 spv2.o

gen_array.o: gen_array.c
time_and_flush.o: time_and_flush.c
spv1.o: spv1.c

clean: 
	rm -f ${PROG} ${OBJS}