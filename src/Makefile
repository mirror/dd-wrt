#
# Toplevel Makefile for the BCM947xx Linux Router release
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.35 2004/02/16 03:55:59 honor Exp $
#

SRCBASE := $(shell pwd)
RELEASEDIR := $(shell (cd $(SRCBASE)/.. && pwd -P))
PATH := $(RELEASEDIR)/tools:$(PATH)

install: cytools all
	install -d $(RELEASEDIR)/image
	$(MAKE) -C router install
	#-cp router/mipsel/linux.trx $(RELEASEDIR)/image/linux-glibc.trx
#	-cp router/mipsel-uclibc/linux.trx $(RELEASEDIR)/image/linux.trx

	# add CyberTAN header
#	addpattern -i $(RELEASEDIR)/image/linux.trx -o $(RELEASEDIR)/image/code.bin -g
#	addpattern_gs -i $(RELEASEDIR)/image/linux.trx -o $(RELEASEDIR)/image/code_gs.bin -g
#	-cp $(RELEASEDIR)/image/linux.trx $(RELEASEDIR)/image/code.bin

all clean distclean:
	$(MAKE) -C router $@

cytools:




.PHONY: all clean distclean install
