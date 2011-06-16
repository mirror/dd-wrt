#
# Top-level Look-Aside Crypto makefile, to kick-off a build for all sub-dirs
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
###############################################################################

# Ensure The ICP_ENV_DIR environmental var is defined.
ifndef ICP_ENV_DIR
$(error ICP_ENV_DIR is undefined. Please set the path to your environment makefile \
        "-> setenv ICP_ENV_DIR <path>")
endif

#Add your project environment Makefile
include $(ICP_ENV_DIR)/environment.mk
include $(ICP_COMMON_ENV_DIR)/common_env.mk

ICP_TOOLS_TARGET?=EP80579

#KBUILD_EXTRA_SYMBOLS=\
#        $(ICP_DCC_DIR)/source/Module.symvers \
#        $(ICP_TOOLS_XSC_CORE_ROOT)/halAe/$(ICP_TOOLS_TARGET)/Module.symvers
KBUILD_EXTRA_SYMBOLS=\
        $(ICP_TOOLS_XSC_CORE_ROOT)/halAe/$(ICP_TOOLS_TARGET)/Module.symvers

##directories
ICP_BUILD_OUTPUT_DIR=build/$(ICP_OS)/$(ICP_OS_LEVEL)/#the folder where the output will be created.
FINAL_OUTPUT_DIR=$(ICP_BUILD_OUTPUT_DIR)


ifdef ICP_CRYPTO
OUTPUT_NAME=icp_crypto
QATAL_SYMBOLS=$(RELATIVE_QATAL_PATH)/linux/qatal_symbols.c
QATAL_LIBS=$(RELATIVE_OSAL_PATH)/lib/$(IX_DEVICE)/linux/linuxle/libosal.a\
                 $(RELATIVE_QATAL_PATH)/common/admin/build/linux_2.6/kernel_space/qatal_admin.a\
                 $(RELATIVE_QATAL_PATH)/common/qat_comms/build/linux_2.6/kernel_space/qatal_qat_comms.a\
                 $(RELATIVE_QATAL_PATH)/common/rand/build/linux_2.6/kernel_space/qatal_qat_rand.a\
                 $(RELATIVE_QATAL_PATH)/common/stat/build/linux_2.6/kernel_space/qatal_qat_stats.a\
		 $(RELATIVE_HAL_PATH)/lib_linux_le/$(ICP_TOOLS_TARGET)/halAe_kernel.a\
	         $(RELATIVE_HAL_PATH)/lib_linux_le/uclo_kernel.a

include $(ICP_BUILDSYSTEM_PATH)/build_files/includes.mk

endif

OUTPUT_NAME?=lac
LIB_STATIC=$(OUTPUT_NAME).a

#include your $(ICP_OS)_$(ICP_OS_LEVEL).mk file
include $(ICP_ENV_DIR)/$(ICP_OS)_$(ICP_OS_LEVEL).mk

MODULE_SOURCES=linux/lac_module.c\
    linux/lac_symbols.c\
    linux/proc/lac_proc_common.c\
    linux/proc/lac_proc_dh.c\
    linux/proc/lac_proc_dsa.c\
    linux/proc/lac_proc_key.c\
    linux/proc/lac_proc_ln.c\
    linux/proc/lac_proc_prime.c\
    linux/proc/lac_proc_rand.c\
    linux/proc/lac_proc_rsa.c\
    linux/proc/lac_proc.c\
    linux/proc/lac_proc_sym.c\
    linux/proc/lac_proc_qat.c\
    $(QATAL_SYMBOLS)

