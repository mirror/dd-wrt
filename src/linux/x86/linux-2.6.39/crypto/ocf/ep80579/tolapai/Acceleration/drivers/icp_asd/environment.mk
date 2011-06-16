###############################################################
# environment.mk
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
################################################################

###################
#Directory paths
###################
ICP_COMMON_ENV_DIR=$(ICP_BUILDSYSTEM_PATH)/build_files
ICP_OSAL_DIR=$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/
ICP_LAC_DIR=$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto
ICP_API_DIR=$(ICP_ROOT)/Acceleration/include
ICP_AE_TOOLS_DIR=$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary
ICP_ASD_DIR=$(ICP_ROOT)/Acceleration/drivers/icp_asd
ICP_COMMON_FW_API_DIR=$(ICP_ROOT)/Acceleration/library/icp_crypto/QAT_FW/common/include
ICP_RM_AL_DIR=$(ICP_ROOT)/Acceleration/library/common/include
ICP_RM_AL_TLP_DIR=$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/rm
ICP_ASD_FP_INCLUDE=$(ICP_ROOT)/Acceleration/library
#####################
#ASD Kernel Includes
#####################
ICP_ASD_KERNEL_INCLUDE=$(ICP_ASD_DIR)/src/kernel/include

#######################
#ICP Services Includes
#######################
ICP_ASD_BTR_INCLUDE=$(ICP_ROOT)/Acceleration/library/icp_services/BufferTranslation/include
ICP_ASD_RM_INCLUDE=$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/include
ICP_ASD_HAL_INCLUDE=$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/include

#####################
#ICP Crypto Includes
#####################
ICP_ASD_AL_INCLUDE=$(ICP_ROOT)/Acceleration/library/icp_crypto/QATAL/include/
ICP_ASD_LAC_INCLUDE=$(ICP_LAC_DIR)/include

####################
#ICP Debug Includes
####################
ICP_ASD_DCC_INCLUDE=$(ICP_ROOT)/Acceleration/library/icp_debug/DCC/include

########################
#Kernel level variables
########################
#KERNEL_SOURCE_ROOT?=/lib/modules/`uname -r`/build
ICP_OS_LEVEL?=kernel_space
ICP_OS?=linux_2.6
ICP_CORE?=ia

####################
#Osal Variables
####################
OSAL_SRCDIR=$(ICP_OSAL_DIR)
LINUX_SRC=$(KERNEL_SOURCE_ROOT)
IX_OSAL_PLATFORM=EP805XX
IX_DEVICE=EP805XX
IX_TARGET=linuxle
IX_LINUXVER=2.6
IX_OSAL_OS_LEVEL=$(ICP_OS_LEVEL)
IX_HW_COHERENT_MEMORY=1

OSAL_CONFIG_MK=$(ICP_OSAL_DIR)/platforms/$(IX_OSAL_PLATFORM)/os/linux/make/OsalConfig.mk

