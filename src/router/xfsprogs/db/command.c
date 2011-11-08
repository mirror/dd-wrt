/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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
#include "attrset.h"
#include "block.h"
#include "bmap.h"
#include "check.h"
#include "command.h"
#include "convert.h"
#include "debug.h"
#include "type.h"
#include "echo.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "agf.h"
#include "agfl.h"
#include "agi.h"
#include "frag.h"
#include "freesp.h"
#include "help.h"
#include "hash.h"
#include "inode.h"
#include "input.h"
#include "io.h"
#include "metadump.h"
#include "output.h"
#include "print.h"
#include "quit.h"
#include "sb.h"
#include "write.h"
#include "malloc.h"
#include "dquot.h"

cmdinfo_t	*cmdtab;
int		ncmds;

static int
cmd_compare(const void *a, const void *b)
{
	return strcmp(((const cmdinfo_t *)a)->name,
		      ((const cmdinfo_t *)b)->name);
}

void
add_command(
	const cmdinfo_t	*ci)
{
	cmdtab = xrealloc((void *)cmdtab, ++ncmds * sizeof(*cmdtab));
	cmdtab[ncmds - 1] = *ci;
	qsort(cmdtab, ncmds, sizeof(*cmdtab), cmd_compare);
}

int
command(
	int		argc,
	char		**argv)
{
	char		*cmd;
	const cmdinfo_t	*ct;

	cmd = argv[0];
	ct = find_command(cmd);
	if (ct == NULL) {
		dbprintf(_("command %s not found\n"), cmd);
		return 0;
	}
	if (argc-1 < ct->argmin || (ct->argmax != -1 && argc-1 > ct->argmax)) {
		dbprintf(_("bad argument count %d to %s, expected "), argc-1, cmd);
		if (ct->argmax == -1)
			dbprintf(_("at least %d"), ct->argmin);
		else if (ct->argmin == ct->argmax)
			dbprintf("%d", ct->argmin);
		else
			dbprintf(_("between %d and %d"), ct->argmin, ct->argmax);
		dbprintf(_(" arguments\n"));
		return 0;
	}
	platform_getoptreset();
	return ct->cfunc(argc, argv);
}

const cmdinfo_t *
find_command(
	const char	*cmd)
{
	cmdinfo_t	*ct;

	for (ct = cmdtab; ct < &cmdtab[ncmds]; ct++) {
		if (strcmp(ct->name, cmd) == 0 ||
		    (ct->altname && strcmp(ct->altname, cmd) == 0))
			return (const cmdinfo_t *)ct;
	}
	return NULL;
}

void
init_commands(void)
{
	addr_init();
	agf_init();
	agfl_init();
	agi_init();
	attrset_init();
	block_init();
	bmap_init();
	check_init();
	convert_init();
	debug_init();
	echo_init();
	frag_init();
	freesp_init();
	help_init();
	hash_init();
	inode_init();
	input_init();
	io_init();
	metadump_init();
	output_init();
	print_init();
	quit_init();
	sb_init();
	type_init();
	write_init();
	dquot_init();
}
