HOME	= .
CXX	= mipsel-uclibc-gcc
CC	= mipsel-uclibc-gcc
LD	= mipsel-uclibc-gcc
LDFLAGS	= 
LIBS	=  -lstdc++
CFLAGS	=  -Ilibpcap-0.7.2
CXXFLAGS = -Wall -g -O2 -Os
CPPFLAGS =  -Ilibpcap-0.7.2
SUID	= 

prefix = /tmp/kismet
exec_prefix = ${prefix}
ETC	= ${prefix}/etc
BIN	= ${exec_prefix}/bin
SHARE = ${prefix}/share/kismet/
WAV = ${SHARE}/wav/

BUZZERO = buzzme.o
BUZZER = buzzme

all:	$(BUZZER) 

$(BUZZER):	$(BUZZERO)
	$(LD) $(LDFLAGS) -o $(BUZZER) $(BUZZERO) $(LIBS)

clean:
	@-rm -f *.o
	@-rm -f $(BUZZER)

distclean:
	@-make clean
	@-rm -f *~
	@-rm -f Makefile

.c.o:	$(DEPEND)
	$(CC) $(CFLAGS) -c $*.c -o $@ 

.cc.o:	$(DEPEND)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $*.cc -o $@ 

.SUFFIXES: .c .cc .o
