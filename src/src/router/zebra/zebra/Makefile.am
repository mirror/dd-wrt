## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\" -DMULTIPATH_NUM=@MULTIPATH_NUM@
INSTALL_SDATA=@INSTALL@ -m 600

LIB_IPV6 = @LIB_IPV6@

ipforward = @IPFORWARD@
if_method = @IF_METHOD@
if_proc = @IF_PROC@
rt_method = @RT_METHOD@
rtread_method = @RTREAD_METHOD@
kernel_method = @KERNEL_METHOD@
other_method = @OTHER_METHOD@

otherobj = $(ipforward) $(if_method) $(if_proc) $(rt_method) \
	$(rtread_method) $(kernel_method) $(other_method)

#sbin_PROGRAMS = zebra client
sbin_PROGRAMS = zebra

zebra_SOURCES = \
	zserv.c main.c interface.c connected.c ioctl.c rib.c redistribute.c \
	debug.c rtadv.c zebra_snmp.c

noinst_HEADERS = \
	connected.h ioctl.h rib.h rt.h zserv.h redistribute.h debug.h rtadv.h \
	interface.h ipforward.h irdp.h

zebra_LDADD = ../lib/libzebra.a $(otherobj)

#client_SOURCES = \
#	client_main.c

sysconf_DATA = zebra.conf.sample

EXTRA_DIST = $(sysconf_DATA) if_ioctl.c if_netlink.c if_proc.c if_sysctl.c \
	ipforward_aix.c ipforward_ews.c ipforward_proc.c \
	ipforward_solaris.c ipforward_sysctl.c rt_ioctl.c rt_netlink.c \
	rt_socket.c rtread_netlink.c rtread_proc.c rtread_sysctl.c \
	rtread_getmsg.c kernel_socket.c kernel_netlink.c mtu_kvm.c \
	GNOME-SMI GNOME-PRODUCT-ZEBRA-MIB irdp.c

zebra : $(zebra_OBJECTS) $(otherobj) ../lib/libzebra.a
	$(CC) -g -o zebra $(zebra_OBJECTS) $(otherobj) ../lib/libzebra.a $(LIBS) $(LIB_IPV6)

client : client_main.o ../lib/libzebra.a
	$(CC) -g -o client client_main.o ../lib/libzebra.a $(LIBS) $(LIB_IPV6)

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
client_main.o: client_main.c ../lib/prefix.h ../lib/stream.h \
 ../lib/zclient.h ../lib/if.h ../lib/linklist.h ../lib/thread.h \
 ../lib/table.h ../zebra/rib.h ../zebra/zserv.h
connected.o: connected.c ../lib/prefix.h ../lib/linklist.h ../lib/if.h \
 ../lib/table.h rib.h ../lib/log.h ../zebra/zserv.h \
 ../zebra/redistribute.h
debug.o: debug.c ../lib/command.h ../lib/vector.h ../lib/vty.h debug.h
if_ioctl.o: if_ioctl.c ../lib/if.h ../lib/linklist.h \
 ../lib/sockunion.h ../lib/prefix.h ioctl.h connected.h \
 ../lib/memory.h ../lib/log.h ../zebra/interface.h
if_netlink.o: if_netlink.c
if_proc.o: if_proc.c ../lib/if.h ../lib/linklist.h ../lib/prefix.h \
 ../lib/log.h ../zebra/ioctl.h ../zebra/connected.h \
 ../zebra/interface.h
if_sysctl.o: if_sysctl.c ../lib/if.h ../lib/linklist.h \
 ../lib/sockunion.h ../lib/prefix.h connected.h ../lib/memory.h \
 ioctl.h ../lib/log.h
interface.o: interface.c ../lib/if.h ../lib/linklist.h ../lib/vty.h \
 ../lib/sockunion.h ../lib/prefix.h ../lib/command.h ../lib/vector.h \
 ../lib/memory.h ioctl.h connected.h ../lib/log.h ../lib/zclient.h \
 ../zebra/interface.h ../zebra/rtadv.h ../zebra/rib.h ../zebra/zserv.h \
 ../zebra/redistribute.h ../lib/table.h ../zebra/debug.h
