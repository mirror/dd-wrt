// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include "platform_defs.h"
#include "util.h"

/*
 * libfrog is a collection of miscellaneous userspace utilities.
 * It's a library of Funny Random Oddball Gunk <cough>.
 */

unsigned int
log2_roundup(unsigned int i)
{
	unsigned int	rval;

	for (rval = 0; rval < NBBY * sizeof(i); rval++) {
		if ((1 << rval) >= i)
			break;
	}
	return rval;
}
