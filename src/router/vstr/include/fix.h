#ifndef FIX_H
#define FIX_H

/* so just #define FIX_NAMSPACE_SYMBOL abcd_ (or whatever namespace
 * you are taking, then everything will just work) */
#define FIX_XSTR2(x, y) x ## y
#define FIX_XSTR1(x, y) FIX_XSTR2(x, y)
#define FIX_SYMBOL(p) FIX_XSTR1(FIX_NAMESPACE_SYMBOL, p)

#ifndef USE_WIDE_CHAR_T
# define USE_WIDE_CHAR_T 1
#endif

#undef NULL
#define NULL ((void *)0)

#ifdef HAVE_PRCTL
/* from linux/prctl.h ... but user space isn't supposed to use that */
# ifndef PR_SET_PDEATHSIG
extern int prctl(int, unsigned long, unsigned long,
                 unsigned long, unsigned long);
#  define PR_SET_PDEATHSIG 1
# endif
#endif

#ifndef SIZE_MAX
# define SIZE_MAX ULONG_MAX
#endif

#ifndef MAP_FAILED
# define MAP_FAILED ((void *) -1)
#endif

#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

#ifndef HAVE_OPEN64
# define open64 open
#endif

#ifndef SHUT_RD
# define SHUT_RD 0
#endif
#ifndef SHUT_WR
# define SHUT_WR 1
#endif
#ifndef SHUT_RDWR
# define SHUT_RDWR 2
#endif

#ifndef SO_PEERCRED
# define SO_PEERCRED 0
#endif

#ifndef SO_PASSCRED
# define SO_PASSCRED 0
#endif

#ifndef MSG_TRUNC
# define MSG_TRUNC 0
#endif

#ifndef MSG_CTRUNC
# define MSG_CTRUNC 0
#endif

#ifndef PF_LOCAL
# define PF_LOCAL PF_UNIX
#endif

#ifndef AF_LOCAL
# define AF_LOCAL AF_UNIX
#endif

#ifndef AF_INET
# define AF_INET 65532
#endif

#ifndef AF_INET6
# define AF_INET6 (AF_INET+1)
#endif

#ifndef INET_ADDRSTRLEN
# define INET_ADDRSTRLEN sizeof("255.255.255.255")
#endif

#ifndef INET6_ADDRSTRLEN
/* the padding at the end is bcause glibc has this value as 46, which is
 * 6 more than the below would give otherwise ... who knows why ? */
# define INET6_ADDRSTRLEN sizeof("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF0123456")
#endif

#ifndef TCP_CORK
# define TCP_CORK 0
#endif

#ifndef SA_NODEFER
# define SA_NODEFER 0
#endif

#ifndef SA_NOMASK
# define SA_NOMASK SA_NODEFER
#endif

#ifndef INTMAX_MIN
# define INTMAX_MIN LONG_MIN
#endif

#ifndef INTMAX_MAX
# define INTMAX_MAX LONG_MAX
#endif

#ifndef UINTMAX_MAX
# define UINTMAX_MAX ULONG_MAX
#endif

#ifndef  UIO_MAXIOV
# define UIO_MAXIOV 1
#endif

#ifndef HAVE_STRERROR_R
# define strerror_r(x, y, z) (errno = ENOSYS, -1)
#endif

#ifndef HAVE_VA_COPY
# ifdef HAVE___VA_COPY
#  define HAVE_VA_COPY 1
#  undef va_copy
#  define va_copy(x, y) __va_copy(x, y)
# endif
#endif

#ifdef HAVE_VA_COPY
# define VA_R_DECL(x) va_list x
# define VA_C_DECL(x) va_list x
# define VA_R_START(x, y) va_start(x, y)
# define VA_C_START(x, y)
# define VA_C_COPY(x, y) va_copy(x, y)
# define VA_R_END(x) va_end(x)
# define VA_C_END(x)
#else
# define VA_R_DECL(x) int x
# define VA_C_DECL(x) va_list x
# define VA_R_START(x, y) x = 0
# define VA_C_START(x, y) va_start(x, y)
# define VA_C_COPY(x, y)
# define VA_R_END(x)
# define VA_C_END(x) va_end(x)
#endif

