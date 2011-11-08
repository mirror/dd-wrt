/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
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
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "io.h"
#include "print.h"
#include "bit.h"
#include "flist.h"
#include "strvec.h"
#include "output.h"
#include "sig.h"
#include "write.h"

static void	print_allfields(const struct field *fields);
static int	print_f(int argc, char **argv);
static void	print_flist_1(struct flist *flist, char **pfx, int parentoff);
static void	print_somefields(const struct field *fields, int argc,
				 char **argv);

static const cmdinfo_t	print_cmd =
	{ "print", "p", print_f, 0, -1, 0, N_("[value]..."),
	  N_("print field values"), NULL };

static void
print_allfields(
	const field_t	*fields)
{
	flist_t		*flist;
#ifdef DEBUG
	int		i;
#endif

	flist = flist_make("");
	flist->fld = fields;
#ifndef DEBUG
	(void)flist_parse(fields, flist, iocur_top->data, 0);
#else
	i = flist_parse(fields, flist, iocur_top->data, 0);
	ASSERT(i == 1);
#endif
	flist_print(flist);
	print_flist(flist);
	flist_free(flist);
}

static int
print_f(
	int	argc,
	char	**argv)
{
	pfunc_t	pf;

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}
	pf = cur_typ->pfunc;
	if (pf == NULL) {
		dbprintf(_("no print function for type %s\n"), cur_typ->name);
		return 0;
	}
	argc--;
	argv++;
	(*pf)(DB_READ, cur_typ->fields, argc, argv);
	return 0;
}

void
print_flist(
	flist_t	*flist)
{
	char	**pfx;

	pfx = new_strvec(0);
	print_flist_1(flist, pfx, 0);
	free_strvec(pfx);
}

static void
print_flist_1(
	flist_t		*flist,
	char		**ppfx,
	int		parentoff)
{
	char		buf[16];
	const field_t	*f;
	const ftattr_t	*fa;
	flist_t		*fl;
	int		low;
	int		neednl;
	char		**pfx;

	for (fl = flist; fl && !seenint(); fl = fl->sibling) {
		pfx = copy_strvec(ppfx);
		if (fl->name[0])
			add_strvec(&pfx, fl->name);
		if (fl->flags & FL_OKLOW) {
			add_strvec(&pfx, "[");
			snprintf(buf, sizeof(buf), "%d", fl->low);
			add_strvec(&pfx, buf);
			if (fl->low != fl->high) {
				add_strvec(&pfx, "-");
				snprintf(buf, sizeof(buf), "%d", fl->high);
				add_strvec(&pfx, buf);
			}
			add_strvec(&pfx, "]");
		}
		if (fl->child) {
			if (fl->name[0])
				add_strvec(&pfx, ".");
			print_flist_1(fl->child, pfx, fl->offset);
		} else {
			f = fl->fld;
			fa = &ftattrtab[f->ftyp];
			ASSERT(fa->ftyp == f->ftyp);
			print_strvec(pfx);
			dbprintf(" = ");
			if (fl->flags & FL_OKLOW)
				low = fl->low;
			else
				low = 0;
			if (fa->prfunc) {
				neednl = fa->prfunc(iocur_top->data, fl->offset,
					fcount(f, iocur_top->data, parentoff),
					fa->fmtstr,
					fsize(f, iocur_top->data, parentoff, 0),
					fa->arg, low,
					(f->flags & FLD_ARRAY) != 0);
				if (neednl)
					dbprintf("\n");
			} else {
				ASSERT(fa->arg & FTARG_OKEMPTY);
				dbprintf(_("(empty)\n"));
			}
		}
		free_strvec(pfx);
	}
}

void
print_init(void)
{
	add_command(&print_cmd);
}

void
print_sarray(
	void		*obj,
	int		bit,
	int		count,
	int		size,
	int		base,
	int		array,
	const field_t	*flds,
	int		skipnms)
{
	int		bitoff;
	const field_t	*f;
	const ftattr_t	*fa;
	int		first;
	int		i;

	ASSERT(bitoffs(bit) == 0);
	if (skipnms == 0) {
		for (f = flds, first = 1; f->name; f++) {
			if (f->flags & FLD_SKIPALL)
				continue;
			dbprintf("%c%s", first ? '[' : ',', f->name);
			first = 0;
		}
		dbprintf("] ");
	}
	for (i = 0, bitoff = bit;
	     i < count && !seenint();
	     i++, bitoff += size) {
		if (array)
			dbprintf("%d:", i + base);
		for (f = flds, first = 1; f->name; f++) {
			if (f->flags & FLD_SKIPALL)
				continue;
			fa = &ftattrtab[f->ftyp];
			ASSERT(fa->ftyp == f->ftyp);
			dbprintf("%c", first ? '[' : ',');
			first = 0;
			if (fa->prfunc)
				fa->prfunc(obj,
					bitoff +
					    bitoffset(f, obj, bitoff, i + base),
					fcount(f, obj, bitoff), fa->fmtstr,
					fsize(f, obj, bitoff, i + base),
					fa->arg, (f->flags & FLD_ABASE1) != 0,
					f->flags & FLD_ARRAY);
			else {
				ASSERT(fa->arg & FTARG_OKEMPTY);
				dbprintf(_("(empty)"));
			}
		}
		dbprintf("]");
		if (i < count - 1)
			dbprintf(" ");
	}
}

static void
print_somefields(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	const ftattr_t	*fa;
	flist_t		*fl;
	flist_t		*lfl;
	flist_t		*nfl;

	fl = lfl = NULL;
	while (argc > 0) {
		nfl = flist_scan(*argv);
		if (!nfl) {
			if (fl)
				flist_free(fl);
			return;
		}
		if (lfl)
			lfl->sibling = nfl;
		else
			fl = nfl;
		lfl = nfl;
		argc--;
		argv++;
	}
	if (fields->name[0] == '\0') {
		fa = &ftattrtab[fields->ftyp];
		ASSERT(fa->ftyp == fields->ftyp);
		fields = fa->subfld;
	}
	if (!flist_parse(fields, fl, iocur_top->data, 0)) {
		flist_free(fl);
		return;
	}
	flist_print(fl);
	print_flist(fl);
	flist_free(fl);
}

/*ARGSUSED*/
void
print_string(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	char		*cp;

	if (argc != 0)
		dbprintf(_("no arguments allowed\n"));
	dbprintf("\"");
	for (cp = iocur_top->data;
	     cp < (char *)iocur_top->data + iocur_top->len && *cp &&
		     !seenint();
	     cp++)
		dbprintf("%c", *cp);
	dbprintf("\"\n");
}

void
print_struct(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	if (argc == 0)
		print_allfields(fields);
	else
		print_somefields(fields, argc, argv);
}
