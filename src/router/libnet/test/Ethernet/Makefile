# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   Libnet ethernet testcode makefile
#   
#   (c) 1998, 1999  Mike D. Schiffman <mike@infonexus.com>
#                   route|daemon9 <route@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   arp.o tcp.o icmp_mask.o get_address.o poink.o
DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all: 	arp tcp icmp_mask get_address poink

arp: $(OBJECTS)
	$(CC) arp.o $(LIBS) -o ./arp

tcp: $(OBJECTS)
	$(CC) tcp.o $(LIBS) -o ./tcp

icmp_mask: $(OBJECTS)
	$(CC) icmp_mask.o $(LIBS) -o ./icmp_mask
    
get_address: $(OBJECTS)
	$(CC) get_address.o $(LIBS) -o ./get_address

poink: $(OBJECTS)
	$(CC) poink.o $(LIBS) -o ./poink

clean:
	rm -f *.core arp tcp icmp_mask get_address poink *.o

distclean: clean
	rm -f Makefile

# EOF
