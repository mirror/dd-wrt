#
# Macro definitions for platform-specific makefile
#
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
#

# Linux EP805XX Compiler and Linker Commands
IX_LINUX_CROSSCOMPILER ?= /usr/bin/

LINUX_CROSS_COMPILE := $(IX_LINUX_CROSSCOMPILER) 
LINUX_UTILS := /bin/
COPY := $(LINUX_UTILS)cp -f

#COMPILE_PREFIX := $(strip $(LINUX_CROSS_COMPILE))
COMPILE_PREFIX := 

# Compiler flags

CFLAGS += -D"KBUILD_STR(s)=\#s" 
CFLAGS += -D__EP805XX__
CFLAGS += -D__linux 

ifeq ($(IX_LINUXVER), 2.6.27)
CFLAGS += -D__KERNEL__ -DEXPORT_SYMTAB -DMODULE -I$(LINUX_SRC)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Wstrict-prototypes -Wundef -Werror-implicit-function-declaration -Os -pipe -msoft-float -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2  -march=i686 -mtune=i686 -mregparm=3 -ffreestanding -Iinclude/asm-i386/mach-generic -Iinclude/asm-i386/mach-default -fomit-frame-pointer -fasynchronous-unwind-tables -g  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign    
else
ifeq ($(IX_LINUXVER), 2.6.28)
CFLAGS += -D__KERNEL__ -DEXPORT_SYMTAB -DMODULE -I$(LINUX_SRC)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Wstrict-prototypes -Wundef -Werror-implicit-function-declaration -Os -pipe -msoft-float -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2  -march=i686 -mtune=i686 -mregparm=3 -ffreestanding -Iinclude/asm-i386/mach-generic -Iinclude/asm-i386/mach-default -fomit-frame-pointer -fasynchronous-unwind-tables -g  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign    
else
ifeq ($(IX_LINUXVER), 2.6.18)
CFLAGS += -D__KERNEL__ -DEXPORT_SYMTAB -DMODULE -I$(LINUX_SRC)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Wstrict-prototypes -Wundef -Werror-implicit-function-declaration -Os -pipe -msoft-float -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2  -march=i686 -mtune=i686 -mregparm=3 -ffreestanding -Iinclude/asm-i386/mach-generic -Iinclude/asm-i386/mach-default -fomit-frame-pointer -fasynchronous-unwind-tables -g  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign    
else
CFLAGS += -nostdinc -iwithprefix include -D__KERNEL__ -DEXPORT_SYMTAB -DMODULE -I$(LINUX_SRC)/include  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Os -fomit-frame-pointer -g -Wdeclaration-after-statement -pipe -msoft-float -m32 -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2 -fno-unit-at-a-time -march=i686 -mregparm=3 -Iinclude/asm-i386/mach-default
endif #2.6.18
endif #2.6.28
endif #2.6.27

CFLAGS += -D"KBUILD_BASENAME=KBUILD_STR(osal_lib)"
CFLAGS += -D"KBUILD_MODNAME=KBUILD_STR(osal_module)"

CFLAGS += $(OSAL_CFLAGS)  

INCLUDE_DIRS +=  $(LINUX_SRC)/include/asm/ $(LINUX_SRC)/include/asm/mach-default/
