/*
 * tune2fs.c - Change the file system parameters on an ext2 file system
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997, 1998, 1999, 2000 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * History:
 * 93/06/01	- Creation
 * 93/10/31	- Added the -c option to change the maximal mount counts
 * 93/12/14	- Added -l flag to list contents of superblock
 *                M.J.E. Mol (marcel@duteca.et.tudelft.nl)
 *                F.W. ten Wolde (franky@duteca.et.tudelft.nl)
 * 93/12/29	- Added the -e option to change errors behavior
 * 94/02/27	- Ported to use the ext2fs library
 * 94/03/06	- Added the checks interval from Uwe Ohse (uwe@tirka.gun.de)
 */

#define _XOPEN_SOURCE 500 /* for inclusion of strptime() */
#define _BSD_SOURCE /* for inclusion of strcasecmp() */
#include <fcntl.h>
#include <grp.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "et/com_err.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"
#include "jfs_user.h"
#include "util.h"
#include "blkid/blkid.h"

#include "../version.h"
#include "nls-enable.h"

const char * program_name = "tune2fs";
char * device_name;
char * new_label, *new_last_mounted, *new_UUID;
char * io_options;
static int c_flag, C_flag, e_flag, f_flag, g_flag, i_flag, l_flag, L_flag;
static int m_flag, M_flag, r_flag, s_flag = -1, u_flag, U_flag, T_flag;
static time_t last_check_time;
static int print_label;
static int max_mount_count, mount_count, mount_flags;
static unsigned long interval, reserved_ratio, reserved_blocks;
static unsigned long resgid, resuid;
static unsigned short errors;
static int open_flag;
static char *features_cmd;
static char *mntopts_cmd;

int journal_size, journal_flags;
char *journal_device;

static const char *please_fsck = N_("Please run e2fsck on the filesystem.\n");

void do_findfs(int argc, char **argv);

static void usage(void)
{
	fprintf(stderr,
		_("Usage: %s [-c max_mounts_count] [-e errors_behavior] "
		  "[-g group]\n"
		  "\t[-i interval[d|m|w]] [-j] [-J journal_options]\n"
		  "\t[-l] [-s sparse_flag] [-m reserved_blocks_percent]\n"
		  "\t[-o [^]mount_options[,...]] [-r reserved_blocks_count]\n"
		  "\t[-u user] [-C mount_count] [-L volume_label] "
		  "[-M last_mounted_dir]\n"
		  "\t[-O [^]feature[,...]] [-T last_check_time] [-U UUID]"
		  " device\n"), program_name);
	exit (1);
}

static __u32 ok_features[3] = {
	EXT3_FEATURE_COMPAT_HAS_JOURNAL |
		EXT2_FEATURE_COMPAT_DIR_INDEX,	/* Compat */
	EXT2_FEATURE_INCOMPAT_FILETYPE,		/* Incompat */
	EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	/* R/O compat */
};

/*
 * Remove an external journal from the filesystem
 */
