PROG        = usb_modeswitch
VERS        = 1.2.7
CC          ?= gcc
#CFLAGS      += -Wall
CFLAGS     = $(COPTS)  $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections    -D_GNU_SOURCE -DNEED_PRINTF -Ilibusb-compat/libusb
#LIBS        = libusb.a
LIBS        = libusb-compat/libusb/.libs/libusb.a libusb/libusb/.libs/libusb-1.0.a -lpthread
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX      = $(DESTDIR)/usr
ETCDIR      = $(DESTDIR)/etc
UDEVDIR     = $(DESTDIR)/lib/udev
SBINDIR     = $(PREFIX)/sbin
MANDIR      = $(PREFIX)/share/man/man1
VPATH       = jimtcl
HOST_TCL   := $(shell cd jim && ./autosetup/find-tclsh)
ifeq (,$(findstring jimsh0,$(HOST_TCL)))
TCL        ?= $(HOST_TCL)
else
TCL        ?= /usr/bin/tclsh
endif
JIM_CONFIGURE_OPTS =--disable-lineedit \
	--with-out-jim-ext="stdlib posix load signal syslog" --prefix=/usr

.PHONY: clean install install-common uninstall \
	script shared static \
	dispatcher-script dispatcher-shared dispatcher-static \
	install-script install-shared install-static

all: script

script: $(PROG) dispatcher-script

shared: $(PROG) dispatcher-shared

static: $(PROG) dispatcher-static

$(PROG): $(OBJS) usb_modeswitch.h
	cd libusb && make all
	cd libusb-compat && make all
	$(CC) -o $(PROG) $(OBJS) $(CFLAGS) $(LIBS) $(LDFLAGS)
	$(CC) $(CFLAGS) -o ozerocdoff ozerocdoff.c $(LIBS)

jim/libjim.so:
	cd jim && CFLAGS="$(CFLAGS)" CC="$(CC)" ./configure $(JIM_CONFIGURE_OPTS) --shared
	$(MAKE) -C jim lib

jim/libjim.a:
	cd jim && CFLAGS="$(CFLAGS)" CC="$(CC)" ./configure $(JIM_CONFIGURE_OPTS)
	$(MAKE) -C jim lib

dispatcher-script: usb_modeswitch.tcl
	sed 's_!/usr/bin/tclsh_!'"$(TCL)"'_' < usb_modeswitch.tcl > usb_modeswitch_dispatcher

dispatcher-shared: jim/libjim.so dispatcher.c usb_modeswitch.string
	$(CC) dispatcher.c $(LDFLAGS) -Ljim -ljim -Ijim -o usb_modeswitch_dispatcher $(CFLAGS)

dispatcher-static: jim/libjim.a dispatcher.c usb_modeswitch.string
	$(CC) dispatcher.c $(LDFLAGS) jim/libjim.a -Ijim -o usb_modeswitch_dispatcher $(CFLAGS)

usb_modeswitch.string: usb_modeswitch.tcl
	$(HOST_TCL) make_string.tcl usb_modeswitch.tcl > $@

configure:
	cd libusb && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -DNEED_PRINTF -fPIC" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC"
	make -C libusb
	cd libusb-compat && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -DNEED_PRINTF -fPIC" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CPPFLAGS="$(COPTS) -DNEED_PRINTF -fPIC" LIBUSB_1_0_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a" LIBUSB_1_0_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb"


clean:
	cd libusb && make clean
	cd libusb-compat && make clean
	$(RM) ozerocdoff
	$(RM) usb_modeswitch
	$(RM) usb_modeswitch_dispatcher
	$(RM) usb_modeswitch.string
	$(RM) jim/autosetup/jimsh0
	$(RM) jim/autosetup/jimsh0.c

distclean: clean
	-$(MAKE) -C jim distclean

ums-clean:
	$(RM) usb_modeswitch
	$(RM) usb_modeswitch_dispatcher
	$(RM) usb_modeswitch.string

install-common: $(PROG) usb_modeswitch_dispatcher
	install -D --mode=755 usb_modeswitch $(SBINDIR)/usb_modeswitch
	install -D --mode=755 usb_modeswitch.sh $(UDEVDIR)/usb_modeswitch
	install -D --mode=644 usb_modeswitch.conf $(ETCDIR)/usb_modeswitch.conf
	install -D --mode=644 usb_modeswitch.1 $(MANDIR)/usb_modeswitch.1
	install -D --mode=755 usb_modeswitch_dispatcher $(SBINDIR)/usb_modeswitch_dispatcher
	install -d $(DESTDIR)/var/lib/usb_modeswitch

install: install-script

install-script: dispatcher-script install-common

install-shared: dispatcher-shared install-common

install-static: dispatcher-static install-common

uninstall:
	$(RM) $(SBINDIR)/usb_modeswitch
	$(RM) $(SBINDIR)/usb_modeswitch_dispatcher
	$(RM) $(UDEVDIR)/usb_modeswitch
	$(RM) $(ETCDIR)/usb_modeswitch.conf
	$(RM) $(MANDIR)/usb_modeswitch.1
	$(RM) -R $(DESTDIR)/var/lib/usb_modeswitch
