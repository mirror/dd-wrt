/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _RTPP_LOG_H_
#define _RTPP_LOG_H_

#ifdef WITHOUT_SIPLOG
#include "rtpp_log_stand.h"
#else
#include <siplog.h>

#define	rtpp_log_t	siplog_t

#define	RTPP_LOG_DBUG	SIPLOG_DBUG
#define	RTPP_LOG_INFO	SIPLOG_INFO
#define	RTPP_LOG_WARN	SIPLOG_WARN
#define	RTPP_LOG_ERR	SIPLOG_ERR
#define	RTPP_LOG_CRIT	SIPLOG_CRIT

#define	rtpp_log_open(gf, app, call_id, flags) siplog_open(app, call_id, flags)
#define	rtpp_log_setlevel(handle, level) siplog_set_level(handle, level)
#define	rtpp_log_write(level, handle, format, args...) siplog_write(level, handle, format, ## args)
#define	rtpp_log_ewrite(level, handle, format, args...) siplog_ewrite(level, handle, format, ## args)
#define	rtpp_log_close(handle) siplog_close(handle)
#endif /* !WITHOUT_SIPLOG */

int rtpp_log_str2lvl(const char *);
int rtpp_log_str2fac(const char *);

#endif
