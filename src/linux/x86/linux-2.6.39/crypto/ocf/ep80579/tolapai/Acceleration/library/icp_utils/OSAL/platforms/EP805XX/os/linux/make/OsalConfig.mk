# 
# @par
# This file is provided under a dual BSD/GPLv2 license.  When using or 
#   redistributing this file, you may do so under either license.
# 
#   GPL LICENSE SUMMARY
# 
#   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify 
#   it under the terms of version 2 of the GNU General Public License as
#   published by the Free Software Foundation.
# 
#   This program is distributed in the hope that it will be useful, but 
#   WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License 
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#   The full GNU General Public License is included in this distribution 
#   in the file called LICENSE.GPL.
# 
#   Contact Information:
#   Intel Corporation
# 
#   BSD LICENSE 
# 
#   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
#   All rights reserved.
# 
#   Redistribution and use in source and binary forms, with or without 
#   modification, are permitted provided that the following conditions 
#   are met:
# 
#     * Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright 
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution.
#     * Neither the name of Intel Corporation nor the names of its 
#       contributors may be used to endorse or promote products derived 
#       from this software without specific prior written permission.
# 
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
#  version: Security.L.1.0.3-98
#

###########################################################
# This file is to be included in user build to configure
# OSAL features. 
# 
# This is specific to EP805XX/Linux build of OSAL only.
########################################################### 


###########################################################
# Set OSAL_SRC_PATH to the absolute path where ixp_osal/ is
# located


OSAL_SRC_PATH := $(OSAL_SRCDIR)


PLATFORM_DIR_NAME := $(IX_DEVICE)

ifdef KERNEL_SOURCE_ROOT
LINUX_SRC := $(KERNEL_SOURCE_ROOT)
endif

###########################################################
# All defines that are valid for OSAL on Linux

# Set this to enable graceful thread exit
#ENABLE_PCI=1

# Set this to using Data Coherent SDRAM definitions
ENABLE_SPINLOCK=1

# Set this to use buffer management module of OSAL
#ENABLE_BUFFERMGT=1

# Set this to use IOMEM module of OSAL
ENABLE_IOMEM=1

# Set this to use buffer management module of OSAL
ENABLE_DDK=1


###########################################################
# Compilation flags for using configurable features of OSAL 
# Set CFLAGS += $(OSAL_CFLAGS) in the including Makefile to
# include the flags set here

OSAL_CFLAGS :=  -D__LITTLE_ENDIAN \
		-DIX_HW_COHERENT_MEMORY \
		-DIX_OSAL_MEM_MAP_GLUECODE \
        -DIX_OSAL_THREAD_EXIT_GRACEFULLY  \
        -DUSE_NATIVE_OS_TIMER_API

ifdef ENABLE_PCI
OSAL_CFLAGS += -DENABLE_PCI
endif

ifdef ENABLE_SPINLOCK
OSAL_CFLAGS += -DENABLE_SPINLOCK
endif

ifdef ENABLE_BUFFERMGT
OSAL_CFLAGS += -DENABLE_BUFFERMGT
endif

ifdef ENABLE_IOMEM
OSAL_CFLAGS += -DENABLE_IOMEM
endif

ifdef ENABLE_DDK
OSAL_CFLAGS += -DENABLE_DDK
endif

ifdef IX_OSAL_ENSURE_ON 
OSAL_CFLAGS += -DIX_OSAL_ENSURE_ON
endif

ifdef OSAL_EXPORT_SYMBOLS
OSAL_CFLAGS += -DOSAL_EXPORT_SYMBOLS
endif

OSAL_LDFLAGS := 

###########################################################
# OSAL_INCLUDES lists all the include paths to be included
# in order to use OSAL headers. This needs OSAL_SRC_PATH to be 
# set to the absolute path where osal is located.
# Include $(OSAL_INCLUDES) to the INCLUDE_DIRS as 
# INCLUDE_DIRS += $(OSAL_INCLUDES) for using. 

# Common includes
OSAL_INCLUDES := -I$(OSAL_SRC_PATH)/common/include \
		 -I$(OSAL_SRC_PATH)/common/include/modules \
		 -I$(OSAL_SRC_PATH)/common/include/modules/ddk \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/core \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/modules \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/modules/ddk \
		 -I$(OSAL_SRC_PATH)/platforms/$(PLATFORM_DIR_NAME)/include \
		 -I$(OSAL_SRC_PATH)/platforms/$(PLATFORM_DIR_NAME)/os/linux/include 

ifdef ENABLE_BUFFERMGT
OSAL_INCLUDES += -I$(OSAL_SRC_PATH)/common/include/modules/bufferMgt \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/modules/bufferMgt 
endif

ifdef ENABLE_IOMEM
OSAL_INCLUDES += -I$(OSAL_SRC_PATH)/common/os/linux/include/modules/ioMem \
		 -I$(OSAL_SRC_PATH)/common/include/modules/ioMem 
endif


