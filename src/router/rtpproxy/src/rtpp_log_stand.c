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
 */

#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#ifdef RTPP_LOG_ADVANCED
#include "rtpp_syslog_async.h"
#endif

#ifdef RTPP_LOG_ADVANCED
static int syslog_async_opened = 0;
#endif

struct rtpp_log_inst {
    char *call_id;
    int level;
    const char *format;
    const char *eformat;
};

struct rtpp_log_inst *
_rtpp_log_open(struct rtpp_cfg_stable *cf, const char *app, const char *call_id)
{
    struct rtpp_log_inst *rli;
#ifdef RTPP_LOG_ADVANCED
    int facility;

    facility = cf->log_facility;
    if (facility == -1)
	facility = LOG_DAEMON;

    if (cf->nodaemon == 0 && syslog_async_opened == 0) {
	if (syslog_async_init(app, facility) == 0)
	    syslog_async_opened = 1;
    }
#endif
    rli = malloc(sizeof(struct rtpp_log_inst));
    if (rli == NULL) {
        return (NULL);
    }
    memset(rli, '\0', sizeof(struct rtpp_log_inst));
    if (call_id != NULL) {
        rli->call_id = strdup(call_id);
    }
    if (cf->log_level == -1) {
	rli->level = (cf->nodaemon != 0) ? RTPP_LOG_DBUG : RTPP_LOG_WARN;
    } else {
        rli->level = cf->log_level;
    }
    if (cf->nodaemon != 0) {
        rli->format = "%s:%s:%s: %s\n";
        rli->eformat = "%s:%s:%s: %s: %s\n";
    } else {
        rli->format = "%s:%s:%s: %s";
        rli->eformat = "%s:%s:%s: %s: %s";
    }
    return (rli);
}

void
_rtpp_log_close(struct rtpp_log_inst *rli)
{
    if (rli->call_id != NULL) {
        free(rli->call_id);
    }
    free(rli);
    return;
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

static int
check_level(struct rtpp_log_inst *rli, int level)
{

    return (level <= rli->level);
}

void
rtpp_log_setlevel(struct rtpp_log_inst *rli, int level)
{

    rli->level = level;
}

void
_rtpp_log_write(struct rtpp_log_inst *rli, int level, const char *function, const char *format, ...)
{
    va_list ap;
    char rtpp_log_buff[2048];
    const char *call_id;

    if (check_level(rli, level) == 0)
	return;

    if (rli->call_id != NULL) {
        call_id = rli->call_id;
    } else {
        call_id = "GLOBAL";
    }

    va_start(ap, format);

    snprintf(rtpp_log_buff, sizeof(rtpp_log_buff), rli->format, strlvl(level),
      function, call_id, format);
#ifdef RTPP_LOG_ADVANCED
    if (syslog_async_opened != 0) {
	vsyslog_async(level, rtpp_log_buff, ap);
        va_end(ap);
        return;
    }
#endif
    vfprintf(stderr, rtpp_log_buff, ap);
    va_end(ap);
}

void
_rtpp_log_ewrite(struct rtpp_log_inst *rli, int level, const char *function, const char *format, ...)
{
    va_list ap;
    char rtpp_log_buff[2048];
    const char *call_id;
    
    if (check_level(rli, level) == 0)
	return;

    if (rli->call_id != NULL) {
        call_id = rli->call_id;
    } else {
        call_id = "GENERAL";
    }

    va_start(ap, format);

    snprintf(rtpp_log_buff, sizeof(rtpp_log_buff), rli->eformat, strlvl(level),
      function, call_id, format, strerror(errno));
#ifdef RTPP_LOG_ADVANCED
    if (syslog_async_opened != 0) {
	vsyslog_async(level, rtpp_log_buff, ap);
	va_end(ap);
	return;
    }
#endif
    vfprintf(stderr, rtpp_log_buff, ap);
    va_end(ap);
}

static struct {
    const char *str_fac;
    int int_fac;
} str2fac[] = {
    {"LOG_AUTH",     LOG_AUTH},
    {"LOG_CRON",     LOG_CRON},
    {"LOG_DAEMON",   LOG_DAEMON},
    {"LOG_KERN",     LOG_KERN},
    {"LOG_LOCAL0",   LOG_LOCAL0},
    {"LOG_LOCAL1",   LOG_LOCAL1},
    {"LOG_LOCAL2",   LOG_LOCAL2},
    {"LOG_LOCAL3",   LOG_LOCAL3},
    {"LOG_LOCAL4",   LOG_LOCAL4},
    {"LOG_LOCAL5",   LOG_LOCAL5},
    {"LOG_LOCAL6",   LOG_LOCAL6},
    {"LOG_LOCAL7",   LOG_LOCAL7},
    {"LOG_LPR",      LOG_LPR},
    {"LOG_MAIL",     LOG_MAIL},
    {"LOG_NEWS",     LOG_NEWS},
    {"LOG_USER",     LOG_USER},
    {"LOG_UUCP",     LOG_UUCP},
#if !defined(__solaris__) && !defined(__sun) && !defined(__svr4__)
    {"LOG_AUTHPRIV", LOG_AUTHPRIV},
    {"LOG_FTP",      LOG_FTP},
    {"LOG_SYSLOG",   LOG_SYSLOG},
#endif
    {NULL,           0}
};

int
rtpp_log_str2fac(const char *s)
{
    int i;

    for (i=0; str2fac[i].str_fac != NULL; i++) {
        if (strcasecmp(s, str2fac[i].str_fac) == 0 || \
	  strcasecmp(s, str2fac[i].str_fac + 4) == 0)
            return str2fac[i].int_fac;
    }
    return -1;
}
