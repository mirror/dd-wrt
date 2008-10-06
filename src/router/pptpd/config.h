/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */
/*
 * acconfig.h
 *
 * Additional autoconf defines for this program.
 *
 * $Id: acconfig.h,v 1.6 2005/12/29 01:21:09 quozl Exp $
 */

/* Use BSD User land PPP? */
/* #undef BSDUSER_PPP */

/* Use SLIRP? */
/* #undef SLIRP */

/* Enable Broadcast Relay? */
#define BCRELAY 1

/* Work as a PNS rather than a PAC? */
/* #undef PNS_MODE */

/* Communicate between control and manager processes with a pipe */
/* #undef CTRL_MGR_PIPE */

/* Where is my pppd? */
#define PPP_BINARY "/usr/sbin/pppd"

/* Syslog Facility to use?  See openlog(3). */
#define PPTP_FACILITY LOG_DAEMON

/* Have libwrap? */
/* #undef HAVE_LIBWRAP */

/* Package name */
#define PACKAGE "pptpd"

/* Version */
#define VERSION "1.3.4"

/* Just #define to int if it's not defined */
/* #undef socklen_t */

/* These would be better as typedefs, but... */
/* #undef u_int8_t */
/* #undef u_int16_t */
/* #undef u_int32_t */

/* And the signed size_t */
/* (normal size_t is done by standard autoconf) */
/* #undef ssize_t */

/* Define if you have an openpty() (non-standard check) */
#define HAVE_OPENPTY 1

/* Define to 1 if you have the `daemon' function. */
#define HAVE_DAEMON 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the `getservbyname' function. */
#define HAVE_GETSERVBYNAME 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `c' library (-lc). */
#define HAVE_LIBC 1

/* Define to 1 if you have the `intl' library (-lintl). */
/* #undef HAVE_LIBINTL */

/* Define to 1 if you have the <libintl.h> header file. */
/* #undef HAVE_LIBINTL_H */

/* Define to 1 if you have the `nsl' library (-lnsl). */
#define HAVE_LIBNSL 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `util' library (-lutil). */
#define HAVE_LIBUTIL 1

/* Define to 1 if you have the <libutil.h> header file. */
/* #undef HAVE_LIBUTIL_H */

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <pty.h> header file. */
#define HAVE_PTY_H 1

/* Define to 1 if you have the `setproctitle' function. */
/* #undef HAVE_SETPROCTITLE */

/* Define to 1 if you have the `setsid' function. */
#define HAVE_SETSID 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcpy' function. */
#define HAVE_STRLCPY 1

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
#define HAVE_SYS_UIO_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `writev' function. */
#define HAVE_WRITEV 1

/* Name of package */
#define PACKAGE "pptpd"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.3.4"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */

/* Define to `unsigned short' if <sys/types.h> does not define. */
/* #undef u_int16_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef u_int32_t */

/* Define to `unsigned char' if <sys/types.h> does not define. */
/* #undef u_int8_t */
