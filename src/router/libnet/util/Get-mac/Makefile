# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:32:51 dyang Exp $
#
#   Libnet get_mac makefile
#   
#   (c) 1998, 1999  Mike D. Schiffman <mike@infonexus.com>
#                   route|daemon9 <route@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   get_mac.o
DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all: 	get_mac

get_mac: $(OBJECTS)
	$(CC) get_mac.o $(LIBS) -lpcap -o ./get_mac

clean:
	rm -f *.core get_mac *.o

distclean: clean
	rm -f Makefile

# EOF
