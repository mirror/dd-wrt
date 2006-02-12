## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

noinst_LIBRARIES = libospf6.a
sbin_PROGRAMS = ospf6d

libospf6_a_SOURCES = \
	ospf6_dump.c ospf6d.c ospf6_interface.c ospf6_network.c \
	ospf6_neighbor.c ospf6_message.c ospf6_lsa.c ospf6_spf.c \
	ospf6_route.c ospf6_zebra.c ospf6_ism.c ospf6_dbex.c \
	ospf6_lsdb.c ospf6_prefix.c ospf6_top.c ospf6_area.c ospf6_nsm.c \
	ospf6_redistribute.c ospf6_routemap.c ospf6_list.c ospf6_proto.c

noinst_HEADERS = \
	ospf6_area.h ospf6_dump.h ospf6_interface.h ospf6_lsa.h \
	ospf6_message.h ospf6_neighbor.h ospf6_network.h ospf6_proto.h \
	ospf6_spf.h ospf6_route.h ospf6_types.h ospf6_zebra.h ospf6d.h \
	ospf6_ism.h ospf6_dbex.h ospf6_lsdb.h ospf6_prefix.h \
	ospf6_top.h ospf6_nsm.h ospf6_redistribute.h ospf6_routemap.h \
	ospf6_list.h

ospf6d_SOURCES = \
	ospf6_main.c $(libospf6_a_SOURCES)

ospf6d_LDADD = ../lib/libzebra.a

sysconf_DATA = ospf6d.conf.sample

EXTRA_DIST = $(sysconf_DATA)

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
ospf6_area.o: ospf6_area.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_dbex.o: ospf6_dbex.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_dump.o: ospf6_dump.c ospf6_dump.h ../lib/command.h ../lib/log.h
ospf6_interface.o: ospf6_interface.c ospf6d.h ../lib/version.h \
 ../lib/log.h ../lib/linklist.h ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_ism.o: ospf6_ism.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_list.o: ospf6_list.c ../lib/memory.h ../lib/log.h ospf6_list.h
ospf6_lsa.o: ospf6_lsa.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_lsdb.o: ospf6_lsdb.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_main.o: ospf6_main.c ../lib/thread.h ../lib/log.h \
 ../lib/version.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/memory.h ospf6d.h ../lib/linklist.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_message.o: ospf6_message.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_neighbor.o: ospf6_neighbor.c ospf6d.h ../lib/version.h \
 ../lib/log.h ../lib/linklist.h ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h ospf6_list.h
ospf6_network.o: ospf6_network.c ../lib/memory.h ../lib/log.h \
 ../lib/sockunion.h ospf6d.h ../lib/version.h ../lib/linklist.h \
 ../lib/thread.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_nsm.o: ospf6_nsm.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_prefix.o: ospf6_prefix.c ../lib/log.h ../lib/prefix.h \
 ../lib/memory.h ../lib/linklist.h 
ospf6_proto.o: ospf6_proto.c ospf6_proto.h
ospf6_redistribute.o: ospf6_redistribute.c ospf6d.h ../lib/version.h \
 ../lib/log.h ../lib/linklist.h ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_route.o: ospf6_route.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_routemap.o: ospf6_routemap.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/routemap.h \
 ../lib/plist.h ../lib/memory.h ../lib/log.h ospf6_redistribute.h \
 ospf6_top.h
ospf6_spf.o: ospf6_spf.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_top.o: ospf6_top.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_zebra.o: ospf6_zebra.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6d.o: ospf6d.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
