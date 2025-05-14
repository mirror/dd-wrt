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

#include "resize.h"

#include <sys/types.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include "screen.h"

#include "process.h"
#include "telnet.h"

/* maximum window width */
#define MAXWIDTH 1000

static void CheckMaxSize(int);
static void FreeMline(struct mline *);
static int AllocMline(struct mline *ml, int);
static void MakeBlankLine(uint32_t *, int);
static void kaablamm(void);
static int BcopyMline(struct mline *, int, struct mline *, int, int, int);
static void SwapAltScreen(Window *);

struct winsize glwz;

static struct mline mline_zero = {
	.image   = NULL,
	.attr    = NULL,
	.font    = NULL,
	.colorbg = NULL,
	.colorfg = NULL
};

/*
 * ChangeFlag:   0: try to modify no window
 *               1: modify fore (and try to modify no other) + redisplay
 *               2: modify all windows
 *
 * Note: Activate() is only called if change_flag == 1
 *       i.e. on a WINCH event
 */

void CheckScreenSize(int change_flag)
{
	int wi, he;

	if (display == NULL) {
		return;
	}

	if (ioctl(D_userfd, TIOCGWINSZ, (char *)&glwz) != 0) {
		wi = D_CO;
		he = D_LI;
	} else {
		wi = glwz.ws_col;
		he = glwz.ws_row;
		if (wi == 0)
			wi = D_CO;
		if (he == 0)
			he = D_LI;
	}

	if (D_width == wi && D_height == he) {
		return;
	}
	KillBlanker();
	ResetIdle();
	ChangeScreenSize(wi, he, change_flag);
}

void ChangeScreenSize(int wi, int he, int change_fore)
{
	Window *p;
	Canvas *cv;
	int wwi;

	cv = &D_canvas;
	cv->c_xe = wi - 1;
	cv->c_ys = ((cv->c_slperp && cv->c_slperp->c_slnext) || captionalways) * captiontop + (D_has_hstatus == HSTATUS_FIRSTLINE);
	cv->c_ye = he - 1 - ((cv->c_slperp && cv->c_slperp->c_slnext)
			     || captionalways) * !captiontop - (D_has_hstatus == HSTATUS_LASTLINE);
	cv->c_blank.l_height = cv->c_ye - cv->c_ys + 1;
	if (cv->c_slperp) {
		ResizeCanvas(cv);
		RecreateCanvasChain();
		RethinkDisplayViewports();
	}
	if (D_forecv == NULL)
		D_forecv = D_cvlist;
	if (D_forecv)
		D_fore = Layer2Window(D_forecv->c_layer);

	D_width = wi;
	D_height = he;

	CheckMaxSize(wi);
	if (D_CWS) {
		D_defwidth = D_CO;
		D_defheight = D_LI;
	} else {
		if (D_CZ0 && (wi == Z0width || wi == Z1width) && (D_CO == Z0width || D_CO == Z1width))
			D_defwidth = D_CO;
		else
			D_defwidth = wi;
		D_defheight = he;
	}
	if (change_fore)
		ResizeLayersToCanvases();
	if (change_fore == 2 && D_CWS == NULL && displays->d_next == NULL) {
		/* adapt all windows  -  to be removed ? */
		for (p = mru_window; p; p = p->w_prev_mru) {
			wwi = wi;
			if (p->w_savelayer && p->w_savelayer->l_cvlist == NULL)
				ResizeLayer(p->w_savelayer, wwi, he, NULL);
		}
	}
}

