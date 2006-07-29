#
# Macro definitions for os-specific makefile
#
# 
# @par
# IXP400 SW Release Crypto version 2.1
# 
# -- Copyright Notice --
# 
# @par
# Copyright (c) 2001-2005, Intel Corporation.
# All rights reserved.
# 
# @par
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Intel Corporation nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# 
# @par
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 
# 
# @par
# -- End of Copyright Notice --
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
ifeq ($(IX_LINUXVER), 2.6)
    LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -march=armv5te -Wa,-mcpu=xscale -mtune=xscale
    CFLAGS_ETC = -mabi=apcs-gnu
else
    LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -mcpu=xscale -mtune=xscale
    CFLAGS_ETC = -mapcs-32 -mshort-load-bytes
endif

CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wno-trigraphs -fno-common \
          -pipe $(CFLAGS_ETC) -msoft-float -DMODULE \
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


