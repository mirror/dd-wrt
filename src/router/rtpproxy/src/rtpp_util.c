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

#include <sys/time.h>
#include <sys/types.h>
//#include <sys/sysctl.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_util.h"

#if defined(CLOCK_UPTIME_PRECISE)
#define RTPP_CLOCK CLOCK_UPTIME_PRECISE
#else
# if defined(CLOCK_MONOTONIC_RAW)
#define RTPP_CLOCK CLOCK_MONOTONIC_RAW
# else
#define RTPP_CLOCK CLOCK_MONOTONIC
#endif
#endif

static double timespec2dtime(time_t, long);

double
getdtime(void)
{
    struct timespec tp;

    if (clock_gettime(RTPP_CLOCK, &tp) == -1)
        return (-1);

    return timespec2dtime(tp.tv_sec, tp.tv_nsec);
}

static double
timespec2dtime(time_t tv_sec, long tv_nsec)
{

    return (double)tv_sec + (double)tv_nsec / 1000000000.0;
}

double
ts2dtime(uint32_t ts_sec, uint32_t ts_usec)
{

    return ts_sec + ((double)ts_usec) / 1000000.0;
}

void
dtime2ts(double dtime, uint32_t *ts_sec, uint32_t *ts_usec)
{

    *ts_sec = my_trunc(dtime);
    *ts_usec = my_round(1000000.0 * (dtime - ((double)*ts_sec)));
}

void
seedrandom(void)
{
    int fd;
    unsigned long junk;
    struct timeval tv;

    fd = open("/dev/urandom", O_RDONLY, 0);
    if (fd >= 0) {
	read(fd, &junk, sizeof(junk));
	close(fd);
    } else {
        junk = 0;
    }

    gettimeofday(&tv, NULL);
    srandom((getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec ^ junk);
}

int
set_rlimits(struct cfg *cf)
{
    struct rlimit rlp;

    if (getrlimit(RLIMIT_CORE, &rlp) < 0) {
        rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "getrlimit(RLIMIT_CORE)");
        return (-1);
    }
    rlp.rlim_cur = RLIM_INFINITY;
    rlp.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &rlp) < 0) {
        rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "setrlimit(RLIMIT_CORE)");
        return (-1);
    }
    return (0);
}

int
drop_privileges(struct cfg *cf)
{

    if (cf->stable->run_gname != NULL) {
	if (setgid(cf->stable->run_gid) != 0) {
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "can't set current group ID: %d", cf->stable->run_gid);
	    return -1;
	}
    }
    if (cf->stable->run_uname == NULL)
	return 0;
    if (setuid(cf->stable->run_uid) != 0) {
	rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "can't set current user ID: %d", cf->stable->run_uid);
	return -1;
    }
    return 0;
}

void
init_port_table(struct cfg *cf)
{
    int i, j;
    uint16_t portnum;

    /* Generate linear table */
    cf->stable->port_table_len = ((cf->stable->port_max - cf->stable->port_min) / 2) + 1;
    portnum = cf->stable->port_min;
    for (i = 0; i < cf->stable->port_table_len; i += 1) {
	cf->stable->port_table[i] = portnum;
	portnum += 2;
    }
    if (cf->stable->seq_ports == 0) {
        /* Shuffle elements ramdomly */
        for (i = 0; i < cf->stable->port_table_len; i += 1) {
	    j = random() % cf->stable->port_table_len;
	    portnum = cf->stable->port_table[i];
	    cf->stable->port_table[i] = cf->stable->port_table[j];
	    cf->stable->port_table[j] = portnum;
        }
    }
    /* Set the last used element to be the last element */
    cf->port_table_idx = cf->stable->port_table_len - 1;
}

/*
 * Portable strsep(3) implementation, borrowed from FreeBSD. For license
 * and other information see:
 *
 * $FreeBSD: src/lib/libc/string/strsep.c,v 1.6 2007/01/09 00:28:12 imp Exp $
 */
