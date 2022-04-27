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
# $Id: //depot/sw/linuxsrc/src/802_11/madwifi/madwifi/Makefile.inc#11 $
#

# other locales sometimes cause trouble
export LC_ALL = POSIX

# BUS defines the bus type to which the wireless devices is attached.
# Currently, the valid BUS types are PCI and AHB.  If BUS is not
# defined, then, we assume the bus type is PCI
ifndef BUS
BUS=		PCI
endif

# DESTDIR is used as path prefix during installation.
DESTDIR ?=

# KERNELPATH is the path to the Linux kernel build tree.  Unless a
# separate build directory was used for the kernel build, it's the same
# as the kernel source tree.  KERNELPATH is used to access the kernel
# configuration, include files and the build system.  To build for
# another kernel, set KERNELPATH manually, for example with:
# make KERNELPATH=/path/to/kernel/source

# The default KERNELPATH points to the directory where the currently
# running kernel was compiled.  Note that the configuration and the
# version of the kernel tree might have changed since then.
ifeq ($(wildcard $(KERNELPATH)),)
KERNELPATH = /lib/modules/$(shell uname -r)/build
# sanity check: does KERNELPATH exist?
ifeq ($(shell cd $(KERNELPATH) && pwd),)
$(error $(KERNELPATH) is missing, please set KERNELPATH)
endif
export KERNELPATH
endif

# KERNELRELEASE is the target kernel's version.  It's always taken from
# the kernel build tree.  Kernel Makefile doesn't always know the exact
# kernel version (especially for vendor stock kernels), so we get it
# from <linux/version.h> instead. But simply grepping it from version.h
# doesn't work, since some distributions have multiple UTS_RELEASE
# in that file.
# This trick has been inspired by the lm_sensors project.
ifndef KERNELRELEASE
KERNELRELEASE := $(shell $(CC) -I $(KERNELPATH)/include -E $(TOP)/kernelversion.c | grep uts_release | cut -f2 -d'"')
#ifeq (,$(KERNELRELEASE))
#$(error Cannot detect kernel version - please check compiler and KERNELPATH)
#endif
endif

# KERNELCONF is the name of the file that holds the configuration
# of the target kernel.
KERNELCONF ?=	$(KERNELPATH)/.config

# sanity check: does KERNELCONF exist?
ifeq ($(wildcard $(KERNELCONF)),)
$(error KERNELCONF: $(KERNELCONF) does not exist.)
endif
include $(KERNELCONF)

# Determine architecture of the kernel.
#include $(TOP)/scripts/get_arch.mk
#export ARCH

# Determine TARGET
include $(TOP)/ath_hal/ah_target.inc
export TARGET
COPTS += -DTARGET='"$(TARGET)"'

# KMODPATH nominates the directory where the modules will be
# installed to
KMODPATH :=	/lib/modules/$(KERNELRELEASE)/net

# Recognize 2.4.x kernels to support the old build system
ifeq ($(filter-out 2.4%,$(KERNELRELEASE)),)
LINUX24 = y
endif

ifndef LINUX24
KMODSUF		:= ko
else
export-objs	= $(foreach m, $(obj-m), $($(m:.o=-objs)))
list-multi	= $(obj-m)
KMODSUF		:= o
endif

NM=		nm

#
# Path to the HAL source code.
#
ifeq ($(HAL),)
HAL=	$(TOP)/hal
endif
#
# Path to HAL/OS interface code
#
ATH_HAL= $(TOP)/ath_hal
#
# Path to the 802.11 include files.
#
WLAN=	$(TOP)/net80211
#
# Path to the Atheros device driver.
#
ATH=	$(TOP)/ath
#
# Path to the rate control algorithms.
#
ATH_RATE=	minstrel
#
# Path to the userspace utilities. 
# 
TOOLS=  $(TOP)/tools 

WARNINGS = 
DEBUG =
COPTS+= $(WARNINGS) $(DEBUG)
INCS=	-include $(TOP)/include/compat.h -I$(TOP)/include

# TARGET defines the target platform architecture. It must match one of
# the target platforms supported by the HAL. The default target is the
# host machine architecture. You can override TARGET on the make command
# line or in the environment. See hal/linux/*.inc for the list of
# supported targets.
ifeq (,$(wildcard $(HAL)/public/$(TARGET).inc))
TARGETS=$(basename $(notdir $(wildcard $(HAL)/public/*.inc)))
$(error TARGET $(TARGET) is invalid, valid targets are: $(TARGETS))
endif

# HAL_DEBUG enables HAL debugging code
# At the moment this consists of wrappers around HAL functions so that 
# stack traces are more decipherable.

ifdef HAL_DEBUG
COPTS+= -fno-inline
endif

# Don't let HAL mess with the toolchain - save toolchain variables
save_CC := $(CC)
save_LD := $(LD)
save_STRIP := $(STRIP)
save_OBJCOPY := $(OBJCOPY)
save_NM := $(NM)

include $(HAL)/public/$(TARGET).inc

# Restore toolchain variables
CC := $(save_CC)
LD := $(save_LD)
STRIP := $(save_STRIP)
OBJCOPY := $(save_OBJCOPY)
NM := $(save_NM)

include $(TOP)/BuildCaps.inc

# Ensure correct endianess
LDFLAGS += $(LDOPTS)

# Filter out compiler options that are not supported by all compilers
# and that are not needed to ensure compatible architecture and calling
# conventions.
COPTS := $(filter-out -mshort-load-bytes -mapcs-32,$(COPTS))


# The following variables will affect developers only, and are used in the
# make-release.bash script.
#
# RELEASE_TMP is used as a temporary store for the files needed to create the
# release tarball. Packaging will take place in TMPDIR/madwifi-release.
# Defaults to /tmp.
RELEASE_TMP=/tmp

# RELEASE_STORE is the (local) directory where the release tarball should be
# stored in. Defaults to the parent directory of the working copy used to
# create the release.
RELEASE_STORE=..

# expose these settings to make-release.bash
export RELEASE_TMP RELEASE_STORE
