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

#include <sys/types.h>
#include <signal.h>
#ifndef sun
#include <sys/ioctl.h>
#endif

#ifdef ISC
# include <sys/tty.h>
# include <sys/sioctl.h>
# include <sys/pty.h>
#endif

#include "config.h"
#include "screen.h"
#include "extern.h"

/* maximum window width */
#define MAXWIDTH 1000

static void CheckMaxSize __P((int));
static void FreeMline  __P((struct mline *));
static int  AllocMline __P((struct mline *ml, int));
static void MakeBlankLine __P((unsigned char *, int));
static void kaablamm __P((void));
static int  BcopyMline __P((struct mline *, int, struct mline *, int, int, int));
static void SwapAltScreen __P((struct win *));

extern struct layer *flayer;
extern struct display *display, *displays;
extern unsigned char *blank, *null;
extern struct mline mline_blank, mline_null, mline_old;
extern struct win *windows;
extern const int Z0width, Z1width;
extern int captionalways;

#if defined(TIOCGWINSZ) || defined(TIOCSWINSZ)
struct winsize glwz;
#endif

static struct mline mline_zero = {
 (unsigned char *)0,
 (unsigned char *)0
#ifdef FONT
 ,(unsigned char *)0
#endif
#ifdef COLOR
 ,(unsigned char *)0
# ifdef COLORS256
 ,(unsigned char *)0
# endif
#endif
};

/*
 * ChangeFlag:   0: try to modify no window
 *               1: modify fore (and try to modify no other) + redisplay
 *               2: modify all windows
 *
 * Note: Activate() is only called if change_flag == 1
 *       i.e. on a WINCH event
 */

void
CheckScreenSize(change_flag)
int change_flag;
{
  int wi, he;

  if (display == 0)
    {
      debug("CheckScreenSize: No display -> no check.\n");
      return;
    }
#ifdef TIOCGWINSZ
  if (ioctl(D_userfd, TIOCGWINSZ, (char *)&glwz) != 0)
    {
      debug2("CheckScreenSize: ioctl(%d, TIOCGWINSZ) errno %d\n", D_userfd, errno);
      wi = D_CO;
      he = D_LI;
    }
  else
    {
      wi = glwz.ws_col;
      he = glwz.ws_row;
      if (wi == 0)
        wi = D_CO;
      if (he == 0)
        he = D_LI;
    }
#else
  wi = D_CO;
  he = D_LI;
#endif
  
  debug2("CheckScreenSize: screen is (%d,%d)\n", wi, he);

#if 0 /* XXX: Fixme */
  if (change_flag == 2)
    {
      debug("Trying to adapt all windows (-A)\n");
      for (p = windows; p; p = p->w_next)
	if (p->w_display == 0 || p->w_display == display)
          ChangeWindowSize(p, wi, he, p->w_histheight);
    }
#endif
  if (D_width == wi && D_height == he)
    {
      debug("CheckScreenSize: No change -> return.\n");
      return;
    }
#ifdef BLANKER_PRG
  KillBlanker();
#endif
  ResetIdle();
  ChangeScreenSize(wi, he, change_flag);
/* XXX Redisplay logic */
#if 0
  if (change_flag == 1)
    Redisplay(D_fore ? D_fore->w_norefresh : 0);
#endif
}

