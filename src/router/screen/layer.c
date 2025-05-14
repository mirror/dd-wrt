/* Copyright (c) 2008, 2009
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

#include "layer.h"

#include "screen.h"
#include "encoding.h"
#include "mark.h"
#include "misc.h"
#include "tty.h"

/*
 * Layer subsystem.
 *
 * ...here is all the clipping code... beware!
 *
 * XXX: add some speedup code!
 *
 */

static struct mline *mlineoffset(const struct mline *ml, const int offset)
{
	static struct mline mml;

	if (ml == NULL)
		return NULL;
	mml.image = ml->image + offset;
	mml.attr = ml->attr + offset;
	mml.font = ml->font + offset;
	mml.colorbg = ml->colorbg + offset;
	mml.colorfg = ml->colorfg + offset;
	return &mml;
}

#define RECODE_MCHAR(mc) ((l->l_encoding == UTF8) != (D_encoding == UTF8) ? recode_mchar(mc, l->l_encoding, D_encoding) : (mc))
#define RECODE_MLINE(ml) ((l->l_encoding == UTF8) != (D_encoding == UTF8) ? recode_mline(ml, l->l_width, l->l_encoding, D_encoding) : (ml))

void LGotoPos(Layer *l, int x, int y)
{
	int x2, y2;

	if (l->l_pause.d)
		LayPauseUpdateRegion(l, x, x, y, y);

	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		x2 = x + cv->c_xoff;
		y2 = y + cv->c_yoff;
		if (x2 < cv->c_xs)
			x2 = cv->c_xs;
		if (y2 < cv->c_ys)
			y2 = cv->c_ys;
		if (x2 > cv->c_xe)
			x2 = cv->c_xe;
		if (y2 > cv->c_ye)
			y2 = cv->c_ye;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			if (x2 < vp->v_xs || x2 > vp->v_xe)
				continue;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			GotoPos(x2, y2);
			break;
		}
	}
}

void LScrollH(Layer *l, int n, int y, int xs, int xe, int bce, struct mline *ol)
{
	int y2, xs2, xe2;

	if (n == 0)
		return;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, xs, xe, y, y);
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			xs2 = xs + vp->v_xoff;
			xe2 = xe + vp->v_xoff;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > xe2)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			ScrollH(y2, xs2, xe2, n, bce, ol ? mlineoffset(ol, -vp->v_xoff) : NULL);
			if (xe2 - xs2 == xe - xs)
				continue;
			if (n > 0) {
				xs2 = xe2 + 1 - n;
				xe2 = xe + vp->v_xoff - n;
			} else {
				xe2 = xs2 - 1 - n;
				xs2 = xs + vp->v_xoff - n;
			}
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 <= xe2)
				RefreshArea(xs2, y2, xe2, y2, 1);
		}
	}
}

void LScrollV(Layer *l, int n, int ys, int ye, int bce)
{
	int ys2, ye2, xs2, xe2;
	if (n == 0)
		return;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, 0, l->l_width - 1, ys, ye);
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			xs2 = vp->v_xoff;
			xe2 = l->l_width - 1 + vp->v_xoff;
			ys2 = ys + vp->v_yoff;
			ye2 = ye + vp->v_yoff;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (ys2 < vp->v_ys)
				ys2 = vp->v_ys;
			if (ye2 > vp->v_ye)
				ye2 = vp->v_ye;
			if (ys2 > ye2 || xs2 > xe2)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			ScrollV(vp->v_xs, ys2, vp->v_xe, ye2, n, bce);
			if (ye2 - ys2 == ye - ys)
				continue;
			if (n > 0) {
				ys2 = ye2 + 1 - n;
				ye2 = ye + vp->v_yoff - n;
			} else {
				ye2 = ys2 - 1 - n;
				ys2 = ys + vp->v_yoff - n;
			}
			if (ys2 < vp->v_ys)
				ys2 = vp->v_ys;
			if (ye2 > vp->v_ye)
				ye2 = vp->v_ye;
			if (ys2 <= ye2)
				RefreshArea(xs2, ys2, xe2, ye2, 1);
		}
	}
}

