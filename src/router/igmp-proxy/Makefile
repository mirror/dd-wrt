
#BINDIR=../bin
BINDIR=/usr/local/bin
ETCDIR=/etc
MANDIR=/usr/share/man

CFLAGS=$(COPTS) -O2  -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS=  -ffunction-sections -fdata-sections -Wl,--gc-sections


default : build.h igmpproxy

all	: igmpproxy

clean	:
	rm -f *.o *.asm build.h igmpproxy  

install : 
install:
	install -D igmpproxy $(INSTALLDIR)/usr/sbin/igmprt

igmpproxy : igmpproxy.o config.o confread.o request.o udpsock.o mcgroup.o rttable.o \
			igmp.o ifvc.o callout.o kern.o lib.o mroute-api.o

build.h	:  
	echo '#define BUILD "' `date +%y%m%d` '"' >build.h


