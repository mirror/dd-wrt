# GPL LICENSE SUMMARY
# 
#   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify 
#   it under the terms of version 2 of the GNU General Public License as
#   published by the Free Software Foundation.
# 
#   This program is distributed in the hope that it will be useful, but 
#   WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License 
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#   The full GNU General Public License is included in this distribution 
#   in the file called LICENSE.GPL.
# 
#   Contact Information:
#   Intel Corporation
# 
#  version: Security.L.1.0.3-98

###########################################################################
# Driver files

# core driver files
CFILES = iegbe_main.c iegbe_hw.c iegbe_param.c \
         iegbe_oem_phy.c iegbe_ethtool.c kcompat.c
HFILES = iegbe.h iegbe_hw.h iegbe_osdep.h \
         iegbe_oem_phy.h kcompat.h

MDIO_PHONY_CFILES = gcu.c
MDIO_CFILES = gcu_main.c gcu_if.c
MDIO_HFILES = gcu.h gcu_if.h gcu_reg.h kcompat.h

KVER=$(shell uname -r)

#
# Variables:
#           KSRC (path to kernel source to build against)
#           KOBJ (path to modules of installed kernels)
#
#           KOBJ == KSRC if building against a uninstalled kernel
#
# are assigned by upper level makefiles and passed to this
# makefile. This makefile can be run outside of ICP's build
# environment if one sets KSRC and KOBJ on the make command line
# or set as environment variables
#

# set KSRC, KOBJ, and EXTERNAL_MDIO to default values of not already set
#
KOBJ=/usr/src/kernels/linux
KSRC=/usr/src/kernels/linux
KSRC=$(KOBJ)
EXTERNAL_MDIO ?= 1
GBE_NAME = iegbe
GCU_NAME = gcu

VERSION_FILE := $(KSRC)/include/linux/version.h
UTS_REL_FILE := $(KSRC)/include/linux/utsrelease.h
CONFIG_FILE  := $(KSRC)/include/linux/autoconf.h

ifeq (,$(wildcard $(VERSION_FILE)))
  $(error Linux kernel source not configured - missing version.h)
endif

ifeq (,$(wildcard $(CONFIG_FILE)))
  $(error Linux kernel source not configured - missing autoconf.h)
endif

ifeq (,$(wildcard $(UTS_REL_FILE)))
  $(error Linux kernel source not configured - missing utsrelease.h)
endif

# set the install path
INSTDIR := /lib/modules/$(KVER)/kernel/drivers/net/iegbe

# look for SMP in config.h
SMP := $(shell $(CC) $(CFLAGS) -E -dM $(CONFIG_FILE) | \
         grep CONFIG_SMP | awk '{ print $$3 }')
ifneq ($(SMP),1)
  SMP := 0
endif

ifneq ($(SMP),$(shell uname -a | grep SMP > /dev/null 2>&1 && echo 1 || echo 0))
  $(warning ***)
  ifeq ($(SMP),1)
    $(warning *** Warning: kernel source configuration (SMP))
    $(warning *** does not match running kernel (UP))
  else
    $(warning *** Warning: kernel source configuration (UP))
    $(warning *** does not match running kernel (SMP))
  endif
  $(warning *** Continuing with build,)
  $(warning *** resulting driver may not be what you want)
  $(warning ***)
endif

ifeq ($(SMP),1)
  EXTRA_CFLAGS += -D__SMP__
endif

ifeq ($(EXTERNAL_MDIO), 1)
  EXTRA_CFLAGS += -DEXTERNAL_MDIO
endif

###########################################################################
# 2.6.18 Specific rules

# Makefile for 2.6.18 kernel
TARGET = iegbe.ko
MDIO_TARGET = gcu.ko

FINAL_OUTPUT_DIR=build/linux_2.6/kernel_space/

# man page
MANSECTION = 7
MANFILE = $(TARGET:.ko=.$(MANSECTION))

ifneq ($(PATCHLEVEL),)
  obj-m += $(TARGET:.ko=.o)
  iegbe-objs := $(CFILES:.c=.o)
  ifeq ($(EXTERNAL_MDIO),1)
    obj-m += $(MDIO_TARGET:.ko=.o) 
    gcu-objs := $(MDIO_CFILES:.c=.o)
  endif
