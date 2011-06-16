######
#
# Makefile Linux 2.6 Kernel Specific Definitions for the Tolapai build system 
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
######

$(info Building for linux_2.6)


INCLUDES+=-I ./#path to production header files and to add another "-I <path to another directory of include files>"


ifeq ($($(PROG_ACY)_OS_LEVEL), user_space)
CFLAGS+=-fPIC $(DEBUGFLAGS) -Wall -Wpointer-arith -O2 $(INCLUDES)
endif


ifeq ($($(PROG_ACY)_OS_LEVEL), kernel_space)
CFLAGS+= 
endif


ifeq ($($(PROG_ACY)_EXTRA_WARNINGS),y)
EXTRA_CFLAGS+= -Wno-div-by-zero -Wfloat-equal -Wtraditional  -Wundef -Wno-endif-labels \
      -Wshadow -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsign-compare -Waggregate-return \
      -Wstrict-prototypes  -Wmissing-prototypes -Wmissing-declarations  -Wmissing-noreturn \
      -Wmissing-format-attribute -Wno-multichar -Wno-deprecated-declarations -Wpacked -Wpadded -Wredundant-decls -Wnested-externs -Wunreachable-code \
      -Winline -Wlong-long -Wdisabled-optimization 

## unrecognized options
## -Wextra -Wdeclaration-after-statement -Wlarger-than-len -Wold-style-definition -Wmissing-field-initializers -Winvalid-pch
## -Wvariadic-macros -Wno-pointer-sign
endif


LIB_SHARED_FLAGS=-shared -soname $(LIB_SHARED)
LIB_STATIC_FLAGS=
EXE_FLAGS=


