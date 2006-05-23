#
# Makefile for the Atheros WLAN driver.
#


HAL=	$(src)/../hal
ATH_HAL=$(src)/../_ath_hal
WLAN=	${src}/../net80211
COMPAT=	${WLAN}/compat

EXTRA_CFLAGS+= -DATH_SUPERG_FF=1
EXTRA_CFLAGS+= -DATH_SUPERG_DYNTURBO=1
EXTRA_CFLAGS+= -DATH_SUPERG_SCAN=1
EXTRA_CFLAGS+= -DATH_PHYERR_DIAG=1

#
# Select bus-specific code.  Note that this defaults to PCI.
#
ifeq ($(strip ${BUS}),AHB)
BUSNAME=ahb
EXTRA_CFLAGS+= -DATH_AHB
else
BUSNAME=pci
EXTRA_CFLAGS+= -DATH_PCI
endif

INCS=	-include ${COMPAT}/compat.h -I${COMPAT}
EXTRA_CFLAGS+=	${INCS} -I${HAL} -I${HAL}/linux -I${ATH_HAL} -I${WLAN} \
	-I${src}/..

obj-$(CONFIG_ATHEROS) += ath_${BUSNAME}.o
ath_${BUSNAME}-objs	:= if_ath.o if_ath_${BUSNAME}.o