# add the path and list of source libraries,
ADDITIONAL_KERNEL_LIBS= common/lac_common/$(ICP_BUILD_OUTPUT_DIR)/lac_cmn.a \
			common/utils/$(ICP_BUILD_OUTPUT_DIR)/utils.a \
			common/init/$(ICP_BUILD_OUTPUT_DIR)/init.a\
			common/sym/$(ICP_BUILD_OUTPUT_DIR)/sym.a \
			common/sym/qat/$(ICP_BUILD_OUTPUT_DIR)/sym_qat.a \
			common/sym/key/$(ICP_BUILD_OUTPUT_DIR)/sym_key.a\
			common/sym/random/$(ICP_BUILD_OUTPUT_DIR)/random.a \
			common/asym/pke_common/$(ICP_BUILD_OUTPUT_DIR)/pke_common.a\
			common/asym/mod_exp/$(ICP_BUILD_OUTPUT_DIR)/mod_exp.a\
			common/asym/diffie_hellman/$(ICP_BUILD_OUTPUT_DIR)/diffie_hellman.a\
			common/asym/rsa/$(ICP_BUILD_OUTPUT_DIR)/rsa.a\
			common/asym/dsa/$(ICP_BUILD_OUTPUT_DIR)/dsa.a\
			common/asym/prime/$(ICP_BUILD_OUTPUT_DIR)/prime.a\
			common/asym/large_number/$(ICP_BUILD_OUTPUT_DIR)/ln.a\


SUBDIRS=$(patsubst %/$(ICP_BUILD_OUTPUT_DIR)/,%,$(dir $(ADDITIONAL_KERNEL_LIBS)))


kernel_module: $(KBUILD_EXTRA_SYMBOLS) lib_kernel
	@echo 'Creating kernel module'; \
	echo '*** NOTE: Filtering of link-warnings enabled (ixOsal, QatComms) ***';\
	make -C $(KERNEL_SOURCE_ROOT)/ M=$(PWD) 2>&1 | grep \
        -i -v -e "^\*\*\* Warning: \"ixOsal.*\" \[$(PWD)/lac.ko\] undefined\!" \
        -i -v -e "^\*\*\* Warning: \"QatComms.*\" \[$(PWD)/lac.ko\] undefined\!" \
		-i -v -e "^\*\*\* Warning: \"icp_Qatal.*\" \[$(PWD)/lac.ko\] undefined\!"; \
	echo "Copying outputs to $(ICP_BUILD_OUTPUT_DIR)";\
	test -d $(ICP_BUILD_OUTPUT_DIR) || mkdir -p $(ICP_BUILD_OUTPUT_DIR);\
	test -f lib.a  &&  mv lib.a $(ICP_BUILD_OUTPUT_DIR)/$(LIB_STATIC);\
	test -f $(OUTPUT_NAME).ko  &&  mv -f $(OUTPUT_NAME).ko $(ICP_BUILD_OUTPUT_DIR);\
	test -f $(OUTPUT_NAME).o  &&  mv -f *.o $(ICP_BUILD_OUTPUT_DIR);\
	$(RM) -rf *.mod.* .*.cmd;

obj-m := $(OUTPUT_NAME).o
$(OUTPUT_NAME)-objs := $(ADDITIONAL_KERNEL_LIBS) $(QATAL_LIBS)\
		$(patsubst %.c,%.o, $(MODULE_SOURCES))
lib-m := $(ADDITIONAL_KERNEL_LIBS)


lib_kernel::clean
	@for dir in  $(SUBDIRS); do \
	(echo ; echo $$dir :; cd $$dir; \
		($(MAKE) clean && $(MAKE)) || return 1) \
	done


lib::clean
	@for dir in  $(SUBDIRS); do \
	(echo ; echo $$dir :; cd $$dir; \
		$(MAKE) || return 1) \
	done


lib:: $(LIB_STATIC)

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
	$(RM) -f *.o *.mod.* .*.cmd;
endif


ifneq ($(ICP_OS_LEVEL),kernel_space)
include $(ICP_BUILDSYSTEM_PATH)/build_files/rules.mk
endif

include $(ICP_BUILDSYSTEM_PATH)/build_files/Core/$(ICP_CORE).mk
include $(ICP_BUILDSYSTEM_PATH)/build_files/OS/$(ICP_OS).mk
#####################################################################################


.DEFAULT: all

all:
	$(LOOP)

clean:
	@echo 'Removing derived objects...'; \
	$(RM) -rf *.o *.a *.mod.* *.ko .*.cmd Module.symvers modules.order ; \
	$(RM) -rf .tmp_versions; \
	$(RM) -rf $(ICP_BUILD_OUTPUT_DIR);
	@for dir in  $(SUBDIRS); do \
		(echo ; echo $$dir :; cd $$dir; \
	($(MAKE) clean) || return 1) \
	done
