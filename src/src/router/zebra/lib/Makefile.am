## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"

noinst_LIBRARIES = libzebra.a

libzebra_a_SOURCES = \
	version.c network.c pid_output.c getopt.c getopt1.c daemon.c \
	print_version.c checksum.c vector.c linklist.c vty.c command.c \
	sockunion.c prefix.c thread.c if.c memory.c buffer.c table.c hash.c \
	filter.c routemap.c distribute.c stream.c str.c log.c plist.c \
	zclient.c sockopt.c smux.c md5.c if_rmap.c keychain.c

libzebra_a_DEPENDENCIES = @LIB_REGEX@

libzebra_a_LIBADD = @LIB_REGEX@

noinst_HEADERS = \
	buffer.h command.h filter.h getopt.h hash.h if.h linklist.h log.h \
	memory.h network.h prefix.h routemap.h distribute.h sockunion.h \
	str.h stream.h table.h thread.h vector.h version.h vty.h zebra.h \
	plist.h zclient.h sockopt.h smux.h md5-gnu.h if_rmap.h keychain.h

EXTRA_DIST = regex.c regex-gnu.h

version.c: Makefile
	echo '' >version.c
	echo 'char *host_name = "$(host_alias)";' >>version.c

depend:
	@$(CPP) -MM $(INCLUDES) $(LDFLAGS) *.c

## File dependency.
buffer.o: buffer.c memory.h buffer.h
checksum.o: checksum.c
command.o: command.c command.h vector.h vty.h memory.h log.h version.h
daemon.o: daemon.c
distribute.o: distribute.c hash.h if.h linklist.h filter.h command.h \
 vector.h vty.h distribute.h memory.h
filter.o: filter.c prefix.h filter.h if.h linklist.h memory.h \
 command.h vector.h vty.h sockunion.h buffer.h
flap.o: flap.c
getopt.o: getopt.c
getopt1.o: getopt1.c getopt.h
hash.o: hash.c hash.h memory.h log.h
if.o: if.c linklist.h vector.h vty.h command.h if.h sockunion.h \
 prefix.h ../zebra/connected.h memory.h table.h buffer.h str.h log.h
if_rmap.o: if_rmap.c hash.h command.h vector.h vty.h memory.h if.h \
 linklist.h if_rmap.h
ifa.o: ifa.c
keychain.o: keychain.c command.h vector.h vty.h memory.h linklist.h \
 keychain.h
linklist.o: linklist.c linklist.h memory.h
log.o: log.c log.h memory.h command.h vector.h vty.h
md5.o: md5.c md5-gnu.h
memory.o: memory.c log.h memory.h vector.h vty.h command.h
network.o: network.c
pid_output.o: pid_output.c
plist.o: plist.c prefix.h command.h vector.h vty.h memory.h plist.h \
 sockunion.h buffer.h
prefix.o: prefix.c prefix.h vty.h sockunion.h memory.h log.h
print_version.o: print_version.c version.h
regex.o: regex.c
routemap.o: routemap.c linklist.h memory.h vector.h prefix.h \
 routemap.h command.h vty.h
smux.o: smux.c
sockopt.o: sockopt.c log.h
sockunion.o: sockunion.c prefix.h vty.h sockunion.h memory.h str.h \
 log.h
str.o: str.c str.h
stream.o: stream.c stream.h memory.h network.h prefix.h
table.o: table.c prefix.h table.h memory.h sockunion.h
tcpfilter.o: tcpfilter.c command.h vector.h vty.h prefix.h
thread.o: thread.c thread.h memory.h log.h
vector.o: vector.c vector.h memory.h
version.o: version.c
vty.o: vty.c linklist.h buffer.h version.h command.h vector.h vty.h \
 sockunion.h thread.h memory.h str.h log.h prefix.h filter.h if.h
zclient.o: zclient.c prefix.h stream.h network.h if.h linklist.h log.h \
 thread.h zclient.h memory.h table.h ../zebra/rib.h ../zebra/zserv.h