void
ChangeScreenSize(wi, he, change_fore)
int wi, he;
int change_fore;
{
  struct win *p;
  struct canvas *cv;
  int wwi;

  debug2("ChangeScreenSize from (%d,%d) ", D_width, D_height);
  debug3("to (%d,%d) (change_fore: %d)\n",wi, he, change_fore);

  cv = &D_canvas;
  cv->c_xe = wi - 1;
  cv->c_ys = (D_has_hstatus == HSTATUS_FIRSTLINE);
  cv->c_ye = he - 1 - ((cv->c_slperp && cv->c_slperp->c_slnext) || captionalways) - (D_has_hstatus == HSTATUS_LASTLINE);
  cv->c_blank.l_height = cv->c_ye - cv->c_ys + 1;
  if (cv->c_slperp)
    {
      ResizeCanvas(cv);
      RecreateCanvasChain();
      RethinkDisplayViewports();
    }
  if (D_forecv == 0)
    D_forecv = D_cvlist;
  if (D_forecv)
    D_fore = Layer2Window(D_forecv->c_layer);

  D_width = wi;
  D_height = he;

  CheckMaxSize(wi);
  if (D_CWS)
    {
      D_defwidth = D_CO;
      D_defheight = D_LI;
    }
  else
    {
      if (D_CZ0 && (wi == Z0width || wi == Z1width) &&
          (D_CO == Z0width || D_CO == Z1width))
        D_defwidth = D_CO;
      else
        D_defwidth = wi;
      D_defheight = he;
    }
  debug2("Default size: (%d,%d)\n", D_defwidth, D_defheight);
  if (change_fore)
    ResizeLayersToCanvases();
  if (change_fore == 2 && D_CWS == NULL && displays->d_next == 0)
    {
      /* adapt all windows  -  to be removed ? */
      for (p = windows; p; p = p->w_next)
        {
          debug1("Trying to change window %d.\n", p->w_number);
          wwi = wi;
#if 0
          if (D_CZ0 && p->w_width != wi && (wi == Z0width || wi == Z1width))
	    {
	      if (p->w_width > (Z0width + Z1width) / 2)
		wwi = Z0width;
	      else
		wwi = Z1width;
	    }
#endif
	  if (p->w_savelayer && p->w_savelayer->l_cvlist == 0)
	    ResizeLayer(p->w_savelayer, wwi, he, 0);
#if 0
          ChangeWindowSize(p, wwi, he, p->w_histheight);
#endif
        }
    }
}

