# uncomment this to get a fully statically linked version
# NO_SHARED_LIBS = 1
include $(TOP)/.config

# uncomment this to disable IPv6 support
# DO_IPV6 = 0
#DO_IPV6 = 1

######################################################################
# YOU SHOULD NOT NEED TO TOUCH ANYTHING BELOW THIS LINE
######################################################################

# Standard part of Makefile for topdir.
TOPLEVEL_INCLUDED=YES

ifndef KERNEL_DIR
KERNEL_DIR="/lib/modules/$(shell uname -r)/build"
endif
IPTABLES_VERSION:=1.3.7
OLD_IPTABLES_VERSION:=1.3.6

PREFIX:=/usr/local
LIBDIR:=$(PREFIX)/lib
BINDIR:=$(PREFIX)/sbin
MANDIR:=$(PREFIX)/man
INCDIR:=$(PREFIX)/include
ARFLAGS=rv $(LTOPLUGIN)
# directory for new iptables releases
RELEASE_DIR:=/tmp

# Need libc6 for this.  FIXME: Should covert to autoconf.
#ifeq ($(shell [ -f /usr/include/netinet/ip6.h ] && echo YES), YES)
#DO_IPV6:=1
#endif
COPTS += $(MIPS16_OPT) $(THUMB) 
COPT_FLAGS:=$(COPTS) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -D_GNU_SOURCE -DIPTCSTATIC=
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
COPT_FLAGS += -DNEED_PRINTF
endif
endif
ifeq ($(CONFIG_DIST),"micro")
COPT_FLAGS += -DNEED_NOPRINTF
endif
ifneq ($(CONFIG_DIST),"micro-special")
COPT_FLAGS += -DNEED_NOPRINTF
endif

COPT_FLAGS+= -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include

ifeq ($(ARCH),mips)
COPT_FLAGS+= -I$(LINUXDIR)/arch/mips/include
endif
ifeq ($(ARCH),mipsel)
COPT_FLAGS+= -I$(LINUXDIR)/arch/mips/include
endif
ifeq ($(ARCH),powerpc)
COPT_FLAGS+= -I$(LINUXDIR)/arch/powerpc/include
endif
ifeq ($(ARCH),arm)
COPT_FLAGS+= -I$(LINUXDIR)/arch/arm/include
endif
ifeq ($(ARCH),armbe)
COPT_FLAGS+= -I$(LINUXDIR)/arch/arm/include
endif
ifeq ($(ARCH),i386)
COPT_FLAGS+= -I$(LINUXDIR)/arch/x86/include
endif


CFLAGS:=$(COPT_FLAGS) -Wall -Wunused -I$(KERNEL_DIR)/include -Iinclude/ -DIPTABLES_VERSION=\"$(IPTABLES_VERSION)\" #-g -DDEBUG #-pg # -DIPTC_DEBUG

CFLAGS += -DARCH_$(ARCHITECTURE)

ifeq ($(CONFIG_HOTPLUG2),y)
CFLAGS += -DNEW_KERNEL
endif
ifeq ($(CONFIG_BCMMODERN),y)
CFLAGS += -DBCMMODERN
endif

NO_SHARED_LIBS=1
DO_MULTI=1
ifdef NO_SHARED_LIBS
CFLAGS += -DNO_SHARED_LIBS=1
endif

EXTRAS+=iptables iptables.o iptables.8
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/iptables $(DESTDIR)$(MANDIR)/man8/iptables.8

# No longer experimental.
ifneq ($(DO_MULTI), 1)
EXTRAS+=iptables-save iptables-restore iptables-xml
endif
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/iptables-save $(DESTDIR)$(BINDIR)/iptables-restore $(DESTDIR)$(BINDIR)/iptables-xml $(DESTDIR)$(MANDIR)/man8/iptables-restore.8 $(DESTDIR)$(MANDIR)/man8/iptables-save.8

ifeq ($(DO_IPV6), 1)
EXTRAS+=ip6tables ip6tables.o ip6tables.8
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/ip6tables $(DESTDIR)$(MANDIR)/man8/ip6tables.8
ifneq ($(DO_MULTI), 1)
EXTRAS+=ip6tables-save ip6tables-restore
endif
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/ip6tables-save $(DESTDIR)$(BINDIR)/ip6tables-restore # $(DESTDIR)$(MANDIR)/man8/iptables-restore.8 $(DESTDIR)$(MANDIR)/man8/iptables-save.8 $(DESTDIR)$(MANDIR)/man8/ip6tables-save.8 $(DESTDIR)$(MANDIR)/man8/ip6tables-restore.8
endif

