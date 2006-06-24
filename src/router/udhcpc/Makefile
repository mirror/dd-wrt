# udhcp makefile

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

prefix=/usr
SBINDIR=/sbin
USRSBINDIR=${prefix}/sbin
USRBINDIR=${prefix}/bin
USRSHAREDIR=${prefix}/share

# Uncomment this to get a shared binary. Call as udhcpd for the server,
# and udhcpc for the client
#COMBINED_BINARY=1

#ifdef DEBUG_DHCPD
# Uncomment this for extra output and to compile with debugging symbols
# DEBUG=1
#endif


CFLAGS += -I$(SRCBASE)/include/ -I$(SRCBASE)/router/shared/ -I$(SRCBASE)/
#LDFLAGS =  -L$(TOP)/shared -lshared  -L$(TOP)/nvram -lnvram

#CROSS_COMPILE=arm-uclibc-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
INSTALL = install

VER := 0.9.8


OBJS_SHARED = options.o socket.o packet.o pidfile.o common.o signalpipe.o
DHCPD_OBJS = dhcpd.o arpping.o files.o leases.o serverpacket.o static_leases.o
DHCPC_OBJS = dhcpc.o clientpacket.o script.o clientsocket.o arpping.o

#ifdef COMBINED_BINARY
#EXEC1 = udhcpd
#OBJS1 = $(DHCPD_OBJS) $(DHCPC_OBJS) $(OBJS_SHARED) frontend.o
#CFLAGS += -DCOMBINED_BINARY
#else
#EXEC1 = udhcpd
#OBJS1 = $(DHCPD_OBJS) $(OBJS_SHARED)
#
EXEC2 = udhcpc
OBJS2 = $(DHCPC_OBJS) $(OBJS_SHARED)
#endif
#
#EXEC3 = dumpleases
#OBJS3 = dumpleases.o

BOOT_PROGRAMS = udhcpc
DAEMONS = udhcpd
COMMANDS = dumpleases

CFLAGS += -DUDHCP_SYSLOG
#CFLAGS += -DUDHCP_DEBUG


#CFLAGS += -W -Wall -Wstrict-prototypes -DVERSION='"$(VER)"'
CFLAGS += -W -Wall -Wstrict-prototypes


ifdef DEBUG
CFLAGS += -g -DDEBUG -DMY_DEBUG
STRIP=true
else
CFLAGS += -fomit-frame-pointer $(COPTS)
STRIP=$(CROSS_COMPILE)strip
endif

all: $(EXEC1) $(EXEC2) $(EXEC3)
	$(STRIP) --remove-section=.note --remove-section=.comment $(EXEC1) $(EXEC2) $(EXEC3)

$(OBJS1) $(OBJS2) $(OBJS3): *.h Makefile $(CY_DEPS)
$(EXEC1) $(EXEC2) $(EXEC3): Makefile

.c.o:
	$(CC) -c $(CFLAGS) $<

$(EXEC1): $(OBJS1)
	$(LD) $(LDFLAGS) $(OBJS1) -o $(EXEC1)

$(EXEC2): $(OBJS2)
	$(LD) $(LDFLAGS) $(OBJS2) -o $(EXEC2)

$(EXEC3): $(OBJS3)
	$(LD) $(LDFLAGS) $(OBJS3) -o $(EXEC3)

foo-install: all

	$(INSTALL) $(DAEMONS) $(USRSBINDIR)
	$(INSTALL) $(COMMANDS) $(USRBINDIR)
ifdef COMBINED_BINARY
	ln -sf $(USRSBINDIR)/$(DAEMONS) $(SBINDIR)/$(BOOT_PROGRAMS)
else
	$(INSTALL) $(BOOT_PROGRAMS) $(SBINDIR)
endif
	mkdir -p $(USRSHAREDIR)/udhcpc
	for name in bound deconfig renew script ; do \
		$(INSTALL) samples/sample.$$name \
			$(USRSHAREDIR)/udhcpc/default.$$name ; \
	done
	mkdir -p $(USRSHAREDIR)/man/man1
	$(INSTALL) dumpleases.1 $(USRSHAREDIR)/man/man1
	mkdir -p $(USRSHAREDIR)/man/man5
	$(INSTALL) udhcpd.conf.5 $(USRSHAREDIR)/man/man5
	mkdir -p $(USRSHAREDIR)/man/man8
	$(INSTALL) udhcpc.8 udhcpd.8 $(USRSHAREDIR)/man/man8

clean:
	-rm -f udhcpd udhcpc dumpleases *.o core

