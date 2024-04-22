#
# Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer,
#    without modification.
# 2. Redistributions in binary form must reproduce at minimum a disclaimer
#    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
#    redistribution must be conditioned upon including a substantially
#    similar Disclaimer requirement for further binary redistribution.
# 3. Neither the names of the above-listed copyright holders nor the names
#    of any contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# Alternatively, this software may be distributed under the terms of the
# GNU General Public License ("GPL") version 2 as published by the Free
# Software Foundation.
#
# NO WARRANTY
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES.
#
# $Id: //depot/sw/linuxsrc/src/802_11/madwifi/madwifi/Makefile $
#

#
# Makefile for the HAL-based Atheros driver.
#
COPTS=
obj := $(firstword $(obj) $(SUBDIRS) .)
export TOP:=$(if $(wildcard $(firstword $(SUBDIRS))/Makefile.inc),$(firstword $(SUBDIRS)),$(CURDIR))
#export NOSTDINC_FLAGS := 

ifneq (svnversion.h,$(MAKECMDGOALS))
include $(TOP)/Makefile.inc
endif

obj-y := ath/ ath_hal/ net80211/

all: modules tools

modules: configcheck svnversion.h
	make -C hal/linux -f Makefile.inc 
	make -C hal/linux -f Makefile.inc release
ifdef LINUX24
	for i in $(obj-y); do \
		$(MAKE) -C $$i TOP="$(TOP)" || exit 1; \
	done
else
ifeq ($(ARCH),armeb)
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) ARCH=arm modules CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-"
else
ifeq ($(ARCH),arm64)
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) ARCH=arm64 modules CROSS_COMPILE="ccache aarch64-linux-uclibc-"
else
ifeq ($(ARCH),mips64)
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) ARCH=mips modules CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-"
else
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-"
endif
endif
endif
endif

.PHONY: svnversion.h
svnversion.h:
	@if [ -d .svn ]; then \
		ver=$$(svnversion -nc . | sed -e 's/^[^:]*://;s/[A-Za-z]//'); \
		echo "#define SVNVERSION \"svn r$$ver\"" > $@.tmp; \
	elif [ -d .git ]; then \
		ver=$$(git svn log | head -n2 | tail -n1 | cut -d\  -f1); \
		echo "#define SVNVERSION \"svn $$ver\"" > $@.tmp; \
	elif [ -s SNAPSHOT ]; then \
		ver=$$(sed -e '/^Revision: */!d;s///;q' SNAPSHOT); \
		echo "#define SVNVERSION \"svn r$$ver\"" > $@.tmp; \
	else \
		touch $@.tmp; \
	fi || exit 1; \
	diff $@ $@.tmp >/dev/null 2>&1 || cp -f $@.tmp $@; rm -f $@.tmp

# conflicts with the 'tools' subdirectory
.PHONY: tools
tools:
	$(MAKE) -C $(TOOLS) all || exit 1

install: install-modules install-tools

install-modules: modules
	@# check if there are modules left from an old installation
	@# might cause make to abort the build
	sh scripts/find-madwifi-modules.sh -r $(KERNELRELEASE) $(DESTDIR)

	for i in $(obj-y); do \
		$(MAKE) -C $$i install TOP="$(TOP)" || exit 1; \
	done
ifeq ($(DESTDIR),)
	(export KMODPATH=$(KMODPATH); /sbin/depmod -ae $(KERNELRELEASE))
endif

install-tools: tools
	$(MAKE) -C $(TOOLS) install || exit 1

uninstall: uninstall-tools uninstall-modules
uninstall-modules:
	sh scripts/find-madwifi-modules.sh -r $(KERNELRELEASE) $(DESTDIR)

list-modules: find-modules
find-modules:
	sh scripts/find-madwifi-modules.sh -l $(KERNELRELEASE)

uninstall-tools:
	$(MAKE) -C $(TOOLS) uninstall

reinstall: uninstall install
reinstall-tools: uninstall-tools install-tools
reinstall-modules: uninstall-modules install-modules

clean:
	-make -C hal/linux -f Makefile.inc clean
	-find $(obj-y) -name '*~' \
		-or -name '*.o' \
		-or -name '*.o.d' \
		-or -name '*.o.cmd' \
		-or -name '*.o.flags' \
		-or -name '*.ko' \
		-or -name '*.ko.cmd' \
		-or -name '*.mod.c' \
		-or -name '.depend' \
		-or -name '.version' \
		-or -name '.symvers' | \
		xargs -r rm -f
	-rm -f *.symvers svnversion.h
	-rm -rf .tmp_versions
	-make -C tools clean

info:
	@echo "The following settings will be used for compilation:"
	@echo "TARGET       : $(TARGET)"
	@echo "ARCH         : $(ARCH)"
	@echo "BUS          : $(BUS)"
	@if [ -n "$(TOOLPATH)" ]; then \
	    @echo "TOOLPATH     : $(TOOLPATH)"; \
	fi	
	@echo "KERNELRELEASE: $(KERNELRELEASE)"
	@echo "KERNELPATH   : $(KERNELPATH)"
	@echo "KERNELCONF   : $(KERNELCONF)"
	@echo "KMODPATH     : $(KMODPATH)"
	@echo "KMODSUF      : $(KMODSUF)"

.PHONY: release
release:
	sh scripts/make-release.bash

.PHONY: unload 
unload:
	bash scripts/madwifi-unload

configcheck:
	@echo -n "Checking kernel configuration... "
	
	@# check version of kernel
#	@echo $(KERNELRELEASE) | grep -q -i '^[2-9]\.[0-9][0-9]\.' || { \
#	    echo "FAILED"; \
#	    echo "Only kernel versions 2.4.x and above are supported."; \
#	    echo "You have $(KERNELRELEASE)."; \
#	    exit 1; \
#	}
	
	@# check kernel configuration
	@if [ -z "$(CONFIG_SYSCTL)" ]; then \
	    echo "FAILED"; \
	    echo "Please enable sysctl support."; \
	    exit 1; \
	fi
	
ifeq ($(strip $(BUS)),PCI)
	@# check PCI support
	@if [ -z "$(CONFIG_PCI)" ]; then \
	    echo "FAILED"; \
	    echo "Please enable PCI support."; \
	    exit 1; \
	fi
endif
	
#	@# check wireless extensions support is enabled
#	@if [ -z "$(CONFIG_NET_RADIO)$(CONFIG_WIRELESS_EXT)" ]; then \
#	    echo "FAILED"; \
#	    echo "Please enable wireless extensions."; \
#	    exit 1; \
#	fi
	
	@echo "ok."
