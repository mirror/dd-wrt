# Rules.make for busybox
#
# Copyright (C) 1999-2005 by Erik Andersen <andersen@codepoet.org>
#
# Licensed under GPLv2, see the file LICENSE in this tarball for details.
#

# Pull in the user's busybox configuration
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(top_builddir)/.config
endif

#--------------------------------------------------------
PROG      := busybox
MAJOR_VERSION   :=1
MINOR_VERSION   :=1
SUBLEVEL_VERSION:=0
EXTRAVERSION    :=
VERSION   :=$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL_VERSION)$(EXTRAVERSION)
BUILDTIME := $(shell TZ=UTC date -u "+%Y.%m.%d-%H:%M%z")


#--------------------------------------------------------
# With a modern GNU make(1) (highly recommended, that's what all the
# developers use), all of the following configuration values can be
# overridden at the command line.  For example:
#   make CROSS=powerpc-linux- top_srcdir="$HOME/busybox" PREFIX=/mnt/app
#--------------------------------------------------------

# If you are running a cross compiler, you will want to set 'CROSS'
# to something more interesting...  Target architecture is determined
# by asking the CC compiler what arch it compiles things for, so unless
# your compiler is broken, you should not need to specify TARGET_ARCH
CROSS           =$(subst ",, $(strip $(CROSS_COMPILER_PREFIX)))
CC             = $(CROSS)gcc
AR             = $(CROSS)ar
AS             = $(CROSS)as
LD             = $(CROSS)ld
NM             = $(CROSS)nm
STRIP          = $(CROSS)strip
CPP            = $(CC) -E
# MAKEFILES      = $(top_builddir)/.config
RM             = rm
RM_F           = $(RM) -f
LN             = ln
LN_S           = $(LN) -s
MKDIR          = mkdir
MKDIR_P        = $(MKDIR) -p
MV             = mv
CP             = cp


# What OS are you compiling busybox for?  This allows you to include
# OS specific things, syscall overrides, etc.
TARGET_OS=linux

# Select the compiler needed to build binaries for your development system
HOSTCC    = gcc
HOSTCFLAGS= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer

# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL:= C

# If you want to add some simple compiler switches (like -march=i686),
# especially from the command line, use this instead of CFLAGS directly.
# For optimization overrides, it's better still to set OPTIMIZATION.
CFLAGS_EXTRA=$(subst ",, $(strip $(EXTRA_CFLAGS_OPTIONS)))

# To compile vs some other alternative libc, you may need to use/adjust
# the following lines to meet your needs...
#
# If you are using Red Hat 6.x with the compatible RPMs (for developing under
# Red Hat 5.x and glibc 2.0) uncomment the following.  Be sure to read about
# using the compatible RPMs (compat-*) at http://www.redhat.com !
#LIBCDIR:=/usr/i386-glibc20-linux
#
# For other libraries, you are on your own.  But these may (or may not) help...
#LDFLAGS+=-nostdlib
#LIBRARIES:=$(LIBCDIR)/lib/libc.a -lgcc
#CROSS_CFLAGS+=-nostdinc -I$(LIBCDIR)/include -I$(GCCINCDIR) -funsigned-char
#GCCINCDIR:=$(shell gcc -print-search-dirs | sed -ne "s/install: \(.*\)/\1include/gp")

WARNINGS=-Wall -Wstrict-prototypes -Wshadow
CFLAGS=-I$(top_builddir)/include -I$(top_srcdir)/include -I$(srcdir)
ARFLAGS=cru


# gcc centric. Perhaps fiddle with findstring gcc,$(CC) for the rest
# get the CC MAJOR/MINOR version
CC_MAJOR:=$(shell printf "%02d" $(shell echo __GNUC__ | $(CC) -E -xc - | tail -n 1))
CC_MINOR:=$(shell printf "%02d" $(shell echo __GNUC_MINOR__ | $(CC) -E -xc - | tail -n 1))

#--------------------------------------------------------
export VERSION BUILDTIME HOSTCC HOSTCFLAGS CROSS CC AR AS LD NM STRIP CPP
ifeq ($(strip $(TARGET_ARCH)),)
TARGET_ARCH:=$(shell $(CC) -dumpmachine | sed -e s'/-.*//' \
		-e 's/i.86/i386/' \
		-e 's/sparc.*/sparc/' \
		-e 's/arm.*/arm/g' \
		-e 's/m68k.*/m68k/' \
		-e 's/ppc/powerpc/g' \
		-e 's/v850.*/v850/g' \
		-e 's/sh[234]/sh/' \
		-e 's/mips-.*/mips/' \
		-e 's/mipsel-.*/mipsel/' \
		-e 's/cris.*/cris/' \
		)
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell \
	if [ "$(1)" != "" ]; then \
		if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
		then echo "$(1)"; else echo "$(2)"; fi \
	fi)

# Setup some shortcuts so that silent mode is silent like it should be
ifeq ($(subst s,,$(MAKEFLAGS)),$(MAKEFLAGS))
export MAKE_IS_SILENT=n
SECHO=@echo
else
export MAKE_IS_SILENT=y
SECHO=-@false
endif

CFLAGS+=$(call check_gcc,-funsigned-char,)



include $(top_builddir)/../.config
CONFIG_UDHCPD=n	
CONFIG_UDHCPC=n
CONFIG_HTTPD=n

CFLAGS += -I../../include.v23

LDFLAGS	+= -L../nvram -L../mipsel-uclibc/install/nvram/usr/lib -L../shared -L../mipsel-uclibc/install/shared/usr/lib -lshared -L../libnet/lib  
LIBRARIES += ../dnsmasq/src/dnsmasq.a -lnvram -lnet -lcrypt


ifeq ($(CONFIG_ONLYCLIENT),y)
CFLAGS += -DHAVE_ONLYCLIENT
endif

ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
endif

ifeq ($(CONFIG_MADWIFI),y)
CFLAGS += -DHAVE_MADWIFI
endif

ifeq ($(CONFIG_TFTP),y)
CFLAGS += -DHAVE_TFTP
endif

ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
endif
ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
endif

#ifeq ($(CONFIG_IPROUTE2),y)
#CFLAGS += -DHAVE_IPROUTE2
#LIBRARIES += ../iproute2/ip/ip.a ../iproute2/lib/libnetlink.a ../iproute2/lib/libutil.a ../iproute2/tc/q_htb.o ../iproute2/tc/tc.o ../iproute2/tc/tc_qdisc.o ../iproute2/tc/tc_class.o ../iproute2/tc/tc_filter.o ../iproute2/tc/tc_util.o ../iproute2/tc/m_police.o ../iproute2/tc/m_estimator.o ../iproute2/tc/m_action.o ../iproute2/tc/q_fifo.o ../iproute2/tc/q_sfq.o ../iproute2/tc/q_red.o ../iproute2/tc/q_prio.o ../iproute2/tc/q_tbf.o ../iproute2/tc/q_cbq.o ../iproute2/tc/q_wrr.o ../iproute2/tc/f_rsvp.o ../iproute2/tc/f_u32.o ../iproute2/tc/f_route.o ../iproute2/tc/f_fw.o ../iproute2/tc/q_dsmark.o ../iproute2/tc/q_gred.o ../iproute2/tc/f_tcindex.o ../iproute2/tc/q_ingress.o ../iproute2/tc/q_hfsc.o ../iproute2/tc/m_gact.o ../iproute2/tc/m_mirred.o ../iproute2/tc/m_ipt.o ../iproute2/tc/m_pedit.o ../iproute2/tc/p_ip.o ../iproute2/tc/p_icmp.o ../iproute2/tc/p_tcp.o ../iproute2/tc/p_udp.o ../iproute2/tc/tc_core.o ../iproute2/tc/tc_red.o ../iproute2/tc/tc_cbq.o ../iproute2/tc/tc_estimator.o -ldl -lm
#endif

ifeq ($(CONFIG_NEWMEDIA),y)
CFLAGS += -DHAVE_NEWMEDIA
endif
ifeq ($(CONFIG_SKYTRON),y)
CFLAGS += -DHAVE_SKYTRON
endif

ifeq ($(CONFIG_SKYTEL),y)
CFLAGS += -DHAVE_SKYTEL
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS += -DHAVE_MACBIND
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS += -DHAVE_ZEROIP
endif

ifeq ($(CONFIG_EBTABLES),y)
CFLAGS += -DHAVE_EBTABLES
endif

ifeq ($(CONFIG_RFLOW),y)
CFLAGS += -DHAVE_RFLOW
LIBRARIES += ../rflow/rflow.a -lpthread
endif

#CFLAGS += -DHAVE_DDNS
#LIBRARIES += ../ipupdate/ez-ipupdate.a


#ifeq ($(CONFIG_UPNP),y)
#CFLAGS += -DHAVE_UPNP
#LIBRARIES += ../upnp/upnp.a
#LIBRARIES += ../upnp/libupnp.a
#LIBRARIES += -L../netconf -lnetconf
#endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS += -DHAVE_OMNI
endif

ifeq ($(CONFIG_DLS),y)
CFLAGS += -DHAVE_DLS
endif

ifeq ($(CONFIG_TELNET),y)
CFLAGS += -DHAVE_TELNET
endif


ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif

ifeq ($(CONFIG_PPTPD),y)
#OBJS += pptpd.o
CFLAGS += -DHAVE_PPTPD
endif

ifeq ($(CONFIG_BOOT_WAIT_ON),y)
CFLAGS += -DBOOT_WAIT_ON
endif

ifeq ($(CONFIG_DROPBEAR_SSHD),y)
#OBJS += sshd.o
CFLAGS += -DHAVE_SSHD
LIBRARIES += -L../zlib ../dropbear/dropbear.a ../dropbear/libtomcrypt/libtomcrypt.a ../dropbear/libtommath/libtommath.a -lutil -lz  -lcrypt
#LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(CONFIG_RADVD),y)
CFLAGS += -DHAVE_RADVD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFWD
LIBRARIES += ../dhcpforwarder/dhcpfwd.a
endif

#ifeq ($(CONFIG_PPP),y)
#CFLAGS += -DHAVE_PPP
#LIBRARIES += ../pppd/pppd/pppd.a
#endif
LIBRARIES += ../net-tools/arp.a ../net-tools/lib/libnet-tools.a		


ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS += -DHAVE_BIRD
LIBRARIES += ../bird/obj/nest/all.o 
LIBRARIES += ../bird/obj/filter/all.o 
LIBRARIES += ../bird/obj/proto/bgp/all.o 
LIBRARIES += ../bird/obj/proto/ospf/all.o 
LIBRARIES += ../bird/obj/proto/pipe/all.o 
LIBRARIES += ../bird/obj/proto/rip/all.o 
LIBRARIES += ../bird/obj/proto/static/all.o 
LIBRARIES += ../bird/obj/conf/all.o
LIBRARIES += ../bird/obj/lib/birdlib.a

endif

ifeq ($(CONFIG_PPP),y)
CFLAGS += -DHAVE_PPP
endif

ifeq ($(CONFIG_ZEBRA),y)
CFLAGS += -DHAVE_ZEBRA
endif

ifeq ($(CONFIG_WSHAPER),y)
CFLAGS += -DHAVE_WSHAPER
endif

ifeq ($(CONFIG_SVQOS),y)
CFLAGS += -DHAVE_SVQOS
endif

ifeq ($(CONFIG_SNMP),y)
CFLAGS += -DHAVE_SNMP
#OBJS += snmp.o
endif

ifeq ($(CONFIG_WOL),y)
CFLAGS += -DHAVE_WOL
#OBJS += ../rc/wol.o
endif

ifeq ($(CONFIG_NOCAT),y)
CFLAGS += -DHAVE_NOCAT
endif

ifeq ($(CONFIG_SER),y)
CFLAGS += -DHAVE_SER
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DHAVE_FREEBIRD
endif

ifeq ($(CONFIG_DHCPFORWARD),y)
CFLAGS += -DHAVE_DHCPFORWARD
endif

ifeq ($(CONFIG_DHCPRELAY),y)
CFLAGS += -DHAVE_DHCPRELAY
endif

#ifeq ($(CONFIG_OPENSSL),y)
#CFLAGS += -DHAVE_HTTPS
#endif


#LIBRARIES += ../iptables/iptables.a ../iptables/extensions/libext.a ../iptables/libiptc/libiptc.a



#--------------------------------------------------------
# Arch specific compiler optimization stuff should go here.
# Unless you want to override the defaults, do not set anything
# for OPTIMIZATION...

# use '-Os' optimization if available, else use -O2
OPTIMIZATION:=$(call check_gcc,-Os,-O2)

# Some nice architecture specific optimizations
ifeq ($(strip $(TARGET_ARCH)),arm)
	OPTIMIZATION+=-fstrict-aliasing
endif
ifeq ($(strip $(TARGET_ARCH)),i386)
	OPTIMIZATION+=$(call check_gcc,-march=i386,)
	OPTIMIZATION+=$(call check_gcc,-mpreferred-stack-boundary=2,)
	OPTIMIZATION+=$(call check_gcc,-falign-functions=0 -falign-jumps=0 -falign-loops=0,\
		-malign-functions=0 -malign-jumps=0 -malign-loops=0)
endif
OPTIMIZATIONS:=$(OPTIMIZATION) -fomit-frame-pointer

#
#--------------------------------------------------------
# If you're going to do a lot of builds with a non-vanilla configuration,
# it makes sense to adjust parameters above, so you can type "make"
# by itself, instead of following it by the same half-dozen overrides
# every time.  The stuff below, on the other hand, is probably less
# prone to casual user adjustment.
#

ifeq ($(strip $(CONFIG_LFS)),y)
    # For large file summit support
    CFLAGS+=-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
endif
ifeq ($(strip $(CONFIG_DMALLOC)),y)
    # For testing mem leaks with dmalloc
    CFLAGS+=-DDMALLOC
    LIBRARIES:=-ldmalloc
else
    ifeq ($(strip $(CONFIG_EFENCE)),y)
	LIBRARIES:=-lefence
    endif
endif
ifeq ($(strip $(CONFIG_DEBUG)),y)
    CFLAGS  +=$(WARNINGS) -g -D_GNU_SOURCE
    LDFLAGS +=-Wl,-warn-common
    STRIPCMD:=/bin/true -Not_stripping_since_we_are_debugging
else
    CFLAGS+=$(WARNINGS) $(OPTIMIZATIONS) -D_GNU_SOURCE -DNDEBUG
    LDFLAGS += -Wl,-warn-common
    STRIPCMD:=$(STRIP) -s --remove-section=.note --remove-section=.comment
endif
ifeq ($(strip $(CONFIG_STATIC)),y)
    LDFLAGS += --static
endif

ifeq ($(strip $(CONFIG_SELINUX)),y)
    LIBRARIES += -lselinux
endif

ifeq ($(strip $(PREFIX)),)
    PREFIX:=`pwd`/_install
endif

# Additional complications due to support for pristine source dir.
# Include files in the build directory should take precedence over
# the copy in top_srcdir, both during the compilation phase and the
# shell script that finds the list of object files.
# Work in progress by <ldoolitt@recycle.lbl.gov>.


OBJECTS:=$(APPLET_SOURCES:.c=.o) busybox.o usage.o applets.o
CFLAGS    += $(CROSS_CFLAGS)
ifdef BB_INIT_SCRIPT
    CFLAGS += -DINIT_SCRIPT='"$(BB_INIT_SCRIPT)"'
endif

# Put user-supplied flags at the end, where they
# have a chance of winning.
CFLAGS += $(CFLAGS_EXTRA)

#------------------------------------------------------------
# Installation options
ifeq ($(strip $(CONFIG_INSTALL_APPLET_HARDLINKS)),y)
INSTALL_OPTS=--hardlinks
endif
ifeq ($(strip $(CONFIG_INSTALL_APPLET_SYMLINKS)),y)
INSTALL_OPTS=--symlinks
endif
ifeq ($(strip $(CONFIG_INSTALL_APPLET_DONT)),y)
INSTALL_OPTS=
endif

.PHONY: dummy
