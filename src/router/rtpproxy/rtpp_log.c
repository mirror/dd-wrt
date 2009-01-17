/*
 * Copyright (c) 2008 Sippy Software, Inc., http://www.sippysoft.com
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
 * $Id: rtpp_log.c,v 1.1 2008/09/17 01:11:20 sobomax Exp $
 *
 */

#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtpp_defines.h"
#include "rtpp_log.h"

static int open_count = 0;

struct cfg *
_rtpp_log_open(struct cfg *cf, const char *app)
{

    if (open_count == 0)
	openlog(app, LOG_PID | LOG_CONS, LOG_DAEMON);
    open_count++;
    return cf;
}

void
_rtpp_log_close(void)
{
     open_count--;
     if (open_count == 0)
	closelog();
}

static const char *
strlvl(int level)
{

    switch(level) {
    case RTPP_LOG_DBUG:
	return "DBUG";

    case RTPP_LOG_INFO:
	return "INFO";

    case RTPP_LOG_WARN:
	return "WARN";

    case RTPP_LOG_ERR:
	return "ERR";

    case RTPP_LOG_CRIT:
	return "CRIT";

    default:
	break;
    }

    abort();

    return NULL;
}

int
rtpp_log_str2lvl(const char *strl)
{

    if (strcasecmp(strl, "DBUG") == 0)
	return RTPP_LOG_DBUG;

    if (strcasecmp(strl, "INFO") == 0)
	return RTPP_LOG_INFO;

    if (strcasecmp(strl, "WARN") == 0)
	return RTPP_LOG_WARN;

    if (strcasecmp(strl, "ERR") == 0)
	return RTPP_LOG_ERR;

    if (strcasecmp(strl, "CRIT") == 0)
	return RTPP_LOG_CRIT;

    return -1;
}

void
_rtpp_log_write(struct cfg *cf, int level, const char *function, const char *format, ...)
{
    va_list ap;
    char rtpp_log_buff[2048];
    char *fmt;

    va_start(ap, format);

    if (cf->nodaemon != 0) {
	fmt = "%s:%s: %s\n";
    } else {
	fmt = "%s:%s: %s";
    }

    snprintf(rtpp_log_buff, sizeof(rtpp_log_buff), fmt, strlvl(level),
      function, format);
    if (cf->nodaemon != 0) {
	vfprintf(stderr, rtpp_log_buff, ap);
    } else {
	vsyslog(level, rtpp_log_buff, ap);
    }

    va_end(ap);
}

void
_rtpp_log_ewrite(struct cfg *cf, int level, const char *function, const char *format, ...)
{
    va_list ap;
    char rtpp_log_buff[2048];
    char *fmt;

    va_start(ap, format);

    if (cf->nodaemon != 0) {
	fmt = "%s:%s: %s: %s\n";
    } else {
	fmt = "%s:%s: %s: %s";
    }

    snprintf(rtpp_log_buff, sizeof(rtpp_log_buff), fmt, strlvl(level),
      function, format, strerror(errno));

    if (cf->nodaemon != 0) {
	vfprintf(stderr, rtpp_log_buff, ap);
    } else {
	vsyslog(level, rtpp_log_buff, ap);
    }

    va_end(ap);
}
