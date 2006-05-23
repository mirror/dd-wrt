#
# Makefile for the Atheros HAL Module.
#


#
# Use ARCH to guess the target platform.
# XXX use .config contents in addition?
#
ifeq ($(strip ${ARCH}),i386)
TARGET=	i386-elf
endif
ifeq ($(strip ${ARCH}),mips)
TARGET=	mips-le-elf
endif
ifeq ($(strip ${ARCH}),x86_64)
TARGET=	x86_64-elf
endif

EXTRA_CFLAGS+=	-I. -I${HAL}/.. -I${HAL} -I${HAL}/linux -DAH_BYTE_ORDER=AH_LITTLE_ENDIAN

ifndef HAL_OBJDIR
#
# Use release version of HAL.
#
HAL=		$(src)/../hal
OPT_AH_H=	${HAL}/public/${TARGET}.opt_ah.h
else
#
# Use private version of HAL.
#
HAL=		$(shell dirname `dirname ${HAL_OBJDIR}`)
OPT_AH_H=	${HAL_OBJDIR}/opt_ah.h
endif

obj-$(CONFIG_ATHEROS_HAL) += ath_hal.o
ath_hal-objs	:= ah_osdep.o hal.o




ifndef HAL_OBJDIR
$(obj)/hal.o:	${HAL}/public/${TARGET}.hal
	cp $< $(obj)/hal.o
else
$(obj)/hal.o:	${HAL_OBJDIR}/hal.o
	cp ${HAL_OBJDIR}/hal.o $(obj)/hal.o
endif

# XXX need opt_ah.h to do dependencies so give this bogus dependency
$(obj)/ah_osdep.c: ${HAL}/linux/ah_osdep.c $(obj)/opt_ah.h
	cp ${HAL}/linux/ah_osdep.c $(obj)/ah_osdep.c
$(obj)/opt_ah.h: ${OPT_AH_H}
	cp ${OPT_AH_H} $(obj)/opt_ah.h
