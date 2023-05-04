/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef __UTIL_H__
#define __UTIL_H__

#define TIMEBUF_SIZE 26

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef WIN32
# include <sys/time.h>
# include <sys/types.h>
# ifdef LINUX
#  include <sys/syscall.h>
# endif
#endif
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <string.h>

#include "sf_types.h"
#include "sflsq.h"
#include "sfutil/sf_ipvar.h"
#include "ipv6_port.h"
#include "control/sfcontrol.h"

/* Macros *********************************************************************/

/* specifies that a function does not return
 * used for quieting Visual Studio warnings */
#ifdef _MSC_VER
# if _MSC_VER >= 1400
#  define NORETURN __declspec(noreturn)
# else
#  define NORETURN
# endif
#else
# define NORETURN
#endif

#if !defined(__GNUC__) || __GNUC__ < 2 || \
    (__GNUC__ == 2 && __GNUC_MINOR__ < 5)
#define	__attribute__(x)	/* delete __attribute__ if non-gcc or gcc1 */
#endif

#define SNORT_SNPRINTF_SUCCESS 0
#define SNORT_SNPRINTF_TRUNCATION 1
#define SNORT_SNPRINTF_ERROR -1

#define SNORT_STRNCPY_SUCCESS 0
#define SNORT_STRNCPY_TRUNCATION 1
#define SNORT_STRNCPY_ERROR -1

#define SNORT_STRNLEN_ERROR -1

#define SECONDS_PER_DAY  86400  /* number of seconds in a day  */
#define SECONDS_PER_HOUR  3600  /* number of seconds in a hour */
#define SECONDS_PER_MIN     60     /* number of seconds in a minute */

#define STD_BUF  1024

#define COPY4(x, y) \
    x[0] = y[0]; x[1] = y[1]; x[2] = y[2]; x[3] = y[3];

#define COPY16(x,y) \
    x[0] = y[0]; x[1] = y[1]; x[2] = y[2]; x[3] = y[3]; \
    x[4] = y[4]; x[5] = y[5]; x[6] = y[6]; x[7] = y[7]; \
    x[8] = y[8]; x[9] = y[9]; x[10] = y[10]; x[11] = y[11]; \
    x[12] = y[12]; x[13] = y[13]; x[14] = y[14]; x[15] = y[15];


/* Externs ********************************************************************/
extern uint32_t *netmasks;


/* Data types *****************************************************************/

typedef struct _IntervalStats
{
    uint64_t recv, recv_total;
    uint64_t drop, drop_total;
    uint64_t processed, processed_total;
    uint64_t tcp, tcp_total;
    uint64_t udp, udp_total;
    uint64_t icmp, icmp_total;
    uint64_t arp, arp_total;
    uint64_t ipx, ipx_total;
    uint64_t eapol, eapol_total;
    uint64_t ipv6, ipv6_total;
    uint64_t ethloopback, ethloopback_total;
    uint64_t other, other_total;
    uint64_t frags, frags_total;
    uint64_t discards, discards_total;
    uint64_t frag_trackers, frag_trackers_total;
    uint64_t frag_rebuilt, frag_rebuilt_total;
    uint64_t frag_element, frag_element_total;
    uint64_t frag_incomp, frag_incomp_total;
    uint64_t frag_timeout, frag_timeout_total;
    uint64_t frag_mem_faults, frag_mem_faults_total;
    uint64_t tcp_str_packets, tcp_str_packets_total;
    uint64_t tcp_str_trackers, tcp_str_trackers_total;
    uint64_t tcp_str_flushes, tcp_str_flushes_total;
    uint64_t tcp_str_segs_used, tcp_str_segs_used_total;
    uint64_t tcp_str_segs_queued, tcp_str_segs_queued_total;
    uint64_t tcp_str_mem_faults, tcp_str_mem_faults_total;

#ifdef GRE
    uint64_t ip4ip4, ip4ip4_total;
    uint64_t ip4ip6, ip4ip6_total;
    uint64_t ip6ip4, ip6ip4_total;
    uint64_t ip6ip6, ip6ip6_total;

    uint64_t gre, gre_total;
    uint64_t gre_ip, gre_ip_total;
    uint64_t gre_eth, gre_eth_total;
    uint64_t gre_arp, gre_arp_total;
    uint64_t gre_ipv6, gre_ipv6_total;
    uint64_t gre_ipx, gre_ipx_total;
    uint64_t gre_loopback, gre_loopback_total;
    uint64_t gre_vlan, gre_vlan_total;
    uint64_t gre_ppp, gre_ppp_total;
#endif

#ifdef DLT_IEEE802_11
    uint64_t wifi_mgmt, wifi_mgmt_total;
    uint64_t wifi_control, wifi_control_total;
    uint64_t wifi_data, wifi_data_total;
#endif

} IntervalStats;


