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
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "flist.h"
#include "debug.h"
#include "output.h"
#include "malloc.h"

static void	flist_expand_arrays(flist_t *fl);
static void	flist_expand_structs(flist_t *fl, void *obj);
static flist_t	*flist_replicate(flist_t *fl);
static ftok_t	*flist_split(char *s);
static void	ftok_free(ftok_t *ft);

static void
flist_expand_arrays(
	flist_t		*fl)
{
	const field_t	*f;
#ifdef DEBUG
	const ftattr_t	*fa;
#endif
	int		high;
	int		idx;
	int		low;
	flist_t		*new;
	flist_t		*prev;
	flist_t		*sib;

	f = fl->fld;
#ifdef DEBUG
	fa = &ftattrtab[f->ftyp];
#endif
	ASSERT(fa->ftyp == f->ftyp);
	ASSERT(f->flags & FLD_ARRAY);
	low = fl->low;
	high = fl->high;
	fl->high = fl->low;
	sib = fl->sibling;
	for (idx = low + 1, prev = fl; idx <= high; idx++) {
		new = flist_make(f->name);
		new->fld = f;
		new->low = new->high = idx;
		new->flags |= FL_OKLOW | FL_OKHIGH;
		new->child = flist_replicate(fl->child);
		prev->sibling = new;
		prev = new;
	}
	prev->sibling = sib;
}

static void
flist_expand_structs(
	flist_t		*fl,
	void		*obj)
{
	const field_t	*cf;
	const field_t	*f;
	const ftattr_t	*fa;
	flist_t		*new;
	flist_t		*prev;

	f = fl->fld;
	fa = &ftattrtab[f->ftyp];
	ASSERT(fa->ftyp == f->ftyp);
	ASSERT(fa->subfld != NULL);
	ASSERT(fl->child == NULL);
	for (cf = fa->subfld, prev = NULL; cf->name != NULL; cf++) {
		if (fcount(cf, obj, fl->offset) == 0)
			continue;
		if (cf->flags & FLD_SKIPALL)
			continue;
		new = flist_make(cf->name);
		new->fld = cf;
		if (prev)
			prev->sibling = new;
		else
			fl->child = new;
		prev = new;
	}
}

void
flist_free(
	flist_t	*fl)
{
	if (fl->child)
		flist_free(fl->child);
	if (fl->sibling)
		flist_free(fl->sibling);
	if (fl->name)
		xfree(fl->name);
	xfree(fl);
}

flist_t *
flist_make(
	char	*name)
{
	flist_t	*fl;

	fl = xmalloc(sizeof(*fl));
	fl->name = xstrdup(name);
	fl->fld = NULL;
	fl->child = NULL;
	fl->sibling = NULL;
	fl->low = 0;
	fl->high = 0;
	fl->flags = 0;
	fl->offset = 0;
	return fl;
}

