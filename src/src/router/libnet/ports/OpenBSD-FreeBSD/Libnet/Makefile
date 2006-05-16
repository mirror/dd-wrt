# New ports collection makefile for:    libnet
# Version required:                     0.99c
# Dated Created:                        1 January 1999
# Dated Updated:                        23 May 1999
# Whom:                                 Mike D. Schiffman
#
# $Id: Makefile,v 1.1 2004/04/27 01:32:20 dyang Exp $
#

#EXTRACT_SUFX=	.tgz

DISTNAME=       libnet
PKGNAME=        libnet-0.99
CATEGORIES=     net
MASTER_SITES=   http://www.packetfactory.net/libnet
MAINTAINER=     mike@infonexus.com
GNU_CONFIGURE=  yes

# Only during the fetch phase.
# This can be removed along with the lynx fetch command when someone puts
# Libnet up on an ftp site.
IS_INTERACTIVE= yes
FETCH_CMD=      /usr/bin/lynx

WRKSRC=         ${WRKDIR}/Libnet

.include <bsd.port.mk>

# EOF
