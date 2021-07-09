/*
 * combine.c
 *
 * Copyright (C) 2005 - 2021 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include <stdio.h>
#define BASEDEF 1
/* GoAhead 2.1 Embedded JavaScript compatibility */

#include "webs.h"
#include "nvram_backup.c"
#ifdef HAVE_WIREGUARD
#include "wireguard.c"
#endif
#include "cgi.c"
#include "ej.c"
#include "base.c"

#ifdef STATIC_BUILD
#include "callvalidate_static.c"
#else
#include "callvalidate.c"
#endif
#ifdef HAVE_UPX86
#include "upgrade_x86.c"
#else
#include "upgrade.c"
#endif
