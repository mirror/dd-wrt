# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   Libnet simple testcode makefile
#   Copyright (c) 1998, 1999 route|daemon9 <route@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   udp.o       \
                udp+data.o

DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all:    udp udp+data

udp: $(OBJECTS)
	$(CC) udp.o $(LIBS) -o ./udp

udp+data: $(OBJECTS)
	$(CC) udp+data.o $(LIBS) -o ./udp+data

clean:
	rm -f *.core udp udp+data *.o

distclean: clean
	rm -f Makefile

# EOF
