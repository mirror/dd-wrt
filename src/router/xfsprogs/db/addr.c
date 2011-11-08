/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

#include <xfs/libxfs.h>
#include "addr.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "io.h"
#include "flist.h"
#include "inode.h"
#include "output.h"

static int addr_f(int argc, char **argv);
static void addr_help(void);

static const cmdinfo_t addr_cmd =
	{ "addr", "a", addr_f, 0, 1, 1, N_("[field-expression]"),
	  N_("set current address"), addr_help };

static void
addr_help(void)
{
	dbprintf(_(
"\n"
" 'addr' uses the given field to set the filesystem address and type\n"
"\n"
" Examples:\n"
"\n"
" sb\n"
" a rootino - set the type to inode and set position to the root inode\n"
" a u.bmx[0].startblock (for inode with blockmap)\n"
"\n"
));

}

static int
addr_f(
	int		argc,
	char		**argv)
{
	adfnc_t		adf;
	const ftattr_t	*fa;
	flist_t		*fl;
	const field_t	*fld;
	typnm_t		next;
	flist_t		*tfl;

	if (argc == 1) {
		print_iocur("current", iocur_top);
		return 0;
	}
	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}
	fld = cur_typ->fields;
	if (fld != NULL && fld->name[0] == '\0') {
		fa = &ftattrtab[fld->ftyp];
		ASSERT(fa->ftyp == fld->ftyp);
		fld = fa->subfld;
	}
	if (fld == NULL) {
		dbprintf(_("no fields for type %s\n"), cur_typ->name);
		return 0;
	}
	fl = flist_scan(argv[1]);
	if (fl == NULL)
		return 0;
	if (!flist_parse(fld, fl, iocur_top->data, 0)) {
		flist_free(fl);
		return 0;
	}
	flist_print(fl);
	for (tfl = fl; tfl->child != NULL; tfl = tfl->child) {
		if ((tfl->flags & FL_OKLOW) && tfl->low < tfl->high) {
			dbprintf(_("array not allowed for addr command\n"));
			flist_free(fl);
			return 0;
		}
	}
	fld = tfl->fld;
	next = fld->next;
	if (next == TYP_INODATA)
		next = inode_next_type();
	if (next == TYP_NONE) {
		dbprintf(_("no next type for field %s\n"), fld->name);
		return 0;
	}
	fa = &ftattrtab[fld->ftyp];
	ASSERT(fa->ftyp == fld->ftyp);
	adf = fa->adfunc;
	if (adf == NULL) {
		dbprintf(_("no addr function for field %s (type %s)\n"),
			fld->name, fa->name);
		return 0;
	}
	(*adf)(iocur_top->data, tfl->offset, next);
	flist_free(fl);
	return 0;
}

void
addr_init(void)
{
	add_command(&addr_cmd);
}
