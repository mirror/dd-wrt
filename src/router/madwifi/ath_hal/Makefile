# $Id: //depot/sw/linuxsrc/src/802_11/madwifi/madwifi/ath_hal/Makefile#9 $
#
# Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

#
# Makefile for the Atheros HAL Module.
#
# NB: we can't support MODVERSIONS because it requires building
#     from source which may not be available
#
ifeq ($(obj),)
obj=	.
endif

DEPTH=	..

include $(obj)/${DEPTH}/Makefile.inc

obj-m		+= ath_hal.o
ath_hal-objs	:= ah_osdep.o ${TARGET}.hal.o
hostprogs-y	:= uudecode

# For older 2.6 kernels
host-progs	:= $(hostprogs-y)

ifdef LINUX24
export-objs	:= ah_osdep.o
endif

INCS+=	-I${HAL} -I${HAL}/${OS} -I${obj}/${ATH_HAL} -I${obj}/${DEPTH}

EXTRA_CFLAGS+= ${INCS} ${COPTS2}

-include $(TOPDIR)/Rules.make

all: opt_ah.h
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) MODVERDIR=$(shell pwd)/${SYMBOLSDIR} modules

ifdef LINUX24
$(obj)/uudecode: $(obj)/uudecode.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<
endif

$(obj)/${TARGET}.hal.o: ${HAL}/public/${TARGET}.hal.o.uu $(obj)/uudecode
	echo "deps_$@ :=" >$(@D)/.$(@F).cmd
	$(obj)/uudecode -o $@ $<

${HAL}/public/${TARGET}.hal: ${HAL}/public/${TARGET}.hal.o.uu
	[ -f $< ] && uudecode -o $@ $< || true

opt_ah.h: ${HAL}/public/${TARGET}.opt_ah.h
	cp -f ${HAL}/public/${TARGET}.opt_ah.h opt_ah.h

install:
	test -d ${DESTDIR}/${KMODPATH} || mkdir -p ${DESTDIR}/${KMODPATH}
	$(STRIP) -S ath_hal.$(KMODSUF)
	cp ath_hal.${KMODSUF} ${DESTDIR}/${KMODPATH}

clean:
	-rm -f *~ *.o *.ko *.mod.c uudecode .uudecode.cmd
	-rm -f .depend .version .*.o.flags .*.o.d .*.o.cmd .*.ko.cmd
	-rm -rf .tmp_versions
# remove the files we copied to this dir
	-rm -f opt_ah.h

ath_hal.o: ${ath_hal-objs}
	$(LD) $(LDOPTS) -o ath_hal.$(KMODSUF) -r $(ath_hal-objs)
