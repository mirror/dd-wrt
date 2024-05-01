################################################################################
#
# Copyright (c) 2012, 2015, 2020 The Linux Foundation. All rights reserved.
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
################################################################################

# Target module name
TARGET := qca-mcs

# Kernel directory - This works only for the host.
# The target kernel is different, and it can't be detected.
ifndef KERNELPATH
KERNELPATH := /lib/modules/$(shell uname -r)/build
endif

ccflags-y += -I$(CURDIR) \
	     -I$(LINUXDIR)/include/linux \
	     -I$(LINUXDIR)/include/asm \
	     -I$(LINUXDIR)/net/bridge \
	     -I$(KBUILDPATH)/include/linux \
	     -I$(KBUILDPATH)/include/asm \
	     -I$(KBUILDPATH)/net/bridge

ccflags-$(CONFIG_SUPPORT_MLD) += -DMC_SUPPORT_MLD

# Module extra compilation flags
ccflags-y += -Werror -Wall -g


ifneq ($(KERNELRELEASE),)
obj-m += $(TARGET).o

# Target objects
$(TARGET)-objs := \
	mc_main.o \
	mc_netlink.o \
	mc_netfilter.o \
	mc_snooping.o \
	mc_forward.o \
	mc_ecm.o

$(TARGET)-$(CONFIG_SUPPORT_OVS) += mc_ovs.o
ccflags-$(CONFIG_SUPPORT_OVS) += -I$(STAGING_DIR)/usr/include/qca-ovsmgr \
				 -DMC_SUPPORT_OVS

else

# Makefile targets

all: build install

build:
	@$(MAKE) -C $(LINUXDIR) M=`pwd` modules MDIR=$(CURDIR)

install:
	@install -m 644 $(TARGET).ko $(MODULEPATH)

uninstall:
	@find $(MODULEPATH) -name $(TARGET).ko | xargs rm -rf

clean:
	@$(MAKE) -C $(LINUXDIR) M=`pwd` clean
	@rm -f Module.symvers

endif
