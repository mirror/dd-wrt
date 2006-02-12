## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

noinst_LIBRARIES = libbgp.a
sbin_PROGRAMS = bgpd
#noinst_PROGRAMS = bgp_btoa

libbgp_a_SOURCES = \
	bgpd.c bgp_fsm.c bgp_aspath.c bgp_community.c bgp_attr.c \
	bgp_debug.c bgp_route.c bgp_zebra.c bgp_open.c bgp_routemap.c \
	bgp_packet.c bgp_network.c bgp_filter.c bgp_regex.c bgp_clist.c \
	bgp_dump.c bgp_snmp.c bgp_ecommunity.c bgp_mplsvpn.c bgp_nexthop.c \
	bgp_damp.c

noinst_HEADERS = \
	bgp_aspath.h bgp_attr.h bgp_community.h bgp_debug.h bgp_fsm.h \
	bgp_network.h bgp_open.h bgp_packet.h bgp_regex.h bgp_route.h \
	bgpd.h bgp_filter.h bgp_clist.h bgp_dump.h bgp_zebra.h \
	bgp_ecommunity.h bgp_mplsvpn.h bgp_nexthop.h bgp_damp.h

bgpd_SOURCES = \
	bgp_main.c $(libbgp_a_SOURCES)

bgpd_LDADD = ../lib/libzebra.a

#bgp_btoa_SOURCES = \
#	bgp_btoa.c

#bgp_btoa_LDADD = libbgp.a ../lib/libzebra.a

sysconf_DATA = bgpd.conf.sample bgpd.conf.sample2

EXTRA_DIST = $(sysconf_DATA) BGP4-MIB.txt