void ResizeLayersToCanvases(void)
{
	Canvas *cv;
	Layer *l;
	int lx, ly;

	D_kaablamm = 0;
	for (cv = D_cvlist; cv; cv = cv->c_next) {
		l = cv->c_layer;
		if (l == NULL)
			continue;
		if (l->l_width == cv->c_xe - cv->c_xs + 1 && l->l_height == cv->c_ye - cv->c_ys + 1) {
			continue;
		}
		if (MayResizeLayer(l)) {
			ResizeLayer(l, cv->c_xe - cv->c_xs + 1, cv->c_ye - cv->c_ys + 1, display);
		}

		/* normalize window, see screen.c */
		lx = cv->c_layer->l_x;
		ly = cv->c_layer->l_y;
		if (ly + cv->c_yoff < cv->c_ys) {
			cv->c_yoff = cv->c_ys - ly;
			RethinkViewportOffsets(cv);
		} else if (ly + cv->c_yoff > cv->c_ye) {
			cv->c_yoff = cv->c_ye - ly;
			RethinkViewportOffsets(cv);
		}
		if (lx + cv->c_xoff < cv->c_xs) {
			int n = cv->c_xs - (lx + cv->c_xoff);
			if (n < (cv->c_xe - cv->c_xs + 1) / 2)
				n = (cv->c_xe - cv->c_xs + 1) / 2;
			if (cv->c_xoff + n > cv->c_xs)
				n = cv->c_xs - cv->c_xoff;
			cv->c_xoff += n;
			RethinkViewportOffsets(cv);
		} else if (lx + cv->c_xoff > cv->c_xe) {
			int n = lx + cv->c_xoff - cv->c_xe;
			if (n < (cv->c_xe - cv->c_xs + 1) / 2)
				n = (cv->c_xe - cv->c_xs + 1) / 2;
			if (cv->c_xoff - n + cv->c_layer->l_width - 1 < cv->c_xe)
				n = cv->c_xoff + cv->c_layer->l_width - 1 - cv->c_xe;
			cv->c_xoff -= n;
			RethinkViewportOffsets(cv);
		}
	}
	Redisplay(0);
	if (D_kaablamm) {
		kaablamm();
		D_kaablamm = 0;
	}
}

int MayResizeLayer(Layer *l)
{
	int cvs = 0;
	for (; l; l = l->l_next) {
		if (l->l_cvlist)
			if (++cvs > 1 || l->l_cvlist->c_lnext) {
				return 0;
			}
	}
	return 1;
}

/*
 *  Easy implementation: rely on the fact that the only layers
 *  supporting resize are Win and Blank. So just kill all overlays.
 *
 *  This is a lot harder if done the right way...
 */

static void kaablamm(void)
{
	Msg(0, "Aborted because of window size change.");
}

/* Kills non-resizable layers. */
#define RESIZE_OR_KILL_LAYERS(l, wi, he)							\
do {												\
	Layer *_last = NULL;									\
	flayer = (l);										\
	while (flayer->l_next) {								\
		if (LayResize(wi, he) == 0) {							\
			_last = flayer;								\
			flayer = flayer->l_next;						\
		} else {									\
			Canvas *_cv;								\
			for (_cv = flayer->l_cvlist; _cv; _cv = _cv->c_lnext)			\
				_cv->c_display->d_kaablamm = 1;					\
			ExitOverlayPage();							\
			if (_last)								\
				_last->l_next = flayer;						\
		}										\
	}											\
	/* We assume that the bottom-most layer, i.e. when flayer->l_next == 0,			\
	 * is always resizable. Currently, WinLf and BlankLf can be the bottom-most layers.	\
	 */											\
	LayResize(wi, he);									\
} while (0)

void ResizeLayer(Layer *l, int wi, int he, Display *norefdisp)
{
	Window *p;
	Canvas *cv;
	Layer *oldflayer = flayer;
	Display *d, *olddisplay = display;

	if (l->l_width == wi && l->l_height == he)
		return;
	p = Layer2Window(l);

	/* If 'flayer' and 'l' are for the same window, then we will not
	 * restore 'flayer'. */
	if (oldflayer && (l == oldflayer || Layer2Window(oldflayer) == p))
		oldflayer = NULL;

	flayer = l;

	if (p) {
		/* It's a window layer. Kill the overlays on it in all displays. */
		for (d = displays; d; d = d->d_next)
			for (cv = d->d_cvlist; cv; cv = cv->c_next) {
				if (p == Layer2Window(cv->c_layer)) {
					/* Canvas 'cv' on display 'd' shows this window. Remove any non-resizable
					 * layers over it. */
					RESIZE_OR_KILL_LAYERS(cv->c_layer, wi, he);
				}
			}
	} else {
		/* It's a Blank layer. Just kill the non-resizable overlays over it. */
		RESIZE_OR_KILL_LAYERS(flayer, wi, he);
	}

	for (display = displays; display; display = display->d_next) {
		if (display == norefdisp)
			continue;
		for (cv = D_cvlist; cv; cv = cv->c_next)
			if (Layer2Window(cv->c_layer) == p) {
				CV_CALL(cv, LayRedisplayLine(-1, -1, -1, 0));
				RefreshArea(cv->c_xs, cv->c_ys, cv->c_xe, cv->c_ye, 0);
			}
		if (D_kaablamm) {
			kaablamm();
			D_kaablamm = 0;
		}
	}

	/* If we started resizing a non-flayer layer, then restore the flayer.
	 * Otherwise, flayer should already be updated to the topmost foreground layer. */
	if (oldflayer)
		flayer = oldflayer;
	display = olddisplay;
}

