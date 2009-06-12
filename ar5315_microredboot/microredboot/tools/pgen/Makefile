OBJS   = $(CFILES:.c=.o)
#INC    = -I../include/
#CFLAGS = -ggdb 

all:	pgen pget phone ap upsd staut apts

pgen:	pgen.c mpx.o
	cc -o pgen pgen.c mpx.o

upsd:	pgen.c mpx.o
	cc -o upsd pgen.c mpx.o


pget:	pgen.c mpx.o
	cc -o pget pgen.c mpx.o

phone: pgen.c mpx.o
	cc -o phone pgen.c mpx.o

ap:	pgen.c mpx.o
	cc -o ap pgen.c mpx.o

apts:	pgen.c mpx.o
	cc -o apts pgen.c mpx.o

staut:	pgen.c mpx.o
	cc -o staut pgen.c mpx.o

mpx.o:  mpx.c
	cc -c mpx.c

clean:	
	if (test -e pgen) then rm pgen; fi
	if (test -e pget) then rm pget; fi
	if (test -e upsd) then rm upsd; fi
	if (test -e phone) then rm phone; fi
	if (test -e ap) then rm ap; fi
	if (test -e apts) then rm apts; fi
	if (test -e staut) then rm staut; fi
	if (test -e *.o) then rm *.o; fi

tarball:
	tar czf pgen.tgz Makefile pgen.c mpx.c pgen.1 pgen.txt README

zipfile:
	zip pgen.zip Makefile pgen.c pgen.1 README
