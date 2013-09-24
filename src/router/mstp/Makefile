MODE = devel
version := 0.03

DSOURCES = main.c epoll_loop.c brmon.c bridge_track.c libnetlink.c mstp.c \
           packet.c netif_utils.c ctl_socket_server.c hmac_md5.c driver_deps.c

DOBJECTS = $(DSOURCES:.c=.o)

CTLSOURCES = ctl_main.c ctl_socket_client.c

CTLOBJECTS = $(CTLSOURCES:.c=.o)

CFLAGS += $(COPTS) -D_REENTRANT -D__LINUX__ -DVERSION=$(version) -I. \
          -D_GNU_SOURCE -D__LIBC_HAS_VERSIONSORT__ -DNEED_PRINTF

ifeq ($(MODE),devel)
CFLAGS += -Werror
endif

all: mstpd mstpctl

mstpd: $(DOBJECTS)
	$(CC) -o $@ $(DOBJECTS)

mstpctl: $(CTLOBJECTS)
	$(CC) -o $@ $(CTLOBJECTS)

-include .depend

clean:
	rm -f *.o *~ .depend.bak mstpd mstpctl

install: all
	-mkdir -pv $(INSTALLDIR)/sbin
	install -m 755 mstpd $(INSTALLDIR)/sbin/mstpd
	install -m 755 mstpctl $(INSTALLDIR)/sbin/mstpctl
	install -m 755 bridge-stp $(INSTALLDIR)/sbin/bridge-stp
#	-mkdir -pv $(INSTALLDIR)/lib/mstpctl-utils/
#	cp -rv lib/* $(INSTALLDIR)/lib/mstpctl-utils/

romfs: all
	$(ROMFSINST) /sbin/mstpd
	$(ROMFSINST) /sbin/mstpctl
	$(ROMFSINST) /sbin/bridge-stp

#depend:
#	makedepend -I. -Y *.c -f .depend