static void FreeMline(struct mline *ml)
{
	if (ml->image)
		free(ml->image);
	if (ml->attr && ml->attr != null)
		free(ml->attr);
	if (ml->font && ml->font != null)
		free(ml->font);
	if (ml->colorbg && ml->colorbg != null)
		free(ml->colorbg);
	if (ml->colorfg && ml->colorfg != null)
		free(ml->colorfg);
	*ml = mline_zero;
}

static int AllocMline(struct mline *ml, int w)
{
	ml->image = malloc(w * 4);
	ml->attr = null;
	ml->font = null;
	ml->colorbg = null;
	ml->colorfg = null;
	if (ml->image == NULL)
		return -1;
	return 0;
}

static int BcopyMline(struct mline *mlf, int xf, struct mline *mlt, int xt, int l, int w)
{
	int r = 0;

	memmove(mlt->image + xt, mlf->image + xf, l * 4);
	if (mlf->attr != null && mlt->attr == null) {
		if ((mlt->attr = calloc(w, 4)) == NULL)
			mlt->attr = null, r = -1;
	}
	if (mlt->attr != null)
		memmove(mlt->attr + xt, mlf->attr + xf, l * 4);
	if (mlf->font != null && mlt->font == null) {
		if ((mlt->font = calloc(w, 4)) == NULL)
			mlt->font = null, r = -1;
	}
	if (mlt->font != null)
		memmove(mlt->font + xt, mlf->font + xf, l * 4);
	if (mlf->colorbg != null && mlt->colorbg == null) {
		if ((mlt->colorbg = calloc(w, 4)) == NULL)
			mlt->colorbg = null, r = -1;
	}
	if (mlt->colorbg != null)
		memmove(mlt->colorbg + xt, mlf->colorbg + xf, l * 4);
	if (mlf->colorfg != null && mlt->colorfg == null) {
		if ((mlt->colorfg = calloc(w, 4)) == NULL)
			mlt->colorfg = null, r = -1;
	}
	if (mlt->colorfg != null)
		memmove(mlt->colorfg + xt, mlf->colorfg + xf, l * 4);
	return r;
}

static int maxwidth;

static void CheckMaxSize(int wi)
{
	uint32_t *oldnull = null;
	uint32_t *oldblank = blank;
	Window *p;
	int i;
	struct mline *ml;

	if (wi > MAXWIDTH)
		wi = MAXWIDTH;
	if (wi <= maxwidth)
		return;
	maxwidth = wi + 1;
	blank = xrealloc(blank, maxwidth * 4);
	null = xrealloc(null, maxwidth * 4);
	mline_old.image = xrealloc(mline_old.image, maxwidth * 4);
	mline_old.attr = xrealloc(mline_old.attr, maxwidth * 4);
	mline_old.font = xrealloc(mline_old.font, maxwidth * 4);
	mline_old.colorbg = xrealloc(mline_old.colorbg, maxwidth * 4);
	mline_old.colorfg = xrealloc(mline_old.colorfg, maxwidth * 4);
	if (!(blank && null && mline_old.image && mline_old.attr && mline_old.font && mline_old.colorbg && mline_old.colorfg))
		Panic(0, "%s", strnomem);

	MakeBlankLine(blank, maxwidth);
	memset(null, 0, maxwidth * 4);

	mline_blank.image = blank;
	mline_blank.attr = null;
	mline_null.image = null;
	mline_null.attr = null;
	mline_blank.font = null;
	mline_null.font = null;
	mline_blank.colorbg = null;
	mline_null.colorbg = null;
	mline_blank.colorfg = null;
	mline_null.colorfg = null;

#define RESET_AFC(x, bl)	\
do {				\
	if (x == old##bl)	\
		x = bl;		\
} while (0)

#define RESET_LINES(lines, count)		\
do {						\
	ml = lines;				\
	for (i = 0; i < count; i++, ml++) {	\
		RESET_AFC(ml->image, blank);	\
		RESET_AFC(ml->attr, null);	\
		RESET_AFC(ml->font, null);	\
		RESET_AFC(ml->colorbg, null);	\
		RESET_AFC(ml->colorfg, null);	\
	}					\
} while (0)

	/* We have to run through all windows to substitute
	 * the null and blank references.
	 */
	for (p = mru_window; p; p = p->w_prev_mru) {
		RESET_LINES(p->w_mlines, p->w_height);

		RESET_LINES(p->w_hlines, p->w_histheight);
		RESET_LINES(p->w_alt.hlines, p->w_alt.histheight);

		RESET_LINES(p->w_alt.mlines, p->w_alt.height);
	}
}

void *xrealloc(void *mem, size_t len)
{
	char *nmem;

	if (mem == NULL)
		return malloc(len);
	if ((nmem = realloc(mem, len)))
		return nmem;
	free(mem);
	return NULL;
}

static void MakeBlankLine(uint32_t *p, int n)
{
	while (n--)
		*p++ = ' ';
}

#define OLDWIN(y) ((y < p->w_histheight) \
        ? &p->w_hlines[(p->w_histidx + y) % p->w_histheight] \
        : &p->w_mlines[y - p->w_histheight])

