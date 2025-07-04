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
#include "cmac.h"
#include "hash.h"

/* Zero a timespec */
extern void UTI_ZeroTimespec(struct timespec *ts);

/* Check if a timespec is zero */
extern int UTI_IsZeroTimespec(struct timespec *ts);

/* Convert a timeval into a timespec */
extern void UTI_TimevalToTimespec(const struct timeval *tv, struct timespec *ts);

/* Convert a timespec into a timeval */
extern void UTI_TimespecToTimeval(const struct timespec *ts, struct timeval *tv);

/* Convert a timespec into a floating point number of seconds */
extern double UTI_TimespecToDouble(const struct timespec *ts);

/* Convert a number of seconds expressed in floating point into a
   timespec */
extern void UTI_DoubleToTimespec(double d, struct timespec *ts);

/* Normalise a timespec, by adding or subtracting seconds to bring
   its nanosecond field into range */
extern void UTI_NormaliseTimespec(struct timespec *ts);

/* Convert a timeval into a floating point number of seconds */
extern double UTI_TimevalToDouble(const struct timeval *tv);

/* Convert a number of seconds expressed in floating point into a
   timeval */
extern void UTI_DoubleToTimeval(double a, struct timeval *b);

/* Normalise a struct timeval, by adding or subtracting seconds to bring
   its microseconds field into range */
extern void UTI_NormaliseTimeval(struct timeval *x);

/* Returns -1 if a comes earlier than b, 0 if a is the same time as b,
   and +1 if a comes after b */
extern int UTI_CompareTimespecs(const struct timespec *a, const struct timespec *b);

/* Calculate result = a - b */
extern void UTI_DiffTimespecs(struct timespec *result,
                              const struct timespec *a, const struct timespec *b);

/* Calculate result = a - b and return as a double */
extern double UTI_DiffTimespecsToDouble(const struct timespec *a, const struct timespec *b);

/* Add a double increment to a timespec to get a new one. 'start' is
   the starting time, 'end' is the result that we return.  This is
   safe to use if start and end are the same */
extern void UTI_AddDoubleToTimespec(const struct timespec *start, double increment,
                                    struct timespec *end);

/* Calculate the average and difference (as a double) of two timespecs */
extern void UTI_AverageDiffTimespecs(const struct timespec *earlier, const struct timespec *later,
                                     struct timespec *average, double *diff);

/* Calculate result = a - b + c */
extern void UTI_AddDiffToTimespec(const struct timespec *a, const struct timespec *b,
                                  const struct timespec *c, struct timespec *result);

/* Convert a timespec into a temporary string, largely for diagnostic
   display */
extern char *UTI_TimespecToString(const struct timespec *ts);

/* Convert an NTP timestamp into a temporary string, largely for
   diagnostic display */
extern char *UTI_Ntp64ToString(const NTP_int64 *ts);

/* Convert ref_id into a temporary string, for diagnostics */
extern char *UTI_RefidToString(uint32_t ref_id);

/* Convert an IP address to string, for diagnostics */
extern char *UTI_IPToString(const IPAddr *ip);

extern int UTI_StringToIP(const char *addr, IPAddr *ip);
extern int UTI_IsStringIP(const char *string);
extern int UTI_StringToIdIP(const char *addr, IPAddr *ip);
extern int UTI_IsIPReal(const IPAddr *ip);
extern uint32_t UTI_IPToRefid(const IPAddr *ip);
extern uint32_t UTI_IPToHash(const IPAddr *ip);
extern void UTI_IPHostToNetwork(const IPAddr *src, IPAddr *dest);
extern void UTI_IPNetworkToHost(const IPAddr *src, IPAddr *dest);
extern int UTI_CompareIPs(const IPAddr *a, const IPAddr *b, const IPAddr *mask);

extern char *UTI_IPSockAddrToString(const IPSockAddr *sa);

extern char *UTI_IPSubnetToString(IPAddr *subnet, int bits);

extern char *UTI_TimeToLogForm(time_t t);

/* Adjust time following a frequency/offset change */
extern void UTI_AdjustTimespec(const struct timespec *old_ts, const struct timespec *when,
                               struct timespec *new_ts, double *delta_time,
                               double dfreq, double doffset);

/* Get zero NTP timestamp with random bits below precision */
extern void UTI_GetNtp64Fuzz(NTP_int64 *ts, int precision);

extern double UTI_Ntp32ToDouble(NTP_int32 x);
extern NTP_int32 UTI_DoubleToNtp32(double x);

extern double UTI_Ntp32f28ToDouble(NTP_int32 x);
extern NTP_int32 UTI_DoubleToNtp32f28(double x);

/* Zero an NTP timestamp */
extern void UTI_ZeroNtp64(NTP_int64 *ts);

/* Check if an NTP timestamp is zero */
extern int UTI_IsZeroNtp64(const NTP_int64 *ts);

/* Compare two NTP timestamps.  Returns -1 if a is before b, 0 if a is equal to
   b, and 1 if a is after b. */
