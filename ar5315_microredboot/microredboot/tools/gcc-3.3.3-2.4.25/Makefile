# Makefile for to build gcc for uClibc
#
# Copyright (C) 2002-2003 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#############################################################
#
# EDIT this stuff to suit your system and preferences
#
# Use := when possible to get precomputation, thereby 
# speeding up the build process.
#
#############################################################

# What sortof target system shall we compile this for?
#ARCH:=i386
#ARCH:=arm
ARCH:=mips
#ARCH:=mipsel
# Possible issues remaining with powerpc and soft float.
#ARCH:=powerpc
#ARCH:=sh4
# Busybox link failing due to needing libgcc functions that are statics.
#ARCH:=cris

# The following currently fail to build since no shared lib support.
#ARCH:=sh64
#ARCH:=m68k
#ARCH:=v850
#ARCH:=sparc
#ARCH:=whatever

# Enable this to use the uClibc daily snapshot instead of a released
# version.  Daily snapshots may contain new features and bugfixes. Or
# they may not even compile at all, depending on what Erik is doing...
USE_UCLIBC_SNAPSHOT:=false

# Enable this to use the busybox daily snapshot instead of a released
# version.  Daily snapshots may contain new features and bugfixes. Or
# they may not even compile at all....
USE_BUSYBOX_SNAPSHOT:=true

# Enable large file (files > 2 GB) support
#BUILD_WITH_LARGEFILE:=true
BUILD_WITH_LARGEFILE:=false

# Command used to download source code
WGET:=wget --passive-ftp

# Soft floating point options.
# Notes:
#   Currently builds with gcc 3.3 for i386, arm, mips, mipsel.
#   Only tested with multilib enabled.
#   For i386, long double is the same as double (64 bits).  While this
#      is unusual for x86, it seemed the best approach considering the
#      limitations in the gcc floating point emulation library.
#   For arm, soft float uses the usual libfloat routines.
#   The uClibc built will support _only_ applications compiled with the
#      -msoft-float flag.  To avoid CFLAGS problems, you may want to use
#      scripts similar to those in the build*/staging_dir/bin directory.
# (Un)comment the appropriate line below.
#SOFT_FLOAT:=true
SOFT_FLOAT:=false

# Any additional gcc options you may want to include....
EXTRA_GCC_CONFIG_OPTIONS:=

# Enable the following if you want locale/gettext/i18n support.
# NOTE!  Currently the pregnerated locale stuff only works for x86!
#ENABLE_LOCALE:=true
ENABLE_LOCALE:=false

# If you want multilib enabled, enable this...
#MULTILIB:=--enable-multilib
MULTILIB:=

# Build/install c++ compiler and libstdc++?
INSTALL_LIBSTDCPP:=true

# For SMP machines some stuff can be run in parallel
#JLEVEL=-j3

#############################################################
#
# You should probably leave this stuff alone unless you know 
# what you are doing.
#
#############################################################
USE_UCLIBC_TOOLCHAIN:=true
TARGETS:=host-sed kernel-headers uclibc-configured binutils gcc3_3 ccache

# Optimize toolchain for which type of CPU?
# WARNING!!!  CURRENTLY BROKEN!!! LEAVE IT AS $(ARCH)!!!
OPTIMIZE_FOR_CPU=$(ARCH)

ifeq ($(SOFT_FLOAT),true)
SOFT_FLOAT_CONFIG_OPTION:=--without-float
TARGET_SOFT_FLOAT:=-msoft-float
ARCH_FPU_SUFFIX:=_nofpu
else
SOFT_FLOAT_CONFIG_OPTION:=
TARGET_SOFT_FLOAT:=
ARCH_FPU_SUFFIX:=
endif

ifeq ($(ENABLE_LOCALE),true)
EXTRA_GCC_CONFIG_OPTIONS += --enable-clocale=gnu
endif

# WARNING -- uClibc currently disables large file support on cris.
ifeq ("$(strip $(ARCH))","cris")
BUILD_WITH_LARGEFILE:=false
endif

ifneq ($(BUILD_WITH_LARGEFILE),true)
DISABLE_LARGEFILE= --disable-largefile 
endif
TARGET_OPTIMIZATION=-Os
TARGET_DEBUGGING= #-g
TARGET_CFLAGS=$(TARGET_OPTIMIZATION) $(TARGET_DEBUGGING) $(TARGET_SOFT_FLOAT)

ifeq ($(HOSTCC),)
HOSTCC:=gcc
endif
BASE_DIR:=${shell pwd}
SOURCE_DIR:=$(BASE_DIR)/sources
DL_DIR:=$(SOURCE_DIR)/dl
PATCH_DIR=$(SOURCE_DIR)/patches
BUILD_DIR=$(BASE_DIR)/toolchain_$(ARCH)$(ARCH_FPU_SUFFIX)
STAGING_DIR=$(BUILD_DIR)
TOOL_BUILD_DIR=$(BASE_DIR)/toolchain_build_$(ARCH)$(ARCH_FPU_SUFFIX)
TARGET_PATH=$(STAGING_DIR)/bin:/bin:/sbin:/usr/bin:/usr/sbin
#TARGET_CROSS=$(STAGING_DIR)/bin/$(ARCH)-uclibc-
TARGET_CROSS=$(STAGING_DIR)/bin/$(ARCH)-linux-
TARGET_CC=$(TARGET_CROSS)gcc$(TARGET_SOFT_FLOAT)
STRIP=$(TARGET_CROSS)strip --remove-section=.comment --remove-section=.note

