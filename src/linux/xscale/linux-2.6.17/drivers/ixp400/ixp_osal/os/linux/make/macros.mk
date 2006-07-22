#
# Macro definitions for os-specific makefile
#
# 
# @par
# IXP400 SW Release version  2.1
# 
# -- Intel Copyright Notice --
# 
# @par
# Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
# 
# @par
# The source code contained or described herein and all documents
# related to the source code ("Material") are owned by Intel Corporation
# or its suppliers or licensors.  Title to the Material remains with
# Intel Corporation or its suppliers and licensors.
# 
# @par
# The Material is protected by worldwide copyright and trade secret laws
# and treaty provisions. No part of the Material may be used, copied,
# reproduced, modified, published, uploaded, posted, transmitted,
# distributed, or disclosed in any way except in accordance with the
# applicable license agreement .
# 
# @par
# No license under any patent, copyright, trade secret or other
# intellectual property right is granted to or conferred upon you by
# disclosure or delivery of the Materials, either expressly, by
# implication, inducement, estoppel, except in accordance with the
# applicable license agreement.
# 
# @par
# Unless otherwise agreed by Intel in writing, you may not remove or
# alter this notice or any other notice embedded in Materials by Intel
# or Intel's suppliers or licensors in any way.
# 
# @par
# For further details, please see the file README.TXT distributed with
# this software.
# 
# @par
# -- End Intel Copyright Notice --
#
# NOTE!
# This file currently depends on the following environment variables:
# - HARDHAT_BASE
# - IX_TARGET
# - linuxbe_KERNEL_DIR or linuxle_KERNEL_DIR


#####################################################################
# Determine the build host OS
#
# Only Linux (and partially Cygwin) are currently supported for Linux builds

IX_OSAL_MK_HOST_OS := $(OSTYPE)

# If we don't have a valid OS name, try to use the Unix uname command
# to find it.
ifneq ($(IX_OSAL_MK_HOST_OS), linux)
  IX_OSAL_MK_HOST_OS := $(shell uname)
  IX_OSAL_MK_HOST_OS := $(subst Linux,linux,$(IX_OSAL_MK_HOST_OS))
# We do not check for 'cygwin' here, as a windows box will often have
# a cygwin "uname" on its PATH even when not running in a true cygwin
# environment. We must rely on the OSTYPE environment variable to tell
# us if we're in a true cygwin environment.
endif


################################################################
# Linux Compiler & linker commands

ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), linuxbe)
LINUX_CROSS_COMPILE := $(HARDHAT_BASE)/devkit/arm/xscale_be/bin/xscale_be-
else
LINUX_CROSS_COMPILE := $(HARDHAT_BASE)/devkit/arm/xscale_le/bin/xscale_le-
endif

LINUX_SRC := $($(IX_TARGET)_KERNEL_DIR)

LD := $(LINUX_CROSS_COMPILE)ld
CC := $(LINUX_CROSS_COMPILE)gcc
AR := $(LINUX_CROSS_COMPILE)ar


################################################################
# Compiler & linker options

# Compiler flags
LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -mcpu=xscale -mtune=xscale

CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wno-trigraphs -fno-common \
          -pipe -mapcs-32 -mshort-load-bytes -msoft-float -DMODULE \
          -D__linux -DCPU=33 -DXSCALE=33 $(LINUX_MACH_CFLAGS) -DEXPORT_SYMTAB

# Linux linker flags
LDFLAGS := -r
MAKE_DEP_FLAG := -M

# Endian-specific flags
ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), linuxbe)
CFLAGS += -mbig-endian
LDFLAGS += -EB
else
CFLAGS += -mlittle-endian
LDFLAGS += -EL
endif

#set additions to the compiler flag based on device chosen
ifneq (,$(filter $(IX_DEVICE), ixp46X))
CFLAGS += -D__ixp46X
else
CFLAGS += -D__ixp42X
endif