#define NEWWIN(y) ((y < hi) ? &nhlines[y] : &nmlines[y - hi])

int ChangeWindowSize(Window *p, int wi, int he, int hi)
{
	struct mline *mlf = NULL, *mlt = NULL, *ml, *nmlines, *nhlines;
	int fy, ty, l, lx, lf, lt, yy, oty, addone;
	int ncx, ncy, naka, t;
	int y, shift;

	if (wi <= 0 || he <= 0)
		wi = he = hi = 0;

	if (p->w_type == W_TYPE_GROUP)
		return 0;

	if (wi > MAXWIDTH) {
		Msg(0, "Window width too large. Truncated to %d.", MAXWIDTH);
		wi = MAXWIDTH;
	}

	if (he > MAXWIDTH) {
		Msg(0, "Window height too large. Truncated to %d.", MAXWIDTH);
		he = MAXWIDTH;
	}

	if (p->w_width == wi && p->w_height == he && p->w_histheight == hi) {
		return 0;
	}

	CheckMaxSize(wi);

	fy = p->w_histheight + p->w_height - 1;
	ty = hi + he - 1;

	nmlines = nhlines = NULL;
	ncx = 0;
	ncy = 0;
	naka = 0;

	if (wi) {
		if (wi != p->w_width || he != p->w_height) {
			if ((nmlines = calloc(he, sizeof(struct mline))) == NULL) {
				KillWindow(p);
				Msg(0, "%s", strnomem);
				return -1;
			}
		} else {
			nmlines = p->w_mlines;
			fy -= he;
			ty -= he;
			ncx = p->w_x;
			ncy = p->w_y;
			naka = p->w_autoaka;
		}
	}
	if (hi) {
		if ((nhlines = calloc(hi, sizeof(struct mline))) == NULL) {
			Msg(0, "No memory for history buffer - turned off");
			hi = 0;
			ty = he - 1;
		}
	}

	/* special case: cursor is at magic margin position */
	addone = 0;
	if (p->w_width && p->w_x == p->w_width) {
		addone = 1;
		p->w_x--;
	}

	/* handle the cursor and autoaka lines now if the widths are equal */
	if (p->w_width == wi) {
		ncx = p->w_x + addone;
		ncy = p->w_y + he - p->w_height;
		/* never lose sight of the line with the cursor on it */
		shift = -ncy;
		for (yy = p->w_y + p->w_histheight - 1; yy >= 0 && ncy + shift < he; yy--) {
			ml = OLDWIN(yy);
			if (!ml->image)
				break;
			if (ml->image[p->w_width] == ' ')
				break;
			shift++;
		}
		if (shift < 0)
			shift = 0;
		ncy += shift;
		if (p->w_autoaka > 0) {
			naka = p->w_autoaka + he - p->w_height + shift;
			if (naka < 1 || naka > he)
				naka = 0;
		}
		while (shift-- > 0) {
			ml = OLDWIN(fy);
			FreeMline(ml);
			fy--;
		}
	}
	if (fy >= 0)
		mlf = OLDWIN(fy);
	if (ty >= 0)
		mlt = NEWWIN(ty);

	while (fy >= 0 && ty >= 0) {
		if (p->w_width == wi) {
			/* here is a simple shortcut: just copy over */
			*mlt = *mlf;
			*mlf = mline_zero;
			if (--fy >= 0)
				mlf = OLDWIN(fy);
			if (--ty >= 0)
				mlt = NEWWIN(ty);
			continue;
		}

		/* calculate lenght */
		for (l = p->w_width - 1; l > 0; l--)
			if (mlf->image[l] != ' ' || mlf->attr[l])
				break;
		if (fy == p->w_y + p->w_histheight && l < p->w_x)
			l = p->w_x;	/* cursor is non blank */
		l++;
		lf = l;

		/* add wrapped lines to length */
		for (yy = fy - 1; yy >= 0; yy--) {
			ml = OLDWIN(yy);
			if (ml->image[p->w_width] == ' ')
				break;
			l += p->w_width;
		}

		/* rewrap lines */
		lt = (l - 1) % wi + 1;	/* lf is set above */
		oty = ty;
		while (l > 0 && fy >= 0 && ty >= 0) {
			lx = lt > lf ? lf : lt;
			if (mlt->image == NULL) {
				if (AllocMline(mlt, wi + 1))
					goto nomem;
				MakeBlankLine(mlt->image + lt, wi - lt);
				mlt->image[wi] = ((oty == ty) ? ' ' : 0);
			}
			if (BcopyMline(mlf, lf - lx, mlt, lt - lx, lx, wi + 1))
				goto nomem;

			/* did we copy the cursor ? */
			if (fy == p->w_y + p->w_histheight && lf - lx <= p->w_x && lf > p->w_x) {
				ncx = p->w_x + lt - lf + addone;
				ncy = ty - hi;
				shift = wi ? -ncy + (l - lx) / wi : 0;
				if (ty + shift > hi + he - 1)
					shift = hi + he - 1 - ty;
				if (shift > 0) {
					for (y = hi + he - 1; y >= ty; y--) {
						mlt = NEWWIN(y);
						FreeMline(mlt);
						if (y - shift < ty)
							continue;
						ml = NEWWIN(y - shift);
						*mlt = *ml;
						*ml = mline_zero;
					}
					ncy += shift;
					ty += shift;
					mlt = NEWWIN(ty);
					if (naka > 0)
						naka = naka + shift > he ? 0 : naka + shift;
				}
			}
			/* did we copy autoaka line ? */
			if (p->w_autoaka > 0 && fy == p->w_autoaka - 1 + p->w_histheight && lf - lx <= 0)
				naka = ty - hi >= 0 ? 1 + ty - hi : 0;

			lf -= lx;
			lt -= lx;
			l -= lx;
			if (lf == 0) {
				FreeMline(mlf);
				lf = p->w_width;
				if (--fy >= 0)
					mlf = OLDWIN(fy);
			}
			if (lt == 0) {
				lt = wi;
				if (--ty >= 0)
					mlt = NEWWIN(ty);
			}
		}
	}
	while (fy >= 0) {
		FreeMline(mlf);
		if (--fy >= 0)
			mlf = OLDWIN(fy);
	}
	while (ty >= 0) {
		if (AllocMline(mlt, wi + 1))
			goto nomem;
		MakeBlankLine(mlt->image, wi + 1);
		if (--ty >= 0)
			mlt = NEWWIN(ty);
	}

	if (p->w_mlines && p->w_mlines != nmlines)
		free((char *)p->w_mlines);
	p->w_mlines = nmlines;
	if (p->w_hlines && p->w_hlines != nhlines)
		free((char *)p->w_hlines);
	p->w_hlines = nhlines;
	nmlines = nhlines = 0;

	/* change tabs */
	if (p->w_width != wi) {
		if (wi) {
			t = p->w_tabs ? p->w_width : 0;
			p->w_tabs = xrealloc(p->w_tabs, (wi + 1) * 4);
			if (p->w_tabs == NULL)
				goto nomem;
			for (; t < wi; t++)
				p->w_tabs[t] = t && !(t & 7) ? 1 : 0;
			p->w_tabs[wi] = 0;
		} else {
			if (p->w_tabs)
				free(p->w_tabs);
			p->w_tabs = NULL;
		}
	}

	/* Change w_saved.y - this is only an estimate... */
	p->w_saved.y += ncy - p->w_y;

	p->w_x = ncx;
	p->w_y = ncy;
	if (p->w_autoaka > 0)
		p->w_autoaka = naka;

	/* do sanity checks */
	if (p->w_x > wi)
		p->w_x = wi;
	if (p->w_y >= he)
		p->w_y = he - 1;
	if (p->w_saved.x > wi)
		p->w_saved.x = wi;
	if (p->w_saved.y >= he)
		p->w_saved.y = he - 1;
	if (p->w_saved.y < 0)
		p->w_saved.y = 0;
	if (p->w_alt.cursor.x > wi)
		p->w_alt.cursor.x = wi;
	if (p->w_alt.cursor.y >= he)
		p->w_alt.cursor.y = he - 1;
	if (p->w_alt.cursor.y < 0)
		p->w_alt.cursor.y = 0;

	/* reset scrolling region */
	p->w_top = 0;
	p->w_bot = he - 1;

	/* signal new size to window */
	if (wi && (p->w_width != wi || p->w_height != he)
	    && p->w_width != 0 && p->w_height != 0 && p->w_ptyfd >= 0 && p->w_pid) {
		glwz.ws_col = wi;
		glwz.ws_row = he;
		ioctl(p->w_ptyfd, TIOCSWINSZ, (char *)&glwz);
	}

	/* store new size */
	p->w_width = wi;
	p->w_height = he;
	if(p->w_scrollback_height > hi)
		p->w_scrollback_height = hi;
	p->w_histidx = 0;
	p->w_histheight = hi;

#ifdef ENABLE_TELNET
	if (p->w_type == W_TYPE_TELNET)
		TelWindowSize(p);
#endif

	return 0;

nomem:
	if (nmlines) {
		for (ty = he + hi - 1; ty >= 0; ty--) {
			mlt = NEWWIN(ty);
			FreeMline(mlt);
		}
		if (nmlines && p->w_mlines != nmlines)
			free((char *)nmlines);
		if (nhlines && p->w_hlines != nhlines)
			free((char *)nhlines);
	}
	KillWindow(p);
	Msg(0, "%s", strnomem);
	return -1;
}

