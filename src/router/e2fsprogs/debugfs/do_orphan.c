/*
 * orphan.c --- list orphan inodes
 *
 */

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "debugfs.h"

/*
 * Return 1 if there was an error, 0 on sucess
 */
static int print_orphan_inode(const char *progname, ext2_ino_t ino,
			      ext2_ino_t *next)
{
	struct ext2_inode inode;

	if (debugfs_read_inode(ino, &inode, progname))
		return 1;
	printf("ino %-6u  links_count %-3u  size %-11llu\n",
	       ino, inode.i_links_count,
	       (unsigned long long) EXT2_I_SIZE(&inode));
	if (next)
		*next = inode.i_dtime;
	return 0;
}

struct process_orphan_block_data {
	const char	*progname;
	char 		*buf;
	char		*block_buf;
	e2_blkcnt_t	blocks;
	int		abort;
	int		clear;
	errcode_t	errcode;
	ext2_ino_t	ino;
	__u32		generation;
};

static int process_orphan_block(ext2_filsys fs,
			       blk64_t	*block_nr,
			       e2_blkcnt_t blockcnt EXT2FS_ATTR((unused)),
			       blk64_t	ref_blk EXT2FS_ATTR((unused)),
			       int	ref_offset EXT2FS_ATTR((unused)),
			       void *priv_data)
{
	struct process_orphan_block_data *pd = priv_data;
	blk64_t			blk = *block_nr;
	__u32			*bdata;
	int			j, inodes_per_ob;

	inodes_per_ob = ext2fs_inodes_per_orphan_block(fs);
	pd->errcode = io_channel_read_blk64(fs->io, blk, 1, pd->buf);
	if (pd->errcode) {
		pd->abort = 1;
		return BLOCK_ABORT;
	}
	bdata = (__u32 *)pd->buf;
	for (j = 0; j < inodes_per_ob; j++) {
		if (!bdata[j])
			continue;
		if (print_orphan_inode(pd->progname,
				       ext2fs_le32_to_cpu(bdata[j]),
				       NULL))
			break;
	}
	return 0;
}


void do_orphan_inodes(int argc, ss_argv_t argv,
		      int sci_idx EXT2FS_ATTR((unused)),
		      void *infop EXT2FS_ATTR((unused)))
{
	ext2_ino_t ino;
	errcode_t retval;
	struct process_orphan_block_data pd;
	char *orphan_buf;
	
	if (check_fs_open(argv[0]))
		return;
	if (argc > 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return;
	}
	ino = current_fs->super->s_last_orphan;
	if (ino)
		printf("Orphan inode list:\n");
	else
		printf("Orphan inode list empty\n");
	while (ino)
		if (print_orphan_inode(argv[0], ino, &ino))
			break;
	if (!ext2fs_has_feature_orphan_file(current_fs->super))
		return;
	printf("Dumping orphan file inode %u:\n",
	       current_fs->super->s_orphan_file_inum);
	orphan_buf = malloc(current_fs->blocksize * 4);
	if (!orphan_buf) {
		fprintf(stderr, "Couldn't allocate orphan block buffer");
		return;
	}
	pd.buf = orphan_buf + 3 * current_fs->blocksize;
	pd.progname = argv[0];
	retval = ext2fs_block_iterate3(current_fs,
				       current_fs->super->s_orphan_file_inum,
				       BLOCK_FLAG_DATA_ONLY,
				       orphan_buf, process_orphan_block, &pd);
	if (retval) {
		com_err(argv[0], retval,
			"while calling ext2fs_block_iterate "
			"for the orphan file");
		return;
	}
}
