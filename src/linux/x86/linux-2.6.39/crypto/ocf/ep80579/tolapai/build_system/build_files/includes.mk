####################
#  Include paths
#
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
#
####################


INCLUDES=-I$(ICP_ROOT)/Acceleration/drivers/icp_asd/include \
 -I$(ICP_ROOT)/Acceleration/drivers/icp_asd/src/kernel/include \
 -I$(ICP_ROOT)/Acceleration/include \
 -I$(ICP_ROOT)/Acceleration/include/dcc \
 -I$(ICP_ROOT)/Acceleration/include/lac \
 -I$(ICP_ROOT)/Acceleration/library/common/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/src/common/asym/diffie_hellman \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/src/common/asym/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/src/common/asym/rsa \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/src/common/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/src/common/sym/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/look_aside_crypto/src/linux/proc \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/QATAL/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/QATAL/src/common/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/QAT_FW/common/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_crypto/QAT_FW/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_debug/DCC/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_debugmgmt/MIL/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/include/rm \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/include/rm/internal \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/ResourceManager/rm \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/include/os/linux \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/Target_CoreLibs/halAe \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/Target_CoreLibs/halAe/include/linux \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/Target_CoreLibs/uclo \
 -I$(ICP_ROOT)/Acceleration/library/icp_services/RuntimeTargetLibrary/Target_CoreLibs/uclo/include/linux \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/include/modules/bufferMgt \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/include/modules/ddk \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/include/modules/ioMem \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/os/linux/include/core \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/os/linux/include/modules/bufferMgt \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/os/linux/include/modules/ddk \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/common/os/linux/include/modules/ioMem \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/platforms/EP805XX/include \
 -I$(ICP_ROOT)/Acceleration/library/icp_utils/OSAL/platforms/EP805XX/os/linux/include \
 -I$(ICP_ROOT)/Acceleration/shims/OCF_Shim/src \
 -I$(ICP_ROOT)/Embedded/src/1588 \
 -I$(ICP_ROOT)/Embedded/src/CAN \
 -I$(ICP_ROOT)/Embedded/src/EDMA \
 -I$(ICP_ROOT)/Embedded/src/EDMA/os \
 -I$(ICP_ROOT)/Embedded/src/GbE \
 -I$(ICP_ROOT)/Embedded/src/GPIO \
 -I$(ICP_ROOT)/Embedded/src/WDT 
 
ifdef ICP_OCF_SRC_DIR
INCLUDES+= -I$(ICP_OCF_SRC_DIR)
endif

EXTRA_CFLAGS+=$(INCLUDES)
