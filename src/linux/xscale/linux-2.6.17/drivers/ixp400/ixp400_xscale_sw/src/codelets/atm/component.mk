# 
# @par
# IXP400 SW Release version  2.1
# 
# -- Intel Copyright Notice --
# 
# @par
# Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
# 
# @par
# The source code contained or described herein and all documents
# related to the source code ("Material") are owned by Intel Corporation
# or its suppliers or licensors.  Title to the Material remains with
# Intel Corporation or its suppliers and licensors.
# 
# @par
# The Material is protected by worldwide copyright and trade secret laws
# and treaty provisions. No part of the Material may be used, copied,
# reproduced, modified, published, uploaded, posted, transmitted,
# distributed, or disclosed in any way except in accordance with the
# applicable license agreement .
# 
# @par
# No license under any patent, copyright, trade secret or other
# intellectual property right is granted to or conferred upon you by
# disclosure or delivery of the Materials, either expressly, by
# implication, inducement, estoppel, except in accordance with the
# applicable license agreement.
# 
# @par
# Unless otherwise agreed by Intel in writing, you may not remove or
# alter this notice or any other notice embedded in Materials by Intel
# or Intel's suppliers or licensors in any way.
# 
# @par
# For further details, please see the file README.TXT distributed with
# this software.
# 
# @par
# -- End Intel Copyright Notice --
codelets_atm_test_DEPS :=  \
	atmm \
	atmsch \
	atmdAcc \
	npeMh \
	npeDl \
	qmgr \
        featureCtrl

codelets_atm_OBJ :=  \
	IxAtmCodelet.o \
	IxAtmCodeletRxTx.o \
	IxAtmCodeletSwLoopback.o \
	IxAtmCodeletMain.o \
	IxAtmUtilsAtm.o \
	IxAtmUtilsBufMan.o \
	IxOamCodelet.o

codelets_atm_CFLAGS := \
	-Isrc/include \
	-Isrc/codelets/atm \

# Uncomment in order to use the Validation Platform 
# codelets_atm_CFLAGS += -DVALIDATION_PLATFORM_USED=1

ifeq ($(IX_TARGET_OS), linux)
codelets_atm_OBJ += IxAtmCodeletSymbols.o
codelets_atm_EXPORT_OBJ := IxAtmCodeletSymbols.o
endif

