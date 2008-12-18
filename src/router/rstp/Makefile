
DSOURCES =  brstate.c libnetlink.c epoll_loop.c bridge_track.c rstp.c \
	   packet.c ctl_socket.c netif_utils.c main.c brmon.c

DOBJECTS = $(DSOURCES:.c=.o)

CTLSOURCES = ctl_main.c ctl_cli_wrap.c ctl_socket_client.c

CTLOBJECTS = $(CTLSOURCES:.c=.o)

#CC=gcc
CFLAGS = -Wall -Werror $(COPTS) -D_REENTRANT -D__LINUX__ -DVERSION=$(version) -DBUILD=$(build) -I. -I./include -DNEED_PRINTF

all: rstpd rstpctl

rstpd: $(DOBJECTS)
	$(CC) -o $@ $(DOBJECTS)

rstpctl: $(CTLOBJECTS)
	$(CC) -o $@ $(CTLOBJECTS)

clean:
	rm -f *.o rstpd rstpctl
	rm -fr $(TOPDIR) $(BUILDDIR)

install: all
	install -m 755 -d $(INSTALLDIR)/sbin
	install -m 755 rstpd $(INSTALLDIR)/sbin
	install -m 755 rstpctl $(INSTALLDIR)/sbin
#	install -m 755 -d $(INSTALLPREFIX)/usr/share/man/man8
#	install -m 644 rstpd.8 $(INSTALLPREFIX)/usr/share/man/man8
#	install -m 644 rstpctl.8 $(INSTALLPREFIX)/usr/share/man/man8

# RPM Building, as non root
version := 0.21
build := 1

BUILDROOT := $(CURDIR)/rpm_buildroot
TOPDIR    := $(CURDIR)/rpm_topdir

RPMBUILD=rpmbuild
RPMDEFS=\
        --buildroot=$(BUILDROOT) \
        --define='_topdir $(TOPDIR)' \
        --define='VERSION $(version)' \
        --define='BUILD $(build)'

rpm:
	mkdir -p $(BUILDROOT) $(TOPDIR)/BUILD $(TOPDIR)/SOURCES $(TOPDIR)/RPMS
	(cd .. ; tar cfz $(TOPDIR)/SOURCES/rstp-$(version).tgz --exclude rstp-$(version)/rpm_buildroot --exclude rstp-$(version)/rpm_topdir rstp-$(version))
	$(RPMBUILD) $(RPMDEFS) -bb rstp.spec
	cp $(TOPDIR)/RPMS/*/rstp-$(version)-$(build).*.rpm .
	cp $(TOPDIR)/RPMS/*/rstp-debuginfo-$(version)-$(build).*.rpm .

