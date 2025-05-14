/* Copyright (c) 2010
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"

#include "help.h"

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include "screen.h"

#include "misc.h"
#include "list_generic.h"
#include "process.h"

char version[60];		/* initialised by main() */

static void PadStr(char *, int, int, int);

/*
**   Here come the help page routines
*/

static void HelpProcess(char **, size_t *);
static void HelpAbort(void);
static void HelpRedisplayLine(int, int, int, int);
static void add_key_to_buf(char *, int);
static void AddAction(struct action *, int, int);
static int helppage(void);

struct helpdata {
	char *class;
	struct action *ktabp;
	int maxrow, grow, numcols, numrows, num_names;
	int numskip, numpages;
	int command_search, command_bindings;
	int refgrow, refcommand_search;
	int inter, mcom, mkey;
	int nact[RC_LAST + 1];
};

#define MAXKLEN 256

static const struct LayFuncs HelpLf = {
	HelpProcess,
	HelpAbort,
	HelpRedisplayLine,
	DefClearLine,
	DefResize,
	DefRestore,
	NULL
};

void display_help(char *class, struct action *ktabp)
{
	int i, n, key, mcom, mkey, l;
	struct helpdata *helpdata;
	int used[RC_LAST + 1];

	if (flayer->l_height < 6) {
		LMsg(0, "Window height too small for help page");
		return;
	}
	if (InitOverlayPage(sizeof(struct helpdata), &HelpLf, 0))
		return;

	helpdata = (struct helpdata *)flayer->l_data;
	helpdata->class = class;
	helpdata->ktabp = ktabp;
	helpdata->num_names = helpdata->command_bindings = 0;
	helpdata->command_search = 0;
	for (n = 0; n <= RC_LAST; n++)
		used[n] = 0;
	mcom = 0;
	mkey = 0;
	for (key = 0; key < 256 + KMAP_KEYS; key++) {
		n = ktabp[key].nr;
		if (n == RC_ILLEGAL)
			continue;
		if (ktabp[key].args == noargs) {
			used[n] += (key <= ' ' || key == 0x7f) ? 3 : (key > 0x7f) ? 5 : 2;
		} else
			helpdata->command_bindings++;
	}
	for (n = i = 0; n <= RC_LAST; n++)
		if (used[n]) {
			l = strlen(comms[n].name);
			if (l > mcom)
				mcom = l;
			if (used[n] > mkey)
				mkey = used[n];
			helpdata->nact[i++] = n;
		}
	helpdata->num_names = i;

	if (mkey > MAXKLEN)
		mkey = MAXKLEN;
	helpdata->numcols = flayer->l_width / (mcom + mkey + 1);
	if (helpdata->numcols == 0) {
		HelpAbort();
		LMsg(0, "Width too small");
		return;
	}
	helpdata->inter = (flayer->l_width - (mcom + mkey) * helpdata->numcols) / (helpdata->numcols + 1);
	if (helpdata->inter <= 0)
		helpdata->inter = 1;
	helpdata->mcom = mcom;
	helpdata->mkey = mkey;
	helpdata->numrows = (helpdata->num_names + helpdata->numcols - 1) / helpdata->numcols;
	helpdata->numskip = flayer->l_height - 5 - (2 + helpdata->numrows);
	while (helpdata->numskip < 0)
		helpdata->numskip += flayer->l_height - 5;
	helpdata->numskip %= flayer->l_height - 5;
	if (helpdata->numskip > flayer->l_height / 3 || helpdata->numskip > helpdata->command_bindings)
		helpdata->numskip = 1;
	helpdata->maxrow = 2 + helpdata->numrows + helpdata->numskip + helpdata->command_bindings;
	helpdata->grow = 0;

	helpdata->numpages = (helpdata->maxrow + flayer->l_height - 6) / (flayer->l_height - 5);
	flayer->l_x = 0;
	flayer->l_y = flayer->l_height - 1;
	helppage();
}

static void HelpProcess(char **ppbuf, size_t *plen)
{
	bool done = false;

	while (!done && *plen > 0) {
		switch (**ppbuf) {
		case ' ':
			if (helppage() == 0)
				break;
			/* FALLTHROUGH */
		case '\r':
		case '\n':
			done = true;
			break;
		default:
			break;
		}
		++*ppbuf;
		--*plen;
	}
	if (done)
		HelpAbort();
}

static void HelpAbort(void)
{
	LAY_CALL_UP(LRefreshAll(flayer, 0));
	ExitOverlayPage();
}

