/*
 * rsync.h
 *
 * Copyright (C) 2026 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#ifndef _rsync_h_
#define _rsync_h_
#include <jansson.h>

struct rsync_share {
	char mp[32];
	char sd[64];
	char label[64];
	struct rsync_share *next;
};

struct rsync_share *getrsyncshares(void);
struct rsync_share *getrsyncshare(char *mp, char *sd, char *label);

#endif
