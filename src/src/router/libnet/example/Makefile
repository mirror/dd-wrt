# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:31:22 dyang Exp $
#
#   Libnet example code toplevel Makefile
#   (c) 1998, 1999  Mike D. Schiffman <mike@infonexus.com>
#                                
#   Generated automatically from Makefile.in by configure.

all: example

example:
	if ! test -e ../lib/libnet.a; then \
	    cd .. &&  make; \
	fi
	gcc `libnet-config --defines` libnet-example-1.c \
`libnet-config --libs` -o example-1
	gcc `libnet-config --defines` libnet-example-2.c \
`libnet-config --libs` -o example-2
	gcc `libnet-config --defines` libnet-example-3.c \
`libnet-config --libs` -o example-3
	gcc `libnet-config --defines` libnet-example-4.c \
`libnet-config --libs` -o example-4


clean:
	rm -f *.o example-1 example-2 example-3 example-4

distclean: clean
	rm -f Makefile

# EOF
