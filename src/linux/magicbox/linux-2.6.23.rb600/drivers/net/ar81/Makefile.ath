###########################################################################
# Copyright(c) 2007 Atheros Corporation. All rights reserved.
# Copyright(c) 2006 xiong huang <xiong.huang@atheros.com>
#
# Derived from Intel e1000 driver
# Copyright(c) 1999 - 2005 Intel Corporation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
###########################################################################
# Driver files

# core driver files
CFILES = at_common_main.c atl1e_main.c atl1c_main.c atl1c_hw.c atl1e_hw.c atl1e_param.c \
	 atl1c_param.c atl1e_ethtool.c atl1c_ethtool.c kcompat.c\

HFILES = at_common.h atl1e.h atl1c.h atl1e_hw.h atl1c_hw.h at_osdep.h kcompat.h

ifeq (,$(BUILD_KERNEL))
BUILD_KERNEL=$(shell uname -r)
endif

DRIVER_NAME = atl1e
###########################################################################
# Environment tests

# Kernel Search Path
# All the places we look for kernel source
KSP :=  /lib/modules/$(BUILD_KERNEL)/build \
        /lib/modules/$(BUILD_KERNEL)/source \
        /usr/src/linux-$(BUILD_KERNEL) \
        /usr/src/linux-$($(BUILD_KERNEL) | sed 's/-.*//') \
        /usr/src/kernel-headers-$(BUILD_KERNEL) \
        /usr/src/kernel-source-$(BUILD_KERNEL) \
        /usr/src/linux-$($(BUILD_KERNEL) | sed 's/\([0-9]*\.[0-9]*\)\..*/\1/') \
        /usr/src/linux

# prune the list down to only values that exist
# and have an include/linux sub-directory
test_dir = $(shell [ -e $(dir)/include/linux ] && echo $(dir))
KSP := $(foreach dir, $(KSP), $(test_dir))

# we will use this first valid entry in the search path
ifeq (,$(KSRC))
  KSRC := $(firstword $(KSP))
endif

ifeq (,$(KSRC))
  $(error Linux kernel source not found)
else
ifeq (/lib/modules/$(shell uname -r)/source, $(KSRC))
  KOBJ :=  /lib/modules/$(shell uname -r)/build
else
  KOBJ :=  $(KSRC)
endif
endif

# check for version.h and autoconf.h for running kernel in /boot (SUSE)
ifneq (,$(wildcard /boot/vmlinuz.version.h))
  VERSION_FILE := /boot/vmlinuz.version.h
  CONFIG_FILE  := /boot/vmlinuz.autoconf.h
  KVER := $(shell $(CC) $(EXTRA_CFLAGS) -E -dM $(VERSION_FILE) | \
          grep UTS_RELEASE | awk '{ print $$3 }' | sed 's/\"//g')
  ifeq ($(KVER),$(shell uname -r))
    # set up include path to override headers from kernel source
    x:=$(shell rm -rf include)
    x:=$(shell mkdir -p include/linux)
    x:=$(shell cp /boot/vmlinuz.version.h include/linux/version.h)
    x:=$(shell cp /boot/vmlinuz.autoconf.h include/linux/autoconf.h)
    EXTRA_CFLAGS += -I./include
  else
    ifneq (,$(wildcard $(KOBJ)/include/linux/utsrelease.h))
      VERSION_FILE := $(KOBJ)/include/linux/utsrelease.h
    else
      VERSION_FILE := $(KOBJ)/include/linux/version.h
    endif
    CONFIG_FILE  := $(KSRC)/include/linux/autoconf.h
  endif
else
  ifneq (,$(wildcard $(KOBJ)/include/linux/utsrelease.h))
    VERSION_FILE := $(KOBJ)/include/linux/utsrelease.h
  else
    VERSION_FILE := $(KOBJ)/include/linux/version.h
  endif
  CONFIG_FILE  := $(KSRC)/include/linux/autoconf.h
endif

ifeq (,$(wildcard $(VERSION_FILE)))
  $(error Linux kernel source not configured - missing version.h)
