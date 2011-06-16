#########################################################################
#
#  Targets supported
#  all     - builds everything and installs
#  install - identical to all
#  depend  - build dependencies
#  clean   - clears derived objects except the .depend files
#  distclean- clears all derived objects and the .depend file
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
############################################################################


####################Common variables and definitions########################
# Ensure The ICP_ENV_DIR environmental var is defined.
ifndef ICP_ENV_DIR
$(error ICP_ENV_DIR is undefined. Please set the path to your environment makefile \
        "-> setenv ICP_ENV_DIR <path>")
endif

#Add your project environment Makefile
include $(ICP_ENV_DIR)/environment.mk
include $(ICP_COMMON_ENV_DIR)/common_env.mk

#include the makefile with all the default and common Make variable definitions
include $(ICP_BUILDSYSTEM_PATH)/build_files/common.mk

#Add the name for the executable, Library or Module output definitions
OUTPUT_NAME= icp_asd


# List of Source Files to be compiled (to be in a single line or on different lines separated by a "\" and tab.
SOURCES+= asd_drv.c asd_cfg.c asd_dram.c icp_asd_cfg.c asd_fw.c asd_init.c asd_isr.c asd_ctl_drv.c asd_acpi.c asd_uclo_ldr.c

ASD_ICP_SERVICES_INCLUDE=-I$(ICP_ASD_BTR_INCLUDE) -I$(ICP_ASD_RM_INCLUDE) -I$(ICP_RM_AL_TLP_DIR)\
 			-I$(ICP_ASD_HAL_INCLUDE_LINUX) -I$(ICP_ASD_HAL_INCLUDE)

ASD_ICP_CRYPTO_INCLUDE=-I$(ICP_ASD_AL_INCLUDE) -I$(ICP_ASD_LAC_INCLUDE)

ASD_ICP_DEBUG_INCLUDE=-I$(ICP_ASD_DCC_INCLUDE)

ASD_ICP_PPC_INCLUDE=-I$(ICP_ASD_FP_INCLUDE)

ASD_INCLUDES=$(ASD_ICP_SERVICES_INCLUDE) $(ASD_ICP_CRYPTO_INCLUDE) $(ASD_ICP_DEBUG_INCLUDE) $(ASD_ICP_PPC_INCLUDE)

TOOLS_INCLUDES=-I$(TOOLS_DIR)/include

INCLUDES=-I$(ICP_API_DIR) -I$(ICP_ASD_DIR)/include -I$(ICP_ASD_KERNEL_INCLUDE) $(ASD_INCLUDES) $(TOOLS_INCLUDES)

EXTRA_CFLAGS += -DWCSE=1 

ifdef ASD_DEBUG
EXTRA_CFLAGS += -DASD_SHOW_DEBUG=1
endif

EXTRA_CFLAGS += -DLICENSE_TYPE=\"Dual\ BSD/GPL\"

KBUILD_EXTRA_SYMBOLS=\
	$(ICP_DCC_DIR)/source/Module.symvers \
	$(ICP_MIL_DIR)/source/Module.symvers \
	$(ICP_LAC_DIR)/src/Module.symvers \
	$(ICP_TOOLS_XSC_CORE_ROOT)/halAe/$(ICP_TOOLS_TARGET)/Module.symvers


# On the line directly below list the outputs you wish to build for,
# e.g "lib_static lib_shared exe module" as show below
install: module

###################Include rules makefiles########################
include $(ICP_BUILDSYSTEM_PATH)/build_files/rules.mk
###################End of Rules inclusion#########################

#include your $(ICP_OS)_$(ICP_OS_LEVEL).mk file
include $(ICP_ENV_DIR)/$(ICP_OS)_$(ICP_OS_LEVEL).mk