ifndef IPT_LIBDIR
IPT_LIBDIR:=$(LIBDIR)/iptables
endif

ifndef NO_SHARED_LIBS
DEPFILES = $(SHARED_LIBS:%.so=%.d)
SH_CFLAGS:=$(CFLAGS) -fPIC
STATIC_LIBS  =
STATIC6_LIBS =
LDFLAGS      = -rdynamic
LDLIBS       = -ldl -lnsl
else
DEPFILES = $(EXT_OBJS:%.o=%.d)
STATIC_LIBS  = extensions/libext.a
STATIC6_LIBS = extensions/libext6.a
#LDFLAGS      = -static
LDLIBS       =
endif

.PHONY: default
default: print-extensions all

.PHONY: print-extensions
print-extensions:
	@[ -n "$(OPTIONALS)" ] && echo Extensions found: $(OPTIONALS)

iptables.o: iptables.c
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" -c -o $@ $<

ifeq ($(DO_MULTI), 1)
iptables: iptables-multi.c iptables-restore.c iptables-standalone.c iptables.o $(STATIC_LIBS) libiptc/libip4tc.o
	$(CC) $(CFLAGS) -DIPTABLES_MULTI -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)
else
iptables: iptables-standalone.c iptables.o $(STATIC_LIBS) libiptc/libip4tc.o
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)
endif

$(DESTDIR)$(BINDIR)/iptables: iptables
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

iptables-save: iptables-save.c iptables.o $(STATIC_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)

ifeq ($(DO_MULTI), 1)
$(DESTDIR)$(BINDIR)/iptables-save: iptables
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	ln -sf $< $@
else
$(DESTDIR)$(BINDIR)/iptables-save: iptables-save
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@
endif

iptables-restore: iptables-restore.c iptables.o $(STATIC_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)

ifeq ($(DO_MULTI), 1)
$(DESTDIR)$(BINDIR)/iptables-restore: iptables
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	ln -sf $< $@
else
$(DESTDIR)$(BINDIR)/iptables-restore: iptables-restore
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@
endif

iptables-xml: iptables-xml.c #iptables.o # $(STATIC_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ifeq ($(DO_MULTI), 1)
$(DESTDIR)$(BINDIR)/iptables-xml: iptables-xml
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	ln -sf $< $@
else
$(DESTDIR)$(BINDIR)/iptables-xml: iptables-xml
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@
endif

ifeq ($(DO_MULTI), 1)
ip6tables: ip6tables-multi.c ip6tables-restore.c ip6tables-standalone.c ip6tables.o $(STATIC6_LIBS) libiptc/libip6tc.o
	$(CC) $(CFLAGS) -DIPTABLES_MULTI -DIPT6_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)
else
ip6tables: ip6tables-standalone.c ip6tables.o $(STATIC6_LIBS) libiptc/libip6tc.o
	$(CC) $(CFLAGS) -DIPT6_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)
endif



ip6tables.o: ip6tables.c
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" -c -o $@ $<

#ip6tables: ip6tables-standalone.c ip6tables.o $(STATIC6_LIBS) libiptc/libiptc.a
#	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DESTDIR)$(BINDIR)/ip6tables: ip6tables
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

ip6tables-save: ip6tables-save.c ip6tables.o $(STATIC6_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)

$(DESTDIR)$(BINDIR)/ip6tables-save: ip6tables-save
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@



ip6tables-restore: ip6tables-restore.c ip6tables.o $(STATIC6_LIBS) libiptc/libip6tc.o
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) $(LDLTO) -o $@ $^ $(LDLIBS)

$(DESTDIR)$(BINDIR)/ip6tables-restore: ip6tables-restore
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

$(DESTDIR)$(MANDIR)/man8/%.8: %.8
	@[ -d $(DESTDIR)$(MANDIR)/man8 ] || mkdir -p $(DESTDIR)$(MANDIR)/man8
	cp $< $@

EXTRA_DEPENDS+=iptables-standalone.d iptables.d

iptables-standalone.d iptables.d: %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | sed -e 's@^.*\.o:@$*.d $*.o:@' > $@

