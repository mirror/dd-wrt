/*  Copyright (C) 2002,2003,2004     Manuel Novoa III
 *  My stdio library for linux and (soon) elks.
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

/*  8-05-2002
 *  Changed fflush() behavior to no-op for r/w streams in read-mode.
 *     This falls under undefined behavior wrt ANSI/ISO C99, but
 *     SUSv3 seems to treat it as a no-op and it occurs in some apps.
 *  Fixed a problem with _stdio_fwrite() not checking for underlying
 *     write() failures.
 *  Fixed both _stdio_fwrite() and _stdio_fread() to make sure that
 *     the putc and getc macros were disabled if the stream was in
 *     and error state.
 *  The above changes should take care of a problem initially reported
 *  by "Steven J. Hill" <sjhill@realitydiluted.com>.
 *
 *  8-25-2002
 *  Changed fclose behavior when custom streams were enabled.  Previously,
 *     the cookie pointer was set to NULL as a debugging aid.  However,
 *     some of the perl 5.8 test rely on being able to close stderr and
 *     still try writing to it.  So now, the cookie pointer and handler
 *     function pointers are set to that it is a "normal" file with a
 *     file descriptor of -1.  Note: The cookie pointer is reset to NULL
 *     if the FILE struct is free'd by fclose.
 *
 *  Nov 21, 2002
 *  Added internal function _wstdio_fwrite.
 *  Jan 3, 2003
 *  Fixed a bug in _wstdio_fwrite.
 *
 *  Jan 22, 2003
 *  Fixed a bug related file position in append mode.  _stdio_fwrite now
 *     seeks to the end of the stream when append mode is set and we are
 *     transitioning to write mode, so that subsequent ftell() return
 *     values are correct.
 *  Also fix _stdio_fopen to support fdopen() with append specified when
 *     the underlying file didn't have O_APPEND set.  It now sets the
 *     O_APPEND flag as recommended by SUSv3 and is done by glibc.
 *
 *  May 15, 2003
 *  Modify __stdio_fread to deal with fake streams used by *sscanf.
 *  Set EOF to end of buffer when fmemopen used on a readonly stream.
 *    Note: I really need to run some tests on this to see what the
 *    glibc code does in each case.
 *
 * Sept 21, 2003
 * Modify _stdio_READ to conform with C99, as stdio input behavior upon
 *    encountering EOF changed with Defect Report #141.  In the current
 *    standard, the stream's EOF indicator is "sticky".  Once it is set,
 *    all further input from the stream should fail until the application
 *    explicitly clears the EOF indicator (clearerr(), file positioning),
 *    even if more data becomes available.
 * Fixed a bug in fgets.  Wasn't checking for read errors.
 * Minor thread locking optimizations to avoid some unnecessary locking.
 * Remove the explicit calls to __builtin_* funcs, as we really need to
 *    implement a more general solution.
 *
 * Nov 17, 2003
 * Fix the return value for fputs when passed an empty string.
 *
 * Jan 1, 2004
 * Fix __freadable and __fwritable... were using '~' instead of '!'. (ugh)
 * Fix (hopefully) a potential problem with failed freopen() calls.  The
 *   fix isn't tested since I've been working on the replacement stdio
 *   core code which will go in after the next release.
 */

/* Before we include anything, convert L_ctermid to L_ctermid_function
 * and undef L_ctermid if defined.  This is necessary as L_ctermid is
 * a SUSv3 standard macro defined in stdio.h. */
#ifdef L_ctermid
#define L_ctermid_function
#undef L_ctermid
#endif

#define _ISOC99_SOURCE			/* for ULLONG primarily... */
#define _GNU_SOURCE
#define _STDIO_UTILITY			/* for _stdio_fdout and _uintmaxtostr. */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef O_LARGEFILE		/* uClibc undefines this if no large file support. */
#ifdef __STDIO_LARGE_FILES
#error missing define for O_LARGEFILE!
#endif
#define O_LARGEFILE		0
#endif

/**********************************************************************/
/* First deal with some build issues... */

#ifndef __STDIO_THREADSAFE
/* Just build empty object files if any of these were defined. */
/* Note though that we do keep the various *_unlocked names as aliases. */
#undef L___fsetlocking
#undef L___flockfile
#undef L___ftrylockfile
#undef L___funlockfile
#endif

#ifndef __STDIO_LARGE_FILES
/* Just build empty object files if any of these were defined. */
#undef L_fopen64
#undef L_freopen64
#undef L_ftello64
#undef L_fseeko64
#undef L_fsetpos64
#undef L_fgetpos64
#endif

/**********************************************************************/

#ifndef __STDIO_THREADSAFE

#if defined(__BCC__) && 0
#define UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,STREAM) \
asm(".text\nexport _" "NAME" "_unlocked\n_" "NAME" "_unlocked = _" "NAME"); \
RETURNTYPE NAME PARAMS
#else
#define UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,STREAM) \
strong_alias(NAME,NAME##_unlocked) \
RETURNTYPE NAME PARAMS
#endif

#define UNLOCKED(RETURNTYPE,NAME,PARAMS,ARGS) \
	UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,stream)

#if defined(__BCC__) && 0
#define UNLOCKED_VOID_RETURN(NAME,PARAMS,ARGS) \
asm(".text\nexport _" "NAME" "_unlocked\n_" "NAME" "_unlocked = _" "NAME"); \
void NAME PARAMS
#else
#define UNLOCKED_VOID_RETURN(NAME,PARAMS,ARGS) \
strong_alias(NAME,NAME##_unlocked) \
void NAME PARAMS
#endif

#define __STDIO_THREADLOCK_OPENLIST
#define __STDIO_THREADUNLOCK_OPENLIST

#else  /* __STDIO_THREADSAFE */

#include <pthread.h>

#define UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,STREAM) \
RETURNTYPE NAME PARAMS \
{ \
	RETURNTYPE retval; \
	__STDIO_THREADLOCK(STREAM); \
	retval = NAME##_unlocked ARGS ; \
	__STDIO_THREADUNLOCK(STREAM); \
	return retval; \
} \
RETURNTYPE NAME##_unlocked PARAMS

#define UNLOCKED(RETURNTYPE,NAME,PARAMS,ARGS) \
	UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,stream)

#define UNLOCKED_VOID_RETURN(NAME,PARAMS,ARGS) \
void NAME PARAMS \
{ \
	__STDIO_THREADLOCK(stream); \
	NAME##_unlocked ARGS ; \
	__STDIO_THREADUNLOCK(stream); \
} \
void NAME##_unlocked PARAMS

#define __STDIO_THREADLOCK_OPENLIST \
	__pthread_mutex_lock(&_stdio_openlist_lock)

#define __STDIO_THREADUNLOCK_OPENLIST \
	__pthread_mutex_unlock(&_stdio_openlist_lock)

#define __STDIO_THREADTRYLOCK_OPENLIST \
	__pthread_mutex_trylock(&_stdio_openlist_lock)

#endif /* __STDIO_THREADSAFE */

/**********************************************************************/

#ifdef __STDIO_WIDE
#define __STDIO_FILE_INIT_UNGOT		{ 0, 0 }, { 0, 0 },
#else
#define __STDIO_FILE_INIT_UNGOT		{ 0, 0 },
#endif

#ifdef __STDIO_GETC_MACRO
#define __STDIO_FILE_INIT_BUFGETC(x) x,
#else
#define __STDIO_FILE_INIT_BUFGETC(x)
#endif

#ifdef __STDIO_PUTC_MACRO
#define __STDIO_FILE_INIT_BUFPUTC(x) x,
#else
#define __STDIO_FILE_INIT_BUFPUTC(x)
#endif

#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
#define __STDIO_FILE_INIT_NEXT(next)	(next),
#else  /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
#define __STDIO_FILE_INIT_NEXT(next)
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */

#ifdef __STDIO_BUFFERS
#define __STDIO_FILE_INIT_BUFFERS(buf,bufsize) \
	(buf), (buf)+(bufsize), (buf), (buf),
#else
#define __STDIO_FILE_INIT_BUFFERS(buf,bufsize)
#endif

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
#define __STDIO_FILE_INIT_CUSTOM_STREAM(stream) \
	&((stream).filedes), { _cs_read, _cs_write, NULL, _cs_close },
#else
#define __STDIO_FILE_INIT_CUSTOM_STREAM(stream)
#endif

#ifdef __STDIO_MBSTATE
#define __STDIO_FILE_INIT_MBSTATE \
	{ 0, 0 },
#else
#define __STDIO_FILE_INIT_MBSTATE
#endif


#ifdef __STDIO_THREADSAFE
#define __STDIO_FILE_INIT_THREADSAFE \
	0, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,
#else
#define __STDIO_FILE_INIT_THREADSAFE
#endif

#define __STDIO_INIT_FILE_STRUCT(stream, flags, filedes, next, buf, bufsize) \
	{ (flags), \
	__STDIO_FILE_INIT_UNGOT \
	(filedes), \
	__STDIO_FILE_INIT_NEXT(next) \
	__STDIO_FILE_INIT_BUFFERS(buf,bufsize) \
	__STDIO_FILE_INIT_BUFGETC((buf)) \
	__STDIO_FILE_INIT_BUFPUTC((buf)) \
	__STDIO_FILE_INIT_CUSTOM_STREAM(stream) \
	__STDIO_FILE_INIT_MBSTATE \
	__STDIO_FILE_INIT_THREADSAFE \
} /* TODO: mbstate and builtin buf */

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS

/* TODO -- what does glibc do for undefined funcs?  errno set? */
#define __READ(STREAMPTR,BUF,SIZE) \
	((((STREAMPTR)->gcs.read) == NULL) ? -1 : \
	(((STREAMPTR)->gcs.read)((STREAMPTR)->cookie,(BUF),(SIZE))))
#define __WRITE(STREAMPTR,BUF,SIZE) \
	((((STREAMPTR)->gcs.write) == NULL) ? -1 : \
	(((STREAMPTR)->gcs.write)((STREAMPTR)->cookie,(BUF),(SIZE))))
#define __CLOSE(STREAMPTR) \
	((((STREAMPTR)->gcs.close) == NULL) ? 0 : \
	(((STREAMPTR)->gcs.close)((STREAMPTR)->cookie)))

#else  /* __STDIO_GLIBC_CUSTOM_STREAMS */

#define __READ(STREAMPTR,BUF,SIZE) \
	(read((STREAMPTR)->filedes,(BUF),(SIZE)))
#define __WRITE(STREAMPTR,BUF,SIZE) \
	(write((STREAMPTR)->filedes,(BUF),(SIZE)))
#define __CLOSE(STREAMPTR) \
	(close((STREAMPTR)->filedes))

#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */

/**********************************************************************/
/* POSIX functions */
/**********************************************************************/
#ifdef L_getw

/* SUSv2 Legacy function -- need not be reentrant. */

int getw(FILE *stream)
{
	int aw[1];

#ifdef __STDIO_WIDE

	return (fread_unlocked((void *)aw, sizeof(int), 1, stream) > 0)
		? (*aw) : EOF;

#else  /* __STDIO_WIDE */

	return (_stdio_fread((unsigned char *)(aw), sizeof(int), stream)
			== sizeof(int)) ? (*aw) : EOF;

#endif /* __STDIO_WIDE */
}

#endif
/**********************************************************************/
#ifdef L_putw

/* SUSv2 Legacy function -- need not be reentrant. */

int putw(int w, FILE *stream)
{
	int aw[1];

	*aw = w;					/* In case 'w' is in a register... */

#ifdef __STDIO_WIDE

	return (fwrite_unlocked((void *)aw, sizeof(int), 1, stream) == 1)
		? 0 : EOF;

#else  /* __STDIO_WIDE */

	return (_stdio_fwrite((unsigned char *)aw, sizeof(int), stream)
			== sizeof(int)) ? 0 : EOF;

#endif /* __STDIO_WIDE */
}

#endif
/**********************************************************************/
#ifdef L_fileno

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,fileno,(register FILE *stream),(stream))
{
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	return ( (stream && (stream->cookie == &(stream->filedes)) && (stream->filedes >= 0))
			 ? stream->filedes
			 : (__set_errno(EBADF), -1) );
#else  /* __STDIO_GLIBC_CUSTOM_STREAMS */
	return ((stream && stream->filedes >= 0)) ? stream->filedes : (__set_errno(EBADF), -1);
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
}

#endif
/**********************************************************************/
#ifdef L_fdopen

/* No reentrancy issues. */

FILE *fdopen(int filedes, const char *mode)
{
	register char *cur_mode;	/* TODO -- use intptr_t?? (also fopencookie) */

	return (((int)(cur_mode = (char *) fcntl(filedes, F_GETFL))) != -1)
		? _stdio_fopen(cur_mode, mode, NULL, filedes) 
		: NULL;
}

#endif
/**********************************************************************/
#ifdef L_fopen64

/* No reentrancy issues. */

FILE *fopen64(const char * __restrict filename, const char * __restrict mode)
{
	return _stdio_fopen(filename, mode, NULL, -2);
}

#endif
/**********************************************************************/
#ifdef L_ctermid_function

/* Not required to be reentrant. */

char *ctermid(register char *s)
{
	static char sbuf[L_ctermid];

#ifdef __BCC__
	/* Currently elks doesn't support /dev/tty. */
	if (!s) {
		s = sbuf;
	}
	*s = 0;

	return s;
#else
	/* glibc always returns /dev/tty for linux. */
	return strcpy((s ? s : sbuf), "/dev/tty");
#endif
}

#endif
/**********************************************************************/
/* BSD functions */
/**********************************************************************/
#ifdef L_setbuffer

/* No reentrancy issues. */

