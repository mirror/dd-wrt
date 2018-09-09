#/***********************license start***************
# Copyright (c) 2003-2007 Cavium Inc. (support@cavium.com). All rights
# reserved.
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
#     * Neither the name of Cavium Inc. nor the names of
#       its contributors may be used to endorse or promote products
#       derived from this software without specific prior written
#       permission.
#
# TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND CAVIUM NETWORKS MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
# REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
# DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
# PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
# POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
# OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
#
#
# For any questions regarding licensing please contact marketing@cavium.com
#
# ***********************license end**************************************/

#
#  component Makefile fragment
#

#  standard component Makefile header
sp              :=  $(sp).x
dirstack_$(sp)  :=  $(d)
d               :=  $(dir)

#EXEC_OBJ_DIR = $(OCTEON_ROOT)/executive/obj$(PREFIX)-$(OCTEON_ISA)

#  special rule to re-compile if important environment variables change
MATCH=${findstring DUSE_RUNTIME_MODEL_CHECKS=1, ${OCTEON_CPPFLAGS_GLOBAL_ADD}}
ifeq (${MATCH}, DUSE_RUNTIME_MODEL_CHECKS=1)
#  We are using runtime model detection, so use "runtime" as model to avoid
#    a re-compile if only OCTEON_MODEL used for simulation changes
MADE_WITH_OCTEON_MODEL = "runtime"-$(CVMX_USE_1_TO_1_TLB_MAPPINGS)
else
MADE_WITH_OCTEON_MODEL = $(OCTEON_MODEL)-$(CVMX_USE_1_TO_1_TLB_MAPPINGS)
endif

#  set special filename for target and change any spaces in it to commas
LIB_MADE_WITH = $(shell echo "$(EXEC_OBJ_DIR)/lib-made_with-OCTEON_MODEL=$(MADE_WITH_OCTEON_MODEL)=-ISA=${OCTEON_ISA}=-OCTEON_CPPFLAGS_GLOBAL_ADD=$(OCTEON_CPPFLAGS_GLOBAL_ADD)=." | sed 's/\ /,/g')

LIB_MADE_WITH_ALL = $(EXEC_OBJ_DIR)/lib-made_with-*

$(LIB_MADE_WITH):
	mkdir -p $(EXEC_OBJ_DIR)
	rm -f $(LIB_MADE_WITH_ALL)
	touch "$(LIB_MADE_WITH)"

#  component specification
LIBRARY_CVMX := $(EXEC_OBJ_DIR)/libcvmx.a

