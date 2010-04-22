# 
# @par
# IXP400 SW Release Crypto version 2.1
# 
# -- Copyright Notice --
# 
# @par
# Copyright (c) 2001-2005, Intel Corporation.
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
# Change the following line to point to your linux kernel source tree
linuxbe_KERNEL_DIR=/path/to/the/be/os/linux-2.6
linuxle_KERNEL_DIR=/path/to/the/le/os/linux-2.6

# Change the following line to point to the root of your Monta Vista install
HARDHAT_BASE=/opt/montavista/pro

# You may want to edit the following to point to your ixp400_xscale_sw
# directory. Otherwise you must source this file from that directory.
IX_XSCALE_SW=/path/to/the/ixp400_xscale_sw

PATH=/usr/bin:$HARDHAT_BASE/host/bin:$HARDHAT_BASE/devkit/arm/xscale_be/bin:$HARDHAT_BASE/devkit/arm/xscale_le/bin:$PATH

#NOTE: All flag definitions below are case sensitive

# For IXDP425 platform, set IX_DEVICE to ixp42X
# For IXDP465 platform, set IX_DEVICE to ixp42X or ixp46X
IX_DEVICE=ixp42X

# For IXDP425 platform, set IX_PLATFORM to ixdp42x
# For IXDP465 platform, set IX_PLATFORM to ixdp46x
IX_PLATFORM=ixdp42x

# For Big endian, set IX_TARGET to linuxbe
# For Little endian, set IX_TARGET to linuxle
IX_TARGET=linuxbe

IX_LINUXVER=2.6

export linuxbe_KERNEL_DIR linuxle_KERNEL_DIR HARDHAT_BASE IX_XSCALE_SW PATH IX_DEVICE IX_PLATFORM IX_TARGET
export IX_LINUXVER
