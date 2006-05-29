CFLAGS ?= -Os -pipe -mips32 -mtune=mips32 -funit-at-a-time
PKG_CONFIG ?= pkg-config


OBJS = cache.o rfc1035.o util.o option.o forward.o isc.o network.o \
       dnsmasq.o dhcp.o lease.o rfc2131.o netlink.o dbus.o bpf.o

.c.o:
	$(CC) $(CFLAGS) $(COPTS) $(I18N) `echo $(COPTS) | ../bld/pkg-wrapper $(PKG_CONFIG) --cflags dbus-1` $(RPM_OPT_FLAGS) -Wall -W -c $<

dnsmasq : $(OBJS)
	$(AR) arc -o $@.a  $(OBJS) `echo $(COPTS) | ../bld/pkg-wrapper $(PKG_CONFIG) --libs dbus-1` $(LIBS) 

dnsmasq.pot : $(OBJS:.o=.c) dnsmasq.h config.h
	xgettext -d dnsmasq --foreign-user --keyword=_ -o dnsmasq.pot -i $(OBJS:.o=.c)

%.mo : ../po/%.po dnsmasq.pot
	msgmerge -o - ../po/$*.po dnsmasq.pot | msgfmt -o $*.mo -
