// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "addr.h"
#include "command.h"
#include "type.h"
#include "io.h"
#include "output.h"
#include "hash.h"

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

	hashval = libxfs_da_hashname((unsigned char *)argv[1], (int)strlen(argv[1]));
	dbprintf("0x%x\n", hashval);
	return 0;
}

void
hash_init(void)
{
	add_command(&hash_cmd);
}