static void remove_journal_device(ext2_filsys fs)
{
	char		*journal_path;
	ext2_filsys	jfs;
	char		buf[1024];
	journal_superblock_t	*jsb;
	int		i, nr_users;
	errcode_t	retval;
	int		commit_remove_journal = 0;
	io_manager	io_ptr;

	if (f_flag)
		commit_remove_journal = 1; /* force removal even if error */

	uuid_unparse(fs->super->s_journal_uuid, buf);
	journal_path = blkid_get_devname(NULL, "UUID", buf);

	if (!journal_path) {
		journal_path =
			ext2fs_find_block_device(fs->super->s_journal_dev);
		if (!journal_path)
			return;
	}

#ifdef CONFIG_TESTIO_DEBUG
	io_ptr = test_io_manager;
	test_io_backing_manager = unix_io_manager;
#else
	io_ptr = unix_io_manager;
#endif
	retval = ext2fs_open(journal_path, EXT2_FLAG_RW|
			     EXT2_FLAG_JOURNAL_DEV_OK, 0,
			     fs->blocksize, io_ptr, &jfs);
	if (retval) {
		com_err(program_name, retval,
			_("while trying to open external journal"));
		goto no_valid_journal;
	}
	if (!(jfs->super->s_feature_incompat & EXT3_FEATURE_INCOMPAT_JOURNAL_DEV)) {
		fprintf(stderr, _("%s is not a journal device.\n"),
			journal_path);
		goto no_valid_journal;
	}

	/* Get the journal superblock */
	if ((retval = io_channel_read_blk(jfs->io, 1, -1024, buf))) {
		com_err(program_name, retval,
			_("while reading journal superblock"));
		goto no_valid_journal;
	}

	jsb = (journal_superblock_t *) buf;
	if ((jsb->s_header.h_magic != (unsigned) ntohl(JFS_MAGIC_NUMBER)) ||
	    (jsb->s_header.h_blocktype != (unsigned) ntohl(JFS_SUPERBLOCK_V2))) {
		fputs(_("Journal superblock not found!\n"), stderr);
		goto no_valid_journal;
	}

	/* Find the filesystem UUID */
	nr_users = ntohl(jsb->s_nr_users);
	for (i=0; i < nr_users; i++) {
		if (memcmp(fs->super->s_uuid,
			   &jsb->s_users[i*16], 16) == 0)
			break;
	}
	if (i >= nr_users) {
		fputs(_("Filesystem's UUID not found on journal device.\n"), 
		      stderr);
		commit_remove_journal = 1;
		goto no_valid_journal;
	}
	nr_users--;
	for (i=0; i < nr_users; i++)
		memcpy(&jsb->s_users[i*16], &jsb->s_users[(i+1)*16], 16);
	jsb->s_nr_users = htonl(nr_users);

	/* Write back the journal superblock */
	if ((retval = io_channel_write_blk(jfs->io, 1, -1024, buf))) {
		com_err(program_name, retval,
			"while writing journal superblock.");
		goto no_valid_journal;
	}

	commit_remove_journal = 1;

no_valid_journal:
	if (commit_remove_journal == 0) {
		fputs(_("Journal NOT removed\n"), stderr);
		exit(1);
	}
	fs->super->s_journal_dev = 0;
	uuid_clear(fs->super->s_journal_uuid);
	ext2fs_mark_super_dirty(fs);
	fputs(_("Journal removed\n"), stdout);
	free(journal_path);
}

/* Helper function for remove_journal_inode */
static int release_blocks_proc(ext2_filsys fs, blk_t *blocknr,
			       int blockcnt EXT2FS_ATTR((unused)), 
			       void *private EXT2FS_ATTR((unused)))
{
	blk_t	block;
	int	group;

	block = *blocknr;
	ext2fs_unmark_block_bitmap(fs->block_map,block);
	group = ext2fs_group_of_blk(fs, block);
	fs->group_desc[group].bg_free_blocks_count++;
	fs->super->s_free_blocks_count++;
	return 0;
}

/*
 * Remove the journal inode from the filesystem
 */
static void remove_journal_inode(ext2_filsys fs)
{
	struct ext2_inode	inode;
	errcode_t		retval;
	ino_t			ino = fs->super->s_journal_inum;
	
	retval = ext2fs_read_inode(fs, ino,  &inode);
	if (retval) {
		com_err(program_name, retval,
			_("while reading journal inode"));
		exit(1);
	}
	if (ino == EXT2_JOURNAL_INO) {
		retval = ext2fs_read_bitmaps(fs);
		if (retval) {
			com_err(program_name, retval,
				_("while reading bitmaps"));
			exit(1);
		}
		retval = ext2fs_block_iterate(fs, ino, 0, NULL,
					      release_blocks_proc, NULL);
		if (retval) {
			com_err(program_name, retval,
				_("while clearing journal inode"));
			exit(1);
		}
		memset(&inode, 0, sizeof(inode));
		ext2fs_mark_bb_dirty(fs);
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	} else
		inode.i_flags &= ~EXT2_IMMUTABLE_FL;
	retval = ext2fs_write_inode(fs, ino, &inode);
	if (retval) {
		com_err(program_name, retval,
			_("while writing journal inode"));
		exit(1);
	}
	fs->super->s_journal_inum = 0;
	ext2fs_mark_super_dirty(fs);
}

