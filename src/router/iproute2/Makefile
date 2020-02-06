DESTDIR=
SBINDIR=/usr/sbin
CONFDIR=/etc/iproute2
DOCDIR=/usr/share/doc/iproute2
MANDIR=/usr/share/man
KERNEL_INCLUDE=$(LINUXDIR)/include
ASM_INCLUDE=-I$(LINUXDIR)/arch/mips/include 
ifeq ($(ARCH),mips)
ASM_INCLUDE=-I$(LINUXDIR)/arch/mips/include 
endif
ifeq ($(ARCH),mipsel)
ifeq ($(ARCHITECTURE),broadcom)
KERNEL_INCLUDE=../include
else
ASM_INCLUDE=-I$(LINUXDIR)/arch/mips/include 
endif
endif
ifeq ($(ARCH),arm)
ASM_INCLUDE=-I$(LINUXDIR)/arch/arm/include 
endif
ifeq ($(ARCH),armeb)
ASM_INCLUDE=-I$(LINUXDIR)/arch/arm/include 
endif
ifeq ($(ARCH),armbe)
ASM_INCLUDE=-I$(LINUXDIR)/arch/arm/include 
endif
ifeq ($(ARCH),i386)
ASM_INCLUDE=-I$(LINUXDIR)/arch/x86/include 
endif
ifeq ($(ARCH),powerpc)
ASM_INCLUDE=-I$(LINUXDIR)/arch/powerpc/include 
endif


# Path to db_185.h include
DBM_INCLUDE:=/usr/include

DEFINES= -DRESOLVE_HOSTNAMES

#options if you have a bind>=4.9.4 libresolv (or, maybe, glibc)
LDLIBS=-lresolv
ADDLIB=inet_ntop.o inet_pton.o

#options for decnet

ADDLIB+=dnet_ntop.o dnet_pton.o

#options for ipx
ADDLIB+=ipx_ntop.o ipx_pton.o

#CC = mipsel-linux-uclibc-gcc
HOSTCC = $(CC) 
#ipsel-linux-uclibc-gcc
CCOPTS = -D_GNU_SOURCE $(COPTS) -Wstrict-prototypes -Wall
CFLAGS = -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include
include ../.config
ifneq ($(CONFIG_NOMESSAGE),y)
CFLAGS += -DNEED_PRINTF
endif
ifeq ($(CONFIG_CAKE),y)
CFLAGS += -DHAVE_CAKE
CFLAGS += -DHAVE_FQ_CODEL_FAST
endif

CFLAGS += $(LTO) $(CCOPTS) $(MIPS16_OPT) -I$(KERNEL_INCLUDE) $(ASM_INCLUDE) -I../include $(DEFINES) -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS += $(COPTS) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections



LDLIBS += -L../lib -lnetlink -lutil 
#-ffunction-sections -fdata-sections -Wl,--gc-sections

SUBDIRS=lib ip tc

LIBNETLINK=../lib/libnetlink.a ../lib/libutil.a

all: Config
	@set -e; for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i; done

Config:
	./configure $(KERNEL_INCLUDE)

install: all
	install -m 0755 -d $(DESTDIR)$(SBINDIR)
	install -m 0755 -d $(DESTDIR)$(CONFDIR)
	install -m 0755 -d $(DESTDIR)$(DOCDIR)/examples
	install -m 0755 -d $(DESTDIR)$(DOCDIR)/examples/diffserv
	install -m 0644 README.iproute2+tc $(shell find examples -maxdepth 1 -type f) \
		$(DESTDIR)$(DOCDIR)/examples
	install -m 0644 $(shell find examples/diffserv -maxdepth 1 -type f) \
		$(DESTDIR)$(DOCDIR)/examples/diffserv
	@set -e; for i in $(SUBDIRS) doc; do $(MAKE) -C $$i install; done
	install -m 0644 $(shell find etc/iproute2 -maxdepth 1 -type f) $(DESTDIR)$(CONFDIR)
	install -m 0755 -d $(DESTDIR)$(MANDIR)/man8
	install -m 0644 $(shell find man/man8 -maxdepth 1 -type f) $(DESTDIR)$(MANDIR)/man8
	ln -sf $(MANDIR)/man8/tc-pbfifo.8  $(DESTDIR)$(MANDIR)/man8/tc-bfifo.8
	ln -sf $(MANDIR)/man8/tc-pbfifo.8  $(DESTDIR)$(MANDIR)/man8/tc-pfifo.8
	install -m 0755 -d $(DESTDIR)$(MANDIR)/man3
	install -m 0644 $(shell find man/man3 -maxdepth 1 -type f) $(DESTDIR)$(MANDIR)/man3

clean:
	@set -e; for i in $(SUBDIRS) doc; \
	do $(MAKE) $(MFLAGS) -C $$i clean; done

clobber: clean
	rm -f Config

distclean: clean clobber

.EXPORT_ALL_VARIABLES:
