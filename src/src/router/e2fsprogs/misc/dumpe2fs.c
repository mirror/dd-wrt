/*
 * dumpe2fs.c		- List the control structures of a second
 *			  extended filesystem
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * History:
 * 94/01/09	- Creation
 * 94/02/27	- Ported to use the ext2fs library
 */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ext2fs/ext2_fs.h"

#include "ext2fs/ext2fs.h"
#include "e2p/e2p.h"
#include "jfs_user.h"
#include <uuid/uuid.h>

#include "../version.h"
#include "nls-enable.h"

#define in_use(m, x)	(ext2fs_test_bit ((x), (m)))

const char * program_name = "dumpe2fs";
char * device_name = NULL;
const char *num_format = "%lu";
char range_format[16];

static void usage(void)
{
	fprintf (stderr, _("Usage: %s [-bfhixV] [-ob superblock] "
		 "[-oB blocksize] device\n"), program_name);
	exit (1);
}

static void print_number (unsigned long num)
{
	printf(num_format, num);
}

static void print_free (unsigned long group, char * bitmap,
			unsigned long nbytes, unsigned long offset)
{
	int p = 0;
	unsigned long i;
	unsigned long j;

	offset += group * nbytes;
	for (i = 0; i < nbytes; i++)
		if (!in_use (bitmap, i))
		{
			if (p)
				printf (", ");
			print_number(i + offset);
			for (j = i; j < nbytes && !in_use (bitmap, j); j++)
				;
			if (--j != i) {
				fputc('-', stdout);
				print_number(j + offset);
				i = j;
			}
			p = 1;
		}
}

static void list_desc (ext2_filsys fs)
{
	unsigned long i;
	long diff;
	blk_t	group_blk, next_blk;
	blk_t	super_blk, old_desc_blk, new_desc_blk;
	char *block_bitmap=NULL, *inode_bitmap=NULL;
	int inode_blocks_per_group, old_desc_blocks, reserved_gdt;
	int has_super;

	if (fs->block_map)
		block_bitmap = fs->block_map->bitmap;
	if (fs->inode_map)
		inode_bitmap = fs->inode_map->bitmap;

	inode_blocks_per_group = ((fs->super->s_inodes_per_group *
				   EXT2_INODE_SIZE(fs->super)) +
				  EXT2_BLOCK_SIZE(fs->super) - 1) /
				 EXT2_BLOCK_SIZE(fs->super);
	reserved_gdt = fs->super->s_reserved_gdt_blocks;
	fputc('\n', stdout);
	group_blk = fs->super->s_first_data_block;
	if (fs->super->s_feature_incompat & EXT2_FEATURE_INCOMPAT_META_BG)
		old_desc_blocks = fs->super->s_first_meta_bg;
	else
		old_desc_blocks = fs->desc_blocks;
	for (i = 0; i < fs->group_desc_count; i++) {
		ext2fs_super_and_bgd_loc(fs, i, &super_blk, 
					 &old_desc_blk, &new_desc_blk, 0);
		next_blk = group_blk + fs->super->s_blocks_per_group;
		if (next_blk > fs->super->s_blocks_count)
			next_blk = fs->super->s_blocks_count;
		printf (_("Group %lu: (Blocks "), i);
		printf(range_format, group_blk, next_blk - 1);
		fputs(")\n", stdout);
		has_super = ((i==0) || super_blk);
		if (has_super) {
			printf (_("  %s superblock at "),
				i == 0 ? _("Primary") : _("Backup"));
			print_number(super_blk);
		}
		if (old_desc_blk) {
			printf(_(", Group descriptors at "));
			printf(range_format, old_desc_blk,
			       old_desc_blk + old_desc_blocks - 1);
			if (reserved_gdt) {
				printf(_("\n  Reserved GDT blocks at "));
				printf(range_format, 
				       old_desc_blk + old_desc_blocks,
				       old_desc_blk + old_desc_blocks + 
				       reserved_gdt - 1);
			}
		} else if (new_desc_blk) {
			fputc(has_super ? ',' : ' ', stdout);
			printf(_(" Group descriptor at "));
			print_number(new_desc_blk);
			has_super++;
		}
		if (has_super)
			fputc('\n', stdout);
		fputs(_("  Block bitmap at "), stdout);
		print_number(fs->group_desc[i].bg_block_bitmap);
		diff = fs->group_desc[i].bg_block_bitmap - group_blk;
		if (diff >= 0)
			printf(" (+%ld)", diff);
		fputs(_(", Inode bitmap at "), stdout);
		print_number(fs->group_desc[i].bg_inode_bitmap);
		diff = fs->group_desc[i].bg_inode_bitmap - group_blk;
		if (diff >= 0)
			printf(" (+%ld)", diff);
		fputs(_("\n  Inode table at "), stdout);
		printf(range_format, fs->group_desc[i].bg_inode_table,
		       fs->group_desc[i].bg_inode_table +
		       inode_blocks_per_group - 1);
		diff = fs->group_desc[i].bg_inode_table - group_blk;
		if (diff > 0)
			printf(" (+%ld)", diff);
		printf (_("\n  %d free blocks, %d free inodes, "
			  "%d directories\n"),
			fs->group_desc[i].bg_free_blocks_count,
			fs->group_desc[i].bg_free_inodes_count,
			fs->group_desc[i].bg_used_dirs_count);
		if (block_bitmap) {
			fputs(_("  Free blocks: "), stdout);
			print_free (i, block_bitmap,
				    fs->super->s_blocks_per_group,
				    fs->super->s_first_data_block);
			fputc('\n', stdout);
			block_bitmap += fs->super->s_blocks_per_group / 8;
		}
		if (inode_bitmap) {
			fputs(_("  Free inodes: "), stdout);
			print_free (i, inode_bitmap,
				    fs->super->s_inodes_per_group, 1);
			fputc('\n', stdout);
			inode_bitmap += fs->super->s_inodes_per_group / 8;
		}
		group_blk = next_blk;
	}
}

