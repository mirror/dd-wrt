// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs.h"
#include "command.h"
#include "init.h"
#include "output.h"
#include "libfrog/fsgeom.h"
#include "libfrog/logging.h"

static void
info_help(void)
{
	dbprintf(_(
"\n"
" Pretty-prints the filesystem geometry as derived from the superblock.\n"
" The output has the same format as mkfs.xfs, xfs_info, and other utilities.\n"
"\n"
));

}

static int
info_f(
	int			argc,
	char			**argv)
{
	struct xfs_fsop_geom	geo;

	libxfs_fs_geometry(mp, &geo, XFS_FS_GEOM_MAX_STRUCT_VER);
	xfs_report_geom(&geo, fsdevice, x.logname, x.rtname);
	return 0;
}

static const struct cmdinfo info_cmd = {
	.name =		"info",
	.altname =	"i",
	.cfunc =	info_f,
	.argmin =	0,
	.argmax =	0,
	.canpush =	0,
	.args =		NULL,
	.oneline =	N_("pretty-print superblock info"),
	.help =		info_help,
};

static void
agresv_help(void)
{
	dbprintf(_(
"\n"
" Print the size and per-AG reservation information some allocation groups.\n"
"\n"
" Specific allocation group numbers can be provided as command line arguments.\n"
" If no arguments are provided, all allocation groups are iterated.\n"
"\n"
));

}

static void
print_agresv_info(
	struct xfs_perag *pag)
{
	struct xfs_buf	*bp;
	struct xfs_agf	*agf;
	xfs_agnumber_t	agno = pag->pag_agno;
	xfs_extlen_t	ask = 0;
	xfs_extlen_t	used = 0;
	xfs_extlen_t	free = 0;
	xfs_extlen_t	length = 0;
	int		error;

	error = -libxfs_refcountbt_calc_reserves(mp, NULL, pag, &ask, &used);
	if (error)
		xfrog_perror(error, "refcountbt");
	error = -libxfs_finobt_calc_reserves(pag, NULL, &ask, &used);
	if (error)
		xfrog_perror(error, "finobt");
	error = -libxfs_rmapbt_calc_reserves(mp, NULL, pag, &ask, &used);
	if (error)
		xfrog_perror(error, "rmapbt");

	error = -libxfs_read_agf(pag, NULL, 0, &bp);
	if (error)
		xfrog_perror(error, "AGF");
	agf = bp->b_addr;
	length = be32_to_cpu(agf->agf_length);
	free = be32_to_cpu(agf->agf_freeblks) +
	       be32_to_cpu(agf->agf_flcount);
	libxfs_buf_relse(bp);

	printf("AG %d: length: %u free: %u reserved: %u used: %u",
			agno, length, free, ask, used);
	if (ask - used > free)
		printf(" <not enough space>");
	printf("\n");
}

static int
agresv_f(
	int			argc,
	char			**argv)
{
	struct xfs_perag	*pag;
	xfs_agnumber_t		agno;
	int			i;

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			long	a;
			char	*p;

			errno = 0;
			a = strtol(argv[i], &p, 0);
			if (p == argv[i])
				errno = ERANGE;
			if (errno) {
				perror(argv[i]);
				continue;
			}

			if (a < 0 || a >= mp->m_sb.sb_agcount) {
				fprintf(stderr, "%ld: Not a AG.\n", a);
				continue;
			}

			pag = libxfs_perag_get(mp, a);
			print_agresv_info(pag);
			libxfs_perag_put(pag);
		}
		return 0;
	}

	for_each_perag(mp, agno, pag)
		print_agresv_info(pag);

	return 0;
}

static const struct cmdinfo agresv_cmd = {
	.name =		"agresv",
	.altname =	NULL,
	.cfunc =	agresv_f,
	.argmin =	0,
	.argmax =	-1,
	.canpush =	0,
	.args =		NULL,
	.oneline =	N_("print AG reservation stats"),
	.help =		agresv_help,
};

void
info_init(void)
{
	add_command(&info_cmd);
	add_command(&agresv_cmd);
}