int
flist_parse(
	const field_t	*fields,
	flist_t		*fl,
	void		*obj,
	int		startoff)
{
	const field_t	*f;
	const ftattr_t	*fa;
	int		high;
	int		low;

	while (fl) {
		f = findfield(fl->name, fields, obj, startoff);
		if (f == NULL) {
			dbprintf(_("field %s not found\n"), fl->name);
			return 0;
		}
		fl->fld = f;
		fa = &ftattrtab[f->ftyp];
		ASSERT(fa->ftyp == f->ftyp);
		if (f->flags & FLD_ARRAY) {
			low = (f->flags & FLD_ABASE1) != 0;
			high = fcount(f, obj, startoff) + low - 1;
			if (low > high) {
				dbprintf(_("no elements in %s\n"), fl->name);
				return 0;
			}
			if (fl->flags & FL_OKHIGH) {
				if (fl->low < low || fl->low > high ||
				    fl->high < low || fl->high > high) {
					dbprintf(_("indices %d-%d for field %s "
						 "out of range %d-%d\n"),
						fl->low, fl->high, fl->name,
						low, high);
					return 0;
				}
			} else if (fl->flags & FL_OKLOW) {
				if (fl->low < low || fl->low > high) {
					dbprintf(_("index %d for field %s out of "
						 "range %d-%d\n"),
						fl->low, fl->name, low, high);
					return 0;
				}
				fl->high = fl->low;
				fl->flags |= FL_OKHIGH;
			} else {
				fl->low = low;
				fl->high = high;
				fl->flags |= FL_OKLOW | FL_OKHIGH;
			}
		} else {
			if (fl->flags & FL_OKLOW) {
				dbprintf(_("field %s is not an array\n"),
					fl->name);
				return 0;
			}
		}
		fl->offset = startoff + bitoffset(f, obj, startoff, fl->low);
		if ((fl->child != NULL || fa->prfunc == NULL) &&
		    (f->flags & FLD_ARRAY) && fl->low != fl->high)
			flist_expand_arrays(fl);
		if (fa->prfunc == NULL && fl->child == NULL)
			flist_expand_structs(fl, obj);
		if (fl->child) {
			if (fa->subfld == NULL) {
				dbprintf(_("field %s has no subfields\n"),
					fl->name);
				return 0;
			}
			if (!flist_parse(fa->subfld, fl->child, obj,
					fl->offset))
				return 0;
		}
		fl = fl->sibling;
	}
	return 1;
}

void
flist_print(
	flist_t	*fl)
{
	if (!(debug_state & DEBUG_FLIST))
		return;
	while (fl) {
		dbprintf(_("fl@%p:\n"), fl);
		dbprintf(_("\tname=%s, fld=%p, child=%p, sibling=%p\n"),
			fl->name, fl->fld, fl->child, fl->sibling);
		dbprintf(_("\tlow=%d, high=%d, flags=%d (%s%s), offset=%d\n"),
			fl->low, fl->high, fl->flags,
			fl->flags & FL_OKLOW ? _("oklow ") : "",
			fl->flags & FL_OKHIGH ? _("okhigh") : "", fl->offset);
		dbprintf(_("\tfld->name=%s, fld->ftyp=%d (%s)\n"),
			fl->fld->name, fl->fld->ftyp,
			ftattrtab[fl->fld->ftyp].name);
		dbprintf(_("\tfld->flags=%d (%s%s%s%s%s)\n"), fl->fld->flags,
			fl->fld->flags & FLD_ABASE1 ? "abase1 " : "",
			fl->fld->flags & FLD_SKIPALL ? "skipall " : "",
			fl->fld->flags & FLD_ARRAY ? "array " : "",
			fl->fld->flags & FLD_OFFSET ? "offset " : "",
			fl->fld->flags & FLD_COUNT ? "count " : "");
		if (fl->child)
			flist_print(fl->child);
		fl = fl->sibling;
	}
}

static flist_t *
flist_replicate(
	flist_t	*f)
{
	flist_t	*new;

	if (f == NULL)
		return NULL;
	new = flist_make(f->name);
	new->fld = f->fld;
	new->child = flist_replicate(f->child);
	new->sibling = flist_replicate(f->sibling);
	new->low = f->low;
	new->high = f->high;
	new->flags = f->flags;
	new->offset = f->offset;
	return new;
}

