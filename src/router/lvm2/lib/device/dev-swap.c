/*
 * Copyright (C) 2009 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"
#include "lib/device/dev-type.h"

#ifdef __linux__

#define MAX_PAGESIZE	(64 * 1024)
#define SIGNATURE_SIZE  10

static int _swap_detect_signature(const char *buf)
{
	if (memcmp(buf, "SWAP-SPACE", 10) == 0 ||
            memcmp(buf, "SWAPSPACE2", 10) == 0)
		return 1;

	if (memcmp(buf, "S1SUSPEND", 9) == 0 ||
	    memcmp(buf, "S2SUSPEND", 9) == 0 ||
	    memcmp(buf, "ULSUSPEND", 9) == 0 ||
	    memcmp(buf, "\xed\xc3\x02\xe9\x98\x56\xe5\x0c", 8) == 0)
		return 1;

	return 0;
}

int dev_is_swap(struct device *dev, uint64_t *offset_found, int full)
{
	char buf[10];
	uint64_t size;
	unsigned page;
	int ret = 0;

	if (!scan_bcache)
		return -EAGAIN;

	if (!dev_get_size(dev, &size)) {
		stack;
		return -1;
	}

	for (page = 0x1000; page <= MAX_PAGESIZE; page <<= 1) {
		/*
		 * skip 32k pagesize since this does not seem to be supported
		 */
		if (page == 0x8000)
			continue;
		if (size < (page >> SECTOR_SHIFT))
			break;
		if (!dev_read_bytes(dev, page - SIGNATURE_SIZE, SIGNATURE_SIZE, buf)) {
			ret = -1;
			break;
		}
		if (_swap_detect_signature(buf)) {
			if (offset_found)
				*offset_found = page - SIGNATURE_SIZE;
			ret = 1;
			break;
		}
	}

	return ret;
}

#endif
