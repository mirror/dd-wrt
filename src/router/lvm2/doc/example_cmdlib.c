/*
 * Copyright (C) 2004 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "tools/lvm2cmd.h"
#include <stdio.h>

/* All output gets passed to this function line-by-line */
void test_log_fn(int level, const char *file, int line,
		 int dm_errno, const char *format)
{
	/* Extract and process output here rather than printing it */

	if (level != 4)
		return;

	printf("%s\n", format);
	return;
}

int main(int argc, char **argv)
{
	void *handle;
	int r;

	lvm2_log_fn(test_log_fn);

	handle = lvm2_init();

	lvm2_log_level(handle, 1);
	r = lvm2_run(handle, "vgs --noheadings vg1");

	/* More commands here */

	lvm2_exit(handle);

	return r;
}

