# SPDX-License-Identifier: GPL-2.0
SSOBJ=libnetlink.o utils.o inet_proto.o rt_names.o json_print.o color.o ll_addr.o rose_ntop.o netrom_ntop.o ax25_ntop.o json_writer.o ll_map.o ll_proto.o mpls_pton.o mpls_ntop.o cg_map.o namespace.o fs.o
LNSTATOBJ=lnstat.o lnstat_util.o

TARGETS=ss nstat ifstat rtacct lnstat
CFLAGS += -Iinclude -Iinclude/uapi -Idb/staging/usr/include -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF
LDFLAGS += -Ldb/staging/usr/lib

include config.mk

ifeq ($(HAVE_BERKELEY_DB),y)
	TARGETS += arpd
endif

all: $(TARGETS)


configure:
	cd db/build_unix && ../dist/configure \
	--enable-shared \
	--disable-static \
	--disable-java \
	--with-mutex=POSIX/pthreads/library \
	--disable-tcl \
	--enable-compat185 \
	--disable-debug \
	--prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(CHILLIEXTRA_CFLAGS) -fcommon -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	$(MAKE) -C db/build_unix
	$(MAKE) -C db/build_unix install DESTDIR=$(TOP)/iptools/db/staging

db:
	$(MAKE) -C db/build_unix
	$(MAKE) -C db/build_unix install DESTDIR=$(TOP)/iptools/db/staging

ss: ss.o ssfilter_check.o ssfilter.tab.o $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

nstat: nstat.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o nstat $^ $(LDLIBS) -lm

ifstat: ifstat.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o ifstat $^ $(LDLIBS) -lm

rtacct: rtacct.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o rtacct $^ $(LDLIBS) -lm

arpd: arpd.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o arpd $^ $(LDLIBS) -ldb

ssfilter.tab.c: ssfilter.y
	$(QUIET_YACC)$(YACC) -b ssfilter ssfilter.y

lnstat: $(LNSTATOBJ) $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	$(MAKE) -C db/build_unix install DESTDIR=$(INSTALLDIR)
	rm -rf $(INSTALLDIR)/$(DESTDIR)/usr/include
	rm -rf $(INSTALLDIR)/$(DESTDIR)/usr/docs
	rm -f $(INSTALLDIR)/$(DESTDIR)/usr/lib/*.la
	mkdir -p $(INSTALLDIR)/$(DESTDIR)/usr/sbin
	install -m 0755 $(TARGETS) $(INSTALLDIR)/$(DESTDIR)/usr/sbin
	ln -sf lnstat $(INSTALLDIR)/$(DESTDIR)/usr/sbin/rtstat
	ln -sf lnstat $(INSTALLDIR)/$(DESTDIR)/usr/sbin/ctstat

clean:
	make -C db/build_unix clean
	rm -f *.o $(TARGETS) ssfilter.c
