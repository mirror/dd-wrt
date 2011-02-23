# $OpenBSD$

COMMENT=	"Automatic IPv6 Connectivity Configuration Utility (AICCU)"

PKGNAME=	sixxs-aiccu-2007-01-07
DISTNAME=	aiccu_20070107
CATEGORIES=	net

HOMEPAGE=	http://www.sixxs.net/tools/aiccu/
MASTER_SITES=	http://www.sixxs.net/archive/sixxs/aiccu/unix/

MAINTAINER=	Thomas Kaschwig <openbsd@kaschwig.net>

SED=		/usr/bin/sed

# GPL
PERMIT_PACKAGE_CDROM=   Yes
PERMIT_PACKAGE_FTP=     Yes
PERMIT_DISTFILES_CDROM= Yes
PERMIT_DISTFILES_FTP=   Yes
WANTLIB=		c pthread

EXTRACT_SUFX=	.tar.gz
WRKDIST=	${WRKDIR}/aiccu

NO_REGRESS=	Yes

USE_GMAKE=	Yes
DOCDIR=		${PREFIX}/share/doc/sixxs-aiccu
EXAMPLEDIR=	${PREFIX}/share/examples/sixxs-aiccu

pre-build:
	${SED}	-e 's:ipv6_interface sixxs:ipv6_interface gif0:' \
			${WRKSRC}/doc/aiccu.conf > ${WRKSRC}/aiccu.conf

do-install:
	${INSTALL_DATA_DIR} ${DOCDIR} ${EXAMPLEDIR}
	${INSTALL_PROGRAM} ${WRKSRC}/unix-console/aiccu ${PREFIX}/sbin/sixxs-aiccu
	${INSTALL_DATA} ${WRKSRC}/aiccu.conf ${SYSCONFDIR}/aiccu.conf.sample
	${INSTALL_DATA} ${WRKSRC}/aiccu.conf ${EXAMPLEDIR}
	${INSTALL_DATA} ${WRKSRC}/doc/README ${DOCDIR}
	${INSTALL_DATA} ${WRKSRC}/doc/LICENSE ${DOCDIR}
	${INSTALL_DATA} ${WRKSRC}/doc/HOWTO ${DOCDIR}

.include <bsd.port.mk>
