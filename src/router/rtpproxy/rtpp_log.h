/*
 * Copyright (c) 2004 Maxim Sobolev
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
 * $Id: rtpp_log.h,v 1.3 2006/04/12 03:10:12 sobomax Exp $
 *
 */

#ifndef _RTPP_LOG_H_
#define _RTPP_LOG_H_

#include <errno.h>
#include <stdio.h>

#include "rtpp_defines.h"

#define	RTPP_LOG_DBUG	0
#define	RTPP_LOG_INFO	1
#define	RTPP_LOG_WARN	2
#define	RTPP_LOG_ERR	3
#define	RTPP_LOG_CRIT	4

#define	rtpp_log_t	int

#define	rtpp_log_open(app, call_id, flag) (0)
#define	rtpp_log_write(level, handle, format, args...)			\
	do {								\
		if (level >= LOG_LEVEL) {				\
			fprintf(stderr, format, ## args);		\
			fprintf(stderr, "\n");				\
		}							\
	} while (0);
#define	rtpp_log_ewrite(level, handle, format, args...)			\
	do {								\
		if (level >= LOG_LEVEL) {				\
			fprintf(stderr, format, ## args);		\
			fprintf(stderr, ": %s\n", strerror(errno));	\
		}							\
	} while (0);
#define	rtpp_log_close(handle) while (0) {}

#endif
