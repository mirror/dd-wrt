#
# Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer,
#    without modification.
# 2. Redistributions in binary form must reproduce at minimum a disclaimer
#    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
#    redistribution must be conditioned upon including a substantially
#    similar Disclaimer requirement for further binary redistribution.
# 3. Neither the names of the above-listed copyright holders nor the names
#    of any contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# Alternatively, this software may be distributed under the terms of the
# GNU General Public License ("GPL") version 2 as published by the Free
# Software Foundation.
#
# NO WARRANTY
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES.
#
# $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/hal/main/public/mipsisa32-be-elf.inc#3 $
#
ifndef TOOLPREFIX
TOOLPREFIX=	mips-linux-uclibc-
endif

CC=	${TOOLPREFIX}gcc
LD=	${TOOLPREFIX}ld
STRIP=	${TOOLPREFIX}strip
OBJCOPY=${TOOLPREFIX}objcopy
NM=	${TOOLPREFIX}nm


#LDOPTS+=	-EB
LDOPTS+=	-m elf64btsmip


COPTS+=	-DAH_WORDSIZE=64
COPTS+=	-DAH_BYTE_ORDER=AH_BIG_ENDIAN 
COPTS+= -DBOESE=1 -D__LINUX_MIPS64_ARCH__
#COPTS+=	-G 0 -mno-abicalls -fno-pic -march=octeon -mabi=64 -Wa,--trap  -D__LINUX_MIPS64_ARCH__ -fno-strict-aliasing -fno-common -fomit-frame-pointer -mlong-calls -isystem ${KERNELPATH}/include -O2 -ffreestanding
COPTS+=	-G 0 -mno-abicalls -fno-pic -march=octeon -mabi=64 -Wa,--trap -fno-strict-aliasing -fno-common -fomit-frame-pointer -mlong-calls -msoft-float -DBOESE=1 -Os
#COPTS+= -DLED1_PIN=0 -DLED2_PIN=1 -DLED3_PIN=6 -DLED4_PIN=7


AH_SUPPORT_AR5210=1
AH_SUPPORT_AR5211=1
AH_SUPPORT_AR5212=1
AH_SUPPORT_AR5312=0
AH_SUPPORT_AR7100=1
AH_SUPPORT_5111=1
AH_SUPPORT_5112=1
AH_SUPPORT_2413=1
AH_SUPPORT_5413=1
AH_SUPPORT_2316=0
AH_SUPPORT_2317=0
#AH_SUPPORT_DFS=1
AH_NEED_DESC_SWAP=0

AH_PRIVATE_DIAG=0
AH_REGOPS_FUNC=0
AH_SUPPORT_2417=0      	# 2417 RF support