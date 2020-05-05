ifeq ($(CONFIG_MIPS),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/shared -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -I$(srctree)/brcm/mipsel/wl/dump/include -I$(srctree)/brcm/mipsel/wl/iocv/include -DBCMDRIVER -Dlinux -DCONFIG_NOPRINTK=1
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/shared -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -I$(srctree)/brcm/mipsel/wl/dump/include -I$(srctree)/brcm/mipsel/wl/iocv/include -DBCMDRIVER -Dlinux
ifeq ($(CONFIG_CPU_MIPS32_R1),y)
SRCBASE := $(srctree)/drivers/net/wl/../../../brcm/mipsel_r1
SRCBASE_OFFSET := ../../../brcm/mipsel_r1
else
SRCBASE := $(srctree)/drivers/net/wl/../../../brcm/mipsel
SRCBASE_OFFSET := ../../../brcm/mipsel
endif
else
DDFLAGS =-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared -I$(srctree)/brcm/arm/common/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -I$(srctree)/brcm/arm/wl/dump/include -I$(srctree)/brcm/arm/wl/iocv/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/hal 
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/include 
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/core 
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/temp
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/ana
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/antdiv
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/btcx
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/cache
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/calmgr
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/noise
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/chanmgr
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/init
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/fcbs
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/utils
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/tbl
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/rssi
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/radar
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/radio
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/rxiqcal
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/txiqlocal
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/papdcal
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/vcocal
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/lpc
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/tssical
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/rxspur
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/rxgcrs
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/samp
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/mu
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/misc
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/cmn/tpc
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/ac/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/ac/core
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/lcn/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/lcn/core
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/lcn40/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/lcn40/core
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/lcn20/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/lcn20/core
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/n/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/n/core
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/ht/include
DDFLAGS+=-I$(srctree)/brcm/arm/wl/phymods/ht/core

DDFLAGS+=-I$(srctree)/brcm/arm/wl/keymgmt/include
EXTRA_CFLAGS+=$(DDFLAGS) -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=$(DDFLAGS) -DBCMDRIVER -Dlinux
SRCBASE := $(srctree)/drivers/net/wl/../../../brcm/arm
SRCBASE_OFFSET := ../../../brcm/arm
endif



#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.
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
# $Id: Makefile,v 1.13 2010-12-13 20:23:49 $
#


TARGET := wl
WLCONF_H := wlconf.h
WLCONF_O := wlconf.o

# Generate list of supported variants
VLIST_BASE := APSTA
ifeq ($(CONFIG_WL_USE_AP_SDSTD),y)
  VLIST_BASE   += AP_SDSTD
endif

VLIST          += $(VLIST_BASE)

ifeq ($(CONFIG_WL_USBAP),y)
# Do variants because we need two drivers for dual band operation
WL_VARIANTS	:= 1
VLIST		+= HIGH
# Note that we don't use CONFIG_WL_USE_HIGH because we need to leave WL_SUFFIX for
# the other driver. HIGH driver is always called wl_high and loaded upon hotplut
# event.
endif

ifeq ($(BUILD_WL_ONCHIP_G),true)
  VLIST        += $(foreach base,$(VLIST_BASE),$(base)_ONCHIP_G)
endif # BUILD_WL_ONCHIP_G

# Set suffix and config based on the selected variant
WL_SUFFIX := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))
CONFIG_WL_CONF := $(CONFIG_WL_$(WL_SUFFIX))
CONFIG_WL_CONF := wlconfig_lx_router_apsta

# Module file names use lowercase of the suffix
wl_suffix := $(shell echo "$(WL_SUFFIX)" | tr A-Z a-z)


# For PREBUILD and postbuild w/WLSRC, compile variants in subdirs
# Subdir names use lowercase versions of the variant suffixes
#wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)/" | tr A-Z a-z)))
wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)" | tr A-Z a-z)))


#subdir-y += $(wlsubdirs)
#subdir-m += $(wlsubdirs)

.NOTPARALLEL:
include $(src)/wl_generic.mk

#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.
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
# $Id: Makefile,v 1.13 2010-12-13 20:23:49 $
#

TARGET := wl
WLCONF_H := wlconf.h
WLCONF_O := wlconf.o

# Generate list of supported variants
VLIST_BASE := APSTA
ifeq ($(CONFIG_WL_USE_AP_SDSTD),y)
  VLIST_BASE   += AP_SDSTD
endif

VLIST          += $(VLIST_BASE)

ifeq ($(CONFIG_WL_USBAP),y)
# Do variants because we need two drivers for dual band operation
WL_VARIANTS	:= 1
VLIST		+= HIGH
# Note that we don't use CONFIG_WL_USE_HIGH because we need to leave WL_SUFFIX for
# the other driver. HIGH driver is always called wl_high and loaded upon hotplut
# event.
endif

ifeq ($(BUILD_WL_ONCHIP_G),true)
  VLIST        += $(foreach base,$(VLIST_BASE),$(base)_ONCHIP_G)
endif # BUILD_WL_ONCHIP_G

# Set suffix and config based on the selected variant
WL_SUFFIX := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))
CONFIG_WL_CONF := $(CONFIG_WL_$(WL_SUFFIX))
CONFIG_WL_CONF := wlconfig_lx_router_apsta

# Module file names use lowercase of the suffix
wl_suffix := $(shell echo "$(WL_SUFFIX)" | tr A-Z a-z)


# For PREBUILD and postbuild w/WLSRC, compile variants in subdirs
# Subdir names use lowercase versions of the variant suffixes
#wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)/" | tr A-Z a-z)))
wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)" | tr A-Z a-z)))


#subdir-y += $(wlsubdirs)
#subdir-m += $(wlsubdirs)

.NOTPARALLEL:
include $(src)/wl_generic.mk
EXTRA_CFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux

#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.
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
# $Id: Makefile,v 1.13 2010-12-13 20:23:49 $
#


TARGET := wl
WLCONF_H := wlconf.h
WLCONF_O := wlconf.o

# Generate list of supported variants
VLIST_BASE := APSTA
ifeq ($(CONFIG_WL_USE_AP_SDSTD),y)
  VLIST_BASE   += AP_SDSTD
endif

VLIST          += $(VLIST_BASE)

ifeq ($(CONFIG_WL_USBAP),y)
# Do variants because we need two drivers for dual band operation
WL_VARIANTS	:= 1
VLIST		+= HIGH
# Note that we don't use CONFIG_WL_USE_HIGH because we need to leave WL_SUFFIX for
# the other driver. HIGH driver is always called wl_high and loaded upon hotplut
# event.
endif

ifeq ($(BUILD_WL_ONCHIP_G),true)
  VLIST        += $(foreach base,$(VLIST_BASE),$(base)_ONCHIP_G)
endif # BUILD_WL_ONCHIP_G

# Set suffix and config based on the selected variant
WL_SUFFIX := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))
CONFIG_WL_CONF := $(CONFIG_WL_$(WL_SUFFIX))
CONFIG_WL_CONF := wlconfig_lx_router_apsta

# Module file names use lowercase of the suffix
wl_suffix := $(shell echo "$(WL_SUFFIX)" | tr A-Z a-z)


# For PREBUILD and postbuild w/WLSRC, compile variants in subdirs
# Subdir names use lowercase versions of the variant suffixes
#wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)/" | tr A-Z a-z)))
wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)" | tr A-Z a-z)))


#subdir-y += $(wlsubdirs)
#subdir-m += $(wlsubdirs)

.NOTPARALLEL:
include $(src)/wl_generic.mk
