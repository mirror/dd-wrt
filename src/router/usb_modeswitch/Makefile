PROG        = usb_modeswitch
VERS        = 0.9.7
#STRIP	    = strip
#CC          = gcc
CCFLAGS     = $(COPTS) -DNEED_PRINTF -Ilibusb-compat/libusb
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX	    = $(INSTALLDIR)/usr
ETCDIR	    = $(INSTALLDIR)/etc
SBINDIR	    = $(PREFIX)/sbin

.PHONY:     clean
all:        $(PROG)
$(PROG):    $(OBJS)
	      cd libusb && make all
	      cd libusb-compat && make all
	      $(CC) $(CCFLAGS) -o $(PROG) $(OBJS) libusb-compat/libusb/.libs/libusb.a libusb/libusb/.libs/libusb-1.0.a -lpthread
	      $(CC) $(CCFLAGS) -o ozerocdoff ozerocdoff.c libusb-compat/libusb/.libs/libusb.a libusb/libusb/.libs/libusb-1.0.a -lpthread


configure:
	cd libusb && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF" CXXFLAGS="$(COPTS) -DNEED_PRINTF" CPPFLAGS="$(COPTS) -DNEED_PRINTF"
	make -C libusb
	cd libusb-compat && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF" CXXFLAGS="$(COPTS) -DNEED_PRINTF" CPPFLAGS="$(COPTS) -DNEED_PRINTF" LIBUSB_1_0_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a" LIBUSB_1_0_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb"

clean:
	    	cd libusb && make clean
	      $(RM) ./usb_modeswitch 

install: all
	mkdir -p $(SBINDIR)
	install ./usb_modeswitch $(SBINDIR)
	mkdir -p $(ETCDIR)
	install ./usb_modeswitch.conf $(ETCDIR)

uninstall: clean
	$(RM) /usr/sbin/usb_modeswitch
	$(RM) /etc/usb_modeswitch.conf

.PHONY:     clean install uninstall