char *
rtpp_strsep(char **stringp, const char *delim)
{
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
	return (NULL);
    for (tok = s;;) {
	c = *s++;
	spanp = delim;
	do {
	    if ((sc = *spanp++) == c) {
		if (c == 0)
		    s = NULL;
		else
		    s[-1] = 0;
		*stringp = s;
		return (tok);
	    }
	} while (sc != 0);
    }
    /* NOTREACHED */
}

/*
 * Portable daemon(3) implementation, borrowed from FreeBSD. For license
 * and other information see:
 *
 * $FreeBSD: src/lib/libc/gen/daemon.c,v 1.8 2007/01/09 00:27:53 imp Exp $
 */
int
rtpp_daemon(int nochdir, int noclose)
{
    struct sigaction osa, sa;
    int fd;
    pid_t newgrp;
    int oerrno;
    int osa_ok;

    /* A SIGHUP may be thrown when the parent exits below. */
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    osa_ok = sigaction(SIGHUP, &sa, &osa);

    switch (fork()) {
    case -1:
        return (-1);
    case 0:
        break;
    default:
        _exit(0);
    }

    newgrp = setsid();
    oerrno = errno;
    if (osa_ok != -1)
        sigaction(SIGHUP, &osa, NULL);

    if (newgrp == -1) {
        errno = oerrno;
        return (-1);
    }

    if (!nochdir)
        (void)chdir("/");

    if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        (void)dup2(fd, STDIN_FILENO);
        (void)dup2(fd, STDOUT_FILENO);
        (void)dup2(fd, STDERR_FILENO);
        if (fd > 2)
            (void)close(fd);
    }
    return (0);
}

static int8_t hex2char[128] = {
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,   1,   2,   3,   4,   5,   6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int
url_unquote(uint8_t *buf, int len)
{
    int outlen;
    uint8_t *cp;

    outlen = len;
    while (len > 0) {
        cp = memchr(buf, '%', len);
        if (cp == NULL)
            return (outlen);
        if (cp - buf + 2 > len)
            return (-1);
        if (cp[1] > 127 || cp[2] > 127 ||
          hex2char[cp[1]] == -1 || hex2char[cp[2]] == -1)
            return (-1);
        cp[0] = (hex2char[cp[1]] << 4) | hex2char[cp[2]];
        len -= cp - buf + 3;
        if (len > 0)
            memmove(cp + 1, cp + 3, len);
        buf = cp + 1;
        outlen -= 2;
    }
    return (outlen);
}

int
pthread_mutex_islocked(pthread_mutex_t *mutex)
{
    int rval;

    rval = pthread_mutex_trylock(mutex);
    if (rval != 0) {
        if (rval == EBUSY)
            return (1);
        return (-1);
    }
    pthread_mutex_unlock(mutex);
    return (0);
}

#if defined(_SC_CLK_TCK) && !defined(__FreeBSD__)
#if defined(LINUX_XXX)
static int
rtpp_get_sched_hz_linux(void)
{
    int fd, rlen;
    char buf[16], *cp;
    int64_t n;

    fd = open("/proc/sys/kernel/sched_min_granularity_ns", O_RDONLY, 0);
    if (fd == -1) {
        return (-1);
    }
    rlen = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (rlen <= 0) {
        return (-1);
    }
    buf[rlen] = '\0'; 
    n = strtol(buf, &cp, 10);
    if (cp == buf) {
        return (-1);
    }
    return ((int64_t)1000000000 / n);
}
#endif

int
rtpp_get_sched_hz(void)
{
    int sched_hz;

#if defined (LINUX_XXX)
    sched_hz = rtpp_get_sched_hz_linux();
    if (sched_hz > 0) {
        return (sched_hz);
    }
#endif
    sched_hz = sysconf(_SC_CLK_TCK);
    return (sched_hz > 0 ? sched_hz : 100);
}
#else
int
rtpp_get_sched_hz(void)
{
    int sched_hz;
    size_t len;

    len = sizeof(sched_hz);
    if (sysctlbyname("kern.hz", &sched_hz, &len, NULL, 0) == -1 || sched_hz <= 0)
        return 1000;
    return (sched_hz);
}
#endif
