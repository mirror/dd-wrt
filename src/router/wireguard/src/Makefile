# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

KERNELRELEASE ?= $(shell uname -r)
KERNELDIR ?= /lib/modules/$(KERNELRELEASE)/build
PREFIX ?= /usr
DESTDIR ?=
SRCDIR ?= $(PREFIX)/src
DKMSDIR ?= $(SRCDIR)/wireguard
DEPMOD ?= depmod

PWD := $(shell pwd)

all: module tools
debug: module-debug tools-debug

ifneq ($(V),1)
MAKEFLAGS += --no-print-directory
endif

WIREGUARD_VERSION = $(patsubst v%,%,$(shell GIT_CEILING_DIRECTORIES="$(PWD)/../.." git describe --dirty 2>/dev/null))

module:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules

module-debug:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) V=1 CONFIG_WIREGUARD_DEBUG=y WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules

clean:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	@$(MAKE) -C tools clean

module-install:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules_install
	$(DEPMOD) -a $(KERNELRELEASE)

install: module-install tools-install

DKMS_TOP_LEVEL := Makefile Kbuild Kconfig $(filter-out wireguard.mod.c, $(wildcard *.c)) $(filter-out version.h, $(wildcard *.h)) version.h dkms.conf
DKMS_SELFTEST_LEVEL := $(wildcard selftest/*.c) $(wildcard selftest/*.h)
DKMS_CRYPTO_LEVEL := $(wildcard crypto/*.c) $(wildcard crypto/*.h) $(wildcard crypto/*.S)
DKMS_UAPI_LEVEL := $(wildcard uapi/*.h)
DKMS_COMPAT_LEVEL := $(shell find compat/ -name '*.c' -o -name '*.h' -o -name '*.include')
dkms-install: $(DKMS_TOP_LEVEL) $(DKMS_SELFTEST_LEVEL) $(DKMS_CRYPTO_LEVEL) $(DKMS_UAPI_LEVEL) $(DKMS_COMPAT_LEVEL)
	@install -v -m0644 -D -t$(DESTDIR)$(DKMSDIR) $(DKMS_TOP_LEVEL)
	@install -v -m0644 -D -t$(DESTDIR)$(DKMSDIR)/selftest $(DKMS_SELFTEST_LEVEL)
	@install -v -m0644 -D -t$(DESTDIR)$(DKMSDIR)/crypto $(DKMS_CRYPTO_LEVEL)
	@install -v -m0644 -D -t$(DESTDIR)$(DKMSDIR)/uapi $(DKMS_UAPI_LEVEL)
	@for file in $(DKMS_COMPAT_LEVEL); do install -v -m0644 -D $$file $(DESTDIR)$(DKMSDIR)/$$file; done

tools:
	@$(MAKE) -C tools

tools-debug:
	@$(MAKE) -C tools V=1 DEBUG_TOOLS=y

tools-install:
	@$(MAKE) -C tools install

style:
	$(KERNELDIR)/scripts/checkpatch.pl -f --max-line-length=4000 --codespell --color=always $(filter-out wireguard.mod.c,$(wildcard *.c)) $(wildcard *.h) $(wildcard selftest/*.c)

check: clean
	scan-build --html-title=WireGuard -maxloop 100 --view --keep-going $(MAKE) module tools CONFIG_WIREGUARD_DEBUG=y C=2 CF="-D__CHECK_ENDIAN__"

coccicheck: clean
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_WIREGUARD_DEBUG=y coccicheck MODE=report

cloc:
	@cloc --skip-uniqueness --by-file --extract-with="$$(readlink -f ../contrib/kernel-tree/filter-compat-defines.sh) >FILE< > \$$(basename >FILE<)" $(filter-out wireguard.mod.c,$(wildcard *.c)) $(wildcard *.h)

-include tests/debug.mk

.PHONY: all module module-debug module-install tools tools-install install dkms-install clean core-cloc check style
