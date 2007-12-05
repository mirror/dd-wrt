# $Id: Makefile,v 1.1 2004/02/04 04:01:51 kanki Exp $
# Makefile for LNS handlers
# LIC: GPL

prefix=/usr
exec_prefix=${prefix}
mandir=${prefix}/man
docdir=/usr/doc/rp-pppoe-$(VERSION)
install=/usr/bin/install -c
install_dir=/usr/bin/install -c -d
sbindir=${exec_prefix}/sbin

HANDLERS=sync-pppd.so cmd.so

OBJS=pty.o sync-pppd.o dstring.o
SRCS=$(OBJS:.o=.c)
CFLAGS=-g $(COPTS)  -I.. -I../libevent -I../../libevent -Wall -ffunction-sections -fdata-sections -Wl,--gc-sections

all: $(HANDLERS) l2tp-control

l2tp-control: cmd-control.o
	$(CC) -o l2tp-control cmd-control.o

cmd-control.o: cmd-control.c
	$(CC) -c -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

sync-pppd.so: pty.o sync-pppd.o
	$(CC) -shared -o $@ $^

cmd.so: cmd.o dstring.o
	$(CC) -shared -o $@ $^

clean:
	rm -f *.so *.o *~
	rm -f l2tp-control

install: all
	-mkdir -p $(RPM_INSTALL_ROOT)$(sbindir)
	-mkdir -p $(RPM_INSTALL_ROOT)$(prefix)/lib/l2tp/plugins
	$(install) -m 755 -s l2tp-control $(RPM_INSTALL_ROOT)$(sbindir)
	$(install) -m 755 $(HANDLERS) $(RPM_INSTALL_ROOT)$(prefix)/lib/l2tp/plugins
distclean: clean
