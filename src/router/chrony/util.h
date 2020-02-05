/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Various utility functions
  */

#ifndef GOT_UTIL_H
#define GOT_UTIL_H

#include "sysincl.h"

#include "addressing.h"
#include "ntp.h"
#include "candm.h"
#include "hash.h"

/* Zero a timespec */
extern void UTI_ZeroTimespec(struct timespec *ts);

/* Check if a timespec is zero */
extern int UTI_IsZeroTimespec(struct timespec *ts);

/* Convert a timeval into a timespec */
extern void UTI_TimevalToTimespec(struct timeval *tv, struct timespec *ts);

/* Convert a timespec into a timeval */
extern void UTI_TimespecToTimeval(struct timespec *ts, struct timeval *tv);

/* Convert a timespec into a floating point number of seconds */
extern double UTI_TimespecToDouble(struct timespec *ts);

/* Convert a number of seconds expressed in floating point into a
   timespec */
extern void UTI_DoubleToTimespec(double d, struct timespec *ts);

/* Normalise a timespec, by adding or subtracting seconds to bring
   its nanosecond field into range */
extern void UTI_NormaliseTimespec(struct timespec *ts);

/* Convert a timeval into a floating point number of seconds */
extern double UTI_TimevalToDouble(struct timeval *tv);

/* Convert a number of seconds expressed in floating point into a
   timeval */
extern void UTI_DoubleToTimeval(double a, struct timeval *b);

/* Normalise a struct timeval, by adding or subtracting seconds to bring
   its microseconds field into range */
extern void UTI_NormaliseTimeval(struct timeval *x);

/* Returns -1 if a comes earlier than b, 0 if a is the same time as b,
   and +1 if a comes after b */
extern int UTI_CompareTimespecs(struct timespec *a, struct timespec *b);

/* Calculate result = a - b */
extern void UTI_DiffTimespecs(struct timespec *result, struct timespec *a, struct timespec *b);

/* Calculate result = a - b and return as a double */
extern double UTI_DiffTimespecsToDouble(struct timespec *a, struct timespec *b);

/* Add a double increment to a timespec to get a new one. 'start' is
   the starting time, 'end' is the result that we return.  This is
   safe to use if start and end are the same */
extern void UTI_AddDoubleToTimespec(struct timespec *start, double increment, struct timespec *end);

/* Calculate the average and difference (as a double) of two timespecs */
extern void UTI_AverageDiffTimespecs(struct timespec *earlier, struct timespec *later, struct timespec *average, double *diff);

/* Calculate result = a - b + c */
extern void UTI_AddDiffToTimespec(struct timespec *a, struct timespec *b, struct timespec *c, struct timespec *result);

/* Convert a timespec into a temporary string, largely for diagnostic
   display */
extern char *UTI_TimespecToString(struct timespec *ts);

/* Convert an NTP timestamp into a temporary string, largely for
   diagnostic display */
extern char *UTI_Ntp64ToString(NTP_int64 *ts);

/* Convert ref_id into a temporary string, for diagnostics */
extern char *UTI_RefidToString(uint32_t ref_id);

/* Convert an IP address to string, for diagnostics */
extern char *UTI_IPToString(IPAddr *ip);

extern int UTI_StringToIP(const char *addr, IPAddr *ip);
extern uint32_t UTI_IPToRefid(IPAddr *ip);
extern uint32_t UTI_IPToHash(IPAddr *ip);
extern void UTI_IPHostToNetwork(IPAddr *src, IPAddr *dest);
extern void UTI_IPNetworkToHost(IPAddr *src, IPAddr *dest);
extern int UTI_CompareIPs(IPAddr *a, IPAddr *b, IPAddr *mask);

extern void UTI_SockaddrToIPAndPort(struct sockaddr *sa, IPAddr *ip, unsigned short *port);
extern int UTI_IPAndPortToSockaddr(IPAddr *ip, unsigned short port, struct sockaddr *sa);
extern char *UTI_SockaddrToString(struct sockaddr *sa);
extern const char *UTI_SockaddrFamilyToString(int family);

