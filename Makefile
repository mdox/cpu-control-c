CC=gcc
LIBS=-lm
CFLAGS=-std=c11
SRC=*.c
OBJS=*.o
NAME=cpu-control-c

compile: ${SRC}
	${CC} -c ${SRC} ${LIBS} ${CFLAGS}

install: compile
	${CC} ${OBJS} ${LIBS} ${CFLAGS} -o ${NAME}

clean:
	rm -rf ${OBJS} ${NAME}
