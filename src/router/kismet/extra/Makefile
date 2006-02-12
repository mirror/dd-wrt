HOME	= .
CXX	= mipsel-uclibc-gcc
CC	= mipsel-uclibc-gcc
LD	= mipsel-uclibc-gcc
LDFLAGS	= 
LIBS	=  -lstdc++
CFLAGS	=  -Ilibpcap-0.7.2 
CXXFLAGS = -Wall -g -O2 -Os
CPPFLAGS =  -Ilibpcap-0.7.2 -I../
SUID	= 

prefix = /tmp/kismet
exec_prefix = ${prefix}
ETC	= ${prefix}/etc
BIN	= ${exec_prefix}/bin
SHARE = ${prefix}/share/kismet/
WAV = ${SHARE}/wav/

CWGDO = kismet2cwgd.o ../expat.o
CWGD = kismet2cwgd

XMLO = kismet2xml.o
XML = kismet2xml

SANITO = ../gpsmap_samples.o ../expat.o gpsxml-sanitize.o
SANIT = gpsxml-sanitize

all:	$(XML) 

$(CWGD):	$(CWGDO)
	$(LD) $(LDFLAGS) -o $(CWGD) $(CWGDO) $(LIBS) -lexpat

$(XML):		$(XMLO)
	$(LD) $(LDFLAGS) -o $(XML) $(XMLO) $(LIBS)

$(SANIT):		$(SANITO)
	$(LD) $(LDFLAGS) -o $(SANIT) $(SANITO) $(LIBS) -lexpat -lz

clean:
	@-rm -f *.o
	@-rm -f $(CWGD)
	@-rm -f $(XML)

distclean:
	@-make clean
	@-rm -f *~
	@-rm -f Makefile

.c.o:	$(DEPEND)
	$(CC) $(CFLAGS) -c $*.c -o $@ 

.cc.o:	$(DEPEND)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $*.cc -o $@ 

.SUFFIXES: .c .cc .o