endif

ifeq (,$(wildcard $(CONFIG_FILE)))
  $(error Linux kernel source not configured - missing autoconf.h)
endif

# pick a compiler
ifneq (,$(findstring egcs-2.91.66, $(shell cat /proc/version)))
  CC := kgcc gcc cc
else
  CC := gcc cc
endif
test_cc = $(shell $(cc) --version > /dev/null 2>&1 && echo $(cc))
CC := $(foreach cc, $(CC), $(test_cc))
CC := $(firstword $(CC))
ifeq (,$(CC))
  $(error Compiler not found)
endif

# we need to know what platform the driver is being built on
# some additional features are only built on Intel platforms
ARCH := $(shell uname -m | sed 's/i.86/i386/')
ifeq ($(ARCH),alpha)
  EXTRA_CFLAGS += -ffixed-8 -mno-fp-regs
endif
ifeq ($(ARCH),x86_64)
  EXTRA_CFLAGS += -mcmodel=kernel -mno-red-zone
endif
ifeq ($(ARCH),ppc)
  EXTRA_CFLAGS += -msoft-float
endif
ifeq ($(ARCH),ppc64)
  EXTRA_CFLAGS += -m64 -msoft-float
  LDFLAGS += -melf64ppc
endif

# extra flags for module builds
EXTRA_CFLAGS += -DDRIVER_$(shell echo $(DRIVER_NAME) | tr '[a-z]' '[A-Z]')
EXTRA_CFLAGS += -DDRIVER_NAME=$(DRIVER_NAME)
EXTRA_CFLAGS += -DDRIVER_NAME_CAPS=$(shell echo $(DRIVER_NAME) | tr '[a-z]' '[A-Z]')
# standard flags for module builds
EXTRA_CFLAGS += -DLINUX -D__KERNEL__ -DMODULE -O2 -pipe -Wall -DDBG=0
EXTRA_CFLAGS += -I$(KSRC)/include -I.
EXTRA_CFLAGS += $(shell [ -f $(KSRC)/include/linux/modversions.h ] && \
            echo "-DMODVERSIONS -DEXPORT_SYMTAB \
                  -include $(KSRC)/include/linux/modversions.h")

EXTRA_CFLAGS += $(CFLAGS_EXTRA)

RHC := $(KSRC)/include/linux/rhconfig.h
ifneq (,$(wildcard $(RHC)))
  # 7.3 typo in rhconfig.h
  ifneq (,$(shell $(CC) $(EXTRA_CFLAGS) -E -dM $(RHC) | grep __module__bigmem))
	EXTRA_CFLAGS += -D__module_bigmem
  endif
endif

# get the kernel version - we use this to find the correct install path
KVER := $(shell $(CC) $(EXTRA_CFLAGS) -E -dM $(VERSION_FILE) | grep UTS_RELEASE | \
        awk '{ print $$3 }' | sed 's/\"//g')

# assume source symlink is the same as build, otherwise adjust KOBJ
ifneq (,$(wildcard /lib/modules/$(KVER)/build))
ifneq ($(KSRC),$(shell cd /lib/modules/$(KVER)/build ; pwd -P))
  KOBJ=/lib/modules/$(KVER)/build
endif
endif

KKVER := $(shell echo $(KVER) | \
         awk '{ if ($$0 ~ /2\.[4-9]\./) print "1"; else print "0"}')
ifeq ($(KKVER), 0)
  $(error *** Aborting the build. \
          *** This driver is not supported on kernel versions older than 2.4.0)
endif

# set the install path
INSTDIR := /lib/modules/$(KVER)/kernel/drivers/net/$(DRIVER_NAME)

# look for SMP in config.h
SMP := $(shell $(CC) $(EXTRA_CFLAGS) -E -dM $(CONFIG_FILE) | \
         grep -w CONFIG_SMP | awk '{ print $$3 }')
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

###########################################################################
# 2.4.x & 2.6.x Specific rules

K_VERSION:=$(shell uname -r | cut -c1-3 | sed 's/2\.[56]/2\.6/')