/*
 * Update the default mount options
 */
static void update_mntopts(ext2_filsys fs, char *mntopts)
{
	struct ext2_super_block *sb= fs->super;

	if (e2p_edit_mntopts(mntopts, &sb->s_default_mount_opts, ~0)) {
		fprintf(stderr, _("Invalid mount option set: %s\n"),
			mntopts);
		exit(1);
	}
	ext2fs_mark_super_dirty(fs);
}

/*
 * Update the feature set as provided by the user.
 */
static void update_feature_set(ext2_filsys fs, char *features)
{
	int sparse, old_sparse, filetype, old_filetype;
	int journal, old_journal, dxdir, old_dxdir;
	struct ext2_super_block *sb= fs->super;
	__u32	old_compat, old_incompat, old_ro_compat;

	old_compat = sb->s_feature_compat;
	old_ro_compat = sb->s_feature_ro_compat;
	old_incompat = sb->s_feature_incompat;

	old_sparse = sb->s_feature_ro_compat &
		EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
	old_filetype = sb->s_feature_incompat &
		EXT2_FEATURE_INCOMPAT_FILETYPE;
	old_journal = sb->s_feature_compat &
		EXT3_FEATURE_COMPAT_HAS_JOURNAL;
	old_dxdir = sb->s_feature_compat &
		EXT2_FEATURE_COMPAT_DIR_INDEX;
	if (e2p_edit_feature(features, &sb->s_feature_compat,
			     ok_features)) {
		fprintf(stderr, _("Invalid filesystem option set: %s\n"),
			features);
		exit(1);
	}
	sparse = sb->s_feature_ro_compat &
		EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
	filetype = sb->s_feature_incompat &
		EXT2_FEATURE_INCOMPAT_FILETYPE;
	journal = sb->s_feature_compat &
		EXT3_FEATURE_COMPAT_HAS_JOURNAL;
	dxdir = sb->s_feature_compat &
		EXT2_FEATURE_COMPAT_DIR_INDEX;
	if (old_journal && !journal) {
		if ((mount_flags & EXT2_MF_MOUNTED) &&
		    !(mount_flags & EXT2_MF_READONLY)) {
			fputs(_("The has_journal flag may only be "
				"cleared when the filesystem is\n"
				"unmounted or mounted "
				"read-only.\n"), stderr);
			exit(1);
		}
		if (sb->s_feature_incompat &
		    EXT3_FEATURE_INCOMPAT_RECOVER) {
			fputs(_("The needs_recovery flag is set.  "
				"Please run e2fsck before clearing\n"
				"the has_journal flag.\n"), stderr);
			exit(1);
		}
		if (sb->s_journal_inum) {
			remove_journal_inode(fs);
		}
		if (sb->s_journal_dev) {
			remove_journal_device(fs);
		}
	}
	if (journal && !old_journal) {
		/*
		 * If adding a journal flag, let the create journal
		 * code below handle creating setting the flag and
		 * creating the journal.  We supply a default size if
		 * necessary.
		 */
		if (!journal_size)
			journal_size = -1;
		sb->s_feature_compat &= ~EXT3_FEATURE_COMPAT_HAS_JOURNAL;
	}
	if (dxdir && !old_dxdir) {
		if (!sb->s_def_hash_version)
			sb->s_def_hash_version = EXT2_HASH_TEA;
		if (uuid_is_null((unsigned char *) sb->s_hash_seed))
			uuid_generate((unsigned char *) sb->s_hash_seed);
	}

	if (sb->s_rev_level == EXT2_GOOD_OLD_REV &&
	    (sb->s_feature_compat || sb->s_feature_ro_compat ||
	     sb->s_feature_incompat))
		ext2fs_update_dynamic_rev(fs);
	if ((sparse != old_sparse) ||
	    (filetype != old_filetype)) {
		sb->s_state &= ~EXT2_VALID_FS;
		printf("\n%s\n", _(please_fsck));
	}
	if ((old_compat != sb->s_feature_compat) ||
	    (old_ro_compat != sb->s_feature_ro_compat) ||
	    (old_incompat != sb->s_feature_incompat))
		ext2fs_mark_super_dirty(fs);
}

