// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "block.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "print.h"
#include "sb.h"
#include "inode.h"
#include "btblock.h"
#include "bmroot.h"
#include "agf.h"
#include "agfl.h"
#include "agi.h"
#include "io.h"
#include "output.h"
#include "write.h"
#include "attr.h"
#include "dquot.h"
#include "dir2.h"
#include "text.h"
#include "symlink.h"
#include "fuzz.h"

static const typ_t	*findtyp(char *name);
static int		type_f(int argc, char **argv);

const typ_t	*cur_typ;

static const cmdinfo_t	type_cmd =
	{ "type", NULL, type_f, 0, 1, 1, N_("[newtype]"),
	  N_("set/show current data type"), NULL };

static const typ_t	__typtab[] = {
	{ TYP_AGF, "agf", handle_struct, agf_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_AGFL, "agfl", handle_struct, agfl_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_AGI, "agi", handle_struct, agi_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_ATTR, "attr", handle_struct, attr_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_BMAPBTA, "bmapbta", handle_struct, bmapbta_hfld, NULL,
		TYP_F_NO_CRC_OFF },
	{ TYP_BMAPBTD, "bmapbtd", handle_struct, bmapbtd_hfld, NULL,
		TYP_F_NO_CRC_OFF },
	{ TYP_BNOBT, "bnobt", handle_struct, bnobt_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_CNTBT, "cntbt", handle_struct, cntbt_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RMAPBT, NULL },
	{ TYP_REFCBT, NULL },
	{ TYP_DATA, "data", handle_block, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_DIR2, "dir2", handle_struct, dir2_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_DQBLK, "dqblk", handle_struct, dqblk_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_INOBT, "inobt", handle_struct, inobt_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_INODATA, "inodata", NULL, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_INODE, "inode", handle_struct, inode_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_LOG, "log", NULL, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RTBITMAP, "rtbitmap", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RTSUMMARY, "rtsummary", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_SB, "sb", handle_struct, sb_hfld, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_SYMLINK, "symlink", handle_string, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_TEXT, "text", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_FINOBT, "finobt", handle_struct, finobt_hfld, NULL,
		TYP_F_NO_CRC_OFF },
	{ TYP_NONE, NULL }
};

static const typ_t	__typtab_crc[] = {
	{ TYP_AGF, "agf", handle_struct, agf_hfld, &xfs_agf_buf_ops,
		XFS_AGF_CRC_OFF },
	{ TYP_AGFL, "agfl", handle_struct, agfl_crc_hfld, &xfs_agfl_buf_ops,
		XFS_AGFL_CRC_OFF },
	{ TYP_AGI, "agi", handle_struct, agi_hfld, &xfs_agi_buf_ops,
		XFS_AGI_CRC_OFF },
	{ TYP_ATTR, "attr3", handle_struct, attr3_hfld,
		&xfs_attr3_db_buf_ops, TYP_F_CRC_FUNC, xfs_attr3_set_crc },
	{ TYP_BMAPBTA, "bmapbta", handle_struct, bmapbta_crc_hfld,
		&xfs_bmbt_buf_ops, XFS_BTREE_LBLOCK_CRC_OFF },
	{ TYP_BMAPBTD, "bmapbtd", handle_struct, bmapbtd_crc_hfld,
		&xfs_bmbt_buf_ops, XFS_BTREE_LBLOCK_CRC_OFF },
	{ TYP_BNOBT, "bnobt", handle_struct, bnobt_crc_hfld,
		&xfs_bnobt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_CNTBT, "cntbt", handle_struct, cntbt_crc_hfld,
		&xfs_cntbt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_RMAPBT, "rmapbt", handle_struct, rmapbt_crc_hfld,
		&xfs_rmapbt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_REFCBT, "refcntbt", handle_struct, refcbt_crc_hfld,
		&xfs_refcountbt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_DATA, "data", handle_block, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_DIR2, "dir3", handle_struct, dir3_hfld,
		&xfs_dir3_db_buf_ops, TYP_F_CRC_FUNC, xfs_dir3_set_crc },
	{ TYP_DQBLK, "dqblk", handle_struct, dqblk_hfld,
		&xfs_dquot_buf_ops, TYP_F_CRC_FUNC, xfs_dquot_set_crc },
	{ TYP_INOBT, "inobt", handle_struct, inobt_crc_hfld,
		&xfs_inobt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_INODATA, "inodata", NULL, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_INODE, "inode", handle_struct, inode_crc_hfld,
		&xfs_inode_buf_ops, TYP_F_CRC_FUNC, xfs_inode_set_crc },
	{ TYP_LOG, "log", NULL, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RTBITMAP, "rtbitmap", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RTSUMMARY, "rtsummary", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_SB, "sb", handle_struct, sb_hfld, &xfs_sb_buf_ops,
		XFS_SB_CRC_OFF },
	{ TYP_SYMLINK, "symlink", handle_struct, symlink_crc_hfld,
		&xfs_symlink_buf_ops, XFS_SYMLINK_CRC_OFF },
	{ TYP_TEXT, "text", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_FINOBT, "finobt", handle_struct, finobt_crc_hfld,
		&xfs_finobt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_NONE, NULL }
};

