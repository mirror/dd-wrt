#####
#
# Navigation Makefile, to kick-off a build for all sub-dirs
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

# Ensure The ICP_ENV_DIR environmental var is defined.
ifndef ICP_ENV_DIR
$(error ICP_ENV_DIR is undefined. Please set the path to your environment makefile \
        "-> setenv ICP_ENV_DIR <path>")
endif

#Add your project environment Makefile
include $(ICP_ENV_DIR)/environment.mk


#include the makefile will all the default and common Make variable definitions
#include $(ICP_BUILDSYSTEM_PATH)/build_files/common.mk

##directories
ICP_BUILD_OUTPUT_DIR=build/$(ICP_OS)/$(ICP_OS_LEVEL)/#the folder where the output will be created.
FINAL_OUTPUT_DIR=$(ICP_BUILD_OUTPUT_DIR)

# add the path and list of source libraries,
# you must point to the source files not the build_output_dir e.g library_1/src/test1.a
# ATTENTION: You need to specify full paths (Think about using $(PWD) and the variables defined in your environment.mk)
ADDITIONAL_OBJECTS= $(PWD)/common/admin/$(ICP_BUILD_OUTPUT_DIR)/qatal_admin.a \
					$(PWD)/common/qat_comms/$(ICP_BUILD_OUTPUT_DIR)/qatal_qat_comms.a \
                                        $(PWD)/common/stat/$(ICP_BUILD_OUTPUT_DIR)/qatal_qat_stats.a \
                                        $(PWD)/common/rand/$(ICP_BUILD_OUTPUT_DIR)/qatal_qat_rand.a

SUBDIRS=$(patsubst %/$(ICP_BUILD_OUTPUT_DIR)/,%,$(dir $(ADDITIONAL_OBJECTS)))


####Making a static target library from other libraries in different directories############
ifdef target_lib

OUTPUT_NAME=$(target_lib)
LIB_STATIC=$(OUTPUT_NAME).a

#path to production header files and to add another "-I <path to another directory of include files>"
INCLUDES= -I$(ICP_API_DIR) -I$(ICP_ASD_DIR)/include \
                	   -I$(ICP_OSAL_DIR)/common/include \
					   -I$(ICP_RM_DIR)/access_layer/src/common/rm/include \
					   -I$(ICP_QAT_FW_API_DIR) \
					   -I$(ICP_COMMON_FW_API_DIR) \
					   -I$(ICP_QATAL_DIR)/include \
					   -I$(ICP_QATAL_DIR)/src/common/include

#include your $(ICP_OS)_$(ICP_OS_LEVEL).mk file
include $(ICP_ENV_DIR)/$(ICP_OS)_$(ICP_OS_LEVEL).mk

#list of sources for the target_lib, can be left blank if there are no source files e.g IxUsrNpeLibU2.c
SOURCES=

kernel_module:lib_kernel
	@echo 'Creating kernel module'; \
	echo 'NOTE: Filtering of link-warnings enabled (ix_rm_*, ixOsal*, also WHOLECHIP';\
	make -C $(KERNEL_SOURCE_ROOT)/ M=$(PWD) 2>&1 | grep \
  	     -i -v -e "^\WARNING: \"ixOsal.*\" \[$(PWD)/qatal.ko\] undefined\!" \
         -i -v -e "^\WARNING: \"ix_rm_.*\" \[$(PWD)/qatal.ko\] undefined\!" \
         -i -v -e "^\WARNING: \"icp_Dcc.*\" \[$(PWD)/qatal.ko\] undefined\!" \
         -i -v -e "^\.*\WHOLECHIP\" is not defined" \
         -i -v -e "^\.*\WHOLECHIP is not defined\" " \
         -i -v -e "^\WARNING\: could not find .*"; \
  	echo 'Copying outputs to $(ICP_BUILD_OUTPUT_DIR)";\';\
	test -d $(ICP_BUILD_OUTPUT_DIR) || mkdir -p $(ICP_BUILD_OUTPUT_DIR);\
	test -f lib.a  &&  mv lib.a $(ICP_BUILD_OUTPUT_DIR)/$(LIB_STATIC);
	test -f $(OUTPUT_NAME).ko  &&  mv -f $(OUTPUT_NAME).ko $(ICP_BUILD_OUTPUT_DIR);\
	test -f $(OUTPUT_NAME).o  &&  mv -f *.o $(ICP_BUILD_OUTPUT_DIR);\
	$(RM) -rf *.mod.* .*.cmd; \



obj-m := $(target_lib).o
$(target_lib)-objs := $(subst $(PWD)/, ,$(ADDITIONAL_OBJECTS))\
		linux/qatal_module.o \
		linux/qatal_symbols.o \
		$(ICP_TOOLS_PATH)/lib_linux_le/$(ICP_TOOLS_TARGET)/halAe_kernel.a \
		$(ICP_TOOLS_PATH)/lib_linux_le/uclo_kernel.a
lib-m := $(subst $(PWD)/, ,$(ADDITIONAL_OBJECTS))

lib_kernel::clean
	@for dir in  $(SUBDIRS); do \
	(echo ; echo $$dir :; cd $$dir; \
		($(MAKE) clean && $(MAKE)) || return 1) \
	done


lib::clean
	@for dir in  $(SUBDIRS); do \
	(echo ; echo $$dir :; cd $$dir; \
		$(MAKE) || return 1) \
	done\

lib:: dirs $(LIB_STATIC)

.PHONY: lib
ifeq ($(ICP_OS_LEVEL),kernel_space)

lib_static: $(LIB_STATIC)

dirs:
	@echo 'Creating output directory' ;\
	test -d $(FINAL_OUTPUT_DIR) || mkdir -p $(FINAL_OUTPUT_DIR);

$(LIB_STATIC): lib_kernel dirs
	@echo 'Creating static library ${LIB_STATIC}'; \
	make -C $(KERNEL_SOURCE_ROOT)/ M=$(PWD) obj-m=""; \
	echo 'Copying outputs';\
	test -f lib.a  &&  mv lib.a $(FINAL_OUTPUT_DIR)/$(LIB_STATIC);\
	$(RM) -rf *.mod.* .*.cmd;
endif
ifneq ($(ICP_OS_LEVEL),kernel_space)
include $(ICP_BUILDSYSTEM_PATH)/build_files/rules.mk
include $(ICP_BUILDSYSTEM_PATH)/build_files/depend.mk
endif

include $(ICP_BUILDSYSTEM_PATH)/build_files/Core/$(ICP_CORE).mk
include $(ICP_BUILDSYSTEM_PATH)/build_files/OS/$(ICP_OS).mk
endif
#####################################################################################


.DEFAULT: all

all depend install run_coding_standards :
	$(LOOP)

clean:
	@echo 'Removing derived objects...'; \
	$(RM) -rf *.o *.cov *.a *.so* *.mod.* *.ko .*.cmd; \
	$(RM) -rf .tmp_versions; \
	$(RM) -rf $(ICP_BUILD_OUTPUT_DIR);
	@for dir in  $(SUBDIRS); do \
		(echo ; echo $$dir :; cd $$dir; \
	($(MAKE) clean) || return 1) \
	done

