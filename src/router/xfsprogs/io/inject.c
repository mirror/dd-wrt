// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"
#include "xfs_errortag.h"

static cmdinfo_t inject_cmd;

static int
error_tag(char *name)
{
	static struct {
		int	tag;
		char	*name;
	} *e, eflags[] = {
		{ XFS_ERRTAG_NOERROR,			"noerror" },
		{ XFS_ERRTAG_IFLUSH_1,			"iflush1" },
		{ XFS_ERRTAG_IFLUSH_2,			"iflush2" },
		{ XFS_ERRTAG_IFLUSH_3,			"iflush3" },
		{ XFS_ERRTAG_IFLUSH_4,			"iflush4" },
		{ XFS_ERRTAG_IFLUSH_5,			"iflush5" },
		{ XFS_ERRTAG_IFLUSH_6,			"iflush6" },
		{ XFS_ERRTAG_DA_READ_BUF,		"dareadbuf" },
		{ XFS_ERRTAG_BTREE_CHECK_LBLOCK,	"btree_chk_lblk" },
		{ XFS_ERRTAG_BTREE_CHECK_SBLOCK,	"btree_chk_sblk" },
		{ XFS_ERRTAG_ALLOC_READ_AGF,		"readagf" },
		{ XFS_ERRTAG_IALLOC_READ_AGI,		"readagi" },
		{ XFS_ERRTAG_ITOBP_INOTOBP,		"itobp" },
		{ XFS_ERRTAG_IUNLINK,			"iunlink" },
		{ XFS_ERRTAG_IUNLINK_REMOVE,		"iunlinkrm" },
		{ XFS_ERRTAG_DIR_INO_VALIDATE,		"dirinovalid" },
		{ XFS_ERRTAG_BULKSTAT_READ_CHUNK,	"bulkstat" },
		{ XFS_ERRTAG_IODONE_IOERR,		"logiodone" },
		{ XFS_ERRTAG_STRATREAD_IOERR,		"stratread" },
		{ XFS_ERRTAG_STRATCMPL_IOERR,		"stratcmpl" },
		{ XFS_ERRTAG_DIOWRITE_IOERR,		"diowrite" },
		{ XFS_ERRTAG_BMAPIFORMAT,		"bmapifmt" },
		{ XFS_ERRTAG_FREE_EXTENT,		"free_extent" },
		{ XFS_ERRTAG_RMAP_FINISH_ONE,		"rmap_finish_one" },
		{ XFS_ERRTAG_REFCOUNT_CONTINUE_UPDATE,	"refcount_continue_update" },
		{ XFS_ERRTAG_REFCOUNT_FINISH_ONE,	"refcount_finish_one" },
		{ XFS_ERRTAG_BMAP_FINISH_ONE,		"bmap_finish_one" },
		{ XFS_ERRTAG_AG_RESV_CRITICAL,		"ag_resv_critical" },
		{ XFS_ERRTAG_DROP_WRITES,		"drop_writes" },
		{ XFS_ERRTAG_LOG_BAD_CRC,		"log_bad_crc" },
		{ XFS_ERRTAG_LOG_ITEM_PIN,		"log_item_pin" },
		{ XFS_ERRTAG_BUF_LRU_REF,		"buf_lru_ref" },
		{ XFS_ERRTAG_FORCE_SCRUB_REPAIR,	"force_repair" },
		{ XFS_ERRTAG_FORCE_SUMMARY_RECALC,	"bad_summary" },
		{ XFS_ERRTAG_IUNLINK_FALLBACK,		"iunlink_fallback" },
		{ XFS_ERRTAG_BUF_IOERROR,		"buf_ioerror" },
		{ XFS_ERRTAG_REDUCE_MAX_IEXTENTS,	"reduce_max_iextents" },
		{ XFS_ERRTAG_BMAP_ALLOC_MINLEN_EXTENT,	"bmap_alloc_minlen_extent" },
		{ XFS_ERRTAG_AG_RESV_FAIL,		"ag_resv_fail" },
		{ XFS_ERRTAG_LARP,			"larp" },
		{ XFS_ERRTAG_DA_LEAF_SPLIT,		"da_leaf_split" },
		{ XFS_ERRTAG_ATTR_LEAF_TO_NODE,		"attr_leaf_to_node" },
		{ XFS_ERRTAG_WB_DELAY_MS,		"wb_delay_ms" },
		{ XFS_ERRTAG_WRITE_DELAY_MS,		"write_delay_ms" },
		{ XFS_ERRTAG_MAX,			NULL }
	};
	int	count;

	/*
	 * If this fails make sure every tag is defined in the array above,
	 * see xfs_errortag_attrs in kernelspace.
	 */
	BUILD_BUG_ON(sizeof(eflags) != (XFS_ERRTAG_MAX + 1) * sizeof(*e));

	/* Search for a name */
	if (name) {
		for (e = eflags; e->name; e++)
			if (strcmp(name, e->name) == 0)
				return e->tag;
		return -1;
	}

	/* Dump all the names */
	fputs("tags: [ ", stdout);
	for (count = 0, e = eflags; e->name; e++, count++) {
		if (count) {
			fputs(", ", stdout);
			if (!(count % 5))
				fputs("\n\t", stdout);
		}
		fputs(e->name, stdout);
	}
	fputs(" ]\n", stdout);
	return 0;
}

static void
inject_help(void)
{
	printf(_(
"\n"
" inject errors into the filesystem of the currently open file\n"
"\n"
" Example:\n"
" 'inject readagf' - cause errors on allocation group freespace reads\n"
"\n"
" Causes the kernel to generate and react to errors within XFS, provided\n"
" the XFS kernel code has been built with debugging features enabled.\n"
" With no arguments, displays the list of error injection tags.\n"
"\n"));
}

static int
inject_f(
	int			argc,
	char			**argv)
{
	xfs_error_injection_t	error;
	int			command = XFS_IOC_ERROR_INJECTION;

	if (argc == 1)
		return error_tag(NULL);

	while (--argc > 0) {
		error.fd = file->fd;
		if ((error.errtag = error_tag(argv[argc])) < 0) {
			fprintf(stderr, _("no such tag -- %s\n"), argv[1]);
			continue;
		}
		if (error.errtag == XFS_ERRTAG_NOERROR)
			command = XFS_IOC_ERROR_CLEARALL;
		if ((xfsctl(file->name, file->fd, command, &error)) < 0) {
			perror("XFS_IOC_ERROR_INJECTION");
			exitcode = 1;
			continue;
		}
	}
	return 0;
}

void
inject_init(void)
{
	inject_cmd.name = "inject";
	inject_cmd.cfunc = inject_f;
	inject_cmd.argmin = 0;
	inject_cmd.argmax = -1;
	inject_cmd.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT;
	inject_cmd.args = _("[tag ...]");
	inject_cmd.oneline = _("inject errors into a filesystem");
	inject_cmd.help = inject_help;

	if (expert)
		add_command(&inject_cmd);
}
