/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
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
 *
 * Test sample code to check for leftovers from secure table loading in
 * userspace memory (initial sample provided by Milan Broz).
 *
 * Compile with:  gcc -O2 -g -o tst dmcrypt.c -ldevmapper
 *
 * Search for string in coredump (needs 'raise', or using 'gcore' tool)
 *
 * grep "434e0cbab02ca68ffba9268222c3789d703fe62427b78b308518b3228f6a2122" core
 *
 */

#include "device_mapper/all.h"

#include <unistd.h>
#include <signal.h>

/* Comment out this define to get coredump instead of sleeping */
#define SLEEP 1

static void rot13(char *s)
{
	unsigned i;

	for (i = 0; s[i]; i++)
		if (s[i] >= 'a' && s[i] <= 'm')
			s[i] += 13;
		else if (s[i] >= 'n' && s[i] <= 'z')
			s[i] -= 13;
}

int main (int argc, char *argv[])
{
	const unsigned sz = 8192;
	/* rot13: 434e0cbab02ca68ffba9268222c3789d703fe62427b78b308518b3228f6a2122  */
	char aes[] = "434r0pono02pn68sson9268222p3789q703sr62427o78o308518o3228s6n2122";
	const char *device = (argc > 1) ? argv[1] : "/dev/loop0";  /* device for use */
	const char *devname = (argc > 2) ? argv[2] : "test-secure"; /* name of dm device */
	uint32_t cookie = 0;
	char table[300];
	struct dm_task *dmt;

	if (geteuid() != 0) {
		fprintf(stderr, "Needs root UID for execution!\n");
		exit(1);
	}

	printf("Going to create %s dm device using backend device: %s\n", devname, device);

	if ((dmt = dm_task_create(DM_DEVICE_CREATE))) {
		(void) dm_task_set_name(dmt, devname);
		(void) dm_task_secure_data(dmt);
		rot13(aes);
		snprintf(table, sizeof(table), "aes-xts-plain64 %s 0 %s %u", aes, device, sz);
		memset(aes, 0, sizeof(aes));
		(void) dm_task_add_target(dmt, 0, sz, "crypt", table);
		memset(table, 0, sizeof(table));
		asm volatile ("" ::: "memory");/* Compiler barrier. */
		(void) dm_task_set_cookie(dmt, &cookie, DM_UDEV_DISABLE_LIBRARY_FALLBACK);
		(void) dm_task_run(dmt);
		(void) dm_task_destroy(dmt);
	}

	dm_task_update_nodes();

	/* At this point there should be no memory trace from a secure table line */

#ifdef SLEEP
	sleep(4);	/* Give time to other process to capture  'gcore pid' */
#else
	raise(SIGABRT); /* Generate core for search of any forgotten traces of key */
#endif
	return 0;
}
