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

PROG=	extractaudio
SRCS=	main.c decoder.c rtpp_loader.c _rtp.c _rtp_analyze.c _rtpp_util.c \
	  _rtpp_log_stand.c
CLEANFILES+= _rtp.c _rtp_analyze.c _rtpp_util.c _rtpp_log_stand.c
MAN1=

WARNS?=	2

LOCALBASE?=	/usr/local

CFLAGS=-g3 -O0 -DWITHOUT_SIPLOG
.if defined(WITHOUT_G722)
CFLAGS+= -DWITHOUT_G722
.else
LDADD+= -lg722
.endif
.if defined(WITHOUT_G729)
CFLAGS+= -DWITHOUT_G729
.else
LDADD+= -lg729
.endif

CFLAGS+= -I${.CURDIR}/.. -I${LOCALBASE}/include
LDFLAGS+= -L${LOCALBASE}/lib
LDADD+=	-lsndfile -lm

_rtp.c: ${.CURDIR}/../rtp.c
	ln -sf ${.CURDIR}/../rtp.c _rtp.c

_rtp_analyze.c: ${.CURDIR}/../rtp_analyze.c
	ln -sf ${.CURDIR}/../rtp_analyze.c _rtp_analyze.c

_rtpp_util.c: ${.CURDIR}/../rtpp_util.c
	ln -sf ${.CURDIR}/../rtpp_util.c _rtpp_util.c

_rtpp_log_stand.c: ${.CURDIR}/../rtpp_log_stand.c
	ln -sf ${.CURDIR}/../rtpp_log_stand.c _rtpp_log_stand.c

includepolice:
	for file in ${SRCS}; do \
	  ${MAKE} $${file}; \
	  python ${.CURDIR}/../misc/includepolice.py $${file} || sleep 5; \
	done

.include <bsd.prog.mk>
