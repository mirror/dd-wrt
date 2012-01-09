ifeq ($(obj),)
obj := $(shell pwd)
endif

# other locales sometimes cause trouble
export LC_ALL = POSIX


# If we are AHB bus type, then we need the kernel path for the target
# kernel sources.

ifeq ($(strip ${BUS}),AHB)
TARGET := mipsisa32-be-elf

ifndef KERNELPATH
$(error KERNELPATH must be defined for bus type AHB.)
endif

ifndef TOOLPATH
$(error TOOLPATH must be defined for bus type AHB.)
endif

KERNELCONF := ${KERNELPATH}/.config

ifndef KERNELRELEASE
VERSION := $(strip $(shell head -n 1 ${KERNELPATH}/Makefile | cut -f 2 -d'='))
PATCHLEVEL := $(strip $(shell head -n 2 ${KERNELPATH}/Makefile | tail -1 | cut -f 2 -d'='))
SUBVERSION := $(strip $(shell head -n 3 ${KERNELPATH}/Makefile | tail -1 | cut -f 2 -d'='))
KERNELRELEASE := $(strip ${VERSION}.${PATCHLEVEL}.${SUBVERSION}${EXTRAVERSION})
endif

ifndef MODULEPATH
MODULEPATH := ${KERNELPATH}/arch/mips/ar531x/ROOTDISK/rootdir/lib/modules/${KERNELRELEASE}/net
endif

endif

#
NM := nm
AWK := awk

# Path to the Linux kernel, include files, etc.
#
# KERNELRELEASE is the target kernel's version.  If KERNELRELEASE
# is not set in the environment then it is taken from the running
# system.
#
# KERNELPATH is the path to the target kernel's build/source area.
# This is used to obtain the kernel configuration and include files.
# If KERNELPATH is not set in the environment then it is derived
# from KERNELRELEASE.
#
ifeq ($(KERNELRELEASE),)
KERNELRELEASE := $(shell uname -r)
endif

ifneq ($(findstring 2.6,$(KERNELRELEASE)),)
export-objs	:=
list-multi	:=
KMODSUF		:= ko
else
KMODSUF		:= o
endif

ifeq ($(KERNELPATH),)
KERNELPATH := /lib/modules/${KERNELRELEASE}/build
endif

ifeq ($(DESTDIR),)
DESTDIR :=
endif

ifeq ($(MODULEPATH),)
MODULEPATH := /lib/modules/${KERNELRELEASE}/net
endif

ifeq ($(strip $(BUS)),AHB)
#INCS += -isystem ${TOOLPATH}/include
endif

TARGET = mips-linux-

CC = ${TOOLPATH}/bin/${TARGET}gcc
STRIP = ${TOOLPATH}/bin/${TARGET}strip
