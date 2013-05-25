PROG        = usb_modeswitch
VERS        = 1.2.4
#STRIP	    = strip
#CC          = gcc
CCFLAGS     = $(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -Ilibusb-compat/libusb
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
	cd libusb && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC"
	make -C libusb
	cd libusb-compat && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fPIC" CPPFLAGS="$(COPTS) -DNEED_PRINTF -fPIC" LIBUSB_1_0_LIBS="$(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a" LIBUSB_1_0_CFLAGS="-I$(TOP)/usb_modeswitch/libusb/libusb"

clean:
	    	cd libusb && make clean
	    	cd libusb-compat && make clean
	      $(RM) ./usb_modeswitch 
	      $(RM) ./ozerocdoff

install: all
	mkdir -p $(SBINDIR)
	install ./usb_modeswitch $(SBINDIR)
	mkdir -p $(ETCDIR)
	install ./usb_modeswitch.conf $(ETCDIR)

uninstall: clean
	$(RM) /usr/sbin/usb_modeswitch
	$(RM) /etc/usb_modeswitch.conf

.PHONY:     clean install uninstall