#if defined(HAVE_OLD_SIGCONTEXT) && !defined(HAVE_SIGCONTEXT)
# include <asm/sigcontext.h>
# define HAVE_SIGCONTEXT 1
# undef sigcontext
# define sigcontext sigcontext_struct
#endif

#if defined(USE_TERMCAP) && !defined(HAVE_DECL_TERMCAP_VARS)
extern char PC; /* NetBSD ... probably among others */
extern short ospeed;
#endif

#ifndef SCANDIR_STRUCT_DIRNET
/* if you are on glibc-2.0.x (and probably others) then you can do...
 * #define SCANDIR_STRUCT_DIRNET struct dirent
 * -- glibc-2.1.x (and presumably above) works with what's below
 */
# define SCANDIR_STRUCT_DIRNET const struct dirent
#endif

#if !defined(__GNUC__) && !defined(__PRETTY_FUNCTION__)
# define __PRETTY_FUNCTION__ "(n/a)"
#endif

#if !defined(__attribute__) && !defined(__GNUC__) || defined(__STRICT_ANSI__)
# define __attribute__(x)
#endif

#ifdef HAVE_C9x_STRUCT_HACK
# define STRUCT_HACK_ARRAY(x) x[]
# define STRUCT_HACK_SZ(type) (0)
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
# define STRUCT_HACK_ARRAY(x) x[0]
# define STRUCT_HACK_SZ(type) (0)
# else
# define STRUCT_HACK_ARRAY(x) x[1]
# define STRUCT_HACK_SZ(type) (sizeof(type))
# endif
#endif

#if !defined(HAVE_CMSGHDR_STRUCT)
struct cmsghdr
{
 size_t cmsg_len;
 int cmsg_level;
 int cmsg_type;
};
#endif

#if !(HAVE_CMSGHDR_RIGHTS)
# define SCM_RIGHTS 0
#endif

#if !(HAVE_CMSGHDR_CREDENTIALS)
# define SCM_CREDENTIALS 0
#endif

#ifndef HAVE_CMSG_DATA
# define CMSG_DATA(x) ((unsigned char *)((struct cmsghdr *)(x) + 1))
#endif

#ifndef HAVE_STRERROR
# define strerror(x) (" ** Not available ** ")
#endif

#ifndef HAVE_OFFSETOF
# warning "This offsetof isn't guaranteed to work"
# define offsetof(type, field) ((size_t) (&((type *)0)->field))
#endif

#ifndef HAVE_DIFFTIME
# warning "This difftime isn't guaranteed to work"
# define difftime(x, y) ((x) - (y))
#endif

#ifndef HAVE_SIGEMPTYSET
# define sigemptyset(x) (*(x) = 0)
#endif

#ifndef HAVE_MEMCHR
# undef memchr
extern void *FIX_SYMBOL(memchr)(const void *source, int character, size_t num);
#define memchr(a, b, c) FIX_SYMBOL(memchr)(a, b, c)
#endif

#ifndef HAVE_MEMRCHR
# undef memrchr
extern void *FIX_SYMBOL(memrchr)(const void *source, int character, size_t num);
#define memrchr(a, b, c) FIX_SYMBOL(memrchr)(a, b, c)
#endif

#ifndef HAVE_MEMCPY
# undef memcpy
extern void *FIX_SYMBOL(memcpy)(void *dest, const void *src, size_t n);
#define memcpy(a, b, c) FIX_SYMBOL(memcpy)(a, b, c)
#endif

#ifndef HAVE_MEMPCPY
# undef mempcpy
extern void *FIX_SYMBOL(mempcpy)(void *dest, const void *src, size_t n);
#define mempcpy(a, b, c) FIX_SYMBOL(mempcpy)(a, b, c)
#endif

#ifndef HAVE_MEMCMP
# undef memcmp
extern int FIX_SYMBOL(memcmp)(const void *dest, const void *src, size_t n);
#define memcmp(a, b, c) FIX_SYMBOL(memcmp)(a, b, c)
#endif