void setbuffer(FILE * __restrict stream, register char * __restrict buf,
			   size_t size)
{
#ifdef __STDIO_BUFFERS
	setvbuf(stream, buf, (buf ? _IOFBF : _IONBF), size);
#else  /* __STDIO_BUFFERS */
	/* Nothing to do. */
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
#ifdef L_setlinebuf

/* No reentrancy issues. */

void setlinebuf(FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
	setvbuf(stream, NULL, _IOLBF, (size_t) 0);
#else  /* __STDIO_BUFFERS */
	/* Nothing to do. */
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
/* GLIBC functions */
/**********************************************************************/
#ifdef L_fcloseall

/* NOTE: GLIBC difference!!! -- fcloseall
 * According to the info pages, glibc actually fclose()s all open files.
 * Apparently, glibc's new version only fflush()s and unbuffers all
 * writing streams to cope with unordered destruction of c++ static
 * objects.  Here we implement the old behavior as default.
 */

/* Not reentrant. */

int fcloseall (void)
{
#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
	register FILE *stream;
	int rv;

	_stdio_term();				/* Let _stdio_term() do all the work. */

	rv = 0;
	for (stream = _stdio_openlist ; stream ; stream = stream->nextopen) {
		if (stream->modeflags & (__FLAG_WRITING|__FLAG_ERROR)) {
			/* TODO -- is this correct?  Maybe ferror set before flush...
			* could check if pending writable but what if term unbuffers?
			* in that case, could clear error flag... */
			rv = EOF;			/* Only care about failed writes. */
		}
	}

	/* Make sure _stdio_term() does nothing on exit. */
	_stdio_openlist = NULL;

	return rv;
#else  /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */

	return 0;

#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
}

#endif
/**********************************************************************/
#ifdef L_fmemopen
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS

typedef struct {
	size_t pos;
	size_t len;
	size_t eof;
	int dynbuf;
	unsigned char *buf;
	FILE *fp;
} __fmo_cookie;

#define COOKIE ((__fmo_cookie *) cookie)

static ssize_t fmo_read(register void *cookie, char *buf, size_t bufsize)
{
	size_t count = COOKIE->len - COOKIE->pos;

	/* Note: 0 < bufsize < SSIZE_MAX because of _stdio_READ. */
	if (!count) {				/* EOF! */
		return 0;
	}

	if (bufsize > count) {
		bufsize = count;
	}

#if 1							/* TODO - choose code option */
	memcpy(buf, COOKIE->buf + COOKIE->pos, bufsize);
	COOKIE->pos += bufsize;
#else
	{
		register char *p = COOKIE->buf + COOKIE->pos;

		count = bufsize;
		while (count) {
			*buf++ = *p++;
			--count;
		}
		COOKIE->pos += bufsize;
	}
#endif

	return bufsize;
}

static ssize_t fmo_write(register void *cookie, const char *buf, size_t bufsize)
{
	size_t count;

	/* Note: bufsize < SSIZE_MAX because of _stdio_WRITE. */

	/* If appending, need to seek to end of file!!!! */
	if (COOKIE->fp->modeflags & __FLAG_APPEND) {
		COOKIE->pos = COOKIE->eof;
	}

	count = COOKIE->len - COOKIE->pos;

	if (bufsize > count) {
		bufsize = count;
		if (count == 0) {		/* We're at the end of the buffer... */
			__set_errno(EFBIG);
			return -1;
		}
	}

#if 1							/* TODO - choose code option */
	memcpy(COOKIE->buf + COOKIE->pos, buf, bufsize);
	COOKIE->pos += bufsize;

	if (COOKIE->pos > COOKIE->eof) {
		COOKIE->eof = COOKIE->pos;
		if (bufsize < count) {	/* New eof and still room in buffer? */
			*(COOKIE->buf + COOKIE->pos) = 0;
		}
	}

#else
	{
		register char *p = COOKIE->buf + COOKIE->pos;
		size_t i = bufsize;

		while (i > 0) {
			*p++ = *buf++;
			--i;
		}
		COOKIE->pos += bufsize;

		if (COOKIE->pos > COOKIE->eof) {
			COOKIE->eof = COOKIE->pos;
			if (bufsize < count) {	/* New eof and still room in buffer? */
				*p = 0;
			}
		}
	}

#endif

	return bufsize;
}

/* glibc doesn't allow seeking, but it has in-buffer seeks... we don't. */
static int fmo_seek(register void *cookie, __offmax_t *pos, int whence)
{
	__offmax_t p = *pos;

	/* Note: fseek already checks that whence is legal, so don't check here
	 * unless debugging. */
	assert(((unsigned int) whence) <= 2);

	if (whence != SEEK_SET) {
		p += (whence == SEEK_CUR) ? COOKIE->pos : /* SEEK_END */ COOKIE->eof;
	}

	/* Note: glibc only allows seeking in the buffer.  We'll actually restrict
	 * to the data. */
	/* Check for offset < 0, offset > eof, or offset overflow... */
	if (((uintmax_t) p) > COOKIE->eof) {
		return -1;
	}

	COOKIE->pos = *pos = p;
	return 0;
}

static int fmo_close(register void *cookie)
{
	if (COOKIE->dynbuf) {
		free(COOKIE->buf);
	}
	free(cookie);
	return 0;
}

#undef COOKIE

static const cookie_io_functions_t _fmo_io_funcs = {
	fmo_read, fmo_write, fmo_seek, fmo_close
};

/* TODO: If we have buffers enabled, it might be worthwile to add a pointer
 * to the FILE in the cookie and have read, write, and seek operate directly
 * on the buffer itself (ie replace the FILE buffer with the cookie buffer
 * and update FILE bufstart, etc. whenever we seek). */

FILE *fmemopen(void *s, size_t len, const char *modes)
{
	FILE *fp;
	register __fmo_cookie *cookie;
	size_t i;

	if ((cookie = malloc(sizeof(__fmo_cookie))) != NULL) {
		cookie->len = len;
		cookie->eof = cookie->pos = 0; /* pos and eof adjusted below. */
		cookie->dynbuf = 0;
		if (((cookie->buf = s) == NULL) && (len > 0)) {
			if ((cookie->buf = malloc(len)) == NULL) {
				goto EXIT_cookie;
			}
			cookie->dynbuf = 1;
			*cookie->buf = 0;	/* If we're appending, treat as empty file. */
		}
		
#ifndef __BCC__
		fp = fopencookie(cookie, modes, _fmo_io_funcs);
#else
		fp = fopencookie(cookie, modes, &_fmo_io_funcs);
#endif
		/* Note: We don't need to worry about locking fp in the thread case
		 * as the only possible access would be a close or flush with
		 * nothing currently in the FILE's write buffer. */

		if (fp != NULL) {
			cookie->fp = fp;
			if (fp->modeflags & __FLAG_READONLY) {
				cookie->eof = len;
			}
			if ((fp->modeflags & __FLAG_APPEND) && (len > 0)) {
				for (i = 0 ; i < len ; i++) {
					if (cookie->buf[i] == 0) {
						break;
					}
				}
				cookie->eof = cookie->pos = i; /* Adjust eof and pos. */
			}
			return fp;
		}
	}

	if (!s) {
		free(cookie->buf);
	}
 EXIT_cookie:
	free(cookie);

	return NULL;
}

#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
#endif
/**********************************************************************/
#ifdef L_open_memstream
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS

#define COOKIE ((__oms_cookie *) cookie)

typedef struct {
	char *buf;
	size_t len;
	size_t pos;
	size_t eof;
	char **bufloc;
	size_t *sizeloc;
} __oms_cookie;

/* Nothing to do here, as memstreams are write-only. */
/*  static ssize_t oms_read(void *cookie, char *buf, size_t bufsize) */
/*  { */
/*  } */

static ssize_t oms_write(register void *cookie, const char *buf, size_t bufsize)
{
	register char *newbuf;
	size_t count;

	/* Note: we already know bufsize < SSIZE_MAX... */

	count = COOKIE->len - COOKIE->pos - 1;
	assert(COOKIE->pos < COOKIE->len); /* Always nul-terminate! */

	if (bufsize > count) {
		newbuf = realloc(COOKIE->buf, COOKIE->len + bufsize - count);
		if (newbuf) {
			*COOKIE->bufloc = COOKIE->buf = newbuf;
			COOKIE->len += (bufsize - count);
		} else {
			bufsize = count;
			if (count == 0) {
				__set_errno(EFBIG);	/* TODO: check glibc errno setting... */
				return -1;
			}
		}
	}

	memcpy(COOKIE->buf + COOKIE->pos, buf, bufsize);
	COOKIE->pos += bufsize;

	if (COOKIE->pos > COOKIE->eof) {
		*COOKIE->sizeloc = COOKIE->eof = COOKIE->pos;
		COOKIE->buf[COOKIE->eof] = 0; /* Need to nul-terminate. */
	}

	return bufsize;
}

static int oms_seek(register void *cookie, __offmax_t *pos, int whence)
{
	__offmax_t p = *pos;
	register char *buf;
	size_t leastlen;

	/* Note: fseek already checks that whence is legal, so don't check here
	 * unless debugging. */
	assert(((unsigned int) whence) <= 2);

	if (whence != SEEK_SET) {
		p += (whence == SEEK_CUR) ? COOKIE->pos : /* SEEK_END */ COOKIE->eof;
	}

	/* Note: glibc only allows seeking in the buffer.  We'll actually restrict
	 * to the data. */
	/* Check for offset < 0, offset >= too big (need nul), or overflow... */
	if (((uintmax_t) p) >= SIZE_MAX - 1) {
		return -1;
	}

	leastlen = ((size_t) p) + 1; /* New pos + 1 for nul if necessary. */

	if (leastlen >= COOKIE->len) { /* Need to grow buffer... */
		buf = realloc(COOKIE->buf, leastlen);
		if (buf) {
			*COOKIE->bufloc = COOKIE->buf = buf;
			COOKIE->len = leastlen;
			memset(buf + COOKIE->eof, leastlen - COOKIE->eof, 0); /* 0-fill */
		} else {
			/* TODO: check glibc errno setting... */
			return -1;
		}
	}

	*pos = COOKIE->pos = --leastlen;

	if (leastlen > COOKIE->eof) {
		memset(COOKIE->buf + COOKIE->eof, leastlen - COOKIE->eof, 0);
		*COOKIE->sizeloc = COOKIE->eof;
	}

	return 0;
}

static int oms_close(void *cookie)
{
	free(cookie);
	return 0;
}

#undef COOKIE

static const cookie_io_functions_t _oms_io_funcs = {
	NULL, oms_write, oms_seek, oms_close
};

/* TODO: If we have buffers enabled, it might be worthwile to add a pointer
 * to the FILE in the cookie and operate directly on the buffer itself
 * (ie replace the FILE buffer with the cookie buffer and update FILE bufstart,
 * etc. whenever we seek). */

FILE *open_memstream(char **__restrict bufloc, size_t *__restrict sizeloc)
{
	register __oms_cookie *cookie;
	register FILE *fp;

	if ((cookie = malloc(sizeof(__oms_cookie))) != NULL) {
		if ((cookie->buf = malloc(cookie->len = BUFSIZ)) == NULL) {
			goto EXIT_cookie;
		}
		*cookie->buf = 0;		/* Set nul terminator for buffer. */
		*(cookie->bufloc = bufloc) = cookie->buf;
		*(cookie->sizeloc = sizeloc) = cookie->eof = cookie->pos = 0;
		
#ifndef __BCC__
		fp = fopencookie(cookie, "w", _oms_io_funcs);
#else
		fp = fopencookie(cookie, "w", &_oms_io_funcs);
#endif
		/* Note: We don't need to worry about locking fp in the thread case
		 * as the only possible access would be a close or flush with
		 * nothing currently in the FILE's write buffer. */

		if (fp != NULL) {
			return fp;
		}
	}

	if (cookie->buf != NULL) {
		free(cookie->buf);
	}
 EXIT_cookie:
	free(cookie);

	return NULL;
}

#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
#endif
/**********************************************************************/
#ifdef L_fopencookie
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS

/* NOTE: GLIBC difference!!! -- fopencookie
 * According to the info pages, glibc allows seeking within buffers even if
 * no seek function is supplied.  We don't. */

/* NOTE: GLIBC difference!!! -- fopencookie
 * When compiled without large file support, the offset pointer for the
 * cookie_seek function is off_t * and not off64_t * as for glibc. */

/* Currently no real reentrancy issues other than a possible double close(). */

#ifndef __BCC__

FILE *fopencookie(void * __restrict cookie, const char * __restrict mode,
				  cookie_io_functions_t io_functions)
{
	FILE *stream;

	/* Fake an fdopen guaranteed to pass the _stdio_fopen basic agreement
	 * check without an fcntl call. */
	if ((stream = _stdio_fopen(((char *)(INT_MAX-1)),
							   mode, NULL, INT_MAX)) /* TODO: use intptr_t? */
		!= NULL
		) {
		stream->filedes = -1;
		stream->gcs = io_functions;
		stream->cookie = cookie;
	}

#if !defined(__STDIO_BUFFERS) && !defined(__STDIO_THREADSAFE)
	/* I we don't have buffers or threads, we only need to worry about
	 * custom streams on the open list, as no flushing is necessary and
	 * no locking of possible underlying normal streams need be done.
	 * We do need to explicitly close custom streams on termination of stdio,
	 * and we need to lock the list as it can be modified by fclose(). */
	__STDIO_THREADLOCK_OPENLIST;
	stream->nextopen = _stdio_openlist;	/* New files are inserted at */
	_stdio_openlist = stream;			/*   the head of the list. */
	__STDIO_THREADUNLOCK_OPENLIST;
#endif /* !defined(__STDIO_BUFFERS) && !defined(__STDIO_THREADSAFE) */

	return stream;
}

#else  /* __BCC__ */

/* NOTE: GLIBC difference!!! -- fopencookie (bcc only)
 * Since bcc doesn't support passing of structs, we define fopencookie as a
 * macro in terms of _fopencookie which takes a struct * for the io functions
 * instead.
 */

FILE *_fopencookie(void * __restrict cookie, const char * __restrict mode,
				   register cookie_io_functions_t *io_functions)
{
	register FILE *stream;

	/* Fake an fdopen guaranteed to pass the _stdio_fopen basic agreement
	 * check without an fcntl call. */
	if ((stream = _stdio_fopen(((char *)(INT_MAX-1)),
							   mode, NULL, INT_MAX)) /* TODO: use intptr_t? */
		!= NULL
		) {
		stream->filedes = -1;
		stream->gcs.read  = io_functions->read;
		stream->gcs.write = io_functions->write;
		stream->gcs.seek  = io_functions->seek;
		stream->gcs.close = io_functions->close;
		stream->cookie = cookie;
	}

#if !defined(__STDIO_BUFFERS) && !defined(__STDIO_THREADSAFE)
	/* I we don't have buffers or threads, we only need to worry about
	 * custom streams on the open list, as no flushing is necessary and
	 * no locking of possible underlying normal streams need be done.
	 * We do need to explicitly close custom streams on termination of stdio,
	 * and we need to lock the list as it can be modified by fclose(). */
	__STDIO_THREADLOCK_OPENLIST;
	stream->nextopen = _stdio_openlist;	/* New files are inserted at */
	_stdio_openlist = stream;			/*   the head of the list. */
	__STDIO_THREADUNLOCK_OPENLIST;
#endif /* !defined(__STDIO_BUFFERS) && !defined(__STDIO_THREADSAFE) */

	return stream;
}

#endif  /* __BCC__ */

#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
#endif
/**********************************************************************/
#ifdef L___fbufsize

/* Not reentrant. */

size_t __fbufsize(register FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
	return (stream->modeflags & __FLAG_NBF)
		? 0 : (stream->bufend - stream->bufstart);
#else  /* __STDIO_BUFFERS */
	return 0;
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
#ifdef L___freading

/* No reentrancy issues. */

int __freading(FILE * __restrict stream)
{
	return stream->modeflags & (__FLAG_READING|__FLAG_READONLY);
}

#endif
/**********************************************************************/
#ifdef L___fwriting

/* No reentrancy issues. */

int __fwriting(FILE * __restrict stream)
{
	return stream->modeflags & (__FLAG_WRITING|__FLAG_WRITEONLY);
}

#endif
/**********************************************************************/
#ifdef L___freadable

/* No reentrancy issues. */

int __freadable(FILE * __restrict stream)
{
	return !(stream->modeflags & __FLAG_WRITEONLY);
}

#endif
/**********************************************************************/
#ifdef L___fwritable

/* No reentrancy issues. */

int __fwritable(FILE * __restrict stream)
{
	return !(stream->modeflags & __FLAG_READONLY);
}

#endif
/**********************************************************************/
#ifdef L___flbf

/* No reentrancy issues. */

int __flbf(FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
	return (stream->modeflags & __FLAG_LBF);
#else  /* __STDIO_BUFFERS */
	/* TODO -- Even though there is no buffer, return flag setting? */
	return __FLAG_NBF;
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
#ifdef L___fpurge

/* Not reentrant. */

void __fpurge(register FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
#ifdef __STDIO_PUTC_MACRO
	stream->bufputc =			/* Must disable putc. */
#endif /* __STDIO_PUTC_MACRO */
#ifdef __STDIO_GETC_MACRO
	stream->bufgetc =			/* Must disable getc. */
#endif
	stream->bufpos = stream->bufread = stream->bufstart; /* Reset pointers. */
#endif /* __STDIO_BUFFERS */
	/* Reset r/w flags and clear ungots. */
	stream->modeflags &= ~(__FLAG_READING|__FLAG_WRITING|__MASK_UNGOT);
}

#endif
/**********************************************************************/
#ifdef L___fpending

/* Not reentrant. */

#ifdef __STDIO_WIDE
#warning Unlike the glibc version, this __fpending returns bytes in buffer for wide streams too!

link_warning(__fpending, "This version of __fpending returns bytes remaining in buffer for both narrow and wide streams.  glibc's version returns wide chars in buffer for the wide stream case.")

#endif  /* __STDIO_WIDE */

size_t __fpending(register FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
	/* TODO -- should we check this?  should we set errno?  just assert? */
	return (stream->modeflags & (__FLAG_READING|__FLAG_READONLY))
		? 0 : (stream->bufpos - stream->bufstart);
#else  /* __STDIO_BUFFERS */
	return 0;
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
#ifdef L__flushlbf

/* No reentrancy issues. */

void _flushlbf(void)
{
#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
	fflush((FILE *) &_stdio_openlist); /* Uses an implementation hack!!! */
#else  /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
	/* Nothing to do. */
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
}

#endif
/**********************************************************************/
#ifdef L___fsetlocking

/* NOT threadsafe!!!  (I don't think glibc's is either)
 *
 * This interacts badly with internal locking/unlocking.  If you use this routine,
 * make sure the file isn't being accessed by any other threads.  Typical use would
 * be to change to user locking immediately after opening the stream.
 */

#ifdef __UCLIBC_MJN3_ONLY__
link_warning(__fsetlocking, "Oddly enough, __fsetlocking() is NOT threadsafe.")
#endif

int __fsetlocking(FILE *stream, int locking_mode)
{
#ifdef __STDIO_THREADSAFE
	int old_mode;
#endif

	assert((FSETLOCKING_QUERY == 0) && (FSETLOCKING_INTERNAL == 1)
		   && (FSETLOCKING_BYCALLER == 2));

	assert(((unsigned int) locking_mode) <= 2);

#ifdef __STDIO_THREADSAFE
	old_mode = stream->user_locking;

	assert(((unsigned int) old_mode) <= 1);	/* Must be 0 (internal) or 1 (user). */

	if (locking_mode != FSETLOCKING_QUERY) {
		/* In case we're not debugging, treat any unknown as a request to
		 * set internal locking, in order to match glibc behavior. */
		stream->user_locking = (locking_mode == FSETLOCKING_BYCALLER);
	}

	return 2 - old_mode;
#else
	return FSETLOCKING_BYCALLER; /* Well, without thread support... */
#endif
}

#endif
/**********************************************************************/
#ifdef L_flockfile

void flockfile(FILE *stream)
{
#ifdef __STDIO_THREADSAFE
	__pthread_mutex_lock(&stream->lock);
#endif
}

#endif
/**********************************************************************/
#ifdef L_ftrylockfile

int ftrylockfile(FILE *stream)
{
#ifdef __STDIO_THREADSAFE
	return __pthread_mutex_trylock(&stream->lock);
#else
	return 1;
#endif
}

#endif
/**********************************************************************/
#ifdef L_funlockfile

void funlockfile(FILE *stream)
{
#ifdef __STDIO_THREADSAFE
	__pthread_mutex_unlock(&stream->lock);
#endif
}

#endif
/**********************************************************************/
#ifdef L_getline

ssize_t getline(char **__restrict lineptr, size_t *__restrict n,
				FILE *__restrict stream)
{
	return __getdelim(lineptr, n, '\n', stream);
}

#endif
/**********************************************************************/
#ifdef L_getdelim

weak_alias(__getdelim,getdelim);

#define GETDELIM_GROWBY		64

ssize_t __getdelim(char **__restrict lineptr, size_t *__restrict n,
				   int delimiter, register FILE *__restrict stream)
{
	register char *buf;
	size_t pos;
	int c;

	if (!lineptr || !n || !stream) { /* Be compatable with glibc... even */
		__set_errno(EINVAL);	/* though I think we should assert here */
		return -1;				/* if anything. */
	}

	if (!(buf = *lineptr)) {	/* If passed NULL for buffer, */
		*n = 0;					/* ignore value passed and treat size as 0. */
	}
	pos = 1;			/* Make sure we have space for terminating nul. */

	__STDIO_THREADLOCK(stream);

	do {
		if (pos >= *n) {
			if (!(buf = realloc(buf, *n + GETDELIM_GROWBY))) {
				__set_errno(ENOMEM); /* Emulate old uClibc implementation. */
				break;
			}
			*n += GETDELIM_GROWBY;
			*lineptr = buf;
		}
	} while (((c = (getc_unlocked)(stream)) != EOF)	/* Disable the macro */
			 && ((buf[pos++ - 1] = c) != delimiter));

	__STDIO_THREADUNLOCK(stream);

	if (--pos) {
		buf[pos] = 0;
		return pos;
	}

	return -1;		/* Either initial realloc failed or first read was EOF. */
}

#endif
/**********************************************************************/
/* my extension functions */
/**********************************************************************/
#ifdef L__stdio_fsfopen
/*
 * Stack|Static File open -- open a file where the FILE is either
 * stack or staticly allocated.
 */

/* No reentrancy issues. */

FILE *_stdio_fsfopen(const char * __restrict filename,
					 const char * __restrict mode,
					 register FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
	stream->modeflags = __FLAG_FBF;
#if __STDIO_BUILTIN_BUF_SIZE > 0
	stream->bufstart = stream->builtinbuf;
	stream->bufend = stream->builtinbuf + sizeof(stream->builtinbuf);
#else  /* __STDIO_BUILTIN_BUF_SIZE > 0 */
	stream->bufend = stream->bufstart = NULL;
#endif /* __STDIO_BUILTIN_BUF_SIZE > 0 */
#endif /* __STDIO_BUFFERS */

	return _stdio_fopen(filename, mode, stream, -1);
}
#endif
/**********************************************************************/
/* stdio internal functions */
/**********************************************************************/
#ifdef L__stdio_adjpos

/* According to the ANSI/ISO C99 definition of ungetwc()
 *     For a text or binary stream, the value of its file position indicator
 *     after a successful call to the ungetwc function is unspecified until
 *     all pushed­back wide characters are read or discarded.
 * Note however, that this applies only to _user_ calls to ungetwc.  We
 * need to allow for internal calls by scanf.


 *  So we store the byte count
 * of the first ungot wide char in ungot_width.  If it is 0 (user case)
 * then the file position is treated as unknown.
 */

/* Internal function -- not reentrant. */

int _stdio_adjpos(register FILE * __restrict stream, register __offmax_t *pos)
{
	__offmax_t r;
	int cor = stream->modeflags & __MASK_UNGOT;	/* handle ungots */

	assert(cor <= 2);

#ifdef __STDIO_WIDE
	/* Assumed narrow stream so correct if wide. */
	if (cor && (stream->modeflags & __FLAG_WIDE)) {
		if ((((stream->modeflags & __MASK_UNGOT) > 1) || stream->ungot[1])) {
			return -1; /* App did ungetwc, so position is indeterminate. */
		}
		if (stream->modeflags & __MASK_UNGOT) {
			cor = cor - 1 + stream->ungot_width[1];
		}
		if (stream->state.mask > 0) { /* Incomplete character (possible bad) */
			cor -= stream->ungot_width[0];
		}
	}
#endif /* __STDIO_WIDE */

#ifdef __STDIO_BUFFERS
	if (stream->modeflags & __FLAG_WRITING) {
		cor -= (stream->bufpos - stream->bufstart); /* pending writes */
	}
	if (stream->modeflags & __FLAG_READING) {
		cor += (stream->bufread - stream->bufpos); /* extra's read */
	}
#endif /* __STDIO_BUFFERS */

	r = *pos;
	return ((*pos -= cor) > r) ? -cor : cor;
}

#endif
/**********************************************************************/
#ifdef L__stdio_lseek
/*
 * This function is only called by fseek and ftell.
 * fseek -- doesn't care about pos val, just success or failure.
 * ftell -- needs pos val but offset == 0 and whence == SET_CUR.
 */

/* Internal function -- not reentrant. */

int _stdio_lseek(register FILE *stream, register __offmax_t *pos, int whence)
{
	__offmax_t res;

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	if (stream->cookie != &stream->filedes) {
		return (((stream->gcs.seek == NULL)
				 || ((stream->gcs.seek)(stream->cookie, pos, whence) < 0))
				? -1 : 0);
	}
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
#ifdef __STDIO_LARGE_FILES
	res = lseek64(stream->filedes, *pos, whence);
#else
	res = lseek(stream->filedes, *pos, whence);
#endif /* __STDIO_LARGE_FILES */
	return (res >= 0) ? ((*pos = res), 0) : -1;
}

#endif
/**********************************************************************/
#ifdef L__stdio_fread
/*
 * NOTE!!! This routine is meant to be callable by both narrow and wide
 * functions.  However, if called by a wide function, there must be
 * NO pending ungetwc()s!!!
 */

/* Unlike write, it's ok for read to return fewer than bufsize, since
 * we may not need all of them. */
static ssize_t _stdio_READ(register FILE *stream, unsigned char *buf, size_t bufsize)
{
	ssize_t rv;

	/* NOTE: C99 change: Input fails once the stream's EOF indicator is set. */
	if ((bufsize == 0) || (stream->modeflags & __FLAG_EOF)) {
		return 0;
	}

	if (bufsize > SSIZE_MAX) {
		bufsize = SSIZE_MAX;
	}

#ifdef __BCC__
 TRY_READ:
#endif
	rv = __READ(stream, buf, bufsize);
	if (rv > 0) {
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
		assert(rv <= bufsize);	/* buggy user handler... TODO: check? */
		if (rv > bufsize) {	/* Num reported written > number requested */
			rv = bufsize;		/* Treat as a full read??? */
		}
#endif
	} else if (rv == 0) {
		stream->modeflags |= __FLAG_EOF;
	} else {
#ifdef __BCC__
		if (errno == EINTR) {
			goto TRY_READ;
		}
#endif
		stream->modeflags |= __FLAG_ERROR;
		rv = 0;
	}

	return rv;
}

/* Internal function -- not reentrant. */

size_t _stdio_fread(unsigned char *buffer, size_t bytes, register FILE *stream)
{
	__stdio_validate_FILE(stream); /* debugging only */

#ifdef __STDIO_BUFFERS

	if (stream->modeflags 
#ifdef __STDIO_AUTO_RW_TRANSITION
		& (__FLAG_WRITEONLY)
#else  /* __STDIO_AUTO_RW_TRANSITION */
		/* ANSI/ISO and SUSv3 require not currently writing. */
		& (__FLAG_WRITEONLY|__FLAG_WRITING)
#endif /* __STDIO_AUTO_RW_TRANSITION */
		) {
#ifdef __STDIO_PUTC_MACRO
		stream->bufputc = stream->bufstart;	/* Must disable putc. */
#endif /* __STDIO_PUTC_MACRO */
		stream->modeflags |= __FLAG_ERROR;
		/* TODO: This is for posix behavior if writeonly.  To save space, we
		 * use this errno for read attempt while writing, as no errno is
		 * specified by posix for this case, even though the restriction is
		 * mentioned in fopen(). */
		__set_errno(EBADF);
		return 0;
	}

	/* We need to disable putc and getc macros in case of error */
#if defined(__STDIO_PUTC_MACRO) || defined(__STDIO_GETC_MACRO)
#ifdef __STDIO_PUTC_MACRO
	stream->bufputc =
#endif /* __STDIO_GETC_MACRO */
#ifdef __STDIO_GETC_MACRO
	stream->bufgetc =
#endif /* __STDIO_GETC_MACRO */
	stream->bufstart;
#endif /*  defined(__STDIO_PUTC_MACRO) || defined(__STDIO_GETC_MACRO) */

	if (stream->modeflags & __MASK_BUFMODE) {
		/* If the stream is readable and not fully buffered, we must first
		 * flush all line buffered output streams.  Do this before the
		 * error check as this may be a read/write line-buffered stream.
		 * Note: Uses an implementation-specific hack!!! */
		fflush_unlocked((FILE *) &_stdio_openlist);
	}

#ifdef __STDIO_AUTO_RW_TRANSITION
	if ((stream->modeflags & __FLAG_WRITING)
		&& (fflush_unlocked(stream) == EOF)
		) {
		return 0;				/* Fail if we need to fflush but can't. */
	}
#endif /* __STDIO_AUTO_RW_TRANSITION */

	stream->modeflags |= __FLAG_READING; /* Make sure Reading flag is set. */

	{
		register unsigned char *p = (unsigned char *) buffer;

		/* First, grab appropriate ungetc() chars.  NOT FOR WIDE ORIENTATED! */
		while (bytes && (stream->modeflags & __MASK_UNGOT)) {
#ifdef __STDIO_WIDE
			assert(stream->modeflags & __FLAG_NARROW);
#endif /* __STDIO_WIDE */
			*p++ = stream->ungot[(--stream->modeflags) & __MASK_UNGOT];
			stream->ungot[1] = 0;
			--bytes;
		}

		/* Now get any other needed chars from the buffer or the file. */
	FROM_BUF:
		while (bytes && (stream->bufpos < stream->bufread)) {
			--bytes;
			*p++ = *stream->bufpos++;
		}

		if (bytes > 0) {
			ssize_t len;

			if (stream->filedes == -2) {
				stream->modeflags |= __FLAG_EOF;
				goto DONE;
			}

			/* The buffer is exhausted, but we still need chars.  */
			stream->bufpos = stream->bufread = stream->bufstart;

			if (bytes <= stream->bufend - stream->bufread) {
				/* We have sufficient space in the buffer. */
				len = _stdio_READ(stream, stream->bufread,
								  stream->bufend - stream->bufread);
				if (len > 0) {
					stream->bufread += len;
					goto FROM_BUF;
				}
			} else {
				/* More bytes needed than fit in the buffer, so read */
				/* directly into caller's buffer. */
				len = _stdio_READ(stream, p, bytes);
				if (len > 0) {
					p += len;
					bytes -= len;
					goto FROM_BUF; /* Redundant work, but stops extra read. */
				}
			}
		}

#ifdef __STDIO_GETC_MACRO
		if (!(stream->modeflags
			  & (__FLAG_WIDE|__MASK_UNGOT|__MASK_BUFMODE|__FLAG_ERROR))
			) {
			stream->bufgetc = stream->bufread; /* Enable getc macro. */
		}
#endif

	DONE:
		__stdio_validate_FILE(stream); /* debugging only */
		return (p - (unsigned char *)buffer);
	}

#else  /* __STDIO_BUFFERS --------------------------------------- */

	if (stream->modeflags 
#ifdef __STDIO_AUTO_RW_TRANSITION
		& (__FLAG_WRITEONLY)
#else  /* __STDIO_AUTO_RW_TRANSITION */
		/* ANSI/ISO and SUSv3 require not currently writing. */
		& (__FLAG_WRITEONLY|__FLAG_WRITING)
#endif /* __STDIO_AUTO_RW_TRANSITION */
		) {
		stream->modeflags |= __FLAG_ERROR;
		/* TODO: This is for posix behavior if writeonly.  To save space, we
		 * use this errno for read attempt while writing, as no errno is
		 * specified by posix for this case, even though the restriction is
		 * mentioned in fopen(). */
		__set_errno(EBADF);
		return 0;
	}

#ifdef __STDIO_AUTO_RW_TRANSITION
	stream->modeflags &= ~(__FLAG_WRITING);	/* Make sure Writing flag clear. */
#endif /* __STDIO_AUTO_RW_TRANSITION */

	stream->modeflags |= __FLAG_READING; /* Make sure Reading flag is set. */

	{
		register unsigned char *p = (unsigned char *) buffer;

		/* First, grab appropriate ungetc() chars.  NOT FOR WIDE ORIENTATED! */
		while (bytes && (stream->modeflags & __MASK_UNGOT)) {
#ifdef __STDIO_WIDE
			assert(stream->modeflags & __FLAG_NARROW);
#endif /* __STDIO_WIDE */
			*p++ = stream->ungot[(--stream->modeflags) & __MASK_UNGOT];
			stream->ungot[1] = 0;
			--bytes;
		}	

		while (bytes > 0) {
			ssize_t len = _stdio_READ(stream, p, (unsigned) bytes);
			if (len == 0) {
				break;
			}
			p += len;
			bytes -= len;
		}

		__stdio_validate_FILE(stream); /* debugging only */
		return (p - (unsigned char *)buffer);
	}

#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
#ifdef L__stdio_fwrite
/*
 * If buffer == NULL, attempt to fflush and return number of chars
 * remaining in buffer (0 if successful fflush).
 */

/* WARNING!!!! Current standards say that termination due to an asyncronous
 * signal may not result in stdio streams being flushed.  This libary makes
 * an effort to do so but there is no way, short of blocking signals for
 * each _stdio_fwrite call, that we can maintain the correct state if a
 * signal is recieved mid-call.  So any stream in mid-_stdio_fwrite could
 * not some flush data or even duplicate-flush some data.  It is possible
 * to avoid the duplicate-flush case by setting/clearing the stream
 * error flag before/after the write process, but it doesn't seem worth
 * the trouble. */

/* Like standard write, but always does a full write unless error, plus
 * deals correctly with bufsize > SSIZE_MAX... not much on an issue on linux
 * but definitly could be on Elks.  Also on Elks, always loops for EINTR..
 * Returns number of bytes written, so a short write indicates an error */
static size_t _stdio_WRITE(register FILE *stream,
						   register const unsigned char *buf, size_t bufsize)
{
	size_t todo;
	ssize_t rv, stodo;

	todo = bufsize;

	while (todo) {
		stodo = (todo <= SSIZE_MAX) ? todo : SSIZE_MAX;
		rv = __WRITE(stream, buf, stodo);
		if (rv >= 0) {
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
			assert(rv <= stodo);	/* buggy user handler... TODO: check? */
			if (rv > stodo) {	/* Num reported written > number requested */
				rv = stodo;		/* Treat as a full write??? */
			}
#endif
			todo -= rv;
			buf += rv;
		} else
#ifdef __BCC__
			if (errno != EINTR)
#endif
		{
			stream->modeflags |= __FLAG_ERROR;
			break;
		}
	}

	return bufsize - todo;
}

/* Internal function -- not reentrant. */

size_t _stdio_fwrite(const unsigned char *buffer, size_t bytes,
					 register FILE *stream)
{
#ifdef __STDIO_BUFFERS
	register const unsigned char *p;

	__stdio_validate_FILE(stream); /* debugging only */

	if ((stream->modeflags & __FLAG_READONLY)
#ifndef __STDIO_AUTO_RW_TRANSITION
	/* ANSI/ISO requires either at EOF or currently not reading. */
		|| ((stream->modeflags & (__FLAG_READING|__FLAG_EOF))
			== __FLAG_READING)
#endif /* __STDIO_AUTO_RW_TRANSITION */
		) {
		stream->modeflags |= __FLAG_ERROR;
		/* TODO: This is for posix behavior if readonly.  To save space, we
		 * use this errno for write attempt while reading, as no errno is
		 * specified by posix for this case, even though the restriction is
		 * mentioned in fopen(). */
		__set_errno(EBADF);
		return 0;
	}

#ifdef __STDIO_AUTO_RW_TRANSITION
	/* If reading, deal with ungots and read-buffered chars. */
	if (stream->modeflags & __FLAG_READING) {
		if (((stream->bufpos < stream->bufread)
			 || (stream->modeflags & __MASK_UNGOT))
			/* If appending, we might as well seek to end to save a seek. */
			/* TODO: set EOF in fseek when appropriate? */
			&& fseek(stream, 0L, 
					 ((stream->modeflags & __FLAG_APPEND)
					  ? SEEK_END : SEEK_CUR))
			) {
			/* Note: This differs from glibc's apparent behavior of
			   not setting the error flag and discarding the buffered
			   read data. */
			stream->modeflags |= __FLAG_ERROR; /* fseek may not set this. */
			return 0;			/* Fail if we need to fseek but can't. */
		}
		/* Always reset even if fseek called (saves a test). */
#ifdef __STDIO_GETC_MACRO
		stream->bufgetc =
#endif /* __STDIO_GETC_MACRO */
		stream->bufpos = stream->bufread = stream->bufstart;
	} else
#endif
	if ((stream->modeflags & (__FLAG_WRITING|__FLAG_APPEND)) == __FLAG_APPEND) {
		/* Append mode, but not currently writing.  Need to seek to end for proper
		 * ftell() return values.  Don't worry if the stream isn't seekable. */
		__offmax_t pos[1];
		*pos = 0;
		if (_stdio_lseek(stream, pos, SEEK_END) && (errno != EPIPE)) { /* Too big? */
			stream->modeflags |= __FLAG_ERROR;
			return 0;
		}
	}

#ifdef __STDIO_PUTC_MACRO
	/* We need to disable putc macro in case of error */
	stream->bufputc = stream->bufstart;
#endif /* __STDIO_GETC_MACRO */

	/* Clear both reading and writing flags.  We need to clear the writing
	 * flag in case we're fflush()ing or in case of an error. */
	stream->modeflags &= ~(__FLAG_READING|__FLAG_WRITING);

	{
		const unsigned char *buf0 = buffer;
		size_t write_count = 1;	/* 0 means a write failed */

		if (!buffer) {				/* fflush the stream */
		FFLUSH:
			{
				size_t count = stream->bufpos - stream->bufstart;
				p = stream->bufstart;

				if (stream->filedes == -2) { /* TODO -- document this hack! */
					stream->modeflags |= __FLAG_WRITING;
					return (!buffer) ? 0 : ((buffer - buf0) + bytes);
				}

				{
					write_count = _stdio_WRITE(stream, p, count);
					p += write_count;
					count -= write_count;
				}
			
				stream->bufpos = stream->bufstart;
				while (count) {
					*stream->bufpos++ = *p++;
					--count;
				}

				if (!buffer) {	/* fflush case... */
					__stdio_validate_FILE(stream); /* debugging only */
					return stream->bufpos - stream->bufstart;
				}
			}
		}

#if 1
		/* TODO: If the stream is buffered, we may be able to omit. */
		if ((stream->bufpos == stream->bufstart) /* buf empty */
			&& (stream->bufend - stream->bufstart <= bytes)	/* fills */
			&& (stream->filedes != -2)) { /* not strinf fake file */
			/* so want to do a direct write of supplied buffer */
			{
				size_t rv = _stdio_WRITE(stream, buffer, bytes);
				buffer += rv;
				bytes -= rv;
			}
		} else
#endif
		/* otherwise buffer not empty and/or data fits */
		{
			size_t count = stream->bufend - stream->bufpos;
			p = buffer;

			if (count > bytes) {
				count = bytes;
			}
			bytes -= count;

			while (count) {
				*stream->bufpos++ = *buffer++;
				--count;
			}

			if (write_count) {	/* no write errors */
				if (bytes) {
					goto FFLUSH;
				}

				if (stream->modeflags & __FLAG_LBF) {
					while (p < buffer) { /* check for newline. */
						if (*p++ == '\n') {
							goto FFLUSH;
						}
					}
				}
			}
		}

#ifdef __STDIO_PUTC_MACRO
		if (!(stream->modeflags & (__FLAG_WIDE|__MASK_BUFMODE|__FLAG_ERROR))) {
			/* Not wide, no errors and fully buffered, so enable putc macro. */
			stream->bufputc = stream->bufend;
		}
#endif /* __STDIO_GETC_MACRO */
		stream->modeflags |= __FLAG_WRITING; /* Ensure Writing flag is set. */

		__stdio_validate_FILE(stream); /* debugging only */
		return buffer - buf0;

	}

#else  /* __STDIO_BUFFERS --------------------------------------- */

	__stdio_validate_FILE(stream); /* debugging only */

	if ((stream->modeflags & __FLAG_READONLY)
#ifndef __STDIO_AUTO_RW_TRANSITION
	/* ANSI/ISO requires either at EOF or currently not reading. */
		|| ((stream->modeflags & (__FLAG_READING|__FLAG_EOF))
			== __FLAG_READING)
#endif /* __STDIO_AUTO_RW_TRANSITION */
		) {
		stream->modeflags |= __FLAG_ERROR;
		/* TODO: This is for posix behavior if readonly.  To save space, we
		 * use this errno for write attempt while reading, as no errno is
		 * specified by posix for this case, even though the restriction is
		 * mentioned in fopen(). */
		__set_errno(EBADF);
		return 0;
	}

	/* We always clear the reading flag in case at EOF. */
	stream->modeflags &= ~(__FLAG_READING);

	/* Unlike the buffered case, we set the writing flag now since we don't
	 * need to do anything here for fflush(). */
	stream->modeflags |= __FLAG_WRITING;

	{
		register unsigned char *p = (unsigned char *) buffer;

		ssize_t rv = _stdio_WRITE(stream, p, bytes);

		p += rv;
		bytes -= rv;

		__stdio_validate_FILE(stream); /* debugging only */
		return (p - (unsigned char *)buffer);
	}

#endif /* __STDIO_BUFFERS *****************************************/
}

#endif
/**********************************************************************/
#ifdef L__stdio_init

/* Internal functions -- _stdio_init() and __stdio_validate_FILE
 * are not reentrant, but _stdio_term() is through fflush().
 * Also, the _cs_{read|write|close} functions are not reentrant. */

#ifndef NDEBUG
void __stdio_validate_FILE(FILE *stream)
{
	if (stream->filedes == -2) { /* fake FILE for sprintf, scanf, etc. */
		return;
	}

	__STDIO_THREADLOCK(stream);

#ifdef __STDIO_BUFFERS
	assert(stream->bufstart <= stream->bufread);
	if (stream->modeflags & (__FLAG_READING)) {
		assert(stream->bufpos <= stream->bufread);
	}
	assert(stream->bufread <= stream->bufend);
	assert(stream->bufpos <= stream->bufend);
	if ((stream->modeflags & __MASK_BUFMODE) == __FLAG_NBF) {
		assert(stream->bufstart == stream->bufend);
	}
	assert((stream->modeflags & __MASK_BUFMODE) <= __FLAG_NBF);
#endif
#ifdef __STDIO_PUTC_MACRO
	assert(stream->bufstart <= stream->bufputc);
	assert(stream->bufputc <= stream->bufend);
	if (stream->bufstart < stream->bufputc) {
		assert(stream->bufputc == stream->bufend);
		assert(stream->modeflags & (__FLAG_WRITING));
		assert(!(stream->modeflags
				 & (__FLAG_WIDE|__MASK_BUFMODE|__MASK_UNGOT|__FLAG_READONLY))
			   );
	}
#endif
#ifdef __STDIO_GETC_MACRO
	assert(stream->bufstart <= stream->bufgetc);
	assert(stream->bufgetc <= stream->bufread);
	if (stream->bufstart < stream->bufgetc) {
		assert(stream->modeflags & (__FLAG_READING));
		assert(!(stream->modeflags
				 & (__FLAG_WIDE|__MASK_BUFMODE|__MASK_UNGOT|__FLAG_WRITEONLY))
			   );
	}
#endif
	assert((stream->modeflags & __MASK_UNGOT) != __MASK_UNGOT);
	if (stream->modeflags & __MASK_UNGOT1) {
		assert(stream->ungot[1] <= 1);
	}
	if (stream->modeflags & __MASK_UNGOT) {
		assert(!(stream->modeflags & __FLAG_EOF));
	}
	assert((stream->modeflags & (__FLAG_READONLY|__FLAG_WRITEONLY))
		   != (__FLAG_READONLY|__FLAG_WRITEONLY));

	/* TODO -- filepos?  ungot_width?  filedes?  nextopen? */

	__STDIO_THREADUNLOCK(stream);
}
#endif

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
ssize_t _cs_read(void *cookie, char *buf, size_t bufsize)
{
	return read(*((int *) cookie), buf, bufsize);
}

ssize_t _cs_write(void *cookie, const char *buf, size_t bufsize)
{
	return write(*((int *) cookie), (char *) buf, bufsize);
}

int _cs_close(void *cookie)
{
	return close(*((int *) cookie));
}
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */

#ifdef __STDIO_BUFFERS
static unsigned char _fixed_buffers[2 * BUFSIZ];
#define bufin (_fixed_buffers)
#define bufout (_fixed_buffers + BUFSIZ)
#endif /* __STDIO_BUFFERS */

static FILE _stdio_streams[] = {
	__STDIO_INIT_FILE_STRUCT(_stdio_streams[0], __FLAG_LBF|__FLAG_READONLY, \
							 0, _stdio_streams + 1, bufin, BUFSIZ ),
	__STDIO_INIT_FILE_STRUCT(_stdio_streams[1], __FLAG_LBF|__FLAG_WRITEONLY, \
							 1, _stdio_streams + 2, bufout, BUFSIZ ), 
	__STDIO_INIT_FILE_STRUCT(_stdio_streams[2], __FLAG_NBF|__FLAG_WRITEONLY, \
							 2, 0, 0, 0 )
};

FILE *stdin = _stdio_streams + 0;
FILE *stdout = _stdio_streams + 1;
FILE *stderr = _stdio_streams + 2;

#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)

#ifdef __STDIO_BUFFERS
FILE *_stdio_openlist = _stdio_streams;
#else  /* __STDIO_BUFFERS */
FILE *_stdio_openlist = NULL;
#endif /* __STDIO_BUFFERS */

#ifdef __STDIO_THREADSAFE
pthread_mutex_t _stdio_openlist_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


void __stdio_init_mutex(pthread_mutex_t *m)
{
	static const pthread_mutex_t __stdio_mutex_initializer
		= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

	memcpy(m, &__stdio_mutex_initializer, sizeof(__stdio_mutex_initializer));
}
#endif /* __STDIO_THREADSAFE */

/* TODO - do we need to lock things, or do we just assume we're the only
 * remaining thread? */

/* Note: We assume here that we are the only remaining thread. */
void _stdio_term(void)
{
#if defined(__STDIO_GLIBC_CUSTOM_STREAMS) || defined(__STDIO_THREADSAFE)
	register FILE *ptr;
#endif

	/* TODO: if called via a signal handler for a signal mid _stdio_fwrite,
	 * the stream may be in an unstable state... what do we do?
	 * perhaps set error flag before and clear when done if successful? */

#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
#ifdef __STDIO_THREADSAFE
	/* First, forceably unlock the open file list and all files.
	 * Note: Set locking mode to "by caller" to save some overhead later. */
	__stdio_init_mutex(&_stdio_openlist_lock);
	for (ptr = _stdio_openlist ; ptr ; ptr = ptr->nextopen ) {
		ptr->user_locking = 1;
		__stdio_init_mutex(&ptr->lock);
	}
#endif /* __STDIO_THREADSAFE */
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */

#ifdef __STDIO_BUFFERS
	/* TODO -- set an alarm and flush each file "by hand"? to avoid blocking? */

	/* Now flush all streams. */
	fflush_unlocked(NULL);
#endif /* __STDIO_BUFFERS */

	/* Next close all custom streams in case of any special cleanup, but
	 * don't use fclose() because that pulls in free and malloc.  Also,
	 * don't worry about removing them from the list.  Just set the cookie
	 * pointer to NULL so that an error will be generated if someone tries
	 * to use the stream. */
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	for (ptr = _stdio_openlist ; ptr ; ptr = ptr->nextopen ) {
		if (ptr->cookie != &ptr->filedes) {	/* custom stream */
			__CLOSE(ptr);
			ptr->cookie = NULL;	/* Generate an error if used later. */
#if 0
/*  #ifdef __STDIO_BUFFERS */
		} else {
			/* TODO: "unbuffer" files like glibc does?  Inconsistent with
			 * custom stream handling above, but that's necessary to deal
			 * with special user-defined close behavior. */
			stream->bufpos = stream->bufread = stream->bufend
#ifdef __STDIO_GETC_MACRO
				= stream->bufgetc
#endif
#ifdef __STDIO_PUTC_MACRO
				= stream->bufputc
#endif
				= stream->bufstart;
#endif /* __STDIO_BUFFERS */
		}
	}
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */
}


#if defined(__STDIO_BUFFERS) || !defined(__UCLIBC__)
void _stdio_init(void)
{
#ifdef __STDIO_BUFFERS
	int old_errno = errno;
	/* stdin and stdout uses line buffering when connected to a tty. */
	_stdio_streams[0].modeflags ^= (1-isatty(0)) * __FLAG_LBF;
	_stdio_streams[1].modeflags ^= (1-isatty(1)) * __FLAG_LBF;
	__set_errno(old_errno);
#endif /* __STDIO_BUFFERS */
#ifndef __UCLIBC__
/* __stdio_term is automatically when exiting if stdio is used.
 * See misc/internals/__uClibc_main.c and and stdlib/atexit.c. */
	atexit(_stdio_term);
#endif /* __UCLIBC__ */
}
#endif /* defined(__STDIO_BUFFERS) || !defined(__UCLIBC__) */
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */

#endif
/**********************************************************************/
/* ANSI/ISO functions. */
/**********************************************************************/
#ifdef L_remove
#include <unistd.h>
#include <errno.h>

/* No reentrancy issues. */

int remove(register const char *filename)
{
	int old_errno = errno;

	/* SUSv3 says equivalent to rmdir() if a directory, and unlink()
	 * otherwise.  Hence, we need to try rmdir() first. */

	return (((rmdir(filename) == 0)
			 || ((errno == ENOTDIR)
				 && ((__set_errno(old_errno), unlink(filename)) == 0)))
			? 0 : -1);
}
#endif
/**********************************************************************/
/* rename is a syscall
#ifdef L_rename
int rename(const char *old, const char *new);
#endif
*/
/**********************************************************************/
/* TODO: tmpfile */
/*  #ifdef L_tmpfile */
/*  FILE *tmpfile(void); */
/*  #endif */
/**********************************************************************/
/* TODO: tmpname */
/*  #ifdef L_tmpname */
/*  char *tmpname(char *s); */
/*  #endif */
/**********************************************************************/
#ifdef L_fclose

/* We need to be careful here to avoid deadlock when threading, as we
 * need to lock both the file and the open file list.  This can clash
 * with fflush.  Since fflush is much more common, we do the extra
 * work here. */

int fclose(register FILE *stream)
{
#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)

	register FILE *ptr;
	int rv = 0;

#ifdef __STDIO_THREADSAFE
	/* Need two non-heirchal mutexs... be careful to avoid deadlock*/
	do {
		__STDIO_THREADLOCK(stream);
		if (__STDIO_THREADTRYLOCK_OPENLIST == 0) {
			break;
		}
		__STDIO_THREADUNLOCK(stream);
		usleep(10000);
	} while (1);
#endif /* __STDIO_THREADSAFE */

	__stdio_validate_FILE(stream); /* debugging only */

#ifdef __STDIO_BUFFERS
	if (stream->modeflags & __FLAG_WRITING) {
		rv = fflush_unlocked(stream); /* Write any pending buffered chars. */
	}								  /* Also disables putc macro if used. */

#ifdef __STDIO_GETC_MACRO
	/* Not necessary after fflush, but always do this to reduce size. */
	stream->bufgetc = stream->bufstart;	/* Disable getc macro for safety. */
#endif /* __STDIO_GETC_MACRO */
#endif /* __STDIO_BUFFERS */

	/* Remove file from open list before closing file descriptor. */
	ptr = _stdio_openlist;
	if (ptr == stream) {
		_stdio_openlist = stream->nextopen;
	} else {
		while (ptr) {
			if (ptr->nextopen == stream) {
				ptr->nextopen = stream->nextopen;
				break;
			}
			ptr = ptr->nextopen;
		}
	}
	__STDIO_THREADUNLOCK_OPENLIST; /* We're done with the open file list. */

	if (__CLOSE(stream) < 0) {	/* Must close even if fflush failed. */
		rv = EOF;
	}
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	stream->cookie = &(stream->filedes);
	stream->gcs.read = _cs_read;
	stream->gcs.write = _cs_write;
	stream->gcs.seek = 0;		/* The internal seek func handles normals. */
	stream->gcs.close = _cs_close;
#endif
	stream->filedes = -1;		/* To aid debugging... */

#ifdef __STDIO_BUFFERS
	if (stream->modeflags & __FLAG_FREEBUF) {
		free(stream->bufstart);
	}
#endif /* __STDIO_BUFFERS */

	/* TODO -- leave the stream locked to catch any dangling refs? */
	__STDIO_THREADUNLOCK(stream);

	/* At this point, any dangling refs to the stream are the result of
	 * a programming bug... so free the unlocked stream. */
	if (stream->modeflags & __FLAG_FREEFILE) {
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	    stream->cookie = NULL;	/* To aid debugging... */
#endif
		free(stream);
	}

	return rv;

#else  /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */

	int rv = 0;

	__STDIO_THREADLOCK(stream);

	__stdio_validate_FILE(stream); /* debugging only */

	if (__CLOSE(stream) < 0) {	/* Must close even if fflush failed. */
		rv = EOF;
	}

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	stream->cookie = &(stream->filedes);
	stream->gcs.read = _cs_read;
	stream->gcs.write = _cs_write;
	stream->gcs.seek = 0;		/* The internal seek func handles normals. */
	stream->gcs.close = _cs_close;
#endif
	stream->filedes = -1;		/* To aid debugging... */

	__STDIO_THREADUNLOCK(stream);

	/* At this point, any dangling refs to the stream are the result of
	 * a programming bug... so free the unlocked stream. */
	if (stream->modeflags & __FLAG_FREEFILE) {
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	    stream->cookie = NULL;	/* To aid debugging... */
#endif
		free(stream);
	}

	return rv;

#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS )*/
}

#endif
/**********************************************************************/
#ifdef L_fflush

/*
 * Special cases:
 *   stream == NULL means fflush all writing streams (ANSI/ISO).
 *   stream == (FILE *) &_stdio_openlist -- implementation-specific hack
 *      meaning fflush all line buffered writing streams
 */

/*
 * NOTE: ANSI/ISO difference!!!  According to the standard, fflush is only
 * defined for write-only streams, or read/write streams whose last op
 * was a write.  However, reading is allowed for a read/write stream if
 * a file positioning operation was done (fseek, fsetpos) even though there
 * is no guarantee of flushing the write data in that case.  Hence, for
 * this case we keep a flag to indicate whether or not the buffer needs to
 * be flushed even if the last operation was a read.  This falls under the
 * implementation-defined behavior.  Otherwise, we would need to flush
 * every time we did fseek, etc. even if we were still in the buffer's range.
 */

/* Since the stream pointer arg is allowed to be NULL, or the address of the
 * stdio open file list if stdio is buffered in this implementation, we can't
 * use the UNLOCKED() macro here. */

#ifndef __STDIO_THREADSAFE
strong_alias(fflush_unlocked,fflush)
#else  /* __STDIO_THREADSAFE */
int fflush(register FILE *stream)
{
	int retval;

	if ((stream != NULL)
#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
		&& (stream != (FILE *) &_stdio_openlist)
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
		) {
		__STDIO_THREADLOCK(stream);
		retval = fflush_unlocked(stream);
		__STDIO_THREADUNLOCK(stream);
	} else {
		retval = fflush_unlocked(stream);
	}

	return retval;
}
#endif /* __STDIO_THREADSAFE */

int fflush_unlocked(register FILE *stream)
{
#ifdef __STDIO_BUFFERS

	int rv = 0;
	unsigned short mask = (__FLAG_NBF|__FLAG_LBF);

#ifndef NDEBUG
	if ((stream != NULL) && (stream != (FILE *) &_stdio_openlist)) {
		__stdio_validate_FILE(stream); /* debugging only */
	}
#endif

	if (stream == (FILE *) &_stdio_openlist) { /* fflush all line-buffered */
		stream = NULL;
		mask = __FLAG_LBF;
	}

	if (stream == NULL) {		/* flush all (line) buffered writing streams */
		/* Note -- We have to lock the list even in the unlocked function. */
		__STDIO_THREADLOCK_OPENLIST;
		/* TODO -- Can we work around locking the list to avoid keeping it
		 * locked if the write blocks? */
		for (stream = _stdio_openlist; stream; stream = stream->nextopen) {
			if (((stream->modeflags ^ __FLAG_NBF) & mask)
				&& (stream->modeflags & __FLAG_WRITING)
				&& fflush(stream)) {
				rv = EOF;
			}
		}
		__STDIO_THREADUNLOCK_OPENLIST;
	} else if (stream->modeflags & __FLAG_WRITING) {
		if (_stdio_fwrite(NULL, 0, stream) > 0) { /* flush buffer contents. */
			rv = -1;			/* Not all chars written. */
		}
#ifdef __UCLIBC_MJN3_ONLY__
#warning WISHLIST: Add option to test for undefined behavior of fflush.
#endif /* __UCLIBC_MJN3_ONLY__ */
#if 0
	} else if (stream->modeflags & (__FLAG_READING|__FLAG_READONLY)) {
		/* ANSI/ISO says behavior in this case is undefined but also says you
		 * shouldn't flush a stream you were reading from.  As usual, glibc
		 * caters to broken programs and simply ignores this. */
		stream->modeflags |= __FLAG_ERROR;
		__set_errno(EBADF);
		rv = -1;
#endif
	}

#ifndef NDEBUG
	if ((stream != NULL) && (stream != (FILE *) &_stdio_openlist)) {
		__stdio_validate_FILE(stream); /* debugging only */
	}
#endif
	return rv;

#else  /* __STDIO_BUFFERS --------------------------------------- */

#ifndef NDEBUG
	if ((stream != NULL)
#if defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
		&& (stream != (FILE *) &_stdio_openlist)
#endif /* defined(__STDIO_BUFFERS) || defined(__STDIO_GLIBC_CUSTOM_STREAMS) */
		) {
		__stdio_validate_FILE(stream); /* debugging only */
	}
#endif

#ifdef __UCLIBC_MJN3_ONLY__
#warning WISHLIST: Add option to test for undefined behavior of fflush.
#endif /* __UCLIBC_MJN3_ONLY__ */
#if 0
	return ((stream != NULL)
			&& (stream->modeflags & (__FLAG_READING|__FLAG_READONLY))
			? ((stream->modeflags |= __FLAG_ERROR), __set_errno(EBADF), EOF)
			: 0 );
#else
	return 0;
#endif

#endif /* __STDIO_BUFFERS */
}
#endif
/**********************************************************************/
#ifdef L_fopen

/* No reentrancy issues. */

FILE *fopen(const char * __restrict filename, const char * __restrict mode)
{
	return _stdio_fopen(filename, mode, NULL, -1);
}

#endif
/**********************************************************************/
#ifdef L__stdio_fopen

/*
 * Cases:
 *  fopen    : filename != NULL, stream == NULL, filedes == -1
 *  freopen  : filename != NULL, stream != NULL, filedes == -1
 *  fdopen   : filename == NULL, stream == NULL, filedes valid
 *  fsfopen  : filename != NULL, stream != NULL, filedes == -1
 *  fopen64  : filename != NULL, stream == NULL, filedes == -2
 */

#if O_ACCMODE != 3 || O_RDONLY != 0 || O_WRONLY != 1 || O_RDWR != 2
#error Assumption violated - mode constants
#endif

/* Internal function -- reentrant (locks open file list) */

FILE *_stdio_fopen(const char * __restrict filename,
				   register const char * __restrict mode,
				   register FILE * __restrict stream, int filedes)
{
	__mode_t open_mode;

	/* parse mode */
	open_mode = O_RDONLY;
	if (*mode != 'r') {			/* not read */
		open_mode = (O_WRONLY | O_CREAT | O_TRUNC);
		if (*mode != 'w') {		/* not write (create or truncate)*/
			open_mode = (O_WRONLY | O_CREAT | O_APPEND);
			if (*mode != 'a') {	/* not write (create or append) */
				__set_errno(EINVAL); /* then illegal mode */
				return NULL;
			}
		}
	}

	if ((*++mode == 'b')) {		/* binary mode (NOP currently) */
		++mode;
	}

	if (*mode == '+') {			/* read-write */
		++mode;
		open_mode &= ~(O_RDONLY | O_WRONLY);
		open_mode |= O_RDWR;
	}

#if defined(__STDIO_FOPEN_EXCLUSIVE_MODE) || defined(__STDIO_FOPEN_LARGEFILE_MODE)
	for ( ; *mode ; ++mode) {  /* ignore everything else except ... */
#ifdef __STDIO_FOPEN_EXCLUSIVE_MODE
		if (*mode == 'x') {	   /* open exclusive -- glibc extension */
			open_mode |= O_EXCL;
			continue;
		}
#endif /* __STDIO_FOPEN_EXCLUSIVE_MODE */
#ifdef __STDIO_FOPEN_LARGEFILE_MODE
		if (*mode == 'F') {		/* open large file */
			open_mode |= O_LARGEFILE;
			continue;
		}
#endif /* __STDIO_FOPEN_LARGEFILE_MODE */
	}
#endif /* __STDIO_FOPEN_EXCLUSIVE_MODE or __STDIO_FOPEN_LARGEFILE_MODE def'd */

#ifdef __BCC__
	mode = filename;			/* TODO:  help BCC with register allocation. */
#define filename mode
#endif /* __BCC__ */

	if (!stream) {				/* Need to allocate a FILE. */
#ifdef __STDIO_BUFFERS
		if ((stream = malloc(sizeof(FILE))) == NULL) {
			return stream;
		}
		stream->modeflags = __FLAG_FREEFILE;
		if ((stream->bufstart = malloc(BUFSIZ)) != 0) {
			stream->bufend = stream->bufstart + BUFSIZ;
			stream->modeflags |= __FLAG_FREEBUF;
		} else {
#if __STDIO_BUILTIN_BUF_SIZE > 0
			stream->bufstart = stream->unbuf;
			stream->bufend = stream->unbuf + sizeof(stream->unbuf);
#else  /* __STDIO_BUILTIN_BUF_SIZE > 0 */
			stream->bufstart = stream->bufend = NULL;
#endif /* __STDIO_BUILTIN_BUF_SIZE > 0 */
		}
#else  /* __STDIO_BUFFERS */
		if ((stream = malloc(sizeof(FILE))) == NULL) {
			return stream;
		}
		stream->modeflags = __FLAG_FREEFILE;
#endif /* __STDIO_BUFFERS */
	}

	if (filedes >= 0) {			/* Handle fdopen trickery. */
		/* NOTE: it is insufficient to just check R/W/RW agreement.
		 * We must also check largefile compatibility if applicable.
		 * Also, if append mode is desired for fdopen but O_APPEND isn't
		 * currently set, then set it as recommended by SUSv3.  However,
		 * if append mode is not specified for fdopen but O_APPEND is set,
		 * leave it set (glibc compat). */
		int i = (open_mode & (O_ACCMODE|O_LARGEFILE)) + 1;

		/* NOTE: fopencookie needs changing if the basic check changes! */
		if (((i & (((int) filename) + 1)) != i)	/* Check basic agreement. */
			|| (((open_mode & O_APPEND)
				 && !(((int) filename) & O_APPEND)
				 && fcntl(filedes, F_SETFL, O_APPEND)))	/* Need O_APPEND. */
			) {
			__set_errno(EINVAL);
			filedes = -1;
		}
#ifdef __STDIO_LARGE_FILES
		/* For later... to reflect largefile setting in stream flags. */
		open_mode |= (((int) filename) & O_LARGEFILE);
#endif /* __STDIO_LARGE_FILES */
		stream->filedes = filedes;
	} else {
#ifdef __STDIO_LARGE_FILES
		if (filedes < -1) {
			open_mode |= O_LARGEFILE;
		}
#endif /* __STDIO_LARGE_FILES */
		stream->filedes = open(filename, open_mode, 0666);
	}

	if (stream->filedes < 0) {
#ifdef __STDIO_BUFFERS
		if (stream->modeflags & __FLAG_FREEBUF) {
			free(stream->bufstart);
		}
#endif /* __STDIO_BUFFERS */
		if (stream->modeflags & __FLAG_FREEFILE) {
			free(stream);
		}
		return NULL;
	}

#ifdef __STDIO_BUFFERS
	{
	    /* Do not let isatty mess up errno */
	    int old_errno = errno;
	    stream->modeflags |= (isatty(stream->filedes) * __FLAG_LBF);
	    __set_errno(old_errno);
	}
#endif

	stream->modeflags |=
#if (O_APPEND == __FLAG_APPEND) \
&& ((O_LARGEFILE == __FLAG_LARGEFILE) || (O_LARGEFILE == 0))
		(open_mode & (O_APPEND|O_LARGEFILE)) | /* i386 linux and elks */
#else  /* (O_APPEND == __FLAG_APPEND) && (O_LARGEFILE == __FLAG_LARGEFILE) */
		((open_mode & O_APPEND) ? __FLAG_APPEND : 0) |
#ifdef __STDIO_LARGE_FILES
		((open_mode & O_LARGEFILE) ? __FLAG_LARGEFILE : 0) |
#endif /* __STDIO_LARGE_FILES */
#endif /* (O_APPEND == __FLAG_APPEND) && (O_LARGEFILE == __FLAG_LARGEFILE) */
		((((open_mode & O_ACCMODE) + 1) ^ 0x03) * __FLAG_WRITEONLY);

#ifdef __STDIO_BUFFERS
#ifdef __STDIO_GETC_MACRO
	stream->bufgetc =
#endif
#ifdef __STDIO_PUTC_MACRO
	stream->bufputc =
#endif
	stream->bufpos = stream->bufread = stream->bufstart;
#endif /* __STDIO_BUFFERS */

#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	stream->cookie = &(stream->filedes);
	stream->gcs.read = _cs_read;
	stream->gcs.write = _cs_write;
	stream->gcs.seek = 0;		/* The internal seek func handles normals. */
	stream->gcs.close = _cs_close;
#endif /* __STDIO_GLIBC_CUSTOM_STREAMS */

#ifdef __STDIO_WIDE
	stream->ungot_width[0] = 0;
#endif /* __STDIO_WIDE */
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(stream->state));
#endif /* __STDIO_MBSTATE */

#ifdef __STDIO_THREADSAFE
	stream->user_locking = 0;
	__stdio_init_mutex(&stream->lock);
#endif /* __STDIO_THREADSAFE */

#if defined(__STDIO_BUFFERS) \
|| (defined(__STDIO_THREADSAFE) && defined(__STDIO_GLIBC_CUSTOM_STREAMS))
	__STDIO_THREADLOCK_OPENLIST;
	stream->nextopen = _stdio_openlist;	/* New files are inserted at */
	_stdio_openlist = stream;			/*   the head of the list. */
	__STDIO_THREADUNLOCK_OPENLIST;
#endif

	__stdio_validate_FILE(stream); /* debugging only */
	return stream;
#ifdef __BCC__
#undef filename
#endif /* __BCC__ */
}

#endif
/**********************************************************************/
#ifdef L_freopen

/* Reentrant. */

FILE *freopen(const char * __restrict filename, const char * __restrict mode,
			  register FILE * __restrict stream)
{
	/*
	 * ANSI/ISO allow (implementation-defined) change of mode for an
	 * existing file if filename is NULL.  It doesn't look like Linux
	 * supports this, so we don't here.
	 *
	 * NOTE: Whether or not the stream is free'd on failure is unclear
	 *       w.r.t. ANSI/ISO.  This implementation chooses to NOT free
	 *       the stream and associated buffer if they were dynamically
	 *       allocated.
	 * TODO: Check the above.
	 * TODO: Apparently linux allows setting append mode.  Implement?
	 */
	unsigned short dynmode;
	register FILE *fp;

	__STDIO_THREADLOCK(stream);

	/* First, flush and close, but don't deallocate, the stream. */
	/* This also removes the stream for the open file list. */
	dynmode = 
#ifdef __STDIO_BUFFERS
		/*		__MASK_BUFMODE | */		/* TODO: check */
#endif /* __STDIO_BUFFERS */
		(stream->modeflags & (__FLAG_FREEBUF|__FLAG_FREEFILE));

	stream->modeflags &= ~(__FLAG_FREEBUF|__FLAG_FREEFILE);
	fclose(stream);				/* Failures are ignored. */

	fp = _stdio_fopen(filename, mode, stream, -1);

	stream->modeflags |= dynmode;

	__STDIO_THREADUNLOCK(stream);

	return fp;
}
#endif
/**********************************************************************/
#ifdef L_freopen64

/* Reentrant. */

/* TODO -- is it worth collecting the common work (40 bytes) in a function? */
FILE *freopen64(const char * __restrict filename, const char * __restrict mode,
				register FILE * __restrict stream)
{
	unsigned short dynmode;
	register FILE *fp;

	__STDIO_THREADLOCK(stream);

	/* First, flush and close, but don't deallocate, the stream. */
	/* This also removes the stream for the open file list. */
	dynmode = 
#ifdef __STDIO_BUFFERS
		/*		__MASK_BUFMODE | */		/* TODO: check */
#endif /* __STDIO_BUFFERS */
		(stream->modeflags & (__FLAG_FREEBUF|__FLAG_FREEFILE));

	stream->modeflags &= ~(__FLAG_FREEBUF|__FLAG_FREEFILE);
	fclose(stream);				/* Failures are ignored. */
	stream->modeflags = dynmode;

	fp = _stdio_fopen(filename, mode, stream, -2); /* TODO -- magic const */

	__STDIO_THREADUNLOCK(stream);

	return fp;
}

#endif
/**********************************************************************/
#ifdef L_setbuf

/* Reentrant through setvbuf(). */

void setbuf(FILE * __restrict stream, register char * __restrict buf)
{
#ifdef __STDIO_BUFFERS
	int mode;

	mode = (buf != NULL) ? _IOFBF : _IONBF;
	setvbuf(stream, buf, mode, BUFSIZ);
#else  /* __STDIO_BUFFERS */
	/* TODO -- assert on stream? */
	/* Nothing to do. */
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************/
#ifdef L_setvbuf

/* Reentrant. */

int setvbuf(register FILE * __restrict stream, register char * __restrict buf,
			int mode, size_t size)
{
#ifdef __STDIO_BUFFERS

	int allocated_buf_flag;
	int rv = EOF;

	__STDIO_THREADLOCK(stream);

	__stdio_validate_FILE(stream); /* debugging only */

	if (((unsigned int) mode) > 2) { /* Illegal mode. */
		/* TODO -- set an errno? */
		goto DONE;
	}

#ifdef __STDIO_FLEXIBLE_SETVBUF
	/* C89 standard requires no ops before setvbuf, but we can be flexible. */
	/* NOTE: This will trash any chars ungetc'd!!! */
	/* TODO: hmm could preserve unget count since ungot slots aren't changed (true?)
	 * but this will fail when buffered chars read from a pipe unless the user buf
	 * is big enough to copy everything over. */
	if (fseek(stream, 0L, SEEK_CUR)) {
		goto DONE;
	}
#else  /* __STDIO_FLEXIBLE_SETVBUF */
	/*
	 * Note: ANSI/ISO requires setvbuf to be called after opening the file
	 * but before any other operation other than a failed setvbuf call.
	 * We'll cheat here and only test if the wide or narrow mode flag has
	 * been set; i.e. no read or write (or unget or fwide) operations have
	 * taken place.
	 */
#ifdef __STDIO_WIDE
	if (stream->modeflags & (__FLAG_WIDE|__FLAG_NARROW)) {
		goto DONE;
	}
#else  /* __STDIO_WIDE */
	/* Note: This only checks if not currently reading or writing. */
	if (stream->modeflags & (__FLAG_READING|__FLAG_WRITING)) {
		goto DONE;
	}
#endif /* __STDIO_WIDE */
#endif /* __STDIO_FLEXIBLE_SETVBUF */

	if (mode == _IONBF) {
		size = 0;
		buf = NULL;
	} else if (!buf && !size) {
		/* If buf==NULL && size==0 && either _IOFBF or _IOLBF, keep
		 * current buffer and only set buffering mode. */
		size = stream->bufend - stream->bufstart;
	}

	stream->modeflags &= ~(__MASK_BUFMODE);	/* Clear current mode */
	stream->modeflags |= mode * __FLAG_LBF;	/* and set new one. */

	allocated_buf_flag = 0;
	if ((!buf) && (size != (stream->bufend - stream->bufstart))) {
		/* No buffer supplied and requested size different from current. */
		/* If size == 0, create a (hopefully) bogus non-null pointer... */
		if (!(buf = ((size > 0)
					 ? ((allocated_buf_flag = __FLAG_FREEBUF), malloc(size))
					 : ((char *)NULL) + 1))
			) {
			/* TODO -- should we really keep current buffer if it was passed
			 * to us earlier by the app? */
			goto DONE;		/* Keep current buffer. */
		}
	}

	/* TODO: setvbuf "signal" safety */
	if (buf && (buf != (char *) stream->bufstart)) { /* Want new buffer. */
		if (stream->modeflags & __FLAG_FREEBUF) {
			stream->modeflags &= ~(__FLAG_FREEBUF);
			free(stream->bufstart);
		}
		stream->modeflags |= allocated_buf_flag;	/* Free-able buffer? */
#ifdef __STDIO_GETC_MACRO
		stream->bufgetc =
#endif
#ifdef __STDIO_PUTC_MACRO
		stream->bufputc =
#endif
		stream->bufpos = stream->bufread = stream->bufstart = buf;
		stream->bufend = buf + size;
	}

	__stdio_validate_FILE(stream); /* debugging only */

	rv = 0;

 DONE:
	__STDIO_THREADUNLOCK(stream);

	return rv;

#else  /* __STDIO_BUFFERS */
	__stdio_validate_FILE(stream); /* debugging only */
	/* TODO -- set errno for illegal mode? */

	return EOF;
#endif /* __STDIO_BUFFERS */
}

#endif
/**********************************************************************
int fprintf(FILE * __restrict stream, const char * __restrict format, ...);
int fscanf(FILE * __restrict stream, const char * __restrict format, ...);
int printf(const char * __restrict format, ...);
int scanf(const char * __restrict format, ...);
int snprintf(char * __restrict s, size_t n,
					const char * __restrict format, ...);
int sprintf(char * __restrict s, const char * __restrict format, ...);
int sscanf(char * __restrict s, const char * __restrict format, ...);
int vfprintf(FILE * __restrict stream, const char * __restrict format,
					va_list arg);
int vfscanf(FILE * __restrict stream, const char * __restrict format,
				   va_list arg);
int vprintf(const char * __restrict format, va_list arg);
int vscanf(const char * __restrict format, va_list arg);
int vsnprintf(char * __restrict s, size_t n,
					 const char * __restrict format, va_list arg);
int vsprintf(char * __restrict s, const char * __restrict format,
					va_list arg);
int vsscanf(char * __restrict s, const char * __restrict format,
				   va_list arg);
**********************************************************************/
#ifdef L_fgetc

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,fgetc,(FILE *stream),(stream))
{
	unsigned char buf[1];

#ifdef __STDIO_WIDE

	return (fread_unlocked(buf, (size_t) 1, (size_t) 1, stream) > 0)
		? *buf : EOF;

#else  /* __STDIO_WIDE */

	return (_stdio_fread(buf, (size_t) 1, stream) > 0) ? *buf : EOF;

#endif /* __STDIO_WIDE */
}

#endif
/**********************************************************************/
#ifdef L_fgets

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(char *,fgets,
		 (char *__restrict s, int n, register FILE * __restrict stream),
		 (s, n, stream))
{
	register char *p;
	int c;

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: What should fgets do if n <= 0?
#endif /* __UCLIBC_MJN3_ONLY__ */
	/* Should we assert here?  Or set errno?  Or just fail... */
	if (n <= 0) {
/* 		__set_errno(EINVAL); */
		goto ERROR;
	}

	p = s;

	while (--n) {
		if ((c = (getc_unlocked)(stream)) == EOF) {	/* Disable the macro. */
			if (__FERROR(stream)) {
				goto ERROR;
			}
			break;
		}
		if ((*p++ = c) == '\n') {
			break;
		}
	}

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: If n==1 and not at EOF, should fgets return an empty string?
#endif /* __UCLIBC_MJN3_ONLY__ */
	if (p > s) {
		*p = 0;
		return s;
	}

 ERROR:
	return NULL;
}

#endif
/**********************************************************************/
#ifdef L_fputc

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,fputc,(int c, FILE *stream),(c,stream))
{
	unsigned char buf[1];

	*buf = (unsigned char) c;

#ifdef __STDIO_WIDE

	return (fwrite_unlocked(buf, (size_t) 1, (size_t) 1, stream) > 0)
		? (*buf) : EOF;

#else  /* __STDIO_WIDE */

	return (_stdio_fwrite(buf, (size_t) 1, stream) > 0) ? (*buf) : EOF;

#endif /* __STDIO_WIDE */
}
#endif
/**********************************************************************/
#ifdef L_fputs

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,fputs,
		 (register const char * __restrict s, FILE * __restrict stream),
		 (s, stream))
{
	size_t n = strlen(s);

#ifdef __STDIO_WIDE

	return (fwrite_unlocked(s, (size_t) 1, n, stream) == n) ? n : EOF;

#else  /* __STDIO_WIDE */

	return (_stdio_fwrite(s, n, stream) == n) ? n : EOF;

#endif /* __STDIO_WIDE */
}

#endif
/**********************************************************************/
#ifdef L_getc
#undef getc
#undef getc_unlocked

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,getc,(register FILE *stream),(stream))
{
	return __GETC(stream);		/* Invoke the macro. */
}

#endif
/**********************************************************************/
#ifdef L_getchar
#undef getchar					/* Just in case. */

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED_STREAM(int,getchar,(void),(),stdin)
{
	register FILE *stream = stdin; /* This helps bcc optimize. */

	return __GETC(stream);
}

#endif
/**********************************************************************/
#ifdef L_gets

link_warning(gets, "the 'gets' function is dangerous and should not be used.")

/* Reentrant. */

char *gets(char *s)				/* WARNING!!! UNSAFE FUNCTION!!! */
{
	register FILE *stream = stdin;	/* This helps bcc optimize. */
	register char *p = s;
	int c;

	__STDIO_THREADLOCK(stream);

	/* Note: don't worry about performance here... this shouldn't be used!
	 * Therefore, force actual function call. */
	while (((c = (getc_unlocked)(stream)) != EOF) && ((*p = c) != '\n')) {
		++p;
	}
	if ((c == EOF) || (s == p)) {
		s = NULL;
	} else {
		*p = 0;
	}

	__STDIO_THREADUNLOCK(stream);

	return s;
}

#endif
/**********************************************************************/
#ifdef L_putc
#undef putc
#undef putc_unlocked

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,putc,(int c, register FILE *stream),(c,stream))
{
	return __PUTC(c, stream);	/* Invoke the macro. */
}

#endif
/**********************************************************************/
#ifdef L_putchar
#undef putchar					/* Just in case. */

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED_STREAM(int,putchar,(int c),(c),stdout)
{
	register FILE *stream = stdout; /* This helps bcc optimize. */

	return __PUTC(c, stream);
}

#endif
/**********************************************************************/
#ifdef L_puts

/* Reentrant. */

int puts(register const char *s)
{
	register FILE *stream = stdout; /* This helps bcc optimize. */
	int n;

	__STDIO_THREADLOCK(stream);

	n = fputs_unlocked(s,stream) + 1;
	if (
#if 1
		fputc_unlocked('\n',stream)
#else
		fputs_unlocked("\n",stream)
#endif
		== EOF) {
		n = EOF;
	}

	__STDIO_THREADUNLOCK(stream);

	return n;
}

#endif
/**********************************************************************/
#ifdef L_ungetc
/*
 * Note: This is the application-callable ungetc.  If scanf calls this, it
 * should also set stream->ungot[1] to 0 if this is the only ungot.
 */

/* Reentrant. */

int ungetc(int c, register FILE *stream)
{
	__STDIO_THREADLOCK(stream);

	__stdio_validate_FILE(stream); /* debugging only */

#ifdef __STDIO_WIDE
	if (stream->modeflags & __FLAG_WIDE) {
		stream->modeflags |= __FLAG_ERROR;
		c = EOF;
		goto DONE;
	}
	stream->modeflags |= __FLAG_NARROW;
#endif /* __STDIO_WIDE */

	/* If can't read or c == EOF or ungot slots already filled, then fail. */
	if ((stream->modeflags
		 & (__MASK_UNGOT2|__FLAG_WRITEONLY
#ifndef __STDIO_AUTO_RW_TRANSITION
			|__FLAG_WRITING		/* Note: technically no, but yes in spirit */
#endif /* __STDIO_AUTO_RW_TRANSITION */
			))
		|| ((stream->modeflags & __MASK_UNGOT1) && (stream->ungot[1]))
		|| (c == EOF) ) {
		c = EOF;
		goto DONE;;
	}

#ifdef __STDIO_BUFFERS
#ifdef __STDIO_AUTO_RW_TRANSITION
	if (stream->modeflags & __FLAG_WRITING) {
		fflush_unlocked(stream); /* Commit any write-buffered chars. */
	}
#endif /* __STDIO_AUTO_RW_TRANSITION */
#endif /* __STDIO_BUFFERS */

	/* Clear EOF and WRITING flags, and set READING FLAG */
	stream->modeflags &= ~(__FLAG_EOF|__FLAG_WRITING);
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Is setting the reading flag after an ungetc necessary?
#endif /* __UCLIBC_MJN3_ONLY__ */
	stream->modeflags |= __FLAG_READING;
	stream->ungot[1] = 1;		/* Flag as app ungetc call; scanf fixes up. */
	stream->ungot[(stream->modeflags++) & __MASK_UNGOT] = c;

#ifdef __STDIO_GETC_MACRO
	stream->bufgetc = stream->bufstart;	/* Must disable getc macro. */
#endif

	__stdio_validate_FILE(stream); /* debugging only */

 DONE:
	__STDIO_THREADUNLOCK(stream);

	return c;
}

#endif
/**********************************************************************/
#ifdef L_fread
/* NOTE: Surely ptr cannot point to a buffer of size > SIZE_MAX.
 * Therefore, we treat the case size * nmemb > SIZE_MAX as an error,
 * and include an assert() for it. */

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(size_t,fread,
		 (void * __restrict ptr, size_t size, size_t nmemb,
		  FILE * __restrict stream),
		 (ptr,size,nmemb,stream))
{
#ifdef __STDIO_WIDE
	if (stream->modeflags & __FLAG_WIDE) {
		stream->modeflags |= __FLAG_ERROR;
		/* TODO -- errno?  it this correct? */
		return 0;
	}
	stream->modeflags |= __FLAG_NARROW;
#endif /* __STDIO_WIDE */

	return (size == 0)
		? 0
		: (	assert( ((size_t)(-1)) / size >= nmemb ),
			_stdio_fread(ptr, nmemb * size, stream) / size );
}

#endif
/**********************************************************************/
#ifdef L_fwrite
/* NOTE: Surely ptr cannot point to a buffer of size > SIZE_MAX.
 * Therefore, we treat the case size * nmemb > SIZE_MAX as an error,
 * and include an assert() for it. */

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(size_t,fwrite,
		 (const void * __restrict ptr, size_t size, size_t nmemb,
		  FILE * __restrict stream),
		 (ptr,size,nmemb,stream))
{
#ifdef __STDIO_WIDE
	if (stream->modeflags & __FLAG_WIDE) {
		stream->modeflags |= __FLAG_ERROR;
		/* TODO -- errno?  it this correct? */
		return 0;
	}
	stream->modeflags |= __FLAG_NARROW;
#endif /* __STDIO_WIDE */

	return (size == 0)
		? 0
		: (	assert( ((size_t)(-1)) / size >= nmemb ),
			_stdio_fwrite(ptr, nmemb * size, stream) / size );
}

#endif
/**********************************************************************/
#if defined(L_fgetpos) || defined(L_fgetpos64)

/* Reentrant -- fgetpos() and fgetpos64(). */

#if defined(L_fgetpos) && defined(L_fgetpos64)
#error L_fgetpos and L_fgetpos64 are defined simultaneously!
#endif

#ifndef L_fgetpos64
#define fgetpos64	fgetpos
#define fpos64_t	fpos_t
#define ftello64	ftell
#endif


int fgetpos64(FILE * __restrict stream, register fpos64_t * __restrict pos)
{
	int retval = -1;

#ifdef __STDIO_WIDE

	if (pos == NULL) {
		__set_errno(EINVAL);
	} else {
		__STDIO_THREADLOCK(stream);

		if ((pos->__pos = ftello64(stream)) >= 0) {
			__COPY_MBSTATE(&(pos->__mbstate), &(stream->state));
			pos->mblen_pending = stream->ungot_width[0];
			retval = 0;
		}

		__STDIO_THREADUNLOCK(stream);
	}

#else  /* __STDIO_WIDE */

	if (pos == NULL) {
		__set_errno(EINVAL);
	} else if ((pos->__pos = ftello64(stream)) >= 0) {
		retval = 0;
	}

#endif /* __STDIO_WIDE */

	return retval;
}

#ifndef L_fgetpos64
#undef fgetpos64
#undef fpos64_t
#undef ftello64
#endif

#endif
/**********************************************************************/
#ifdef L_fseek
strong_alias(fseek,fseeko);
#endif

#if defined(L_fseek) && defined(__STDIO_LARGE_FILES)

int fseek(register FILE *stream, long int offset, int whence)
{
	return fseeko64(stream, offset, whence);
}

#endif

#if defined(L_fseeko64) || (defined(L_fseek) && !defined(__STDIO_LARGE_FILES))

#ifndef L_fseeko64
#define fseeko64	fseek
#define __off64_t	long int
#endif

/* Reentrant -- fseek(), fseeko(), fseeko64() */

int fseeko64(register FILE *stream, __off64_t offset, int whence)
{
#if SEEK_SET != 0 || SEEK_CUR != 1 || SEEK_END != 2
#error Assumption violated -- values of SEEK_SET, SEEK_CUR, SEEK_END
#endif
	__offmax_t pos[1];
	int retval;

	if ( ((unsigned int) whence) > 2 ) {
		__set_errno(EINVAL);
		return -1;
	}

	__STDIO_THREADLOCK(stream);

	__stdio_validate_FILE(stream); /* debugging only */

	retval = -1;
	*pos = offset;
	if (
#ifdef __STDIO_BUFFERS
		/* First commit any pending buffered writes. */
		((stream->modeflags & __FLAG_WRITING) && fflush_unlocked(stream)) ||
#endif /* __STDIO_BUFFERS */
		((whence == SEEK_CUR) && (_stdio_adjpos(stream, pos) < 0))
		|| (_stdio_lseek(stream, pos, whence) < 0)
		) {
		__stdio_validate_FILE(stream); /* debugging only */
		goto DONE;
	}

#ifdef __STDIO_BUFFERS
	/* only needed if reading but do it anyway to avoid test */
#ifdef __STDIO_GETC_MACRO
	stream->bufgetc =			/* Must disable getc. */
#endif
	stream->bufpos = stream->bufread = stream->bufstart;
#endif /* __STDIO_BUFFERS */

	stream->modeflags &=
		~(__FLAG_READING|__FLAG_WRITING|__FLAG_EOF|__MASK_UNGOT);

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: How do we deal with fseek to an ftell position for incomplete or error wide?  Right now, we clear all multibyte state info.  If we do not clear, then fix rewind() to do so if fseek() succeeds.
#endif /* __UCLIBC_MJN3_ONLY__ */

#ifdef __STDIO_WIDE
	/* TODO: don't clear state if don't move? */
	stream->ungot_width[0] = 0;
#endif /* __STDIO_WIDE */
#ifdef __STDIO_MBSTATE
	/* TODO: don't clear state if don't move? */
	__INIT_MBSTATE(&(stream->state));
#endif /* __STDIO_MBSTATE */

	__stdio_validate_FILE(stream); /* debugging only */

	retval = 0;

 DONE:
	__STDIO_THREADUNLOCK(stream);

	return retval;
}

#ifndef L_fseeko64
#undef fseeko64
#undef __off64_t
#endif

#endif
/**********************************************************************/
#if defined(L_fsetpos) || defined(L_fsetpos64)

#if defined(L_fsetpos) && defined(L_fsetpos64)
#error L_fsetpos and L_fsetpos64 are defined simultaneously!
#endif

#ifndef L_fsetpos64
#define fsetpos64	fsetpos
#define fpos64_t	fpos_t
#define fseeko64	fseek
#endif

/* Reentrant -- fgetpos{64}() through fseek{64}(). */

int fsetpos64(FILE *stream, register const fpos64_t *pos)
{
	if (!pos) {
		__set_errno(EINVAL);
		return EOF;
	}
#ifdef __STDIO_WIDE
	{
		int retval;

		__STDIO_THREADLOCK(stream);

		if ((retval = fseeko64(stream, pos->__pos, SEEK_SET)) == 0) {
			__COPY_MBSTATE(&(stream->state), &(pos->__mbstate));
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Moving mblen_pending into some mbstate field might be useful.  But we would need to modify all the mb<->wc funcs.
#endif /* __UCLIBC_MJN3_ONLY__ */
			stream->ungot_width[0] = pos->mblen_pending;
		}

		__STDIO_THREADUNLOCK(stream);

		return retval;
	}
#else  /* __STDIO_WIDE */
	return fseeko64(stream, pos->__pos, SEEK_SET);
#endif /* __STDIO_WIDE */
}

#ifndef L_fsetpos64
#undef fsetpos64
#undef fpos64_t
#undef fseeko64
#endif

#endif
/**********************************************************************/
#ifdef L_ftell
strong_alias(ftell,ftello);
#endif

#if defined(L_ftell) && defined(__STDIO_LARGE_FILES)
long int ftell(register FILE *stream)
{
	__offmax_t pos = ftello64(stream);

	return (pos == ((long int) pos)) ? pos : (__set_errno(EOVERFLOW), -1);
}
#endif

#if defined(L_ftello64) || (defined(L_ftell) && !defined(__STDIO_LARGE_FILES))

#ifndef L_ftello64
#define ftello64	ftell
#define __off64_t	long int
#endif

/* Reentrant -- ftell, ftello, ftello64. */

__off64_t ftello64(register FILE *stream)
{
	__offmax_t pos[1];
	__off64_t retval;

	__STDIO_THREADLOCK(stream);

	retval = (((*pos = 0), (_stdio_lseek(stream, pos, SEEK_CUR) < 0))
			  || (_stdio_adjpos(stream, pos) < 0)) ? -1 : *pos;

	__STDIO_THREADUNLOCK(stream);

	return retval;
}

#ifndef L_ftello64
#undef ftello64
#undef __off64_t
#endif

#endif
/**********************************************************************/
#ifdef L_rewind

void rewind(register FILE *stream)
{
	__STDIO_THREADLOCK(stream);

	__CLEARERR(stream);			/* Clear errors first and then seek */
	fseek(stream, 0L, SEEK_SET); /* in case there is an error seeking. */

	__STDIO_THREADUNLOCK(stream);
}

#endif
/**********************************************************************/
#ifdef L_clearerr
#undef clearerr

/* Reentrancy handled by UNLOCKED_VOID_RETURN() macro. */

UNLOCKED_VOID_RETURN(clearerr,(FILE *stream),(stream))
{
	__CLEARERR(stream);
}

#endif
/**********************************************************************/
#ifdef L_feof
#undef feof

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,feof,(FILE *stream),(stream))
{
	return __FEOF(stream);
}

