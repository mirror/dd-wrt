#
# Makefile for the Linux CAN-bus driver.
# Written by Arnaud Westenberg email:arnaud@wanadoo.nl
# This software is released under the GPL-License.
# Version 0.7  6 Aug 2001
#
# Changes made by Pavel Pisa pisa@cmp.felk.cvut.cz as preliminary
# study for OCERA Real Time CAN/CANOpen project ORTCAN
#
########## The following options can be changed ##########

SUBDIRS = src utils

ifndef SOURCES_DIR
SOURCES_DIR := $(shell ( pwd -L ) )
endif

ifneq ($(wildcard $(SOURCES_DIR)/Standalone.mk),)
# Standalone make outside of OCERA framework

all: default

.PHONY: dep default subdirs clean cleandepend
.PHONY: check-dir include-pass library-pass binary-pass utils-pass kernel-lib-pass kernel-pass

dirs = src utils

default dep clean install utils-pass kernel-pass:
	$(foreach dir, $(dirs), $(MAKE) -C $(SOURCES_DIR)/$(dir) \
	    SOURCES_DIR=$(SOURCES_DIR)/$(dir) $@ ; )

else
# Build in structure of OCERA framework

TOPDIR = $(LINUX_DIR)

ifneq ($(wildcard ../../../../ocera.mk),)
include ../../../../ocera.mk
else
all: 
	@echo -e "\nTo build LinCAN driver for OCERA framework, ocera.mk file have"
	@echo -e "to be generated first. Go to ocera/ directory and do 'make', please."
	@echo -e "If standalone build is required, mark lincan/ by 'touch ./Standalone.mk'.\n"
endif
endif

