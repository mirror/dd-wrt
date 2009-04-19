SRC +=	$(wildcard ./src/*.c)

OBJS = $(SRC:.c=.o)

CC = gcc
CFLAGS = -c -g -Wall -Werror -I./src
LFLAGS = -g -Wall 
BUILDLIBS = -L/usr/lib -lpcap -Wl

.c.o:
	${CC} ${CFLAGS} -o $@ $^

netsimpcap:	${OBJS}
	${CC} -o ./bin/$@ ${LFLAGS} ${BUILDLIBS} ${OBJS}

clean:
	rm -f ${OBJS} ./bin/netsimpcap

all: netsimpcap
