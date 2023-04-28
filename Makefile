PROG=spv1 spv2 spv1_char
OBJS=spv1.o spv2.o spv1_char.o gen_array.o time_and_flush.o
CFLAGS=-g -O0
LOADLIBES=-lm

all: ${PROG}

#spv: spv1.0 spv2.o

gen_array.o: gen_array.c
time_and_flush.o: time_and_flush.c
spv2.o: spv2.c
spv1.o: spv1.c
spv1_char.o: spv1_char.c

clean: 
	rm -f ${PROG} ${OBJS}