iptables.8: iptables.8.in extensions/libipt_matches.man extensions/libipt_targets.man
	sed -e '/@MATCH@/ r extensions/libipt_matches.man' -e '/@TARGET@/ r extensions/libipt_targets.man' iptables.8.in >iptables.8

ip6tables.8: ip6tables.8.in extensions/libip6t_matches.man extensions/libip6t_targets.man
	sed -e '/@MATCH@/ r extensions/libip6t_matches.man' -e '/@TARGET@/ r extensions/libip6t_targets.man' ip6tables.8.in >ip6tables.8

# Development Targets
.PHONY: install-devel-man3
install-devel-man3: $(DEVEL_MAN3)
	@[ -d $(DESTDIR)$(MANDIR)/man3 ] || mkdir -p $(DESTDIR)$(MANDIR)/man3
	@cp -v $(DEVEL_MAN3) $(DESTDIR)$(MANDIR)/man3

.PHONY: install-devel-headers
install-devel-headers: $(DEVEL_HEADERS)
	@[ -d $(DESTDIR)$(INCDIR) ] || mkdir -p $(DESTDIR)$(INCDIR)
	@cp -v $(DEVEL_HEADERS) $(DESTDIR)$(INCDIR)

.PHONY: install-devel-libs
install-devel-libs: $(DEVEL_LIBS)
	@[ -d $(DESTDIR)$(LIBDIR) ] || mkdir -p $(DESTDIR)$(LIBDIR)
	@cp -v $(DEVEL_LIBS) $(DESTDIR)$(LIBDIR)

.PHONY: install-devel
install-devel: all install-devel-man3 install-devel-headers install-devel-libs

.PHONY: distclean
distclean: clean
	@rm -f TAGS `find . -name '*~' -o -name '.*~'` `find . -name '*.rej'` `find . -name '*.d'` .makefirst

# Rusty's distro magic.
.PHONY: distrib
distrib: check distclean delrelease $(RELEASE_DIR)/iptables-$(IPTABLES_VERSION).tar.bz2 diff md5sums # nowhitespace

# Makefile must not define:
# -g -pg -DIPTC_DEBUG
.PHONY: check
check:
	@if echo $(CFLAGS) | egrep -e '-g|-pg|IPTC_DEBUG' >/dev/null; then echo Remove debugging flags; exit 1; else exit 0; fi

.PHONY: nowhitespace
nowhitespace:
	@if grep -n '[ 	]$$' `find . -name 'Makefile' -o -name '*.[ch]'`; then exit 1; else exit 0; fi

.PHONY: delrelease
delrelease:
	rm -f $(RELEASE_DIR)/iptables-$(IPTABLES_VERSION).tar.bz2

$(RELEASE_DIR)/iptables-$(IPTABLES_VERSION).tar.bz2:
	cd .. && ln -sf iptables iptables-$(IPTABLES_VERSION) && tar cvf - --exclude .svn iptables-$(IPTABLES_VERSION)/. | bzip2 -9 > $@ && rm iptables-$(IPTABLES_VERSION)

.PHONY: diff
diff: $(RELEASE_DIR)/iptables-$(IPTABLES_VERSION).tar.bz2
	@mkdir /tmp/diffdir
	@cd /tmp/diffdir && tar -x --bzip2 -f $(RELEASE_DIR)/iptables-$(IPTABLES_VERSION).tar.bz2
	@set -e; cd /tmp/diffdir; tar -x --bzip2 -f $(RELEASE_DIR)/iptables-$(OLD_IPTABLES_VERSION).tar.bz2; echo Creating patch-iptables-$(OLD_IPTABLES_VERSION)-$(IPTABLES_VERSION).bz2; diff -urN iptables-$(OLD_IPTABLES_VERSION) iptables-$(IPTABLES_VERSION) | bzip2 -9 > $(RELEASE_DIR)/patch-iptables-$(OLD_IPTABLES_VERSION)-$(IPTABLES_VERSION).bz2
	@rm -rf /tmp/diffdir

.PHONY: md5sums
md5sums:
	cd $(RELEASE_DIR)/ && md5sum patch-iptables-*-$(IPTABLES_VERSION).bz2 iptables-$(IPTABLES_VERSION).tar.bz2

# $(wildcard) fails wierdly with make v.3.78.1.
include $(shell echo */Makefile)
include Rules.make