#ifndef HAVE_MEMMEM
# undef memmem
extern void *FIX_SYMBOL(memmem)(const void *src, size_t src_len,
                                const void *needle, size_t needle_len);
#define memmem(a, b, c, d) FIX_SYMBOL(memmem)(a, b, c, d)
#endif

#ifndef HAVE_STRNCMP
# undef strncmp
extern int FIX_SYMBOL(strncmp)(const char *, const char *, size_t);
#define strncmp(a, b, c) FIX_SYMBOL(strncmp)(a, b, c)
#endif

#ifndef HAVE_STRNCASECMP
# undef strncasecmp
extern int FIX_SYMBOL(strncasecmp)(const char *, const char *, size_t);
#define strncasecmp(a, b, c) FIX_SYMBOL(strncasecmp)(a, b, c)
#endif

#ifndef HAVE_STRNLEN
# undef strnlen
extern size_t FIX_SYMBOL(strnlen)(const char *, size_t);
#define strnlen(a, b) FIX_SYMBOL(strnlen)(a, b)
#endif

#ifndef HAVE_STRNCHR
# undef strnchr
extern char *FIX_SYMBOL(strnchr)(const char *, char, size_t);
#define strnchr(a, b, c) FIX_SYMBOL(strnchr)(a, b, c)
#endif

#ifndef HAVE_STRCASECMP
# undef strcasecmp
extern int FIX_SYMBOL(strcasecmp)(const char *, const char *);
#define strcasecmp(a, b) FIX_SYMBOL(strncasecmp)(a, b)
#endif

#ifndef HAVE_STPCPY
extern char *FIX_SYMBOL(stpcpy)(char *, const char *);
#define stpcpy(a, b) FIX_SYMBOL(stpcpy)(a, b)
#endif

#ifndef HAVE_C9X_SNPRINTF_RET
# undef vsnprintf
extern int FIX_SYMBOL(vsnprintf)(char *, size_t, const char *, va_list);
#define vsnprintf(a, b, c, d) FIX_SYMBOL(vsnprintf)(a, b, c, d)
#endif

