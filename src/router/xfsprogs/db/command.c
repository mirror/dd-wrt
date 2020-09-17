// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "addr.h"
#include "attrset.h"
#include "block.h"
#include "bmap.h"
#include "check.h"
#include "command.h"
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
#include "logformat.h"
#include "metadump.h"
#include "output.h"
#include "print.h"
#include "quit.h"
#include "sb.h"
#include "write.h"
#include "malloc.h"
#include "dquot.h"
#include "fsmap.h"
#include "crc.h"
#include "fuzz.h"

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
	btdump_init();
	btheight_init();
	check_init();
	convert_init();
	crc_init();
	debug_init();
	echo_init();
	frag_init();
	freesp_init();
	fsmap_init();
	help_init();
	hash_init();
	info_init();
	inode_init();
	input_init();
	logres_init();
	logformat_init();
	io_init();
	metadump_init();
	output_init();
	print_init();
	quit_init();
	sb_init();
	type_init();
	write_init();
	dquot_init();
	fuzz_init();
}
