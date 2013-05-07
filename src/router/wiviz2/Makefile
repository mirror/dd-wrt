## Wi-viz makefile
# Supply your own C cross-compiler; I recommend the one from the OpenWRT buildroot
# Also requires a libpcap to link with, use libpcap.a for static, .so for shared
#CC=~/buildroot/staging_dir_mipsel/bin/mipsel-linux-gcc
include $(TOP)/.config

LIBS=-L../libpcap_noring -lpcap -lcrypt -lnvram -lutils -lwireless
ifeq ($(CONFIG_ATH9K),y)
LIBS += -L$(TOP)/libnl-tiny -lnl-tiny
endif

CCOPTS=$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections
INCLUDE=../libpcap_noring
SOURCES=wiviz.c wl_access.c channelhopper.c
OBJS=wiviz.o wl_access.o channelhopper.o
TARGET=wiviz
ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_BUFFALO),y)
CCOPTS	+= -I$(SRCBASE)/include.bcm -L../shared -L../nvram -L../libutils -I../shared
else
CCOPTS	+= -I$(SRCBASE)/include.v24 -L../shared -L../nvram -L../libutils -I../shared
endif
else
CCOPTS	+= -I$(SRCBASE)/include.v23 -L../shared -L../nvram -L../libutils -I../shared
endif
ifeq ($(CONFIG_MADWIFI),y)
CCOPTS+=-DHAVE_MADWIFI   -include $(TOP)/madwifi.dev/madwifi.dev/include/compat.h -DHEADERS_KERNEL 
endif
ifeq ($(CONFIG_MADWIFI_MIMO),y)
CCOPTS+=-DHAVE_MADWIFI_MIMO -DHAVE_MADWIFI   -include $(TOP)/madwifi.dev/madwifi.dev/include/compat.h -DHEADERS_KERNEL
endif
ifeq ($(CONFIG_RT2880),y)
CCOPTS+=-DHAVE_RT2880 
endif
ifeq ($(CONFIG_ATH9K),y)
CCOPTS+=-DHAVE_ATH9K
CCOPTS+=-DHAVE_MADWIFI
endif
ifeq ($(CONFIG_RT61),y)
CCOPTS+=-DHAVE_RT2880 
endif
ifeq ($(CONFIG_TIEXTRA1),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_TIEXTRA2),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_SAMBA3),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_FTP),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif
ifeq ($(CONFIG_MINIDLNA),y)
LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
ifeq ($(ARCHITECTURE),openrisc)
LDFLAGS +=-L$(TOP)/jansson/src/.libs -ljansson
endif
endif

wiviz: clean ${OBJS}
	${CC} ${CCOPTS} -I${INCLUDE} -o ${TARGET} ${OBJS} ${LIBS} ${LDFLAGS}

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
