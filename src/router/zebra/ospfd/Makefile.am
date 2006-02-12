## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

noinst_LIBRARIES = libospf.a
sbin_PROGRAMS = ospfd

libospf_a_SOURCES = \
	ospfd.c ospf_zebra.c ospf_interface.c ospf_ism.c ospf_neighbor.c \
	ospf_nsm.c ospf_dump.c ospf_network.c ospf_packet.c ospf_lsa.c \
	ospf_spf.c ospf_route.c ospf_ase.c ospf_abr.c ospf_ia.c ospf_flood.c \
	ospf_lsdb.c ospf_asbr.c ospf_routemap.c ospf_snmp.c

noinst_HEADERS = \
	ospf_dump.h ospf_interface.h ospf_ism.h ospf_neighbor.h \
	ospf_network.h ospf_nsm.h ospf_packet.h ospf_zebra.h ospfd.h \
	ospf_lsa.h ospf_spf.h ospf_route.h ospf_ase.h ospf_abr.h ospf_ia.h \
	ospf_flood.h ospf_lsdb.h ospf_asbr.h ospf_snmp.h

ospfd_SOURCES = \
	ospf_main.c $(libospf_a_SOURCES)

ospfd_LDADD = ../lib/libzebra.a

sysconf_DATA = ospfd.conf.sample

EXTRA_DIST = $(sysconf_DATA) OSPF-MIB.txt OSPF-TRAP-MIB.txt

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
ospf_abr.o: ospf_abr.c ../lib/thread.h ../lib/memory.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/vty.h ../lib/filter.h ../lib/log.h ../ospfd/ospfd.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ia.h ../ospfd/ospf_flood.h ../ospfd/ospf_abr.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_asbr.o: ospf_asbr.c ../lib/thread.h ../lib/memory.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/vty.h ../lib/filter.h ../lib/log.h ../ospfd/ospfd.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_spf.h \
 ../ospfd/ospf_flood.h ../ospfd/ospf_route.h ../ospfd/ospf_zebra.h \
 ../ospfd/ospf_dump.h
ospf_ase.o: ospf_ase.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/vty.h ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_dump.o: ospf_dump.c ../lib/linklist.h ../lib/thread.h \
 ../lib/prefix.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/stream.h ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../lib/if.h ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h \
 ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h \
 ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h ../ospfd/ospf_dump.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_network.h
ospf_flood.o: ospf_flood.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/if.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/table.h ../lib/thread.h ../lib/memory.h ../lib/log.h \
 ../lib/zclient.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_flood.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_abr.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_ia.o: ospf_ia.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/table.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../lib/if.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_abr.h ../ospfd/ospf_ia.h \
 ../ospfd/ospf_dump.h
ospf_interface.o: ospf_interface.c ../lib/thread.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/if.h ../lib/table.h ../lib/memory.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/stream.h \
 ../lib/log.h ../ospfd/ospf_spf.h ../ospfd/ospf_network.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_packet.h ../ospfd/ospf_abr.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_dump.h
ospf_ism.o: ospf_ism.c ../lib/thread.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/if.h ../lib/table.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h \
 ../ospfd/ospf_network.h ../ospfd/ospf_dump.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_flood.h ../ospfd/ospf_abr.h
ospf_lsa.o: ospf_lsa.c ../lib/linklist.h ../lib/prefix.h ../lib/if.h \
 ../lib/table.h ../lib/memory.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/stream.h ../lib/log.h ../lib/thread.h \
 ../lib/hash.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_flood.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_spf.h ../ospfd/ospf_dump.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_zebra.h
ospf_lsdb.o: ospf_lsdb.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/stream.h \
 ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_flood.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_spf.h ../ospfd/ospf_dump.h ../ospfd/ospf_route.h
ospf_main.o: ospf_main.c ../lib/version.h ../lib/thread.h \
 ../lib/prefix.h ../lib/linklist.h ../lib/if.h ../lib/vector.h \
 ../lib/vty.h ../lib/command.h ../lib/filter.h ../lib/stream.h \
 ../lib/log.h ../lib/memory.h ../ospfd/ospfd.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_dump.h \
 ../ospfd/ospf_zebra.h
ospf_neighbor.o: ospf_neighbor.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/memory.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/thread.h ../lib/stream.h ../lib/table.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../lib/if.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_network.h ../ospfd/ospf_flood.h
ospf_network.o: ospf_network.c ../lib/thread.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/if.h ../lib/sockunion.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_network.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_packet.h
ospf_nsm.o: ospf_nsm.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/stream.h ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_network.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_dump.h ../ospfd/ospf_flood.h ../ospfd/ospf_abr.h
ospf_packet.o: ospf_packet.c ../lib/thread.h ../lib/memory.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/sockunion.h ../lib/stream.h ../lib/log.h ../lib/md5-gnu.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_network.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_packet.h ../ospfd/ospf_spf.h \
 ../ospfd/ospf_flood.h ../ospfd/ospf_dump.h
ospf_route.o: ospf_route.c ../lib/prefix.h ../lib/table.h \
 ../lib/memory.h ../lib/linklist.h ../lib/log.h ../lib/if.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/sockunion.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_spf.h ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_routemap.o: ospf_routemap.c ../lib/memory.h ../lib/prefix.h \
 ../lib/routemap.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h ../lib/if.h \
 ../lib/linklist.h ../ospfd/ospf_asbr.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_route.h ../ospfd/ospf_zebra.h
ospf_snmp.o: ospf_snmp.c
ospf_spf.o: ospf_spf.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/log.h ../lib/sockunion.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ia.h ../ospfd/ospf_ase.h ../ospfd/ospf_abr.h \
 ../ospfd/ospf_dump.h
ospf_zebra.o: ospf_zebra.c ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/network.h ../lib/prefix.h \
 ../lib/routemap.h ../lib/table.h ../lib/stream.h ../lib/memory.h \
 ../lib/zclient.h ../lib/if.h ../lib/linklist.h ../lib/filter.h \
 ../lib/log.h ../ospfd/ospfd.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h ../ospfd/ospf_abr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_dump.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_zebra.h
ospfd.o: ospfd.c ../lib/thread.h ../lib/vty.h ../lib/command.h \
 ../lib/vector.h ../lib/linklist.h ../lib/prefix.h ../lib/table.h \
 ../lib/if.h ../lib/memory.h ../lib/stream.h ../lib/log.h \
 ../lib/sockunion.h ../lib/zclient.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_network.h ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h \
 ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h \
 ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_dump.h ../ospfd/ospf_zebra.h \
 ../ospfd/ospf_abr.h ../ospfd/ospf_flood.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h