extern int UTI_CompareNtp64(const NTP_int64 *a, const NTP_int64 *b);

/* Compare an NTP timestamp with up to three other timestamps.  Returns 0
   if a is not equal to any of b1, b2, and b3, 1 otherwise. */
extern int UTI_IsEqualAnyNtp64(const NTP_int64 *a, const NTP_int64 *b1,
                               const NTP_int64 *b2, const NTP_int64 *b3);

/* Convert a timespec into an NTP timestamp */
extern void UTI_TimespecToNtp64(const struct timespec *src, NTP_int64 *dest,
                                const NTP_int64 *fuzz);

/* Convert an NTP timestamp into a timespec */
extern void UTI_Ntp64ToTimespec(const NTP_int64 *src, struct timespec *dest);

/* Calculate a - b in any epoch */
extern double UTI_DiffNtp64ToDouble(const NTP_int64 *a, const NTP_int64 *b);

/* Convert a difference in double (not a timestamp) from and to NTP format */
extern double UTI_Ntp64ToDouble(NTP_int64 *src);
extern void UTI_DoubleToNtp64(double src, NTP_int64 *dest);

/* Check if time + offset is sane */
extern int UTI_IsTimeOffsetSane(const struct timespec *ts, double offset);

/* Get 2 raised to power of a signed integer */
extern double UTI_Log2ToDouble(int l);

extern void UTI_TimespecNetworkToHost(const Timespec *src, struct timespec *dest);
extern void UTI_TimespecHostToNetwork(const struct timespec *src, Timespec *dest);

uint64_t UTI_Integer64NetworkToHost(Integer64 i);
Integer64 UTI_Integer64HostToNetwork(uint64_t i);

extern double UTI_FloatNetworkToHost(Float x);
extern Float UTI_FloatHostToNetwork(double x);

extern CMC_Algorithm UTI_CmacNameToAlgorithm(const char *name);
extern HSH_Algorithm UTI_HashNameToAlgorithm(const char *name);

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

/* Check and log a warning message if a file has more permissions than
   specified.  It does not return error if it is not an accessible file. */
extern int UTI_CheckFilePermissions(const char *path, mode_t perm);

/* Log a warning message if not having read access or having write access
   to a file/directory */
extern void UTI_CheckReadOnlyAccess(const char *path);

/* Open a file.  The full path of the file is constructed from the basedir
   (may be NULL), '/' (if basedir is not NULL), name, and suffix (may be NULL).
   Created files have specified permissions (umasked).  Returns NULL on error.
   The following modes are supported (if the mode is an uppercase character,
   errors are fatal):
   r/R - open an existing file for reading
   w/W - open a new file for writing (remove existing file)
   a/A - open an existing file for appending (create if does not exist) */
extern FILE *UTI_OpenFile(const char *basedir, const char *name, const char *suffix,
                          char mode, mode_t perm);

/* Rename a temporary file by changing its suffix.  The paths are constructed as
   in UTI_OpenFile().  If the renaming fails, the file will be removed. */
extern int UTI_RenameTempFile(const char *basedir, const char *name,
                              const char *old_suffix, const char *new_suffix);

/* Remove a file.  The path is constructed as in UTI_OpenFile(). */
extern int UTI_RemoveFile(const char *basedir, const char *name, const char *suffix);

/* Set process user/group IDs and drop supplementary groups */
extern void UTI_DropRoot(uid_t uid, gid_t gid);

/* Fill buffer with random bytes from /dev/urandom */
extern void UTI_GetRandomBytesUrandom(void *buf, unsigned int len);

/* Fill buffer with random bytes from /dev/urandom or a faster source if it's
   available (e.g. arc4random()), which may not necessarily be suitable for
   generating long-term keys */
extern void UTI_GetRandomBytes(void *buf, unsigned int len);

/* Close /dev/urandom and drop any cached data used by the GetRandom functions
   to prevent forked processes getting the same sequence of random numbers */
extern void UTI_ResetGetRandomFunctions(void);

/* Print data in hexadecimal format */
extern int UTI_BytesToHex(const void *buf, unsigned int buf_len, char *hex, unsigned int hex_len);

/* Parse a string containing data in hexadecimal format.  In-place conversion
   is supported. */
extern unsigned int UTI_HexToBytes(const char *hex, void *buf, unsigned int len);

/* Split a string into words separated by whitespace characters.  It returns
   the number of words found in the string, but saves only up to the specified
   number of pointers to the words. */
extern int UTI_SplitString(char *string, char **words, int max_saved_words);

/* Check if two buffers of the same length contain the same data, but do the
   comparison in constant time with respect to the returned value to avoid
   creating a timing side channel */
extern int UTI_IsMemoryEqual(const void *s1, const void *s2, unsigned int len);

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

/* Macro to make an assertion with the text of a long argument replaced
   with "0" to avoid bloating the compiled binary */
#ifdef NDEBUG
#define BRIEF_ASSERT(a)
#else
#define BRIEF_ASSERT(a) if (!(a)) assert(0)
#endif

#endif /* GOT_UTIL_H */
