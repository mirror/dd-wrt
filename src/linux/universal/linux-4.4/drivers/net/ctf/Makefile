#
# Makefile for the Broadcom CTF Layer
#
# Copyright (C) 2013, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: Makefile,v 1.3 2008-08-13 20:20:16 $
#
ifeq ($(CONFIG_MIPS),y)
ifeq ($(CONFIG_CPU_MIPS32_R1),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/common/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/common/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
CTFDIR := ../../../brcm/mipsel_r1/ctf/linux
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
CTFDIR := ../../../brcm/mipsel/ctf/linux
endif
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/common/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/common/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
CTFDIR := ../../../brcm/arm/ctf/linux
endif

ctf-objs := $(CTFDIR)/ctf.o

obj-$(CONFIG_BCM_CTF) := ctf.o

clean-files += $(CTFDIR)/*.o $(CTFDIR)/.*.o.cmd
