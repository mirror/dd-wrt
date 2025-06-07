/*
 * signals.c - signal name, and number, conversions
 *
 * Copyright 2023 by Roman Žilka <roman.zilka@gmail.com>
 * Copyright 1998-2003 by Albert Cahalan
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

#include <ctype.h>
#include <signal.h>
#include <strings.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "signals.h"
#include "c.h"
#include "strutils.h"
#include "xalloc.h"

// The SUS and C define a minimal list of available signals and guarantee that
// their numerical values be distinct. The values are positive, of type int.
// 0 is reserved for the non-communicable "null" signal. The signal range
// between SIGRTMIN and SIGRTMAX are the real-time signals and are only covered
// by the SUS.
//
// That being said, real-world systems may divert from the standards. Moreover,
// they usually employ a larger set of signals.
//
// Following from all of that,
// * signals aren't assigned predictable numerical values;
// * the values may not be stable over time;
// * the values may not form an uninterrupted sequence;
// * multiple signals may share numerical value.
//
// signal(7) details some portability issues.



// UNUSED and SYS used to be synonyms. These days the system ought to provide
// SYS and UNUSED is of low importance.
#ifndef SIGSYS
#warning "<signal.h> doesn't provide SIGSYS as required by SUS. Assuming 31."
#define SIGSYS 31
#endif

// The defaulting of SIGPWR to 29 on Linux has been removed. It's not 29 on
// current GNU/Linux on x86 and ARM. If SPARC is at risk of needing an explicit
// define, include it with an appropriate SPARC ifdef.

#ifndef SIGRTMIN
// glibc uses the first two RT signals internally for pthreads
#if defined __GLIBC__ && (defined _REENTRANT || defined _THREAD_SAFE || _POSIX_C_SOURCE >= 199506L)
#warning "<signal.h> doesn't provide SIGRTMIN as required by SUS. Assuming 34."
#define SIGRTMIN 34
#else
#warning "<signal.h> doesn't provide SIGRTMIN as required by SUS. Assuming 32."
#define SIGRTMIN 32
#endif
#endif

#ifndef SIGRTMAX
#warning "<signal.h> doesn't provide SIGRTMAX as required by SUS. Assuming 64."
#define SIGRTMAX 64
#endif



typedef struct mapstruct {
  const char *name;
  int num;
} mapstruct;

// Rank the more common names higher up the list to prioritize them in number-
// -to-name lookups.
static const mapstruct sigtable[] = {
  {"HUP",    SIGHUP},
  {"INT",    SIGINT},
  {"QUIT",   SIGQUIT},
  {"ILL",    SIGILL},
  {"TRAP",   SIGTRAP},
  {"ABRT",   SIGABRT},
#ifdef SIGIOT
  {"IOT",    SIGIOT},     // possibly = ABRT
#endif
  {"BUS",    SIGBUS},
#ifdef SIGEMT
  {"EMT",    SIGEMT},
#endif
  {"FPE",    SIGFPE},
  {"KILL",   SIGKILL},
  {"USR1",   SIGUSR1},
  {"SEGV",   SIGSEGV},
  {"USR2",   SIGUSR2},
  {"PIPE",   SIGPIPE},
  {"ALRM",   SIGALRM},
  {"TERM",   SIGTERM},
#ifdef SIGSTKFLT
  {"STKFLT", SIGSTKFLT},
#endif
  {"CHLD",   SIGCHLD},
#ifdef SIGCLD
  {"CLD",    SIGCLD},     // possibly = CHLD
#endif
  {"CONT",   SIGCONT},
  {"STOP",   SIGSTOP},
  {"TSTP",   SIGTSTP},
  {"TTIN",   SIGTTIN},
  {"TTOU",   SIGTTOU},
  {"URG",    SIGURG},
  {"XCPU",   SIGXCPU},
  {"XFSZ",   SIGXFSZ},
  {"VTALRM", SIGVTALRM},
  {"PROF",   SIGPROF},
  {"WINCH",  SIGWINCH},
#ifdef SIGPOLL
  {"POLL",   SIGPOLL},    // in SUSv3, prioritize over IO
#endif
#ifdef SIGIO
  {"IO",     SIGIO},      // possibly = POLL
#endif
#ifdef SIGPWR             // absent in kFreeBSD (Debian #832148)
  {"PWR",    SIGPWR},
#endif
#ifdef SIGINFO
  {"INFO",   SIGINFO},    // possibly = PWR
#endif
#ifdef __GNU__            // for Hurd (gitlab#93)
  {"LOST",   SIGLOST},
#endif
  {"SYS",    SIGSYS},
#ifdef SIGUNUSED
  {"UNUSED", SIGUNUSED},  // possibly = SYS, absent in modern glibc
#endif
#if defined __sun || defined __SUN || defined __solaris__ || defined __SOLARIS__
  {"EXIT", 0},
#endif
#if defined _AIX || defined __AIX__ || defined __aix__
  {"NULL", 0},            // not a terminator, that's how it's called
#endif
};

#define sigtable_nritems (sizeof(sigtable) / sizeof(*sigtable))

#define XJOIN(a, b) JOIN(a, b)
#define JOIN(a, b) a##b
#define STATIC_ASSERT(x) typedef int XJOIN(static_assert_on_line_,__LINE__)[(x) ? 1 : -1]

/* sanity check */
#if defined(__linux__)
STATIC_ASSERT(sigtable_nritems >= 31);
#elif defined(__FreeBSD_kernel__) || defined(__FreeBSD__)
STATIC_ASSERT(sigtable_nritems >= 30);
#elif defined(__GNU__)
STATIC_ASSERT(sigtable_nritems >= 31);
#elif defined(__CYGWIN__)
STATIC_ASSERT(sigtable_nritems >= 31);
#else
#warning Unknown operating system; assuming sigtable[] is correct
#endif



