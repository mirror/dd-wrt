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
#ICP_ENV_DIR=/$(ICP_ROOT)/infrastructure/access_layer

ifndef ICP_ENV_DIR
$(error ICP_ENV_DIR is undefined. Please set the path to your environment makefile \
        "-> setenv ICP_ENV_DIR <path>")
endif

#Add your project environment Makefile
include $(ICP_ENV_DIR)/environment.mk
include $(ICP_COMMON_ENV_DIR)/common_env.mk

#include the makefile with all the default and common Make variable definitions
include $(ICP_BUILDSYSTEM_PATH)/build_files/common.mk

#include the version file that contains the package version information
include $(VERSION_FILE)

MIL_DIR?=$(ICP_ENV_DIR)/src/common/mil

ifeq ($(ICP_OS_LEVEL),user_space)
OUTPUT_NAME=debugmgr
else 
OUTPUT_NAME=icp_debugmgmt
endif

# List of Source Files to be compiled (to be in a single line or on different lines separated by a "\" and tab.
ifeq ($(ICP_OS_LEVEL),user_space)
ifdef ICP_DEBUG
CFLAGS += -DMIL_DEBUG
endif

#Enable all warnings
CFLAGS += -Wall
#Add the name for the executable, Library or Module output definitions

CFLAGS += -DIOSTYLE=HARDWARE -D__LITTLE_ENDIAN -D__BYTE_ORDER=__LITTLE_ENDIAN 
SOURCES = mil_exe/mil_user_interface.c \
	mil_exe/mil_kernel_interface.c \
	mil_exe/mil_file_interface.c

else #ICP_OS_LEVEL

ifdef ICP_DEBUG
EXTRA_CFLAGS += -DMIL_DEBUG
endif

#Enable all warnings
EXTRA_CFLAGS += -Wall
#Add the name for the executable, Library or Module output definitions

EXTRA_CFLAGS += -DIOSTYLE=HARDWARE -D__LITTLE_ENDIAN -D__BYTE_ORDER=__LITTLE_ENDIAN 
SOURCES = mil_drv/mil_linux_kernel.c \
	mil_drv/mil_internal.c
endif

KBUILD_EXTRA_SYMBOLS=\
	$(ICP_DCC_DIR)/source/Module.symvers


#common includes between all supported OSes
ifeq ($(ICP_OS_LEVEL),user_space)
INCLUDES= -I$(MIL_DIR)/include \
          -I$(DCC_DIR)/include \
          -I$(ASD_PATH)/include \
          -I$(EXTERNAL_INTERFACE_API_INCLUDE)
else
INCLUDES= -I$(DCC_DIR)/include \
          -I$(ASD_PATH)/include \
          -I$(EXTERNAL_INTERFACE_API_INCLUDE) \
          -I$(MIL_DIR)/include
endif


ifneq ($(ICP_OS_LEVEL),user_space)
#include your $(ICP_OS)_$(ICP_OS_LEVEL).mk file
include $(ICP_ENV_DIR)/$(ICP_OS)_$(ICP_OS_LEVEL).mk
endif

# On the line directly below list the outputs you wish to build for,
# e.g "lib_static lib_shared exe module" as show below
install: lib_static
###################Include rules makefiles########################
include $(ICP_BUILDSYSTEM_PATH)/build_files/rules.mk
###################End of Rules inclusion#########################
