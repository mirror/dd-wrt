# Uncomment this on Solaris.
#LIBS = -lsocket -lnsl

CFLAGS ?= -O2
PKG_CONFIG ?= pkg-config


OBJS = cache.o rfc1035.o util.o option.o forward.o isc.o network.o \
       dnsmasq.o dhcp.o lease.o rfc2131.o netlink.o dbus.o

.c.o:
	$(CC) $(CFLAGS) $(I18N) `../bld/pkg-wrapper $(PKG_CONFIG) --cflags dbus-1` $(RPM_OPT_FLAGS) -Wall -W -c $<

dnsmasq : $(OBJS)
	$(CC) -o $@  $(OBJS) `../bld/pkg-wrapper $(PKG_CONFIG) --libs dbus-1` $(LIBS) 

dnsmasq.pot : $(OBJS:.o=.c) dnsmasq.h config.h
	xgettext -d dnsmasq --foreign-user --keyword=_ -o dnsmasq.pot -i $(OBJS:.o=.c)

%.mo : ../po/%.po dnsmasq.pot
	msgmerge -o - ../po/$*.po dnsmasq.pot | msgfmt -o $*.mo -
