/* MHD_config.h for W32 */
/* Created manually. */

/* *** Basic OS/compiler information *** */

/* This is a Windows system */
#define WINDOWS 1

/* Define if MS VC compiler is used */
#define MSVC 1

#ifndef __clang__
/* Define that MS VC does not support VLAs */
#ifndef __STDC_NO_VLA__
#define __STDC_NO_VLA__ 1
#endif /* ! __STDC_NO_VLA__ */
#else
/* If clang is used then variable-length arrays are supported. */
#define HAVE_C_VARARRAYS 1
#endif

/* Define to 1 if your C compiler supports inline functions. */
#define INLINE_FUNC 1

/* Define to prefix which will be used with MHD inline functions. */
#define _MHD_static_inline static __forceinline

#ifdef __clang__
/* Define to 1 if you have __builtin_bswap32() builtin function */
#define MHD_HAVE___BUILTIN_BSWAP32 1

/* Define to 1 if you have __builtin_bswap64() builtin function */
#define MHD_HAVE___BUILTIN_BSWAP64 1
#endif /* __clang__ */

/* The size of `size_t', as computed by sizeof. */
#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64) || defined(_WIN64)
#define SIZEOF_SIZE_T 8
#else  /* ! _WIN64 */
#define SIZEOF_SIZE_T 4
#endif /* ! _WIN64 */

/* The size of `tv_sec' member of `struct timeval', as computed by sizeof */
#define SIZEOF_STRUCT_TIMEVAL_TV_SEC 4

/* The size of `int64_t', as computed by sizeof. */
#define SIZEOF_INT64_T 8

/* The size of `uint64_t', as computed by sizeof. */
#define SIZEOF_UINT64_T 8

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG 8

/* Define to supported 'noreturn' function declaration */
#if defined(_STDC_VERSION__) && (__STDC_VERSION__ + 0) >= 201112L
#define _MHD_NORETURN _Noreturn
#else  /* before C11 */
#define _MHD_NORETURN __declspec(noreturn)
#endif /* before C11 */

/* *** MHD configuration *** */
/* Undef to disable feature */

/* Enable basic Auth support */
#define BAUTH_SUPPORT 1

/* Enable digest Auth support */
#define DAUTH_SUPPORT 1

/* The default HTTP Digest Auth default maximum nc (nonce count) value */
#define MHD_DAUTH_DEF_MAX_NC_ 1000

/* The default HTTP Digest Auth default nonce timeout value (in seconds) */
#define MHD_DAUTH_DEF_TIMEOUT_ 90

/* Enable MD5 hashing support. */
#define MHD_MD5_SUPPORT 1

/* Enable SHA-256 hashing support. */
#define MHD_SHA256_SUPPORT 1

/* Enable SHA-512/256 hashing support. */
#define MHD_SHA512_256_SUPPORT 1

/* Enable postprocessor.c */
#define HAVE_POSTPROCESSOR 1

/* Enable error messages */
#define HAVE_MESSAGES 1

/* Enable HTTP Upgrade support. */
#define UPGRADE_SUPPORT 1

/* Enable HTTP cookie parsing support. */
#define COOKIE_SUPPORT 1

/* *** OS features *** */

/* Provides IPv6 headers */
#define HAVE_INET6 1

/* Define to 1 if your system allow overriding the value of FD_SETSIZE macro  */
#define HAS_FD_SETSIZE_OVERRIDABLE 1

#if 0 /* Do not define the macro to keep maintability simple if system value is updated */
/* Define to system default value of FD_SETSIZE macro */
#  define MHD_SYS_FD_SETSIZE_ 64
#endif

/* Define to use socketpair for inter-thread communication */
#define _MHD_ITC_SOCKETPAIR 1

/* define to use W32 threads */
#define MHD_USE_W32_THREADS 1

#ifndef _WIN32_WINNT
/* MHD supports Windows XP and later W32 systems*/
#define _WIN32_WINNT 0x0600
#endif /* _WIN32_WINNT */

/* winsock poll is available only on Vista and later */
#if _WIN32_WINNT >= 0x0600
#define HAVE_POLL 1
#endif /* _WIN32_WINNT >= 0x0600 */

/* Define to 1 if you have the <winsock2.h> header file. */
#define HAVE_WINSOCK2_H 1

/* Define to 1 if you have the <ws2tcpip.h> header file. */
#define HAVE_WS2TCPIP_H 1

/* Define to 1 if you have the `_lseeki64' function. */
#define HAVE___LSEEKI64 1

/* Define to 1 if you have the `gmtime_s' function in W32 form. */
#define HAVE_W32_GMTIME_S 1

/* Define to 1 if you have the usable `calloc' function. */
#define HAVE_CALLOC 1

/* Define if you have usable assert() and assert.h */
#define HAVE_ASSERT 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H   1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H       1

#if _MSC_VER >= 1900 /* snprintf() supported natively since VS2015 */
/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1
#endif

#if _MSC_VER >= 1800
/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1
#endif

#if _MSC_VER + 0 >= 1800 /* VS 2013 and later */
/* Define to 1 if you have the <stdbool.h> header file and <stdbool.h> defines
   'bool' type. */
#define HAVE_STDBOOL_H 1
/* Define to 1 if you have the real boolean type. */
#define HAVE_REAL_BOOL 1
/* Define to 1 if you have the real boolean type. */
#define HAVE_BUILTIN_TYPE_BOOL 1
#else  /* before VS 2013 */

/* Define to type name which will be used as boolean type. */
#define bool int

/* Define to value interpreted by compiler as boolean "false", if "false" is
   not defined by system headers. */
#define false 0

/* Define to value interpreted by compiler as boolean "true", if "true" is not
   defined by system headers. */
#define true (!0)
#endif /* before VS 2013 */

/* Define to 1 if you have the `getsockname' function. */
#define HAVE_GETSOCKNAME 1

/* Define if you have usable `getsockname' function. */
#define MHD_USE_GETSOCKNAME 1

/* Define to 1 if your compiler supports __func__ magic-macro. */
#define HAVE___FUNC__ 1

#if _MSC_VER + 0 >= 1900 /* VS 2015 and later */
#if defined(_STDC_VERSION__) && (__STDC_VERSION__ + 0) >= 201112L
/* Define to 1 if your compiler supports 'alignof()' */
#define HAVE_C_ALIGNOF 1
/* Define to 1 if you have the <stdalign.h> header file. */
#define HAVE_STDALIGN_H 1
#endif /* C11 */
#endif /* VS 2015 and later */

/* Define to 1 if you have the 'rand' function. */
#define HAVE_RAND 1

/* *** Headers information *** */
/* Not really important as not used by code currently */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if you have the <sdkddkver.h> header file. */
#define HAVE_SDKDDKVER_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <windows.h> header file. */
#define HAVE_WINDOWS_H 1


/* *** Other useful staff *** */

#define _GNU_SOURCE  1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1


/* End of MHD_config.h */
