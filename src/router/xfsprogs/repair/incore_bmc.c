// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "avl.h"
#include "globals.h"
#include "incore.h"
#include "agheader.h"
#include "protos.h"
#include "err_protos.h"

void
init_bm_cursor(bmap_cursor_t *cursor, int num_levels)
{
	int i;

	memset(cursor, 0, sizeof(bmap_cursor_t));
	cursor->ino = NULLFSINO;
	cursor->num_levels = num_levels;

	for (i = 0; i < XR_MAX_BMLEVELS; i++)  {
		cursor->level[i].fsbno = NULLFSBLOCK;
		cursor->level[i].right_fsbno = NULLFSBLOCK;
		cursor->level[i].left_fsbno = NULLFSBLOCK;
		cursor->level[i].first_key = NULLFILEOFF;
		cursor->level[i].last_key = NULLFILEOFF;
	}
}
