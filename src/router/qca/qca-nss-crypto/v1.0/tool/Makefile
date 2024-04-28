# Copyright (c) 2013, 2016, 2019 The Linux Foundation. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
TOOL_MOD_NAME=qca-nss-crypto-tool

obj-m	+= $(TOOL_MOD_NAME).o
$(TOOL_MOD_NAME)-objs = nss_crypto_bench.o

obj ?= .
#ccflags-y += -DCONFIG_NSS_CRYPTO_TOOL_DBG
ccflags-y += -DNSS_CRYPTO_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S')'\"
ccflags-y += -I$(obj)/../include -I$(obj)/../src -I$(obj)/
ccflags-y += -Werror