void LInsChar(Layer *l, struct mchar *c, int x, int y, struct mline *ol)
{
	int xs2, xe2, y2, f;
	struct mchar *c2, cc;
	struct mline *rol;

	if (l->l_pause.d)
		LayPauseUpdateRegion(l, x, l->l_width - 1, y, y);
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			xs2 = x + vp->v_xoff;
			xe2 = l->l_width - 1 + vp->v_xoff;
			c2 = c;
			f = 0;
			if (xs2 < vp->v_xs) {
				xs2 = vp->v_xs;
				c2 = &mchar_blank;
				if (ol) {
					int i;
					i = xs2 - vp->v_xoff - 1;
					if (i >= 0 && i < l->l_width) {
						copy_mline2mchar(&cc, ol, i);
						c2 = &cc;
					}
				} else
					f = 1;
			}
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > xe2)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			rol = RECODE_MLINE(ol);
			InsChar(RECODE_MCHAR(c2), xs2, xe2, y2, mlineoffset(rol, -vp->v_xoff));
			if (f)
				RefreshArea(xs2, y2, xs2, y2, 1);
		}
	}
}

void LPutChar(Layer *l, struct mchar *c, int x, int y)
{
	int x2, y2;

	if (l->l_pause.d)
		LayPauseUpdateRegion(l, x, x + (c->mbcs ? 1 : 0)
				     , y, y);

	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		display = cv->c_display;
		if (D_blocked)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			x2 = x + vp->v_xoff;
			if (x2 < vp->v_xs || x2 > vp->v_xe)
				continue;
			PutChar(RECODE_MCHAR(c), x2, y2);
			break;
		}
	}
}

void LPutStr(Layer *l, char *s, int n, struct mchar *r, int x, int y)
{
	char *s2;
	int xs2, xe2, y2;

	if (x + n > l->l_width)
		n = l->l_width - x;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, x, x + n - 1, y, y);

	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			xs2 = x + vp->v_xoff;
			xe2 = xs2 + n - 1;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > xe2)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			GotoPos(xs2, y2);
			SetRendition(r);
			s2 = s + xs2 - x - vp->v_xoff;
			if (D_encoding == UTF8 && l->l_encoding != UTF8 && (r->font || l->l_encoding)) {
				struct mchar mc;
				mc = *r;
				while (xs2 <= xe2) {
					mc.image = *s2++;
					PutChar(RECODE_MCHAR(&mc), xs2++, y2);
				}
				continue;
			}
			while (xs2++ <= xe2)
				PUTCHARLP(*s2++);
		}
	}
}

void LPutWinMsg(Layer *l, char *s, int n, struct mchar *r, int x, int y)
{
	int xs2, xe2, y2, len, len2;
	struct mchar or;

	if (x + n > l->l_width)
		n = l->l_width - x;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, x, x + n - 1, y, y);
	len = strlen(s);
	if (len > n)
		len = n;
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			xs2 = x + vp->v_xoff;
			xe2 = xs2 + n - 1;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > xe2)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			GotoPos(xs2, y2);
			SetRendition(r);
			len2 = xe2 - (x + vp->v_xoff) + 1;
			if (len2 > len)
				len2 = len;
			PutWinMsg(s, xs2 - x - vp->v_xoff, len2);
			xs2 = x + vp->v_xoff + len2;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			or = D_rend;
			GotoPos(xs2, y2);
			SetRendition(&or);
			while (xs2++ <= xe2)
				PUTCHARLP(' ');
		}
	}
}

void LClearLine(Layer *l, int y, int xs, int xe, int bce, struct mline *ol)
{
	int y2, xs2, xe2;

	/* check for magic margin condition */
	if (xs >= l->l_width)
		xs = l->l_width - 1;
	if (xe >= l->l_width)
		xe = l->l_width - 1;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, xs, xe, y, y);
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			xs2 = xs + vp->v_xoff;
			xe2 = xe + vp->v_xoff;
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > xe2)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			ClearLine(ol ? mlineoffset(RECODE_MLINE(ol), -vp->v_xoff) : NULL, y2, xs2, xe2,
				  bce);
		}
	}
}

