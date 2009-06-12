#
# (C) Copyright 2004 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/releases/linuxsrc/tools/Makefile#3 $
# $File: //depot/sw/releases/linuxsrc/tools/Makefile $
# $Author: zhifeng $
# $DateTime: 2005/08/09 13:42:05 $
# $Change: 146233 $
#
ifdef ATH_ROOT
include ${ATH_ROOT}/make/includes.mk
endif

SUBDIRS    = ejtag
TOOLS      = 

ifeq (${TARGET_ARCH}, mips)
TOOLS = gcc-3.3.3-2.4.25
endif

export TOPDIR     =${ATH_ROOT}
export TOOLPATH   =${ATH_ROOT}/tools/gcc-3.3.3-2.4.25/toolchain_mips
export TOOLPREFIX =mips-linux-
export KERNELPATH =${ATH_ROOT}/src/kernels/mips-linux-2.4.25
export TFTPPATH   =/tftpboot

prep::

build::
	@for t in ${TOOLS}; do \
	   (cd $$t; make) \
	done;

gdb-5.3:
	tar xvfz gdb-5.3.tar.gz
	cd gdb-5.3; patch -p1 < ../gdb-5.3-soc.patch
	mkdir -p gdb-5.3/build/mips
	cd gdb-5.3/build/mips; ../../configure --target=mips-unknown-elf; make

clean::
	@${RM} gcc-3.3.3-2.4.25/toolchain_build_mips

install::


