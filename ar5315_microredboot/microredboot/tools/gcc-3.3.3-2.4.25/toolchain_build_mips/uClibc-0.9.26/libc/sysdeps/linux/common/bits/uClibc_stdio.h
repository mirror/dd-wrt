/*  Copyright (C) 2002     Manuel Novoa III
 *  Header for my stdio library for linux and (soon) elks.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  This code is currently under development.  Also, I plan to port
 *  it to elks which is a 16-bit environment with a fairly limited
 *  compiler.  Therefore, please refrain from modifying this code
 *  and, instead, pass any bug-fixes, etc. to me.  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */

#ifndef _STDIO_H
#error Always include <stdio.h> rather than <bits/uClibc_stdio.h>
#endif

/**********************************************************************/
#ifdef __UCLIBC__

#ifdef __UCLIBC_HAS_THREADS__
#define __STDIO_THREADSAFE
#endif

#ifdef __UCLIBC_HAS_LFS__
#define __STDIO_LARGE_FILES
#endif /* __UCLIBC_HAS_LFS__ */

#ifdef __UCLIBC_HAS_WCHAR__
#define __STDIO_WIDE
#endif

#define __STDIO_BUFFERS
/* ANSI/ISO mandate at least 256. */
#if defined(__UCLIBC_HAS_STDIO_BUFSIZ_NONE__)
/* Fake this because some apps use stdio.h BUFSIZ. */
#define _STDIO_BUFSIZ			256
#undef __STDIO_BUFFERS
#elif defined(__UCLIBC_HAS_STDIO_BUFSIZ_256__)
#define _STDIO_BUFSIZ			256
#elif defined(__UCLIBC_HAS_STDIO_BUFSIZ_512__)
#define _STDIO_BUFSIZ			512
#elif defined(__UCLIBC_HAS_STDIO_BUFSIZ_1024__)
#define _STDIO_BUFSIZ		   1024
#elif defined(__UCLIBC_HAS_STDIO_BUFSIZ_2048__)
#define _STDIO_BUFSIZ		   2048
#elif defined(__UCLIBC_HAS_STDIO_BUFSIZ_4096__)
#define _STDIO_BUFSIZ		   4096
#elif defined(__UCLIBC_HAS_STDIO_BUFSIZ_8192__)
#define _STDIO_BUFSIZ		   8192
#else
#error config seems to be out of sync regarding bufsiz options
#endif

#ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
#define __STDIO_GETC_MACRO
#endif

#ifdef __UCLIBC_HAS_STDIO_PUTC_MACRO__
#define __STDIO_PUTC_MACRO
#endif

#ifdef __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__
#define __STDIO_AUTO_RW_TRANSITION
#endif

#ifdef __UCLIBC_HAS_FOPEN_LARGEFILE_MODE__
#define __STDIO_FOPEN_LARGEFILE_MODE
#endif

#ifdef __UCLIBC_HAS_FOPEN_LARGEFILE_MODE__
#define __STDIO_FOPEN_EXCLUSIVE_MODE
#endif

#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
#define __STDIO_PRINTF_M_SUPPORT
#endif

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#define __STDIO_GLIBC_CUSTOM_STREAMS
#endif

#ifdef __UCLIBC_HAS_STDIO_BUFSIZ_NONE__
#define _STDIO_BUILTIN_BUF_SIZE		0
#else  /* __UCLIBC_HAS_STDIO_BUFSIZ_NONE__ */
#if defined(__UCLIBC_HAS_STDIO_BUILTIN_BUFFER_NONE__)
#define _STDIO_BUILTIN_BUF_SIZE		0
#elif defined(__UCLIBC_HAS_STDIO_BUILTIN_BUFFER_4__)
#define _STDIO_BUILTIN_BUF_SIZE		4
#elif defined(__UCLIBC_HAS_STDIO_BUILTIN_BUFFER_8__)
#define _STDIO_BUILTIN_BUF_SIZE		8
#else
#error config seems to be out of sync regarding builtin buffer size
#endif
#endif /* __UCLIBC_HAS_STDIO_BUFSIZ_NONE__ */

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
#define __STDIO_GLIBC_CUSTOM_PRINTF
#endif


/* Currently unimplemented/untested */
/* #define __STDIO_FLEXIBLE_SETVBUF */


/* Make sure defines related to large files are consistent. */
#ifdef _LIBC

#ifdef __UCLIBC_HAS_LFS__
#undef __USE_LARGEFILE
#undef __USE_LARGEFILE64
#undef __USE_FILE_OFFSET64
/* if we're actually building uClibc with large file support, only define... */
#define __USE_LARGEFILE64	1
#endif /* __UCLIBC_HAS_LFS__ */