void FreeAltScreen(Window *p)
{
	int i;

	if (p->w_alt.mlines) {
		for (i = 0; i < p->w_alt.height; i++)
			FreeMline(p->w_alt.mlines + i);
		free(p->w_alt.mlines);
	}
	p->w_alt.mlines = NULL;
	p->w_alt.width = 0;
	p->w_alt.height = 0;
	if (p->w_alt.hlines) {
		for (i = 0; i < p->w_alt.histheight; i++)
			FreeMline(p->w_alt.hlines + i);
		free(p->w_alt.hlines);
	}
	p->w_alt.hlines = NULL;
	p->w_alt.histidx = 0;
	p->w_alt.histheight = 0;
}

static void SwapAltScreen(Window *p)
{
	struct mline *ml;
	int t;

#define SWAP(item, t)			\
do {					\
	(t) = p->w_alt. item;		\
	p->w_alt. item = p->w_##item;	\
	p->w_##item = (t);		\
} while (0)

	SWAP(mlines, ml);
	SWAP(width, t);
	SWAP(height, t);

	SWAP(histheight, t);
	SWAP(hlines, ml);
	SWAP(histidx, t);
#undef SWAP
}

void EnterAltScreen(Window *p)
{
	if (!p->w_alt.on) {
		/* If not already using the alternate screen buffer, then create
		   a new one and swap it with the 'real' screen buffer. */
		FreeAltScreen(p);
		SwapAltScreen(p);
	} else {
		/* Already using the alternate buffer. Just clear the screen. To do so, it
		   is only necessary to reset the height(s) without resetting the width. */
		p->w_height = 0;
		p->w_histheight = 0;
	}
	ChangeWindowSize(p, p->w_alt.width, p->w_alt.height, p->w_alt.histheight);
	p->w_alt.on = 1;
}

void LeaveAltScreen(Window *p)
{
	if (!p->w_alt.on)
		return;
	SwapAltScreen(p);
	ChangeWindowSize(p, p->w_alt.width, p->w_alt.height, p->w_alt.histheight);
	FreeAltScreen(p);
	p->w_alt.on = 0;
}