ifeq ("$(strip $(ARCH))","mips")
EXTRA_STRIP_ARGS= --remove-section=.pdr
EXTRA_STRIP=$(TARGET_CROSS)strip $(EXTRA_STRIP_ARGS)
else
EXTRA_STRIP_ARGS=
EXTRA_STRIP=/bin/true
endif

#STRIP:=/bin/true
IMAGE:=$(BASE_DIR)/root_fs_$(ARCH)$(ARCH_FPU_SUFFIX)
GNU_TARGET_NAME=$(OPTIMIZE_FOR_CPU)-linux
#KERNEL_CROSS=$(STAGING_DIR)/bin/$(ARCH)-uclibc-
KERNEL_CROSS=$(STAGING_DIR)/bin/$(ARCH)-linux-
HOST_ARCH:=$(shell $(HOSTCC) -dumpmachine | sed -e s'/-.*//' \
	-e 's/sparc.*/sparc/' \
	-e 's/arm.*/arm/g' \
	-e 's/m68k.*/m68k/' \
	-e 's/ppc/powerpc/g' \
	-e 's/v850.*/v850/g' \
	-e 's/sh[234]/sh/' \
	-e 's/mips-.*/mips/' \
	-e 's/mipsel-.*/mipsel/' \
	-e 's/cris.*/cris/' \
	-e 's/i[3-9]86/i386/' \
	)
GNU_HOST_NAME:=$(HOST_ARCH)-pc-linux-gnu
TARGET_CONFIGURE_OPTS=PATH=$(TARGET_PATH) \
		AR=$(TARGET_CROSS)ar \
		AS=$(TARGET_CROSS)as \
		LD=$(TARGET_CROSS)ld \
		NM=$(TARGET_CROSS)nm \
		CC=$(TARGET_CROSS)gcc$(TARGET_SOFT_FLOAT) \
		GCC=$(TARGET_CROSS)gcc$(TARGET_SOFT_FLOAT) \
		CXX=$(TARGET_CROSS)g++$(TARGET_SOFT_FLOAT) \
		RANLIB=$(TARGET_CROSS)ranlib
ifeq ($(ENABLE_LOCALE),true)
DISABLE_NLS:=
else
DISABLE_NLS:=--disable-nls
endif


all:   world

TARGETS_CLEAN:=$(patsubst %,%-clean,$(TARGETS))
TARGETS_SOURCE:=$(patsubst %,%-source,$(TARGETS))
TARGETS_DIRCLEAN:=$(patsubst %,%-dirclean,$(TARGETS))

world: $(DL_DIR) $(BUILD_DIR) $(STAGING_DIR) $(TARGETS)

.PHONY: all world clean dirclean distclean source $(TARGETS) \
	$(TARGETS_CLEAN) $(TARGETS_DIRCLEAN) $(TARGETS_SOURCE)

include make/*.mk

#############################################################
#
# staging and target directories do NOT list these as 
# dependancies anywhere else
#
#############################################################
$(DL_DIR):
	mkdir $(DL_DIR)

$(STAGING_DIR):
	rm -rf $(STAGING_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p $(TOOL_BUILD_DIR)
	mkdir -p $(STAGING_DIR)/lib
	mkdir -p $(STAGING_DIR)/usr
	mkdir -p $(STAGING_DIR)/include
	ln -fs ../lib $(STAGING_DIR)/usr/lib

source: $(TARGETS_SOURCE)

install:
	$(EXTRA_STRIP) $(BUILD_DIR)/lib/ld-uClibc-0.9.26.so
	cp $(BUILD_DIR)/lib/ld-uClibc-0.9.26.so $(PREFIX)/lib
	ln -fs ld-uClibc-0.9.26.so $(PREFIX)/lib/ld-uClibc.so.0
	$(EXTRA_STRIP) $(BUILD_DIR)/lib/libuClibc-0.9.26.so
	cp $(BUILD_DIR)/lib/libuClibc-0.9.26.so $(PREFIX)/lib
	ln -fs libuClibc-0.9.26.so $(PREFIX)/lib/libc.so.0
	$(EXTRA_STRIP) $(BUILD_DIR)/lib/libcrypt-0.9.26.so
	cp $(BUILD_DIR)/lib/libcrypt-0.9.26.so $(PREFIX)/lib
	ln -fs libcrypt-0.9.26.so $(PREFIX)/lib/libcrypt.so.0

#############################################################
#
# Cleanup and misc junk
#
#############################################################
clean:
	rm -rf $(TOOL_BUILD_DIR) $(BUILD_DIR)

dirclean: $(TARGETS_DIRCLEAN)
	rm -rf $(STAGING_DIR) $(IMAGE)

distclean:
	rm -rf $(DL_DIR) $(BUILD_DIR) $(LINUX_KERNEL) $(IMAGE)

sourceball: 
	rm -rf $(BUILD_DIR)
	set -e; \
	cd ..; \
	rm -f buildroot.tar.bz2; \
	tar -cvf buildroot.tar buildroot; \
	bzip2 -9 buildroot.tar; \
