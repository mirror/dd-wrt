PROG        = usb_modeswitch
VERS        = 2.6.1
CC          ?= gcc
#CFLAGS      += -Wall
CFLAGS     = $(COPTS) $(LTO) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections    -D_GNU_SOURCE -DNEED_PRINTF -Ilibusb/libusb 
#-Ilibusb-compat/libusb
#LIBS        = libusb.a
#LIBS        = libusb-compat/libusb/.libs/libusb.a 
LIBS        = libusb/libusb/.libs/libusb-1.0.a -lpthread
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX      = $(DESTDIR)/usr
ETCDIR      = $(DESTDIR)/etc
UDEVDIR     = $(DESTDIR)/lib/udev
SBINDIR     = $(PREFIX)/sbin
MANDIR      = $(PREFIX)/share/man/man1
SYSDIR      = $(ETCDIR)/systemd/system
UPSDIR      = $(ETCDIR)/init

.PHONY: clean install install-common uninstall \
	dispatcher-script dispatcher-dynlink dispatcher-statlink \
	install-script install-dynlink install-statlink

all: all-with-script-dispatcher

all-with-script-dispatcher: $(PROG) dispatcher-script

all-with-dynlink-dispatcher: $(PROG) dispatcher-dynlink

all-with-statlink-dispatcher: $(PROG) dispatcher-statlink

$(PROG): $(OBJS) usb_modeswitch.h
	cd libusb && make all
	cd libusb-compat && make all
	$(CC) -o $(PROG) $(OBJS) $(CFLAGS) $(LDLTO) $(LIBS) $(LDFLAGS)
	$(CC) $(CFLAGS) -Ilibusb-compat/libusb -o ozerocdoff ozerocdoff.c  libusb-compat/libusb/.libs/libusb.a $(LIBS) 

dispatcher-script: usb_modeswitch_dispatcher.tcl
	DISPATCH=dispatcher-script
	cp -f usb_modeswitch_dispatcher.tcl usb_modeswitch_dispatcher

dispatcher-dynlink: dispatcher.c dispatcher.h
	DISPATCH=dispatcher-dynlink
	$(CC) dispatcher.c $(LDFLAGS) -Ljim -ljim -Ijim -o usb_modeswitch_dispatcher $(CFLAGS)

dispatcher-statlink: dispatcher.c dispatcher.h
	DISPATCH=dispatcher-statlink
	$(CC) dispatcher.c $(LDFLAGS) -l:libjim.a -ldl -Ijim -o usb_modeswitch_dispatcher $(CFLAGS)

dispatcher.h: usb_modeswitch_dispatcher.tcl
	./make_string.sh usb_modeswitch_dispatcher.tcl > $@

configure:
	cd libusb && autoreconf -ivf
	cd libusb && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-log --disable-udev CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -DNEED_PRINTF -fPIC -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC  -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include"
	make -C libusb
	cd libusb-compat && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -DNEED_PRINTF -fPIC" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CPPFLAGS="$(COPTS) -DNEED_PRINTF -fPIC" LIBUSB_1_0_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a" LIBUSB_1_0_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb"


clean:
	cd libusb && make clean
	cd libusb-compat && make clean
	$(RM) ozerocdoff
	$(RM) usb_modeswitch
	$(RM) usb_modeswitch_dispatcher
	$(RM) dispatcher.h

distclean: clean

# If the systemd folder is present, install the service for starting the dispatcher
# If not, use the dispatcher directly from the udev rule as in previous versions

install-common: $(PROG) $(DISPATCH)
	install -D --mode=755 usb_modeswitch $(SBINDIR)/usb_modeswitch
	install -D --mode=755 usb_modeswitch.sh $(UDEVDIR)/usb_modeswitch
	install -D --mode=644 usb_modeswitch.conf $(ETCDIR)/usb_modeswitch.conf
	install -D --mode=644 usb_modeswitch.1 $(MANDIR)/usb_modeswitch.1
	install -D --mode=644 usb_modeswitch_dispatcher.1 $(MANDIR)/usb_modeswitch_dispatcher.1
	install -D --mode=755 usb_modeswitch_dispatcher $(SBINDIR)/usb_modeswitch_dispatcher
	install -d $(DESTDIR)/var/lib/usb_modeswitch
	test -d $(UPSDIR) -a -e /sbin/initctl && install --mode=644 usb-modeswitch-upstart.conf $(UPSDIR) || test 1
	test -d $(SYSDIR) -a \( -e /usr/bin/systemctl -o -e /bin/systemctl \) && install --mode=644 usb_modeswitch@.service $(SYSDIR) || test 1

install: install-script

install-script: dispatcher-script install-common

install-dynlink: dispatcher-dynlink install-common

install-statlink: dispatcher-statlink install-common

uninstall:
	$(RM) $(SBINDIR)/usb_modeswitch
	$(RM) $(SBINDIR)/usb_modeswitch_dispatcher
	$(RM) $(UDEVDIR)/usb_modeswitch
	$(RM) $(ETCDIR)/usb_modeswitch.conf
	$(RM) $(MANDIR)/usb_modeswitch.1
	$(RM) -R $(DESTDIR)/var/lib/usb_modeswitch
	$(RM) $(SYSDIR)/usb_modeswitch@.service