/*
 * Add a journal to the filesystem.
 */
static void add_journal(ext2_filsys fs)
{
	unsigned long journal_blocks;
	errcode_t	retval;
	ext2_filsys	jfs;
	io_manager	io_ptr;

	if (fs->super->s_feature_compat &
	    EXT3_FEATURE_COMPAT_HAS_JOURNAL) {
		fputs(_("The filesystem already has a journal.\n"), stderr);
		goto err;
	}
	if (journal_device) {
		check_plausibility(journal_device);
		check_mount(journal_device, 0, _("journal"));
#ifdef CONFIG_TESTIO_DEBUG
		io_ptr = test_io_manager;
		test_io_backing_manager = unix_io_manager;
#else
		io_ptr = unix_io_manager;
#endif
		retval = ext2fs_open(journal_device, EXT2_FLAG_RW|
				     EXT2_FLAG_JOURNAL_DEV_OK, 0,
				     fs->blocksize, io_ptr, &jfs);
		if (retval) {
			com_err(program_name, retval,
				_("\n\twhile trying to open journal on %s\n"),
				journal_device);
			goto err;
		}
		printf(_("Creating journal on device %s: "),
		       journal_device);
		fflush(stdout);

		retval = ext2fs_add_journal_device(fs, jfs);
		ext2fs_close(jfs);
		if (retval) {
			com_err (program_name, retval,
				 _("while adding filesystem to journal on %s"),
				 journal_device);
			goto err;
		}
		fputs(_("done\n"), stdout);
	} else if (journal_size) {
		fputs(_("Creating journal inode: "), stdout);
		fflush(stdout);
		journal_blocks = figure_journal_size(journal_size, fs);

		retval = ext2fs_add_journal_inode(fs, journal_blocks,
						  journal_flags);
		if (retval) {
			fprintf(stderr, "\n");
			com_err(program_name, retval,
				_("\n\twhile trying to create journal file"));
			exit(1);
		} else
			fputs(_("done\n"), stdout);
		/*
		 * If the filesystem wasn't mounted, we need to force
		 * the block group descriptors out.
		 */
		if ((mount_flags & EXT2_MF_MOUNTED) == 0)
			fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	}
	print_check_message(fs);
	return;

err:
	if (journal_device)
		free(journal_device);
	exit(1);
}


static void parse_e2label_options(int argc, char ** argv)
{
	if ((argc < 2) || (argc > 3)) {
		fputs(_("Usage: e2label device [newlabel]\n"), stderr);
		exit(1);
	}
	io_options = strchr(argv[1], '?');
	if (io_options)
		*io_options++ = 0;
	device_name = blkid_get_devname(NULL, argv[1], NULL);
	if (!device_name) {
		com_err("e2label", 0, _("Unable to resolve '%s'"), 
			argv[1]);
		exit(1);
	}
	if (argc == 3) {
		open_flag = EXT2_FLAG_RW | EXT2_FLAG_JOURNAL_DEV_OK;
		L_flag = 1;
		new_label = argv[2];
	} else 
		print_label++;
}

static time_t parse_time(char *str)
{
	struct	tm	ts;

	if (strcmp(str, "now") == 0) {
		return (time(0));
	}
	memset(&ts, 0, sizeof(ts));
#ifdef HAVE_STRPTIME
	strptime(str, "%Y%m%d%H%M%S", &ts);
#else
	sscanf(str, "%4d%2d%2d%2d%2d%2d", &ts.tm_year, &ts.tm_mon,
	       &ts.tm_mday, &ts.tm_hour, &ts.tm_min, &ts.tm_sec);
	ts.tm_year -= 1900;
	ts.tm_mon -= 1;
	if (ts.tm_year < 0 || ts.tm_mon < 0 || ts.tm_mon > 11 ||
	    ts.tm_mday < 0 || ts.tm_mday > 31 || ts.tm_hour > 23 ||
	    ts.tm_min > 59 || ts.tm_sec > 61)
		ts.tm_mday = 0;
#endif
	if (ts.tm_mday == 0) {
		com_err(program_name, 0,
			_("Couldn't parse date/time specifier: %s"),
			str);
		usage();
	}
	return (mktime(&ts));
}