#else  /* not _LIBC */

#ifndef __UCLIBC_HAS_LFS__
#if defined(__LARGEFILE64_SOURCE) || defined(__USE_LARGEFILE64) \
    || defined(__USE_FILE_OFFSET64)
#error Sorry... uClibc was built without large file support!
#endif
#endif /* __UCLIBC_HAS_LFS__ */

#endif /* _LIBC */

#endif /* __UCLIBC__ */
/**********************************************************************/
/* These are the stdio configuration options.  Keep them here until
   uClibc's configuration process gets reworked. */

#ifdef __STDIO_WIDE
#define __need_wchar_t
#include <stddef.h>
/* Note: we don't really need mbstate for 8-bit locales.  We do for UTF-8.
 * For now, always use it. */
#define __STDIO_MBSTATE
#define __need_mbstate_t
#include <wchar.h>
#endif


/* For uClibc, these are currently handled above. */
/* #define __STDIO_BUFFERS */
/* #define __STDIO_GETC_MACRO */
/* #define __STDIO_PUTC_MACRO */
/* #define __STDIO_LARGE_FILES */
/* #define __STDIO_THREADSAFE */
/* ANSI/ISO mandate at least 256. */
/* #define _STDIO_BUFSIZ			256 */
/* #define __STDIO_AUTO_RW_TRANSITION */
/* #define __STDIO_FOPEN_EXCLUSIVE_MODE */
/* #define __STDIO_PRINTF_M_SPEC */
/* #define __STDIO_GLIBC_CUSTOM_STREAMS */
/* L mode extension for fopen. */
/* #define __STDIO_FOPEN_LARGEFILE_MODE */
/* size of builtin buf -- only tested with 0 */
/* #define _STDIO_BUILTIN_BUF_SIZE		0 */
/* Currently unimplemented/untested */
/* #define __STDIO_FLEXIBLE_SETVBUF */

/**********************************************************************/
/* TODO -- posix or gnu -- belongs in limits.h and >= 9 for sus */
/* NOTE: for us it is currently _always_ 9 */
/*#define NL_ARGMAX			9*/

/**********************************************************************/

/* These are consistency checks on the different options */

#ifndef __STDIO_BUFFERS
#undef __STDIO_GETC_MACRO
#undef __STDIO_PUTC_MACRO
#endif

#ifdef __BCC__
#undef __STDIO_LARGE_FILES
#endif

#ifndef __STDIO_LARGE_FILES
#undef __STDIO_FOPEN_LARGEFILE_MODE
#endif

/**********************************************************************/

#ifdef __STDIO_THREADSAFE
/* Need this for pthread_mutex_t. */
#include <bits/pthreadtypes.h>

#define __STDIO_THREADLOCK(STREAM) \
	if ((STREAM)->user_locking == 0) { \
		__pthread_mutex_lock(&(STREAM)->lock); \
	}

#define __STDIO_THREADUNLOCK(STREAM) \
	if ((STREAM)->user_locking == 0) { \
		__pthread_mutex_unlock(&(STREAM)->lock); \
	}

#define __STDIO_THREADTRYLOCK(STREAM) \
	if ((STREAM)->user_locking == 0) { \
		__pthread_mutex_trylock(&(STREAM)->lock); \
	}

#define __STDIO_SET_USER_LOCKING(STREAM)   	((STREAM)->user_locking = 1)

#else  /* __STDIO_THREADSAFE */

#define __STDIO_THREADLOCK(STREAM)
#define __STDIO_THREADUNLOCK(STREAM)
#define __STDIO_THREADTRYLOCK(STREAM)

#define __STDIO_SET_USER_LOCKING(STREAM)

#endif /* __STDIO_THREADSAFE */

/* This file may eventually have two personalities:
   1) core stuff (similar to glibc's libio.h)
   2) extern inlines (for glibc's bits/stdio.h)
   Right now, only (1) is implemented. */

#define _STDIO_IOFBF 0	/* Fully buffered.  */
#define _STDIO_IOLBF 1	/* Line buffered.  */
#define _STDIO_IONBF 2	/* No buffering.  */

typedef struct {
	__off_t __pos;
#ifdef __STDIO_MBSTATE
	__mbstate_t __mbstate;
#endif
#ifdef __STDIO_WIDE
	int mblen_pending;
#endif
} __stdio_fpos_t;

