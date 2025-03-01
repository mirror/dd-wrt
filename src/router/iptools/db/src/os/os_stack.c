/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#if defined(HAVE_SYSTEM_INCLUDE_FILES) && defined(HAVE_BACKTRACE) && \
    defined(HAVE_BACKTRACE_SYMBOLS) && defined(HAVE_EXECINFO_H)
#include <execinfo.h>
#endif

#undef __DB_STACK_MAXFRAMES
#define	__DB_STACK_MAXFRAMES	25

/*
 * __os_stack --
 *	Output a stack trace in a single write to the error file handle.
 *
 * PUBLIC: void __os_stack __P((const ENV *));
 */
void
__os_stack(env)
	const ENV *env;
{
	/* Adjust by 2 to exclude __os_stack() and __os_stack_top(). */
	__os_stack_top(env, __DB_STACK_MAXFRAMES - 2, 2);
}

/*
 * __os_stack_top --
 *	Output just a certain range of stack frames to the error file handle.
 *
 * PUBLIC: void __os_stack_top __P((const ENV *, unsigned, unsigned));
 */
void
__os_stack_top(env, nframes, skipframes)
	const ENV *env;
	unsigned nframes;
	unsigned skipframes;
{
#if defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS)
	char buf[__DB_STACK_MAXFRAMES * 80];	/* Allow for 80 chars/line. */

	__os_stack_text(env, buf, sizeof(buf), nframes, skipframes + 1);
	__db_errx(env, "Top of stack:\n%s", buf);
#else
	COMPQUIET(env, NULL);
	COMPQUIET(nframes, 0);
	COMPQUIET(skipframes, 0);
#endif
}

/*
 * __os_stack_text --
 *	'Print' the current stack into a char text buffer.
 *
 * PUBLIC: void __os_stack_text
 * PUBLIC:     __P((const ENV *, char *, size_t, unsigned, unsigned));
 */
void
__os_stack_text(env, result, bufsize, nframes, skip)
	const ENV *env;
	char *result;
	size_t bufsize;
	unsigned nframes;
	unsigned skip;
{
	DB_MSGBUF mb;

	DB_MSGBUF_INIT_BUFFER(&mb, result, bufsize);
	__os_stack_msgadd(env, &mb, nframes, skip, NULL);
}

/*
 * __os_stack_save --
 *	Save a certain range of stack frames into the frames argument.
 *
 * PUBLIC: int __os_stack_save __P((const ENV *, unsigned, void **));
 */
int
__os_stack_save(env, nframes, frames)
	const ENV *env;
	unsigned nframes;
	void **frames;
{
	COMPQUIET(env, NULL);
#if defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS)
	/*
	 * Solaris and the GNU C library support this interface.  Solaris
	 * has additional interfaces (printstack and walkcontext), I don't
	 * know if they offer any additional value or not.
	 */
	return ((int) backtrace(frames, nframes));
#else
	COMPQUIET(nframes, 0);
	COMPQUIET(frames, NULL);
	return (0);
#endif
}

/*
 * __os_stack_msgadd --
 *	Decode a stack and add it to a DB_MSGBUF. The stack was either
 *	previously obtained stack, e.g., from __os_stack_save(), or if it is
 *	null, the current stack is fetched here.
 *
 * PUBLIC: void __os_stack_msgadd
 * PUBLIC:       __P((const ENV *, DB_MSGBUF *, unsigned, unsigned, void **));
 */
void
__os_stack_msgadd(env, mb, totalframes, skipframes, stack)
	const ENV *env;
	DB_MSGBUF *mb;
	unsigned totalframes;
	unsigned skipframes;
	void **stack;
{
#if defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS)
	char **strings;
	void *local_frames[__DB_STACK_MAXFRAMES];
	unsigned i;

	if (stack == NULL) {
		stack = local_frames;
		if (totalframes > __DB_STACK_MAXFRAMES)
			totalframes = __DB_STACK_MAXFRAMES;
		totalframes = backtrace(local_frames, totalframes);
		skipframes++;
	}

	/*
	 * Solaris and the GNU C library support this interface.  Solaris
	 * has additional interfaces (printstack and walkcontext) which have
	 * know if they offer any additional value or not.
	 */
	strings = backtrace_symbols(stack, totalframes);

	for (i = skipframes; i < totalframes; ++i)
		__db_msgadd((ENV *)env, mb, "\t%s\n", strings[i]);
	free(strings);
#else
	COMPQUIET(env, NULL);
	COMPQUIET(mb, NULL);
	COMPQUIET(totalframes, 0);
	COMPQUIET(skipframes, 0);
	COMPQUIET(stack, NULL);
#endif
}
