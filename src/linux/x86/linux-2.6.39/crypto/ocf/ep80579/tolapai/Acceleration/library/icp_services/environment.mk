###############################################################
# environment.mk
#
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


ICP_COMMON_ENV_DIR=$(ICP_BUILDSYSTEM_PATH)/build_files
ICP_OS_LEVEL?=kernel_space
ICP_OS?=linux_2.6
ICP_CORE?=ia
ICP_EXTRA_WARNINGS?=n

KERNEL_DIR?=/lib/modules/`uname -r`/build
KERNEL_SOURCE_ROOT?=/usr/src/kernels/`uname -r`-`uname -m`

#
# Please change these env variable as per installation

TOOLS_RELATIVE_PATH=$(shell perl -e 'my @count=@ARGV[0]=~tr/\//\//;for(@i=4; @i[0]<@count[0]; @i[0]++){print "../";}' $(PWD))
ICP_TOOLS_PATH?=$(TOOLS_RELATIVE_PATH)../../../../../../../$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary

OSAL_RELATIVE_PATH=$(shell perl -e 'my @count=@ARGV[0]=~tr/\//\//;for(@i=4; @i[0]<@count[0]; @i[0]++){print "../";}' $(PWD))

ifeq ($(ICP_OS_LEVEL),user_space)
OSAL_LIB_PATH?=$(OSAL_RELATIVE_PATH)../../../../../../../../../../$(OSAL_DIR)/lib/EP805XX/linux_user/linuxle
else
OSAL_LIB_PATH?=$(OSAL_RELATIVE_PATH)../../../../../../../../../../$(OSAL_DIR)/lib/EP805XX/linux/linuxle
endif

RM_RELATIVE_PATH=$(shell perl -e 'my @count=@ARGV[0]=~tr/\//\//;for(@i=4; @i[0]<@count[0]; @i[0]++){print "../";}' $(PWD))

ifeq ($(ICP_OS_LEVEL),user_space)
RM_LIB_PATH?=$(RM_RELATIVE_PATH)../../../../../../../../$(RM_DIR)//source/build/linux_2.6/user_space
else
RM_LIB_PATH?=$(RM_RELATIVE_PATH)../../../../../../../../$(RM_DIR)//source/build/linux_2.6/kernel_space
endif

BT_RELATIVE_PATH=$(shell perl -e 'my @count=@ARGV[0]=~tr/\//\//;for(@i=4; @i[0]<@count[0]; @i[0]++){print "../";}' $(PWD))

ifeq ($(ICP_OS_LEVEL),user_space)
BT_LIB_PATH?=$(BT_RELATIVE_PATH)../../../../../../../../$(BT_DIR)/source/build/linux_2.6/user_space
else
BT_LIB_PATH?=$(BT_RELATIVE_PATH)../../../../../../../../$(BT_DIR)/source/build/linux_2.6/kernel_space
endif

ME_ACCEL_PATH?=$(ICP_ROOT)/me_acceleration_layer

ASD_PATH?=$(ICP_ROOT)/Acceleration/drivers/icp_asd
OSAL_DIR?=$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL
OSAL_SRCDIR?=$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL
CCI_HW_PLATFORM?=EP805XX
RM_HW_PLATFORM?=icp_services/ResourceManager
RM_DIR?=$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager
COMMON_INCL?=$(ICP_ROOT)/Acceleration/library/common/include

CCI_DIR?=$(ICP_ROOT)/infrastructure/access_layer/src/common/cci
CCI_BUILD_PATH?=$(ICP_ROOT)/infrastructure/access_layer/src/common/cci/build

RM_BUILD_PATH?=$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/build
RM_PLATFORM_DIR?=$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/rm
RM_COMMON_INCL_DIR?=$(ICP_ROOT)/Acceleration/library/common/include
EXTERNAL_INTERFACE?=$(ICP_ROOT)/external_interfaces 
ME_ACCELERATION_INCLUDE?=$(ICP_ROOT)/Acceleration/library/common/include
ME_ACCELERATION_PATH?=$(ICP_ROOT)/me_acceleration_layer

CSF_BUILD_RULES_PATH?=$(ICP_ROOT)/Acceleration/library/icp_services
USER_INCLUDE?=/usr/include
BT_DIR?=$(ICP_ROOT)/Acceleration/library/icp_services/BufferTranslation
DCC_DIR?=$(ICP_ROOT)/Acceleration/library/icp_debug/DCC/
MIL_DIR?=$(ICP_ROOT)/Acceleration/library/icp_debugmgmt/MIL
VERSION_FILE?=$(ICP_ROOT)/versionfile
RM_VERSION_FILE?=$(ICP_ROOT)/versionfile
PH_DIR?=$(ICP_ROOT)/infrastructure/access_layer/src/common/ph
MSG_DIR?=$(ICP_ROOT)/infrastructure/access_layer/src/common/msg_support
SYSAPP_DIR?=$(ICP_ROOT)/infrastructure/access_layer/src/common/sysapp_common
UTIL_DIR?=$(ICP_ROOT)/infrastructure/access_layer/src/common/utilities
SYSAPP_BUILD_PATH?=$(ICP_ROOT)/infrastructure/access_layer/src/common/sysapp_common/build
APPLICATION_BINDINGS_FILE?=$(ICP_ROOT)/infrastructure/access_layer/unit_test/sysapp_common/include
APPLICATION_DIR?=$(SYSAPP_DIR)/..
SYSAPP_BINDINGS?=$(SYSAPP_DIR)/..
GENERIC_BINDINGS?=$(SYSAPP_DIR)/..

include $(OSAL_DIR)/platforms/EP805XX/os/linux/make/OsalConfig.mk

EXTERNAL_INTERFACE_INCLUDE := $(ICP_ROOT)/Acceleration/include/
EXTERNAL_INTERFACE_API_INCLUDE := $(ICP_ROOT)/Acceleration/include/
IX_DEVICE?=EP805XX
#only for user space
OSAL_INCLUDES +=  -I$(OSAL_DIR)/common/POSIX/include

COMMON_INCL+=-I$(RM_DIR)/include \
	  -I$(EXTERNAL_INTERFACE_INCLUDE) \
	  -I$(RM_PLATFORM_DIR) \
	  -I$(USER_INCLUDE) 


