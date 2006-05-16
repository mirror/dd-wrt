CC=	gcc
#CC=     arm-linux-gcc

CFLAGS= -Wall `pkg-config --cflags gtk+-2.0` #-g #to use GDB
LFLAGS= `pkg-config --libs gtk+-2.0`

SRCS=	src/main.c src/interface.c src/callbacks.c src/ipc.c \
	src/packet.c src/nodes.c src/routes.c

OBJS=	src/main.o src/interface.o src/callbacks.o src/ipc.o \
	src/packet.o src/nodes.o src/routes.o

HDRS=	src/main.h src/interface.h src/callbacks.h src/ipc.h \
	src/olsr.h src/types.h src/packet.h src/nodes.h \
	src/pixmaps.h src/routes.h

all:	olsrd-gui

olsrd-gui:	$(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS)

install:
	cp ./olsrd-gui /usr/bin/
