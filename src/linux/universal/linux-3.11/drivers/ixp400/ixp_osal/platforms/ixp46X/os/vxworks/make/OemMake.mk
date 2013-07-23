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

IX_OSAL_MK_PLATFORM := ixp400

ifeq ($(IX_DEVICE), ixp42X)
    IX_OSAL_MK_TGT_PLATFORM := ixdp425
else
ifeq ($(IX_DEVICE), ixp46X) 
    IX_OSAL_MK_TGT_PLATFORM := ixdp465
else
ifeq ($(IX_DEVICE), ixp43X)
ifeq ($(IX_PLATFORM), ixdp43x)
    IX_OSAL_MK_TGT_PLATFORM := ixdp435
else
    IX_OSAL_MK_TGT_PLATFORM := kixrp435
endif
else
    @echo "Error: IX_DEVICE not defined"
    @exit 1 
endif
endif
endif


ifeq ($(IX_OSAL_MK_TARGET_ENDIAN), vxle)
BSP := $(IX_OSAL_MK_TGT_PLATFORM)_le
else
BSP := $(IX_OSAL_MK_TGT_PLATFORM)
endif

#CFLAGS += -D__$(IX_DEVICE) -DIX_OSAL_THREAD_EXIT_GRACEFULLY \
#          -DENABLE_IOMEM -DENABLE_BUFFERMGT
#
CFLAGS += $(OSAL_CFLAGS) 
