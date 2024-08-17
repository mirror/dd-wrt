# $Id: Makefile.in,v 1.43 2008/06/08 17:32:29 dtucker Exp $

sysconfdir=${prefix}/etc
piddir=/var/run
srcdir=.
top_srcdir=..


#CC=$gcc
#LD=gcc
CFLAGS=$(COPTS) $(LTO) -Wall -Wpointer-arith -Wuninitialized -Wsign-compare -Wno-pointer-sign -Wformat-security -fno-builtin-memset -std=gnu99 -I$(SSLPATH)/include $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections
CPPFLAGS=-I. -I.. -I$(srcdir) -I$(srcdir)/..  -DHAVE_CONFIG_H
LIBS=-lcrypto -lutil -lz -lnsl  -lcrypt -lresolv -lresolv
AR=/usr/bin/ar
RANLIB=ranlib $(LTOPLUGIN)
INSTALL=/usr/bin/install -c
LDFLAGS=-L. $(LDLTO)

OPENBSD=getopt.o strmode.o vis.o strtonum.o

#COMPAT=bsd-arc4random.o bsd-asprintf.o bsd-closefrom.o bsd-cray.o bsd-cygwin_util.o bsd-getpeereid.o bsd-misc.o bsd-nextstep.o bsd-openpty.o bsd-poll.o bsd-snprintf.o bsd-statvfs.o bsd-waitpid.o fake-rfc2553.o openssl-compat.o xmmap.o xcrypt.o
COMPAT=bsd-misc.o
PORTS=port-tun.o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

all: libopenbsd-compat.a

$(COMPAT): ../config.h
$(OPENBSD): ../config.h
$(PORTS): ../config.h

libopenbsd-compat.a:  $(COMPAT) $(OPENBSD) $(PORTS)
	$(AR) rv $(LTOPLUGIN) $@ $(COMPAT) $(OPENBSD) $(PORTS)
	$(RANLIB) $@

clean:
	rm -f *.o *.a core 

distclean: clean
	rm -f Makefile *~
