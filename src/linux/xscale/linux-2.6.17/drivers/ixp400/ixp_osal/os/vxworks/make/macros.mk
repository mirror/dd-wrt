#
# Macro definitions for top-level OSAL Makefile
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

