/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <libxfs.h>
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
		cursor->level[i].fsbno = NULLDFSBNO;
		cursor->level[i].right_fsbno = NULLDFSBNO;
		cursor->level[i].left_fsbno = NULLDFSBNO;
		cursor->level[i].first_key = NULLDFILOFF;
		cursor->level[i].last_key = NULLDFILOFF;
	}
}
