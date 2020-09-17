// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "io.h"
#include "bit.h"
#include "output.h"
#include "init.h"
#include "agfl.h"

static int agfl_bno_size(void *obj, int startoff);
static int agfl_f(int argc, char **argv);
static void agfl_help(void);

static const cmdinfo_t agfl_cmd =
	{ "agfl", NULL, agfl_f, 0, 1, 1, N_("[agno]"),
	  N_("set address to agfl block"), agfl_help };

const field_t	agfl_hfld[] = { {
	"", FLDT_AGFL, OI(0), C1, 0, TYP_NONE, },
	{ NULL }
};

const field_t	agfl_crc_hfld[] = { {
	"", FLDT_AGFL_CRC, OI(0), C1, 0, TYP_NONE, },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_agfl, agfl_ ## f))
const field_t	agfl_flds[] = {
	{ "bno", FLDT_AGBLOCKNZ, OI(OFF(magicnum)), agfl_bno_size,
	  FLD_ARRAY|FLD_COUNT, TYP_DATA },
	{ NULL }
};

const field_t	agfl_crc_flds[] = {
	{ "magicnum", FLDT_UINT32X, OI(OFF(magicnum)), C1, 0, TYP_NONE },
	{ "seqno", FLDT_AGNUMBER, OI(OFF(seqno)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(uuid)), C1, 0, TYP_NONE },
	{ "lsn", FLDT_UINT64X, OI(OFF(lsn)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(crc)), C1, 0, TYP_NONE },
	/* the bno array is after the actual structure */
	{ "bno", FLDT_AGBLOCKNZ, OI(bitize(sizeof(struct xfs_agfl))),
	  agfl_bno_size, FLD_ARRAY|FLD_COUNT, TYP_DATA },
	{ NULL }
};

static int
agfl_bno_size(
	void	*obj,
	int	startoff)
{
	return libxfs_agfl_size(mp);
}

static void
agfl_help(void)
{
	dbprintf(_(
"\n"
" set allocation group freelist\n"
"\n"
" Example:\n"
"\n"
" agfl 5"
"\n"
" Located in the fourth sector of each allocation group,\n"
" the agfl freelist for internal btree space allocation is maintained\n"
" for each allocation group.  This acts as a reserved pool of space\n"
" separate from the general filesystem freespace (not used for user data).\n"
"\n"
));

}

static int
agfl_f(
	int		argc,
	char		**argv)
{
	xfs_agnumber_t	agno;
	char		*p;

	if (argc > 1) {
		agno = (xfs_agnumber_t)strtoul(argv[1], &p, 0);
		if (*p != '\0' || agno >= mp->m_sb.sb_agcount) {
			dbprintf(_("bad allocation group number %s\n"), argv[1]);
			return 0;
		}
		cur_agno = agno;
	} else if (cur_agno == NULLAGNUMBER)
		cur_agno = 0;
	ASSERT(typtab[TYP_AGFL].typnm == TYP_AGFL);
	set_cur(&typtab[TYP_AGFL],
		XFS_AG_DADDR(mp, cur_agno, XFS_AGFL_DADDR(mp)),
		XFS_FSS_TO_BB(mp, 1), DB_RING_ADD, NULL);
	return 0;
}

void
agfl_init(void)
{
	add_command(&agfl_cmd);
}

/*ARGSUSED*/
int
agfl_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	return bitize(mp->m_sb.sb_sectsize);
}