static void list_bad_blocks(ext2_filsys fs, int dump)
{
	badblocks_list		bb_list = 0;
	badblocks_iterate	bb_iter;
	blk_t			blk;
	errcode_t		retval;
	const char		*header, *fmt;

	retval = ext2fs_read_bb_inode(fs, &bb_list);
	if (retval) {
		com_err("ext2fs_read_bb_inode", retval, "");
		return;
	}
	retval = ext2fs_badblocks_list_iterate_begin(bb_list, &bb_iter);
	if (retval) {
		com_err("ext2fs_badblocks_list_iterate_begin", retval,
			_("while printing bad block list"));
		return;
	}
	if (dump) {
		header = fmt = "%d\n";
	} else {
		header =  _("Bad blocks: %d");
		fmt = ", %d";
	}
	while (ext2fs_badblocks_list_iterate(bb_iter, &blk)) {
		printf(header ? header : fmt, blk);
		header = 0;
	}
	ext2fs_badblocks_list_iterate_end(bb_iter);
	if (!dump)
		fputc('\n', stdout);
}

static void print_journal_information(ext2_filsys fs)
{
	errcode_t	retval;
	char		buf[1024];
	char		str[80];
	unsigned int	i;
	journal_superblock_t	*jsb;

	/* Get the journal superblock */
	if ((retval = io_channel_read_blk(fs->io, fs->super->s_first_data_block+1, -1024, buf))) {
		com_err(program_name, retval,
			_("while reading journal superblock"));
		exit(1);
	}
	jsb = (journal_superblock_t *) buf;
	if ((jsb->s_header.h_magic != (unsigned) ntohl(JFS_MAGIC_NUMBER)) ||
	    (jsb->s_header.h_blocktype !=
	     (unsigned) ntohl(JFS_SUPERBLOCK_V2))) {
		com_err(program_name, 0,
			_("Couldn't find journal superblock magic numbers"));
		exit(1);
	}

	printf(_("\nJournal block size:       %d\n"
		 "Journal length:           %d\n"
		 "Journal first block:      %d\n"
		 "Journal sequence:         0x%08x\n"
		 "Journal start:            %d\n"
		 "Journal number of users:  %d\n"),
	       ntohl(jsb->s_blocksize),  ntohl(jsb->s_maxlen),
	       ntohl(jsb->s_first), ntohl(jsb->s_sequence),
	       ntohl(jsb->s_start), ntohl(jsb->s_nr_users));

	for (i=0; i < ntohl(jsb->s_nr_users); i++) {
		uuid_unparse(&jsb->s_users[i*16], str);
		printf(i ? "                          %s\n"
		       : _("Journal users:            %s\n"),
		       str);
	}
}

