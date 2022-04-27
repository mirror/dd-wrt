#
# Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2006 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/branches/sam_hal/ardecode/Makefile#2 $
#
DESTDIR=

CFLAGS=	-I. -I.. -g

OBJS=	main.o ar5210.o ar5211.o ar5212.o

ardecode: ${OBJS}
	${CC} -o ardecode ${OBJS}

install:
	rm -f ${DESTDIR}/usr/local/bin/ardecode
	cp ardecode ${DESTDIR}/usr/local/bin

clean:
	rm -f ardecode ${OBJS} core
