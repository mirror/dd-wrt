#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# $Id: Makefile.linux,v 1.9 2012/05/04 21:48:00 dgolle Exp $
#***********************************************************************

DESTDIR = $(INSTROOT)@DESTDIR@
LIBDIR = $(DESTDIR)/lib/pppd/$(PPPDVERSION)

PPPDVERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../../patchlevel.h)

INSTALL	= install

#COPTS=-O2 -g
CFLAGS=$(LTO) -Wall -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include  -I. -I../../../ $(COPTS)  $(MIPS16_OPT) -fPIC -DRC_LOG_FACILITY=LOG_DAEMON
#CFLAGS  = $(COPTS) -I. -I../.. -I../../../include -fPIC -DPPPD_VERSION=\"$(PPPDVERSION)\"
all: pptp.so

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

pptp.so: dirutil.o orckit_quirks.o pptp.o pptp_callmgr.o pptp_ctrl.o pptp_quirks.o util.o vector.o
	$(CC)  $(COPTS) $(MIPS16_OPT) $(LDLTO) -fPIC -o pptp.so -shared dirutil.o orckit_quirks.o pptp.o pptp_callmgr.o pptp_ctrl.o pptp_quirks.o util.o vector.o

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -c -m 4550 pptp.so $(LIBDIR)

clean:
	rm -f *.o *.so

