# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020-2023 OpenVPN, Inc.
#
#  Author:	Antonio Quartulli <antonio@openvpn.net>

PWD:=$(shell pwd)
KERNEL_SRC ?= $(LINUXDIR)
ifeq ($(shell cd $(KERNEL_SRC) && pwd),)
$(warning $(KERNEL_SRC) is missing, please set KERNEL_SRC)
endif

KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched

export KERNEL_SRC
RM ?= rm -f
CP := cp -fpR
LN := ln -sf
DEPMOD := depmod -a

REVISION= $(shell	if [ -d "$(PWD)/.git" ]; then \
				echo $$(git --git-dir="$(PWD)/.git" describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)

ELFLAG := $(EL8FLAG) $(EL9FLAG)

NOSTDINC_FLAGS += \
	-I$(PWD)/include/ \
	-include $(PWD)/linux-compat.h \
	-I$(PWD)/compat-include/

ifneq ($(REVISION),)
NOSTDINC_FLAGS += -DOVPN_DCO_VERSION=\"$(REVISION)\"
endif

ifeq ($(DEBUG),1)
NOSTDINC_FLAGS += -DDEBUG=1
endif

obj-y += drivers/net/ovpn-dco/
export ovpn-dco-v2-y

BUILD_FLAGS := \
	M=$(PWD) \
	PWD=$(PWD) \
	REVISION=$(REVISION) \
	CONFIG_OVPN_DCO_V2=m

all: config
	$(MAKE) -C $(KERNEL_SRC) $(BUILD_FLAGS)	modules

clean:
	$(RM) psk_client
	$(RM) compat-autoconf.h*
	$(MAKE) -C $(KERNEL_SRC) $(BUILD_FLAGS) clean
	$(MAKE) -C tests clean

install: config
	install -v -m 644 -D drivers/net/ovpn-dco/ovpn-dco-v2.ko $(INSTALLDIR)/openvpn/lib/modules/$(KERNEL_VERSION)/ovpn-dco-v2.ko

config:
	$(PWD)/gen-compat-autoconf.sh $(PWD)/compat-autoconf.h

tests:
	$(MAKE) -C tests

.PHONY: all clean install config tests 
