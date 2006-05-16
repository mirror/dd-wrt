# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   Libnet OSPF Makefile.in
#   Copyright (c) 1999 Mike D. Schiffman <mike@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   ospf_hello.o     \
		ospf_lsa.o       \
		ospf_request.o   \

DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all: 	ospf_hello       \
	ospf_lsa         \
	ospf_request
 

ospf_hello: $(OBJECTS)
	$(CC) ospf_hello.o $(LIBS) -o ./ospf_hello

ospf_lsa: $(OBJECTS)
	$(CC) ospf_lsa.o $(LIBS) -o ./ospf_lsa

ospf_request: $(OBJECTS)
	$(CC) ospf_request.o $(LIBS) -o ./ospf_request

clean:
	rm -f *.core ospf_hello ospf_lsa ospf_request *.o

distclean: clean
	rm -f Makefile

# EOF
