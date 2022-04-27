#
# Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2004 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/branches/sam_hal/darwin/Makefile#1 $
#
DEPTH=	..

#
# Makefile for building the Atheros/MADWIFI HAL from source code with gmake
#
HAL=	${DEPTH}
PUBLIC=	${HAL}/public

#
ifdef TARGET
ALL=	${TARGET}
else
ALL=	powerpc-be-macho  i386-macho
endif
#

all:	${ALL} universal

release:
	for i in ${ALL}; do \
		${MAKE} $$i-release; \
	done

symrelease:
	for i in ${ALL}; do \
		${MAKE} $$i-symrelease; \
	done

clean:
	rm -f obj/hal_universal.o
	for i in ${ALL}; do \
		${MAKE} $$i-clean; \
	done


universal: ${ALL}
	find obj -name hal.o | xargs lipo -create -output obj/hal_universal.o

#
# XXX someone with some gmake foo should be able to do this right...
#
powerpc-be-macho: ${PUBLIC}/powerpc-be-macho.inc
	${MAKE} -f Makefile.inc TARGET=powerpc-be-macho ARCH="-arch ppc"
powerpc-be-macho-release:
	${MAKE} -f Makefile.inc TARGET=powerpc-be-macho ARCH="-arch ppc" release
powerpc-be-macho-symrelease:
	${MAKE} -f Makefile.inc TARGET=powerpc-be-macho ARCH="-arch ppc" symrelease
powerpc-be-macho-clean:
	${MAKE} -f Makefile.inc TARGET=powerpc-be-macho ARCH="-arch ppc" clean

i386-macho: ${PUBLIC}/i386-macho.inc
	${MAKE} -f Makefile.inc TARGET=i386-macho ARCH="-arch i386"
i386-macho-release:
	${MAKE} -f Makefile.inc TARGET=i386-macho ARCH="-arch i386" release
i386-macho-symrelease:
	${MAKE} -f Makefile.inc TARGET=i386-macho ARCH="-arch i386" symrelease
i386-macho-clean:
	${MAKE} -f Makefile.inc TARGET=i386-macho ARCH="-arch i386" clean