EXEC_OBJS_$(d)  :=  \
	$(EXEC_OBJ_DIR)/cvmx-adma.o \
	$(EXEC_OBJ_DIR)/cvmx-agl.o \
	$(EXEC_OBJ_DIR)/cvmx-app-config.o \
	$(EXEC_OBJ_DIR)/cvmx-app-hotplug.o \
	$(EXEC_OBJ_DIR)/cvmx-appcfg-transport.o \
	$(EXEC_OBJ_DIR)/cvmx-bch.o \
	$(EXEC_OBJ_DIR)/cvmx-bootmem.o \
	$(EXEC_OBJ_DIR)/cvmx-clock.o \
	$(EXEC_OBJ_DIR)/cvmx-cmd-queue.o \
	$(EXEC_OBJ_DIR)/cvmx-cn3010-evb-hs5.o \
	$(EXEC_OBJ_DIR)/cvmx-config-parse.o \
	$(EXEC_OBJ_DIR)/cvmx-core.o \
	$(EXEC_OBJ_DIR)/cvmx-coremask.o \
	$(EXEC_OBJ_DIR)/cvmx-csr-db.o \
	$(EXEC_OBJ_DIR)/cvmx-csr-db-support.o \
	$(EXEC_OBJ_DIR)/cvmx-crypto.o \
	$(EXEC_OBJ_DIR)/cvmx-dfa.o \
	$(EXEC_OBJ_DIR)/cvmx-dma-engine.o \
	$(EXEC_OBJ_DIR)/cvmx-ebt3000.o \
	$(EXEC_OBJ_DIR)/cvmx-error.o \
	$(EXEC_OBJ_DIR)/cvmx-error-custom.o \
	$(EXEC_OBJ_DIR)/cvmx-error-gmx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn30xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn31xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn38xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn38xxp2.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn50xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn52xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn52xxp1.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn56xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn56xxp1.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn58xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn58xxp1.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn61xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn63xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn63xxp1.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn66xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn68xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn68xxp1.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cnf71xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn70xx.o \
	$(EXEC_OBJ_DIR)/cvmx-error-init-cn78xx.o \
	$(EXEC_OBJ_DIR)/cvmx-flash.o \
	$(EXEC_OBJ_DIR)/cvmx-fpa.o \
	$(EXEC_OBJ_DIR)/cvmx-fpa-resource.o \
	$(EXEC_OBJ_DIR)/cvmx-fau.o \
	$(EXEC_OBJ_DIR)/cvmx-global-resources.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-agl.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-board.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-errata.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-cfg.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-fpa.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-ilk.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-loop.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-npi.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-pki.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-pko3.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-rgmii.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-sgmii.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-spi.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-srio.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-util.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-xaui.o \
	$(EXEC_OBJ_DIR)/cvmx-helper-jtag.o \
	$(EXEC_OBJ_DIR)/cvmx-helper.o \
	$(EXEC_OBJ_DIR)/cvmx-hfa.o \
	$(EXEC_OBJ_DIR)/cvmx-ilk.o \
	$(EXEC_OBJ_DIR)/cvmx-ipd.o \
	$(EXEC_OBJ_DIR)/cvmx-ixf18201.o \
	$(EXEC_OBJ_DIR)/cvmx-l2c.o \
	$(EXEC_OBJ_DIR)/cvmx-llm.o \
	$(EXEC_OBJ_DIR)/cvmx-log-arc.o \
	$(EXEC_OBJ_DIR)/cvmx-log.o \
	$(EXEC_OBJ_DIR)/cvmx-mgmt-port.o \
	$(EXEC_OBJ_DIR)/cvmx-nand.o \
	$(EXEC_OBJ_DIR)/cvmx-ocla.o \
	$(EXEC_OBJ_DIR)/cvmx-pcie.o \
	$(EXEC_OBJ_DIR)/cvmx-pki.o \
	$(EXEC_OBJ_DIR)/cvmx-pki-parse.o \
	$(EXEC_OBJ_DIR)/cvmx-pki-resources.o \
	$(EXEC_OBJ_DIR)/cvmx-pko.o \
	$(EXEC_OBJ_DIR)/cvmx-pko-internal-ports-range.o \
	$(EXEC_OBJ_DIR)/cvmx-pko3.o \
	$(EXEC_OBJ_DIR)/cvmx-pko3-resources.o \
	$(EXEC_OBJ_DIR)/cvmx-pko3-queue.o \
	$(EXEC_OBJ_DIR)/cvmx-pow.o \
	$(EXEC_OBJ_DIR)/cvmx-power-throttle.o \
	$(EXEC_OBJ_DIR)/cvmx-profiler.o \
	$(EXEC_OBJ_DIR)/cvmx-qlm.o \
	$(EXEC_OBJ_DIR)/cvmx-qlm-tables.o \
	$(EXEC_OBJ_DIR)/cvmx-raid.o \
	$(EXEC_OBJ_DIR)/cvmx-range.o \
	$(EXEC_OBJ_DIR)/cvmx-shmem.o \
	$(EXEC_OBJ_DIR)/cvmx-spi.o \
	$(EXEC_OBJ_DIR)/cvmx-spi4000.o \
	$(EXEC_OBJ_DIR)/cvmx-srio.o \
	$(EXEC_OBJ_DIR)/cvmx-sysinfo.o \
	$(EXEC_OBJ_DIR)/cvmx-thunder.o \
	$(EXEC_OBJ_DIR)/cvmx-tim.o \
	$(EXEC_OBJ_DIR)/cvmx-tra.o \
	$(EXEC_OBJ_DIR)/cvmx-twsi.o \
	$(EXEC_OBJ_DIR)/cvmx-usb.o \
	$(EXEC_OBJ_DIR)/cvmx-usbd.o \
	$(EXEC_OBJ_DIR)/cvmx-warn.o \
	$(EXEC_OBJ_DIR)/cvmx-zip.o \
	$(EXEC_OBJ_DIR)/cvmx-zone.o \
	$(EXEC_OBJ_DIR)/cvmx-gser.o \
	$(EXEC_OBJ_DIR)/cvmx-bgx.o \
	$(EXEC_OBJ_DIR)/cvmx-mbox.o \
	$(EXEC_OBJ_DIR)/octeon-feature.o \
	$(EXEC_OBJ_DIR)/octeon-model.o \
	$(EXEC_OBJ_DIR)/octeon-pci-console.o

