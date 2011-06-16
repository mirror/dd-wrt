###################
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
###################

include $(OSAL_CONFIG_MK) 

#specific include directories in kernel space

INCLUDES+= $(OSAL_INCLUDES) \
	-I$(ICP_OSAL_DIR)/common/include \
	-I$(ICP_IX_TOOLS_DIR)/include/os/linux \
	-I$(ICP_IX_TOOLS_XSC_CORE_DIR)/halAe/include/linux \
	-I$(ICP_ASD_DIR)/include \
	-I$(ICP_API_DIR)/dcc \
	-I$(ICP_OSSL_DIR)/linux_kernel/include \
	-I$(ICP_RM_DIR)/access_layer/include \
	-I$(ICP_RM_DIR)/access_layer/src/tolapai/rm \
	-I$(ICP_RM_DIR)/access_layer/src/common/rm/include \
	-I$(ICP_DCC_DIR)/include \
	-I$(ICP_ME_ACCESS_LAYER_COMMON_DIR)/include \
	-I$(ICP_IX_TOOLS_DIR)/include/

INCLUDES+= -I$(ICP_API_DIR) \
	-I$(ICP_QATAL_DIR)/include \
	-I$(ICP_QATAL_DIR)/src/common/include \
	-I$(ICP_QAT_FW_API_DIR) \
	-I$(ICP_COMMON_FW_API_DIR)

#Extra Flags Specific in kernel space e.g. include path or debug flags etc. e.g to add an include path EXTRA_CFLAGS += -I$(src)/../include
EXTRA_CFLAGS += -D_IX_HARDWARE_TYPE_=_IX_HW_TOLAPAI_ -D ENABLE_SPINLOCK $(INCLUDES) -D_IX_RM_IMPL_HARDWARE_ -D_IX_RM_MULTI_THREADED_ -DIOSTYLE=HARDWARE -D__LITTLE_ENDIAN -D__BYTE_ORDER=__LITTLE_ENDIAN
EXTRA_LDFLAGS +=-whole-archive
