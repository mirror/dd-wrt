# Copyright (c) 2014, 2018-2019, 2021 The Linux Foundation. All rights reserved.
# Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

###################################################
# Makefile for the NSS MACSEC driver
###################################################

obj ?= .

obj-m +=qca-nss-macsec.o

qca-nss-macsec-objs +=	nss_macsec_init.o \
			fal/nss_macsec_fal_api.o \
			fal/nss_macsec_interrupt.o \
			fal/nss_macsec.o \
			fal/nss_macsec_mib.o \
			fal/nss_macsec_secy.o \
			fal/nss_macsec_secy_rx.o \
			fal/nss_macsec_secy_tx.o \
			fal/nss_macsec_utility.o \
			dal/dal_utility.o \
			dal/qca808x/qca808x_macsec.o

NSS_MACSEC_INCLUDE = -I$(obj)/include -I$(obj)/dal -I$(obj)/fal -I$(obj)/dal/qca808x

ccflags-y += $(NSS_MACSEC_INCLUDE)
ccflags-y += -Wall -Werror