/* Public function prototypes *************************************************/
void StoreSnortInfoStrings(void);
int DisplayBanner(void);
int gmt2local(time_t);
void ts_print(register const struct timeval *, char *);
char *copy_argv(char **);
void strip(char *);
double CalcPct(uint64_t, uint64_t);
void GoDaemon(void);
void SignalWaitingParent(void);
void CheckLogDir(void);
char *read_infile(char *);
void CleanupProtoNames(void);
void CreatePidFile(const char *, pid_t);
void ClosePidFile(void);
void SetUidGid(int, int);
void InitGroups(int, int);
void SetChroot(char *, char **);
void DropStats(int);
/* Function For Displaying in SFR CLI */
void DisplayActionStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);
void TimeStart(void);
void TimeStop(void);

#ifndef __GNUC__
#define __attribute__(x)  /*NOTHING*/
#endif
void LogMessage(const char *, ...) __attribute__((format (printf, 1, 2)));
void WarningMessage(const char *, ...) __attribute__((format (printf, 1, 2)));
void ErrorMessage(const char *, ...) __attribute__((format (printf, 1, 2)));
typedef struct _ThrottleInfo
{
    time_t lastUpdate;
    /*Within this duration (in seconds), maximal one distinct message is logged*/
    uint32_t duration_to_log;
    uint64_t count;
    /*Till the message count reaches to count_to_log, maximal one distinct message is logged*/
    uint64_t count_to_log;
}ThrottleInfo;
void ErrorMessageThrottled(ThrottleInfo*,const char *, ...) __attribute__((format (printf, 2, 3)));
void LogThrottledByTimeCount(ThrottleInfo*,const char *, ...) __attribute__((format (printf, 2, 3)));

NORETURN void FatalError(const char *, ...) __attribute__((format (printf, 1, 2)));
NORETURN void SnortFatalExit(void);
int SnortSnprintf(char *, size_t, const char *, ...) __attribute__((format (printf, 3, 4)));
int SnortSnprintfAppend(char *, size_t, const char *, ...) __attribute__((format (printf, 3, 4)));

char *SnortStrdup(const char *);
int SnortStrncpy(char *, const char *, size_t);
char *SnortStrndup(const char *, size_t);
int SnortStrnlen(const char *, int);
const char *SnortStrnPbrk(const char *s, int slen, const char *accept);
const char *SnortStrnStr(const char *s, int slen, const char *searchstr);
const char *SnortStrcasestr(const char *s, int slen, const char *substr);
int CheckValueInRange(const char *value_str, char *option,
        unsigned long lo, unsigned long hi, unsigned long *value);

void *SnortAlloc2(size_t, const char *, ...);
char *CurrentWorkingDir(void);
char *GetAbsolutePath(char *dir);
char *StripPrefixDir(char *prefix, char *dir);
void PrintPacketData(const uint8_t *, const uint32_t);

char * ObfuscateIpToText(sfaddr_t *);

#ifndef WIN32
SF_LIST * SortDirectory(const char *);
int GetFilesUnderDir(const char *, SF_QUEUE *, const char *);
#endif

