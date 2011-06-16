######
#
# Makefile Common Definitions for the Tolapai build system 
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
#####

######Support $(PROG_ACY) and previous vars#################################
PROG_ACY?=ICP
ifdef $(PROG_ACY)_KERNEL_SOURCE_ROOT
KERNEL_SOURCE_ROOT=$($(PROG_ACY)_KERNEL_SOURCE_ROOT)
endif
$(PROG_ACY)_CORE?=ia
$(PROG_ACY)_OS?=linux_2.6
####################################################################

#Check to ensure that the BUILDSYSTEM is defined, it should point to the top of the build dir structure where build_files  are located
ifndef $(PROG_ACY)_BUILDSYSTEM_PATH
$(error $(PROG_ACY)_BUILDSYSTEM_PATH is undefined. Please set the path to the top of the build structure \
	"-> setenv BUILDSYSTEM_PATH <path>")
endif

# Ensure the ENV_DIR environmental var is defined.
ifndef $(PROG_ACY)_ENV_DIR
$(error $(PROG_ACY)_ENV_DIR is undefined. Please set the path to your environment makefile \
        "-> setenv $(PROG_ACY)_ENV_DIR <path>")
endif

#Add your project environment Makefile
include $($(PROG_ACY)_ENV_DIR)/environment.mk

# Compiler, Linker and Archive commands are defined in the core specific makefile
CC=$(COMPILER)
LD=$(LINKER)
AR=$(ARCHIVER)

EXTRA_CFLAGS+=-O2


PWD= $(shell pwd)

##directories
# path where to store all the build outputs
# only this one can be overwritten by users.
# This way, and with the 3 variables, we can prevent any rm -rf `pwd`
# when doing `make clean or distclean'
$(PROG_ACY)_BUILD_OUTPUT_DIR?=$(PWD)
$(PROG_ACY)_MID_OUTPUT_DIR=$($(PROG_ACY)_BUILD_OUTPUT_DIR)/build
$(PROG_ACY)_FINAL_OUTPUT_DIR=$($(PROG_ACY)_MID_OUTPUT_DIR)/$($(PROG_ACY)_OS)/$($(PROG_ACY)_OS_LEVEL)/
FINAL_OUTPUT_DIR=$($(PROG_ACY)_FINAL_OUTPUT_DIR)

# Defines a loop macro for making subdirectories
define LOOP
@for dir in $(SUBDIRS); do \
	(echo ; echo $$dir :; cd $$dir && \
		$(MAKE) $@ || return 1) \
	done
endef

include $($(PROG_ACY)_BUILDSYSTEM_PATH)/build_files/Core/$($(PROG_ACY)_CORE).mk
include $($(PROG_ACY)_BUILDSYSTEM_PATH)/build_files/OS/$($(PROG_ACY)_OS).mk

