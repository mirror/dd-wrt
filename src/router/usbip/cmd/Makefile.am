# $Id: Makefile.am 85 2009-01-13 17:10:51Z hirofuchi $

bin_PROGRAMS = usbip usbipd bind_driver

usbip_SOURCES = vhci_attach.c usbip_network.c usbip_network.h
usbipd_SOURCES = stub_server.c usbip_network.c usbip_network.h
bind_driver_SOURCES = bind-driver.c \
		utils.c utils.h \
		usbip_network.h usbip_network.c

#bind_driver_CFLAGS = -Wall -W -Wstrict-prototypes @PACKAGE_CFLAGS@
#bind_driver_LDADD = @PACKAGE_LIBS@

#AM_CPPFLAGS = -I../include -Wall -std=gnu99
INCLUDES = -I$(top_srcdir)/lib
LDADD = ../lib/libusbip.la @PACKAGE_LIBS@
EXTRA_CFLAGS = @EXTRA_CFLAGS@
AM_CFLAGS = -Wall -W -Wstrict-prototypes -std=gnu99 $(EXTRA_CFLAGS) @PACKAGE_CFLAGS@