// not thread-safe, used to return strings
static char buf[24];



static int mapstruct_cmp_num(const void *i1, const void *i2) {
    if (((const mapstruct *)i1)->num < ((const mapstruct *)i2)->num) return -1;
    if (((const mapstruct *)i1)->num > ((const mapstruct *)i2)->num) return 1;
    return 0;
}



// -1 on failure
// also converts a string with a number
int signal_name_to_number(const char *name)
{
    long n;
    char *endp;

    // string with number?
    errno = 0;
    n = strtol(name, &endp, 10);
    if (endp != name) {
        if (*endp || errno || n < 0 || n > INT_MAX)
            return -1;
        return n;
    }

    if (! strncasecmp(name, "SIG", 3))
        name += 3;

    /* search the table, includes known null names */
    const unsigned char c1 = toupper((unsigned char)name[0]);
    for (uf8 i=0; i<sigtable_nritems; ++i) {
        if ( sigtable[i].name[0] == c1 &&
             ! strcasecmp(name, sigtable[i].name)
           ) {
            return sigtable[i].num;
        }
    }

    // RTMIN, RTMAX, RTMIN+n, RTMAX-n, all inside [RTMIN, RTMAX]
    bool add;
    if (! strncasecmp(name, "RTMIN", 5)) {
        switch (name[5]) {
            case '\0': return SIGRTMIN;
            case '+': add = true; break;
            default: return -1;
        }
    }
    else if (! strncasecmp(name, "RTMAX", 5)) {
        switch (name[5]) {
            case '\0': return SIGRTMAX;
            case '-': add = false; break;
            default: return -1;
        }
    }
    else return -1;
    name += 6;
    if (*name < '0' || *name > '9')
        return -1;
    errno = 0;
    n = strtol(name, &endp, 10);
    if (*endp || errno || n > SIGRTMAX-SIGRTMIN)
        return -1;
    return add ? SIGRTMIN+n : SIGRTMAX-n;
}



// always returns something printable
// not found => returns "-" (this is used elsewhere)
const char *signal_number_to_name(int signo)
{
    //signo &= 0x7f;
    for (uf8 n=0; n<sigtable_nritems; ++n) {
        if (sigtable[n].num == signo)
            return sigtable[n].name;
    }

    if (signo == 0)
        return "0";  // in case there's no named null signal in sigtable
    if (signo == SIGRTMIN)
        return "RTMIN";
    if (signo > SIGRTMIN && signo <= SIGRTMAX) {
        snprintf(buf, sizeof(buf), "RTMIN+%d", signo-SIGRTMIN);
        return buf;
    }

    return "-";
}



int skill_sig_option(int *argc, char **argv)
{
    int i;
    int signo = -1;
    for (i = 1; i < *argc; i++) {
        if (argv[i][0] == '-') {
            signo = signal_name_to_number(argv[i] + 1);
            if (-1 < signo) {
                memmove(argv + i, argv + i + 1,
                        sizeof(char *) * (*argc - i));
                (*argc)--;
                return signo;
            }
        }
    }
    return signo;
}



/* Takes a string, and converts it to a signal name or a number string depending
 * on which way around conversion is queried. Non-existing signals return NULL.
 */
const char *strtosig(const char *s)
{
    char *endp;
    long n;

    // number -> name?
    errno = 0;
    n = strtol(s, &endp, 10);
    if (endp != s) {
        if (*endp || errno || n < 0 || n > INT_MAX)
            return NULL;
        s = signal_number_to_name(n);
        if (s[0] == '-' && s[1] == '\0')
            return NULL;
        return s;
    }

    // name -> number?
    // do this second, because signal_name_to_number() accepts strings with
    // numbers
    n = signal_name_to_number(s);
    if (n >= 0) {
        snprintf(buf, sizeof(buf), "%ld", n);
        return buf;
    }
    return NULL;
}