ifeq ($(K_VERSION), 2.6)

# Makefile for 2.6.x kernel
TARGET = $(DRIVER_NAME).ko

# man page
MANSECTION = 7
MANFILE = $(TARGET:.ko=.$(MANSECTION))

ifneq ($(PATCHLEVEL),)
EXTRA_CFLAGS += $(CFLAGS_EXTRA)
obj-m += $(DRIVER_NAME).o
$(DRIVER_NAME)-objs := $(CFILES:.c=.o)
else
default:
ifeq ($(KOBJ),$(KSRC))
	$(MAKE) -C $(KSRC) SUBDIRS=$(shell pwd) modules
else
	$(MAKE) -C $(KSRC) O=$(KOBJ) SUBDIRS=$(shell pwd) modules
endif
endif

else # ifeq ($(K_VERSION),2.6)

# Makefile for 2.4.x kernel
TARGET = $(DRIVER_NAME).o

# man page
MANSECTION = 7
MANFILE = $(TARGET:.o=.$(MANSECTION))

# Get rid of compile warnings in kernel header files from SuSE
ifneq (,$(wildcard /etc/SuSE-release))
  EXTRA_CFLAGS += -Wno-sign-compare -fno-strict-aliasing
endif

# Get rid of compile warnings in kernel header files from fedora
ifneq (,$(wildcard /etc/fedora-release))
  EXTRA_CFLAGS += -fno-strict-aliasing
endif
CFLAGS += $(EXTRA_CFLAGS)

.SILENT: $(TARGET)
$(TARGET): $(filter-out $(TARGET), $(CFILES:.c=.o))
	$(LD) $(LDFLAGS) -r $^ -o $@
	echo; echo
	echo "**************************************************"
	echo "** $(TARGET) built for $(KVER)"
	echo -n "** SMP               "
	if [ "$(SMP)" = "1" ]; \
		then echo "Enabled"; else echo "Disabled"; fi
	echo "**************************************************"
	echo

$(CFILES:.c=.o): $(HFILES) Makefile
default:
	$(MAKE)

endif # ifeq ($(K_VERSION),2.6)

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

$(MANFILE).gz: ../$(MANFILE)
	gzip -c $< > $@

install: default $(MANFILE).gz
	# remove all old versions of the driver
	find $(INSTALL_MOD_PATH)/lib/modules/$(KVER) -name $(TARGET) -exec rm -f {} \; || true
	find $(INSTALL_MOD_PATH)/lib/modules/$(KVER) -name $(TARGET).gz -exec rm -f {} \; || true
	install -D -m 644 $(TARGET) $(INSTALL_MOD_PATH)$(INSTDIR)/$(TARGET)
ifeq (,$(INSTALL_MOD_PATH))
	/sbin/depmod -a || true
else
  ifeq ($(DEPVER),1 )
	/sbin/depmod -r $(INSTALL_MOD_PATH) -a || true
  else
	/sbin/depmod -b $(INSTALL_MOD_PATH) -a -n $(KVERSION) > /dev/null || true
  endif
endif
	install -D -m 644 $(MANFILE).gz $(INSTALL_MOD_PATH)$(MANDIR)/man$(MANSECTION)/$(MANFILE).gz
	man -c -P'cat > /dev/null' $(MANFILE:.$(MANSECTION)=) || true

uninstall:
	if [ -e $(INSTDIR)/$(TARGET) ] ; then \
	    rm -f $(INSTDIR)/$(TARGET) ; \
	fi
	/sbin/depmod -a
	if [ -e $(MANDIR)/man$(MANSECTION)/$(MANFILE).gz ] ; then \
		rm -f $(MANDIR)/man$(MANSECTION)/$(MANFILE).gz ; \
	fi

.PHONY: clean install

clean:
	rm -rf $(TARGET) $(TARGET:.ko=.o) $(TARGET:.ko=.mod.c) $(TARGET:.ko=.mod.o) $(CFILES:.c=.o) $(MANFILE).gz .*cmd .tmp_versions
