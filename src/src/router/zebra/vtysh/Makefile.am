## Process this file with Automake to create Makefile.in

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"

LIBS = @LIBS@ @CURSES@ @LIBPAM@

bin_PROGRAMS = vtysh

vtysh_SOURCES = vtysh_main.c vtysh.c vtysh_cmd.c vtysh_user.c vtysh_config.c
noinst_HEADERS = vtysh.h vtysh_user.h
vtysh_LDADD = ../lib/libzebra.a

sysconf_DATA = vtysh.conf.sample

EXTRA_DIST = extract.pl vtysh.conf.sample

rebuild4:
	./extract.pl ../zebra/*.c ../ripd/*.c ../ospfd/*.c ../bgpd/*.c ../lib/keychain.c ../lib/routemap.c ../lib/filter.c ../lib/plist.c >vtysh_cmd.c

rebuild:
	./extract.pl ../zebra/*.c ../ripd/*.c ../ripngd/*.c ../ospfd/*.c ../ospf6d/*.c ../bgpd/*.c ../lib/keychain.c ../lib/routemap.c ../lib/filter.c ../lib/plist.c >vtysh_cmd.c

depend:
	@$(CPP) -MM $(INCLUDES) $(LDFLAGS) *.c

## File dependency.
vtysh.o: vtysh.c ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../vtysh/vtysh.h
vtysh_cmd.o: vtysh_cmd.c ../lib/command.h ../lib/vector.h ../lib/vty.h \
 vtysh.h
vtysh_config.o: vtysh_config.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/linklist.h ../lib/memory.h ../vtysh/vtysh.h
vtysh_main.o: vtysh_main.c ../lib/version.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../vtysh/vtysh.h ../vtysh/vtysh_user.h
vtysh_user.o: vtysh_user.c ../lib/memory.h ../lib/linklist.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h