install-sysconfDATA: $(sysconf_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	@list='$(sysconf_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_SDATA) $(srcdir)/$$p $(DESTDIR)$(sysconfdir)/$$p"; \
	    $(INSTALL_SDATA) $(srcdir)/$$p $(DESTDIR)$(sysconfdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_SDATA) $$p $(DESTDIR)$(sysconfdir)/$$p"; \
	    $(INSTALL_SDATA) $$p $(DESTDIR)$(sysconfdir)/$$p; \
	  fi; fi; \
	done

depend:
	@$(CPP) -MM $(INCLUDES) $(LDFLAGS) *.c

## File dependency.
bgp_aspath.o: bgp_aspath.c ../lib/hash.h ../lib/memory.h \
 ../lib/vector.h ../lib/vty.h ../lib/str.h ../lib/log.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_aspath.h
bgp_attr.o: bgp_attr.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/memory.h ../lib/vector.h ../lib/vty.h ../lib/stream.h \
 ../lib/log.h ../lib/hash.h ../bgpd/bgpd.h ../lib/sockunion.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_route.h ../bgpd/bgp_aspath.h \
 ../bgpd/bgp_community.h ../bgpd/bgp_debug.h ../bgpd/bgp_packet.h \
 ../bgpd/bgp_ecommunity.h
bgp_btoa.o: bgp_btoa.c ../lib/stream.h ../lib/log.h ../lib/prefix.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_dump.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_aspath.h
bgp_clist.o: bgp_clist.c ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/memory.h ../lib/log.h ../lib/buffer.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_community.h ../bgpd/bgp_clist.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_regex.h ../lib/regex-gnu.h
bgp_community.o: bgp_community.c ../lib/hash.h ../lib/memory.h \
 ../lib/vector.h ../lib/vty.h ../lib/str.h ../bgpd/bgp_community.h
bgp_damp.o: bgp_damp.c ../lib/prefix.h ../lib/table.h \
 ../lib/linklist.h ../lib/memory.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/stream.h ../lib/filter.h ../lib/if.h ../lib/str.h \
 ../lib/log.h ../lib/routemap.h ../lib/buffer.h ../lib/sockunion.h \
 ../lib/plist.h ../lib/thread.h ../bgpd/bgp_damp.h ../bgpd/bgpd.h \
 ../bgpd/bgp_route.h
bgp_debug.o: bgp_debug.c ../lib/version.h ../lib/prefix.h \
 ../lib/linklist.h ../lib/stream.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/str.h ../lib/log.h ../lib/sockunion.h \
 ../bgpd/bgpd.h ../bgpd/bgp_aspath.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_community.h
bgp_dump.o: bgp_dump.c ../lib/log.h ../lib/stream.h ../lib/sockunion.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/prefix.h \
 ../lib/thread.h ../lib/table.h ../bgpd/bgpd.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_dump.h
bgp_ecommunity.o: bgp_ecommunity.c ../lib/hash.h ../lib/memory.h \
 ../lib/prefix.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../bgpd/bgpd.h ../lib/sockunion.h ../bgpd/bgp_ecommunity.h
bgp_filter.o: bgp_filter.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/log.h ../lib/memory.h ../lib/buffer.h \
 ../bgpd/bgpd.h ../lib/sockunion.h ../bgpd/bgp_aspath.h \
 ../bgpd/bgp_regex.h ../lib/regex-gnu.h ../bgpd/bgp_filter.h
bgp_fsm.o: bgp_fsm.c ../lib/linklist.h ../lib/prefix.h ../lib/vty.h \
 ../lib/sockunion.h ../lib/thread.h ../lib/log.h ../lib/stream.h \
 ../lib/memory.h ../bgpd/bgpd.h ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h \
 ../bgpd/bgp_fsm.h ../bgpd/bgp_packet.h ../bgpd/bgp_network.h \
 ../bgpd/bgp_route.h ../bgpd/bgp_dump.h
bgp_main.o: bgp_main.c ../lib/vector.h ../lib/vty.h ../lib/command.h \
 ../lib/thread.h ../lib/version.h ../lib/memory.h ../lib/prefix.h \
 ../lib/log.h ../bgpd/bgpd.h ../lib/sockunion.h ../bgpd/bgp_network.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_mplsvpn.h
bgp_mplsvpn.o: bgp_mplsvpn.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/prefix.h ../lib/table.h ../lib/log.h \
 ../lib/memory.h ../lib/stream.h ../bgpd/bgpd.h ../lib/sockunion.h \
 ../bgpd/bgp_route.h ../bgpd/bgp_attr.h ../bgpd/bgp_mplsvpn.h
bgp_network.o: bgp_network.c ../lib/thread.h ../lib/sockunion.h \
 ../lib/memory.h ../lib/log.h ../lib/if.h ../lib/linklist.h \
 ../lib/prefix.h ../bgpd/bgpd.h ../bgpd/bgp_fsm.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_debug.h
bgp_nexthop.o: bgp_nexthop.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/thread.h ../lib/prefix.h ../lib/table.h \
 ../lib/zclient.h ../lib/if.h ../lib/linklist.h ../lib/stream.h \
 ../lib/network.h ../lib/log.h ../lib/memory.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_route.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_nexthop.h ../zebra/rib.h ../zebra/zserv.h
bgp_open.o: bgp_open.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/stream.h ../lib/thread.h ../lib/log.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../bgpd/bgpd.h ../lib/sockunion.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_fsm.h \
 ../bgpd/bgp_packet.h ../bgpd/bgp_open.h
bgp_packet.o: bgp_packet.c ../lib/thread.h ../lib/stream.h \
 ../lib/network.h ../lib/prefix.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/log.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/linklist.h ../bgpd/bgpd.h ../bgpd/bgp_dump.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_fsm.h \
 ../bgpd/bgp_route.h ../bgpd/bgp_packet.h ../bgpd/bgp_open.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_community.h ../bgpd/bgp_network.h \
 ../bgpd/bgp_mplsvpn.h
bgp_regex.o: bgp_regex.c ../lib/log.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h bgpd.h ../lib/sockunion.h bgp_aspath.h \
 bgp_regex.h ../lib/regex-gnu.h
bgp_route.o: bgp_route.c ../lib/prefix.h ../lib/table.h \
 ../lib/linklist.h ../lib/memory.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/stream.h ../lib/filter.h ../lib/if.h ../lib/str.h \
 ../lib/log.h ../lib/routemap.h ../lib/buffer.h ../lib/sockunion.h \
 ../lib/plist.h ../lib/thread.h ../bgpd/bgpd.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_aspath.h \
 ../bgpd/bgp_clist.h ../bgpd/bgp_community.h ../bgpd/bgp_ecommunity.h \
 ../bgpd/bgp_packet.h ../bgpd/bgp_regex.h ../lib/regex-gnu.h \
 ../bgpd/bgp_filter.h ../bgpd/bgp_fsm.h ../bgpd/bgp_mplsvpn.h \
 ../bgpd/bgp_nexthop.h ../bgpd/bgp_damp.h
bgp_routemap.o: bgp_routemap.c ../lib/memory.h ../lib/prefix.h \
 ../lib/filter.h ../lib/if.h ../lib/linklist.h ../lib/routemap.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/log.h \
 ../lib/plist.h ../lib/regex-gnu.h ../lib/buffer.h ../lib/table.h \
 ../lib/sockunion.h ../bgpd/bgpd.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_route.h ../bgpd/bgp_regex.h \
 ../bgpd/bgp_community.h ../bgpd/bgp_clist.h ../bgpd/bgp_filter.h \
 ../bgpd/bgp_mplsvpn.h ../bgpd/bgp_ecommunity.h
bgp_snmp.o: bgp_snmp.c
bgp_zebra.o: bgp_zebra.c ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/stream.h ../lib/network.h ../lib/prefix.h ../lib/log.h \
 ../lib/sockunion.h ../lib/zclient.h ../lib/if.h ../lib/linklist.h \
 ../lib/routemap.h ../bgpd/bgpd.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_nexthop.h
bgpd.o: bgpd.c ../lib/prefix.h ../lib/thread.h ../lib/buffer.h \
 ../lib/stream.h ../lib/table.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/sockunion.h ../lib/network.h ../lib/memory.h \
 ../lib/filter.h ../lib/if.h ../lib/linklist.h ../lib/routemap.h \
 ../lib/str.h ../lib/log.h ../lib/plist.h ../bgpd/bgpd.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_route.h ../bgpd/bgp_dump.h \
 ../bgpd/bgp_debug.h ../bgpd/bgp_attr.h ../bgpd/bgp_community.h \
 ../bgpd/bgp_clist.h ../bgpd/bgp_fsm.h ../bgpd/bgp_packet.h \
 ../bgpd/bgp_zebra.h ../bgpd/bgp_open.h ../bgpd/bgp_filter.h \
 ../bgpd/bgp_nexthop.h ../bgpd/bgp_damp.h
