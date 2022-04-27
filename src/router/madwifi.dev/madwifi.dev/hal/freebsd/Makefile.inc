#
# Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2006 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/branches/sam_hal/freebsd/Makefile.inc#2 $
#

#
# Hardware Access Layer (HAL) for the Atheros Wireless NIC driver.
#
# This module contains the hardware-specific bits for the network
# interface driver.  It is built as a separate module to simplify
# maintenance and isolate the bits that are not (currently) distributed
# in source form.
#
# There is support for 3 "generations" of Atheros hardware devices:
#
# AH_SUPPORT_AR5210	802.11a only MAC
# AH_SUPPORT_AR5211	802.11a+802.11b MAC (also pure g, but not yet supported)
# AH_SUPPORT_AR5212	802.11a+802.11b+802.11g MAC
# AH_SUPPORT_AR5312     5212 MAC WiSoC (requires AH_SUPPORT_AR5212)
#
# AH_SUPPORT_5111	5111 RF support for 5212 et. al.
# AH_SUPPORT_5112	5112 RF support for 5212 et. al.
# AH_SUPPORT_2413	2413 RF support for 5212 et. al.
# AH_SUPPORT_2316	2416 RF (Cobra) support for 5212 et. al.
#
# and numerous optional debugging facilities (typically named AH_DEBUG_*):
#
# AH_DEBUG		enables debugging stuff that's controlled by the sysctl:
# 			hw.ath.hal.debug
# AH_ASSERT 		enables assertions that panic or trap into the debugger.
# AH_DEBUG_ALQ 		enables support for tracing register reads+writes
#			via alq
# AH_DEBUG_COUNTRY	accept the CTR_DEBUG country code (for debugging
#			the regulatory domain support)
# AH_REGOPS_FUNC	arrange for OS_REG_READ and OS_REG_WRITE to be function
#			calls and not inline expanded to memory references
# AH_ENABLE_AP_SUPPORT	enables AP-specific support not publicly available
#			(e.g. don't cap turbo power limits)
# AH_DISABLE_WME	disable HAL_TXQ_USE_LOCKOUT_BKOFF_DIS support
#
# AH_PRIVATE_DIAG	enable private diagnostic support
# AH_WRITE_EEPROM	permit EEPROM writes (use with AH_PRIVATE_DIAG)
# AH_WRITE_REGDOMAIN	permit EEPROM writes of the regulatory domain through
#			ah_setRegulatoryDomain (implies AH_WRITE_EEPROM)
#
AH_SUPPORT_AR5210?=1		# 5210 support
AH_SUPPORT_AR5211?=1		# 5211 support
AH_SUPPORT_AR5212?=1		# 5212 support
AH_SUPPORT_AR5312?=0		# 5312 support
AH_SUPPORT_5111?=1		# 5111 RF support
AH_SUPPORT_5112?=1		# 5112 RF support
AH_SUPPORT_2413?=1		# 2413 RF support
AH_SUPPORT_2316?=0		# 2413 RF support

AH_DEBUG?=0			# debugging (e.g. hw.ath.hal.debug)
AH_ASSERT?=0			# assertions (pretty lame)
AH_DEBUG_ALQ?=0			# ALQ register tracing support
AH_REGOPS_FUNC?=0		# use inline memory references
AH_WRITE_REGDOMAIN?=0		# support EEPROM writes of the regulatory domain
AH_DEBUG_COUNTRY?=0		# accept CTR_DEBUG country code
AH_DISABLE_WME?=0		# disable HAL_TXQ_USE_LOCKOUT_BKOFF_DIS
AH_ENABLE_AP_SUPPORT?=0		# enable AP-specific support
AH_WRITE_EEPROM?=0		# support EEPROM writes
AH_PRIVATE_DIAG?=0		# non-public diagnostic support
AH_NEED_DESC_SWAP?=0		# Need to swap TX descriptor
AH_SUPPORT_XR?=0		# enable XR support

#
# If 5312 support is enabled it's almost certain we're not going
# to use the 5210 or 5211 support so disable it to save some space.
#
.if ${AH_SUPPORT_AR5312}
AH_SUPPORT_AR5210=0
AH_SUPPORT_AR5211=0
AH_SUPPORT_AR5414=0
.endif

.PATH:	${HAL}
HAL_SRCS+=ah.c ah_eeprom.c ah_regdomain.c

