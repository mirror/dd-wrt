#
# mdadm - manage Linux "md" devices aka RAID arrays.
#
# Copyright (C) 2001-2002 Neil Brown <neilb@cse.unsw.edu.au>
# Copyright (C) 2013 Neil Brown <neilb@suse.de>
#
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    Author: Neil Brown
#    Email: <neilb@cse.unsw.edu.au>
#    Paper: Neil Brown
#           School of Computer Science and Engineering
#           The University of New South Wales
#           Sydney, 2052
#           Australia
#

# define "CXFLAGS" to give extra flags to CC.
# e.g.  make CXFLAGS=-O to optimise
#CXFLAGS ?=-O2
TCC = tcc
UCLIBC_GCC = $(shell for nm in i386-uclibc-linux-gcc i386-uclibc-gcc; do which $$nm > /dev/null && { echo $$nm ; exit; } ; done; echo false No uclibc found )
#DIET_GCC = diet gcc
# sorry, but diet-libc doesn't know about posix_memalign,
# so we cannot use it any more.
DIET_GCC = gcc -DHAVE_STDINT_H

KLIBC=/home/src/klibc/klibc-0.77

KLIBC_GCC = gcc -nostdinc -iwithprefix include -I$(KLIBC)/klibc/include -I$(KLIBC)/linux/include -I$(KLIBC)/klibc/arch/i386/include -I$(KLIBC)/klibc/include/bits32

ifdef COVERITY
COVERITY_FLAGS=-include coverity-gcc-hack.h
endif

CC ?= $(CROSS_COMPILE)gcc
CXFLAGS ?= -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/libudev/src/libudev -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections
CWFLAGS = -Wall -Wstrict-prototypes -Wextra -Wno-unused-parameter
ifdef WARN_UNUSED
CWFLAGS += -Wp,-D_FORTIFY_SOURCE=2
endif

FALLTHROUGH := $(shell gcc -v --help 2>&1 | grep "implicit-fallthrough" | wc -l)
ifneq "$(FALLTHROUGH)"  "0"
CWFLAGS += -Wno-implicit-fallthrough
endif

ifdef DEBIAN
CPPFLAGS += -DDEBIAN
endif
ifdef DEFAULT_OLD_METADATA
 CPPFLAGS += -DDEFAULT_OLD_METADATA
 DEFAULT_METADATA=0.90
else
 DEFAULT_METADATA=1.2
endif
CPPFLAGS += -DBINDIR=\"$(BINDIR)\"

PKG_CONFIG ?= pkg-config

SYSCONFDIR = /etc
CONFFILE = $(SYSCONFDIR)/mdadm.conf
CONFFILE2 = $(SYSCONFDIR)/mdadm/mdadm.conf
MAILCMD =/usr/sbin/sendmail -t
CONFFILEFLAGS = -DCONFFILE=\"$(CONFFILE)\" -DCONFFILE2=\"$(CONFFILE2)\"
# Both MAP_DIR and MDMON_DIR should be somewhere that persists across the
# pivotroot from early boot to late boot.
# /run is best, but for distros that don't support that.
# /dev can work, in which case you probably want /dev/.mdadm
RUN_DIR=/var/run/mdadm
CHECK_RUN_DIR=1
MAP_DIR=$(RUN_DIR)
MAP_FILE = map
MAP_PATH = $(MAP_DIR)/$(MAP_FILE)
MDMON_DIR = $(RUN_DIR)
# place for autoreplace cookies
FAILED_SLOTS_DIR = $(RUN_DIR)/failed-slots
SYSTEMD_DIR=/lib/systemd/system
LIB_DIR=/usr/libexec/mdadm

COROSYNC:=$(shell [ -d /usr/include/corosync ] || echo -DNO_COROSYNC)
DLM:=$(shell [ -f /usr/include/libdlm.h ] || echo -DNO_DLM)

DIRFLAGS = -DMAP_DIR=\"$(MAP_DIR)\" -DMAP_FILE=\"$(MAP_FILE)\"
DIRFLAGS += -DMDMON_DIR=\"$(MDMON_DIR)\"
DIRFLAGS += -DFAILED_SLOTS_DIR=\"$(FAILED_SLOTS_DIR)\"
CFLAGS = $(CWFLAGS) $(CXFLAGS) -DSendmail=\""$(MAILCMD)"\" $(CONFFILEFLAGS) $(DIRFLAGS) $(COROSYNC) $(DLM)

