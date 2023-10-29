# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020-2023 OpenVPN, Inc.
#
#  Author:	Antonio Quartulli <antonio@openvpn.net>

PWD:=$(shell pwd)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
ifeq ($(shell cd $(KERNEL_SRC) && pwd),)
$(warning $(KERNEL_SRC) is missing, please set KERNEL_SRC)
endif

export KERNEL_SRC
RM ?= rm -f
CP := cp -fpR
LN := ln -sf
DEPMOD := depmod -a

REVISION= $(shell	if [ -d "$(PWD)/.git" ]; then \
				echo $$(git --git-dir="$(PWD)/.git" describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)

EL8 := $(shell cat /etc/redhat-release 2>/dev/null | grep -c " 8." )
ifeq (1, $(EL8))
EL8FLAG := -DEL8
endif

EL9 := $(shell cat /etc/redhat-release 2>/dev/null | grep -c " 9." )
ifeq (1, $(EL9))
EL9FLAG := -DEL9
endif

ELFLAG := $(EL8FLAG) $(EL9FLAG)

NOSTDINC_FLAGS += \
	-I$(PWD)/include/ \
	$(CFLAGS) $(ELFLAG) \
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
	CONFIG_OVPN_DCO_V2=m \
	INSTALL_MOD_DIR=updates/

all: config
	$(MAKE) -C $(KERNEL_SRC) $(BUILD_FLAGS)	modules

clean:
	$(RM) psk_client
	$(RM) compat-autoconf.h*
	$(MAKE) -C $(KERNEL_SRC) $(BUILD_FLAGS) clean
	$(MAKE) -C tests clean

install: config
	$(MAKE) -C $(KERNEL_SRC) $(BUILD_FLAGS) modules_install
	$(DEPMOD)

config:
	$(PWD)/gen-compat-autoconf.sh $(PWD)/compat-autoconf.h

tests:
	$(MAKE) -C tests

.PHONY: all clean install config tests 
