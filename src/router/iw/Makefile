MAKEFLAGS += --no-print-directory

PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/sbin
MANDIR ?= $(PREFIX)/share/man
PKG_CONFIG ?= pkg-config

MKDIR ?= mkdir -p
INSTALL ?= install
CC ?= "gcc"

cc-option = $(shell set -e ; $(CC) $(1) -c -x c /dev/null -o /dev/null >/dev/null 2>&1 && echo '$(1)')

CFLAGS_EVAL := $(call cc-option,-Wstringop-overflow=4)

CFLAGS ?= -O2 -g
CFLAGS += -Wall -Wextra -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common
CFLAGS += -Werror-implicit-function-declaration -Wsign-compare -Wno-unused-parameter
CFLAGS += -Wdeclaration-after-statement
CFLAGS += -D__SANE_USERSPACE_TYPES__
CFLAGS += $(CFLAGS_EVAL)
CFLAGS += $(EXTRA_CFLAGS)

_OBJS := $(sort $(patsubst %.c,%.o,$(wildcard *.c)))
VERSION_OBJS := $(filter-out version.o, $(_OBJS))
OBJS := $(VERSION_OBJS) version.o

ALL = iw

ifeq ($(NO_PKG_CONFIG),)
NL3xFOUND := $(shell $(PKG_CONFIG) --atleast-version=3.2 libnl-3.0 && echo Y)
ifneq ($(NL3xFOUND),Y)
NL31FOUND := $(shell $(PKG_CONFIG) --exact-version=3.1 libnl-3.1 && echo Y)
ifneq ($(NL31FOUND),Y)
NL3FOUND := $(shell $(PKG_CONFIG) --atleast-version=3 libnl-3.0 && echo Y)
ifneq ($(NL3FOUND),Y)
NL2FOUND := $(shell $(PKG_CONFIG) --atleast-version=2 libnl-2.0 && echo Y)
ifneq ($(NL2FOUND),Y)
NL1FOUND := $(shell $(PKG_CONFIG) --atleast-version=1 libnl-1 && echo Y)
endif
endif
endif
endif

ifeq ($(NL1FOUND),Y)
NLLIBNAME = libnl-1
endif

ifeq ($(NL2FOUND),Y)
override CFLAGS += -DCONFIG_LIBNL20
override LIBS += -lnl-genl
NLLIBNAME = libnl-2.0
endif

ifeq ($(NL3xFOUND),Y)
# libnl 3.2 might be found as 3.2 and 3.0
NL3FOUND = N
override CFLAGS += -DCONFIG_LIBNL30
override LIBS += -lnl-genl-3
NLLIBNAME = libnl-3.0
endif

ifeq ($(NL3FOUND),Y)
override CFLAGS += -DCONFIG_LIBNL30
override LIBS += -lnl-genl
NLLIBNAME = libnl-3.0
endif

# nl-3.1 has a broken libnl-gnl-3.1.pc file
# as show by pkg-config --debug --libs --cflags --exact-version=3.1 libnl-genl-3.1;echo $?
ifeq ($(NL31FOUND),Y)
override CFLAGS += -DCONFIG_LIBNL30
override LIBS += -lnl-genl
NLLIBNAME = libnl-3.1
endif

ifeq ($(NLLIBNAME),)
$(error Cannot find development files for any supported version of libnl)
endif

override LIBS += $(shell $(PKG_CONFIG) --libs $(NLLIBNAME))
override CFLAGS += $(shell $(PKG_CONFIG) --cflags $(NLLIBNAME))
endif # NO_PKG_CONFIG

ifeq ($(V),1)
Q=
NQ=true
else
Q=@
NQ=echo
endif

all: $(ALL)

version.c: version.sh $(patsubst %.o,%.c,$(VERSION_OBJS)) nl80211.h iw.h Makefile \
		$(wildcard .git/index .git/refs/tags)
	@$(NQ) ' GEN ' $@
	$(Q)./version.sh $@

nl80211-commands.inc: nl80211.h
	@$(NQ) ' GEN ' $@
	$(Q)sed 's%^\tNL80211_CMD_%%;t n;d;:n s%^\([^=]*\),.*%\t[NL80211_CMD_\1] = \"\L\1\",%;t;d' nl80211.h | grep -v "reserved" > $@

%.o: %.c iw.h nl80211.h nl80211-commands.inc
	@$(NQ) ' CC  ' $@
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

ifeq ($(IW_ANDROID_BUILD),)
iw:	$(OBJS)
	@$(NQ) ' CC  ' iw
	$(Q)$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o iw
endif

check:
	$(Q)$(MAKE) all CC="REAL_CC=$(CC) CHECK=\"sparse -Wall\" cgcc"

%.gz: %
	@$(NQ) ' GZIP' $<
	$(Q)gzip < $< > $@

install: iw iw.8.gz
	@$(NQ) ' INST iw'
	$(Q)$(MKDIR) $(DESTDIR)$(SBINDIR)
	$(Q)$(INSTALL) -m 755 iw $(DESTDIR)$(SBINDIR)
	@$(NQ) ' INST iw.8'
	$(Q)$(MKDIR) $(DESTDIR)$(MANDIR)/man8/
	$(Q)$(INSTALL) -m 644 iw.8.gz $(DESTDIR)$(MANDIR)/man8/

clean:
	$(Q)rm -f iw *.o *~ *.gz version.c *-stamp nl80211-commands.inc
