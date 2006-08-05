# Generated automatically from Makefile.in by configure.
#
#   $Id: Makefile,v 1.1 2004/04/27 01:35:00 dyang Exp $
#
#   Libnet makefile
#   (c) 1998, 1999  Mike D. Schiffman <mike@infonexus.com>
#                                
#   Generated automatically from Makefile.in by configure.

#   Enabled:    Dumps debugging info and verbose error information.
#   Disabled:   Doesn't.
#   default:    disabled.
DEFINES     +=  #-D__DEBUG 
prefix      =   /usr
exec_prefix =   ${prefix}
srcdir      =   .
BIN_PREFIX  =   ${exec_prefix}/bin/
INC_PREFIX  =   ${prefix}/include/
LIB_PREFIX  =   ${exec_prefix}/lib/
MAN_PREFIX  =   /usr/man/man3/

LIBNET      =   libnet
LIBPWRITE   =   libpwrite
LIB         =   lib/$(LIBNET).a
MAN         =   doc/$(LIBNET).3
INCLUDE     =   include/$(LIBNET).h
INCLUDE-H   =   include/$(LIBNET)/$(LIBNET)-headers.h
INCLUDE-F   =   include/$(LIBNET)/$(LIBNET)-functions.h
INCLUDE-S   =   include/$(LIBNET)/$(LIBNET)-structures.h
INCLUDE-M   =   include/$(LIBNET)/$(LIBNET)-macros.h
INCLUDE-A   =   include/$(LIBNET)/$(LIBNET)-asn1.h
INCLUDE-O   =   include/$(LIBNET)/$(LIBNET)-ospf.h
CONFIG      =   $(LIBNET)-config
DEFINES     +=  -DHAVE_CONFIG_H
RANLIB      =   ranlib
AR          =   ar
LN          =   ln
CC          =   gcc
ENSUREDIR   =   $(srcdir)/ensure-dir.sh
INSTALL     =   $(srcdir)/install-sh
CFLAGS      =   $(COPTS) -fomit-frame-pointer -Wall
CPPFLAGS    =   
OBJECTS     =   src/libnet_resolve.o\
		src/libnet_socket.o\
		src/libnet_checksum.o\
		src/libnet_prand.o\
		src/libnet_version.o\
		src/libnet_write_ip.o\
		src/libnet_insert_ipo.o\
		src/libnet_insert_tcpo.o\
		src/libnet_error.o\
		src/libnet_link_sockpacket.o\
		src/libnet_packet_mem.o\
		src/libnet_build_ip.o\
		src/libnet_build_tcp.o\
		src/libnet_build_udp.o\
		src/libnet_build_arp.o\
		src/libnet_build_ethernet.o\
		src/libnet_build_icmp.o\
		src/libnet_build_igmp.o\
		src/libnet_build_dns.o\
		src/libnet_build_snmp.o\
		src/libnet_build_rip.o\
		src/libnet_build_ospf.o\
		src/libnet_asn1.o\
		src/libnet_hex_dump.o\
		src/libnet_if_addr.o\
		src/libnet_port_list.o 

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFINES) -c $< -o $@

all: libnet
supp: test example

libnet: version.h $(OBJECTS)
	$(AR) -cr $(LIB) $(OBJECTS)
	$(RANLIB) $(LIB)

version.o: version.h
version.h: ./VERSION
	@rm -f $@
	sed -e 's/.*/#define VERSION "&"/' ./VERSION > $@

test:    libnet
	cd test; make

util:    libnet install
	cd util; make

example: libnet install
	cd example; make

install: libnet
	$(ENSUREDIR) $(DESTDIR)$(prefix) 755
	$(ENSUREDIR) $(DESTDIR)$(LIB_PREFIX) 755
	$(ENSUREDIR) $(DESTDIR)$(INC_PREFIX)$(LIBNET) 755
	$(ENSUREDIR) $(DESTDIR)$(INC_PREFIX) 755
	$(ENSUREDIR) $(DESTDIR)$(MAN_PREFIX) 755
	$(ENSUREDIR) $(DESTDIR)$(BIN_PREFIX) 755
	$(INSTALL) -c -m 0644 $(LIB) $(DESTDIR)$(LIB_PREFIX)
	rm -f $(DESTDIR)$(LIB_PREFIX)$(LIBPWRITE)
	$(LN) -f -s $(DESTDIR)$(LIB_PREFIX)$(LIBNET).a $(DESTDIR)$(LIB_PREFIX)$(LIBPWRITE)
	$(INSTALL) -c -m 0644 $(INCLUDE) $(DESTDIR)$(INC_PREFIX)
	$(INSTALL) -c -m 0644 $(INCLUDE-H) $(DESTDIR)$(INC_PREFIX)$(LIBNET)
	$(INSTALL) -c -m 0644 $(INCLUDE-F) $(DESTDIR)$(INC_PREFIX)$(LIBNET)
	$(INSTALL) -c -m 0644 $(INCLUDE-S) $(DESTDIR)$(INC_PREFIX)$(LIBNET)
	$(INSTALL) -c -m 0644 $(INCLUDE-M) $(DESTDIR)$(INC_PREFIX)$(LIBNET)
	$(INSTALL) -c -m 0644 $(INCLUDE-A) $(DESTDIR)$(INC_PREFIX)$(LIBNET)
	$(INSTALL) -c -m 0644 $(INCLUDE-O) $(DESTDIR)$(INC_PREFIX)$(LIBNET)
	$(INSTALL) -c -m 0644 $(MAN) $(DESTDIR)$(MAN_PREFIX)
	$(INSTALL) -c -m 0755 $(CONFIG) $(DESTDIR)$(BIN_PREFIX)

clean:
	if test -f test/Makefile; then \
	    cd test && make clean; \
	fi
	if test -f example/Makefile; then \
	    cd example && make clean; \
	fi
	if test -f util/Makefile; then \
	    cd util && make clean; \
	fi
	if test -f support/bpf-lkm/etherspoof; then \
	    cd support/bpf-lkm/etherspoof && make clean; \
	fi
	rm -f *.core src/*.o $(LIB)

distclean: clean
	if test -f test/Makefile; then \
	    cd test && make distclean; \
	fi
	if test -f example/Makefile; then \
	    cd example && make distclean; \
	fi
	if test -f util/Makefile; then \
	    cd util && make distclean; \
	fi
	if test -f support/bpf-lkm/etherspoof; then \
	    cd support/bpf-lkm/etherspoof && rm -rf Makefile; \
	fi
	rm -f config.log config.cache config.status Makefile include/config.h \
	version.h support/bpf-lkm/etherspoof conftest* libnet-config

# EOF
