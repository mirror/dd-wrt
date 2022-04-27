#
# Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2006 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/branches/sam_hal/diag/Makefile#5 $
#
DEPTH=	..

DESTDIR=/usr/local/bin
ATHDIR=	/data/freebsd/head/sys/dev/ath
ATH_DEFAULT=ath0

COPTS=	-g
CFLAGS=	-I. -I${DEPTH} -I${ATHDIR}
CFLAGS+=-DATH_DEFAULT='"${ATH_DEFAULT}"' ${COPTS}
LDFLAGS=

ALL=	eeprom rfgain ani txpow dumpregs key reg

all:	${ALL}

eeprom:	eeprom.c
	${CC} -o eeprom ${LDFLAGS} ${CFLAGS} eeprom.c
rfgain:	rfgain.o diag.o
	${CC} -o rfgain ${LDFLAGS} ${CFLAGS} rfgain.o diag.o
ani:	ani.o diag.o
	${CC} -o ani ${LDFLAGS} ${CFLAGS} ani.o diag.o
txpow:	txpow.c
	${CC} -o txpow ${LDFLAGS} ${CFLAGS} txpow.c
dumpregs: dumpregs.o dumpregs_5210.o dumpregs_5211.o dumpregs_5212.o dumpregs_5416.o
	${CC} -o dumpregs ${LDFLAGS} ${CFLAGS} dumpregs.o \
		dumpregs_5210.o dumpregs_5211.o dumpregs_5212.o dumpregs_5416.o
key:	key.c
	${CC} -o key ${LDFLAGS} ${CFLAGS} key.c
reg:	reg.c
	${CC} -o reg ${LDFLAGS} ${CFLAGS} reg.c
11ncompat: 11ncompat.c
	${CC} -o 11ncompat ${LDFLAGS} ${CFLAGS} 11ncompat.c

install: ${ALL}
	for i in ${ALL}; do \
		rm -f ${DESTDIR}/$$i; \
		cp $$i ${DESTDIR}/$$i; \
	done

clean:
	rm -f ${ALL} *.o core