.PATH: ${HAL}/ar5210
HAL_SRCS+=ar5210_attach.c ar5210_beacon.c ar5210_interrupts.c \
	ar5210_keycache.c ar5210_misc.c ar5210_power.c ar5210_phy.c \
	ar5210_radar.c ar5210_recv.c ar5210_reset.c ar5210_xmit.c
.PATH: ${HAL}/ar5211
HAL_SRCS+=ar5211_attach.c ar5211_beacon.c ar5211_interrupts.c \
	ar5211_keycache.c ar5211_misc.c ar5211_power.c ar5211_phy.c \
	ar5211_radar.c ar5211_recv.c ar5211_reset.c ar5211_xmit.c
.PATH: ${HAL}/ar5212
HAL_SRCS+=ar5212_ani.c ar5212_attach.c ar5212_beacon.c ar5212_interrupts.c \
	ar5212_eeprom.c ar5212_gpio.c \
	ar5212_keycache.c ar5212_misc.c ar5212_power.c ar5212_phy.c \
	ar5212_radar.c ar5212_recv.c ar5212_reset.c ar5212_xmit.c

.PATH: ${HAL}/ar5312
HAL_SRCS+=ar5312_attach.c ar5312_interrupts.c ar5312_misc.c \
	ar5312_eeprom.c ar5312_gpio.c ar5315_gpio.c \
	ar5312_power.c ar5312_reset.c ar5312_bsp.c

HAL_SRCS+=ar5111.c ar5112.c ar2413.c ar2316.c

HAL_OBJS=${HAL_SRCS:N*.h:R:S/$/.o/g}
CLEANFILES+=	${HAL_OBJS}

.if ${AH_DEBUG}
COPTS?=	-g			# build with symbols
.endif

opt_ah.h: Makefile.inc
	rm -f opt_ah.h
.if ${AH_SUPPORT_AR5210}
	echo "#define AH_SUPPORT_AR5210 1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_AR5211}
	echo "#define AH_SUPPORT_AR5211 1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_AR5212}
	echo "#define AH_SUPPORT_AR5212 1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_AR5312}
	echo "#define AH_SUPPORT_AR5312	1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_5111}
	echo "#define AH_SUPPORT_5111	1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_5112}
	echo "#define AH_SUPPORT_5112	1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_2413}
	echo "#define AH_SUPPORT_2413	1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_2316}
	echo "#define AH_SUPPORT_2316	1"	>>opt_ah.h
.endif
.if ${AH_DEBUG}
	echo "#define AH_DEBUG 1"		>>opt_ah.h
.endif
.if ${AH_ASSERT}
	echo "#define AH_ASSERT 1"		>>opt_ah.h
.endif
.if ${AH_DEBUG_ALQ}
	echo "#define AH_DEBUG_ALQ 1"		>>opt_ah.h
.endif
.if ${AH_WRITE_REGDOMAIN}
	echo "#define AH_SUPPORT_WRITE_EEPROM 1"    >>opt_ah.h
	echo "#define AH_SUPPORT_WRITE_REGDOMAIN 1" >>opt_ah.h
.elsif ${AH_WRITE_EEPROM}
	echo "#define AH_SUPPORT_WRITE_EEPROM 1"    >>opt_ah.h
.endif
.if ${AH_DEBUG_COUNTRY}
	echo "#define AH_DEBUG_COUNTRY 1"	>>opt_ah.h
.endif
.if ${AH_DISABLE_WME}
	echo "#define AH_DISABLE_WME 1"		>>opt_ah.h
.endif
.if ${AH_ENABLE_AP_SUPPORT}
	echo "#define AH_ENABLE_AP_SUPPORT 1"	>>opt_ah.h
.endif
.if ${AH_PRIVATE_DIAG}
	echo "#define AH_PRIVATE_DIAG 1"	>>opt_ah.h
.endif
.if ${AH_NEED_DESC_SWAP}
	echo "#define AH_NEED_DESC_SWAP 1"	>>opt_ah.h
.endif
.if ${AH_SUPPORT_XR}
	echo "#define AH_SUPPORT_XR 1"		>>opt_ah.h
.endif

ah.o:	version.h
${HAL_OBJS}: opt_ah.h

LD?=	ld

hal.o:	${HAL_OBJS} Makefile
	${LD} -o $@ -r ${HAL_OBJS}