#ifdef __STDIO_LARGE_FILES
typedef struct {
	__off64_t __pos;
#ifdef __STDIO_MBSTATE
	__mbstate_t __mbstate;
#endif
#ifdef __STDIO_WIDE
	int mblen_pending;
#endif
} __stdio_fpos64_t;
#endif


/**********************************************************************/
#ifdef __STDIO_LARGE_FILES
typedef __off64_t __offmax_t;	/* TODO -- rename this? */
#else
typedef __off_t __offmax_t;		/* TODO -- rename this? */
#endif

/**********************************************************************/

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS

typedef __ssize_t __io_read_fn(void *cookie,
							   char *buf, size_t bufsize);
typedef __ssize_t __io_write_fn(void *cookie,
								const char *buf, size_t bufsize);
/* NOTE: GLIBC difference!!! -- fopencookie seek function
 * For glibc, the type of pos is always (__off64_t *) but in our case
 * it is type (__off_t *) when the lib is built without large file support.
 */
typedef int __io_seek_fn(void *cookie,
						 __offmax_t *pos, int whence);
typedef int __io_close_fn(void *cookie);

typedef struct {
	__io_read_fn *read;
	__io_write_fn *write;
	__io_seek_fn *seek;
	__io_close_fn *close;
} _IO_cookie_io_functions_t;

#if defined(_LIBC) || defined(_GNU_SOURCE)
typedef __io_read_fn cookie_read_function_t;
typedef __io_write_fn cookie_write_function_t;
typedef __io_seek_fn cookie_seek_function_t;
typedef __io_close_fn cookie_close_function_t;

typedef _IO_cookie_io_functions_t cookie_io_functions_t;
#endif /* _GNU_SOURCE */

#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */

/*
 * ungot scheme:
 * 0 0   none
 * 0 1   one user (unused ungot is 1) or one scanf (unused ungot is 0)
 * 1 0   must be scanf[0] and user[1]
 * 1 1   illegal -- could be used to signal safe for setbuf
 *         but if used, need to fix _stdio_adjpos at least!
 */

#ifdef __UCLIBC__
#define __stdio_file_struct _UC_FILE
#endif

struct __stdio_file_struct {
	unsigned short modeflags;
	/* There could be a hole here, but modeflags is used most.*/
#ifdef __STDIO_WIDE
	/* TODO - ungot_width could be combined with ungot.  But what about hole? */
	unsigned char ungot_width[2]; /* 0 is current (building) char, 1 is scanf */
	wchar_t ungot[2];
#else  /* __STDIO_WIDE */
	unsigned char ungot[2];
#endif /* __STDIO_WIDE */
	int filedes;
#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
	struct __stdio_file_struct *nextopen;
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
#ifdef __STDIO_BUFFERS
	unsigned char *bufstart;	/* pointer to buffer */
	unsigned char *bufend;		/* pointer to 1 past end of buffer */
	unsigned char *bufpos;
	unsigned char *bufread;		/* pointer to 1 past last buffered read char. */
#ifdef __STDIO_GETC_MACRO
	unsigned char *bufgetc;		/* 1 past last readable by getc */
#endif /* __STDIO_GETC_MACRO */
#ifdef __STDIO_PUTC_MACRO
	unsigned char *bufputc;		/* 1 past last writeable by putc */
#endif /* __STDIO_PUTC_MACRO */
#endif /* __STDIO_BUFFERS */
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	void *cookie;
	_IO_cookie_io_functions_t gcs;
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
#ifdef __STDIO_MBSTATE
	__mbstate_t state;
#endif
#ifdef __STDIO_THREADSAFE
	int user_locking;
	pthread_mutex_t lock;
#endif
/* Everything after this is unimplemented... and may be trashed. */
#if __STDIO_BUILTIN_BUF_SIZE > 0
	unsigned char builtinbuf[__STDIO_BUILTIN_BUF_SIZE];
#endif /* __STDIO_BUILTIN_BUF_SIZE > 0 */
};


/***********************************************************************/

#define __MASK_UNGOT    	(0x0002|0x0001)
#define __MASK_UNGOT1    	0x0001
#define __MASK_UNGOT2    	0x0002
#define __FLAG_EOF			0x0004	/* EOF reached? */
#define __FLAG_ERROR		0x0008	/* stream in error state? */
#define __FLAG_WRITEONLY  	0x0010	/* unreadable */
#define __FLAG_READONLY  	0x0020	/* unwriteable */
#define __FLAG_FREEFILE		0x0040	/* free FILE struct after use */
#define __FLAG_NARROW       0x0080