int main (int argc, char ** argv)
{
	errcode_t	retval;
	ext2_filsys	fs;
	int		print_badblocks = 0;
	int		use_superblock = 0;
	int		use_blocksize = 0;
	int		image_dump = 0;
	int		force = 0;
	int		flags;
	int		header_only = 0;
	int		big_endian;
	int		c;

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
#endif
	initialize_ext2_error_table();
	fprintf (stderr, "dumpe2fs %s (%s)\n", E2FSPROGS_VERSION,
		 E2FSPROGS_DATE);
	if (argc && *argv)
		program_name = *argv;
	
	while ((c = getopt (argc, argv, "bfhixVo:")) != EOF) {
		switch (c) {
		case 'b':
			print_badblocks++;
			break;
		case 'f':
			force++;
			break;
		case 'h':
			header_only++;
			break;
		case 'i':
			image_dump++;
			break;
		case 'o':
			if (optarg[0] == 'b')
				use_superblock = atoi(optarg+1);
			else if (optarg[0] == 'B')
				use_blocksize = atoi(optarg+1);
			else
				usage();
			break;
		case 'V':
			/* Print version number and exit */
			fprintf(stderr, _("\tUsing %s\n"),
				error_message(EXT2_ET_BASE));
			exit(0);
		case 'x':
			num_format = "0x%04x";
			break;
		default:
			usage();
		}
	}
	if (optind > argc - 1)
		usage();
	sprintf(range_format, "%s-%s", num_format, num_format);
	device_name = argv[optind++];
	if (use_superblock && !use_blocksize)
		use_blocksize = 1024;
	flags = EXT2_FLAG_JOURNAL_DEV_OK;
	if (force)
		flags |= EXT2_FLAG_FORCE;
	if (image_dump)
		flags |= EXT2_FLAG_IMAGE_FILE;
	
	retval = ext2fs_open (device_name, flags, use_superblock,
			      use_blocksize, unix_io_manager, &fs);
	if (retval) {
		com_err (program_name, retval, _("while trying to open %s"),
			 device_name);
		printf (_("Couldn't find valid filesystem superblock.\n"));
		exit (1);
	}
	if (print_badblocks) {
		list_bad_blocks(fs, 1);
	} else {
		big_endian = ((fs->flags & EXT2_FLAG_SWAP_BYTES) != 0);
#ifdef WORDS_BIGENDIAN
		big_endian = !big_endian;
#endif
		if (big_endian)
			printf(_("Note: This is a byte-swapped filesystem\n"));
		list_super (fs->super);
		if (fs->super->s_feature_incompat &
		      EXT3_FEATURE_INCOMPAT_JOURNAL_DEV) {
			print_journal_information(fs);
			ext2fs_close(fs);
			exit(0);
		}
		list_bad_blocks(fs, 0);
		if (header_only) {
			ext2fs_close (fs);
			exit (0);
		}
		retval = ext2fs_read_bitmaps (fs);
		list_desc (fs);
		if (retval) {
			printf(_("\n%s: %s: error reading bitmaps: %s\n"),
			       program_name, device_name,
			       error_message(retval));
		}
	}
	ext2fs_close (fs);
	exit (0);
}
