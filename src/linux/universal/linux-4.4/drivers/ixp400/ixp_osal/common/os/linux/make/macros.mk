#
# Macro definitions for os-specific makefile
#
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
#
# NOTE!
# This file currently depends on the following environment variables:
# - HARDHAT_BASE
# - IX_TARGET
# - linuxbe_KERNEL_DIR or linuxle_KERNEL_DIR


#####################################################################
# Determine the build host OS
#
# Only Linux (and partially Cygwin) are currently supported for Linux builds

IX_OSAL_MK_HOST_OS := $(OSTYPE)

# If we don't have a valid OS name, try to use the Unix uname command
# to find it.
ifneq ($(IX_OSAL_MK_HOST_OS), linux)
  IX_OSAL_MK_HOST_OS := $(shell uname)
  IX_OSAL_MK_HOST_OS := $(subst Linux,linux,$(IX_OSAL_MK_HOST_OS))
# We do not check for 'cygwin' here, as a windows box will often have
# a cygwin "uname" on its PATH even when not running in a true cygwin
# environment. We must rely on the OSTYPE environment variable to tell
# us if we're in a true cygwin environment.
endif


################################################################
# Linux Compiler & linker commands

LD := $(COMPILE_PREFIX)ld
CC := $(COMPILE_PREFIX)gcc
AR := $(COMPILE_PREFIX)ar

################################################################
# Compiler & linker options

# Compiler flags

# Linux linker flags
LDFLAGS := -r
MAKE_DEP_FLAG := -M

