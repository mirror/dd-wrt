# Rules.make for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000-2002 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


#--------------------------------------------------------
# This file contains rules which are shared between multiple Makefiles.
# All normal configuration options live in the file named ".config".
# Don't mess with this file unless you know what you are doing.


#--------------------------------------------------------
# If you are running a cross compiler, you will want to set 'CROSS'
# to something more interesting...  Target architecture is determined
# by asking the CC compiler what arch it compiles things for, so unless
# your compiler is broken, you should not need to specify TARGET_ARCH
#
# Most people will set this stuff on the command line, i.e.
#        make CROSS=arm-linux-
# will build uClibc for 'arm'.

ifndef CROSS
CROSS=/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_mips/bin/mips-linux-
endif
CC= $(CROSS)gcc
AR= $(CROSS)ar
LD= $(CROSS)ld
NM= $(CROSS)nm
STRIPTOOL= $(CROSS)strip

INSTALL= install
LN= ln
RM= rm -f

# Select the compiler needed to build binaries for your development system
HOSTCC=gcc
HOSTCFLAGS=-O2 -Wall


#--------------------------------------------------------
# Nothing beyond this point should ever be touched by mere mortals.
# Unless you hang out with the gods, you should probably leave all
# this stuff alone.
MAJOR_VERSION:=0
MINOR_VERSION:=9
SUBLEVEL:=26
VERSION:=$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL:= C
export MAJOR_VERSION MINOR_VERSION SUBLEVEL VERSION LC_ALL

SHARED_FULLNAME:=libuClibc-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so
SHARED_MAJORNAME:=libc.so.$(MAJOR_VERSION)
UCLIBC_LDSO:=ld-uClibc.so.$(MAJOR_VERSION)
LIBNAME:=libc.a
LIBC:=$(TOPDIR)libc/$(LIBNAME)

# Pull in the user's uClibc configuration
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(TOPDIR).config
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; else echo "$(2)"; fi)

# Make certain these contain a final "/", but no "//"s.
TARGET_ARCH:=$(strip $(subst ",, $(strip $(TARGET_ARCH))))
RUNTIME_PREFIX:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(RUNTIME_PREFIX))))))
DEVEL_PREFIX:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(DEVEL_PREFIX))))))
export RUNTIME_PREFIX DEVEL_PREFIX

ARFLAGS:=r

OPTIMIZATION:=
PICFLAG:=-fPIC
# Some nice CPU specific optimizations
ifeq ($(strip $(TARGET_ARCH)),i386)
	OPTIMIZATION+=$(call check_gcc,-mpreferred-stack-boundary=2,)
	OPTIMIZATION+=$(call check_gcc,-falign-jumps=0 -falign-loops=0,-malign-jumps=0 -malign-loops=0)
	CPU_CFLAGS-$(CONFIG_386)+=-march=i386
	CPU_CFLAGS-$(CONFIG_486)+=-march=i486
	CPU_CFLAGS-$(CONFIG_586)+=-march=i586
	CPU_CFLAGS-$(CONFIG_586MMX)+=$(call check_gcc,-march=pentium-mmx,-march=i586)
	CPU_CFLAGS-$(CONFIG_686)+=-march=i686
	CPU_CFLAGS-$(CONFIG_PENTIUMIII)+=$(call check_gcc,-march=pentium3,-march=i686)
	CPU_CFLAGS-$(CONFIG_PENTIUM4)+=$(call check_gcc,-march=pentium4,-march=i686)
	CPU_CFLAGS-$(CONFIG_K6)+=$(call check_gcc,-march=k6,-march=i586)
	CPU_CFLAGS-$(CONFIG_K7)+=$(call check_gcc,-march=athlon,-malign-functions=4 -march=i686)
	CPU_CFLAGS-$(CONFIG_CRUSOE)+=-march=i686 -malign-functions=0 -malign-jumps=0 -malign-loops=0
	CPU_CFLAGS-$(CONFIG_WINCHIPC6)+=$(call check_gcc,-march=winchip-c6,-march=i586)
	CPU_CFLAGS-$(CONFIG_WINCHIP2)+=$(call check_gcc,-march=winchip2,-march=i586)
	CPU_CFLAGS-$(CONFIG_CYRIXIII)+=$(call check_gcc,-march=c3,-march=i586)
