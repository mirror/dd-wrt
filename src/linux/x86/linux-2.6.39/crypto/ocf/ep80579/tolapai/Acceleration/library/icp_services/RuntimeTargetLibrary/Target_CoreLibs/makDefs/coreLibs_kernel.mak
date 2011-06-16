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

include ../makDefs/commonMak.def

HALAE_DIR 	= ..$(DIR_SEP)halAe
HALAETGT_DIR 	= $(HALAE_DIR)$(DIR_SEP)$(ICP_TOOLS_TARGET)
UCLO_DIR	 	= ..$(DIR_SEP)uclo
HAVE_HALDRV	= $(wildcard $(BIN_DIR)/$(ICP_TOOLS_TARGET)/icp_hal.ko)
HAVE_HALLIB	= $(wildcard $(LIB_DIR)/$(ICP_TOOLS_TARGET)/halAe_kernel.a)
HAVE_UCLOLIB	= $(wildcard $(LIB_DIR)/uclo_kernel.a)

all : driver

driver :
ifeq ($(SYS),LINUX)
	-@ $(MKDIR) "$(BIN_DIR)"
	-@ $(MKDIR) "$(BIN_DIR)/$(ICP_TOOLS_TARGET)"
	-@ $(MKDIR) "$(LIB_DIR)"
	-@ $(MKDIR) "$(LIB_DIR)/$(ICP_TOOLS_TARGET)"
	@cd $(HALAETGT_DIR) $(CMD_CAT) make $(KVER_OPTION)
	mv $(HALAETGT_DIR)/icp_hal.ko $(BIN_DIR)/$(ICP_TOOLS_TARGET)/icp_hal.ko
	mv $(HALAETGT_DIR)/lib.a $(LIB_DIR)/$(ICP_TOOLS_TARGET)/halAe_kernel.a
	@cd $(UCLO_DIR) $(CMD_CAT) make $(KVER_OPTION)
	mv $(UCLO_DIR)/lib.a $(LIB_DIR)/uclo_kernel.a	
endif

clean :
ifeq ($(SYS),LINUX)
	@cd $(HALAETGT_DIR) $(CMD_CAT) make clean
	@cd $(UCLO_DIR) $(CMD_CAT) make clean
ifneq ($(strip $(HAVE_HALDRV)),)	
	rm $(BIN_DIR)/$(ICP_TOOLS_TARGET)/icp_hal.ko  
endif
ifneq ($(strip $(HAVE_HALLIB)),)	
	rm $(LIB_DIR)/$(ICP_TOOLS_TARGET)/halAe_kernel.a
endif
ifneq ($(strip $(HAVE_UCLOLIB)),)	
	rm $(LIB_DIR)/uclo_kernel.a
endif
	
endif
