// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "addr.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "flist.h"
#include "io.h"
#include "init.h"
#include "output.h"
#include "bit.h"
#include "print.h"
#include "crc.h"

static int crc_f(int argc, char **argv);
static void crc_help(void);

static const cmdinfo_t crc_cmd =
	{ "crc", NULL, crc_f, 0, 1, 0, "[-i|-r|-v]",
	  N_("manipulate crc values for V5 filesystem structures"), crc_help };

void
crc_init(void)
{
	if (xfs_has_crc(mp))
		add_command(&crc_cmd);
}

static void
crc_help(void)
{
	dbprintf(_(
"\n"
" 'crc' validates, invalidates, or recalculates the crc value for\n"
" the current on-disk metadata structures in Version 5 filesystems.\n"
"\n"
" Usage:  \"crc [-i|-r|-v]\"\n"
"\n"
));

}

static int
crc_f(
	int		argc,
	char		**argv)
{
	const struct xfs_buf_ops *stashed_ops = NULL;
	struct xfs_buf_ops nowrite_ops;
	extern char	*progname;
	const field_t	*fields;
	const ftattr_t	*fa;
	flist_t		*fl;
	int		invalidate = 0;
	int		recalculate = 0;
	int		validate = 0;
	int		c;

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}

	if (cur_typ->fields == NULL) {
		dbprintf(_("current type (%s) is not a structure\n"),
			 cur_typ->name);
		return 0;
	}

	if (argc) while ((c = getopt(argc, argv, "irv")) != EOF) {
		switch (c) {
		case 'i':
			invalidate = 1;
			break;
		case 'r':
			recalculate = 1;
			break;
		case 'v':
			validate = 1;
			break;
		default:
			dbprintf(_("bad option for crc command\n"));
			return 0;
		}
	} else
		validate = 1;

	if (invalidate + recalculate + validate > 1) {
		dbprintf(_("crc command accepts only one option\n"));
		return 0;
	}

	if ((invalidate || recalculate) &&
	    ((x.isreadonly & LIBXFS_ISREADONLY) || !expert_mode)) {
		dbprintf(_("%s not in expert mode, writing disabled\n"),
			progname);
		return 0;
	}

	fields = cur_typ->fields;

	/* if we're a root field type, go down 1 layer to get field list */
	if (fields->name[0] == '\0') {
		fa = &ftattrtab[fields->ftyp];
		ASSERT(fa->ftyp == fields->ftyp);
		fields = fa->subfld;
	}

	/* Search for a CRC field */
	fl = flist_find_ftyp(fields, FLDT_CRC, iocur_top->data, 0);
	if (!fl) {
		dbprintf(_("No CRC field found for type %s\n"), cur_typ->name);
		return 0;
	}

	/* run down the field list and set offsets into the data */
	if (!flist_parse(fields, fl, iocur_top->data, 0)) {
		flist_free(fl);
		dbprintf(_("parsing error\n"));
		return 0;
	}

	if (invalidate) {
		flist_t		*sfl;
		int		bit_length;
		int		parentoffset;
		uint32_t	crc;

		sfl = fl;
		parentoffset = 0;
		while (sfl->child) {
			parentoffset = sfl->offset;
			sfl = sfl->child;
		}
		ASSERT(sfl->fld->ftyp == FLDT_CRC);

		bit_length = fsize(sfl->fld, iocur_top->data, parentoffset, 0);
		bit_length *= fcount(sfl->fld, iocur_top->data, parentoffset);
		crc = getbitval(iocur_top->data, sfl->offset, bit_length,
				BVUNSIGNED);
		/* Off by one, ignore endianness - we're just corrupting it. */
		crc++;
		setbitval(iocur_top->data, sfl->offset, bit_length, &crc);

		/* Temporarily remove write verifier to write a bad CRC */
		stashed_ops = iocur_top->bp->b_ops;
		nowrite_ops.verify_read = stashed_ops->verify_read;
		nowrite_ops.verify_write = xfs_dummy_verify;
		iocur_top->bp->b_ops = &nowrite_ops;
	}

	if (invalidate || recalculate) {
		if (invalidate)
			dbprintf(_("Invalidating CRC:\n"));
		else
			dbprintf(_("Recalculating CRC:\n"));

		write_cur();
		if (stashed_ops)
			iocur_top->bp->b_ops = stashed_ops;
		/* re-verify to get proper b_error state */
		iocur_top->bp->b_ops->verify_read(iocur_top->bp);
	} else
		dbprintf(_("Verifying CRC:\n"));

	/* And show us what we've got! */
	flist_print(fl);
	print_flist(fl);
	flist_free(fl);
	return 0;
}