ifeq (linux,$(findstring linux,$(OCTEON_TARGET)))
EXEC_OBJS_$(d)  +=  \
	$(EXEC_OBJ_DIR)/cvmx-app-init-linux.o
else
EXEC_OBJS_$(d)  +=  \
	$(EXEC_OBJ_DIR)/cvmx-debug.o \
	$(EXEC_OBJ_DIR)/cvmx-debug-handler.o \
	$(EXEC_OBJ_DIR)/cvmx-debug-remote.o \
	$(EXEC_OBJ_DIR)/cvmx-debug-uart.o \
	$(EXEC_OBJ_DIR)/cvmx-interrupt.o \
	$(EXEC_OBJ_DIR)/cvmx-interrupt-handler.o \
	$(EXEC_OBJ_DIR)/cvmx-app-init.o \
	$(EXEC_OBJ_DIR)/cvmx-malloc.o \
	$(EXEC_OBJ_DIR)/cvmx-otrace.o \
	$(EXEC_OBJ_DIR)/cvmx-tlb.o \
	$(EXEC_OBJ_DIR)/cvmx-uart.o \
	$(EXEC_OBJ_DIR)/cvmx-coredump.o
endif

#Always enable lto for the library.
$(EXEC_OBJS_$(d)):  CFLAGS_LOCAL := -I$(d) -O2 -g -W -Wall -Wno-unused-parameter -Wundef

#  standard component Makefile rules

EXEC_DEPS_$(d)   :=  $(EXEC_OBJS_$(d):.o=.d)

LIBS_LIST   :=  $(OBJ_DIR)/cvmx-config-init.o $(LIBS_LIST) $(LIBRARY_CVMX)

LIB_CLEAN_LIST  :=  $(LIB_CLEAN_LIST) $(EXEC_OBJS_$(d)) $(EXEC_DEPS_$(d)) $(LIBRARY_CVMX) $(LIB_MADE_WITH) $(EXEC_OBJ_DIR_STAMP)
CLEAN_LIST := $(OBJ_DIR)/cvmx-config-init.o $(OBJ_DIR)/cvmx-config-init.d

-include $(EXEC_DEPS_$(d))

$(EXEC_OBJ_DIR_STAMP): $(LIB_MADE_WITH)
	mkdir -p $(EXEC_OBJ_DIR)
	touch $(EXEC_OBJ_DIR_STAMP)

$(LIBRARY_CVMX): $(EXEC_OBJS_$(d)) $(EXEC_OBJ_DIR_STAMP)
	$(AR) -cr $@ $^

cvmx-lib: $(EXEC_OBJ_DIR) $(LIBRARY_CVMX) $(LIB_MADE_WITH)

cvmx-lib-clean:
	rm -f $(LIB_CLEAN_LIST) EXEC_DEPS_$(d) $(EXEC_OBJS_$(d))

clean-all: cvmx-lib-clean clean
	rm -f $(CLEAN_LIST)

$(EXEC_OBJ_DIR)/%.o:	$(d)/%.c $(EXEC_OBJ_DIR_STAMP)
	$(COMPILE)

$(EXEC_OBJ_DIR)/%.o:	$(d)/%.S $(EXEC_OBJ_DIR_STAMP)
	$(ASSEMBLE)

$(EXEC_OBJ_DIR)/cvmx-app-init-linux.o: $(d)/cvmx-app-init-linux.c $(EXEC_OBJ_DIR_STAMP)
	$(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MMD -c -Umain -o $@ $<

CFLAGS_SPECIAL := -I$(d) -I$(d)/cvmx-malloc -O2 -g -DUSE_CVM_THREADS=1 -D_REENTRANT

$(EXEC_OBJ_DIR)/cvmx-malloc.o: $(d)/cvmx-malloc/malloc.c $(EXEC_OBJ_DIR_STAMP)
	$(CC) $(CFLAGS_GLOBAL) $(CFLAGS_SPECIAL) -MMD -c -o $@ $<

#  standard component Makefile footer

d   :=  $(dirstack_$(sp))
sp  :=  $(basename $(sp))
