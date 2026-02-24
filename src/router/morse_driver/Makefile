# Comment/uncomment the following line to disable/enable debugging
DEBUG ?= y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DCONFIG_MORSE_DEBUG -DDEBUG # "-O" is needed to expand inlines
else
	DEBFLAGS = -O2
endif

# Set 0 to a version number. This is done to match the Linux expectations
override MORSE_VERSION = "0-rel_1_16_4_2025_Sep_18"

USING_CLANG := $(shell $(CC) -v 2>&1 | grep -c "clang version")

ccflags-$(CONFIG_MORSE_USER_ACCESS) += "-DCONFIG_MORSE_USER_ACCESS"
ccflags-y += $(DEBFLAGS) -Wall -Werror
ccflags-y += -Wno-address-of-packed-member
ifeq ($(USING_CLANG), 0)
ccflags-y += -Wno-stringop-truncation -Wmaybe-uninitialized -Wno-missing-attributes
endif
ccflags-y += "-DMORSE_VERSION=$(MORSE_VERSION)"
ccflags-y += -I$(KERNEL_SRC)
ccflags-$(CONFIG_MORSE_SDIO) += "-DCONFIG_MORSE_SDIO"
ccflags-$(CONFIG_MORSE_SPI) += "-DCONFIG_MORSE_SPI"
ccflags-$(CONFIG_MORSE_USB) += "-DCONFIG_MORSE_USB"
ccflags-$(CONFIG_MORSE_VENDOR_COMMAND) += "-DCONFIG_MORSE_VENDOR_COMMAND"
ccflags-$(CONFIG_MORSE_DEBUGFS) += "-DCONFIG_MORSE_DEBUGFS"
ccflags-$(CONFIG_MORSE_ENABLE_TEST_MODES) += "-DCONFIG_MORSE_ENABLE_TEST_MODES"
ccflags-$(CONFIG_MORSE_HW_TRACE) += "-DCONFIG_MORSE_HW_TRACE"
ccflags-$(CONFIG_MORSE_DEBUG_IRQ) += "-DCONFIG_MORSE_DEBUG_IRQ"
ccflags-$(CONFIG_MORSE_DEBUG_TXSTATUS) += "-DCONFIG_MORSE_DEBUG_TXSTATUS"
ccflags-$(CONFIG_MORSE_IPMON) += "-DCONFIG_MORSE_IPMON"
ccflags-$(CONFIG_MORSE_MONITOR) += "-DCONFIG_MORSE_MONITOR"
ccflags-$(CONFIG_MORSE_PAGESET_TRACE) += "-DCONFIG_MORSE_PAGESET_TRACE"
ccflags-$(CONFIG_MORSE_BUS_TRACE) += "-DCONFIG_MORSE_BUS_TRACE"
ccflags-$(CONFIG_ANDROID) += "-DCONFIG_ANDROID"

ifneq ($(CONFIG_BACKPORT_VERSION),)
# Convert a version string from "vX.Y.Z-stuff" into an integer for comparison with KERNEL_VERSION
ccflags-y += "-DMAC80211_BACKPORT_VERSION_CODE=$(shell echo "$(CONFIG_BACKPORT_VERSION)" | \
		awk -F[v.-] '// {printf("%u", lshift($$2, 16) + lshift($$3, 8) + $$4)}')"
endif
ifneq ($(LINUX_BACKPORT_VERSION_CODE),)
#error 1
endif

CONFIG_MORSE_COUNTRY ?= "AU"
ccflags-y += "-DCONFIG_MORSE_COUNTRY=\"$(CONFIG_MORSE_COUNTRY)\""

# Default debug_mask to MORSE_MSG_ERR
CONFIG_MORSE_DEBUG_MASK ?= 8
ccflags-y += "-DCONFIG_MORSE_DEBUG_MASK=$(CONFIG_MORSE_DEBUG_MASK)"

CONFIG_MORSE_SDIO_RESET_TIME ?= 400
ccflags-y += "-DCONFIG_MORSE_SDIO_RESET_TIME=$(CONFIG_MORSE_SDIO_RESET_TIME)"

# Default enable_ps to POWERSAVE_MODE_FULLY_ENABLED
CONFIG_MORSE_POWERSAVE_MODE ?= 2
ccflags-y += "-DCONFIG_MORSE_POWERSAVE_MODE=$(CONFIG_MORSE_POWERSAVE_MODE)"

CONFIG_MORSE_SDIO_ALIGNMENT ?= 2
ccflags-y += "-DCONFIG_MORSE_SDIO_ALIGNMENT=$(CONFIG_MORSE_SDIO_ALIGNMENT)"

