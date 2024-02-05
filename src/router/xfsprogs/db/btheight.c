// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs.h"
#include "command.h"
#include "output.h"
#include "init.h"
#include "io.h"
#include "type.h"
#include "input.h"
#include "libfrog/convert.h"

static int refc_maxrecs(struct xfs_mount *mp, int blocklen, int leaf)
{
	return libxfs_refcountbt_maxrecs(blocklen, leaf != 0);
}

static int rmap_maxrecs(struct xfs_mount *mp, int blocklen, int leaf)
{
	return libxfs_rmapbt_maxrecs(blocklen, leaf);
}

struct btmap {
	const char	*tag;
	unsigned int	(*maxlevels)(void);
	int		(*maxrecs)(struct xfs_mount *mp, int blocklen,
				   int leaf);
} maps[] = {
	{
		.tag		= "bnobt",
		.maxlevels	= libxfs_allocbt_maxlevels_ondisk,
		.maxrecs	= libxfs_allocbt_maxrecs,
	},
	{
		.tag		= "cntbt",
		.maxlevels	= libxfs_allocbt_maxlevels_ondisk,
		.maxrecs	= libxfs_allocbt_maxrecs,
	},
	{
		.tag		= "inobt",
		.maxlevels	= libxfs_iallocbt_maxlevels_ondisk,
		.maxrecs	= libxfs_inobt_maxrecs,
	},
	{
		.tag		= "finobt",
		.maxlevels	= libxfs_iallocbt_maxlevels_ondisk,
		.maxrecs	= libxfs_inobt_maxrecs,
	},
	{
		.tag		= "bmapbt",
		.maxlevels	= libxfs_bmbt_maxlevels_ondisk,
		.maxrecs	= libxfs_bmbt_maxrecs,
	},
	{
		.tag		= "refcountbt",
		.maxlevels	= libxfs_refcountbt_maxlevels_ondisk,
		.maxrecs	= refc_maxrecs,
	},
	{
		.tag		= "rmapbt",
		.maxlevels	= libxfs_rmapbt_maxlevels_ondisk,
		.maxrecs	= rmap_maxrecs,
	},
};

static void
btheight_help(void)
{
	struct btmap	*m;
	int		i;

	dbprintf(_(
"\n"
" For a given number of btree records and a btree type, report the number of\n"
" records and blocks for each level of the btree, and the total btree size.\n"
" The btree type must be given after the options.  A raw btree geometry can\n"
" be provided in the format \"record_bytes:key_bytes:ptr_bytes:header_type\"\n"
" where header_type is one of \"short\", \"long\", \"shortcrc\", or \"longcrc\".\n"
"\n"
" Options:\n"
"   -b -- Override the btree block size.\n"
"   -n -- Number of records we want to store.\n"
"   -w max -- Show only the best case scenario.\n"
"   -w min -- Show only the worst case scenario.\n"
"   -w absmax -- Print the maximum possible btree height for all filesystems.\n"
"\n"
" Supported btree types:\n"
"   all "
));
	for (i = 0, m = maps; i < ARRAY_SIZE(maps); i++, m++)
		printf("%s ", m->tag);
	printf("\n");
}

static void
calc_height(
	unsigned long long	nr_records,
	uint			*records_per_block)
{
	unsigned int		level = 0;
	unsigned long long	total_blocks = 0;
	unsigned long long	blocks;
	char			*levels_suffix = "s";
	char			*totblocks_suffix = "s";

	while (nr_records) {
		unsigned int	level_rpb = records_per_block[level != 0];
		char		*recs_suffix = "s";
		char		*blocks_suffix = "s";

		blocks = (nr_records + level_rpb - 1) / level_rpb;

		if (nr_records == 1)
			recs_suffix = "";
		if (blocks == 1)
			blocks_suffix = "";

		printf(_("level %u: %llu record%s, %llu block%s\n"),
				level, nr_records, recs_suffix, blocks,
				blocks_suffix);

		total_blocks += blocks;
		nr_records = blocks == 1 ? 0 : blocks;
		level++;
	}

	if (level == 1)
		levels_suffix = "";
	if (total_blocks == 1)
		totblocks_suffix = "";

	printf(_("%u level%s, %llu block%s total\n"), level, levels_suffix,
			total_blocks, totblocks_suffix);
}

