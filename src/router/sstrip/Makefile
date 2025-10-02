#  Makefile for sstrip

CC = gcc
CFLAGS = -Wall -Wextra -Ielfrw

sstrip: sstrip.c elfrw/libelfrw.a

elfrw/libelfrw.a:
	$(MAKE) -C elfrw libelfrw.a

clean:
	rm -f sstrip
