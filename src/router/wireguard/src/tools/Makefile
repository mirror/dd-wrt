# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

PKG_CONFIG ?= pkg-config
PREFIX ?= /usr
DESTDIR ?=
SYSCONFDIR ?= /etc
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
MANDIR ?= $(PREFIX)/share/man
BASHCOMPDIR ?= $(PREFIX)/share/bash-completion/completions
SYSTEMDUNITDIR ?= $(shell $(PKG_CONFIG) --variable=systemdsystemunitdir systemd 2>/dev/null || echo "$(PREFIX)/lib/systemd/system")
RUNSTATEDIR ?= /var/run
WITH_BASHCOMPLETION ?=
WITH_WGQUICK ?=
WITH_SYSTEMDUNITS ?=

ifeq ($(WITH_BASHCOMPLETION),)
ifneq ($(strip $(wildcard $(BASHCOMPDIR))),)
WITH_BASHCOMPLETION := yes
endif
endif
ifeq ($(WITH_WGQUICK),)
ifneq ($(strip $(wildcard $(BINDIR)/bash)),)
WITH_WGQUICK := yes
endif
ifneq ($(strip $(wildcard $(DESTDIR)/bin/bash)),)
WITH_WGQUICK := yes
endif
endif
ifeq ($(WITH_SYSTEMDUNITS),)
ifneq ($(strip $(wildcard $(SYSTEMDUNITDIR))),)
WITH_SYSTEMDUNITS := yes
endif
endif

PLATFORM ?= $(shell uname -s | tr '[:upper:]' '[:lower:]')

CFLAGS ?= -O3
CFLAGS += -std=gnu99 -D_GNU_SOURCE -DNEED_PRINTF $(LTO)
CFLAGS += -Wall -Wextra
CFLAGS += -MMD -MP
CFLAGS += -DRUNSTATEDIR="\"$(RUNSTATEDIR)\""
ifeq ($(DEBUG_TOOLS),y)
CFLAGS += -g
endif
CFLAGS += $(LIBMNL_CFLAGS)
LDLIBS += $(LIBMNL_LDLIBS)  $(LDLTO)

ifneq ($(V),1)
BUILT_IN_LINK.o := $(LINK.o)
LINK.o = @echo "  LD      $$(pwd)/$@";
LINK.o += $(BUILT_IN_LINK.o)
BUILT_IN_COMPILE.c := $(COMPILE.c)
COMPILE.c = @echo "  CC      $$(pwd)/$@";
COMPILE.c += $(BUILT_IN_COMPILE.c)
endif
ifeq ($(PLATFORM),haiku)
LDLIBS += -lnetwork -lbsd
endif
ifeq ($(PLATFORM),windows)
CC := x86_64-w64-mingw32-gcc
WINDRES := x86_64-w64-mingw32-windres
CFLAGS += -Iwincompat/include -include wincompat/compat.h -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -flto
LDLIBS += -lws2_32 -flto
VERSION := $(patsubst "%",%,$(filter "%",$(file < version.h)))
wg: wincompat/libc.o wincompat/init.o wincompat/resources.o
wincompat/resources.o: wincompat/resources.rc wincompat/manifest.xml
	$(WINDRES) -DVERSION_STR=$(VERSION) -O coff -c 65001 -i $< -o $@
endif

wg: $(patsubst %.c,%.o,$(wildcard *.c))

ifneq ($(V),1)
clean:
	@echo "  CLEAN   $$(pwd)/{wg,*.o,*.d}"
	@$(RM) wg *.o *.d
else
clean:
	$(RM) wg *.o *.d
endif

install: wg
	@install -v -d "$(DESTDIR)$(BINDIR)" && install -m 0755 -v wg "$(DESTDIR)$(BINDIR)/wg"
	@install -v -d "$(DESTDIR)$(MANDIR)/man8" && install -m 0644 -v man/wg.8 "$(DESTDIR)$(MANDIR)/man8/wg.8"
	@[ "$(WITH_BASHCOMPLETION)" = "yes" ] || exit 0; \
	install -v -d "$(DESTDIR)$(BASHCOMPDIR)" && install -m 0644 -v completion/wg.bash-completion "$(DESTDIR)$(BASHCOMPDIR)/wg"
	@[ "$(WITH_WGQUICK)" = "yes" ] || exit 0; \
	install -m 0755 -v wg-quick/$(PLATFORM).bash "$(DESTDIR)$(BINDIR)/wg-quick" && install -m 0700 -v -d "$(DESTDIR)$(SYSCONFDIR)/wireguard"
	@[ "$(WITH_WGQUICK)" = "yes" ] || exit 0; \
	install -m 0644 -v man/wg-quick.8 "$(DESTDIR)$(MANDIR)/man8/wg-quick.8"
	@[ "$(WITH_WGQUICK)" = "yes" -a "$(WITH_BASHCOMPLETION)" = "yes" ] || exit 0; \
	install -m 0644 -v completion/wg-quick.bash-completion "$(DESTDIR)$(BASHCOMPDIR)/wg-quick"
	@[ "$(WITH_WGQUICK)" = "yes" -a "$(WITH_SYSTEMDUNITS)" = "yes" ] || exit 0; \
	install -v -d "$(DESTDIR)$(SYSTEMDUNITDIR)" && install -m 0644 -v systemd/wg-quick@.service "$(DESTDIR)$(SYSTEMDUNITDIR)/wg-quick@.service"

help:
	@cat INSTALL

.PHONY: clean install help

-include *.d
