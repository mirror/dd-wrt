################################################################
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
# Ensure The ICP_ROOT environmental var is defined.
ifndef ICP_ROOT
$(error ICP_ROOT is undefined. Please set the path \
        "-> setenv ICP_ROOT <path to root of source code>")
endif
DATE=$(shell date '+%m_%d_%y')
ICP_BUILD_OUTPUT?=~/build_$(DATE)
KERNEL_SOURCE_ROOT?=/usr/src/kernels/`uname -r`-`uname -m`
ICP_TOOLS_TARGET=EP80579
ifdef ICP_DEBUG
ICP_DEBUG_ON=ICP_DEBUG=1
endif
export OSAL_PATH=$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/
ASD_PATH=$(ICP_ROOT)/Acceleration/drivers/icp_asd/
HAL_PATH=$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/
LAC_PATH=$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/
ICPSERVICES_PATH=$(ICP_ROOT)/Acceleration/library/icp_services
ICPSERVICES_BIN_PATH=$(ICP_ROOT)/Acceleration/library/icp_services/build/linux_2.6/kernel_space/
DCC_PATH=$(ICP_ROOT)/Acceleration/library/icp_debug/DCC/source
DCC_BIN_PATH=$(ICP_ROOT)/Acceleration/library/icp_debug/DCC/source/build/linux_2.6/kernel_space
ICP_DEBUG_MGT_PATH=$(ICP_ROOT)/Acceleration/library/icp_debugmgmt/MIL/source
QAT_AL_PATH=$(ICP_ROOT)/Acceleration/library/icp_crypto/QATAL/
export OSAL_DIR=$(OSAL_PATH)
OSAL_SRCDIR=$(OSAL_PATH)
export ICP_ACCEL_INC=YES
export ICP_BUILDSYSTEM_PATH := $(ICP_ROOT)/build_system
export BUILDSYSTEM_PATH := $(ICP_ROOT)/build_system
QAT_FW_PATH=$(ICP_ROOT)/Acceleration/firmware


all: osal hal qat-al icp_crypto install_scripts

lac: osal hal dcc qat-al icp_crypto

list_build_output:
	@ls $(ICP_BUILD_OUTPUT)/

output_dir:
	@test -d $(ICP_BUILD_OUTPUT) || mkdir $(ICP_BUILD_OUTPUT);
	
install_scripts:
	@echo ; echo 'Copying Install Scripts';
	@chmod 777 $(ICP_BUILDSYSTEM_PATH)/build_files/install_security.sh $(ICP_BUILDSYSTEM_PATH)/build_files/qat_service
	@cp $(ICP_BUILDSYSTEM_PATH)/build_files/install_security.sh  $(ICP_BUILD_OUTPUT)/;
	@cp $(ICP_BUILDSYSTEM_PATH)/build_files/qat_service  $(ICP_BUILD_OUTPUT)/;
	

osal: output_dir
	@echo ; echo 'Building OSAL';
	@cd $(OSAL_PATH) && make libosal IX_TARGET=linuxle KERNEL_SOURCE_ROOT=$(KERNEL_SOURCE_ROOT) IX_OSAL_PLATFORM=EP805XX IX_DEVICE=EP805XX IX_TARGET=linuxle IX_OSAL_OS_LEVEL=kernel_space IX_LINUXVER=2.6.18 IX_HW_COHERENT_MEMORY=1;

asd: output_dir
	@echo ; echo 'Building System Driver';
	@cd $(ASD_PATH) && export ICP_ENV_DIR=$(ASD_PATH) && $(MAKE) asd KERNEL_SOURCE_ROOT=$(KERNEL_SOURCE_ROOT);
	@echo ; echo 'Copying System Driver binary';
	@cp -f $(ASD_PATH)/src/kernel/linux/build/linux_2.6/kernel_space/icp_asd.ko $(ICP_BUILD_OUTPUT)/;
	@cp -f $(ASD_PATH)/src/user/build/linux_2.6/user_space/asd_ctl $(ICP_BUILD_OUTPUT)/
	@cp -f $(ASD_PATH)/icp_asd.conf $(ICP_BUILD_OUTPUT)/

hal: output_dir
	@echo ; echo 'Building Hal Libs';
	@cd $(HAL_PATH)/Target_CoreLibs/makDefs && make -f coreLibs_kernel.mak ICP_TOOLS_TARGET=$(ICP_TOOLS_TARGET);
	@echo ; echo 'Copying HAL Binary';
	@cp $(HAL_PATH)/bin_linux_le/$(ICP_TOOLS_TARGET)/icp_hal.ko $(ICP_BUILD_OUTPUT)/