static const typ_t	__typtab_spcrc[] = {
	{ TYP_AGF, "agf", handle_struct, agf_hfld, &xfs_agf_buf_ops,
		XFS_AGF_CRC_OFF },
	{ TYP_AGFL, "agfl", handle_struct, agfl_crc_hfld, &xfs_agfl_buf_ops ,
		XFS_AGFL_CRC_OFF },
	{ TYP_AGI, "agi", handle_struct, agi_hfld, &xfs_agi_buf_ops ,
		XFS_AGI_CRC_OFF },
	{ TYP_ATTR, "attr3", handle_struct, attr3_hfld,
		&xfs_attr3_db_buf_ops, TYP_F_CRC_FUNC, xfs_attr3_set_crc },
	{ TYP_BMAPBTA, "bmapbta", handle_struct, bmapbta_crc_hfld,
		&xfs_bmbt_buf_ops, XFS_BTREE_LBLOCK_CRC_OFF },
	{ TYP_BMAPBTD, "bmapbtd", handle_struct, bmapbtd_crc_hfld,
		&xfs_bmbt_buf_ops, XFS_BTREE_LBLOCK_CRC_OFF },
	{ TYP_BNOBT, "bnobt", handle_struct, bnobt_crc_hfld,
		&xfs_bnobt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_CNTBT, "cntbt", handle_struct, cntbt_crc_hfld,
		&xfs_cntbt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_RMAPBT, "rmapbt", handle_struct, rmapbt_crc_hfld,
		&xfs_rmapbt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_REFCBT, "refcntbt", handle_struct, refcbt_crc_hfld,
		&xfs_refcountbt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_DATA, "data", handle_block, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_DIR2, "dir3", handle_struct, dir3_hfld,
		&xfs_dir3_db_buf_ops, TYP_F_CRC_FUNC, xfs_dir3_set_crc },
	{ TYP_DQBLK, "dqblk", handle_struct, dqblk_hfld,
		&xfs_dquot_buf_ops, TYP_F_CRC_FUNC, xfs_dquot_set_crc },
	{ TYP_INOBT, "inobt", handle_struct, inobt_spcrc_hfld,
		&xfs_inobt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_INODATA, "inodata", NULL, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_INODE, "inode", handle_struct, inode_crc_hfld,
		&xfs_inode_buf_ops, TYP_F_CRC_FUNC, xfs_inode_set_crc },
	{ TYP_LOG, "log", NULL, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RTBITMAP, "rtbitmap", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_RTSUMMARY, "rtsummary", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_SB, "sb", handle_struct, sb_hfld, &xfs_sb_buf_ops,
		XFS_SB_CRC_OFF },
	{ TYP_SYMLINK, "symlink", handle_struct, symlink_crc_hfld,
		&xfs_symlink_buf_ops, XFS_SYMLINK_CRC_OFF },
	{ TYP_TEXT, "text", handle_text, NULL, NULL, TYP_F_NO_CRC_OFF },
	{ TYP_FINOBT, "finobt", handle_struct, finobt_spcrc_hfld,
		&xfs_finobt_buf_ops, XFS_BTREE_SBLOCK_CRC_OFF },
	{ TYP_NONE, NULL }
};