void LClearArea(Layer *l, int xs, int ys, int xe, int ye, int bce, int uself)
{
	int xs2, ys2, xe2, ye2;
	/* Check for zero-height window */
	if (ys < 0 || ye < ys)
		return;

	/* check for magic margin condition */
	if (xs >= l->l_width)
		xs = l->l_width - 1;
	if (xe >= l->l_width)
		xe = l->l_width - 1;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, xs, xe, ys, ye);
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		display = cv->c_display;
		if (D_blocked)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			xs2 = xs + vp->v_xoff;
			xe2 = xe + vp->v_xoff;
			ys2 = ys + vp->v_yoff;
			ye2 = ye + vp->v_yoff;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > vp->v_xe)
				ys2++;
			if (xe2 < vp->v_xs)
				ye2--;
			if (ys2 < vp->v_ys)
				ys2 = vp->v_ys;
			if (ye2 > vp->v_ye)
				ye2 = vp->v_ye;
			if (ys2 > ye2)
				continue;
			if (xs == 0 || ys2 != ys + vp->v_yoff)
				xs2 = vp->v_xs;
			if (xe == l->l_width - 1 || ye2 != ye + vp->v_yoff)
				xe2 = vp->v_xe;
			display = cv->c_display;
			ClearArea(xs2, ys2, vp->v_xs, vp->v_xe, xe2, ye2, bce, uself);
			if (xe == l->l_width - 1 && xe2 > vp->v_xoff + xe) {
				SetRendition(&mchar_blank);
				for (int y = ys2; y <= ye2; y++) {
					GotoPos(xe + vp->v_xoff + 1, y);
					PUTCHARLP('|');
				}
			}
		}
	}
}

void LCDisplayLine(Layer *l, struct mline *ml, int y, int xs, int xe, int isblank)
{
	int xs2, xe2, y2;
	if (l->l_pause.d)
		LayPauseUpdateRegion(l, xs, xe, y, y);
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		if (l->l_pause.d && cv->c_slorient)
			continue;
		display = cv->c_display;
		if (D_blocked)
			continue;
		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			xs2 = xs + vp->v_xoff;
			xe2 = xe + vp->v_xoff;
			y2 = y + vp->v_yoff;
			if (y2 < vp->v_ys || y2 > vp->v_ye)
				continue;
			if (xs2 < vp->v_xs)
				xs2 = vp->v_xs;
			if (xe2 > vp->v_xe)
				xe2 = vp->v_xe;
			if (xs2 > xe2)
				continue;
			display = cv->c_display;
			DisplayLine(isblank ? &mline_blank : &mline_null, mlineoffset(RECODE_MLINE(ml), -vp->v_xoff),
				    y2, xs2, xe2);
		}
	}
}

void LCDisplayLineWrap(Layer *l, struct mline *ml, int y, int from, int to, int isblank)
{
	struct mchar nc;
	copy_mline2mchar(&nc, ml, 0);
	if (dw_left(ml, 0, l->l_encoding)) {
		nc.mbcs = ml->image[1];
		from++;
	}
	LWrapChar(l, &nc, y - 1, -1, -1, false);
	from++;
	if (from <= to)
		LCDisplayLine(l, ml, y, from, to, isblank);
}

void LSetRendition(Layer *l, struct mchar *r)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		SetRendition(r);
	}
}