endif

ifeq ($(strip $(TARGET_ARCH)),arm)
	OPTIMIZATION+=-fstrict-aliasing
	CPU_LDFLAGS-$(ARCH_LITTLE_ENDIAN)+=-EL
	CPU_LDFLAGS-$(ARCH_BIG_ENDIAN)+=-EB
	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN)+=-mlittle-endian
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN)+=-mbig-endian
	CPU_CFLAGS-$(CONFIG_GENERIC_ARM)+=
	CPU_CFLAGS-$(CONFIG_ARM610)+=-mtune=arm610 -march=armv3
	CPU_CFLAGS-$(CONFIG_ARM710)+=-mtune=arm710 -march=armv3
	CPU_CFLAGS-$(CONFIG_ARM720T)+=-mtune=arm7tdmi -march=armv4
	CPU_CFLAGS-$(CONFIG_ARM920T)+=-mtune=arm9tdmi -march=armv4
	CPU_CFLAGS-$(CONFIG_ARM922T)+=-mtune=arm9tdmi -march=armv4
	CPU_CFLAGS-$(CONFIG_ARM926T)+=-mtune=arm9tdmi -march=armv4
	CPU_CFLAGS-$(CONFIG_ARM_SA110)+=-mtune=strongarm110 -march=armv4
	CPU_CFLAGS-$(CONFIG_ARM_SA1100)+=-mtune=strongarm1100 -march=armv4
	CPU_CFLAGS-$(CONFIG_ARM_XSCALE)+=$(call check_gcc,-mtune=xscale,-mtune=strongarm110)
	CPU_CFLAGS-$(CONFIG_ARM_XSCALE)+=-march=armv4 -Wa,-mcpu=xscale
endif

ifeq ($(strip $(TARGET_ARCH)),mips)
	CPU_CFLAGS-$(CONFIG_MIPS_ISA_1)+=-mips1
	CPU_CFLAGS-$(CONFIG_MIPS_ISA_2)+=-mips2 -mtune=mips2
	CPU_CFLAGS-$(CONFIG_MIPS_ISA_3)+=-mips3 -mtune=mips3
	CPU_CFLAGS-$(CONFIG_MIPS_ISA_4)+=-mips4 -mtune=mips4
	CPU_CFLAGS-$(CONFIG_MIPS_ISA_MIPS32)+=-mips32 -mtune=mips32
	CPU_CFLAGS-$(CONFIG_MIPS_ISA_MIPS64)+=-mips64 -mtune=mips32
endif

ifeq ($(strip $(TARGET_ARCH)),sh)
	OPTIMIZATION+=-fstrict-aliasing
	OPTIMIZATION+= $(call check_gcc,-mprefergot,)
	CPU_LDFLAGS-$(ARCH_LITTLE_ENDIAN)+=-EL
	CPU_LDFLAGS-$(ARCH_BIG_ENDIAN)+=-EB
	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN)+=-ml
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN)+=-mb
	CPU_CFLAGS-$(CONFIG_SH2)+=-m2
	CPU_CFLAGS-$(CONFIG_SH3)+=-m3
	CPU_CFLAGS-$(CONFIG_SH4)+=-m4
endif

ifeq ($(strip $(TARGET_ARCH)),sh64)
	OPTIMIZATION+=-fstrict-aliasing
	CPU_LDFLAGS-$(ARCH_LITTLE_ENDIAN):=-EL
	CPU_LDFLAGS-$(ARCH_BIG_ENDIAN):=-EB
	CPU_CFLAGS-$(ARCH_LITTLE_ENDIAN):=-ml
	CPU_CFLAGS-$(ARCH_BIG_ENDIAN):=-mb
	CPU_CFLAGS-$(CONFIG_SH5)+=-m5-32media
endif

ifeq ($(strip $(TARGET_ARCH)),h8300)
	CPU_LDFLAGS-$(CONFIG_H8300H)+= -ms8300h
	CPU_LDFLAGS-$(CONFIG_H8S)   += -ms8300s
	CPU_CFLAGS-$(CONFIG_H8300H) += -mh -mint32 -fsigned-char
	CPU_CFLAGS-$(CONFIG_H8S)    += -ms -mint32 -fsigned-char
