PROG=spv1
OBJS=spv1.o
CFLAGS=-g
LOADLIBES=-lm

all: ${PROG}

spv: spv1.o

clean: 
	rm -f ${PROG} ${OBJS}