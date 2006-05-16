# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   Libnet TCP testcode makefile
#   (c) 1998, 1999 route|daemon9 <route@infonexus.com>
#   Generated automatically from Makefile.in by configure.

CC          =   gcc
CFLAGS      =   -O3 -funroll-loops -fomit-frame-pointer -Wall
OBJECTS     =   syn-flood.o         \
                tcp.o               \
                tcp+data.o          \
                tcp+data+ipopt.o

DEFINES     +=  -DHAVE_CONFIG_H
LIBS        =   -L../../lib/ -lnet 

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

all: 	syn-flood tcp tcp+data tcp+data+ipopt

syn-flood: $(OBJECTS)
	$(CC) syn-flood.o $(LIBS) -o ./syn-flood

tcp: $(OBJECTS)
	$(CC) tcp.o $(LIBS) -o ./tcp

tcp+data: $(OBJECTS)
	$(CC) tcp+data.o $(LIBS) -o ./tcp+data

tcp+data+ipopt: $(OBJECTS)
	$(CC) tcp+data+ipopt.o $(LIBS) -o ./tcp+data+ipopt

clean:
	rm -f *.core syn-flood tcp tcp+data tcp+data+ipopt *.o

distclean: clean
	rm -f Makefile

# EOF
