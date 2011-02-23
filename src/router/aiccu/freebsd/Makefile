# New ports collection makefile for:   sixxs-aiccu
# Date created:                2004-09-09
# Initial FreeBSD port by: Meno Abels <meno.abels@adviser.com>
# Comments for this port please to the SixXS staff <info@sixxs.net>
#
## $FreeBSD: /repoman/r/pcvs/ports/net/sixxs-aiccu/Makefile,v 1.1 2005/03/14 17:47:07 vs Exp $
#

#PORTNAME=	sixxs-aiccu
PORTVERSION=	20070107
CATEGORIES=	net ipv6
MASTER_SITES=	http://www.sixxs.net/archive/sixxs/aiccu/unix/
DISTNAME=	aiccu_20070107

# Maintainer of the FreeBSD port, but always copy: info@sixxs.net
MAINTAINER=	meno.abels@adviser.com
COMMENT=	IPv6 Tunnel Broker client capable of configuring using TIC, static, AYIYA and heartbeat tunnels

USE_RC_SUBR=	sixxs-aiccu
USE_GMAKE=	yes
CFLAGS+=	${PTHREAD_CFLAGS}
LDFLAGS+=	${PTHREAD_LIBS}
MAKE_ARGS=	CC="${CC}"
MAKE_ENV=	CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}"
WRKSRC=		${WRKDIR}/aiccu
BUILD_WRKSRC=	${WRKDIR}/aiccu/unix-console

PORTDOCS=	README

# FreeBSD doesn't use named interfaces, thus default to gif0
pre-build:
	@${SED} \
		-e 's:ipv6_interface sixxs:ipv6_interface gif0:' \
		${WRKSRC}/doc/aiccu.conf > ${WRKDIR}/aiccu.conf

do-install:
	@${MKDIR} ${DOCSDIR} ${EXAMPLESDIR}
	${INSTALL_PROGRAM} ${WRKSRC}/unix-console/aiccu ${PREFIX}/sbin/sixxs-aiccu
	${INSTALL_DATA} ${WRKSRC}/doc/README ${DOCSDIR}
	${INSTALL_DATA} ${WRKSRC}/doc/LICENSE ${DOCSDIR}
	${INSTALL_DATA} ${WRKSRC}/doc/aiccu.conf ${EXAMPLESDIR}
	${INSTALL_DATA} ${WRKDIR}/aiccu.conf ${PREFIX}/etc/aiccu.conf.sample

.include <bsd.port.mk>

