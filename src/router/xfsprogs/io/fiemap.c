/*
 * Copyright (c) 2010 Red Hat, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <linux/fiemap.h>
#include <linux/fs.h>
#include "init.h"
#include "io.h"

static cmdinfo_t fiemap_cmd;

static void
fiemap_help(void)
{
	printf(_(
"\n"
" prints the block mapping for a file's data or attribute forks"
"\n"
" Example:\n"
" 'fiemap -v' - tabular format verbose map\n"
"\n"
" fiemap prints the map of disk blocks used by the current file.\n"
" The map lists each extent used by the file, as well as regions in the\n"
" file that do not have any corresponding blocks (holes).\n"
" By default, each line of the listing takes the following form:\n"
"     extent: [startoffset..endoffset]: startblock..endblock\n"
" Holes are marked by replacing the startblock..endblock with 'hole'.\n"
" All the file offsets and disk blocks are in units of 512-byte blocks.\n"
" -a -- prints the attribute fork map instead of the data fork.\n"
" -l -- also displays the length of each extent in 512-byte blocks.\n"
" -n -- query n extents.\n"
" -v -- Verbose information\n"
"\n"));
}

static int
numlen(
	__u64	val,
	int	base)
{
	__u64	tmp;
	int	len;

	for (len = 0, tmp = val; tmp > 0; tmp = tmp/base)
		len++;
	return (len == 0 ? 1 : len);
}

static void
print_verbose(
	struct fiemap_extent	*extent,
	int			blocksize,
	int			foff_w,
	int			boff_w,
	int			tot_w,
	int			flg_w,
	int			max_extents,
	int			*cur_extent,
	__u64			*last_logical)
{
	__u64			lstart;
	__u64			len;
	__u64			block;
	char			lbuf[48];
	char			bbuf[48];
	char			flgbuf[16];

	lstart = extent->fe_logical / blocksize;
	len = extent->fe_length / blocksize;
	block = extent->fe_physical / blocksize;

	memset(lbuf, 0, sizeof(lbuf));
	memset(bbuf, 0, sizeof(bbuf));

	if (lstart != *last_logical) {
		snprintf(lbuf, sizeof(lbuf), "[%llu..%llu]:", *last_logical,
			 lstart - 1ULL);
		printf("%4d: %-*s %-*s %*llu\n", *cur_extent, foff_w, lbuf,
		       boff_w, _("hole"), tot_w, lstart - *last_logical);
		(*cur_extent)++;
		memset(lbuf, 0, sizeof(lbuf));
	}

	if ((*cur_extent + 1) == max_extents)
		return;

	snprintf(lbuf, sizeof(lbuf), "[%llu..%llu]:", lstart,
		 lstart + len - 1ULL);
	snprintf(bbuf, sizeof(bbuf), "%llu..%llu", block, block + len - 1ULL);
	snprintf(flgbuf, sizeof(flgbuf), "0x%x", extent->fe_flags);
	printf("%4d: %-*s %-*s %*llu %*s\n", *cur_extent, foff_w, lbuf,
	       boff_w, bbuf, tot_w, len, flg_w, flgbuf);

	(*cur_extent)++;
	*last_logical = lstart + len;
}

static void
print_plain(
	struct fiemap_extent	*extent,
	int			lflag,
	int			blocksize,
	int			max_extents,
	int			*cur_extent,
	__u64			*last_logical)
{
	__u64			lstart;
	__u64			block;
	__u64			len;

	lstart = extent->fe_logical / blocksize;
	len = extent->fe_length / blocksize;
	block = extent->fe_physical / blocksize;

	if (lstart != *last_logical) {
		printf("\t%d: [%llu..%llu]: hole", *cur_extent,
		       *last_logical, lstart - 1ULL);
		if (lflag)
			printf(_(" %llu blocks\n"),
			       lstart - *last_logical);
		else
			printf("\n");
		(*cur_extent)++;
	}

	if ((*cur_extent + 1) == max_extents)
		return;

	printf("\t%d: [%llu..%llu]: %llu..%llu", *cur_extent,
	       lstart, lstart + len - 1ULL, block,
	       block + len - 1ULL);

	if (lflag)
		printf(_(" %llu blocks\n"), len);
	else
		printf("\n");
	(*cur_extent)++;
	*last_logical = lstart + len;
}

int
fiemap_f(
	int		argc,
	char		**argv)
{
	struct fiemap	*fiemap;
	int		max_extents = 0;
	int		num_extents = 32;
	int		last = 0;
	int		lflag = 0;
	int		vflag = 0;
	int		fiemap_flags = FIEMAP_FLAG_SYNC;
	int		c;
	int		i;
	int		map_size;
	int		ret;
	int		cur_extent = 0;
	int		foff_w = 16;	/* 16 just for a good minimum range */
	int		boff_w = 16;
	int		tot_w = 5;	/* 5 since its just one number */
	int		flg_w = 5;
	__u64		blocksize = 512;
	__u64		last_logical = 0;
	struct stat	st;

	while ((c = getopt(argc, argv, "aln:v")) != EOF) {
		switch (c) {
		case 'a':
			fiemap_flags |= FIEMAP_FLAG_XATTR;
			break;
		case 'l':
			lflag = 1;
			break;
		case 'n':
			max_extents = atoi(optarg);
			break;
		case 'v':
			vflag++;
			break;
		default:
			return command_usage(&fiemap_cmd);
		}
	}

	if (max_extents)
		num_extents = min(num_extents, max_extents);
	map_size = sizeof(struct fiemap) +
		(num_extents * sizeof(struct fiemap_extent));
	fiemap = malloc(map_size);
	if (!fiemap) {
		fprintf(stderr, _("%s: malloc of %d bytes failed.\n"),
			progname, map_size);
		exitcode = 1;
		return 0;
	}

	printf("%s:\n", file->name);

	if (vflag) {
		for (i = 0; i < fiemap->fm_mapped_extents; i++) {
			char			lbuf[32];
			char			bbuf[32];
			__u64			logical;
			__u64			block;
			__u64			len;
			struct fiemap_extent	*extent;

			extent = &fiemap->fm_extents[i];
			logical = extent->fe_logical / blocksize;
			len = extent->fe_length / blocksize;
			block = extent->fe_physical / blocksize;

			snprintf(lbuf, sizeof(lbuf), "[%llu..%llu]", logical,
				 logical + len - 1);
			snprintf(bbuf, sizeof(bbuf), "%llu..%llu", block,
				 block + len - 1);
			foff_w = max(foff_w, strlen(lbuf));
			boff_w = max(boff_w, strlen(bbuf));
			tot_w = max(tot_w, numlen(len, 10));
			flg_w = max(flg_w, numlen(extent->fe_flags, 16));
			if (extent->fe_flags & FIEMAP_EXTENT_LAST)
				break;
		}
		printf("%4s: %-*s %-*s %*s %*s\n", _("EXT"),
		       foff_w, _("FILE-OFFSET"),
		       boff_w, _("BLOCK-RANGE"),
		       tot_w, _("TOTAL"),
		       flg_w, _("FLAGS"));
	}

	while (!last && ((cur_extent + 1) != max_extents)) {
		if (max_extents)
			num_extents = min(num_extents,
					  max_extents - (cur_extent + 1));

		memset(fiemap, 0, map_size);
		fiemap->fm_flags = fiemap_flags;
		fiemap->fm_start = last_logical;
		fiemap->fm_length = -1;
		fiemap->fm_extent_count = num_extents;

		ret = ioctl(file->fd, FS_IOC_FIEMAP, (unsigned long)fiemap);
		if (ret < 0) {
			fprintf(stderr, "%s: ioctl(FS_IOC_FIEMAP) [\"%s\"]: "
				"%s\n", progname, file->name, strerror(errno));
			free(fiemap);
			exitcode = 1;
			return 0;
		}

		/* No more extents to map, exit */
		if (!fiemap->fm_mapped_extents)
			break;

		for (i = 0; i < fiemap->fm_mapped_extents; i++) {
			struct fiemap_extent	*extent;

			extent = &fiemap->fm_extents[i];
			if (vflag)
				print_verbose(extent, blocksize, foff_w,
					      boff_w, tot_w, flg_w,
					      max_extents, &cur_extent,
					      &last_logical);
			else
				print_plain(extent, lflag, blocksize,
					    max_extents, &cur_extent,
					    &last_logical);
			if (extent->fe_flags & FIEMAP_EXTENT_LAST) {
				last = 1;
				break;
			}

			if ((cur_extent + 1) == max_extents)
				break;
		}
	}

	if ((cur_extent + 1) == max_extents)
		goto out;

	memset(&st, 0, sizeof(st));
	if (fstat(file->fd, &st)) {
		fprintf(stderr, "%s: fstat failed: %s\n", progname,
			strerror(errno));
		free(fiemap);
		exitcode = 1;
		return 0;
	}

	if (cur_extent && last_logical < (st.st_size / blocksize)) {
		char	lbuf[32];

		snprintf(lbuf, sizeof(lbuf), "[%llu..%llu]:",
			 last_logical, (st.st_size / blocksize) - 1);
		if (vflag) {
			printf("%4d: %-*s %-*s %*llu\n", cur_extent,
			       foff_w, lbuf, boff_w, _("hole"), tot_w,
			       (st.st_size / blocksize) - last_logical);
		} else {
			printf("\t%d: %s %s", cur_extent, lbuf,
			       _("hole"));
			if (lflag)
				printf(_(" %llu blocks\n"),
				       (st.st_size / blocksize) -
				       last_logical);
			else
				printf("\n");
		}
	}

out:
	free(fiemap);
	return 0;
}

void
fiemap_init(void)
{
	fiemap_cmd.name = "fiemap";
	fiemap_cmd.cfunc = fiemap_f;
	fiemap_cmd.argmin = 0;
	fiemap_cmd.argmax = -1;
	fiemap_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fiemap_cmd.args = _("[-alv] [-n nx]");
	fiemap_cmd.oneline = _("print block mapping for a file");
	fiemap_cmd.help = fiemap_help;

	add_command(&fiemap_cmd);
}
