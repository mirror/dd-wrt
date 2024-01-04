# SPDX-License-Identifier: GPL-2.0
#
# Makefile for MIPS crypto files..
#



obj-$(CONFIG_CRYPTO_POLY1305_MIPS) += poly1305-mips.o
poly1305-mips-y := poly1305-core.o poly1305-glue.o

CFLAGS_poly1305-glue.o += $(DISABLE_LTO)

perlasm-flavour-$(CONFIG_32BIT) := o32
perlasm-flavour-$(CONFIG_64BIT) := 64

quiet_cmd_perlasm = PERLASM $@
      cmd_perlasm = $(PERL) $(<) $(perlasm-flavour-y) $(@)

$(obj)/poly1305-core.S: $(src)/poly1305-mips.pl FORCE
	$(call if_changed,perlasm)

targets += poly1305-core.S