ioctl.o: ioctl.c ../lib/linklist.h ../lib/if.h ../lib/prefix.h ioctl.h \
 ../lib/log.h ../zebra/rib.h ../zebra/rt.h
ipforward_aix.o: ipforward_aix.c
ipforward_ews.o: ipforward_ews.c
ipforward_proc.o: ipforward_proc.c
ipforward_solaris.o: ipforward_solaris.c ../lib/memory.h
ipforward_sysctl.o: ipforward_sysctl.c ../lib/log.h
irdp.o: irdp.c ../lib/if.h ../lib/linklist.h ../lib/stream.h \
 ../lib/memory.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/log.h ../lib/sockunion.h ../zebra/irdp.h
kernel_netlink.o: kernel_netlink.c
kernel_socket.o: kernel_socket.c ../lib/if.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/sockunion.h connected.h ../lib/memory.h \
 ioctl.h ../lib/log.h ../lib/str.h ../lib/table.h rib.h \
 ../zebra/interface.h ../zebra/zserv.h ../zebra/debug.h \
 ../lib/thread.h
main.o: main.c ../lib/version.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/thread.h ../lib/filter.h ../lib/if.h \
 ../lib/linklist.h ../lib/memory.h ../lib/prefix.h ../lib/log.h \
 ../zebra/rib.h ../zebra/zserv.h ../zebra/debug.h
mtu_kvm.o: mtu_kvm.c ../lib/if.h ../lib/linklist.h
redistribute.o: redistribute.c ../lib/vector.h ../lib/vty.h \
 ../lib/command.h ../lib/prefix.h ../lib/table.h ../lib/stream.h \
 ../lib/zclient.h ../lib/if.h ../lib/linklist.h ../lib/log.h \
 ../zebra/rib.h ../zebra/zserv.h ../zebra/redistribute.h \
 ../zebra/debug.h
rib.o: rib.c ../lib/prefix.h ../lib/table.h ../lib/memory.h \
 ../lib/vty.h ../lib/str.h ../lib/command.h ../lib/vector.h \
 ../lib/linklist.h ../lib/if.h ../lib/log.h ../lib/sockunion.h \
 ../zebra/rib.h ../zebra/rt.h ../zebra/zserv.h ../zebra/redistribute.h \
 ../zebra/debug.h
rt_ioctl.o: rt_ioctl.c ../lib/prefix.h ../lib/log.h ../lib/if.h \
 ../lib/linklist.h ../zebra/rib.h ../zebra/debug.h
rt_netlink.o: rt_netlink.c ../lib/linklist.h ../lib/if.h ../lib/log.h \
 ../lib/prefix.h connected.h ../lib/table.h rib.h ../zebra/zserv.h \
 ../zebra/redistribute.h ../zebra/interface.h ../zebra/debug.h \
 ../lib/thread.h
rt_socket.o: rt_socket.c ../lib/if.h ../lib/linklist.h ../lib/prefix.h \
 ../lib/sockunion.h ../lib/log.h ../lib/str.h ../zebra/debug.h \
 ../zebra/rib.h
rtadv.o: rtadv.c ../lib/memory.h ../lib/sockopt.h ../lib/thread.h \
 ../lib/if.h ../lib/linklist.h ../lib/log.h ../lib/prefix.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../zebra/interface.h \
 ../zebra/rtadv.h ../zebra/debug.h
rtread_getmsg.o: rtread_getmsg.c ../lib/prefix.h ../lib/log.h \
 ../lib/if.h ../lib/linklist.h ../zebra/rib.h
rtread_netlink.o: rtread_netlink.c
rtread_proc.o: rtread_proc.c ../lib/prefix.h ../lib/log.h ../lib/if.h \
 ../lib/linklist.h rib.h
rtread_sysctl.o: rtread_sysctl.c ../lib/memory.h ../lib/log.h
zebra_snmp.o: zebra_snmp.c
zserv.o: zserv.c ../lib/prefix.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/if.h ../lib/linklist.h ../lib/thread.h \
 ../lib/stream.h ../lib/memory.h ../lib/table.h rib.h ../lib/network.h \
 ../lib/sockunion.h ../lib/log.h ../lib/zclient.h ../zebra/zserv.h \
 ../zebra/redistribute.h ../zebra/debug.h ../zebra/ipforward.h