static int
construct_records_per_block(
	const char	*tag,
	int		blocksize,
	unsigned int	*records_per_block)
{
	char		*toktag;
	struct btmap	*m;
	unsigned int	record_size, key_size, ptr_size;
	char		*p;
	int		i, ret;

	for (i = 0, m = maps; i < ARRAY_SIZE(maps); i++, m++) {
		if (!strcmp(m->tag, tag)) {
			records_per_block[0] = m->maxrecs(mp, blocksize, 1);
			records_per_block[1] = m->maxrecs(mp, blocksize, 0);
			return 0;
		}
	}

	toktag = strdup(tag);
	ret = -1;

	p = strtok(toktag, ":");
	if (!p) {
		fprintf(stderr, _("%s: record size not found.\n"), tag);
		goto out;
	}
	record_size = cvt_u16(p, 0);
	if (errno) {
		perror(p);
		goto out;
	}
	if (record_size == 0) {
		fprintf(stderr, _("%s: record size cannot be zero.\n"), tag);
		goto out;
	}

	p = strtok(NULL, ":");
	if (!p) {
		fprintf(stderr, _("%s: key size not found.\n"), tag);
		goto out;
	}
	key_size = cvt_u16(p, 0);
	if (errno) {
		perror(p);
		goto out;
	}
	if (key_size == 0) {
		fprintf(stderr, _("%s: key size cannot be zero.\n"), tag);
		goto out;
	}

	p = strtok(NULL, ":");
	if (!p) {
		fprintf(stderr, _("%s: pointer size not found.\n"), tag);
		goto out;
	}
	ptr_size = cvt_u16(p, 0);
	if (errno) {
		perror(p);
		goto out;
	}
	if (ptr_size == 0) {
		fprintf(stderr, _("%s: pointer size cannot be zero.\n"), tag);
		goto out;
	}

	p = strtok(NULL, ":");
	if (!p) {
		fprintf(stderr, _("%s: header type not found.\n"), tag);
		goto out;
	}
	if (!strcmp(p, "short"))
		blocksize -= XFS_BTREE_SBLOCK_LEN;
	else if (!strcmp(p, "shortcrc"))
		blocksize -= XFS_BTREE_SBLOCK_CRC_LEN;
	else if (!strcmp(p, "long"))
		blocksize -= XFS_BTREE_LBLOCK_LEN;
	else if (!strcmp(p, "longcrc"))
		blocksize -= XFS_BTREE_LBLOCK_CRC_LEN;
	else {
		fprintf(stderr, _("%s: unrecognized btree header type."),
				p);
		goto out;
	}

	if (record_size > blocksize) {
		fprintf(stderr,
_("%s: record size must be less than selected block size (%u bytes).\n"),
			tag, blocksize);
		goto out;
	}

	if (key_size > blocksize) {
		fprintf(stderr,
_("%s: key size must be less than selected block size (%u bytes).\n"),
			tag, blocksize);
		goto out;
	}

	if (ptr_size > blocksize) {
		fprintf(stderr,
_("%s: pointer size must be less than selected block size (%u bytes).\n"),
			tag, blocksize);
		goto out;
	}

	p = strtok(NULL, ":");
	if (p) {
		fprintf(stderr,
			_("%s: unrecognized raw btree geometry."),
				tag);
		goto out;
	}

	records_per_block[0] = blocksize / record_size;
	records_per_block[1] = blocksize / (key_size + ptr_size);
	ret = 0;
out:
	free(toktag);
	return ret;
}

#define REPORT_DEFAULT	(-1U)
#define REPORT_MAX	(1 << 0)
#define REPORT_MIN	(1 << 1)
#define REPORT_ABSMAX	(1 << 2)

static void
report_absmax(const char *tag)
{
	struct btmap	*m;
	int		i;

	for (i = 0, m = maps; i < ARRAY_SIZE(maps); i++, m++) {
		if (!strcmp(m->tag, tag)) {
			printf("%s: %u\n", tag, m->maxlevels());
			return;
		}
	}
	printf(_("%s: Don't know how to report max height.\n"), tag);
}