#endif
/**********************************************************************/
#ifdef L_ferror
#undef ferror

/* Reentrancy handled by UNLOCKED() macro. */

UNLOCKED(int,ferror,(FILE *stream),(stream))
{
	return __FERROR(stream);
}

#endif
/**********************************************************************/
#ifdef L_perror

void perror(register const char *s)
{
	/* If the program is calling perror, it's a safe bet that printf and
	 * friends are used as well.  It is also possible that the calling
	 * program could buffer stderr, or reassign it. */

	register const char *sep;

	sep = ": ";
	if (!(s && *s)) {			/* Caller did not supply a prefix message */
		s = (sep += 2);			/* or passed an empty string. */
	}

#if 1
#ifdef __STDIO_PRINTF_M_SPEC
	fprintf(stderr, "%s%s%m\n", s, sep); /* Use the gnu %m feature. */
#else
	{
		char buf[64];
		fprintf(stderr, "%s%s%s\n", s, sep,
				_glibc_strerror_r(errno, buf, sizeof(buf)));
	}
#endif
#else
	/* Note: Assumes stderr not closed or buffered. */
	{
		char buf[64];

		__STDIO_THREADLOCK(stderr);
		_stdio_fdout(STDERR_FILENO, s, sep,
					 _glibc_strerror_r(errno, buf, sizeof(buf)));
		__STDIO_THREADUNLOCK(stderr);
	}
#endif
}

