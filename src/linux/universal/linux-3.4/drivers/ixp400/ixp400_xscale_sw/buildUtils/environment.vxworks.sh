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
# Tornado environment settings. Edit these to reflect your installation.
WIND_BASE=/apps/tornado
WIND_HOST_BASE=$WIND_BASE

# The location of the ixp400_xscale_sw directory (needed to build the vxWorks BSP)
CSR_BASE=/change/this/to/your/ixp400_xscale_sw

#NOTE: All flag definitions below are case sensitive

# For IXDP425 platform, set IX_DEVICE to ixp42X
# For KIXRP435 platform, set IX_DEVICE to ixp43X
# For IXDP465 platform, set IX_DEVICE to ixp46X
IX_DEVICE=ixp42X

# For IXDP425 platform, set IX_PLATFORM to ixdp42x
# For KIXRP435 platform, set IX_PLATFORM to kixrp43x
# For IXDP465 platform, set IX_PLATFORM to ixdp46x
IX_PLATFORM=ixdp42x

# For Big endian, set IX_TARGET to vxbe
# For Little endian, set IX_TARGET to vxle
IX_TARGET=vxbe

# For vxWorks 5.5.1, set VX_VERSION to vx55. Otherwise comment it out.
VX_VERSION=vx55

export WIND_BASE WIND_HOST_BASE CSR_BASE IX_DEVICE IX_PLATFORM IX_TARGET
export VX_VERSION