ifneq ($(CONFIG_DISABLE_MORSE_RC),y)
	ccflags-y += "-DCONFIG_MORSE_RC"
ifeq ($(CONFIG_ANDROID),y)
	ccflags-y += "-I$(srctree)/$(src)/mmrc"
else
	ccflags-y += "-I$(src)/mmrc"
endif
endif

ifeq ($(CONFIG_MORSE_DHCP_OFFLOAD),y)
	ccflags-y += "-DENABLE_DHCP_OFFLOAD_DEFAULT=1"
else
	ccflags-y += "-DENABLE_DHCP_OFFLOAD_DEFAULT=0"
endif

ifeq ($(CONFIG_MORSE_ARP_OFFLOAD),y)
	ccflags-y += "-DENABLE_ARP_OFFLOAD_DEFAULT=1"
else
	ccflags-y += "-DENABLE_ARP_OFFLOAD_DEFAULT=0"
endif

ifeq ($(CONFIG_MORSE_WATCHDOG_RESET_DEFAULT_DISABLED),y)
	ccflags-y += "-DENABLE_WATCHDOG_DEFAULT=0"
else
	ccflags-y += "-DENABLE_WATCHDOG_DEFAULT=1"
endif

ifeq ($(CONFIG_MORSE_DISABLE_CHANNEL_SURVEY),y)
	ccflags-y += "-DENABLE_SURVEY_DEFAULT=0"
else
	ccflags-y += "-DENABLE_SURVEY_DEFAULT=1"
endif

ifeq ($(CONFIG_MORSE_PAGESET_TRACE),y)
	ccflags-y += "-DPAGESET_TRACE_DEPTH=64"
endif

ifeq ($(CONFIG_MORSE_BUS_TRACE),y)
	ccflags-y += "-DBUS_TRACE_DEPTH=64"
endif

ccflags_trace.o := -I$(src)
CFLAGS_trace.o := -I$(src)

ifeq ($(MORSE_TRACE_PATH),)
	MORSE_TRACE_PATH := .
endif

ifneq ($(MORSE_TRACE_PATH),)
	ccflags-y += -DMORSE_TRACE_PATH=$(MORSE_TRACE_PATH)
endif

obj-$(CONFIG_WLAN_VENDOR_MORSE) += morse.o dot11ah/

morse-y = mac.o
morse-y += init.o
morse-y += skbq.o
morse-y += debug.o
morse-y += trace.o
morse-y += mm8108.o
morse-y += mm6108.o
morse-y += command.o
morse-y += hw.o
morse-y += beacon.o
morse-y += pager_if.o
morse-y += pageset.o
morse-y += ps.o
morse-y += raw.o
morse-y += twt.o
morse-y += cac.o
morse-y += ndpprobe.o
morse-y += of.o
morse-y += firmware.o
morse-y += pager_if_hw.o
morse-y += pager_if_sw.o
morse-y += yaps.o
morse-y += yaps-hw.o
morse-y += watchdog.o
morse-y += event.o
morse-y += crc16_xmodem.o
morse-y += offload.o
morse-y += vendor_ie.o
morse-y += bus_test.o
morse-y += wiphy.o
morse-y += ocs.o
morse-y += mbssid.o
morse-y += mesh.o
morse-y += page_slicing.o
morse-y += pv1.o
morse-y += hw_scan.o
morse-y += coredump.o
morse-y += peer.o
morse-y += led.o
morse-y += bss_stats.o
morse-$(CONFIG_MORSE_MONITOR) += monitor.o
morse-$(CONFIG_MORSE_SDIO) += sdio.o
morse-$(CONFIG_MORSE_SPI) += spi.o
morse-$(CONFIG_MORSE_USB) += usb.o
morse-$(CONFIG_MORSE_VENDOR_COMMAND) += vendor.o
morse-$(CONFIG_MORSE_USER_ACCESS) += uaccess.o
morse-$(CONFIG_MORSE_HW_TRACE) += hw_trace.o
morse-$(CONFIG_MORSE_PAGESET_TRACE) += pageset_trace.o
morse-$(CONFIG_MORSE_BUS_TRACE) += bus_trace.o
morse-$(CONFIG_ANDROID) += apf.o

ifeq ($(CONFIG_DISABLE_MORSE_RC),y)
	morse-y += minstrel_rc.o
else
	morse-y += mmrc/mmrc_osal.o
	morse-y += mmrc-submodule/src/core/mmrc.o
	morse-y += rc.o
	morse-y += mmrc_debugfs.o
endif

SRC := $(shell pwd)

all:
	$(MAKE) MORSE_VERSION=$(MORSE_VERSION) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) MORSE_VERSION=$(MORSE_VERSION) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f  $(morse-y) *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
	make -C ./dot11ah clean