void unix_print_signals(void)
{
    mapstruct *sigtable2 = xmalloc(sizeof(sigtable));
    memcpy(sigtable2, sigtable, sizeof(sigtable));
    stablesort(sigtable2, sigtable_nritems, sizeof(*sigtable), mapstruct_cmp_num);

    // omit null signal(s)
    int lastseen = 0, pos = 0;
    for (uf8 i=0; i<sigtable_nritems; ++i) {
        if (sigtable2[i].num != lastseen) {
            if (lastseen) putchar((pos>73) ? (pos=0,'\n') : (pos++,' '));
            pos += printf("%s", sigtable2[i].name);
            lastseen = sigtable2[i].num;
        }
    }

    free(sigtable2);
    if (lastseen) putchar('\n');
}



void pretty_print_signals(void)
{
    mapstruct *sigtable2 = xmalloc(sizeof(sigtable));
    memcpy(sigtable2, sigtable, sizeof(sigtable));
    stablesort(sigtable2, sigtable_nritems, sizeof(*sigtable), mapstruct_cmp_num);

    // omit null signal(s)
    int lastseen = 0, n;
    uf8 i, printed = 0;
    for (i=0; i<sigtable_nritems; ++i) {
        if (sigtable2[i].num != lastseen) {
            n = printf("%2d %s", sigtable2[i].num, sigtable2[i].name);
            ++printed;
            if (printed % 7)
                fputs("           \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" + n, stdout);
            else putchar('\n');
            lastseen = sigtable2[i].num;
        }
    }

    free(sigtable2);
    if (lastseen && printed % 7) putchar('\n');
}



/*
int main() {
    struct tuple {
        const char *name;
        int nr;
    };

    const struct tuple c99_posix2001[] = {
        {"ABRT", SIGABRT},
        {"ALRM", SIGALRM},
        {"BUS", SIGBUS},
        {"CHLD", SIGCHLD},
        {"CONT", SIGCONT},
        {"FPE", SIGFPE},
        {"HUP", SIGHUP},
        {"ILL", SIGILL},
        {"INT", SIGINT},
        {"KILL", SIGKILL},
        {"PIPE", SIGPIPE},
        {"QUIT", SIGQUIT},
        {"SEGV", SIGSEGV},
        {"STOP", SIGSTOP},
        {"TERM", SIGTERM},
        {"TSTP", SIGTSTP},
        {"TTIN", SIGTTIN},
        {"TTOU", SIGTTOU},
        {"USR1", SIGUSR1},
        {"USR2", SIGUSR2},
        {"PROF", SIGPROF},
        {"SYS", SIGSYS},
        {"TRAP", SIGTRAP},
        {"URG", SIGURG},
        {"XCPU", SIGXCPU},
        {"XFSZ", SIGXFSZ},
    };
    const struct tuple set_str2int[] = {
        {"SIGABRT", SIGABRT},
        {"sigXFSZ", SIGXFSZ},
        {" SIGUSR1", -1},
        {"SIGUSR1 ", -1},
        {"BUS+0", -1},
        {"RTMIN+0", SIGRTMIN},
        {"RTMIN+ 0", -1},
        {"RTMAX-", -1},
        {"RTMAX-999", -1},
        {"SIGRTMIN++1", -1}
    };
    const struct tuple set_int2str[] = {
        {"-", -2},
        {"-", SIGRTMAX+1},
        {"RTMIN+1", SIGRTMIN+1},
        {"0", 0}
    };
    uf8 i;
    char buf[16];

    unix_print_signals();
    putchar('\n');
    pretty_print_signals();
    putchar('\n');

    for (i=0; i<sizeof(c99_posix2001)/sizeof(*c99_posix2001); ++i) {
        if (c99_posix2001[i].nr != signal_name_to_number(c99_posix2001[i].name))
            return 1;
    }
    for (i=0; i<sizeof(c99_posix2001)/sizeof(*c99_posix2001); ++i) {
        if (strcmp(c99_posix2001[i].name, signal_number_to_name(c99_posix2001[i].nr)))
            return 1;
    }

    for (i=0; i<sizeof(c99_posix2001)/sizeof(*c99_posix2001); ++i) {
        sprintf(buf, "%d", c99_posix2001[i].nr);
        if (strcmp(buf, strtosig(c99_posix2001[i].name)))
            return 1;
    }
    for (i=0; i<sizeof(c99_posix2001)/sizeof(*c99_posix2001); ++i) {
        sprintf(buf, "%d", c99_posix2001[i].nr);
        if (strcmp(c99_posix2001[i].name, strtosig(buf)))
            return 1;
    }

    for (i=0; i<sizeof(set_str2int)/sizeof(*set_str2int); ++i) {
        if (set_str2int[i].nr != signal_name_to_number(set_str2int[i].name))
            return 1;
    }
    for (i=0; i<sizeof(set_int2str)/sizeof(*set_int2str); ++i) {
        if (strcmp(set_int2str[i].name, signal_number_to_name(set_int2str[i].nr)))
            return 1;
    }

    puts("ALL OK");
    return 0;
}
*/
