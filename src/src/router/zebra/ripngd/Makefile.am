## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

noinst_LIBRARIES = libripng.a
sbin_PROGRAMS = ripngd

libripng_a_SOURCES = \
	ripng_interface.c ripngd.c ripng_zebra.c ripng_route.c ripng_debug.c \
	ripng_routemap.c

noinst_HEADERS = \
	ripng_debug.h ripng_route.h ripngd.h

ripngd_SOURCES = \
	ripng_main.c $(libripng_a_SOURCES)

ripngd_LDADD = ../lib/libzebra.a

sysconf_DATA = ripngd.conf.sample

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
ripng_debug.o: ripng_debug.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../ripngd/ripng_debug.h
ripng_interface.o: ripng_interface.c ../lib/linklist.h ../lib/if.h \
 ../lib/prefix.h ../lib/memory.h ../lib/network.h ../lib/filter.h \
 ../lib/log.h ../lib/stream.h ../lib/zclient.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/table.h ../lib/thread.h \
 ../ripngd/ripngd.h ../ripngd/ripng_debug.h
ripng_main.o: ripng_main.c ../lib/version.h ../lib/vector.h \
 ../lib/vty.h ../lib/command.h ../lib/thread.h ../lib/log.h \
 ../lib/prefix.h ../lib/if.h ../lib/linklist.h ../ripngd/ripngd.h
ripng_route.o: ripng_route.c ../lib/prefix.h ../lib/table.h \
 ../lib/memory.h ../lib/if.h ../lib/linklist.h ../ripngd/ripngd.h \
 ../ripngd/ripng_route.h
ripng_routemap.o: ripng_routemap.c ../lib/if.h ../lib/linklist.h \
 ../lib/memory.h ../lib/prefix.h ../lib/routemap.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../ripngd/ripngd.h
ripng_zebra.o: ripng_zebra.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/prefix.h ../lib/stream.h ../lib/zclient.h \
 ../lib/if.h ../lib/linklist.h ../lib/log.h ../ripngd/ripngd.h
ripngd.o: ripngd.c ../lib/prefix.h ../lib/filter.h ../lib/if.h \
 ../lib/linklist.h ../lib/log.h ../lib/thread.h ../lib/memory.h \
 ../lib/stream.h ../lib/table.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/sockopt.h ../lib/distribute.h ../lib/plist.h \
 ../lib/routemap.h ../lib/if_rmap.h ../ripngd/ripngd.h \
 ../ripngd/ripng_route.h ../ripngd/ripng_debug.h