flist_t *
flist_scan(
	char	*name)
{
	flist_t	*fl;
	flist_t	*lfl;
	flist_t	*nfl;
	int	num;
	ftok_t	*p;
	ftok_t	*v;
	char	*x;

	v = flist_split(name);
	if (!v)
		return NULL;
	p = v;
	fl = lfl = NULL;
	while (p->tokty != TT_END) {
		if (p->tokty != TT_NAME)
			goto bad;
		nfl = flist_make(p->tok);
		if (lfl)
			lfl->child = nfl;
		else
			fl = nfl;
		lfl = nfl;
		p++;
		if (p->tokty == TT_LB) {
			p++;
			if (p->tokty != TT_NUM)
				goto bad;
			num = (int)strtoul(p->tok, &x, 0);
			if (*x != '\0')
				goto bad;
			nfl->flags |= FL_OKLOW;
			nfl->low = num;
			p++;
			if (p->tokty == TT_DASH) {
				p++;
				if (p->tokty != TT_NUM)
					goto bad;
				num = (int)strtoul(p->tok, &x, 0);
				if (*x != '\0')
					goto bad;
				nfl->flags |= FL_OKHIGH;
				nfl->high = num;
				p++;
			}
			if (p->tokty != TT_RB)
				goto bad;
			p++;
		}
		if (p->tokty == TT_DOT) {
			p++;
			if (p->tokty == TT_END)
				goto bad;
		}
	}
	ftok_free(v);
	return fl;
bad:
	dbprintf(_("bad syntax in field name %s\n"), name);
	ftok_free(v);
	if (fl)
		flist_free(fl);
	return NULL;
}

static ftok_t *
flist_split(
	char		*s)
{
	char		*a;
	int		i;
	static char	*idchars;
	static char	*initidchar;
	int		l;
	int             tailskip = 0;
	static char	*numchars;
	static char     *xnumchars;  /* extended for hex conversion */
	int		nv;
	static char	punctchars[] = "[-].";
	static tokty_t	puncttypes[] = { TT_LB, TT_DASH, TT_RB, TT_DOT };
	tokty_t		t;
	ftok_t		*v;

	if (idchars == NULL) {
		idchars = xmalloc(26 + 10 + 1 + 1);
		initidchar = xmalloc(26 + 1);
		numchars = xmalloc(10 + 1);
		xnumchars = xmalloc(12 + 1);
		for (i = 'a'; i <= 'z'; i++) {
			idchars[i - 'a'] = i;
			initidchar[i - 'a'] = i;
		}

		for (i = '0'; i <= '9'; i++) {
			idchars[26 + (i - '0')] = i;
			numchars[i - '0'] = i;
			xnumchars[i - '0'] = i;
		}
		idchars[26 + 10] = '_';
		idchars[26 + 10 + 1] = '\0';
		initidchar[26] = '\0';
		numchars[10] = '\0';
		xnumchars[10] = 'x';
		xnumchars[11] = 'X';
		xnumchars[12] = '\0';
	}
	nv = 0;
	v = xmalloc(sizeof(*v));
	v->tok = NULL;
	while (*s) {
		/* need to add string handling */
		if (*s == '\"') {
			s++; /* skip first quote */
			if ((a = strrchr(s, '\"')) == NULL) {
				dbprintf(_("missing closing quote %s\n"), s);
				ftok_free(v);
				return NULL;
			}
			tailskip = 1; /* skip remaing quote */
			l = (int)(a - s);
			t = TT_STRING;
		} else if (strchr(initidchar, *s)) {
			l = (int)strspn(s, idchars);
			t = TT_NAME;
		} else if (strchr(numchars, *s)) {
			l = (int)strspn(s, xnumchars);
			t = TT_NUM;
		} else if ((a = strchr(punctchars, *s))) {
			l = 1;
			t = puncttypes[a - punctchars];
		} else {
			dbprintf(_("bad character in field %s\n"), s);
			ftok_free(v);
			return NULL;
		}
		a = xmalloc(l + 1);
		strncpy(a, s, l);
		a[l] = '\0';
		v = xrealloc(v, (nv + 2) * sizeof(*v));
		v[nv].tok = a;
		v[nv].tokty = t;
		nv++;
		s += l + tailskip;
		tailskip = 0;
	}
	v[nv].tok = NULL;
	v[nv].tokty = TT_END;
	return v;
}

static void
ftok_free(
	ftok_t	*ft)
{
	ftok_t	*p;

	for (p = ft; p->tok; p++)
		xfree(p->tok);
	xfree(ft);
}
