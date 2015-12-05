## Wi-viz makefile
# Supply your own C cross-compiler; I recommend the one from the OpenWRT buildroot
# Also requires a libpcap to link with, use libpcap.a for static, .so for shared
#CC=~/buildroot/staging_dir_mipsel/bin/mipsel-linux-gcc
LIBS=../libpcap_noring/libpcap.a

CCOPTS=$(COPTS) $(MIPS16_OPT)
INCLUDE=../libpcap_noring
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
	install -d $(INSTALLDIR)/../www/www
	install -d $(INSTALLDIR)/../www/www/style
	install -d $(INSTALLDIR)/../www/www/js
	install -d $(INSTALLDIR)/../www/www/images
	install -d $(INSTALLDIR)/../www/www/images/wiviz
	cp wiviz $(INSTALLDIR)/usr/sbin
	cp www/wiviz/wiviz.css $(INSTALLDIR)/../www/www/style/wiviz.css
	cp www/wiviz/wiviz.js $(INSTALLDIR)/../www/www/js/wiviz.js
	cp www/wiviz/*.gif $(INSTALLDIR)/../www/www/images/wiviz
	cp www/wiviz/*.png $(INSTALLDIR)/../www/www/images/wiviz
	cp www/wiviz/*.asp $(INSTALLDIR)/../www/www

clean:
	rm -f *.o *.c~ *.h~ wiviz