#endif
/**********************************************************************/
/* UTILITY funcs */
/**********************************************************************/
#ifdef L__stdio_fdout

/* Not reentrant -- TODO: lock associated stream if a know file descriptor? */

void _stdio_fdout(int fd, ...)
{
	va_list arg;
	register const char *p;

	va_start(arg, fd);
	while ((p = va_arg(arg, const char *)) != NULL) {
		write(fd, p, strlen(p));
	}
	va_end(arg);
}

#endif
/**********************************************************************/
#ifdef L__uintmaxtostr

/* Avoid using long long / and % operations to cut down dependencies on
 * libgcc.a.  Definitely helps on i386 at least. */
#if (INTMAX_MAX > INT_MAX) && (((INTMAX_MAX/INT_MAX)/2) - 2 <= INT_MAX)
#define INTERNAL_DIV_MOD
#endif

#include <locale.h>

char *_uintmaxtostr(register char * __restrict bufend, uintmax_t uval,
					int base, __UIM_CASE alphacase)
{
    int negative;
    unsigned int digit;
#ifdef INTERNAL_DIV_MOD
	unsigned int H, L, high, low, rh;
#endif
#ifndef __LOCALE_C_ONLY
	int grouping, outdigit;
	const char *g;		   /* This does not need to be initialized. */
#endif /* __LOCALE_C_ONLY */

	negative = 0;
	if (base < 0) {				/* signed value */
		base = -base;
		if (uval > INTMAX_MAX) {
			uval = -uval;
			negative = 1;
		}
	}

	/* this is an internal routine -- we shouldn't need to check this */
	assert(!((base < 2) || (base > 36)));

#ifndef __LOCALE_C_ONLY
	grouping = -1;
	outdigit = 0x80 & alphacase;
	alphacase ^= outdigit;
	if (alphacase == __UIM_GROUP) {
		assert(base == 10);
		if (*(g = __UCLIBC_CURLOCALE_DATA.grouping)) {
			grouping = *g;
		}
	}
#endif /* __LOCALE_C_ONLY */

    *bufend = '\0';

#ifndef INTERNAL_DIV_MOD
    do {
#ifndef __LOCALE_C_ONLY
		if (!grouping) {		/* Finished a group. */
			bufend -= __UCLIBC_CURLOCALE_DATA.thousands_sep_len;
			memcpy(bufend, __UCLIBC_CURLOCALE_DATA.thousands_sep,
				   __UCLIBC_CURLOCALE_DATA.thousands_sep_len);
			if (g[1] != 0) { 	/* g[1] == 0 means repeat last grouping. */
				/* Note: g[1] == -1 means no further grouping.  But since
				 * we'll never wrap around, we can set grouping to -1 without
				 * fear of */
				++g;
			}
			grouping = *g;
		}
		--grouping;
#endif /* __LOCALE_C_ONLY */
		digit = uval % base;
		uval /= base;

#ifndef __LOCALE_C_ONLY
		if (unlikely(outdigit)) {
			bufend -= __UCLIBC_CURLOCALE_DATA.outdigit_length[digit];
			memcpy(bufend,
				   (&__UCLIBC_CURLOCALE_DATA.outdigit0_mb)[digit],
				   __UCLIBC_CURLOCALE_DATA.outdigit_length[digit]);
		} else
#endif
		{
			*--bufend = ( (digit < 10) ? digit + '0' : digit + alphacase );
		}
    } while (uval);

#else  /* ************************************************** */

	H = (UINT_MAX / base);
	L = UINT_MAX % base + 1;
	if (L == base) {
		++H;
		L = 0;
	}
	low = (unsigned int) uval;
	high = (unsigned int) (uval >> (sizeof(unsigned int) * CHAR_BIT));

    do {
#ifndef __LOCALE_C_ONLY
		if (!grouping) {		/* Finished a group. */
			bufend -= __UCLIBC_CURLOCALE_DATA.thousands_sep_len;
			memcpy(bufend, __UCLIBC_CURLOCALE_DATA.thousands_sep,
				   __UCLIBC_CURLOCALE_DATA.thousands_sep_len);
			if (g[1] != 0) { 	/* g[1] == 0 means repeat last grouping. */
				/* Note: g[1] == -1 means no further grouping.  But since
				 * we'll never wrap around, we can set grouping to -1 without
				 * fear of */
				++g;
			}
			grouping = *g;
		}
		--grouping;
#endif /* __LOCALE_C_ONLY */

		if (unlikely(high)) {
			rh = high % base;
			high /= base;
			digit = (low % base) + (L * rh);
			low = (low / base) + (H * rh) + (digit / base);
			digit %= base;
		} else {
			digit = low % base;
			low /= base;
		}
		
#ifndef __LOCALE_C_ONLY
		if (unlikely(outdigit)) {
			bufend -= __UCLIBC_CURLOCALE_DATA.outdigit_length[digit];
			memcpy(bufend,
				   (&__UCLIBC_CURLOCALE_DATA.outdigit0_mb)[digit],
				   __UCLIBC_CURLOCALE_DATA.outdigit_length[digit]);
		} else
#endif
		{
			*--bufend = ( (digit < 10) ? digit + '0' : digit + alphacase );
		}
    } while (low | high);

#endif /******************************************************/

    if (negative) {
		*--bufend = '-';
    }

    return bufend;
}
#undef INTERNAL_DIV_MOD

