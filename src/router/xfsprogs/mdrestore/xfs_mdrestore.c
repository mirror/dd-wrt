/*
 * Copyright (c) 2007 Silicon Graphics, Inc.
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

#include "libxfs.h"
#include "xfs_metadump.h"

char 		*progname;
int		show_progress = 0;
int		show_info = 0;
int		progress_since_warning = 0;

static void
fatal(const char *msg, ...)
{
	va_list		args;

	va_start(args, msg);
	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, msg, args);
	exit(1);
}

static void
print_progress(const char *fmt, ...)
{
	char		buf[60];
	va_list		ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	buf[sizeof(buf)-1] = '\0';

	printf("\r%-59s", buf);
	fflush(stdout);
	progress_since_warning = 1;
}

static void
perform_restore(
	FILE			*src_f,
	int			dst_fd,
	int			is_target_file)
{
	xfs_metablock_t 	*metablock;	/* header + index + blocks */
	__be64			*block_index;
	char			*block_buffer;
	int			block_size;
	int			max_indices;
	int			cur_index;
	int			mb_count;
	xfs_metablock_t		tmb;
	xfs_sb_t		sb;
	int64_t			bytes_read;

	/*
	 * read in first blocks (superblock 0), set "inprogress" flag for it,
	 * read in the rest of the file, and if complete, clear SB 0's
	 * "inprogress flag"
	 */

	if (fread(&tmb, sizeof(tmb), 1, src_f) != 1)
		fatal("error reading from file: %s\n", strerror(errno));

	if (be32_to_cpu(tmb.mb_magic) != XFS_MD_MAGIC)
		fatal("specified file is not a metadata dump\n");

	block_size = 1 << tmb.mb_blocklog;
	max_indices = (block_size - sizeof(xfs_metablock_t)) / sizeof(__be64);

	metablock = (xfs_metablock_t *)calloc(max_indices + 1, block_size);
	if (metablock == NULL)
		fatal("memory allocation failure\n");

	mb_count = be16_to_cpu(tmb.mb_count);
	if (mb_count == 0 || mb_count > max_indices)
		fatal("bad block count: %u\n", mb_count);

	block_index = (__be64 *)((char *)metablock + sizeof(xfs_metablock_t));
	block_buffer = (char *)metablock + block_size;

	if (fread(block_index, block_size - sizeof(tmb), 1, src_f) != 1)
		fatal("error reading from file: %s\n", strerror(errno));

	if (block_index[0] != 0)
		fatal("first block is not the primary superblock\n");


	if (fread(block_buffer, mb_count << tmb.mb_blocklog,
			1, src_f) != 1)
		fatal("error reading from file: %s\n", strerror(errno));

	libxfs_sb_from_disk(&sb, (xfs_dsb_t *)block_buffer);

	if (sb.sb_magicnum != XFS_SB_MAGIC)
		fatal("bad magic number for primary superblock\n");

	/*
	 * Normally the upper bound would be simply XFS_MAX_SECTORSIZE
	 * but the metadump format has a maximum number of BBSIZE blocks
	 * it can store in a single metablock.
	 */
	if (sb.sb_sectsize < XFS_MIN_SECTORSIZE ||
	    sb.sb_sectsize > XFS_MAX_SECTORSIZE ||
	    sb.sb_sectsize > max_indices * block_size)
		fatal("bad sector size %u in metadump image\n", sb.sb_sectsize);

	((xfs_dsb_t*)block_buffer)->sb_inprogress = 1;

	if (is_target_file)  {
		/* ensure regular files are correctly sized */

		if (ftruncate(dst_fd, sb.sb_dblocks * sb.sb_blocksize))
			fatal("cannot set filesystem image size: %s\n",
				strerror(errno));
	} else  {
		/* ensure device is sufficiently large enough */

		char		*lb[XFS_MAX_SECTORSIZE] = { NULL };
		off64_t		off;

		off = sb.sb_dblocks * sb.sb_blocksize - sizeof(lb);
		if (pwrite(dst_fd, lb, sizeof(lb), off) < 0)
			fatal("failed to write last block, is target too "
				"small? (error: %s)\n", strerror(errno));
	}

	bytes_read = 0;

	for (;;) {
		if (show_progress && (bytes_read & ((1 << 20) - 1)) == 0)
			print_progress("%lld MB read", bytes_read >> 20);

		for (cur_index = 0; cur_index < mb_count; cur_index++) {
			if (pwrite(dst_fd, &block_buffer[cur_index <<
					tmb.mb_blocklog], block_size,
					be64_to_cpu(block_index[cur_index]) <<
						BBSHIFT) < 0)
				fatal("error writing block %llu: %s\n",
					be64_to_cpu(block_index[cur_index]) << BBSHIFT,
					strerror(errno));
		}
		if (mb_count < max_indices)
			break;

		if (fread(metablock, block_size, 1, src_f) != 1)
			fatal("error reading from file: %s\n", strerror(errno));

		mb_count = be16_to_cpu(metablock->mb_count);
		if (mb_count == 0)
			break;
		if (mb_count > max_indices)
			fatal("bad block count: %u\n", mb_count);

		if (fread(block_buffer, mb_count << tmb.mb_blocklog,
								1, src_f) != 1)
			fatal("error reading from file: %s\n", strerror(errno));

		bytes_read += block_size + (mb_count << tmb.mb_blocklog);
	}

	if (progress_since_warning)
		putchar('\n');

	memset(block_buffer, 0, sb.sb_sectsize);
	sb.sb_inprogress = 0;
	libxfs_sb_to_disk((xfs_dsb_t *)block_buffer, &sb);
	if (xfs_sb_version_hascrc(&sb)) {
		xfs_update_cksum(block_buffer, sb.sb_sectsize,
				 offsetof(struct xfs_sb, sb_crc));
	}

	if (pwrite(dst_fd, block_buffer, sb.sb_sectsize, 0) < 0)
		fatal("error writing primary superblock: %s\n", strerror(errno));

	free(metablock);
}

