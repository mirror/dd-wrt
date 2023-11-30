# SPDX-License-Identifier: GPL-2.0
# Copyright (C) B.A.T.M.A.N. contributors:
#
# Marek Lindner, Simon Wunderlich

# read README.external for more information about the configuration
# B.A.T.M.A.N. V routing algorithm (experimental):
export CONFIG_BATMAN_ADV_BATMAN_V=y
# B.A.T.M.A.N. bridge loop avoidance:
export CONFIG_BATMAN_ADV_BLA=y
# B.A.T.M.A.N. distributed ARP table:
export CONFIG_BATMAN_ADV_DAT=y
# B.A.T.M.A.N. debugging:
export CONFIG_BATMAN_ADV_DEBUG=n
# B.A.T.M.A.N. multicast optimizations:
export CONFIG_BATMAN_ADV_MCAST=y
# B.A.T.M.A.N network coding (catwoman):
export CONFIG_BATMAN_ADV_NC=n
# B.A.T.M.A.N. tracing support:
export CONFIG_BATMAN_ADV_TRACING=n

PWD:=$(shell pwd)
KERNELPATH ?= /lib/modules/$(shell uname -r)/build
# sanity check: does KERNELPATH exist?
ifeq ($(shell cd $(KERNELPATH) && pwd),)
$(warning $(KERNELPATH) is missing, please set KERNELPATH)
endif

export KERNELPATH
RM ?= rm -f
CP := cp -fpR
LN := ln -sf
DEPMOD := depmod -a

REVISION= $(shell	if [ -d "$(PWD)/.git" ]; then \
				echo $$(git --git-dir="$(PWD)/.git" describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)
NOSTDINC_FLAGS += \
	-I$(PWD)/compat-include/ \
	-I$(PWD)/include/ \
	-include $(PWD)/compat.h \
	$(CFLAGS)

ifneq ($(REVISION),)
NOSTDINC_FLAGS += -DBATADV_SOURCE_VERSION=\"$(REVISION)\"
endif

obj-y += net/batman-adv/

export batman-adv-y


BUILD_FLAGS := \
	M=$(PWD) \
	PWD=$(PWD) \
	REVISION=$(REVISION) \
	CONFIG_BATMAN_ADV=m \
	CONFIG_BATMAN_ADV_DEBUG=$(CONFIG_BATMAN_ADV_DEBUG) \
	CONFIG_BATMAN_ADV_BLA=$(CONFIG_BATMAN_ADV_BLA) \
	CONFIG_BATMAN_ADV_DAT=$(CONFIG_BATMAN_ADV_DAT) \
	CONFIG_BATMAN_ADV_NC=$(CONFIG_BATMAN_ADV_NC) \
	CONFIG_BATMAN_ADV_MCAST=$(CONFIG_BATMAN_ADV_MCAST) \
	CONFIG_BATMAN_ADV_TRACING=$(CONFIG_BATMAN_ADV_TRACING) \
	CONFIG_BATMAN_ADV_BATMAN_V=$(CONFIG_BATMAN_ADV_BATMAN_V) \
	INSTALL_MOD_DIR=updates/

all: config
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS)	modules

clean:
	$(RM) compat-autoconf.h*
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS) clean

install: config
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS) modules_install
	$(DEPMOD)

config:
	$(PWD)/gen-compat-autoconf.sh $(PWD)/compat-autoconf.h

.PHONY: all clean install config