/***********************************************************
 If you use any of the functions in this section, you need
 to call free() on the char * that is returned after you are
 done using it. Otherwise, you will have created a memory
 leak.
***********************************************************/
char *hex(const u_char *, int);
char *fasthex(const u_char *, int);
int xatol(const char *, const char *);
unsigned int xatou(const char *, const char *);
unsigned int xatoup(const char *, const char *); // return > 0

static inline void* SnortMalloc (unsigned long size)
{
    void* pv = malloc(size);

    if ( pv )
        return pv;

    FatalError("Unable to allocate memory!  (%lu requested)\n", size);

    return NULL;
}

static inline void* SnortAlloc (unsigned long size)
{
    void* pv = calloc(size, sizeof(char));

    if ( pv )
        return pv;

    FatalError("Unable to allocate memory!  (%lu requested)\n", size);

    return NULL;
}

static inline long SnortStrtol(const char *nptr, char **endptr, int base)
{
    long iRet;
    errno = 0;
    iRet = strtol(nptr, endptr, base);

    return iRet;
}

static inline unsigned long SnortStrtoul(const char *nptr, char **endptr, int base)
{
        unsigned long iRet;
        errno = 0;
        iRet = strtoul(nptr, endptr, base);

        return iRet;
}

// Checks to make sure we're not going to evaluate a negative number for which
// strtoul() gladly accepts and parses returning an underflowed wrapped unsigned
// long without error.
//
// Buffer passed in MUST be NULL terminated.
//
// Returns
//  int
//    -1 if buffer is nothing but spaces or first non-space character is a
//       negative sign.  Also if errno is EINVAL (which may be due to a bad
//       base) or there was nothing to convert.
//     0 on success
//
// Populates pointer to uint32_t value passed in which should
// only be used on a successful return from this function.
//
// Also will set errno to ERANGE on a value returned from strtoul that is
// greater than UINT32_MAX, but still return success.
//
static inline int SnortStrToU32(const char *buffer, char **endptr,
        uint32_t *value, int base)
{
    unsigned long int tmp;

    if ((buffer == NULL) || (endptr == NULL) || (value == NULL))
        return -1;

    // Only positive numbers should be processed and strtoul will
    // eat up white space and process '-' and '+' so move past
    // white space and check for a negative sign.
    while (isspace((int)*buffer))
        buffer++;

    // If all spaces or a negative sign is found, return error.
    // XXX May also want to exclude '+' as well.
    if ((*buffer == '\0') || (*buffer == '-'))
        return -1;

    tmp = SnortStrtoul(buffer, endptr, base);

    // The user of the function should check for ERANGE in errno since this
    // function can be used such that an ERANGE error is acceptable and
    // value gets truncated to UINT32_MAX.
    if ((errno == EINVAL) || (*endptr == buffer))
        return -1;

    // If value is greater than a UINT32_MAX set value to UINT32_MAX
    // and errno to ERANGE
    if (tmp > UINT32_MAX)
    {
        tmp = UINT32_MAX;
        errno = ERANGE;
    }

    *value = (uint32_t)tmp;

    return 0;
}

static inline long SnortStrtolRange(const char *nptr, char **endptr, int base, long lo, long hi)
{
    long iRet = SnortStrtol(nptr, endptr, base);
    if ((iRet > hi) || (iRet < lo))
        *endptr = (char *)nptr;

    return iRet;
}

static inline unsigned long SnortStrtoulRange(const char *nptr, char **endptr, int base, unsigned long lo, unsigned long hi)
{
    unsigned long iRet = SnortStrtoul(nptr, endptr, base);
    if ((iRet > hi) || (iRet < lo))
        *endptr = (char *)nptr;

    return iRet;
}

static inline int IsEmptyStr(const char *str)
{
    const char *end;

    if (str == NULL)
        return 1;

    end = str + strlen(str);

    while ((str < end) && isspace((int)*str))
        str++;

    if (str == end)
        return 1;

    return 0;
}

#ifndef HAVE_GETTID
static inline pid_t gettid(void)
{
#if defined(LINUX) && defined(SYS_gettid)
    return syscall(SYS_gettid);
#else
    return getpid();
#endif
}
#endif

#endif /*__UTIL_H__*/
