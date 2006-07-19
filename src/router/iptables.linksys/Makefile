# uncomment this to get a fully statically linked version
# NO_SHARED_LIBS = 1

######################################################################
# YOU SHOULD NOT NEED TO TOUCH ANYTHING BELOW THIS LINE
######################################################################

# Standard part of Makefile for topdir.
TOPLEVEL_INCLUDED=YES

ifndef KERNEL_DIR
KERNEL_DIR=$(LINUXDIR)
endif
IPTABLES_VERSION:=1.2.7a
OLD_IPTABLES_VERSION:=1.2.6a

PREFIX:=/usr/local
LIBDIR:=$(PREFIX)/lib
BINDIR:=$(PREFIX)/sbin
MANDIR:=$(PREFIX)/man
INCDIR:=$(PREFIX)/include

# directory for new iptables releases
RELEASE_DIR:=/tmp

# Need libc6 for this.  FIXME: Should covert to autoconf.
# ifeq ($(shell [ -f /usr/include/netinet/ip6.h ] && echo YES), YES)
# DO_IPV6=1
# endif

#LD=mipsel-linux-uclibc-ld
#CC=mipsel-linux-uclibc-gcc
COPT_FLAGS:=$(COPTS)
CFLAGS:=$(COPT_FLAGS) -Wall -Wunused -I$(KERNEL_DIR)/include -Iinclude/ -DIPTABLES_VERSION=\"$(IPTABLES_VERSION)\" #-g -DDEBUG #-pg # -DIPTC_DEBUG

ifdef NO_SHARED_LIBS
CFLAGS += -DNO_SHARED_LIBS=1
endif

ifndef NO_SHARED_LIBS
DEPFILES = $(SHARED_LIBS:%.so=%.d)
SH_CFLAGS:=$(CFLAGS) -fPIC
STATIC_LIBS  =
STATIC6_LIBS =
LDFLAGS      = -rdynamic
LDLIBS       = -ldl
else
DEPFILES = $(EXT_OBJS:%.o=%.d)
STATIC_LIBS  = extensions/libext.a
STATIC6_LIBS = extensions/libext6.a
LDFLAGS      = -static
LDLIBS       =
endif

EXTRAS+=iptables iptables.o
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/iptables $(DESTDIR)$(MANDIR)/man8/iptables.8

# No longer experimental.
EXTRAS+=iptables-save iptables-restore
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/iptables-save $(DESTDIR)$(BINDIR)/iptables-restore $(DESTDIR)$(MANDIR)/man8/iptables-restore.8 $(DESTDIR)$(MANDIR)/man8/iptables-save.8

ifdef DO_IPV6
EXTRAS+=ip6tables ip6tables.o
EXTRA_INSTALLS+=$(DESTDIR)$(BINDIR)/ip6tables $(DESTDIR)$(MANDIR)/man8/ip6tables.8
EXTRAS_EXP+=ip6tables-save ip6tables-restore
EXTRA_INSTALLS_EXP+=$(DESTDIR)$(BINDIR)/ip6tables-save $(DESTDIR)$(BINDIR)/ip6tables-restore # $(DESTDIR)$(MANDIR)/man8/iptables-restore.8 $(DESTDIR)$(MANDIR)/man8/iptables-save.8 $(DESTDIR)$(MANDIR)/man8/ip6tables-save.8 $(DESTDIR)$(MANDIR)/man8/ip6tables-restore.8
endif

# Sparc64 hack
ifeq ($(shell uname -m),sparc64)
# The kernel is 64-bit, even though userspace is 32.
CFLAGS+=-DIPT_MIN_ALIGN=8 -DKERNEL_64_USERSPACE_32
endif

# HPPA hack
ifeq ($(shell uname -m),parisc64)
# The kernel is 64-bit, even though userspace is 32.
CFLAGS+=-DIPT_MIN_ALIGN=8 -DKERNEL_64_USERSPACE_32
endif

ifndef IPT_LIBDIR
IPT_LIBDIR:=$(LIBDIR)/iptables
endif

.PHONY: default
default: print-extensions all

.PHONY: print-extensions
print-extensions:
ifneq ($(OPTIONALS),)
	echo Extensions found: $(OPTIONALS)
endif

libiptables.so: iptables.o libiptc/libip4tc.o
	$(LD) -shared -o $@ $^

libiptables.a: iptables.o libiptc/libip4tc.o
	$(AR) arc -o $@ $^

iptables.o: iptables.c
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" -c -o $@ $<

#iptables: iptables-standalone.c iptables.o $(STATIC_LIBS) libiptc/libiptc.a
#	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)
iptables: iptables-standalone.o iptables.o $(STATIC_LIBS) libiptables.so libiptables.a
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ iptables-standalone.o $(LDLIBS) -L. -liptables

$(DESTDIR)$(BINDIR)/iptables: iptables
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

#iptables-save: iptables-save.c iptables.o $(STATIC_LIBS) libiptc/libiptc.a
#	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)
iptables-save: iptables-save.o iptables.o $(STATIC_LIBS) libiptables.so libiptables.a
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ iptables-save.o $(LDLIBS) -L. -liptables

$(DESTDIR)$(BINDIR)/iptables-save: iptables-save
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

#iptables-restore: iptables-restore.c iptables.o $(STATIC_LIBS) libiptc/libiptc.a
#	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)
iptables-restore: iptables-restore.o iptables.o $(STATIC_LIBS) libiptables.so libiptables.a
	$(CC) $(CFLAGS) -DIPT_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ iptables-restore.o $(LDLIBS) -L. -liptables

$(DESTDIR)$(BINDIR)/iptables-restore: iptables-restore
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

ip6tables.o: ip6tables.c
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" -c -o $@ $<

ip6tables: ip6tables-standalone.c ip6tables.o $(STATIC6_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DESTDIR)$(BINDIR)/ip6tables: ip6tables
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

ip6tables-save: ip6tables-save.c ip6tables.o $(STATIC6_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DESTDIR)$(BINDIR)/ip6tables-save: ip6tables-save
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

ip6tables-restore: ip6tables-restore.c ip6tables.o $(STATIC6_LIBS) libiptc/libiptc.a
	$(CC) $(CFLAGS) -DIP6T_LIB_DIR=\"$(IPT_LIBDIR)\" $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DESTDIR)$(BINDIR)/ip6tables-restore: ip6tables-restore
	@[ -d $(DESTDIR)$(BINDIR) ] || mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $@

$(DESTDIR)$(MANDIR)/man8/%.8: %.8
	@[ -d $(DESTDIR)$(MANDIR)/man8 ] || mkdir -p $(DESTDIR)$(MANDIR)/man8
	cp $< $@

EXTRA_DEPENDS+=iptables-standalone.d iptables.d

iptables-standalone.d iptables.d: %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | sed -e 's@^.*\.o:@$*.d $*.o:@' > $@


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
	cd .. && ln -sf userspace iptables-$(IPTABLES_VERSION) && tar cvf - --exclude CVS iptables-$(IPTABLES_VERSION)/. | bzip2 -9 > $@ && rm iptables-$(IPTABLES_VERSION)

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