void LWrapChar(Layer *l, struct mchar *c, int y, int top, int bot, bool ins)
{
	Canvas *cvlist, *cvlnext;
	Viewport *vp, *evp, **vpp;
	int yy, y2, yy2, top2, bot2;
	int bce;

	if (l->l_pause.d)
		/* XXX: 'y'? */
		LayPauseUpdateRegion(l, 0, l->l_width - 1, top, bot);

	bce = c->colorbg;
	if (y != bot) {
		/* simple case: no scrolling */

		/* cursor after wrapping */
		yy = y == l->l_height - 1 ? y : y + 1;

		for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
			if (l->l_pause.d && cv->c_slorient)
				continue;
			y2 = 0;	/* gcc -Wall */
			display = cv->c_display;
			if (D_blocked)
				continue;
			/* find the viewport of the wrapped character */
			for (vp = cv->c_vplist; vp; vp = vp->v_next) {
				y2 = y + vp->v_yoff;
				yy2 = yy + vp->v_yoff;
				if (yy2 >= vp->v_ys && yy2 <= vp->v_ye && vp->v_xoff >= vp->v_xs
				    && vp->v_xoff <= vp->v_xe)
					break;
			}
			if (vp == NULL)
				continue;	/* nothing to do, character not visible */
			/* find the viewport of the character at the end of the line */
			for (evp = cv->c_vplist; evp; evp = evp->v_next)
				if (y2 >= evp->v_ys && y2 <= evp->v_ye
				    && evp->v_xoff + l->l_width - 1 >= evp->v_xs
				    && evp->v_xoff + l->l_width - 1 <= evp->v_xe)
					break;	/* gotcha! */
			if (evp == NULL || (ins && vp->v_xoff + l->l_width - 1 > vp->v_ye)) {
				/* no wrapping possible */
				cvlist = l->l_cvlist;
				cvlnext = cv->c_lnext;
				l->l_cvlist = cv;
				cv->c_lnext = NULL;
				if (ins)
					LInsChar(l, c, 0, yy, NULL);
				else
					LPutChar(l, c, 0, yy);
				l->l_cvlist = cvlist;
				cv->c_lnext = cvlnext;
			} else {
				WrapChar(RECODE_MCHAR(c), vp->v_xoff + l->l_width, y2, vp->v_xoff, -1,
					 vp->v_xoff + l->l_width - 1, -1, ins);
			}
		}
	} else {
		/* hard case: scroll up */

		for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
			if (l->l_pause.d && cv->c_slorient)
				continue;
			display = cv->c_display;
			if (D_blocked)
				continue;
			/* search for wrap viewport */
			for (vpp = &cv->c_vplist; (vp = *vpp); vpp = &vp->v_next) {
				yy2 = bot + vp->v_yoff;
				if (yy2 >= vp->v_ys && yy2 <= vp->v_ye && vp->v_xoff >= vp->v_xs
				    && vp->v_xoff + l->l_width - 1 <= vp->v_xe)
					break;
			}

			if (vp) {
				/* great, can use Wrap on the vp */
				/* temporarily remove vp from cvlist */
				*vpp = vp->v_next;
			}
			if (cv->c_vplist) {
				/* scroll all viewports != vp */
				cvlist = l->l_cvlist;
				cvlnext = cv->c_lnext;
				l->l_cvlist = cv;
				cv->c_lnext = NULL;
				LScrollV(l, 1, top, bot, bce);
				if (!vp) {
					if (ins)
						LInsChar(l, c, 0, bot, NULL);
					else
						LPutChar(l, c, 0, bot);
				}
				l->l_cvlist = cvlist;
				cv->c_lnext = cvlnext;
			}
			if (vp) {
				/* add vp back to cvlist */
				*vpp = vp;
				top2 = top + vp->v_yoff;
				bot2 = bot + vp->v_yoff;
				if (top2 < vp->v_ys)
					top2 = vp->v_ys;
				WrapChar(RECODE_MCHAR(c), vp->v_xoff + l->l_width, bot2, vp->v_xoff, top2,
					 vp->v_xoff + l->l_width - 1, bot2, ins);
			}
		}
	}
}

void LCursorVisibility(Layer *l, int vis)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		CursorVisibility(vis);
	}
}

void LSetFlow(Layer *l, bool flow)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (cv != D_forecv)
			continue;
		SetFlow(flow);
	}
}

void LKeypadMode(Layer *l, int on)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		KeypadMode(on);
	}
}

