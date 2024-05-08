/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
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

#ifndef _RTPP_UTIL_H_
#define _RTPP_UTIL_H_

#define	GET_RTP(sp)	(((sp)->rtp != NULL) ? (sp)->rtp : (sp))
#define	NOT(x)		(((x) == 0) ? 1 : 0)

struct cfg;

double my_trunc(double x);
double my_round(double x);


/* Function prototypes */
double getdtime(void);
double ts2dtime(uint32_t, uint32_t);
void dtime2ts(double, uint32_t *, uint32_t *);
void seedrandom(void);
int set_rlimits(struct cfg *);
int drop_privileges(struct cfg *);
void init_port_table(struct cfg *);
char *rtpp_strsep(char **, const char *);
int rtpp_daemon(int, int);
int url_unquote(uint8_t *, int);
int pthread_mutex_islocked(pthread_mutex_t *);
int rtpp_get_sched_hz(void);
long long rtpp_rlim_max(struct cfg *cf);

/* Some handy/compat macros */
#if !defined(INFTIM)
#define	INFTIM		(-1)
#endif

#if !defined(ACCESSPERMS)
#define	ACCESSPERMS	(S_IRWXU|S_IRWXG|S_IRWXO)
#endif
#if !defined(DEFFILEMODE)
#define	DEFFILEMODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

#if !defined(HAVE_ERR_H)
#define err(exitcode, format, args...) \
  errx(exitcode, format ": %s", ## args, strerror(errno))
#define errx(exitcode, format, args...) \
  { warnx(format, ## args); exit(exitcode); }
#define warn(format, args...) \
  warnx(format ": %s", ## args, strerror(errno))
#define warnx(format, args...) \
  fprintf(stderr, format "\n", ## args)
#endif

#endif
