#
# Copyright 2020-2026 Morse Micro
# SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
#
# Makefile for the morse-ctrl collection of utilities

# Note: this will default to hiding away the command lines of executed commands to make
#       the console output easier to read.
#       This can be disabled by overriding V to a value other than 0.
V ?= 0

ifeq ($(V),0)
Q = @
endif


override MORSECTRL_VERSION_STRING = "rel_mm8108_2_0_0_2026_Apr_21"
DEFAULT_INTERFACE_NAME ?= "wlan0"
PKG_CONFIG ?= pkg-config

MORSECTRL_CFLAGS_COMMON := $(CFLAGS) $(COPTS) $(MIPS16_OPT) $(LTO)
MORSECTRL_CFLAGS_COMMON += -Wall -Werror
MORSECTRL_CFLAGS_COMMON += -DMORSECTRL_VERSION_STRING="\"$(MORSECTRL_VERSION_STRING)\""
MORSECTRL_CFLAGS += $(MORSECTRL_CFLAGS_COMMON)
MORSECTRL_CFLAGS += -DDEFAULT_INTERFACE_NAME="\"$(DEFAULT_INTERFACE_NAME)\""
MORSECTRL_LDFLAGS_COMMON := $(LDFLAGS)

DEPS := $(wildcard *.h)
DEPS += $(wildcard */*.h)

STATS_DECODER_SRCS := elf_file_read.c
STATS_DECODER_SRCS += offchip_statistics.c
STATS_DECODER_SRCS += utilities.c
STATS_DECODER_SRCS += stats_decode.c
STATS_DECODER_SRCS += stats_format.c
STATS_DECODER_SRCS += stats_format_regular.c
STATS_DECODER_SRCS += stats_format_json.c

SRCS := morsectrl.c
SRCS += config_file.c
SRCS += elf_file_load.c
SRCS += command.c
SRCS += version.c
SRCS += hw_version.c
SRCS += stats.c
SRCS += channel.c
SRCS += bsscolor.c
SRCS += ampdu.c
SRCS += raw.c
SRCS += health.c
SRCS += cts_self_ps.c
SRCS += long_sleep.c
SRCS += duty_cycle.c
SRCS += coredump.c
SRCS += opclass.c
SRCS += tx_pkt_lifetime_us.c
SRCS += maxampdulen.c
SRCS += macaddr.c
SRCS += reset.c
SRCS += wakeaction.c
SRCS += standby.c
SRCS += mpsw.c
SRCS += dhcpc.c
SRCS += keep_alive.c
SRCS += tcp_keepalive.c
SRCS += vendor_ie.c
SRCS += twt.c
SRCS += cac.c
SRCS += params.c
SRCS += uapsd.c
SRCS += li.c
SRCS += whitelist.c
SRCS += arp_periodic_refresh.c
SRCS += otp.c
SRCS += boardtype.c
SRCS += country_code.c
SRCS += power.c
SRCS += rc_stats.c
SRCS += tcp_periodic.c
SRCS += tx_polar.c
SRCS += medium_eval.c
SRCS += connect.c
SRCS += hmi.c
SRCS += $(STATS_DECODER_SRCS)

SRCS += transport/transport.c

LIB_SRCS += argtable3/argtable3.c

WIN_LIB_SRCS += win/strsep.c
LINUX_SRCS += gpioctrl.c

LINUX_LDFLAGS_COMMON := -lm
LINUX_LDFLAGS += $(LINUX_LDFLAGS_COMMON)
ifeq ($(CONFIG_MORSE_STATIC),1)
	MORSECTRL_LDFLAGS_COMMON += -static
endif

MORSECTRL_LDFLAGS := $(MORSECTRL_LDFLAGS_COMMON)

WIN_LDFLAGS += -lws2_32

#ifeq ($(CONFIG_MORSE_TRANS_NL80211),1)
	ifeq ($(CFLAGS),)
		LINUX_CFLAGS += -I${SYSROOT}/usr/include/libnl3
	endif
	ifeq ($(CONFIG_ANDROID),1)
		LINUX_LDFLAGS += -lnl
	else
		LINUX_LDFLAGS += -lnl-3 -lnl-genl-3 -lpthread
	endif
	LINUX_SRCS += transport/nl80211.c
#endif
	LINUX_CFLAGS += -D_GNU_SOURCE

ifeq ($(CONFIG_MORSE_TRANS_UART_SLIP),1)
	SRCS += transport/slip.c
	SRCS += transport/uart_slip.c
	LINUX_SRCS += transport/uart_linux.c
	WIN_SRCS += transport/uart_win.c
endif

ifeq ($(CONFIG_MORSE_TRANS_TCP_SLIP),1)
	SRCS += transport/slip.c
	LINUX_SRCS += transport/tcp_slip.c
endif

ifeq ($(CONFIG_MORSE_TRANS_FTDI_SPI),1)
	SRCS += transport/ftdi_spi.c
	SRCS += transport/sdio_over_spi.c
	LIB_SRCS += transport/libmpsse/source/ftdi_i2c.c
	LIB_SRCS += transport/libmpsse/source/ftdi_infra.c
	LIB_SRCS += transport/libmpsse/source/ftdi_mid.c
	LIB_SRCS += transport/libmpsse/source/ftdi_spi.c
	LIB_SRCS += transport/libmpsse/source/memcpy.c

	MAJOR_VERSION = 1
	MINOR_VERSION = 0
	BUILD_VERSION = 3
	MORSECTRL_CFLAGS += -DFTDIMPSSE_STATIC
	MORSECTRL_CFLAGS += -DENABLE_TRANS_FTDI_SPI
	MORSECTRL_CFLAGS += -DFT_VER_MAJOR=$(MAJOR_VERSION)
	MORSECTRL_CFLAGS += -DFT_VER_MINOR=$(MINOR_VERSION)
	MORSECTRL_CFLAGS += -DFT_VER_BUILD=$(BUILD_VERSION)
	MORSECTRL_CFLAGS += -Itransport/libmpsse/include
	MORSECTRL_CFLAGS += -Itransport/libmpsse/libftd2xx
	MORSECTRL_CFLAGS += -Itransport/libmpsse/source
	MORSECTRL_LDFLAGS += -Wl,--wrap=memcpy

	LINUX_CFLAGS += -D_GNU_SOURCE
	ifeq ($(CONFIG_ANDROID),1)
		LINUX_LDFLAGS += -ldl
	else
		LINUX_LDFLAGS += -lpthread -lrt -ldl
	endif
endif

ifeq ($(CONFIG_MORSE_TRANS_OFFLINE),1)
	SRCS += transport/offline.c
endif

# by default, enable USB. But force it to disable if Windows or Android.
CONFIG_MORSE_USB ?= 0
ifeq ($(CONFIG_ANDROID),1)
	CONFIG_MORSE_USB = 0
endif
ifeq ($(MORSE_WIN_BUILD),1)
	CONFIG_MORSE_USB = 0
endif

ifeq ($(CONFIG_MORSE_USB),1)
LINUX_SRCS += usb.c

	LINUX_CFLAGS  += -I$(TOP)/usb_modeswitch/libusb/libusb/ -DNEED_PRINTF
	LINUX_LDFLAGS += $(TOP)/usb_modeswitch/libusb/libusb/.libs/libusb-1.0.a
endif

# Set Windows Vista as the minimum supported windows version
WIN_CFLAGS += -DMORSE_WIN_BUILD -D__USE_MINGW_ANSI_STDIO -D_WIN32_WINNT=0x0600
WIN_CC ?= x86_64-w64-mingw32-gcc

SRCS += $(LIB_SRCS)
WIN_SRCS += $(WIN_LIB_SRCS)
LINUX_SRCS += $(LINUX_LIB_SRCS)

# Decorate inclusions of other targets for make_code_release.py


MORSE_CLI_CFLAGS = $(MORSECTRL_CFLAGS)
MORSE_CLI_LDFLAGS = $(MORSECTRL_LDFLAGS)

MORSE_CLI_CFLAGS += -DMORSE_CLIENT

all: morse_cli

# As noted in transport.c, the default transport is the first transport linked. Therefore we
# put LINUX_SRCS before SRCS so that nl80211 has higher priority.
CLIENT_OBJS = $(patsubst %.c, %_cli.o, $(LINUX_SRCS) $(SRCS))
CLIENT_OBJS_WIN = $(patsubst %.c, %_cli_win.o, $(SRCS) $(WIN_SRCS))

%_cli.o: %.c $(DEPS)
	@echo Compiling $<
	$(Q) $(CC) $(MORSE_CLI_CFLAGS) $(LINUX_CFLAGS) -c -o $@ $<

morse_cli: $(CLIENT_OBJS)
	@echo Linking $@
	$(Q) $(CC) $(MORSE_CLI_CFLAGS) $(LINUX_CFLAGS) -o $@ $^ \
		$(MORSE_CLI_LDFLAGS) $(LINUX_LDFLAGS)


%_cli_win.o: %.c $(DEPS)
	@echo Compiling $<
	$(Q) $(WIN_CC) $(MORSE_CLI_CFLAGS) $(WIN_CFLAGS) -c -o $@ $<

morse_cli_win: $(CLIENT_OBJS_WIN)
	@echo Linking $@
	$(Q) $(WIN_CC) $(MORSE_CLI_CFLAGS) $(WIN_CFLAGS) -o morse_cli $^ \
		$(MORSE_CLI_LDFLAGS) $(WIN_LDFLAGS)

install:
	@echo Installing morse_cli to /usr/bin
	mkdir -p $(INSTALLDIR)/usr/bin
	$(Q) cp morse_cli $(INSTALLDIR)/usr/bin

clean:
	rm -rf morsectrl morse_cli morse_stats_decoder *.exe output
	find . -iname '*.o' -exec rm {} \;