void LCursorkeysMode(Layer *l, int on)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		CursorkeysMode(on);
	}
}

void LMouseMode(Layer *l, int on)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		MouseMode(on);
	}
}

void LExtMouseMode(Layer *l, int on)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		ExtMouseMode(on);
	}
}

void LBracketedPasteMode(Layer *l, bool on)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		BracketedPasteMode(on);
	}
}

void LCursorStyle(Layer *l, int style)
{
	for (Canvas *cv = l->l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_blocked)
			continue;
		if (cv != D_forecv)
			continue;
		CursorStyle(style);
	}
}

void LClearAll(Layer *l, int uself)
{
	LClearArea(l, 0, 0, l->l_width - 1, l->l_height - 1, 0, uself);
}

void LRefreshAll(Layer *l, int isblank)
{
	Layer *oldflayer;

	oldflayer = flayer;
	flayer = l;
	if (!isblank)
		LClearArea(l, 0, 0, l->l_width - 1, l->l_height - 1, 0, 0);
	/* signal full refresh */
	LayRedisplayLine(-1, -1, -1, 1);
	for (int y = 0; y < l->l_height; y++)
		LayRedisplayLine(y, 0, l->l_width - 1, 1);
	flayer = oldflayer;
}

void LMsg(int err, const char *fmt, ...)
{
	va_list ap;
	char buf[MAXPATHLEN * 2];
	char *p = buf;
	Canvas *cv;

	va_start(ap, fmt);
	(void)vsnprintf(p, ARRAY_SIZE(buf) - 100, fmt, ap);
	va_end(ap);
	if (err) {
		p += strlen(p);
		*p++ = ':';
		*p++ = ' ';
		strncpy(p, strerror(err), buf + ARRAY_SIZE(buf) - p - 1);
		buf[ARRAY_SIZE(buf) - 1] = 0;
	}
	for (display = displays; display; display = display->d_next) {
		for (cv = D_cvlist; cv; cv = cv->c_next)
			if (cv->c_layer == flayer)
				break;
		if (cv == NULL)
			continue;
		MakeStatus(buf);
	}
}

/*******************************************************************/
/*******************************************************************/

/*
 *  Layer creation / removal
 */

void KillLayerChain(Layer *lay)
{
	Canvas *cv, *ncv;
	Layer *l, *oldflayer;

	oldflayer = flayer;
	for (l = lay; l; l = l->l_next) {
		if (l->l_layfn == &WinLf || l->l_layfn == &BlankLf)
			break;
		if (oldflayer == l)
			oldflayer = NULL;
		for (cv = l->l_cvlist; cv; cv = ncv) {
			ncv = cv->c_lnext;
			cv->c_layer = NULL;
			cv->c_lnext = NULL;
		}
	}
	flayer = lay;
	while (flayer != l)
		ExitOverlayPage();
	flayer = oldflayer;
}

