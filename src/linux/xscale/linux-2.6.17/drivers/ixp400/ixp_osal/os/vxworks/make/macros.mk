#
# Macro definitions for top-level OSAL Makefile
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

# NOTE!
# This file currently depends on the following environment variables:
# - WIND_HOST_TYPE
# - WIND_HOST_BASE
# - WIND_BASE


#####################################################################
# Determine the build host OS
#
# Only Solaris and Windows are currently supported for VxWorks builds

# In a Tornado environment, WIND_HOST_TYPE should be set
ifeq ($(WIND_HOST_TYPE), x86-win32)
  IX_OSAL_MK_HOST_OS := windows
else
  ifeq ($(WIND_HOST_TYPE), sun4-solaris2)
    IX_OSAL_MK_HOST_OS := solaris
  endif
endif

# If we don't have a valid OS name, try to use the Unix uname command
# to find it.
ifeq (,$(filter $(IX_OSAL_MK_HOST_OS), solaris windows))
  IX_OSAL_MK_HOST_OS := $(shell uname)
  IX_OSAL_MK_HOST_OS := $(subst SunOS,solaris,$(IX_OSAL_MK_HOST_OS))
endif

# If we still don't know, assume it's Windows
ifeq (,$(filter $(IX_OSAL_MK_HOST_OS), solaris windows))
  IX_OSAL_MK_HOST_OS := windows
endif


################################################################
# vxWorks BSP selection
#
ifeq ($(IX_DEVICE),ixp46X)

ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), vxle)
BSP := ixdp465_le
else
BSP := ixdp465
endif

else

ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), vxle)
BSP := ixdp425_le
else
BSP := ixdp425
endif

endif

BSP_DIR := $(WIND_BASE)/target/config/$(BSP)

# Windows paths must use '\' as seperator for the MSDOS 'cd' command
ifeq ($(IX_OSAL_MK_HOST_OS),windows)
  BSP_DIR := $(subst /,\,$(BSP_DIR))
endif


################################################################
# Tornado Compiler & linker commands

ifeq ($(IX_TARGET),vxsim)
VX_TOOL_SUFFIX = simso
CFLAGS := -DCPU=SIMSPARCSOLARIS

else
VX_TOOL_SUFFIX = arm
endif

ifeq ($(TOOL_FAMILY),)
ifeq  ($(findstring gnu,$(TOOL)),gnu)
TOOL_FAMILY	= gnu
else
ifeq  ($(findstring diab,$(TOOL)),diab)
TOOL_FAMILY	= diab
endif
endif
endif

ifeq ($(TOOL_FAMILY),diab)
CC := dcc
LD := dld
AR := dar
VXWORKS_VER := vxworks55
else
CC := cc$(VX_TOOL_SUFFIX)
LD := $(CC)
AR := ar$(VX_TOOL_SUFFIX)
endif


# These are tools used to make a .out file with vxWorks ctor/dtor table.

ifeq ($(TOOL_FAMILY),diab)
NM := ddump -M
else
NM := nm$(VX_TOOL_SUFFIX)
endif
MUNCH := wtxtcl $(WIND_HOST_BASE)/host/src/hutils/munch.tcl -asm $(VX_TOOL_SUFFIX)
ifeq ($(TOOL_FAMILY),diab)
COMPILE_TRADITIONAL := $(CC) -c -Xdollar-in-ident
else
COMPILE_TRADITIONAL := $(CC) -c -fdollars-in-identifiers
endif

# This is used for the 'memusage' target
OBJDUMP := objdump$(VX_TOOL_SUFFIX)


################################################################
# Compiler & linker options

# Compiler flags
# vxWorks compiler flags

 

ifeq ($(IX_TARGET),vxsim)

CFLAGS := -DCPU=SIMSPARCSOLARIS 
LDFLAGS := -nostdlib -r -Wl,-X 
MAKE_DEP_FLAG := -M

else
CFLAGS := -DRW_MULTI_THREAD -D_REENTRANT \
	-DCPU=XSCALE -DCPU_XSCALE -DARMMMU=ARMMMU_XSCALE -DARMCACHE=ARMCACHE_XSCALE

ifeq ($(TOOL_FAMILY),diab)
# compiler and linker flags using Diab compiler
  CFLAGS += -w -Xdialect-ansi -Xno-common -D__vxworks -D_DIAB_TOOL
  LDFLAGS := -r -W:as:,-x,-X -Ws
  MAKE_DEP_FLAG := -Xmake-dependency

else
# compiler and linker flags using GNU compiler
  CFLAGS += -Wall -ansi -pedantic -fno-common -mcpu=xscale -mapcs-32 -mno-sched-prolog
  LDFLAGS := -nostdlib -r -Wl,-X
  MAKE_DEP_FLAG := -M
endif
	
ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), vxle)
  CFLAGS += -DARMEL -D__ARMEL__ -DLITTLE_ENDIAN_MODE
  ifeq ($(TOOL_FAMILY),diab)
    LDFLAGS += -tARMXLS:$(VXWORKS_VER)
    CFLAGS += -tARMXLS:$(VXWORKS_VER)
  else
    LDFLAGS += -mlittle-endian
    CFLAGS += -mlittle-endian
  endif
else
  CFLAGS += -DARMEB -D__ARMEB__ -DBIG_ENDIAN_MODE
  ifeq ($(TOOL_FAMILY),diab)
    LDFLAGS += -tARMXES:$(VXWORKS_VER)
    CFLAGS += -tARMXES:$(VXWORKS_VER)
  else
    LDFLAGS += -mbig-endian
    CFLAGS += -mbig-endian
  endif
endif
endif

#Set additions to the compiler flag based on device chosen

ifneq (,$(filter $(IX_DEVICE), ixp46X))
CFLAGS += -D__ixp46X
else
CFLAGS += -D__ixp42X
endif

CFLAGS +=  -Isrc/include -I$(WIND_BASE)/target/h -I$(BSP_DIR) -I$(BSP_DIR)/../all 