static void
report(
	const char		*tag,
	unsigned int		report_what,
	unsigned long long	nr_records,
	unsigned int		blocksize)
{
	unsigned int		records_per_block[2];
	int			ret;

	if (report_what == REPORT_ABSMAX) {
		report_absmax(tag);
		return;
	}

	ret = construct_records_per_block(tag, blocksize, records_per_block);
	if (ret)
		return;

	if (report_what & REPORT_MAX) {
		if (records_per_block[0] < 2) {
			fprintf(stderr,
_("%s: cannot calculate best case scenario due to leaf geometry underflow.\n"),
				tag);
			return;
		}

		if (records_per_block[1] < 4) {
			fprintf(stderr,
_("%s: cannot calculate best case scenario due to node geometry underflow.\n"),
				tag);
			return;
		}

		printf(
_("%s: best case per %u-byte block: %u records (leaf) / %u keyptrs (node)\n"),
				tag, blocksize, records_per_block[0],
				records_per_block[1]);

		calc_height(nr_records, records_per_block);
	}

	if (report_what & REPORT_MIN) {
		records_per_block[0] /= 2;
		records_per_block[1] /= 2;

		if (records_per_block[0] < 1) {
			fprintf(stderr,
_("%s: cannot calculate worst case scenario due to leaf geometry underflow.\n"),
				tag);
			return;
		}

		if (records_per_block[1] < 2) {
			fprintf(stderr,
_("%s: cannot calculate worst case scenario due to node geometry underflow.\n"),
				tag);
			return;
		}

		printf(
_("%s: worst case per %u-byte block: %u records (leaf) / %u keyptrs (node)\n"),
				tag, blocksize, records_per_block[0],
				records_per_block[1]);

		calc_height(nr_records, records_per_block);
	}
}

static void
report_all(
	unsigned int		report_what,
	unsigned long long	nr_records,
	unsigned int		blocksize)
{
	struct btmap		*m;
	int			i;

	for (i = 0, m = maps; i < ARRAY_SIZE(maps); i++, m++)
		report(m->tag, report_what, nr_records, blocksize);
}

static int
btheight_f(
	int		argc,
	char		**argv)
{
	long long	blocksize = mp->m_sb.sb_blocksize;
	uint64_t	nr_records = 0;
	int		report_what = REPORT_DEFAULT;
	int		i, c;

	while ((c = getopt(argc, argv, "b:n:w:")) != -1) {
		switch (c) {
		case 'b':
			errno = 0;
			blocksize = cvtnum(mp->m_sb.sb_blocksize,
					mp->m_sb.sb_sectsize,
					optarg);
			if (errno) {
				perror(optarg);
				return 0;
			}
			break;
		case 'n':
			nr_records = cvt_u64(optarg, 0);
			if (errno) {
				perror(optarg);
				return 0;
			}
			break;
		case 'w':
			if (!strcmp(optarg, "min"))
				report_what = REPORT_MIN;
			else if (!strcmp(optarg, "max"))
				report_what = REPORT_MAX;
			else if (!strcmp(optarg, "absmax"))
				report_what = REPORT_ABSMAX;
			else {
				btheight_help();
				return 0;
			}
			break;
		default:
			btheight_help();
			return 0;
		}
	}

	if (report_what != REPORT_ABSMAX && nr_records == 0) {
		fprintf(stderr,
_("Number of records must be greater than zero.\n"));
		return 0;
	}

	if (report_what != REPORT_ABSMAX && blocksize > INT_MAX) {
		fprintf(stderr,
_("The largest block size this command will consider is %u bytes.\n"),
			INT_MAX);
		return 0;
	}

	if (report_what != REPORT_ABSMAX && blocksize < 128) {
		fprintf(stderr,
_("The smallest block size this command will consider is 128 bytes.\n"));
		return 0;
	}

	if (argc == optind) {
		btheight_help();
		return 0;
	}

	for (i = optind; i < argc; i++) {
		if (!strcmp(argv[i], "all")) {
			report_all(report_what, nr_records, blocksize);
			return 0;
		}
	}

	for (i = optind; i < argc; i++)
		report(argv[i], report_what, nr_records, blocksize);

	return 0;
}

static const cmdinfo_t btheight_cmd =
	{ "btheight", "b", btheight_f, 1, -1, 0,
	  "[-b blksz] [-n recs] [-w max|-w min] btree types...",
	  N_("compute btree heights"), btheight_help };

void
btheight_init(void)
{
	add_command(&btheight_cmd);
}