const typ_t	*typtab = __typtab;

void
type_set_tab_crc(void)
{
	typtab = __typtab_crc;
}

void
type_set_tab_spcrc(void)
{
	typtab = __typtab_spcrc;
}

static const typ_t *
findtyp(
	char		*name)
{
	const typ_t	*tt;

	for (tt = typtab; tt->typnm != TYP_NONE; tt++) {
		ASSERT(tt->typnm == (typnm_t)(tt - typtab));
		if (tt->name && strcmp(tt->name, name) == 0)
			return tt;
	}
	return NULL;
}

static int
type_f(
	int		argc,
	char		**argv)
{
	const typ_t	*tt;
	int count = 0;

	if (argc == 1) {
		if (cur_typ == NULL)
			dbprintf(_("no current type\n"));
		else
			dbprintf(_("current type is \"%s\"\n"), cur_typ->name);

		dbprintf(_("\n supported types are:\n "));
		for (tt = typtab, count = 0; tt->typnm != TYP_NONE; tt++) {
			if (tt->name == NULL)
				continue;
			if ((tt+1)->name != NULL) {
				dbprintf("%s, ", tt->name);
				if ((++count % 8) == 0)
					dbprintf("\n ");
			} else if ((tt+1)->typnm == TYP_NONE) {
				dbprintf("%s\n", tt->name);
			}
		}


	} else {
		tt = findtyp(argv[1]);
		if (tt == NULL) {
			dbprintf(_("no such type %s\n"), argv[1]);
		} else if (iocur_top->typ == tt) {
			return 0;
		} else {
			if (iocur_top->typ == NULL)
				dbprintf(_("no current object\n"));
			else {
				cur_typ = tt;
				set_iocur_type(tt);
			}
		}
	}
	return 0;
}

void
type_init(void)
{
	add_command(&type_cmd);
}

/* read/write selectors for each major data type */

void
handle_struct(
	int           action,
	const field_t *fields,
	int           argc,
	char          **argv)
{
	switch (action) {
	case DB_FUZZ:
		fuzz_struct(fields, argc, argv);
		break;
	case DB_WRITE:
		write_struct(fields, argc, argv);
		break;
	case DB_READ:
		print_struct(fields, argc, argv);
		break;
	}
}

void
handle_string(
	int           action,
	const field_t *fields,
	int           argc,
	char          **argv)
{
	switch (action) {
	case DB_WRITE:
		write_string(fields, argc, argv);
		break;
	case DB_READ:
		print_string(fields, argc, argv);
		break;
	case DB_FUZZ:
		dbprintf(_("string fuzzing not supported.\n"));
		break;
	}
}

void
handle_block(
	int           action,
	const field_t *fields,
	int           argc,
	char          **argv)
{
	switch (action) {
	case DB_WRITE:
		write_block(fields, argc, argv);
		break;
	case DB_READ:
		print_block(fields, argc, argv);
		break;
	case DB_FUZZ:
		dbprintf(_("use 'blocktrash' or 'write' to fuzz a block.\n"));
		break;
	}
}

void
handle_text(
	int           action,
	const field_t *fields,
	int           argc,
	char          **argv)
{
	switch (action) {
	case DB_FUZZ:
		fallthrough;
	case DB_WRITE:
		dbprintf(_("text writing/fuzzing not supported.\n"));
		break;
	case DB_READ:
		print_text(fields, argc, argv);
		break;
	}
}
