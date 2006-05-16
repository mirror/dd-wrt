#
# lib/Makefile	Makefile for the net-lib function collection
#
# NET-LIB	A collection of functions used from the base set of the
#		NET-2 Networking Distribution for the LINUX operating
#		system.  (net-tools, net-drivers)
#
# Author:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
#		Copyright 1993 MicroWalt Corporation
#
#		This program is free software; you can redistribute it
#		and/or  modify it under  the terms of  the GNU General
#		Public  License as  published  by  the  Free  Software
#		Foundation;  either  version 2 of the License, or  (at
#		your option) any later version.
#


HWOBJS	 = hw.o loopback.o slip.o ether.o ax25.o ppp.o arcnet.o tr.o tunnel.o frame.o sit.o rose.o ash.o fddi.o hippi.o hdlclapb.o strip.o irda.o ec_hw.o x25.o
AFOBJS	 = unix.o inet.o inet6.o ax25.o ipx.o ddp.o ipx.o netrom.o af.o rose.o econet.o x25.o
AFGROBJS = inet_gr.o inet6_gr.o ipx_gr.o ddp_gr.o netrom_gr.o ax25_gr.o rose_gr.o getroute.o x25_gr.o
AFSROBJS = inet_sr.o inet6_sr.o netrom_sr.o ipx_sr.o setroute.o x25_sr.o
ACTOBJS  = slip_ac.o ppp_ac.o activate.o
VARIA	 = getargs.o masq_info.o proc.o util.o nstrcmp.o interface.o sockets.o

# Default Name
NET_LIB_NAME = net-tools

ifeq ($(HAVE_IP_TOOLS),1)
VARIA	+= util-ank.o
endif

OBJS	= $(sort $(VARIA) $(AFOBJS) $(HWOBJS) \
		$(AFGROBJS) $(AFSROBJS) $(ACTOBJS))


# This can be overwritten by the TOPLEVEL Makefile
TOPDIR=..
CFLAGS += -I$(TOPDIR) -idirafter $(TOPDIR)/include # -fPIC
SONAME=libnet-tools.so.0

.SUFFIXES: .a .so

all:	lib$(NET_LIB_NAME).a # lib$(NET_LIB_NAME).so

lib$(NET_LIB_NAME).a:	Makefile $(TOPDIR)/config.h $(OBJS)
			@echo Building $@
			@rm -f $@
			@$(AR) rcs $@ $(OBJS)

.a.so:;
	$(CC) -o $@ -shared -Wl,--whole-archive -Wl,--soname -Wl,$(SONAME) -nostdlib -nostartfiles $<

clean:
		rm -f *.o *~ *.orig lib$(NET_LIB_NAME).a lib$(NET_LIB_NAME).so

clobber:	clean

# End of lib/Makefile.
