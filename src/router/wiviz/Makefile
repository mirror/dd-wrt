## Wi-viz makefile
# Supply your own C cross-compiler; I recommend the one from the OpenWRT buildroot
# Also requires a libpcap to link with, use libpcap.a for static, .so for shared
#CC=~/buildroot/staging_dir_mipsel/bin/mipsel-linux-gcc
LIBS=../libpcap/libpcap.a

CCOPTS=$(COPTS) 
INCLUDE=../libpcap
SOURCES=wiviz.c wl_access.c channelhopper.c
OBJS=wiviz.o wl_access.o channelhopper.o
TARGET=wiviz

wiviz: ${OBJS}
	${CC} ${CCOPTS} -I${INCLUDE} -o ${TARGET} ${OBJS} ${LIBS}

wiviz.o: wiviz.c
	${CC} ${CCOPTS} -I${INCLUDE} -c ${SOURCES}
wl_access.o: wl_access.c
	${CC} ${CCOPTS} -I${INCLUDE} -c ${SOURCES}
channelhopper.o: channelhopper.c
	${CC} ${CCOPTS} -I${INCLUDE} -c ${SOURCES}

remake:
	touch wiviz.c wl_access.c channelhopper.c

install:
	install -d $(INSTALLDIR)/usr/sbin
	cp wiviz $(INSTALDIR)/usr/sbin

clean:
	rm -f *.o *.c~ *.h~ wiviz
