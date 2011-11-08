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

#include <xfs/libxfs.h>
#include "addr.h"
#include "command.h"
#include "type.h"
#include "io.h"
#include "output.h"

static int hash_f(int argc, char **argv);
static void hash_help(void);

static const cmdinfo_t hash_cmd =
	{ "hash", NULL, hash_f, 1, 1, 0, N_("string"),
	  N_("calculate hash value"), hash_help };

static void
hash_help(void)
{
	dbprintf(_(
"\n"
" 'hash' prints out the calculated hash value for a string using the\n"
"directory/attribute code hash function.\n"
"\n"
" Usage:  \"hash <string>\"\n"
"\n"
));

}

/* ARGSUSED */
static int
hash_f(
	int		argc,
	char		**argv)
{
	xfs_dahash_t	hashval;

	hashval = libxfs_da_hashname((uchar_t *)argv[1], (int)strlen(argv[1]));
	dbprintf("0x%x\n", hashval);
	return 0;
}

void
hash_init(void)
{
	add_command(&hash_cmd);
}
