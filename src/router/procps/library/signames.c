/*
 * signames.c - Translate signal masks
 *
 * Copyright © 2023-2024 Craig Small <csmall@dropbear.xyz>
 * Copyright © 2020      Luis Chamberlain <mcgrof@kernel.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "signals.h"
#include "misc.h"
#include "procps-private.h"

/* For libraries like musl */
#ifndef __SIGRTMIN
#define __SIGRTMIN SIGRTMIN
#endif
#ifndef __SIGRTMAX
#define __SIGRTMAX SIGRTMAX
#endif

#define SIGNAME_MAX 256
/*
 * The actual list of unsupported signals varies by operating system. This
 * program is Linux specific as it processes /proc/ for signal information and
 * there is no generic way to extract each process signal information for each
 * OS. This program also relies on Linux glibc defines to figure out which
 * signals are reserved for use by libc and then which ones are real time
 * specific.
 */

#if !HAVE_SIGABBREV_NP
static inline const char *sigabbrev_np(int sig)
{
#define SIGABBREV(a_) case SIG##a_: return #a_
    switch(sig) {
        SIGABBREV(HUP);
        SIGABBREV(INT);
        SIGABBREV(QUIT);
        SIGABBREV(ILL);
        SIGABBREV(TRAP);
        SIGABBREV(ABRT);
        SIGABBREV(BUS);
        SIGABBREV(FPE);
        SIGABBREV(KILL);
        SIGABBREV(USR1);
        SIGABBREV(SEGV);
        SIGABBREV(USR2);
        SIGABBREV(PIPE);
        SIGABBREV(ALRM);
        SIGABBREV(TERM);
#ifdef SIGSTKFLT
        SIGABBREV(STKFLT);
#endif
        SIGABBREV(CHLD);
        SIGABBREV(CONT);
        SIGABBREV(STOP);
        SIGABBREV(TSTP);
        SIGABBREV(TTIN);
        SIGABBREV(TTOU);
        SIGABBREV(URG);
        SIGABBREV(XCPU);
        SIGABBREV(XFSZ);
        SIGABBREV(VTALRM);
        SIGABBREV(PROF);
        SIGABBREV(WINCH);
        SIGABBREV(POLL);
#ifdef SIGPWR               // absent in kFreeBSD (Debian #832148)
        SIGABBREV(PWR);
#endif
        SIGABBREV(SYS);
#ifdef SIGLOST              // Hurd (gitlab#93)
        SIGABBREV(LOST);
#endif
#if defined __sun || defined __SUN || defined __solaris__ || defined __SOLARIS__
        case 0: return "EXIT";
#endif
#if defined _AIX || defined __AIX__ || defined __aix__
        case 0: return "NULL";
#endif
    };
#undef SIGABBREV
    return NULL;
}

#endif /* HAVE_SIGABBREV_NP */
/*
 * As per glibc:
 *
 * A system that defines real time signals overrides __SIGRTMAX to be something
 * other than __SIGRTMIN. This also means we can count on __SIGRTMIN being the
 * first real time signal, meaning what Linux programs it for your architecture
 * in the kernel. SIGRTMIN then will be the application specific first real
 * time signal, that is, on top of libc. The values in between
 *
 * 	__SIGRTMIN .. SIGRTMIN
 *
 * are used by * libc, typically for helping threading implementation.
 */
static const char *sigstat_strsignal_abbrev(int sig, char *abbrev, size_t len)
{
	memset(abbrev, '\0', len);

	if (sig == 0 || sig >= NSIG) {
		snprintf(abbrev, len, "BOGUS_%02d", sig - _NSIG);
		return abbrev;
	}

	/*
	 * The standard lower signals we can count on this being the kernel
	 * specific SIGRTMIN.
	 */
	if (sig < __SIGRTMIN) {
                const char *signame = NULL;
                signame = sigabbrev_np(sig);

                if (signame != NULL && signame[0] != '\0') {
                    strncpy(abbrev, signame, len);
                    return abbrev;
                }
	}

	/* This means your system should *not* have realtime signals */
	if (__SIGRTMAX == __SIGRTMIN) {
		snprintf(abbrev, len, "INVALID_%02d", sig);
		return abbrev;
	}

	/*
	 * If we're dealing with a libc real time signal start counting
	 * after libc's version of SIGRTMIN
	 */
	if (sig >= SIGRTMIN) {
		if (sig == SIGRTMIN)
			snprintf(abbrev, len, "RTMIN");
		else if (sig == SIGRTMAX)
			snprintf(abbrev, len, "RTMAX");
		else
			snprintf(abbrev, len, "RTMIN+%02d", sig - SIGRTMIN);
	} else
		snprintf(abbrev, len, "LIBC+%02d", sig - __SIGRTMIN);

	return abbrev;
}

/*
 * For instance SIGTERM is 15, but its actual mask value is
 * 1 << (15-1) = 0x4000
 */
static uint64_t mask_sig_val_num(int signum)
{
	return ((uint64_t) 1 << (signum -1));
}

PROCPS_EXPORT int procps_sigmask_names(
        char *str,
        size_t size,
        const char *sigmask)
{
	unsigned int i;
	char abbrev[SIGNAME_MAX];
	unsigned int n = 0;
	char *c = str;
	uint64_t mask, mask_in;
	uint64_t test_val = 0;

        if (str == NULL || sigmask == NULL || size == 0)
            return -EINVAL;

        if (1 != sscanf(sigmask, "%" PRIx64, &mask_in))
            return -EINVAL;
        mask = mask_in;

	for (i=0; i < NSIG; i++) {
		test_val = mask_sig_val_num(i);
		if (test_val & mask) {
                        n = strlen(sigstat_strsignal_abbrev(i, abbrev, SIGNAME_MAX));
                        if (n+1 >= size) { // +1 for the '+'
                            strcpy(c, "+");
                            size -= 1;
                            c += 1;
                            break;
                        } else {
			    n = snprintf(c, size, (c==str)?"%s":",%s",
				     sigstat_strsignal_abbrev(i, abbrev,
					   		      SIGNAME_MAX));
			    size -= n;
			    c+=n;
                        }
		}
	}
        if (c == str) {
            n = snprintf(c, size, "%c", '-');
            size -= n;
            c += n;
        }
	return (int) (c-str);
}