static void
usage(void)
{
	fprintf(stderr, "Usage: %s [-V] [-g] source target\n", progname);
	exit(1);
}

extern int	platform_check_ismounted(char *, char *, struct stat *, int);

int
main(
	int 		argc,
	char 		**argv)
{
	FILE		*src_f;
	int		dst_fd;
	int		c;
	int		open_flags;
	struct stat	statbuf;
	int		is_target_file;

	progname = basename(argv[0]);

	while ((c = getopt(argc, argv, "giV")) != EOF) {
		switch (c) {
			case 'g':
				show_progress = 1;
				break;
			case 'i':
				show_info = 1;
				break;
			case 'V':
				printf("%s version %s\n", progname, VERSION);
				exit(0);
			default:
				usage();
		}
	}

	if (argc - optind < 1 || argc - optind > 2)
		usage();

	/* show_info without a target is ok */
	if (!show_info && argc - optind != 2)
		usage();

	/* open source */
	if (strcmp(argv[optind], "-") == 0) {
		src_f = stdin;
		if (isatty(fileno(stdin)))
			fatal("cannot read from a terminal\n");
	} else {
		src_f = fopen(argv[optind], "rb");
		if (src_f == NULL)
			fatal("cannot open source dump file\n");
	}

	if (show_info) {
		xfs_metablock_t		mb;

		if (fread(&mb, sizeof(mb), 1, src_f) != 1)
			fatal("error reading from file: %s\n", strerror(errno));

		if (be32_to_cpu(mb.mb_magic) != XFS_MD_MAGIC)
			fatal("specified file is not a metadata dump\n");

		if (mb.mb_info & XFS_METADUMP_INFO_FLAGS) {
			printf("%s: %sobfuscated, %s log, %s metadata blocks\n",
			argv[optind],
			mb.mb_info & XFS_METADUMP_OBFUSCATED ? "":"not ",
			mb.mb_info & XFS_METADUMP_DIRTYLOG ? "dirty":"clean",
			mb.mb_info & XFS_METADUMP_FULLBLOCKS ? "full":"zeroed");
		} else {
			printf("%s: no informational flags present\n",
				argv[optind]);
		}

		if (argc - optind == 1)
			exit(0);

		/* Go back to the beginning for the restore function */
		fseek(src_f, 0L, SEEK_SET);
	}

	optind++;

	/* check and open target */
	open_flags = O_RDWR;
	is_target_file = 0;
	if (stat(argv[optind], &statbuf) < 0)  {
		/* ok, assume it's a file and create it */
		open_flags |= O_CREAT;
		is_target_file = 1;
	} else if (S_ISREG(statbuf.st_mode))  {
		open_flags |= O_TRUNC;
		is_target_file = 1;
	} else  {
		/*
		 * check to make sure a filesystem isn't mounted on the device
		 */
		if (platform_check_ismounted(argv[optind], NULL, &statbuf, 0))
			fatal("a filesystem is mounted on target device \"%s\","
				" cannot restore to a mounted filesystem.\n",
				argv[optind]);
	}

	dst_fd = open(argv[optind], open_flags, 0644);
	if (dst_fd < 0)
		fatal("couldn't open target \"%s\"\n", argv[optind]);

	perform_restore(src_f, dst_fd, is_target_file);

	close(dst_fd);
	if (src_f != stdin)
		fclose(src_f);

	return 0;
}
