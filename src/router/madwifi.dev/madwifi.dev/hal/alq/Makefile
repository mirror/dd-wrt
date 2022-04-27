#
# Copyright (c) 2004 Sam Leffler, Errno Consulting
# Copyright (c) 2004 Video54 Technologies, Inc.
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
# $Id: //depot/sw/branches/sam_hal/alq/Makefile#1 $
#

#
# Makefile for Jeff Roberson's ALQ event collection module.
#
ifeq ($(obj),)
obj=	.
endif

DEPTH=	../../head

obj-m		+= alq.o
alq-objs	:= kern_alq.o
export-objs	:= kern_alq.o
list-multi	:= alq.o

include $(obj)/${DEPTH}/Makefile.inc

INCS+=	-I${obj}/${DEPTH}

EXTRA_CFLAGS+= ${INCS} ${COPTS}

-include $(TOPDIR)/Rules.make

all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

install: all
	test -d ${DESTDIR}/${MODULEPATH} || mkdir -p ${DESTDIR}/${MODULEPATH}
	${TOOLPREFIX}strip -S alq.$(KMODSUF)
	cp alq.${KMODSUF} ${DESTDIR}/${MODULEPATH}

clean:
	-rm -f *~ *.o *.ko *.mod.c
	-rm -f .version .*.o.flags .*.o.d .*.o.cmd .*.ko.cmd

alq.o: ${alq-objs}
	$(LD) $(LDOPTS) -o alq.$(KMODSUF) -r $(alq-objs)