static int helppage(void)
{
	struct helpdata *helpdata;
	int col, crow, n, key, x;
	char buf[MAXKLEN], Esc_buf[5], cbuf[512];
	struct action *ktabp;

	helpdata = (struct helpdata *)flayer->l_data;

	ktabp = helpdata->ktabp;
	if (helpdata->grow >= helpdata->maxrow)
		return -1;
	helpdata->refgrow = helpdata->grow;
	helpdata->refcommand_search = helpdata->command_search;

	/* Clear the help screen */
	LClearAll(flayer, 0);

	sprintf(cbuf, "Screen key bindings, page %d of %d.", helpdata->grow / (flayer->l_height - 5) + 1,
		helpdata->numpages);
	centerline(cbuf, 0);
	crow = 2;

	*Esc_buf = '\0';
	*buf = '\0';
	/* XXX fix escape character */
	if (flayer->l_cvlist && flayer->l_cvlist->c_display) {
		add_key_to_buf(buf, flayer->l_cvlist->c_display->d_user->u_MetaEsc);
		add_key_to_buf(Esc_buf, flayer->l_cvlist->c_display->d_user->u_Esc);
	} else {
		strncpy(Esc_buf, "??", 5);
		strncpy(buf, "??", 256);
	}

	for (; crow < flayer->l_height - 3; crow++) {
		if (helpdata->grow < 1) {
			if (ktabp == ktab)
				sprintf(cbuf, "Command key:  %s   Literal %s:  %s", Esc_buf, Esc_buf, buf);
			else
				sprintf(cbuf, "Command class: '%.80s'", helpdata->class);
			centerline(cbuf, crow);
			helpdata->grow++;
		} else if (helpdata->grow >= 2 && helpdata->grow - 2 < helpdata->numrows) {
			x = 0;
			for (col = 0;
			     col < helpdata->numcols
			     && (n = helpdata->numrows * col + (helpdata->grow - 2)) < helpdata->num_names; col++) {
				x += helpdata->inter - !col;
				n = helpdata->nact[n];
				buf[0] = '\0';
				for (key = 0; key < 256 + KMAP_KEYS; key++)
					if (ktabp[key].nr == n && ktabp[key].args == noargs
					    && strlen(buf) < ARRAY_SIZE(buf) - 7) {
						strcat(buf, " ");
						add_key_to_buf(buf, key);
					}
				PadStr(comms[n].name, helpdata->mcom, x, crow);
				x += helpdata->mcom;
				PadStr(buf, helpdata->mkey, x, crow);
				x += helpdata->mkey;
			}
			helpdata->grow++;
		} else if (helpdata->grow - 2 - helpdata->numrows >= helpdata->numskip
			   && helpdata->grow - 2 - helpdata->numrows - helpdata->numskip < helpdata->command_bindings) {
			while ((n = ktabp[helpdata->command_search].nr) == RC_ILLEGAL
			       || ktabp[helpdata->command_search].args == noargs) {
				if (++helpdata->command_search >= 256 + KMAP_KEYS)
					return -1;
			}
			buf[0] = '\0';
			add_key_to_buf(buf, helpdata->command_search);
			PadStr(buf, 5, 0, crow);
			AddAction(&ktabp[helpdata->command_search++], 5, crow);
			helpdata->grow++;
		} else
			helpdata->grow++;
	}
	sprintf(cbuf, "[Press Space %s Return to end.]", helpdata->grow < helpdata->maxrow ? "for next page;" : "or");
	centerline(cbuf, flayer->l_height - 2);
	LaySetCursor();
	return 0;
}

static void AddAction(struct action *act, int x, int y)
{
	char buf[256];
	int del, l;
	char *bp, *cp, **pp;
	int *lp, ll;
	int fr;
	struct mchar mchar_dol;

	mchar_dol = mchar_blank;
	mchar_dol.image = '$';

	fr = flayer->l_width - 1 - x;
	if (fr <= 0)
		return;
	l = strlen(comms[act->nr].name);

	if (l + 1 > fr)
		l = fr - 1;
	PadStr(comms[act->nr].name, l, x, y);
	x += l;
	fr -= l + 1;
	LPutChar(flayer, fr ? &mchar_blank : &mchar_dol, x++, y);

	pp = act->args;
	lp = act->argl;
	while (pp && (cp = *pp) != NULL) {
		del = 0;
		bp = buf;
		ll = *lp++;
		if (!ll || (strchr(cp, ' ') != NULL)) {
			if (strchr(cp, '\'') != NULL)
				*bp++ = del = '"';
			else
				*bp++ = del = '\'';
		}
		while (ll-- && bp < buf + 250)
			bp += AddXChar(bp, *(unsigned char *)cp++);
		if (del)
			*bp++ = del;
		*bp = 0;
		if ((fr -= (bp - buf) + 1) < 0) {
			fr += bp - buf;
			if (fr > 0)
				PadStr(buf, fr, x, y);
			if (fr == 0)
				LPutChar(flayer, &mchar_dol, x, y);
			return;
		}
		PadStr(buf, strlen(buf), x, y);
		x += strlen(buf);
		pp++;
		if (*pp)
			LPutChar(flayer, fr ? &mchar_blank : &mchar_dol, x++, y);
	}
}

