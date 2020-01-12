#
# C library Makefile
# 
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.5 2003/10/29 03:43:55 honor Exp $
#
include $(TOP)/.config
ifneq ($(wildcard ../../cy_conf.mak),)
  include ../../cy_conf.mak
endif

# Extra libraries
lib-$(CONFIG_LIBCRYPT) += libcrypt
lib-$(CONFIG_LIBDL) += libdl
lib-$(CONFIG_LIBM) += libm
lib-$(CONFIG_LIBRT) += librt
lib-$(CONFIG_LIBCPP) += libstdc++
lib-$(CONFIG_LIBNSL) += libnsl
#ifeq ($(PARENTAL_CONTROL_SUPPORT),1)
lib-$(CONFIG_LIBPTHREAD) += libpthread
#endif
lib-$(CONFIG_LIBRESOLV) += libresolv
lib-$(CONFIG_LIBUTIL) += libutil
lib-y += libgcc_s
ifeq ($(CONFIG_UCLIBC),y)

# uClibc names
LINKER := ld-uClibc.so.0
LIBC := libc.so.0
MAP := uClibc.map

else

# GNU libc names
LINKER := $(foreach lib,ld.so.* ld-linux.so.*,$(notdir $(wildcard $(LIBDIR)/$(lib))))
LIBC := libc.so.6
MAP := libc.map

endif

# Nothing to do until installation
all:

ifeq ($(CONFIG_LIBOPT),y)

# Find all ELF binaries (including extra libraries)
BINARIES := $(shell find $(TARGETDIR) -path $(TARGETDIR)/lib -prune -o -type f -print | file -f - | grep ELF | cut -d':' -f1)
BINARIES += $(wildcard $(foreach lib,$(LINKER) $(lib-y),$(LIBDIR)/$(lib) $(LIBDIR)/$(lib).so.?))

ifneq ($(wildcard $(LIBDIR)/optinfo),)
# Try the toolchain directory first for precursors
OPTINFO := $(LIBDIR)/optinfo
else
# Fall back on platform directory
OPTINFO := $(PLATFORM)
endif

ifeq ($(CONFIG_UCLIBC),y)

# uClibc precursors
OBJS := $(wildcard $(foreach file,libgcc-need.a libc.a interp.o,$(OPTINFO)/$(file)))

# uClibc link parameters
ENTRY := __uClibc_init
LDFLAGS := -s -shared --warn-common --warn-once -z combreloc  -soname=$(LIBC) -init $(ENTRY)
LIBGCC := $(shell $(CC) -print-libgcc-file-name)

$(MAP): $(OPTINFO)/libc.a
	$(NM) -o --defined-only --no-sort $< | cut -d' ' -f3 > $@

# Find all unresolved symbols that uClibc could resolve
.symbols: $(MAP) $(BINARIES)
	rm -f $@
	for symbol in `$(NM) --dynamic -u --no-sort $(BINARIES) | sort -u` ; do \
	    grep -q "^$${symbol}$$" $(MAP) && echo "-u $${symbol}" >> $@ ; \
	    true ; \
	done
#ifeq ($(CONFIG_PTHREAD),y)
#	echo "__libc_pthread_init >> .symbols
#endif

# Re-link uClibc
$(LIBC): .symbols $(OBJS)
	xargs -t $(LD) $(LDFLAGS) -o $@ $(OBJS) < .symbols

else

# GNU libc precursors
OBJS := $(wildcard $(foreach file,soinit.os libc_pic.a sofini.os,$(OPTINFO)/$(file)))

# GNU libc link parameters
ENTRY := __libc_main
CFLAGS := -shared -Wl,-Os -pipe -mips32 -mtune=mips32 -funit-at-a-time -Wl,-dynamic-linker=$(LINKER) -Wl,--version-script=$(MAP) -Wl,-soname=$(LIBC) -nostdlib -nostartfiles -e $(ENTRY)

$(MAP): $(OPTINFO)/libc.map
	cp $< $@

# Find all unresolved symbols that GNU libc could resolve
.symbols: $(MAP) $(BINARIES)
	rm -f $@
	for symbol in `$(NM) --dynamic -u --no-sort $(BINARIES) | sort -u` ; do \
	    grep -q "$${symbol};" $(MAP) && echo "-u $${symbol}" >> $@ ; \
	done

# Re-link GNU libc
$(LIBC): .symbols $(OBJS) $(LIBDIR)/$(LINKER)
	xargs $(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBDIR)/$(LINKER) -lgcc < .symbols

endif

# Install optimized C library
install: $(LIBC)

else

# Install default C library
install: $(LIBDIR)/$(LIBC)

endif

# Also install extra libraries
install: $(wildcard $(foreach lib,$(LINKER) $(lib-y),$(LIBDIR)/$(lib) $(LIBDIR)/$(lib).so.?))
#	$(NM) -o --defined-only --no-sort $(OPTINFO)/libc.a | cut -d' ' -f3 > uClibc.map
	install -d $(TARGETDIR)/lib
	install $^ $(TARGETDIR)/lib/
#	install libgcc_s.so.1 $(TARGETDIR)/lib/
	#$(STRIP) $(TARGETDIR)/lib/*.so.*

clean:
	rm -f $(LIBC) $(MAP) .symbols