endif

ifeq ($(strip $(TARGET_ARCH)),cris)
	CPU_LDFLAGS-$(CONFIG_CRIS)+=-mcrislinux
	CPU_CFLAGS-$(CONFIG_CRIS)+=-mlinux
	PICFLAG=-fpic
endif

# use '-Os' optimization if available, else use -O2, allow Config to override
OPTIMIZATION+=$(call check_gcc,-Os,-O2)


# Add a bunch of extra pedantic annoyingly strict checks
XWARNINGS=$(subst ",, $(strip $(WARNINGS))) -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing
XARCH_CFLAGS=$(subst ",, $(strip $(ARCH_CFLAGS)))
CPU_CFLAGS=$(subst ",, $(strip $(CPU_CFLAGS-y)))

LDADD_LIBFLOAT=
ifeq ($(strip $(UCLIBC_HAS_SOFT_FLOAT)),y)
# Add -msoft-float to the CPU_FLAGS since ldso and libdl ignore CFLAGS.
# If -msoft-float isn't supported, we want an error anyway.
    CPU_CFLAGS += -msoft-float
ifeq ($(strip $(TARGET_ARCH)),arm)
    LDADD_LIBFLOAT=-lfloat
endif
endif

# Some nice CFLAGS to work with
CFLAGS=$(XWARNINGS) $(OPTIMIZATION) $(XARCH_CFLAGS) $(CPU_CFLAGS) \
	-fno-builtin -nostdinc -D_LIBC -I$(TOPDIR)include -I.

ifeq ($(DODEBUG),y)
    #CFLAGS += -g3
    CFLAGS = $(XWARNINGS) -O0 -g3 $(CPU_CFLAGS) -fno-builtin -nostdinc -D_LIBC -I$(TOPDIR)include -I.
    LDFLAGS:= $(CPU_LDFLAGS-y) -shared --warn-common --warn-once -z combreloc
    STRIPTOOL:= true -Since_we_are_debugging
else
    LDFLAGS := $(CPU_LDFLAGS-y) -s -shared --warn-common --warn-once -z combreloc
endif

# Sigh, some stupid versions of gcc can't seem to cope with '-iwithprefix include'
#CFLAGS+=-iwithprefix include
CFLAGS+=$(shell $(CC) -print-search-dirs | sed -ne "s/install: *\(.*\)/-I\1include/gp")

ifneq ($(DOASSERTS),y)
    CFLAGS += -DNDEBUG
endif

ifeq ($(HAVE_SHARED),y)
    LIBRARY_CACHE:=#-DUSE_CACHE
    ifeq ($(BUILD_UCLIBC_LDSO),y)
	LDSO:=$(TOPDIR)lib/$(UCLIBC_LDSO)
	DYNAMIC_LINKER:=$(SHARED_LIB_LOADER_PREFIX)/$(UCLIBC_LDSO)
    else
	LDSO:=$(SYSTEM_LDSO)
	DYNAMIC_LINKER:=/lib/$(strip $(subst ",, $(notdir $(SYSTEM_LDSO))))
   endif
endif

CFLAGS_NOPIC:=$(CFLAGS)
ifeq ($(DOPIC),y)
    CFLAGS += $(PICFLAG)
endif

LIBGCC_CFLAGS ?= $(CFLAGS) $(CPU_CFLAGS-y)
LIBGCC:=$(shell $(CC) $(LIBGCC_CFLAGS) -print-libgcc-file-name)
LIBGCC_DIR:=$(dir $(LIBGCC))

########################################
#
# uClinux shared lib support
#

ifeq ($(CONFIG_BINFMT_SHARED_FLAT),y)
  # For the shared version of this, we specify no stack and its library ID
  FLTFLAGS += -s 0
  LIBID=1
  export LIBID FLTFLAGS
  SHARED_TARGET = lib/libc
endif

TARGET_ARCH:=$(strip $(subst ",, $(strip $(TARGET_ARCH))))

