# $Id: Makefile,v 1.1 2005/09/28 11:53:38 seg Exp $

CFLAGS = -Wall -g $(COPTS) 
ifeq ($(strip $(DOSTATIC)),true)
    CFLAGS += --static
endif


all:		bidi tap

clean:
		rcsclean *
		rm -f *~
		rm -f bidi
		rm -f tap

bidi:		bidi.c
		$(CC) $(CFLAGS) -o bidi bidi.c

tap:		tap.c
		$(CC) $(CFLAGS) -o tap tap.c
