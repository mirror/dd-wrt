/* A secure tmpfile() replacement.
 *
 * Copyright (c) 2015-2020  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <paths.h>
#include <fcntl.h>		/* O_TMPFILE requires -D_GNU_SOURCE */
#include <stdio.h>		/* fdopen() */
#include <sys/stat.h>		/* umask() */

/**
 * tempfile - A secure tmpfile() replacement
 *
 * This is the secure replacement for tmpfile() that does not exist in
 * GLIBC.  The function uses the Linux specific %O_TMPFILE and %O_EXCL
 * for security.  When the %FILE is fclose()'ed the file contents is
 * lost.  The file is hidden in the %_PATH_TMP directory on the system.
 *
 * This function requires Linux 3.11, or later, due to %O_TMPFILE.
 *
 * Returns:
 * An open %FILE pointer, or %NULL on error.
 */
FILE *tempfile(void)
{
#ifdef O_TMPFILE	  /* Only on Linux, with fairly recent (G)LIBC */
	mode_t oldmask;
	int fd;

	oldmask = umask(0077);
	fd = open(_PATH_TMP, O_TMPFILE | O_RDWR | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR);
	umask(oldmask);
	if (-1 == fd)
		return NULL;

	return fdopen(fd, "w+");
#else
	return tmpfile(); /* Fallback on older GLIBC/Linux and actual UNIX systems */
#endif
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
