#
# Makefile for Sample's rate control algorithm.
#
# $Id: //depot/sw/linuxsrc/src/802_11/madwifi/madwifi/ath_rate/sample/Makefile.kernel#1 $
#


HAL=	$(src)/../hal
ATH=	$(src)/../ath
WLAN=	${src}/../net80211
COMPAT=	${WLAN}/compat

INCS=	-include ${COMPAT}/compat.h -I${COMPAT}
EXTRA_CFLAGS+=	${INCS} -I${HAL} -I${HAL}/linux -I${ATH} -I${WLAN} -I${src}/.. -DAH_BYTE_ORDER=AH_LITTLE_ENDIAN

obj-$(CONFIG_ATHEROS_RATE) += ath_rate_sample.o
ath_rate_sample-objs	:= sample.o