#define __FLAG_FBF          0		/* convenience value */
#define __FLAG_LBF          0x0100
#define __FLAG_NBF          0x0200
#define __MASK_BUFMODE      0x0300

#define __FLAG_APPEND       0x0400
#define __FLAG_WIDE			0x0800

#define __FLAG_READING		0x1000
#define __FLAG_WRITING		0x2000

#define __FLAG_FREEBUF		0x4000	/* free buffer after use */
#define __FLAG_LARGEFILE    0x8000

/**********************************************************************/

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
extern __ssize_t _cs_read(void *cookie, char *buf, size_t bufsize);
extern __ssize_t _cs_write(void *cookie, const char *buf, size_t bufsize);
extern int _cs_seek(void *cookie, __offmax_t *pos, int whence);
extern int _cs_close(void *cookie);
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */

/**********************************************************************/

#ifdef __STDIO_MBSTATE
#define __COPY_MBSTATE(dest,src)  ((dest)->mask = (src)->mask, (dest)->wc = (src)->wc)
#define __INIT_MBSTATE(dest) ((dest)->mask = 0)
#else
#define __COPY_MBSTATE(dest,src)
#define __INIT_MBSTATE(dest)
#endif

/**********************************************************************/

/* TODO -- thread safety issues */
#define __CLEARERR(stream) \
	((stream)->modeflags &= ~(__FLAG_EOF|__FLAG_ERROR), (void)0)
#define __FEOF(stream)		((stream)->modeflags & __FLAG_EOF)
#define __FERROR(stream)	((stream)->modeflags & __FLAG_ERROR)

#define __FEOF_OR_FERROR(stream) \
	((stream)->modeflags & (__FLAG_EOF|__FLAG_ERROR))


/* TODO: check this
 * If we want to implement the getc and putc macros, we need to take
 * into account wide streams.  So... would need two additional variables
 * if we have wide streams (bufread and bufwrite), and one otherwise
 * (bufwrite).  getc would be effective for FBF streams.  It isn't for
 * LBF streams because other LBF streams need to be flushed.  putc
 * thouch is only effective for FBF streams.  Of course, to support
 * threads, we have to use functions.
 */

#ifdef __STDIO_GETC_MACRO
#define __GETC(stream)		( ((stream)->bufpos < (stream)->bufgetc) \
							? (*(stream)->bufpos++) \
							: fgetc_unlocked(stream) )
#else  /* __STDIO_GETC_MACRO */
#define __GETC(stream)		fgetc_unlocked(stream)
#endif /* __STDIO_GETC_MACRO */

#ifdef __STDIO_PUTC_MACRO
#define __PUTC(c, stream)	( ((stream)->bufpos < (stream)->bufputc) \
							? (*(stream)->bufpos++) = (c) \
							: fputc_unlocked((c),(stream)) )
#else  /* __STDIO_PUTC_MACRO */
#define __PUTC(c, stream)	fputc_unlocked(c, stream)
#endif /* __STDIO_PUTC_MACRO */


#if 0
/* TODO: disabled for now */
/* Masking macros for the above _are_ allowed by the standard. */
#define clearerr(stream)	__CLEARERR(stream)
#define feof(stream)		__FEOF(stream)
#define ferror(stream)		__FERROR(stream)
#endif

#if 0
/* TODO -- what about custom streams!!! */
/* Only use the macro below if you know fp is a valid FILE for a valid fd. */
#define __fileno(fp)	((fp)->filedes)
#endif

/**********************************************************************
 * PROTOTYPES OF INTERNAL FUNCTIONS
 **********************************************************************/

extern FILE *_stdio_openlist;

#ifdef __STDIO_THREADSAFE
extern pthread_mutex_t _stdio_openlist_lock;
extern void __stdio_init_mutex(pthread_mutex_t *m);
#endif

extern int _stdio_adjpos(FILE * __restrict stream, __offmax_t * pos);
extern int _stdio_lseek(FILE *stream, __offmax_t *pos, int whence);
/* TODO: beware of signals with _stdio_fwrite!!!! */
extern size_t _stdio_fwrite(const unsigned char *buffer, size_t bytes,
							  FILE *stream);
extern size_t _stdio_fread(unsigned char *buffer, size_t bytes,
							 FILE *stream);

extern FILE *_stdio_fopen(const char * __restrict filename,
							const char * __restrict mode,
							FILE * __restrict stream, int filedes);

extern FILE *_stdio_fsfopen(const char * __restrict filename,
							const char * __restrict mode,
							register FILE * __restrict stream);