static void add_key_to_buf(char *buf, int key)
{
	buf += strlen(buf);
	if (key < 0)
		strncpy(buf, "unset", 6);
	else if (key == ' ')
		strncpy(buf, "sp", 3);
	else if (key >= 256) {
		key = key - 256 + T_CAPS;
		buf[0] = ':';
		buf[1] = term[key].tcname[0];
		buf[2] = term[key].tcname[1];
		buf[3] = ':';
		buf[4] = 0;
	} else
		buf[AddXChar(buf, key)] = 0;
}

static void HelpRedisplayLine(int y, int xs, int xe, int isblank)
{
	if (y < 0) {
		struct helpdata *helpdata;

		helpdata = (struct helpdata *)flayer->l_data;
		helpdata->grow = helpdata->refgrow;
		helpdata->command_search = helpdata->refcommand_search;
		helppage();
		return;
	}
	if (y != 0 && y != flayer->l_height - 1)
		return;
	if (!isblank)
		LClearArea(flayer, xs, y, xe, y, 0, 0);
}

/*
**
**    The bindkey help page
**
*/

static void BindkeyProcess(char **, size_t *);
static void BindkeyAbort(void);
static void BindkeyRedisplayLine(int, int, int, int);
static void bindkeypage(void);

struct bindkeydata {
	char *title;
	struct action *tab;
	int pos;
	int last;
	int page;
	int pages;
};

static const struct LayFuncs BindkeyLf = {
	BindkeyProcess,
	BindkeyAbort,
	BindkeyRedisplayLine,
	DefClearLine,
	DefResize,
	DefRestore,
	NULL
};

void display_bindkey(char *title, struct action *tab)
{
	struct bindkeydata *bindkeydata;
	int i, n;

	if (flayer->l_height < 6) {
		LMsg(0, "Window height too small for bindkey page");
		return;
	}
	if (InitOverlayPage(sizeof(struct bindkeydata), &BindkeyLf, 0))
		return;

	bindkeydata = (struct bindkeydata *)flayer->l_data;
	bindkeydata->title = title;
	bindkeydata->tab = tab;

	n = 0;
	for (i = 0; i < KMAP_KEYS + KMAP_AKEYS + kmap_extn; i++) {
		if (tab[i].nr != RC_ILLEGAL)
			n++;
	}
	bindkeydata->pos = 0;
	bindkeydata->page = 1;
	bindkeydata->pages = (n + flayer->l_height - 6) / (flayer->l_height - 5);
	if (bindkeydata->pages == 0)
		bindkeydata->pages = 1;
	flayer->l_x = 0;
	flayer->l_y = flayer->l_height - 1;
	bindkeypage();
}

static void BindkeyAbort(void)
{
	LAY_CALL_UP(LRefreshAll(flayer, 0));
	ExitOverlayPage();
}