qat-al: output_dir
	@echo ; echo 'Building QAT access layer';
	@cd $(QAT_AL_PATH) && export ENV_DIR=$(QAT_AL_PATH) &&export ICP_ENV_DIR=$(QAT_AL_PATH) && cd src && make target_lib=qatal lib_kernel ICP_OS_LEVEL=kernel_space; 
	@echo ; echo 'Copying QAT-FW Binary';
	@cp $(QAT_FW_PATH)/uof_firmware.bin $(ICP_BUILD_OUTPUT)/uof_firmware.bin;
	@echo ; echo 'Copying MMP Binary';
	@cp $(QAT_FW_PATH)/mmp_firmware.bin $(ICP_BUILD_OUTPUT)/mmp_firmware.bin;
icp_crypto: output_dir
	@echo ; echo 'Building icp_crypto';
	@cd $(LAC_PATH) && export ICP_ENV_DIR=$(LAC_PATH) && cd src && make ICP_TOOLS_TARGET=$(ICP_TOOLS_TARGET) ICP_OS_LEVEL=kernel_space ICP_CRYPTO=1; 
	@echo ; echo 'Copying icp_crypto Binary';
	@cp $(LAC_PATH)/src/build/linux_2.6/kernel_space/icp_crypto.ko $(ICP_BUILD_OUTPUT)/;

dcc: output_dir
	@echo ; echo 'Building DCC';
	@cd $(DCC_PATH) && export ICP_ENV_DIR=$(ICPSERVICES_PATH) && make module COMP=dcc $(ICP_DEBUG_ON) OSAL_DIR=$(OSAL_PATH); 
	@echo ; echo 'Copying icp_debug Binary';
	@cp $(DCC_BIN_PATH)/icp_debug.ko $(ICP_BUILD_OUTPUT)/

icp_debugmgt: output_dir
	@echo ; echo 'Building icp_debugmgt';
	@cd $(ICP_DEBUG_MGT_PATH) && export TOOLS_PATH=$(TOOLS_PATH) && export ICP_ENV_DIR=$(ICPSERVICES_PATH) && make module $(ICP_DEBUG_ON) OSAL_DIR=$(OSAL_PATH);
	@echo ; echo 'Copying MIL binary';
	cp $(ICP_DEBUG_MGT_PATH)/build/linux_2.6/kernel_space/icp_debugmgmt.ko $(ICP_BUILD_OUTPUT)/
	@echo ; echo 'Building MIL exe';
	@cd $(ICP_DEBUG_MGT_PATH)  && export ICP_ENV_DIR=$(ICPSERVICES_PATH) && make exe ICP_OS_LEVEL=user_space;
	@echo ; echo 'Copying MIL binary'; 
	cp $(ICP_DEBUG_MGT_PATH)/build/linux_2.6/user_space/debugmgr $(ICP_BUILD_OUTPUT)/

	
clean:
	@rm -rf $(ICP_BUILD_OUTPUT)/*
	@echo ; echo 'Cleaning OSAL';
	@cd $(OSAL_PATH) && make clean IX_TARGET=linuxle KERNEL_SOURCE_ROOT=$(KERNEL_SOURCE_ROOT) IX_OSAL_PLATFORM=EP805XX IX_DEVICE=EP805XX IX_TARGET=linuxle IX_OSAL_OS_LEVEL=kernel_space IX_LINUXVER=2.6.18 IX_HW_COHERENT_MEMORY=1;
	@echo ; echo 'Cleaning System Driver';
	@cd $(ASD_PATH) && export ICP_ENV_DIR=$(ASD_PATH) && $(MAKE) clean
	@echo ; echo 'Cleaning Hal';
	@cd $(HAL_PATH)/Target_CoreLibs/makDefs && make -f coreLibs_kernel.mak ICP_TOOLS_TARGET=$(ICP_TOOLS_TARGET) clean;
	@echo ; echo 'Cleaning QAT access layer';
	@cd $(QAT_AL_PATH) && export ENV_DIR=$(QAT_AL_PATH) && export ICP_ENV_DIR=$(QAT_AL_PATH) && cd src && make target_lib=qatal clean ICP_OS_LEVEL=kernel_space;
	@echo ; echo 'Cleaning icp_crypto';
	@cd $(LAC_PATH) && export ICP_ENV_DIR=$(LAC_PATH) && cd src && make ICP_TOOLS_TARGET=$(ICP_TOOLS_TARGET) ICP_OS_LEVEL=kernel_space ICP_CRYPTO=1 clean;
	@echo ; echo 'Cleaning icp_debug';
	@cd $(DCC_PATH) && export ICP_ENV_DIR=$(ICPSERVICES_PATH) && make clean COMP=dcc ;	