extern void _stdio_init(void);
extern void _stdio_term(void);

#ifndef NDEBUG
extern void __stdio_validate_FILE(FILE *stream);
#else
#define __stdio_validate_FILE(stream)		((void)0)
#endif

#ifdef __STDIO_WIDE
extern size_t _wstdio_fwrite(const wchar_t *__restrict ws, size_t n,
							 register FILE *__restrict stream);
#endif

/**********************************************************************
 * UTILITY functions
 **********************************************************************/
#ifdef _STDIO_UTILITY

#include <features.h>
#include <limits.h>
#include <stdint.h>

#if INTMAX_MAX <= 2147483647L
#define __UIM_BUFLEN			12 /* 10 digits + 1 nul + 1 sign */
#elif INTMAX_MAX <= 9223372036854775807LL
#define __UIM_BUFLEN			22 /* 20 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for intmax_t!
#endif

#ifdef LLONG_MAX				/* --------------- */
#if LLONG_MAX <= 2147483647L
#define __UIM_BUFLEN_LLONG		12 /* 10 digits + 1 nul + 1 sign */
#elif LLONG_MAX <= 9223372036854775807LL
#define __UIM_BUFLEN_LLONG		22 /* 20 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for long long!
#endif
#endif /* ULLONG_MAX ----------------------------- */

#if LONG_MAX <= 2147483647L
#define __UIM_BUFLEN_LONG		12 /* 10 digits + 1 nul + 1 sign */
#elif LONG_MAX <= 9223372036854775807LL
#define __UIM_BUFLEN_LONG		22 /* 20 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for long!
#endif

#if INT_MAX <= 32767
#define __UIM_BUFLEN_INT		7 /* 10 digits + 1 nul + 1 sign */
#elif INT_MAX <= 2147483647L
#define __UIM_BUFLEN_INT		12 /* 10 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for int!
#endif

typedef enum {
	__UIM_DECIMAL = 0,
	__UIM_GROUP = ',',			/* Base 10 locale-dependent grouping. */
	__UIM_LOWER = 'a' - 10,
	__UIM_UPPER = 'A' - 10,
} __UIM_CASE;

/* Write a NULL-terminated list of "char *" args to file descriptor fd.
 * For an example of usage, see __assert.c.
 */
extern void _stdio_fdout(int fd, ...);

/* Convert the int val to a string in base abs(base).  val is treated as
 * an unsigned ??? int type if base > 0, and signed if base < 0.  This
 * is an internal function with _no_ error checking done unless assert()s
 * are enabled.
 *
 * Note: bufend is a pointer to the END of the buffer passed.
 * Call like this:
 *     char buf[SIZE], *p;
 *     p = _xltostr(buf + sizeof(buf) - 1, {unsigned int},  10, __UIM_DECIMAL)
 *     p = _xltostr(buf + sizeof(buf) - 1,          {int}, -10, __UIM_DECIMAL)
 *
 * WARNING: If base > 10, case _must_be_ either __UIM_LOWER or __UIM_UPPER
 *          for lower and upper case alphas respectively.
 * WARNING: If val is really a signed type, make sure base is negative!
 *          Otherwise, you could overflow your buffer.
 */
extern char *_uintmaxtostr(char * __restrict bufend, uintmax_t uval,
						   int base, __UIM_CASE alphacase);

/* TODO -- make this either a (possibly inline) function? */
#ifndef __BCC__
#define _int10tostr(bufend, intval) \
	_uintmaxtostr((bufend), (intval), -10, __UIM_DECIMAL)
#else  /* bcc doesn't do prototypes, we need to explicitly cast */
#define _int10tostr(bufend, intval) \
	_uintmaxtostr((bufend), (uintmax_t)(intval), -10, __UIM_DECIMAL)
#endif

#define __BUFLEN_INT10TOSTR		__UIM_BUFLEN_INT

#endif /* _STDIO_UTILITY */
/**********************************************************************/
/* uClibc translations */
/**********************************************************************/

/* TODO: note done above..  typedef struct __stdio_file_struct _UC_FILE; */
typedef __stdio_fpos_t		_UC_fpos_t;
#ifdef __STDIO_LARGE_FILES
typedef __stdio_fpos64_t	_UC_fpos64_t;
#endif

#define _UC_IOFBF		_STDIO_IOFBF /* Fully buffered.  */
#define _UC_IOLBF 		_STDIO_IOLBF /* Line buffered.  */
#define _UC_IONBF 		_STDIO_IONBF /* No buffering.  */

#define _UC_BUFSIZ		_STDIO_BUFSIZ
