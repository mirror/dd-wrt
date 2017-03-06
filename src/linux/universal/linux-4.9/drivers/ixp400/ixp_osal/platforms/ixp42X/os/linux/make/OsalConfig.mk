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

###########################################################
# This file is to be included in user build to configure
# OSAL features. 
# 
# This is specific to ixp4XX/Linux build of OSAL only.
########################################################### 



###########################################################
# Set OSAL_SRC_PATH to the absolute path where ixp_osal/ is
# located

OSAL_SRC_PATH := $(OSAL_SRCDIR)


###########################################################
# All defines that are valid for OSAL on Linux

# Set this to use Fast Mutex implementation provided by Oem.  
IX_OSAL_OEM_FAST_MUTEX=1

# Set this to enable graceful thread exit
IX_OSAL_THREAD_EXIT_GRACEFULLY=1

# Set this to using Data Coherent SDRAM definitions
IX_SDRAM_DC=1

# Set this to use IOMEM module of OSAL
ENABLE_IOMEM=1

# Set this to use buffer management module of OSAL
ENABLE_BUFFERMGT=1

# Set this to use buffer management module of OSAL
ENABLE_DDK=1

###########################################################
# Compilation flags for using configurable features of OSAL 
# Set CFLAGS += $(OSAL_CFLAGS) in the including Makefile to
# include the flags set here

OSAL_CFLAGS := 

ifdef IX_OSAL_OEM_FAST_MUTEX
OSAL_CFLAGS += -DIX_OSAL_OEM_FAST_MUTEX
endif

ifdef IX_OSAL_THREAD_EXIT_GRACEFULLY
OSAL_CFLAGS += -DIX_OSAL_THREAD_EXIT_GRACEFULLY
endif

ifdef IX_SDRAM_DC
OSAL_CFLAGS += -DIX_SDRAM_DC
endif

ifdef ENABLE_IOMEM
OSAL_CFLAGS += -DENABLE_IOMEM
endif

ifdef ENABLE_BUFFERMGT
OSAL_CFLAGS += -DENABLE_BUFFERMGT
endif

ifdef ENABLE_DDK
OSAL_CFLAGS += -DENABLE_DDK
endif

OSAL_LDFLAGS := 

###########################################################
# OSAL_INCLUDES lists all the include paths to be included
# in order to use OSAL headers. This needs OSAL_SRC_PATH to be 
# set to the absolute path where ixp_osal is located.
# Include $(OSAL_INCLUDES) to the INCLUDE_DIRS as 
# INCLUDE_DIRS += $(OSAL_INCLUDES) for using. 

# Common includes
OSAL_INCLUDES := -I$(OSAL_SRC_PATH)/common/include \
		 -I$(OSAL_SRC_PATH)/common/include/modules \
		 -I$(OSAL_SRC_PATH)/common/include/modules/ddk \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/core \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/modules \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/modules/ddk \
		 -I$(OSAL_SRC_PATH)/platforms/ixp42X/include \
		 -I$(OSAL_SRC_PATH)/platforms/ixp42X/os/linux/include 

ifdef ENABLE_BUFFERMGT
OSAL_INCLUDES += -I$(OSAL_SRC_PATH)/common/include/modules/bufferMgt \
		 -I$(OSAL_SRC_PATH)/common/os/linux/include/modules/bufferMgt 
endif

ifdef ENABLE_IOMEM
OSAL_INCLUDES += -I$(OSAL_SRC_PATH)/common/os/linux/include/modules/ioMem \
		 -I$(OSAL_SRC_PATH)/common/include/modules/ioMem 
endif


