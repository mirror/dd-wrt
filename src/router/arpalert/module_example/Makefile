# Copyright (c) 2005-2010 Thierry FOURNIER
# $Id: Makefile 690 2008-03-31 18:36:43Z  $

CC = gcc
CFLAGS = -Wall -g

all: example.so

clean:
	-@rm -f *.o *.so

example.so: example.o
	${CC} ${CFLAGS} -fPIC -shared -o example.so example.o
