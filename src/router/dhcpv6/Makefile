# Copyright (c) 2004 WIDE Project. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

#
# $Id: Makefile.in,v 1.16 2007/02/27 14:47:11 suzsuz Exp $
# $KAME: Makefile.in,v 1.45 2005/10/16 16:25:38 suz Exp $
#


prefix=	
srcdir=	.
sysconfdir= ${prefix}/etc
localdbdir= /var
user= bin
group= bin

CFLAGS=	-Os -pipe -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9  -msoft-float -mfloat-abi=soft -fno-caller-saves  -DNEED_PRINTF -D_GNU_SOURCE  -DUSE_DHCP6SRV -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(srcdir) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GETIFADDRS=1 -DHAVE_IF_NAMETOINDEX=1 -DHAVE_STRLCPY=1 -DHAVE_STRLCAT=1 -DHAVE_DAEMON=1 -DHAVE_WARNX=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FCNTL_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_SYSLOG_H=1 -DHAVE_UNISTD_H=1 -DHAVE_IFADDRS_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_STRUCT_TM_TM_ZONE=1 -DHAVE_TM_ZONE=1 -DHAVE_SIG_ATOMIC_T=1 -DGETPGRP_VOID=1 -DSETPGRP_VOID=1 -DRETSIGTYPE=void -DHAVE_MKTIME=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_CLOCK_GETTIME=1 -DHAVE_ANSI_FUNC=1 -DHAVE_TAILQ_FOREACH_REVERSE=1 -DHAVE_STDARG_H=1  -DSYSCONFDIR=\"${sysconfdir}\" \
	-DLOCALDBDIR=\"${localdbdir}\"
LDFLAGS=-Os -pipe -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9  -msoft-float -mfloat-abi=soft -fno-caller-saves  -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections
LIBOBJS= arc4random$U.o
LIBS=	 
CC=	ccache arm-linux-uclibc-gcc
TARGET=	dhcp6c dhcp6s dhcp6relay dhcp6ctl

INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
prefix=	
exec_prefix=	${prefix}
bindir=	${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
mandir=	${prefix}/man

GENSRCS=cfparse.c cftoken.c
CLIENTOBJS=	dhcp6c.o common.o config.o prefixconf.o dhcp6c_ia.o timer.o \
	dhcp6c_script.o if.o base64.o auth.o addrconf.o \
	$(GENSRCS:%.c=%.o)
ifeq ($(USE_DHCP6CTL),y)
CLIENTOBJS+=	dhcp6_ctl.o
CFLAGS+=	-DUSE_DHCP6CTL
endif
ifeq ($(USE_DHCP6SRV),y)
CLIENTOBJS+=	lease.o
CFLAGS+=	-DUSE_DHCP6SRV
endif

SERVOBJS=	dhcp6s.o common.o if.o config.o timer.o lease.o \
	base64.o auth.o dhcp6_ctl.o $(GENSRCS:%.c=%.o)
RELAYOBJS =	dhcp6relay.o dhcp6relay_script.o common.o timer.o
CTLOBJS= dhcp6_ctlclient.o base64.o auth.o
CLEANFILES+=	y.tab.h

all:	$(TARGET)
dhcp6c:	$(CLIENTOBJS) $(LIBOBJS)
	$(CC) $(LDFLAGS) -o dhcp6c $(CLIENTOBJS) $(LIBOBJS) $(LIBS)
dhcp6s:	$(SERVOBJS) $(LIBOBJS)
	$(CC) $(LDFLAGS) -o dhcp6s $(SERVOBJS) $(LIBOBJS) $(LIBS)
dhcp6relay: $(RELAYOBJS) $(LIBOBJS)
	$(CC) $(LDFLAGS) -o $@ $(RELAYOBJS) $(LIBOBJS) $(LIBS)
dhcp6ctl: $(CTLOBJS)
	$(CC) $(LDFLAGS) -o $@ $(CTLOBJS) $(LIBOBJS) $(LIBS)

cfparse.c y.tab.h: cfparse.y
	bison -y -d cfparse.y
	mv y.tab.c cfparse.c

cftoken.c: cftoken.l y.tab.h
	flex cftoken.l
	mv lex.yy.c $@	

getaddrinfo.o:	$(srcdir)/missing/getaddrinfo.c
	$(CC) -c $(srcdir)/missing/$*.c
getnameinfo.o:	$(srcdir)/missing/getnameinfo.c
	$(CC) -c $(srcdir)/missing/$*.c
strlcat.o:	$(srcdir)/missing/strlcat.c
	$(CC) -c $(srcdir)/missing/$*.c
strlcpy.o:	$(srcdir)/missing/strlcpy.c
	$(CC) -c $(srcdir)/missing/$*.c
arc4random.o:	$(srcdir)/missing/arc4random.c
	$(CC) $(CFLAGS) -c $(srcdir)/missing/$*.c
getifaddrs.o:	$(srcdir)/missing/getifaddrs.c
	$(CC) $(CFLAGS) -c $(srcdir)/missing/$*.c
daemon.o:	$(srcdir)/missing/daemon.c
	$(CC) -c $(srcdir)/missing/$*.c
warnx.o:	$(srcdir)/missing/warnx.c
	$(CC) -c $(srcdir)/missing/$*.c

$(srcdir)/ianaopts.h: gentab.pl bootp-dhcp-parameters
	expand bootp-dhcp-parameters | perl gentab.pl > ianaopts.h

install::
	-mkdir -p $(sbindir) $(mandir)/man5 $(mandir)/man8
	$(INSTALL_PROGRAM) -s -o $(user) -g $(group) $(TARGET) $(sbindir)
	$(INSTALL_DATA) -o $(user) -g $(group) dhcp6c.8 $(mandir)/man8
	$(INSTALL_DATA) -o $(user) -g $(group) dhcp6s.8 $(mandir)/man8
	$(INSTALL_DATA) -o $(user) -g $(group) dhcp6relay.8 $(mandir)/man8
	$(INSTALL_DATA) -o $(user) -g $(group) dhcp6ctl.8 $(mandir)/man8
	$(INSTALL_DATA) -o $(user) -g $(group) dhcp6c.conf.5 $(mandir)/man5
	$(INSTALL_DATA) -o $(user) -g $(group) dhcp6s.conf.5 $(mandir)/man5

includes::

clean::
	/bin/rm -f *.o $(TARGET) $(CLEANFILES) $(GENSRCS)

distclean:: clean
	/bin/rm -f Makefile config.cache config.log config.status .depend

depend:
	mkdep ${CFLAGS:M-[ID]*} $(srcdir)/*.c

package:
	tar -zcvf wide-dhcpv6.tar.gz $(srcdir)/*.[chyl1-8] $(srcdir)/Makefile.in \
		$(srcdir)/README $(srcdir)/COPYRIGHT $(srcdir)/CHANGES \
		$(srcdir)/configure $(srcdir)/configure.in \
		$(srcdir)/install-sh  $(srcdir)/*.sample \
		$(srcdir)/missing/arc4random.?  $(srcdir)/missing/strlcat.c \
		$(srcdir)/missing/strlcpy.c $(srcdir)/missing/daemon.c \
		$(srcdir)/missing/err.h $(srcdir)/missing/warnx.c \
		$(srcdir)/missing/ifaddrs.h $(srcdir)/missing/getifaddrs.c \
		$(srcdir)/missing/sys/queue.h
