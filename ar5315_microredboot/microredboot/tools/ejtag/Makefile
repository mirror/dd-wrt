#
# (C) Copyright 2004 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/releases/linuxsrc/tools/ejtag/Makefile#3 $
# $File: //depot/sw/releases/linuxsrc/tools/ejtag/Makefile $
# $Author: zhifeng $
# $DateTime: 2005/08/09 13:42:05 $
# $Change: 146233 $
#
include ${ATH_ROOT}/make/includes.mk

DIRS    = ejtag_client gdbstub

prep::

build::
	for dir in ${DIRS} ; do \
		(cd $$dir; make) \
	done;

install::

clean::
	for dir in ${DIRS} ; do \
		(cd $$dir; make clean) \
	done;
