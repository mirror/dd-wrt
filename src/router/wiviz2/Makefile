## Wi-viz makefile
# Supply your own C cross-compiler; I recommend the one from the OpenWRT buildroot
# Also requires a libpcap to link with, use libpcap.a for static, .so for shared
#CC=~/buildroot/staging_dir_mipsel/bin/mipsel-linux-gcc
include $(TOP)/.config

LIBS=../libpcap_noring/libpcap.a -lcrypt -lnvram -lshared -lutils

CCOPTS=$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections
INCLUDE=../libpcap_noring
SOURCES=wiviz.c wl_access.c channelhopper.c
OBJS=wiviz.o wl_access.o channelhopper.o
TARGET=wiviz
ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CCOPTS	+= -I$(SRCBASE)/include.bcm -L../shared -L../nvram -L../libutils
else
CCOPTS	+= -I$(SRCBASE)/include.v24 -L../shared -L../nvram -L../libutils
endif
else
CCOPTS	+= -I$(SRCBASE)/include.v23 -L../shared -L../nvram -L../libutils
endif



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
	cp wiviz $(INSTALLDIR)/usr/sbin

wwwinstall:
	install -d $(INSTALLDIR)/../www/www
	install -d $(INSTALLDIR)/../www/www/style
	install -d $(INSTALLDIR)/../www/www/js
	install -d $(INSTALLDIR)/../www/www/images
	install -d $(INSTALLDIR)/../www/www/images/wiviz
	cp www/wiviz2/wiviz2.css $(INSTALLDIR)/../www/www/style/wiviz2.css
	cp www/wiviz2/wiviz2.js $(INSTALLDIR)/../www/www/js/wiviz2.js
	cp www/wiviz2/*.gif $(INSTALLDIR)/../www/www/images/wiviz
	cp www/wiviz2/*.png $(INSTALLDIR)/../www/www/images/wiviz
#	cp www/wiviz2/*.asp $(INSTALLDIR)/../www/www

clean:
	rm -f *.o *.c~ *.h~ wiviz
