#CC := $(CROSS)gcc
#AR := $(CROSS)ar
#RANLIB := $(CROSS)ranlib

# Stolen from Linux build system
comma = ,
try-run = $(shell set -e; ($(1)) >/dev/null 2>&1 && echo "$(2)" || echo "$(3)")
cc-option = $(call try-run, $(CC) $(1) -c -xc /dev/null -o /dev/null,$(1),$(2))

CFLAGS ?= $(COPTS)
WFLAGS := -Wall \
	$(call cc-option,-Wextra) \
	$(call cc-option,-Wwrite-strings) \
	$(call cc-option,-Wno-sign-compare)
CFLAGS += $(WFLAGS)  -L$(TOP)/zlib -DNEED_PRINTF
SECTION_CFLAGS := $(call cc-option,-ffunction-sections -fdata-sections -Wl$(comma)--gc-sections)
CFLAGS += $(SECTION_CFLAGS)

ifneq ($(WITHOUT_LARGEFILE), 1)
  CPPFLAGS += -D_FILE_OFFSET_BITS=64
endif

DESTDIR?=
PREFIX=/usr
EXEC_PREFIX=$(PREFIX)
SBINDIR=$(EXEC_PREFIX)/sbin
MANDIR=$(PREFIX)/share/man
INCLUDEDIR=$(PREFIX)/include

ifndef BUILDDIR
ifeq ($(origin CROSS),undefined)
  BUILDDIR := $(CURDIR)
else
# Remove the trailing slash to make the directory name
  BUILDDIR := $(CURDIR)/$(CROSS:-=)
endif
endif
override BUILDDIR := $(patsubst %/,%,$(BUILDDIR))

override TARGETS := $(addprefix $(BUILDDIR)/,$(TARGETS))

SUBDIRS_ALL = $(patsubst %,subdirs_%_all,$(SUBDIRS))
SUBDIRS_CLEAN = $(patsubst %,subdirs_%_clean,$(SUBDIRS))
SUBDIRS_INSTALL = $(patsubst %,subdirs_%_install,$(SUBDIRS))

all:: $(TARGETS) $(SUBDIRS_ALL)

clean:: $(SUBDIRS_CLEAN)
	rm -f $(BUILDDIR)/*.o $(TARGETS) $(BUILDDIR)/.*.c.dep

install:: $(TARGETS) $(SUBDIRS_INSTALL)

%: %.o $(LDDEPS) $(LDDEPS_$(notdir $@))
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_$(notdir $@)) -g -o $@ $^ $(LDLIBS) $(LDLIBS_$(notdir $@))

$(BUILDDIR)/%.a:
	$(AR) crv $@ $^
	$(RANLIB) $@

$(BUILDDIR)/%.o: %.c
ifneq ($(BUILDDIR),$(CURDIR))
	mkdir -p $(dir $@)
endif
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $< -g -Wp,-MD,$(BUILDDIR)/.$(<F).dep

subdirs_%:
	d=$(patsubst subdirs_%,%,$@); \
	t=`echo $$d | sed s:.*_::` d=`echo $$d | sed s:_.*::`; \
	$(MAKE) BUILDDIR=$(BUILDDIR)/$$d -C $$d $$t

.SUFFIXES:

IGNORE=${wildcard $(BUILDDIR)/.*.c.dep}
-include ${IGNORE}

PHONY += all clean install
.PHONY: $(PHONY)