#ifndef HAVE_ASPRINTF
# undef asprintf
extern int FIX_SYMBOL(asprintf)(char **, const char *, ...);
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  define asprintf(ret, fmt, args...) FIX_SYMBOL(asprintf)(ret, fmt , ## args)
# else
#  define asprintf FIX_SYMBOL(asprintf)
# endif
#endif

#if !defined(HAVE_GETOPT_LONG) && defined(HAVE_POSIX_HOST)
/* Declarations for getopt.
   Copyright (C) 1989,90,91,92,93,94,96,97 Free Software Foundation, Inc.
                 2000 James Antill <james@and.org>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.  */

/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

#undef optarg
extern char *FIX_SYMBOL(optarg);
#define optarg FIX_SYMBOL(optarg)

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns -1, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

#undef optind
extern int FIX_SYMBOL(optind);
#define optind FIX_SYMBOL(optind)
/* Callers store zero here to inhibit the error message `getopt' prints
   for unrecognized options.  */

#undef opterr
extern int FIX_SYMBOL(opterr);
#define opterr FIX_SYMBOL(opterr)

/* Set to an option character which was unrecognized.  */

#undef optopt
extern int FIX_SYMBOL(optopt);
#define optopt FIX_SYMBOL(optopt)

/* Describe the long-named options requested by the application.
   The LONG_OPTIONS argument to getopt_long or getopt_long_only is a vector
   of `struct option' terminated by an element containing a name which is
   zero.

   The field `has_arg' is:
   no_argument          (or 0) if the option does not take an argument,
   required_argument    (or 1) if the option requires an argument,
   optional_argument    (or 2) if the option takes an optional argument.

   If the field `flag' is not NULL, it points to a variable that is set
   to the value given in the field `val' when the option is found, but
   left unchanged if the option is not found.

   To have a long-named option do something other than set an `int' to
   a compiled-in constant, such as set a value from `optarg', set the
   option's `flag' field to zero and its `val' field to a nonzero
   value (the equivalent single-letter option character, if there is
   one).  For long options that have a zero `flag' field, `getopt'
   returns the contents of the `val' field.  */

struct option
{
  const char *name;
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */

#define no_argument             0
#define required_argument       1
#define optional_argument       2

#undef getopt
extern int FIX_SYMBOL(getopt) (int argc, char *const *argv, const char *shortopts);
#define getopt(a, b, c) FIX_SYMBOL(getopt)(a, b, c)

#undef getopt_long
extern int FIX_SYMBOL(getopt_long) (int argc, char *const *argv,
                                    const char *shortopts,
                                    const struct option *longopts,
                                    int *longind);
#define getopt_long(a, b, c, d, e) FIX_SYMBOL(getopt_long)(a, b, c, d, e)

#undef getopt_long_only
extern int FIX_SYMBOL(getopt_long_only) (int argc, char *const *argv,
                             const char *shortopts,
                             const struct option *longopts, int *longind);
#define getopt_long_only(a, b, c, d, e) FIX_SYMBOL(getopt_long_only)(a, b, c, d, e)

/* Internal only.  Users should not call this directly.  */
#undef _getopt_internal
extern int FIX_SYMBOL(_getopt_internal) (int argc, char *const *argv,
                                         const char *shortopts,
                                         const struct option *longopts,
                                         int *longind,
                                         int long_only);
#define _getopt_internal(a, b, c, d, e, f) FIX_SYMBOL(_getopt_internal)(a, b, c, d, e, f)
#endif

#if !defined(HAVE_POLL) && defined(HAVE_POSIX_HOST)
/* Compatibility definitions for System V `poll' interface.
   Copyright (C) 1994, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Data structure describing a polling request.  */
struct pollfd
  {
    int fd;                     /* File descriptor to poll.  */
    short int events;           /* Types of events poller cares about.  */
    short int revents;          /* Types of events that actually occurred.  */
  };

/* Event types that can be polled for.  These bits may be set in `events'
   to indicate the interesting event types; they will appear in `revents'
   to indicate the status of the file descriptor.  */
#define POLLIN          01              /* There is data to read.  */
#define POLLPRI         02              /* There is urgent data to read.  */
#define POLLOUT         04              /* Writing now will not block.  */
#define POLLERR        010              /* there is an error on the fd */
#define POLLNVAL       010              /* the fd is closed */
#define POLLHUP        010              /* a hangup occured on the fd */
/* Some aliases.  */
#define POLLWRNORM      POLLOUT

extern int FIX_SYMBOL(poll)(struct pollfd *, unsigned long int, int);
#define poll(a, b, c) FIX_SYMBOL(poll)(a, b, c)
#endif


#endif

#ifndef HAVE_SYS_UIO_H
struct iovec
  {
    void *iov_base;     /* Pointer to data.  */
    size_t iov_len;     /* Length of data.  */
  };
#endif

#ifndef HAVE_INET_NTOP
#undef inet_ntop
extern const char *FIX_SYMBOL(inet_ntop)(int, const void *, char *, size_t);
#define inet_ntop(a, b, c, d) FIX_SYMBOL(inet_ntop)(a, b, c, d)
#endif

#ifndef HAVE_INET_PTON
#undef inet_pton
extern int FIX_SYMBOL(inet_pton)(int, const char *, void *);
#define inet_pton(a, b, c) FIX_SYMBOL(inet_pton)(a, b, c)
#endif

/* the problem is that even in Linux you cannot pass anything as the fds
 * to sendfile(2) currently only local files are allowed as the in_fd */
#ifndef HAVE_SENDFILE
extern ssize_t FIX_SYMBOL(sendfile)(int, int, off_t *, size_t);
#define sendfile(a, b, c, d) FIX_SYMBOL(sendfile)(a, b, c, d)
#endif

#if !defined(HAVE_WCSNRTOMBS) && USE_WIDE_CHAR_T
#undef wcsnrtombs
extern size_t FIX_SYMBOL(wcsnrtombs)(char *, const wchar_t **, size_t,
                                     size_t, mbstate_t *);
#define wcsnrtombs(a, b, c, d, e) FIX_SYMBOL(wcsnrtombs)(a, b, c, d, e)
#endif