static void parse_tune2fs_options(int argc, char **argv)
{
	int c;
	char * tmp;
	struct group * gr;
	struct passwd * pw;

	printf("tune2fs %s (%s)\n", E2FSPROGS_VERSION, E2FSPROGS_DATE);
	while ((c = getopt(argc, argv, "c:e:fg:i:jlm:o:r:s:u:C:J:L:M:O:T:U:")) != EOF)
		switch (c)
		{
			case 'c':
				max_mount_count = strtol (optarg, &tmp, 0);
				if (*tmp || max_mount_count > 16000) {
					com_err (program_name, 0,
						 _("bad mounts count - %s"),
						 optarg);
					usage();
				}
				if (max_mount_count == 0)
					max_mount_count = -1;
				c_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'C':
				mount_count = strtoul (optarg, &tmp, 0);
				if (*tmp || mount_count > 16000) {
					com_err (program_name, 0,
						 _("bad mounts count - %s"),
						 optarg);
					usage();
				}
				C_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'e':
				if (strcmp (optarg, "continue") == 0)
					errors = EXT2_ERRORS_CONTINUE;
				else if (strcmp (optarg, "remount-ro") == 0)
					errors = EXT2_ERRORS_RO;
				else if (strcmp (optarg, "panic") == 0)
					errors = EXT2_ERRORS_PANIC;
				else {
					com_err (program_name, 0,
						 _("bad error behavior - %s"),
						 optarg);
					usage();
				}
				e_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'f': /* Force */
				f_flag = 1;
				break;
			case 'g':
				resgid = strtoul (optarg, &tmp, 0);
				if (*tmp) {
					gr = getgrnam (optarg);
					if (gr == NULL)
						tmp = optarg;
					else {
						resgid = gr->gr_gid;
						*tmp =0;
					}
				}
				if (*tmp) {
					com_err (program_name, 0,
						 _("bad gid/group name - %s"),
						 optarg);
					usage();
				}
				g_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'i':
				interval = strtoul (optarg, &tmp, 0);
				switch (*tmp) {
				case 's':
					tmp++;
					break;
				case '\0':
				case 'd':
				case 'D': /* days */
					interval *= 86400;
					if (*tmp != '\0')
						tmp++;
					break;
				case 'm':
				case 'M': /* months! */
					interval *= 86400 * 30;
					tmp++;
					break;
				case 'w':
				case 'W': /* weeks */
					interval *= 86400 * 7;
					tmp++;
					break;
				}
				if (*tmp || interval > (365 * 86400)) {
					com_err (program_name, 0,
						_("bad interval - %s"), optarg);
					usage();
				}
				i_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'j':
				if (!journal_size)
					journal_size = -1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'J':
				parse_journal_opts(optarg);
				open_flag = EXT2_FLAG_RW;
				break;
			case 'l':
				l_flag = 1;
				break;
			case 'L':
				new_label = optarg;
				L_flag = 1;
				open_flag = EXT2_FLAG_RW |
					EXT2_FLAG_JOURNAL_DEV_OK;
				break;
			case 'm':
				reserved_ratio = strtoul (optarg, &tmp, 0);
				if (*tmp || reserved_ratio > 50) {
					com_err (program_name, 0,
						 _("bad reserved block ratio - %s"),
						 optarg);
					usage();
				}
				m_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'M':
				new_last_mounted = optarg;
				M_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'o':
				if (mntopts_cmd) {
					com_err (program_name, 0,
					 _("-o may only be specified once"));
					usage();
				}
				mntopts_cmd = optarg;
				open_flag = EXT2_FLAG_RW;
				break;
				
			case 'O':
				if (features_cmd) {
					com_err (program_name, 0,
					 _("-O may only be specified once"));
					usage();
				}
				features_cmd = optarg;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'r':
				reserved_blocks = strtoul (optarg, &tmp, 0);
				if (*tmp) {
					com_err (program_name, 0,
						 _("bad reserved blocks count - %s"),
						 optarg);
					usage();
				}
				r_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 's':
				s_flag = atoi(optarg);
				open_flag = EXT2_FLAG_RW;
				break;
			case 'T':
				T_flag = 1;
				last_check_time = parse_time(optarg);
				open_flag = EXT2_FLAG_RW;
				break;
			case 'u':
				resuid = strtoul (optarg, &tmp, 0);
				if (*tmp) {
					pw = getpwnam (optarg);
					if (pw == NULL)
						tmp = optarg;
					else {
						resuid = pw->pw_uid;
						*tmp = 0;
					}
				}
				if (*tmp) {
					com_err (program_name, 0,
						 _("bad uid/user name - %s"),
						 optarg);
					usage();
				}
				u_flag = 1;
				open_flag = EXT2_FLAG_RW;
				break;
			case 'U':
				new_UUID = optarg;
				U_flag = 1;
				open_flag = EXT2_FLAG_RW |
					EXT2_FLAG_JOURNAL_DEV_OK;
				break;
			default:
				usage();
		}
	if (optind < argc - 1 || optind == argc)
		usage();
	if (!open_flag && !l_flag)
		usage();
	io_options = strchr(argv[optind], '?');
	if (io_options)
		*io_options++ = 0;
	device_name = blkid_get_devname(NULL, argv[optind], NULL);
	if (!device_name) {
		com_err("tune2fs", 0, _("Unable to resolve '%s'"), 
			argv[optind]);
		exit(1);
	}
}