int InitOverlayPage(int datasize, const struct LayFuncs *lf, int block)
{
	char *data;
	Layer *newlay;
	Canvas *cv, *cvp, **cvpp;
	Window *win;

	cv = NULL;
	if (display && D_forecv->c_layer == flayer)
		cv = D_forecv;	/* work only on this cv! */

	if ((newlay = calloc(1, sizeof(Layer))) == NULL) {
		Msg(0, "No memory for layer struct");
		return -1;
	}
	data = NULL;
	if (datasize) {
		if ((data = calloc(1, datasize)) == NULL) {
			free((char *)newlay);
			Msg(0, "No memory for layer data");
			return -1;
		}
	}

	win = Layer2Window(flayer);

	if (win && (win->w_savelayer == flayer || (block && flayer->l_next == NULL))) {
		if (win->w_savelayer && win->w_savelayer != flayer && win->w_savelayer->l_cvlist == NULL)
			KillLayerChain(win->w_savelayer);
		win->w_savelayer = newlay;
	}

	if (cv && flayer->l_next == NULL && !block) {
		Display *olddisplay = display;
		display = cv->c_display;
		RemoveStatus();
		display = olddisplay;

		/* new branch -> just get canvas vps */
		for (cvpp = &flayer->l_cvlist; (cvp = *cvpp); cvpp = &cvp->c_lnext)
			if (cvp == cv)
				break;
		*cvpp = cv->c_lnext;
		newlay->l_cvlist = cv;
		cv->c_lnext = NULL;
		cv->c_layer = newlay;
	} else {
		LAY_DISPLAYS(flayer, RemoveStatus());
		if (block && flayer->l_layfn == &WinLf) {
			if (win)
				win->w_blocked++;
			newlay->l_blocking = 1;
		}
		/* change all canvases */
		newlay->l_cvlist = flayer->l_cvlist;
		for (cvp = newlay->l_cvlist; cvp; cvp = cvp->c_lnext)
			cvp->c_layer = newlay;
		flayer->l_cvlist = NULL;
	}
	newlay->l_width = flayer->l_width;
	newlay->l_height = flayer->l_height;
	newlay->l_encoding = 0;
	newlay->l_layfn = lf;
	newlay->l_data = data;
	newlay->l_next = flayer;
	newlay->l_bottom = flayer->l_bottom;
	flayer = newlay;
	LayRestore();
	return 0;
}

void ExitOverlayPage(void)
{
	Layer *oldlay;
	Window *p;
	int doredisplay = 0;
	Canvas *cv, *ocv;
	Layout *lay;

	oldlay = flayer;
	if (oldlay->l_data) {
		if (oldlay->l_layfn->lf_LayFree)
			LayFree(oldlay->l_data);
		free(oldlay->l_data);
	}

	p = Layer2Window(flayer);

	flayer = oldlay->l_next;
	if (flayer->l_layfn == &WinLf) {
		if (oldlay->l_blocking) {
			p->w_blocked--;
		}
		/* don't warp dead layers: check cvlist */
		if (p->w_blocked && p->w_savelayer && p->w_savelayer != flayer && oldlay->l_cvlist) {
			/* warp ourself into savelayer */
			flayer = p->w_savelayer;
			doredisplay = 1;
		}
	}
	if (p && p->w_savelayer == oldlay)
		p->w_savelayer = flayer;
	if (p && oldlay == p->w_paster.pa_pastelayer)
		p->w_paster.pa_pastelayer = NULL;

	for (lay = layouts; lay; lay = lay->lay_next)
		for (cv = lay->lay_cvlist; cv; cv = cv->c_next)
			if (cv->c_layer == oldlay)
				cv->c_layer = flayer;

	/* add all canvases back into next layer's canvas list */
	for (ocv = NULL, cv = oldlay->l_cvlist; cv; cv = cv->c_lnext) {
		cv->c_layer = flayer;
		ocv = cv;
	}
	if (ocv) {
		cv = flayer->l_cvlist;
		ocv->c_lnext = NULL;
		flayer->l_cvlist = oldlay->l_cvlist;
		/* redisplay only the warped cvs */
		if (doredisplay)
			LRefreshAll(flayer, 0);
		ocv->c_lnext = cv;
	}
	oldlay->l_cvlist = NULL;
	LayerCleanupMemory(oldlay);
	free((char *)oldlay);
	LayRestore();
	LaySetCursor();
}

int LayProcessMouse(Layer *l, unsigned char ch)
{
	/* XXX: Make sure the layer accepts mouse events */
	size_t len;

	if (l->l_mouseevent.len >= ARRAY_SIZE(l->l_mouseevent.buffer))
		return -1;

	len = l->l_mouseevent.len++;
	l->l_mouseevent.buffer[len] = (len > 0 ? ch - 33 : ch);
	return (l->l_mouseevent.len == ARRAY_SIZE(l->l_mouseevent.buffer));
}

void LayProcessMouseSwitch(Layer *l, bool start)
{
	if ((l->l_mouseevent.start = start)) {
		l->l_mouseevent.len = 0;
	}
}

