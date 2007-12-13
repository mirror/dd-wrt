# 
# @par
# IXP400 SW Release version 2.4
# 
# -- Copyright Notice --
# 
# @par
# Copyright (c) 2001-2007, Intel Corporation.
# All rights reserved.
# 
# @par
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Intel Corporation nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# 
# @par
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 
# 
# @par
# -- End of Copyright Notice --

codelets_ethAcc_OBJ := IxEthAccCodeletMain.o \
	IxEthAccCodeletBufMan.o \
	IxEthAccCodeletDispatcher.o \
	IxEthAccCodeletPortSetup.o \
	IxEthAccCodeletLinkSetup.o \
	IxEthAccCodeletLoopbacks.o \
	IxEthAccCodeletDbLearning.o \
	IxEthAccCodeletSwBridge.o \
	IxEthAccCodeletSwBridgeFirewall.o \
	IxEthAccCodeletSwBridgeQoS.o \
	IxEthAccCodeletSwBridgeWiFi.o

codelets_ethAcc_CFLAGS := -Isrc/ethAcc -Isrc/ethAcc/include
codelets_ethAcc_test_DEPS := ethAcc ethDB ethMii hssAcc qmgr npeMh npeDl featureCtrl

ifneq ($(IX_DEVICE), ixp42X)
codelets_ethAcc_test_DEPS += errHdlAcc parityENAcc
endif

ifeq ($(IX_PLATFORM),kixrp43x)
codelets_ethAcc_CFLAGS += -D__kixrp435
endif

ifeq ($(IX_TARGET_OS), linux)
codelets_ethAcc_OBJ += IxEthAccCodeletSymbols.o
codelets_ethAcc_EXPORT_OBJ := IxEthAccCodeletSymbols.o
endif