#endif
/**********************************************************************/
#ifdef L__wstdio_fwrite

#include <wchar.h>

size_t _wstdio_fwrite(const wchar_t *__restrict ws, size_t n,
					  register FILE *__restrict stream)
{
	size_t r, count;
	char buf[64];
	const wchar_t *pw;

#if defined(__STDIO_WIDE) && defined(__STDIO_BUFFERS)
	if (stream->filedes == -3) { /* Special case to support {v}swprintf. */
		count = ((wchar_t *)(stream->bufend)) - ((wchar_t *)(stream->bufpos));
		if (count > n) {
			count = n;
		}
		if (count) {
			wmemcpy((wchar_t *)(stream->bufpos), ws, count);
			stream->bufpos = (char *)(((wchar_t *)(stream->bufpos)) + count);
		}
		return n;
	}
#endif /* defined(__STDIO_WIDE) && defined(__STDIO_BUFFERS) */

	if (stream->modeflags & __FLAG_NARROW) {
		stream->modeflags |= __FLAG_ERROR;
		__set_errno(EBADF);
		return 0;
	}
	stream->modeflags |= __FLAG_WIDE;

	pw = ws;
	count = 0;
	while (n > count) {
		r = wcsnrtombs(buf, &pw, n-count, sizeof(buf), &stream->state);
		if (r != ((size_t) -1)) { /* No encoding errors */
			if (!r) {
				++r;		  /* 0 is returned when nul is reached. */
				pw = ws + count + r; /* pw was set to NULL, so correct. */
			}
			if (_stdio_fwrite(buf, r, stream) == r) {
				count = pw - ws;
				continue;
			}
		}
		break;
	}

	/* Note: The count is incorrect if 0 < _stdio_fwrite return < r!!! */
	return count;
}

#endif
/**********************************************************************/
