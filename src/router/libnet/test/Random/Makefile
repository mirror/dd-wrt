# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   Libnet simple testcode makefile
#   Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
#                            route|daemon9 <route@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   prand_trials.o
DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all:    prand_trials

prand_trials: $(OBJECTS)
	$(CC) prand_trials.o $(LIBS) -o ./prand_trials

clean:
	rm -f *.core prand_trials *.o

distclean: clean
	rm -f Makefile

# EOF
