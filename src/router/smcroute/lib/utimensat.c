/* Replacement in case utimensat(2) is missing
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
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

#include "config.h"

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <time.h>
#include <sys/time.h>		/* lutimes(), utimes(), utimensat() */

int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags)
{
	struct timespec ts[2];
	struct timeval tv[2];
	int ret = -1;

	if (dirfd != 0) {
		errno = ENOTSUP;
		return -1;
	}

	if (!times) {
		clock_gettime(CLOCK_REALTIME, &ts[0]);
		ts[1] = ts[0];
	} else {
		ts[0] = times[0];
		ts[1] = times[1];
	}

	TIMESPEC_TO_TIMEVAL(&tv[0], &ts[0]);
	TIMESPEC_TO_TIMEVAL(&tv[1], &ts[1]);

#ifdef AT_SYMLINK_NOFOLLOW
	if ((flags & AT_SYMLINK_NOFOLLOW) == AT_SYMLINK_NOFOLLOW)
		ret = lutimes(pathname, tv);
	else
#endif
		ret = utimes(pathname, tv);

	return ret;
}

#ifdef UNITTEST
#include <err.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char *fn;

	if (argc < 2)
		errx(1, "Usage: touch FILENAME");
	fn = argv[1];

	if (access(fn, F_OK)) {
		FILE *fp;

		fp = fopen(fn, "w");
		if (!fp)
			err(1, "Failed creating %s", fn);
		fclose(fp);
	}
	utimensat(0, fn, NULL, 0);

	return 0;
}
#endif
/**
 * Local Variables:
 *  compile-command: "gcc -W -Wall -Wextra -I.. -DUNITTEST -o touch utimensat.c"
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
