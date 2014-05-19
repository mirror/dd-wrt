#
# Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2005 Atheros Communications, Inc.
# All rights reserved.
#
ifeq ($(obj),)
obj := .
endif

DEPTH := ../../../../..

BINDIR= /usr/local/bin

include Makefile.inc

INSTALL_ROOT = ${DEPTH}/rootfs-${BOARD_TYPE}.optbuild/
INSTALL_ROOT2 = ${DEPTH}/rootfs-${BOARD_TYPE}.build/
INCS+=  -I${HAL} -I${HAL}/${OS} -I${ATH_HAL} -I${obj}/${DEPTH} -I${COMMON} -I${ATH_RATE} -I${ATH} -I${WLAN}
CFLAGS= ${INCS} -g -O2 -Wall

ifeq (${BUILD_STATIC}, y)
LDFLAGS= -static
endif

all:    ${ALL}


ALL=	ethreg

all: clean ${ALL}

ethreg:	ethreg.c
	${CC} -o ethreg ${CFLAGS} ethreg.c

strip_all:
	${STRIP} ethreg	

install: all strip_all
	for i in ${ALL}; do \
		rm -f $(INSTALL_ROOT)/usr/bin/$$i; \
		rm -f $(INSTALL_ROOT2)/usr/bin/$$i; \
		cp $$i $(INSTALL_ROOT)/usr/bin/$$i; \
		cp $$i $(INSTALL_ROOT2)/usr/bin/$$i; \
	done

clean:
	rm -f ${ALL} 
