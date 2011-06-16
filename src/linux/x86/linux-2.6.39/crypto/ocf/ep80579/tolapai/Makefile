###############################################################################
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
###############################################################################

# Ensure The ICP_ROOT var is defined.
export ICP_ROOT := $(shell pwd)
#export ICP_BUILDSYSTEM_PATH := $(shell pwd)
ifndef ICP_ROOT
$(error ICP_ROOT is undefined. Please set the path to the code \
        "-> setenv ICP_ROOT <path to code>")
endif

DATE=$(shell date '+%m_%d_%y')
ICP_BUILD_OUTPUT?=$(ICP_ROOT)/build_$(DATE)
INSTALL_SCRIPT=install_security.sh

all: Accel


Accel:
	@cd Acceleration/ && make ICP_BUILD_OUTPUT=$(ICP_BUILD_OUTPUT);

Embed:
	@cd Embedded/src/GbE && make ICP_ENV_DIR=$(ICP_ROOT)/Embedded/ BUILD_OUTPUT=$(ICP_BUILD_OUTPUT);
	@test -d $(ICP_BUILD_OUTPUT) || mkdir $(ICP_BUILD_OUTPUT);
	@cp -f Embedded/src/GbE/build/linux_2.6/kernel_space/gcu.ko $(ICP_BUILD_OUTPUT)/
	@cp -f Embedded/src/GbE/build/linux_2.6/kernel_space/iegbe.ko $(ICP_BUILD_OUTPUT)/

clean: Embed_clean Accel_clean 
	@test $(ICP_BUILD_OUTPUT) && rm -rf $(ICP_BUILD_OUTPUT)
	
Embed_clean:
	@test $(ICP_BUILD_OUTPUT) && rm -rf $(ICP_BUILD_OUTPUT)/gcu.ko && rm -rf $(ICP_BUILD_OUTPUT)/iegbe.ko;
	@cd Embedded/src/GbE && make ICP_ENV_DIR=$(ICP_ROOT)/Embedded/ clean;
	
Accel_clean:
	@test $(ICP_BUILD_OUTPUT) && rm -rf $(ICP_BUILD_OUTPUT)/icp*.ko && rm -rf $(ICP_BUILD_OUTPUT)/asd_ctl && rm -rf $(ICP_BUILD_OUTPUT)/debugmgr && rm -rf $(ICP_BUILD_OUTPUT)/*firmware.bin && rm -rf $(ICP_BUILD_OUTPUT)/$(INSTALL_SCRIPT);
	@cd Acceleration/ && make clean;
	
install: Accel_install Embed_install

Accel_install:
	@echo 'Installing modules';
	$(ICP_BUILD_OUTPUT)/$(INSTALL_SCRIPT) install $(ICP_BUILD_OUTPUT);
	
Embed_install:
	@cd $(ICP_ROOT)/Embedded/src/GbE && make ICP_ENV_DIR=$(ICP_ROOT)/Embedded/ install;
	
uninstall: Accel_uninstall Embed_uninstall
	@echo 'Un-installing modules';
	
Accel_uninstall:
	$(ICP_BUILD_OUTPUT)/$(INSTALL_SCRIPT) uninstall ;

Embed_uninstall:
	@cd $(ICP_ROOT)/Embedded/src/GbE && make ICP_ENV_DIR=$(ICP_ROOT)/Embedded/ uninstall;