void LayPause(Layer *layer, bool pause)
{
	Window *win;

	if (layer->l_pause.d == pause)
		return;

	if ((layer->l_pause.d = pause)) {
		/* Start pausing */
		layer->l_pause.top = layer->l_pause.bottom = -1;
		return;
	}

	/* Unpause. So refresh the regions in the displays! */
	if (layer->l_pause.top == -1 && layer->l_pause.bottom == -1)
		return;

	if (layer->l_layfn == &WinLf)	/* Currently, this will always be the case! */
		win = layer->l_data;
	else
		win = NULL;

	for (Canvas *cv = layer->l_cvlist; cv; cv = cv->c_lnext) {
		if (!cv->c_slorient)
			continue;	/* Wasn't split, so already updated. */

		display = cv->c_display;

		for (Viewport *vp = cv->c_vplist; vp; vp = vp->v_next) {
			for (int line = layer->l_pause.top; line <= layer->l_pause.bottom; line++) {
				int xs, xe;

				if (line + vp->v_yoff >= vp->v_ys && line + vp->v_yoff <= vp->v_ye &&
				    ((xs = layer->l_pause.left[line]) >= 0) &&
				    ((xe = layer->l_pause.right[line]) >= 0)) {
					xs += vp->v_xoff;
					xe += vp->v_xoff;

					if (xs < vp->v_xs)
						xs = vp->v_xs;
					if (xe > vp->v_xe)
						xe = vp->v_xe;

					if (layer->l_encoding == UTF8 && xe < vp->v_xe && win) {
						struct mline *ml = win->w_mlines + line;
						if (dw_left(ml, xe, UTF8))
							xe++;
					}

					if (xs <= xe)
						RefreshLine(line + vp->v_yoff, xs, xe, 0);
				}
			}
		}

		if (cv == D_forecv) {
			int cx = layer->l_x + cv->c_xoff;
			int cy = layer->l_y + cv->c_yoff;

			if (cx < cv->c_xs)
				cx = cv->c_xs;
			if (cy < cv->c_ys)
				cy = cv->c_ys;
			if (cx > cv->c_xe)
				cx = cv->c_xe;
			if (cy > cv->c_ye)
				cy = cv->c_ye;

			GotoPos(cx, cy);
		}
	}

	for (int line = layer->l_pause.top; line <= layer->l_pause.bottom; line++)
		layer->l_pause.left[line] = layer->l_pause.right[line] = -1;
}

void LayPauseUpdateRegion(Layer *layer, int xs, int xe, int ys, int ye)
{
	if (!layer->l_pause.d)
		return;
	if (ys < 0)
		ys = 0;
	if (ye >= layer->l_height)
		ye = layer->l_height - 1;
	if (xe >= layer->l_width)
		xe = layer->l_width - 1;

	if (layer->l_pause.top == -1 || layer->l_pause.top > ys)
		layer->l_pause.top = ys;
	if (layer->l_pause.bottom < ye) {
		layer->l_pause.bottom = ye;
		if (layer->l_pause.lines <= ye) {
			int o = layer->l_pause.lines;
			layer->l_pause.lines = ye + 32;
			layer->l_pause.left = realloc(layer->l_pause.left, sizeof(int) * layer->l_pause.lines);
			layer->l_pause.right = realloc(layer->l_pause.right, sizeof(int) * layer->l_pause.lines);
			while (o < layer->l_pause.lines) {
				layer->l_pause.left[o] = layer->l_pause.right[o] = -1;
				o++;
			}
		}
	}

	while (ys <= ye) {
		if (layer->l_pause.left[ys] == -1 || layer->l_pause.left[ys] > xs)
			layer->l_pause.left[ys] = xs;
		if (layer->l_pause.right[ys] < xe)
			layer->l_pause.right[ys] = xe;
		ys++;
	}
}

void LayerCleanupMemory(Layer *layer)
{
	if (layer->l_pause.left)
		free(layer->l_pause.left);
	if (layer->l_pause.right)
		free(layer->l_pause.right);
}
