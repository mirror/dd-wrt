#
# Macro definitions for platform-specific makefile
#
# 
# @par
# IXP400 SW Release version 2.4
# 
# -- Copyright Notice --
# 
# @par
# Copyright (c) 2001-2007, Intel Corporation.
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
#
#
IX_OSAL_MK_PLATFORM := ixp400

ifeq ($(IX_DEVICE), ixp42X)
	IX_OSAL_MK_TGT_PLATFORM := ixdp425
else
ifeq ($(IX_DEVICE), ixp46X) 
	IX_OSAL_MK_TGT_PLATFORM := ixdp465
else
ifeq ($(IX_DEVICE), ixp43X)
	IX_OSAL_MK_TGT_PLATFORM := ixdp435
else
    @echo "Error: IX_DEVICE not defined"
    @exit 1 
endif
endif
endif

# Linux IXP4XX Compiler and Linker commands
ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), linuxbe)
	LINUX_CROSS_COMPILE := ${IX_LINUX_CROSSCOMPILER}
else
	LINUX_CROSS_COMPILE := ${IX_LINUX_CROSSCOMPILER}
endif
 
COMPILE_PREFIX := $(LINUX_CROSS_COMPILE)
LINUX_SRC := $($(IX_TARGET)_KERNEL_DIR)


# Endian-specific flags
#ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), linuxbe)
ifeq ($(IX_TARGET),linuxbe)
CFLAGS += -mbig-endian
LDFLAGS += -EB
else
CFLAGS += -mlittle-endian
LDFLAGS += -EL
endif

ifeq ($(IX_LINUXVER), 2.6)
    LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -march=armv5te -Wa,-mcpu=xscale -mtune=xscale
# Commented out to remove a warning   
#    CFLAGS_ETC = -mabi=apcs-gnu 
else 
    LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -mcpu=xscale -mtune=xscale 
    CFLAGS_ETC = -mapcs-32 -mshort-load-bytes
endif 
				 
CFLAGS += -D__$(IX_DEVICE) -D__KERNEL__ -I$(LINUX_SRC)/include -Wall \
			-Wno-trigraphs -fno-common -pipe $(CFLAGS_ETC) -msoft-float -DMODULE \
			-D__linux -DCPU=33 -DXSCALE=33 $(LINUX_MACH_CFLAGS) -DEXPORT_SYMTAB

CFLAGS += $(OSAL_CFLAGS)

INCLUDE_DIRS += $(LINUX_SRC)/include/asm-arm/arch-ixp425/ 
