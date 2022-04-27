#
# Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2005 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/hal/main/linux/Makefile#3 $
#
DEPTH=	..

#
# Makefile for building the Atheros/MADWIFI HAL from source code with gmake
#
HAL=	${DEPTH}
PUBLIC=	${HAL}/public

ALL = \
	mipsisa32-le-elf \
	mipsisa32-be-elf \
	mipsisa32_soc-be-elf \
	i386-boese-elf \
	i386-elf \
	x86_64-elf \
	powerpc-be-elf \
	powerpc-boese-be-elf \
	xscale-boese-be-elf \
	ap51-boese \
	ap30- \
	ap30-boese-be-elf \
	ap43- \
	ap43-boese-be-elf \
	ap30-boese-be-elf \
	ap43-boese-be-elf \
	ap61-boese-be-elf \
	ap61-lite-boese-be-elf \
	armv9tdmi-boese-be-elf \
	armv9tdmi-boese-le-elf \
	ls5 \
	bs2 \
	ls2 \
	eoc2610 \
	eoc5610 \
	ar7100-be-elf \
	ar7100-rs-be-elf \
	adm5120-le-elf

all: release

release:
	for i in ${ALL}; do \
		${MAKE} -f Makefile.inc TARGET=$$i release; \
	done

clean:
	-rm -rf ../public/*.hal.o.uu ../public/*.opt_ah.h obj

