CFLAGS ?= -O2
PKG_CONFIG ?= pkg-config


OBJS = cache.c rfc1035.c util.c option.c forward.c isc.c network.c \
       dnsmasq.c dhcp.c lease.c rfc2131.c netlink.c dbus.c bpf.c helper.c

.c.o:
	$(CC) $(CFLAGS) $(COPTS) $(I18N) `echo $(COPTS) | ../bld/pkg-wrapper $(PKG_CONFIG) --cflags dbus-1` $(RPM_OPT_FLAGS) -Wall -W -c $<

dnsmasq : $(OBJS)
	$(CC) $(LDFLAGS) -o $@  $(OBJS) `echo $(COPTS) | ../bld/pkg-wrapper $(PKG_CONFIG) --libs dbus-1` $(LIBS) 
 
dnsmasq.pot : $(OBJS:.o=.c) dnsmasq.h config.h
	xgettext -d dnsmasq --foreign-user --keyword=_ -o dnsmasq.pot -i $(OBJS:.o=.c)

%.mo : ../po/%.po dnsmasq.pot
	msgmerge -o - ../po/$*.po dnsmasq.pot | msgfmt -o $*.mo -