else
  ifneq ($(KOBJ),$(KSRC))
    KOBJ_PARAM := O=$(KOBJ)
  endif
default: 
	test -d $(FINAL_OUTPUT_DIR) || mkdir -p $(FINAL_OUTPUT_DIR);
	make -C $(KSRC) $(KOBJ_PARAM) SUBDIRS=$(shell pwd) modules
	@echo 'Copying outputs';\
	mv -f *.o $(FINAL_OUTPUT_DIR);\
	mv -f *.ko $(FINAL_OUTPUT_DIR);
endif


ifeq (,$(MANDIR))
  # find the best place to install the man page
  MANPATH := $(shell (manpath 2>/dev/null || echo $MANPATH) | sed 's/:/ /g')
  ifneq (,$(MANPATH))
    # test based on inclusion in MANPATH
    test_dir = $(findstring $(dir), $(MANPATH))
  else
    # no MANPATH, test based on directory existence
    test_dir = $(shell [ -e $(dir) ] && echo $(dir))
  endif
  # our preferred install path
  # should /usr/local/man be in here ?
  MANDIR := /usr/share/man /usr/man
  MANDIR := $(foreach dir, $(MANDIR), $(test_dir))
  MANDIR := $(firstword $(MANDIR))
endif
ifeq (,$(MANDIR))
  # fallback to /usr/man
  MANDIR := /usr/man
endif

# depmod version for rpm builds
DEPVER := $(shell /sbin/depmod -V 2>/dev/null | \
          awk 'BEGIN {FS="."} NR==1 {print $$2}')

###########################################################################
# Build rules

$(MANFILE).gz: ./$(MANFILE)
	gzip -c $< > $@

install: default $(MANFILE).gz
	# remove all old versions of the driver
	find $(INSTALL_MOD_PATH)/lib/modules/$(KVER) -name $(TARGET) -exec rm -f {} \; || true
	find $(INSTALL_MOD_PATH)/lib/modules/$(KVER) -name $(TARGET).gz -exec rm -f {} \; || true
	install -D -m 644 $(FINAL_OUTPUT_DIR)$(TARGET) $(INSTALL_MOD_PATH)$(INSTDIR)/$(TARGET)
ifeq ($(EXTERNAL_MDIO),1)
	find $(INSTALL_MOD_PATH)/lib/modules/$(KVER) -name $(MDIO_TARGET) -exec rm -f {} \; || true
	find $(INSTALL_MOD_PATH)/lib/modules/$(KVER) -name $(MDIO_TARGET).gz -exec rm -f {} \; || true
	install -D -m 644 $(FINAL_OUTPUT_DIR)$(MDIO_TARGET) $(INSTALL_MOD_PATH)$(INSTDIR)/$(MDIO_TARGET)
endif
ifeq (,$(INSTALL_MOD_PATH))
	/sbin/depmod -a || true
	/sbin/modprobe $(GBE_NAME)

else
  ifeq ($(DEPVER),1 )
	/sbin/depmod -r $(INSTALL_MOD_PATH) -a || true
	/sbin/modprobe $(GBE_NAME)

  else
	/sbin/depmod -b $(INSTALL_MOD_PATH) -a -n > /dev/null || true
	/sbin/modprobe $(GBE_NAME)
  endif
endif
	install -D -m 644 $(MANFILE).gz $(INSTALL_MOD_PATH)$(MANDIR)/man$(MANSECTION)/$(MANFILE).gz
	man -c -P'cat > /dev/null' $(MANFILE:.$(MANSECTION)=) || true

uninstall:
	if [ -e $(INSTDIR)/$(TARGET) ] ; then \
	    rm -f $(INSTDIR)/$(MDIO_TARGET) ; \
	    rm -f $(INSTDIR)/$(TARGET) ; \
	    rmmod $(GBE_NAME); \
	    rmmod $(GCU_NAME); \
	fi
	/sbin/depmod -a
	if [ -e $(MANDIR)/man$(MANSECTION)/$(MANFILE).gz ] ; then \
		rm -f $(MANDIR)/man$(MANSECTION)/$(MANFILE).gz ; \
	fi

.PHONY: clean install

clean:
	rm -rf $(FINAL_OUTPUT_DIR) \
	$(MANFILE).gz .*cmd .tmp_versions *.mod.c
 