void
ResizeLayersToCanvases()
{
  struct canvas *cv;
  struct layer *l;
  int lx, ly;

  debug("ResizeLayersToCanvases\n");
  D_kaablamm = 0;
  for (cv = D_cvlist; cv; cv = cv->c_next)
    {
      l = cv->c_layer;
      if (l == 0)
	continue;
      debug("Doing canvas: ");
      if (l->l_width  == cv->c_xe - cv->c_xs + 1 &&
          l->l_height == cv->c_ye - cv->c_ys + 1)
        {
          debug("already fitting.\n");
          continue;
        }
      if (!MayResizeLayer(l))
        {
          debug("may not resize.\n");
        }
      else
	{
	  debug("doing resize.\n");
	  ResizeLayer(l, cv->c_xe - cv->c_xs + 1, cv->c_ye - cv->c_ys + 1, display);
	}

      /* normalize window, see screen.c */
      lx = cv->c_layer->l_x;
      ly = cv->c_layer->l_y;
      if (ly + cv->c_yoff < cv->c_ys)
	{
          cv->c_yoff = cv->c_ys - ly;
          RethinkViewportOffsets(cv);
	}
      else if (ly + cv->c_yoff > cv->c_ye)
	{
	  cv->c_yoff = cv->c_ye - ly;
          RethinkViewportOffsets(cv);
	}
      if (lx + cv->c_xoff < cv->c_xs)
        {
	  int n = cv->c_xs - (lx + cv->c_xoff);
	  if (n < (cv->c_xe - cv->c_xs + 1) / 2)
	    n = (cv->c_xe - cv->c_xs + 1) / 2;
	  if (cv->c_xoff + n > cv->c_xs)
	    n = cv->c_xs - cv->c_xoff;
	  cv->c_xoff += n;
	  RethinkViewportOffsets(cv);
        }
      else if (lx + cv->c_xoff > cv->c_xe)
	{
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
  if (D_kaablamm)
    {
      kaablamm();
      D_kaablamm = 0;
    }
}

int
MayResizeLayer(l)
struct layer *l;
{
  int cvs = 0;
  debug("MayResizeLayer:\n");
  for (; l; l = l->l_next)
    {
      if (l->l_cvlist)
        if (++cvs > 1 || l->l_cvlist->c_lnext)
	  {
	    debug1("may not - cvs %d\n", cvs);
	    return 0;
	  }
    }
  debug("may resize\n");
  return 1;
}

/*
 *  Easy implementation: rely on the fact that the only layers
 *  supporting resize are Win and Blank. So just kill all overlays.
 *
 *  This is a lot harder if done the right way...
 */

static void
kaablamm()
{
  Msg(0, "Aborted because of window size change.");
}

/* Kills non-resizable layers. */
#define RESIZE_OR_KILL_LAYERS(l, wi, he) do \
  {	\
    struct layer *_last = NULL;	\
    flayer = (l);	\
    while (flayer->l_next)	\
      {	\
	if (LayResize(wi, he) == 0)	\
	  {	\
	    _last = flayer;	\
	    flayer = flayer->l_next;	\
	  }	\
	else	\
	  {	\
	    struct canvas *_cv;	\
	    for (_cv = flayer->l_cvlist; _cv; _cv = _cv->c_lnext)	\
	      _cv->c_display->d_kaablamm = 1;	\
	    ExitOverlayPage();	\
	    if (_last)	\
	      _last->l_next = flayer;	\
	  }	\
      }	\
    /* We assume that the bottom-most layer, i.e. when flayer->l_next == 0,	\
     * is always resizable. Currently, WinLf and BlankLf can be the bottom-most layers.	\
     */	\
    LayResize(wi, he);	\
  } while (0)

void
ResizeLayer(l, wi, he, norefdisp)
struct layer *l;
int wi, he;
struct display *norefdisp;
{
  struct win *p;
  struct canvas *cv;
  struct layer *oldflayer = flayer;
  struct display *d, *olddisplay = display;

  if (l->l_width == wi && l->l_height == he)
    return;
  p = Layer2Window(l);

  /* If 'flayer' and 'l' are for the same window, then we will not
   * restore 'flayer'. */
  if (oldflayer && (l == oldflayer || Layer2Window(oldflayer) == p))
    oldflayer = NULL;

  flayer = l;

  if (p)
    {
      /* It's a window layer. Kill the overlays on it in all displays. */
      for (d = displays; d; d = d->d_next)
	for (cv = d->d_cvlist; cv; cv = cv->c_next)
	  {
	    if (p == Layer2Window(cv->c_layer))
	      {
		/* Canvas 'cv' on display 'd' shows this window. Remove any non-resizable
		 * layers over it. */
		RESIZE_OR_KILL_LAYERS(cv->c_layer, wi, he);
	      }
	  }
    }
  else
    {
      /* It's a Blank layer. Just kill the non-resizable overlays over it. */
      RESIZE_OR_KILL_LAYERS(flayer, wi, he);
    }

  for (display = displays; display; display = display->d_next)
    {
      if (display == norefdisp)
	continue;
      for (cv = D_cvlist; cv; cv = cv->c_next)
	if (Layer2Window(cv->c_layer) == p)
	  {
            CV_CALL(cv, LayRedisplayLine(-1, -1, -1, 0));
            RefreshArea(cv->c_xs, cv->c_ys, cv->c_xe, cv->c_ye, 0);
	  }
      if (D_kaablamm)
	{
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

static void
FreeMline(ml)
struct mline *ml;
{
  if (ml->image)
    free(ml->image);
  if (ml->attr && ml->attr != null)
    free(ml->attr);
#ifdef FONT
  if (ml->font && ml->font != null)
    free(ml->font);
  if (ml->fontx && ml->fontx != null)
    free(ml->fontx);
#endif
#ifdef COLOR
  if (ml->color && ml->color != null)
    free(ml->color);
# ifdef COLORS256
  if (ml->colorx && ml->colorx != null)
    free(ml->colorx);
# endif
#endif
  *ml = mline_zero;
}

static int
AllocMline(ml, w)
struct mline *ml;
int w;
{
  ml->image = malloc(w);
  ml->attr  = null;
#ifdef FONT
  ml->font  = null;
  ml->fontx = null;
#endif
#ifdef COLOR
  ml->color = null;
# ifdef COLORS256
  ml->colorx = null;
# endif
#endif
  if (ml->image == 0)
    return -1;
  return 0;
}


static int
BcopyMline(mlf, xf, mlt, xt, l, w)
struct mline *mlf, *mlt;
int xf, xt, l, w;
{
  int r = 0;

  bcopy((char *)mlf->image + xf, (char *)mlt->image + xt, l);
  if (mlf->attr != null && mlt->attr == null)
    {
      if ((mlt->attr = (unsigned char *)calloc(w, 1)) == 0)
	mlt->attr = null, r = -1;
    }
  if (mlt->attr != null)
    bcopy((char *)mlf->attr + xf, (char *)mlt->attr + xt, l);
#ifdef FONT
  if (mlf->font != null && mlt->font == null)
    {
      if ((mlt->font = (unsigned char *)calloc(w, 1)) == 0)
	mlt->font = null, r = -1;
    }
  if (mlt->font != null)
    bcopy((char *)mlf->font + xf, (char *)mlt->font + xt, l);
  if (mlf->fontx != null && mlt->fontx == null)
    {
      if ((mlt->fontx = (unsigned char *)calloc(w, 1)) == 0)
	mlt->fontx = null, r = -1;
    }
  if (mlt->fontx != null)
    bcopy((char *)mlf->fontx + xf, (char *)mlt->fontx + xt, l);
#endif
#ifdef COLOR
  if (mlf->color != null && mlt->color == null)
    {
      if ((mlt->color = (unsigned char *)calloc(w, 1)) == 0)
	mlt->color = null, r = -1;
    }
  if (mlt->color != null)
    bcopy((char *)mlf->color + xf, (char *)mlt->color + xt, l);
# ifdef COLORS256
  if (mlf->colorx != null && mlt->colorx == null)
    {
      if ((mlt->colorx = (unsigned char *)calloc(w, 1)) == 0)
	mlt->colorx = null, r = -1;
    }
  if (mlt->colorx != null)
    bcopy((char *)mlf->colorx + xf, (char *)mlt->colorx + xt, l);
# endif
#endif
  return r;
}


static int maxwidth;

static void
CheckMaxSize(wi)
int wi;
{
  unsigned char *oldnull = null;
  unsigned char *oldblank = blank;
  struct win *p;
  int i;
  struct mline *ml;

  if (wi > MAXWIDTH)
	  wi = MAXWIDTH;
  if (wi <= maxwidth)
    return;
  maxwidth = wi + 1;
  debug1("New maxwidth: %d\n", maxwidth);
  blank = (unsigned char *)xrealloc((char *)blank, maxwidth);
  null = (unsigned char *)xrealloc((char *)null, maxwidth);
  mline_old.image = (unsigned char *)xrealloc((char *)mline_old.image, maxwidth);
  mline_old.attr = (unsigned char *)xrealloc((char *)mline_old.attr, maxwidth);
#ifdef FONT
  mline_old.font = (unsigned char *)xrealloc((char *)mline_old.font, maxwidth);
  mline_old.fontx = (unsigned char *)xrealloc((char *)mline_old.fontx, maxwidth);
#endif
#ifdef COLOR
  mline_old.color = (unsigned char *)xrealloc((char *)mline_old.color, maxwidth);
# ifdef COLORS256
  mline_old.colorx = (unsigned char *)xrealloc((char *)mline_old.colorx, maxwidth);
# endif
#endif
  if (!(blank && null && mline_old.image && mline_old.attr IFFONT(&& mline_old.font) IFFONTX(&& mline_old.fontx) IFCOLOR(&& mline_old.color) IFCOLORX(&& mline_old.colorx)))
    Panic(0, "%s", strnomem);

  MakeBlankLine(blank, maxwidth);
  bzero((char *)null, maxwidth);

  mline_blank.image = blank;
  mline_blank.attr  = null;
  mline_null.image = null;
  mline_null.attr  = null;
#ifdef FONT
  mline_blank.font  = null;
  mline_null.font   = null;
  mline_blank.fontx = null;
  mline_null.fontx  = null;
#endif
#ifdef COLOR
  mline_blank.color = null;
  mline_null.color = null;
# ifdef COLORS256
  mline_blank.colorx = null;
  mline_null.colorx = null;
# endif
#endif

#define RESET_AFC(x, bl) do { if (x == old##bl) x = bl; } while (0)

#define RESET_LINES(lines, count) \
  do { \
    ml = lines; \
    for (i = 0; i < count; i++, ml++) \
      { \
	RESET_AFC(ml->image, blank); \
	RESET_AFC(ml->attr, null); \
	IFFONT(RESET_AFC(ml->font, null)); \
	IFFONT(RESET_AFC(ml->fontx, null)); \
	IFCOLOR(RESET_AFC(ml->color, null)); \
	IFCOLORX(RESET_AFC(ml->colorx, null)); \
      } \
  } while (0)

  /* We have to run through all windows to substitute
   * the null and blank references.
   */
  for (p = windows; p; p = p->w_next)
    {
      RESET_LINES(p->w_mlines, p->w_height);

#ifdef COPY_PASTE
      RESET_LINES(p->w_hlines, p->w_histheight);
      RESET_LINES(p->w_alt.hlines, p->w_alt.histheight);
#endif

      RESET_LINES(p->w_alt.mlines, p->w_alt.height);
    }
}


char *
xrealloc(mem, len)
char *mem;
int len;
{
  register char *nmem;

  if (mem == 0)
    return malloc(len);
  if ((nmem = realloc(mem, len)))
    return nmem;
  free(mem);
  return (char *)0;
}

static void
MakeBlankLine(p, n)
register unsigned char *p;
register int n;
{
  while (n--)
    *p++ = ' ';
}




#ifdef COPY_PASTE

#define OLDWIN(y) ((y < p->w_histheight) \
        ? &p->w_hlines[(p->w_histidx + y) % p->w_histheight] \
        : &p->w_mlines[y - p->w_histheight])

#define NEWWIN(y) ((y < hi) ? &nhlines[y] : &nmlines[y - hi])
	
#else

#define OLDWIN(y) (&p->w_mlines[y])
#define NEWWIN(y) (&nmlines[y])

#endif


int
ChangeWindowSize(p, wi, he, hi)
struct win *p;
int wi, he, hi;
{
  struct mline *mlf = 0, *mlt = 0, *ml, *nmlines, *nhlines;
  int fy, ty, l, lx, lf, lt, yy, oty, addone;
  int ncx, ncy, naka, t;
  int y, shift;

  if (wi <= 0 || he <= 0)
    wi = he = hi = 0;

  if (p->w_type == W_TYPE_GROUP)
    return 0;

  if (wi > MAXWIDTH)
    {
      Msg(0, "Window width too large. Truncated to %d.", MAXWIDTH);
      wi = MAXWIDTH;
    }

  if (he > MAXWIDTH)
    {
      Msg(0, "Window height too large. Truncated to %d.", MAXWIDTH);
      he = MAXWIDTH;
    }

  if (p->w_width == wi && p->w_height == he && p->w_histheight == hi)
    {
      debug("ChangeWindowSize: No change.\n");
      return 0;
    }

  CheckMaxSize(wi);

  /* XXX */
#if 0
  /* just in case ... */
  if (wi && (p->w_width != wi || p->w_height != he) && p->w_lay != &p->w_winlay)
    {
      debug("ChangeWindowSize: No resize because of overlay?\n");
      return -1;
    }
#endif

  debug("ChangeWindowSize");
  debug3(" from (%d,%d)+%d", p->w_width, p->w_height, p->w_histheight);
  debug3(" to(%d,%d)+%d\n", wi, he, hi);

  fy = p->w_histheight + p->w_height - 1;
  ty = hi + he - 1;

  nmlines = nhlines = 0;
  ncx = 0;
  ncy = 0;
  naka = 0;

  if (wi)
    {
      if (wi != p->w_width || he != p->w_height)
	{
	  if ((nmlines = (struct mline *)calloc(he, sizeof(struct mline))) == 0)
	    {
	      KillWindow(p);
	      Msg(0, "%s", strnomem);
	      return -1;
	    }
	}
      else
	{
	  debug1("image stays the same: %d lines\n", he);
	  nmlines = p->w_mlines;
	  fy -= he;
	  ty -= he;
	  ncx = p->w_x;
	  ncy = p->w_y;
	  naka = p->w_autoaka;
	}
    }
#ifdef COPY_PASTE
  if (hi)
    {
      if ((nhlines = (struct mline *)calloc(hi, sizeof(struct mline))) == 0)
	{
	  Msg(0, "No memory for history buffer - turned off");
	  hi = 0;
	  ty = he - 1;
	}
    }
#endif

  /* special case: cursor is at magic margin position */
  addone = 0;
  if (p->w_width && p->w_x == p->w_width)
    {
      debug2("Special addone case: %d %d\n", p->w_x, p->w_y);
      addone = 1;
      p->w_x--;
    }

  /* handle the cursor and autoaka lines now if the widths are equal */
  if (p->w_width == wi)
    {
      ncx = p->w_x + addone;
      ncy = p->w_y + he - p->w_height;
      /* never lose sight of the line with the cursor on it */
      shift = -ncy;
      for (yy = p->w_y + p->w_histheight - 1; yy >= 0 && ncy + shift < he; yy--)
	{
	  ml = OLDWIN(yy);
	  if (!ml->image)
	    break;
	  if (ml->image[p->w_width] == ' ')
	    break;
	  shift++;
	}
      if (shift < 0)
	shift = 0;
      else
	debug1("resize: cursor out of bounds, shifting %d\n", shift);
      ncy += shift;
      if (p->w_autoaka > 0)
	{
	  naka = p->w_autoaka + he - p->w_height + shift;
	  if (naka < 1 || naka > he)
	    naka = 0;
	}
      while (shift-- > 0)
	{
	  ml = OLDWIN(fy);
	  FreeMline(ml);
	  fy--;
	}
    }
  debug2("fy %d ty %d\n", fy, ty);
  if (fy >= 0)
    mlf = OLDWIN(fy);
  if (ty >= 0)
    mlt = NEWWIN(ty);

  while (fy >= 0 && ty >= 0)
    {
      if (p->w_width == wi)
	{
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
      for (yy = fy - 1; yy >= 0; yy--)
	{
	  ml = OLDWIN(yy);
	  if (ml->image[p->w_width] == ' ')
	    break;
	  l += p->w_width;
	}

      /* rewrap lines */
      lt = (l - 1) % wi + 1;	/* lf is set above */
      oty = ty;
      while (l > 0 && fy >= 0 && ty >= 0)
	{
	  lx = lt > lf ? lf : lt;
	  if (mlt->image == 0)
	    {
	      if (AllocMline(mlt, wi + 1))
		goto nomem;
    	      MakeBlankLine(mlt->image + lt, wi - lt);
	      mlt->image[wi] = ((oty == ty) ? ' ' : 0);
	    }
	  if (BcopyMline(mlf, lf - lx, mlt, lt - lx, lx, wi + 1))
	    goto nomem;

	  /* did we copy the cursor ? */
	  if (fy == p->w_y + p->w_histheight && lf - lx <= p->w_x && lf > p->w_x)
	    {
	      ncx = p->w_x + lt - lf + addone;
	      ncy = ty - hi;
	      shift = wi ? -ncy + (l - lx) / wi : 0;
	      if (ty + shift > hi + he - 1)
		shift = hi + he - 1 - ty;
	      if (shift > 0)
		{
	          debug3("resize: cursor out of bounds, shifting %d [%d/%d]\n", shift, lt - lx, wi);
		  for (y = hi + he - 1; y >= ty; y--)
		    {
		      mlt = NEWWIN(y);
		      FreeMline(mlt);
		      if (y - shift < ty)
			continue;
		      ml  = NEWWIN(y - shift);
		      *mlt = *ml;
		      *ml = mline_zero;
		    }
		  ncy += shift;
		  ty += shift;
		  mlt = NEWWIN(ty);
		  if (naka > 0)
		    naka = naka + shift > he ? 0 : naka + shift;
		}
	      ASSERT(ncy >= 0);
	    }
	  /* did we copy autoaka line ? */
	  if (p->w_autoaka > 0 && fy == p->w_autoaka - 1 + p->w_histheight && lf - lx <= 0)
	    naka = ty - hi >= 0 ? 1 + ty - hi : 0;

	  lf -= lx;
	  lt -= lx;
	  l  -= lx;
	  if (lf == 0)
	    {
	      FreeMline(mlf);
	      lf = p->w_width;
	      if (--fy >= 0)
	        mlf = OLDWIN(fy);
	    }
	  if (lt == 0)
	    {
	      lt = wi;
	      if (--ty >= 0)
	        mlt = NEWWIN(ty);
	    }
	}
      ASSERT(l != 0 || fy == yy);
    }
  while (fy >= 0)
    {
      FreeMline(mlf);
      if (--fy >= 0)
	mlf = OLDWIN(fy);
    }
  while (ty >= 0)
    {
      if (AllocMline(mlt, wi + 1))
	goto nomem;
      MakeBlankLine(mlt->image, wi + 1);
      if (--ty >= 0)
	mlt = NEWWIN(ty);
    }

#ifdef DEBUG
  if (nmlines != p->w_mlines)
    for (fy = 0; fy < p->w_height + p->w_histheight; fy++)
      {
	ml = OLDWIN(fy);
	ASSERT(ml->image == 0);
      }
#endif

  if (p->w_mlines && p->w_mlines != nmlines)
    free((char *)p->w_mlines);
  p->w_mlines = nmlines;
#ifdef COPY_PASTE
  if (p->w_hlines && p->w_hlines != nhlines)
    free((char *)p->w_hlines);
  p->w_hlines = nhlines;
#endif
  nmlines = nhlines = 0;

  /* change tabs */
  if (p->w_width != wi)
    {
      if (wi)
	{
	  t = p->w_tabs ? p->w_width : 0;
	  p->w_tabs = xrealloc(p->w_tabs, wi + 1);
	  if (p->w_tabs == 0)
            goto nomem;
	  for (; t < wi; t++)
	    p->w_tabs[t] = t && !(t & 7) ? 1 : 0; 
	  p->w_tabs[wi] = 0; 
	}
      else
	{
	  if (p->w_tabs)
	    free(p->w_tabs);
	  p->w_tabs = 0;
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
#ifdef TIOCSWINSZ
  if (wi && (p->w_width != wi || p->w_height != he)
      && p->w_width != 0 && p->w_height != 0 && p->w_ptyfd >= 0 && p->w_pid)
    {
      glwz.ws_col = wi;
      glwz.ws_row = he;
      debug("Setting pty winsize.\n");
      if (ioctl(p->w_ptyfd, TIOCSWINSZ, (char *)&glwz))
	debug2("SetPtySize: errno %d (fd:%d)\n", errno, p->w_ptyfd);
    }
#endif /* TIOCSWINSZ */

  /* store new size */
  p->w_width = wi;
  p->w_height = he;
  if(p->w_scrollback_height > hi)
    p->w_scrollback_height = hi;
#ifdef COPY_PASTE
  p->w_histidx = 0;
  p->w_histheight = hi;
#endif

#ifdef BUILTIN_TELNET
  if (p->w_type == W_TYPE_TELNET)
    TelWindowSize(p);
#endif

#ifdef DEBUG
  /* Test if everything was ok */
  for (fy = 0; fy < p->w_height + p->w_histheight; fy++)
    {
      ml = OLDWIN(fy);
      ASSERT(ml->image);
# ifdef UTF8
      if (p->w_encoding == UTF8)
	{
	  for (l = 0; l < p->w_width; l++)
	    ASSERT(ml->image[l] >= ' ' || ml->font[l] || ml->fontx);
	}
      else
#endif
        for (l = 0; l < p->w_width; l++)
          ASSERT(ml->image[l] >= ' ');
    }
#endif
  return 0;

nomem:
  if (nmlines || nhlines)
    {
      for (ty = he + hi - 1; ty >= 0; ty--)
        {
          mlt = NEWWIN(ty);
          FreeMline(mlt);
        }
      if (nmlines && p->w_mlines != nmlines)
        free((char *)nmlines);
#ifdef COPY_PASTE
      if (nhlines && p->w_hlines != nhlines)
        free((char *)nhlines);
#endif
    }
  KillWindow(p);
  Msg(0, "%s", strnomem);
  return -1;
}

void
FreeAltScreen(p)
struct win *p;
{
  int i;

  if (p->w_alt.mlines)
    {
      for (i = 0; i < p->w_alt.height; i++)
        FreeMline(p->w_alt.mlines + i);
      free(p->w_alt.mlines);
    }
  p->w_alt.mlines = 0;
  p->w_alt.width = 0;
  p->w_alt.height = 0;
#ifdef COPY_PASTE
  if (p->w_alt.hlines)
    {
      for (i = 0; i < p->w_alt.histheight; i++)
        FreeMline(p->w_alt.hlines + i);
      free(p->w_alt.hlines);
    }
  p->w_alt.hlines = 0;
  p->w_alt.histidx = 0;
  p->w_alt.histheight = 0;
#endif
}

static void
SwapAltScreen(p)
struct win *p;
{
  struct mline *ml;
  int t;

#define SWAP(item, t) do { (t) = p->w_alt. item; p->w_alt. item = p->w_##item; p->w_##item = (t); } while (0)

  SWAP(mlines, ml);
  SWAP(width, t);
  SWAP(height, t);

#ifdef COPY_PASTE
  SWAP(histheight, t);
  SWAP(hlines, ml);
  SWAP(histidx, t);
#endif
#undef SWAP
}

void
EnterAltScreen(p)
struct win *p;
{
  if (!p->w_alt.on)
    {
      /* If not already using the alternate screen buffer, then create
	 a new one and swap it with the 'real' screen buffer. */
      FreeAltScreen(p);
      SwapAltScreen(p);
    }
  else
    {
      /* Already using the alternate buffer. Just clear the screen. To do so, it
	 is only necessary to reset the height(s) without resetting the width. */
      p->w_height = 0;
      p->w_histheight = 0;
    }
  ChangeWindowSize(p, p->w_alt.width, p->w_alt.height, p->w_alt.histheight);
  p->w_alt.on = 1;
}

void
LeaveAltScreen(p)
struct win *p;
{
  if (!p->w_alt.on)
    return;
  SwapAltScreen(p);
  ChangeWindowSize(p, p->w_alt.width, p->w_alt.height, p->w_alt.histheight);
  FreeAltScreen(p);
  p->w_alt.on = 0;
}