void do_findfs(int argc, char **argv)
{
	char	*dev;

	if ((argc != 2) ||
	    (strncmp(argv[1], "LABEL=", 6) && strncmp(argv[1], "UUID=", 5))) {
		fprintf(stderr, "Usage: findfs LABEL=<label>|UUID=<uuid>\n");
		exit(2);
	}
	dev = blkid_get_devname(NULL, argv[1], NULL);
	if (!dev) {
		com_err("findfs", 0, _("Unable to resolve '%s'"), 
			argv[1]);
		exit(1);
	}
	puts(dev);
	exit(0);
}


int main (int argc, char ** argv)
{
	errcode_t retval;
	ext2_filsys fs;
	struct ext2_super_block *sb;
	io_manager io_ptr;

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
#endif
	if (argc && *argv)
		program_name = *argv;
	initialize_ext2_error_table();

	if (strcmp(get_progname(argv[0]), "findfs") == 0)
		do_findfs(argc, argv);
	if (strcmp(get_progname(argv[0]), "e2label") == 0)
		parse_e2label_options(argc, argv);
	else
		parse_tune2fs_options(argc, argv);
	
#ifdef CONFIG_TESTIO_DEBUG
	io_ptr = test_io_manager;
	test_io_backing_manager = unix_io_manager;
#else
	io_ptr = unix_io_manager;
#endif
	retval = ext2fs_open2(device_name, io_options, open_flag, 
			      0, 0, io_ptr, &fs);
        if (retval) {
		com_err (program_name, retval, _("while trying to open %s"),
			 device_name);
		fprintf(stderr,
			_("Couldn't find valid filesystem superblock.\n"));
		exit(1);
	}
	sb = fs->super;
	if (print_label) {
		/* For e2label emulation */
		printf("%.*s\n", (int) sizeof(sb->s_volume_name),
		       sb->s_volume_name);
		exit(0);
	}
	retval = ext2fs_check_if_mounted(device_name, &mount_flags);
	if (retval) {
		com_err("ext2fs_check_if_mount", retval,
			_("while determining whether %s is mounted."),
			device_name);
		exit(1);
	}
	/* Normally we only need to write out the superblock */
	fs->flags |= EXT2_FLAG_SUPER_ONLY;

	if (c_flag) {
		sb->s_max_mnt_count = max_mount_count;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting maximal mount count to %d\n"),
			max_mount_count);
	}
	if (C_flag) {
		sb->s_mnt_count = mount_count;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting current mount count to %d\n"), mount_count);
	}
	if (e_flag) {
		sb->s_errors = errors;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting error behavior to %d\n"), errors);
	}
	if (g_flag) {
		sb->s_def_resgid = resgid;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting reserved blocks gid to %lu\n"), resgid);
	}
	if (i_flag) {
		sb->s_checkinterval = interval;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting interval between checks to %lu seconds\n"), interval);
	}
	if (m_flag) {
		sb->s_r_blocks_count = (sb->s_blocks_count / 100)
			* reserved_ratio;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting reserved blocks percentage to %lu (%u blocks)\n"),
			reserved_ratio, sb->s_r_blocks_count);
	}
	if (r_flag) {
		if (reserved_blocks >= sb->s_blocks_count/2) {
			com_err (program_name, 0,
				 _("reserved blocks count is too big (%lu)"),
				 reserved_blocks);
			exit (1);
		}
		sb->s_r_blocks_count = reserved_blocks;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting reserved blocks count to %lu\n"),
			reserved_blocks);
	}
	if (s_flag == 1) {
		if (sb->s_feature_ro_compat &
		    EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)
			fputs(_("\nThe filesystem already has sparse "
				"superblocks.\n"), stderr);
		else {
			sb->s_feature_ro_compat |=
				EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
			sb->s_state &= ~EXT2_VALID_FS;
			ext2fs_mark_super_dirty(fs);
			printf(_("\nSparse superblock flag set.  %s"),
			       _(please_fsck));
		}
	}
	if (s_flag == 0) {
		if (!(sb->s_feature_ro_compat &
		      EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER))
			fputs(_("\nThe filesystem already has sparse "
				"superblocks disabled.\n"), stderr);
		else {
			sb->s_feature_ro_compat &=
				~EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
			sb->s_state &= ~EXT2_VALID_FS;
			fs->flags |= EXT2_FLAG_MASTER_SB_ONLY;
			ext2fs_mark_super_dirty(fs);
			printf(_("\nSparse superblock flag cleared.  %s"),
			       _(please_fsck));
		}
	}
	if (T_flag) {
		sb->s_lastcheck = last_check_time;
		ext2fs_mark_super_dirty(fs);
		printf(_("Setting time filesystem last checked to %s\n"),
		       ctime(&last_check_time));
	}
	if (u_flag) {
		sb->s_def_resuid = resuid;
		ext2fs_mark_super_dirty(fs);
		printf (_("Setting reserved blocks uid to %lu\n"), resuid);
	}
	if (L_flag) {
		if (strlen(new_label) > sizeof(sb->s_volume_name))
			fputs(_("Warning: label too long, truncating.\n"), 
			      stderr);
		memset(sb->s_volume_name, 0, sizeof(sb->s_volume_name));
		strncpy(sb->s_volume_name, new_label,
			sizeof(sb->s_volume_name));
		ext2fs_mark_super_dirty(fs);
	}
	if (M_flag) {
		memset(sb->s_last_mounted, 0, sizeof(sb->s_last_mounted));
		strncpy(sb->s_last_mounted, new_last_mounted,
			sizeof(sb->s_last_mounted));
		ext2fs_mark_super_dirty(fs);
	}
	if (mntopts_cmd)
		update_mntopts(fs, mntopts_cmd);
	if (features_cmd)
		update_feature_set(fs, features_cmd);
	if (journal_size || journal_device)
		add_journal(fs);
	
	if (U_flag) {
		if ((strcasecmp(new_UUID, "null") == 0) ||
		    (strcasecmp(new_UUID, "clear") == 0)) {
			uuid_clear(sb->s_uuid);
		} else if (strcasecmp(new_UUID, "time") == 0) {
			uuid_generate_time(sb->s_uuid);
		} else if (strcasecmp(new_UUID, "random") == 0) {
			uuid_generate(sb->s_uuid);
		} else if (uuid_parse(new_UUID, sb->s_uuid)) {
			com_err(program_name, 0, _("Invalid UUID format\n"));
			exit(1);
		}
		ext2fs_mark_super_dirty(fs);
	}

	if (l_flag)
		list_super (sb);
	return (ext2fs_close (fs) ? 1 : 0);
}
