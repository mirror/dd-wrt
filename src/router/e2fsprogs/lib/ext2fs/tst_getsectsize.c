/*
 * tst_getsize.c --- this function tests the getsize function
 * 
 * Copyright (C) 1997 by Theodore Ts'o.
 * 
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#if HAVE_ERRNO_H
#include <errno.h>
#endif

#include "ext2_fs.h"
#include "ext2fs.h"

int main(int argc, char **argv)
{
	int	sectsize;
	int	retval;
	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		exit(1);
	}

	retval = ext2fs_get_device_sectsize(argv[1], &sectsize);
	if (retval) {
		com_err(argv[0], retval,
			"while calling ext2fs_get_device_sectsize");
		exit(1);
	}
	printf("Device %s has a hardware sector size of %d.\n",
	       argv[1], sectsize);
	exit(0);
}
