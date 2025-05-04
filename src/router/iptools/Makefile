# SPDX-License-Identifier: GPL-2.0
SSOBJ=libnetlink.o utils.o inet_proto.o rt_names.o json_print.o color.o ll_addr.o rose_ntop.o netrom_ntop.o ax25_ntop.o json_writer.o ll_map.o ll_proto.o mpls_pton.o mpls_ntop.o cg_map.o namespace.o fs.o selinux.o
LNSTATOBJ=lnstat.o lnstat_util.o

TARGETS=ss nstat ifstat rtacct lnstat
CFLAGS += -Iinclude -Iinclude/uapi -Idb/staging/usr/include -I$(TOP)/libmnl/include -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -DARPDDIR=\"/var/lib/arpd\"
LDFLAGS += -Ldb/staging/usr/lib -L$(TOP)/libmnl/src/.libs

include config.mk

ifeq ($(HAVE_BERKELEY_DB),y)
	TARGETS += arpd
endif


all: make_db $(TARGETS)
	install -D config/arpd.webservices $(TOP)/httpd/ej_temp/arpd.webservices


configure:
	cd db/build_unix && ../dist/configure \
	--enable-shared \
	--disable-static \
	--disable-java \
	--with-mutex=POSIX/pthreads/library \
	--disable-tcl \
	--disable-stl \
	--disable-statistics \
	--enable-stripped_messages \
	--enable-compat185 \
	--disable-debug \
	--prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	$(MAKE) -C db/build_unix
	make -C db/build_unix install DESTDIR=$(TOP)/iptools/db/staging

make_db:
	$(MAKE) -C db/build_unix
	make -C db/build_unix install DESTDIR=$(TOP)/iptools/db/staging

ss: ss.o ssfilter_check.o ssfilter.tab.o $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(CFLAGS) $(LDLTO) $(LDFLAGS) $(LDLIBS) -o $@

nstat: nstat.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDLTO) $(CPPFLAGS) $(LDFLAGS) -o nstat $^ $(LDLIBS) -lm

ifstat: ifstat.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDLTO) $(CPPFLAGS) $(LDFLAGS) -o ifstat $^ $(LDLIBS) -lm

rtacct: rtacct.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDLTO) $(CPPFLAGS) $(LDFLAGS) -o rtacct $^ $(LDLIBS) -lm

arpd: arpd.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDLTO) $(CPPFLAGS) $(LDFLAGS) -o arpd $^ $(LDLIBS) -ldb

ssfilter.tab.c: ssfilter.y
	$(QUIET_YACC)$(YACC) -b ssfilter ssfilter.y

lnstat: $(LNSTATOBJ) $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(CFLAGS) $(LDLTO) $(LDFLAGS) $(LDLIBS) -o $@

install: all
	make -C db/build_unix install DESTDIR=$(INSTALLDIR)
	rm -rf $(INSTALLDIR)/$(DESTDIR)/usr/include
	rm -rf $(INSTALLDIR)/$(DESTDIR)/usr/bin
	rm -rf $(INSTALLDIR)/$(DESTDIR)/usr/docs
	rm -f $(INSTALLDIR)/$(DESTDIR)/usr/lib/*.la
	mkdir -p $(INSTALLDIR)/$(DESTDIR)/usr/sbin
	install -m 0755 $(TARGETS) $(INSTALLDIR)/$(DESTDIR)/usr/sbin
	ln -sf lnstat $(INSTALLDIR)/$(DESTDIR)/usr/sbin/rtstat
	ln -sf lnstat $(INSTALLDIR)/$(DESTDIR)/usr/sbin/ctstat
	install -D config/arpd.nvramconfig $(INSTALLDIR)/etc/config/arpd.nvramconfig
	install -D config/arpd.webservices $(INSTALLDIR)/etc/config/arpd.webservices

clean:
	make -C db/build_unix clean
	rm -f *.o $(TARGETS) ssfilter.c