VERSION = $(shell [ -d .git ] && git describe HEAD | sed 's/mdadm-//')
VERS_DATE = $(shell [ -d .git ] && date --iso-8601 --date="`git log -n1 --format=format:%cd --date=iso --date=short`")
DVERS = $(if $(VERSION),-DVERSION=\"$(VERSION)\",)
DDATE = $(if $(VERS_DATE),-DVERS_DATE="\"$(VERS_DATE)\"",)
DEXTRAVERSION = $(if $(EXTRAVERSION),-DEXTRAVERSION="\" - $(EXTRAVERSION)\"",)
CFLAGS += $(DVERS) $(DDATE) $(DEXTRAVERSION)

# The glibc TLS ABI requires applications that call clone(2) to set up
# TLS data structures, use pthreads until mdmon implements this support
USE_PTHREADS = 1
ifdef USE_PTHREADS
CFLAGS += -DUSE_PTHREADS
MON_LDFLAGS += -pthread
endif

# If you want a static binary, you might uncomment these
LDFLAGS +=  -L$(TOP)/libudev/src/libudev/.libs -ludev -ffunction-sections -fdata-sections -Wl,--gc-sections
STRIP =
LDLIBS=-ldl

INSTALL = /usr/bin/install
DESTDIR = $(INSTALLDIR)
BINDIR  = /sbin
LIBDIR  = /lib
MANDIR  = /usr/share/man
MAN4DIR = $(MANDIR)/man4
MAN5DIR = $(MANDIR)/man5
MAN8DIR = $(MANDIR)/man8

UDEVDIR := $(shell $(PKG_CONFIG) --variable=udevdir udev 2>/dev/null)
ifndef UDEVDIR
 UDEVDIR = /lib/udev
endif

ifeq (,$(findstring s,$(MAKEFLAGS)))
	ECHO=echo
else
	ECHO=:
endif
SHARE_OBJS = util.o maps.o sysfs.o sha1.o crc32.o msg.o xmalloc.o \
	platform-intel.o probe_roms.o crc32c.o super-mbr.o \
	super-gpt.o super-ddf.o super-intel.o bitmap.o dlink.o \
	restripe.o lib.o sg_io.o mdstat.o policy.o


OBJS =  mdadm.o config.o ReadMe.o uuid.o \
	Manage.o Assemble.o Build.o \
	Create.o Detail.o Examine.o Grow.o Monitor.o Kill.o Query.o \
	Incremental.o Dump.o \
	mdopen.o super0.o super1.o \
	mapfile.o
	

CHECK_OBJS =

SRCS =  $(patsubst %.o,%.c,$(OBJS))

INCL = mdadm.h part.h bitmap.h

MON_OBJS = mdmon.o monitor.o uuid.o managemon.o \
	Kill.o ReadMe.o

MON_SRCS = $(patsubst %.o,%.c,$(MON_OBJS))

STATICSRC = pwgr.c
STATICOBJS = pwgr.o

all : libmdadm.so mdadm mdmon
man : mdadm.man md.man mdadm.conf.man mdmon.man raid6check.man

check_rundir:
	@if [ ! -d "$(dir $(RUN_DIR))" -a  "$(CHECK_RUN_DIR)" = 1 ]; then \
		echo "***** Parent of $(RUN_DIR) does not exist.  Maybe set different RUN_DIR="; \
		echo "*****  e.g. make RUN_DIR=/dev/.mdadm" ; \
		echo "***** or set CHECK_RUN_DIR=0"; exit 1; \
	fi

everything: all mdadm.static swap_super test_stripe raid6check \
	mdadm.Os mdadm.O2 man
everything-test: all mdadm.static swap_super test_stripe \
	mdadm.Os mdadm.O2 man
# mdadm.uclibc doesn't work on x86-64
# mdadm.tcc doesn't work..

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(COVERITY_FLAGS) -o $@ -c $<

mdadm : $(OBJS) | check_rundir
	$(CC) $(CFLAGS) $(LDFLAGS) -o mdadm $(OBJS) $(LDLIBS) -L. -lmdadm

mdadm.static : $(OBJS) $(STATICOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -static -o mdadm.static $(OBJS) $(STATICOBJS) $(LDLIBS)

mdadm.tcc : $(SRCS) $(INCL)
	$(TCC) -o mdadm.tcc $(SRCS)

mdadm.klibc : $(SRCS) $(INCL)
	rm -f $(OBJS)
	$(CC) -nostdinc -iwithprefix include -I$(KLIBC)/klibc/include -I$(KLIBC)/linux/include -I$(KLIBC)/klibc/arch/i386/include -I$(KLIBC)/klibc/include/bits32 $(CFLAGS) $(SRCS)

mdadm.Os : $(SRCS) $(INCL)
	$(CC) -o mdadm.Os $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -DHAVE_STDINT_H -Os $(SRCS) $(LDLIBS)

mdadm.O2 : $(SRCS) $(INCL) mdmon.O2
	$(CC) -o mdadm.O2 $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -DHAVE_STDINT_H -O2 -D_FORTIFY_SOURCE=2 $(SRCS) $(LDLIBS)

mdmon.O2 : $(MON_SRCS) $(INCL) mdmon.h
	$(CC) -o mdmon.O2 $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(MON_LDFLAGS) -DHAVE_STDINT_H -O2 -D_FORTIFY_SOURCE=2 $(MON_SRCS) $(LDLIBS)

libmdadm.so: $(SHARE_OBJS)
	$(CC) --shared $(CFLAGS) -o libmdadm.so $(SHARE_OBJS)

# use '-z now' to guarantee no dynamic linker interactions with the monitor thread
mdmon : $(MON_OBJS) | check_rundir
	$(CC) $(CFLAGS) $(LDFLAGS) $(MON_LDFLAGS) -Wl,-z,now -o mdmon $(MON_OBJS) $(LDLIBS) -L. -lmdadm


msg.o: msg.c msg.h

test_stripe : restripe.c xmalloc.o mdadm.h
	$(CC) $(CFLAGS) $(CXFLAGS) $(LDFLAGS) -o test_stripe xmalloc.o  -DMAIN restripe.c

raid6check : raid6check.o mdadm.h 
	$(CC) $(CXFLAGS) $(LDFLAGS) -o raid6check raid6check.o -L. -lmdadm

mdadm.8 : mdadm.8.in
	sed -e 's/{DEFAULT_METADATA}/$(DEFAULT_METADATA)/g' \
	-e 's,{MAP_PATH},$(MAP_PATH),g'  mdadm.8.in > mdadm.8

mdadm.man : mdadm.8
	man -l mdadm.8 > mdadm.man

mdmon.man : mdmon.8
	man -l mdmon.8 > mdmon.man

md.man : md.4
	man -l md.4 > md.man

mdadm.conf.man : mdadm.conf.5
	man -l mdadm.conf.5 > mdadm.conf.man

raid6check.man : raid6check.8
	man -l raid6check.8 > raid6check.man

$(OBJS) : $(INCL) mdmon.h
$(MON_OBJS) : $(INCL) mdmon.h
$(SHARE_OBJS) : $(INCL) mdmon.h

sha1.o : sha1.c sha1.h md5.h
	$(CC) $(CFLAGS) -DHAVE_STDINT_H -o sha1.o -c sha1.c

install : install-bin install-udev

install-static : mdadm.static install-man
	$(INSTALL) -D $(STRIP) -m 755 mdadm.static $(DESTDIR)$(BINDIR)/mdadm

install-tcc : mdadm.tcc install-man
	$(INSTALL) -D $(STRIP) -m 755 mdadm.tcc $(DESTDIR)$(BINDIR)/mdadm

install-uclibc : mdadm.uclibc install-man
	$(INSTALL) -D $(STRIP) -m 755 mdadm.uclibc $(DESTDIR)$(BINDIR)/mdadm

install-klibc : mdadm.klibc install-man
	$(INSTALL) -D $(STRIP) -m 755 mdadm.klibc $(DESTDIR)$(BINDIR)/mdadm

install-man: mdadm.8 md.4 mdadm.conf.5 mdmon.8
	$(INSTALL) -D -m 644 mdadm.8 $(DESTDIR)$(MAN8DIR)/mdadm.8
	$(INSTALL) -D -m 644 mdmon.8 $(DESTDIR)$(MAN8DIR)/mdmon.8
	$(INSTALL) -D -m 644 md.4 $(DESTDIR)$(MAN4DIR)/md.4
	$(INSTALL) -D -m 644 mdadm.conf.5 $(DESTDIR)$(MAN5DIR)/mdadm.conf.5

install-udev: udev-md-raid-arrays.rules udev-md-raid-assembly.rules udev-md-raid-creating.rules \
		udev-md-clustered-confirm-device.rules 
	@for file in 01-md-raid-creating.rules 63-md-raid-arrays.rules 64-md-raid-assembly.rules \
		69-md-clustered-confirm-device.rules ; \
	do sed -e 's,BINDIR,$(BINDIR),g' udev-$${file#??-} > .install.tmp.1 && \
	   $(ECHO) $(INSTALL) -D -m 644 udev-$${file#??-} $(DESTDIR)$(UDEVDIR)/rules.d/$$file ; \
	   $(INSTALL) -D -m 644 .install.tmp.1 $(DESTDIR)$(UDEVDIR)/rules.d/$$file ; \
	   rm -f .install.tmp.1; \
	done

install-systemd: systemd/mdmon@.service
	@for file in mdmon@.service mdmonitor.service mdadm-last-resort@.timer \
		mdadm-last-resort@.service mdadm-grow-continue@.service \
		mdcheck_start.timer mdcheck_start.service \
		mdcheck_continue.timer mdcheck_continue.service \
		mdmonitor-oneshot.timer mdmonitor-oneshot.service \
		; \
	do sed -e 's,BINDIR,$(BINDIR),g' systemd/$$file > .install.tmp.2 && \
	   $(ECHO) $(INSTALL) -D -m 644 systemd/$$file $(DESTDIR)$(SYSTEMD_DIR)/$$file ; \
	   $(INSTALL) -D -m 644 .install.tmp.2 $(DESTDIR)$(SYSTEMD_DIR)/$$file ; \
	   rm -f .install.tmp.2; \
	done
	@for file in mdadm.shutdown ; \
	do sed -e 's,BINDIR,$(BINDIR),g' systemd/$$file > .install.tmp.3 && \
	   $(ECHO) $(INSTALL) -D -m 755  systemd/$$file $(DESTDIR)$(SYSTEMD_DIR)-shutdown/$$file ; \
	   $(INSTALL) -D -m 755  .install.tmp.3 $(DESTDIR)$(SYSTEMD_DIR)-shutdown/$$file ; \
	   rm -f .install.tmp.3; \
	done
	if [ -f /etc/SuSE-release -o -n "$(SUSE)" ] ;then $(INSTALL) -D -m 755 systemd/SUSE-mdadm_env.sh $(DESTDIR)$(LIB_DIR)/mdadm_env.sh ;fi

install-bin: mdadm mdmon
	$(INSTALL) -D $(STRIP) -m 755 mdadm $(DESTDIR)$(BINDIR)/mdadm
	$(INSTALL) -D $(STRIP) -m 755 mdmon $(DESTDIR)$(BINDIR)/mdmon
	$(INSTALL) -D $(STRIP) -m 755 misc/mdcheck $(DESTDIR)$(BINDIR)/mdcheck
	$(INSTALL) -D $(STRIP) -m 755 libmdadm.so $(DESTDIR)$(LIBDIR)/libmdadm.so

uninstall:
	rm -f $(DESTDIR)$(MAN8DIR)/mdadm.8 $(DESTDIR)$(MAN8DIR)/mdmon.8 $(DESTDIR)$(MAN4DIR)/md.4 $(DESTDIR)$(MAN5DIR)/mdadm.conf.5 $(DESTDIR)$(BINDIR)/mdadm

test: mdadm mdmon test_stripe swap_super raid6check
	@echo "Please run './test' as root"

clean :
	rm -f mdadm mdmon $(OBJS) $(MON_OBJS) $(SHARE_OBJS) $(STATICOBJS) core *.man \
	mdadm.tcc mdadm.uclibc mdadm.static *.orig *.porig *.rej *.alt \
	.merge_file_* mdadm.Os mdadm.O2 mdmon.O2 swap_super init.cpio.gz \
	mdadm.uclibc.static test_stripe raid6check raid6check.o mdmon mdadm.8
	rm -rf cov-int

dist : clean
	./makedist

testdist : everything-test clean
	./makedist test

TAGS :
	etags *.h *.c

DISTRO_MAKEFILE := $(wildcard distropkg/Makefile)
ifdef DISTRO_MAKEFILE
include $(DISTRO_MAKEFILE)
endif