extern char *UTI_TimeToLogForm(time_t t);

/* Adjust time following a frequency/offset change */
extern void UTI_AdjustTimespec(struct timespec *old_ts, struct timespec *when, struct timespec *new_ts, double *delta_time, double dfreq, double doffset);

/* Get zero NTP timestamp with random bits below precision */
extern void UTI_GetNtp64Fuzz(NTP_int64 *ts, int precision);

extern double UTI_Ntp32ToDouble(NTP_int32 x);
extern NTP_int32 UTI_DoubleToNtp32(double x);

/* Zero an NTP timestamp */
extern void UTI_ZeroNtp64(NTP_int64 *ts);

/* Check if an NTP timestamp is zero */
extern int UTI_IsZeroNtp64(NTP_int64 *ts);

/* Compare two NTP timestamps.  Returns -1 if a is before b, 0 if a is equal to
   b, and 1 if a is after b. */
extern int UTI_CompareNtp64(NTP_int64 *a, NTP_int64 *b);

/* Compare an NTP timestamp with up to three other timestamps.  Returns 0
   if a is not equal to any of b1, b2, and b3, 1 otherwise. */
extern int UTI_IsEqualAnyNtp64(NTP_int64 *a, NTP_int64 *b1, NTP_int64 *b2, NTP_int64 *b3);

/* Convert a timespec into an NTP timestamp */
extern void UTI_TimespecToNtp64(struct timespec *src, NTP_int64 *dest, NTP_int64 *fuzz);

/* Convert an NTP timestamp into a timespec */
extern void UTI_Ntp64ToTimespec(NTP_int64 *src, struct timespec *dest);

/* Check if time + offset is sane */
extern int UTI_IsTimeOffsetSane(struct timespec *ts, double offset);

/* Get 2 raised to power of a signed integer */
extern double UTI_Log2ToDouble(int l);

extern void UTI_TimespecNetworkToHost(Timespec *src, struct timespec *dest);
extern void UTI_TimespecHostToNetwork(struct timespec *src, Timespec *dest);

extern double UTI_FloatNetworkToHost(Float x);
extern Float UTI_FloatHostToNetwork(double x);

/* Set FD_CLOEXEC on descriptor */
extern int UTI_FdSetCloexec(int fd);

extern void UTI_SetQuitSignalsHandler(void (*handler)(int), int ignore_sigpipe);

/* Get directory (as an allocated string) for a path */
extern char *UTI_PathToDir(const char *path);

/* Create a directory with a specified mode (umasked) and set its uid/gid.
   Create also any parent directories that don't exist with mode 755 and
   default uid/gid.  Returns 1 if created or already exists (even with
   different mode/uid/gid), 0 otherwise. */
extern int UTI_CreateDirAndParents(const char *path, mode_t mode, uid_t uid, gid_t gid);

/* Check if a directory is secure.  It must not have other than the specified
   permissions and its uid/gid must match the specified values. */
extern int UTI_CheckDirPermissions(const char *path, mode_t perm, uid_t uid, gid_t gid);

/* Set process user/group IDs and drop supplementary groups */
extern void UTI_DropRoot(uid_t uid, gid_t gid);

/* Fill buffer with random bytes from /dev/urandom */
extern void UTI_GetRandomBytesUrandom(void *buf, unsigned int len);

/* Fill buffer with random bytes from /dev/urandom or a faster source if it's
   available (e.g. arc4random()), which may not necessarily be suitable for
   generating long-term keys */
extern void UTI_GetRandomBytes(void *buf, unsigned int len);

/* Macros to get maximum and minimum of two values */
#ifdef MAX
#undef MAX
#endif
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#ifdef MIN
#undef MIN
#endif
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Macro to clamp a value between two values */
#define CLAMP(min, x, max) (MAX((min), MIN((x), (max))))

#define SQUARE(x) ((x) * (x))

#endif /* GOT_UTIL_H */
