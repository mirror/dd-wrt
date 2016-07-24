# Copyright (c) 2003-2006 Maksym Sobolyev
# Copyright (c) 2006-2008 Sippy Software, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $Id$

PKGNAME=	rtpproxy
PKGFILES=	GNUmakefile Makefile README extractaudio makeann \
		  rtpproxy.init rtpproxy.sh udp_storm ${SRCS:Nconfig[.]h}

.if !defined(RTPP_DEBUG)
PROG=	rtpproxy
CLEANFILES+=	rtpproxy_debug
LIBSIPLOG=	siplog
.else
PROG=	rtpproxy_debug
CLEANFILES+=	rtpproxy
DEBUG_FLAGS=	-g3 -O0 -DRTPP_DEBUG -include rtpp_memdeb.h
LDADD+=		-static
SRCS+=		${SRCS_DEBUG}
LIBSIPLOG=	siplog_debug
.endif

SRCS+=	main.c rtp_server.c rtp_server.h rtpp_defines.h \
	rtpp_record.c rtpp_record.h rtpp_session.h rtpp_util.c \
	rtpp_util.h rtpp_log.h rtp_resizer.c rtp_resizer.h rtp.c \
	rtp.h rtpp_session.c rtpp_command.c rtpp_command.h \
	rtpp_network.c rtpp_network.h rtpp_log.c rtpp_notify.c \
	rtpp_notify.h rtpp_command_async.h rtpp_command_async.c \
	config.h rtpp_proc_async.c rtpp_proc_async.h rtpp_proc.h \
	rtpp_proc.c rtpp_version.h rtpp_math.c rtpp_math.h rtpp_wi.c \
	rtpp_wi.h rtpp_wi_private.h rtpp_queue.c rtpp_queue.h \
	rtpp_netio_async.c rtpp_netio_async.h rtpp_record_private.h \
	rtpp_command_parse.c rtpp_command_parse.h rtpp_command_copy.c \
	rtpp_command_copy.h rtpp_command_ul.c rtpp_command_ul.h \
	rtpp_hash_table.c rtpp_hash_table.h rtpp_types.h \
	rtpp_pcache.c rtpp_pcache.h config_pp.h rtpp_stats.c \
	rtpp_stats.h rtpp_cfg_stable.h rtpp_command_stats.h \
	rtpp_command_stats.c config_std.h rtp_info.h \
	rtpp_command_private.h rtpp_controlfd.h rtpp_controlfd.c \
	rtpp_list.h rtpp_command_stream.h rtpp_command_stream.c
SRCS_DEBUG=	rtpp_memdeb.c rtpp_memdeb.h rtpp_memdeb_internal.h \
                rtpp_memdeb_stats.h rtpp_memdeb_test.c
MAN1=

WARNS?=	2

LOCALBASE?=	/usr/local
BINDIR?=	${LOCALBASE}/bin

CFLAGS+=	-I../siplog -I${LOCALBASE}/include
LDADD+=	-L../siplog -L${LOCALBASE}/lib -l${LIBSIPLOG} -lpthread -lm

cleantabs:
	perl -pi -e 's|        |\t|g' ${SRCS}

TSTAMP!=	date "+%Y%m%d%H%M%S"
GIT_BRANCH!=	sh -c "git branch | grep '^[*]' | awk '{print \$$2}'"
.if ${GIT_BRANCH} == "master"
RTPP_SW_VERSION=	${TSTAMP}
.else
RTPP_SW_VERSION=	${TSTAMP}_${GIT_BRANCH}
.endif

CLEANFILES+=	config.h

config.h: config_std.h
	cp ${.ALLSRC} ${.TARGET}

distribution: clean
	echo '#define RTPP_SW_VERSION "'rel.${RTPP_SW_VERSION}'"' > rtpp_version.h
	git commit -m "Update to rel.${RTPP_SW_VERSION}" rtpp_version.h
	git push origin ${GIT_BRANCH}
	tar cvfy /tmp/${PKGNAME}-sippy-${RTPP_SW_VERSION}.tbz2 ${PKGFILES}
	scp /tmp/${PKGNAME}-sippy-${RTPP_SW_VERSION}.tbz2 sobomax@download.sippysoft.com:/usr/local/www/data/rtpproxy/
	git tag rel.${RTPP_SW_VERSION}
	git push origin rel.${RTPP_SW_VERSION}

includepolice:
	for file in ${SRCS}; do \
	  SMAKE="${MAKE}" SMAKEFLAGS="-f ../mk/Makefile" python ../misc/includepolice.py $${file} || sleep 5; \
	done

.include <bsd.prog.mk>