static void bindkeypage(void)
{
	struct bindkeydata *bindkeydata;
	struct kmap_ext *kme;
	char tbuf[256];
	int del, i, y, sl;
	struct action *act;
	char *xch, *s, *p;

	bindkeydata = (struct bindkeydata *)flayer->l_data;

	LClearAll(flayer, 0);

	sprintf(tbuf, "%s key bindings, page %d of %d.", bindkeydata->title, bindkeydata->page, bindkeydata->pages);
	centerline(tbuf, 0);
	y = 2;
	for (i = bindkeydata->pos; i < KMAP_KEYS + KMAP_AKEYS + kmap_extn && y < flayer->l_height - 3; i++) {
		p = tbuf;
		xch = "   ";
		if (i < KMAP_KEYS) {
			act = &bindkeydata->tab[i];
			if (act->nr == RC_ILLEGAL)
				continue;
			del = *p++ = ':';
			s = term[i + T_CAPS].tcname;
			sl = s ? strlen(s) : 0;
		} else if (i < KMAP_KEYS + KMAP_AKEYS) {
			act = &bindkeydata->tab[i];
			if (act->nr == RC_ILLEGAL)
				continue;
			del = *p++ = ':';
			s = term[i + (T_CAPS - T_OCAPS + T_CURSOR)].tcname;
			sl = s ? strlen(s) : 0;
			xch = "[A]";
		} else {
			kme = kmap_exts + (i - (KMAP_KEYS + KMAP_AKEYS));
			del = 0;
			s = kme->str;
			sl = kme->fl & ~KMAP_NOTIMEOUT;
			if ((kme->fl & KMAP_NOTIMEOUT) != 0)
				xch = "[T]";
			act = bindkeydata->tab == dmtab ? &kme->dm : bindkeydata->tab == mmtab ? &kme->mm : &kme->um;
			if (act->nr == RC_ILLEGAL)
				continue;
		}
		while (sl-- > 0)
			p += AddXChar(p, *(unsigned char *)s++);
		if (del)
			*p++ = del;
		*p++ = ' ';
		while (p < tbuf + 15)
			*p++ = ' ';
		sprintf(p, "%s -> ", xch);
		p += 7;
		if (p - tbuf > flayer->l_width - 1) {
			tbuf[flayer->l_width - 2] = '$';
			tbuf[flayer->l_width - 1] = 0;
		}
		PadStr(tbuf, strlen(tbuf), 0, y);
		AddAction(act, strlen(tbuf), y);
		y++;
	}
	y++;
	bindkeydata->last = i;
	sprintf(tbuf, "[Press Space %s Return to end.]",
		bindkeydata->page < bindkeydata->pages ? "for next page;" : "or");
	centerline(tbuf, flayer->l_height - 2);
	LaySetCursor();
}

static void BindkeyProcess(char **ppbuf, size_t *plen)
{
	int done = 0;
	struct bindkeydata *bindkeydata;

	bindkeydata = (struct bindkeydata *)flayer->l_data;
	while (!done && *plen > 0) {
		switch (**ppbuf) {
		case ' ':
			if (bindkeydata->page < bindkeydata->pages) {
				bindkeydata->pos = bindkeydata->last;
				bindkeydata->page++;
				bindkeypage();
				break;
			}
			/* FALLTHROUGH */
		case '\r':
		case '\n':
			done = 1;
			break;
		default:
			break;
		}
		++*ppbuf;
		--*plen;
	}
	if (done)
		BindkeyAbort();
}

static void BindkeyRedisplayLine(int y, int xs, int xe, int isblank)
{
	if (y < 0) {
		bindkeypage();
		return;
	}
	if (y != 0 && y != flayer->l_height - 1)
		return;
	if (!isblank)
		LClearArea(flayer, xs, y, xe, y, 0, 0);
}

/*
**
**    The zmodem active page
**
*/

static void ZmodemRedisplayLine(int, int, int, int);
static int ZmodemResize(int, int);

static const struct LayFuncs ZmodemLf = {
	DefProcess,
	NULL,
	ZmodemRedisplayLine,
	DefClearLine,
	ZmodemResize,
	DefRestore,
	NULL
};

static int ZmodemResize(int wi, int he)
{
	flayer->l_width = wi;
	flayer->l_height = he;
	flayer->l_x = flayer->l_width > 32 ? 32 : 0;
	return 0;
}

static void ZmodemRedisplayLine(int y, int xs, int xe, int isblank)
{
	DefRedisplayLine(y, xs, xe, isblank);
	if (y == 0 && xs == 0)
		LPutStr(flayer, "Zmodem active on another display", flayer->l_width > 32 ? 32 : flayer->l_width,
			&mchar_blank, 0, 0);
}

void ZmodemPage(void)
{
	if (InitOverlayPage(1, &ZmodemLf, 1))
		return;
	LRefreshAll(flayer, 0);
	flayer->l_x = flayer->l_width > 32 ? 32 : 0;
	flayer->l_y = 0;
}

static void PadStr(char *str, int n, int x, int y)
{
	int l;

	l = strlen(str);
	if (l > n)
		l = n;
	LPutStr(flayer, str, l, &mchar_blank, x, y);
	if (l < n)
		LPutStr(flayer, (char *)blank, n - l, &mchar_blank, x + l, y);
}
