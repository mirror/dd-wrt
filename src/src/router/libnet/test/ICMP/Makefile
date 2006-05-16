# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   Libnet ICMP testcode makefile
#   Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
#                            route|daemon9 <route@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   icmp_echo.o         \
		icmp_unreach.o      \
		icmp_timexceed.o    \
		icmp_timestamp.o    \
		silvertongue.o      \
		ping_of_death.o

DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all: 	icmp_echo       \
	icmp_unreach    \
	icmp_timexceed  \
	icmp_timestamp  \
	silvertongue    \
	ping_of_death
 

icmp_echo: $(OBJECTS)
	$(CC) icmp_echo.o $(LIBS) -o ./icmp_echo

icmp_unreach: $(OBJECTS)
	$(CC) icmp_unreach.o $(LIBS) -o ./icmp_unreach

icmp_timexceed: $(OBJECTS)
	$(CC) icmp_timexceed.o $(LIBS) -o ./icmp_timexceed

icmp_timestamp: $(OBJECTS)
	$(CC) icmp_timestamp.o $(LIBS) -o ./icmp_timestamp

silvertongue: $(OBJECTS)
	$(CC) silvertongue.o $(LIBS) -o ./silvertongue

ping_of_death: $(OBJECTS)
	$(CC) ping_of_death.o $(LIBS) -o ./ping_of_death

clean:
	rm -f *.core icmp_echo icmp_unreach icmp_timexceed icmp_timestamp \
	silvertongue ping_of_death *.o

distclean: clean
	rm -f Makefile

# EOF
