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
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"
#include "screen.h"
#include "extern.h"
#include "canvas.h"
#include "list_generic.h"

extern struct display *display;
extern struct win *fore, *windows;
extern struct layer *flayer;
extern int captionalways;
extern struct LayFuncs BlankLf;
extern int focusminwidth, focusminheight;

static void
CanvasInitBlank(cv)
struct canvas *cv;
{
  cv->c_blank.l_cvlist = cv;
  cv->c_blank.l_width = cv->c_xe - cv->c_xs + 1;
  cv->c_blank.l_height = cv->c_ye - cv->c_ys + 1;
  cv->c_blank.l_x = cv->c_blank.l_y = 0;
  cv->c_blank.l_layfn = &BlankLf;
  cv->c_blank.l_data = 0;
  cv->c_blank.l_next = 0;
  cv->c_blank.l_bottom = &cv->c_blank;
  cv->c_blank.l_blocking = 0;
  cv->c_layer = &cv->c_blank;
}

static void
FreePerp(pcv)
struct canvas *pcv;
{
  struct canvas *cv;

  if (!pcv->c_slperp)
    return;
  cv = pcv->c_slperp;
  cv->c_slprev = pcv->c_slprev;
  if (cv->c_slprev)
    cv->c_slprev->c_slnext = cv;
  cv->c_slback = pcv->c_slback;
  if (cv->c_slback && cv->c_slback->c_slperp == pcv)
    cv->c_slback->c_slperp = cv;
  cv->c_slorient = pcv->c_slorient;
  cv->c_slweight = pcv->c_slweight;
  while (cv->c_slnext)
    {
      cv = cv->c_slnext;
      cv->c_slorient = pcv->c_slorient;
      cv->c_slback = pcv->c_slback;
      cv->c_slweight = pcv->c_slweight;
    }
  cv->c_slnext = pcv->c_slnext;
  if (cv->c_slnext)
    cv->c_slnext->c_slprev = cv;
  LayerCleanupMemory(&pcv->c_blank);
  free(pcv);
}

void
FreeCanvas(cv)
struct canvas *cv;
{
  struct viewport *vp, *nvp;
  struct canvas **cvp;
  struct win *p;

  if (cv->c_slprev)
    cv->c_slprev->c_slnext = cv->c_slnext;
  if (cv->c_slnext)
    cv->c_slnext->c_slprev = cv->c_slprev;
  if (cv->c_slback && cv->c_slback->c_slperp == cv)
    cv->c_slback->c_slperp = cv->c_slnext ? cv->c_slnext : cv->c_slprev;
  if (cv->c_slperp)
    {
      while (cv->c_slperp)
	FreeCanvas(cv->c_slperp);
      LayerCleanupMemory(&cv->c_blank);
      free(cv);
      return;
    }

  if (display)
    {
      if (D_forecv == cv)
	D_forecv = 0;
      /* remove from canvas chain as SetCanvasWindow might call
       * some layer function */
      for (cvp = &D_cvlist; *cvp ; cvp = &(*cvp)->c_next)
	if (*cvp == cv)
	  {
	    *cvp = cv->c_next;
	    break;
	  }
    }
  p = cv->c_layer ? Layer2Window(cv->c_layer) : 0;
  SetCanvasWindow(cv, 0);
  if (p)
    WindowChanged(p, 'u');
  if (flayer == cv->c_layer)
    flayer = 0;
  for (vp = cv->c_vplist; vp; vp = nvp)
    {
      vp->v_canvas = 0;
      nvp = vp->v_next;
      vp->v_next = 0;
      free(vp);
    }
  evdeq(&cv->c_captev);
  LayerCleanupMemory(&cv->c_blank);
  free(cv);
}

int
CountCanvas(cv)
struct canvas *cv;
{
  int num = 0;
  for (; cv; cv = cv->c_slnext)
    {
      if (cv->c_slperp)
	{
	  struct canvas *cvp;
	  int nump = 1, n;
          for (cvp = cv->c_slperp; cvp; cvp = cvp->c_slnext)
	    if (cvp->c_slperp)
	      {
		n = CountCanvas(cvp->c_slperp);
		if (n > nump)
		  nump = n;
	      }
	  num += nump;
	}
      else
	num++;
    }
  return num;
}

int
CountCanvasPerp(cv)
struct canvas *cv;
{
  struct canvas *cvp;
  int num = 1, n;
  for (cvp = cv->c_slperp; cvp; cvp = cvp->c_slnext)
    if (cvp->c_slperp)
      {
	n = CountCanvas(cvp->c_slperp);
	if (n > num)
	  num = n;
      }
  return num;
}

struct canvas *
FindCanvas(x, y)
int x, y;
{
  struct canvas *cv, *mcv = 0;
  int m, mm = 0;

  for (cv = D_cvlist; cv; cv = cv->c_next)
    {
      /* ye + 1 because of caption line */
      if (x >= cv->c_xs && x <= cv->c_xe && y >= cv->c_ys && y <= cv->c_ye + 1)
	return cv;
      if (cv == D_forecv)
	continue;
      m = 0;
      if (x >= D_forecv->c_xs && x <= D_forecv->c_xe)
	{
	  if (x < cv->c_xs || x > cv->c_xe)
	    continue;
          if (y < D_forecv->c_ys && y < cv->c_ys)
	    continue;
          if (y > D_forecv->c_ye + 1 && y > cv->c_ye + 1)
	    continue;
	  if (y < cv->c_ys)
	    m = cv->c_ys - y;
	  if (y > cv->c_ye + 1)
	    m = y - (cv->c_ye + 1);
	}
      if (y >= D_forecv->c_ys && y <= D_forecv->c_ye + 1)
	{
	  if (y < cv->c_ys || y > cv->c_ye + 1)
	    continue;
          if (x < D_forecv->c_xs && x < cv->c_xs)
	    continue;
          if (x > D_forecv->c_xe && x > cv->c_xe)
	    continue;
	  if (x < cv->c_xs)
	    m = cv->c_xs - x;
	  if (x > cv->c_xe)
	    m = x - cv->c_xe;
	}
      if (m && (!mm || m < mm))
	{
	  mcv = cv;
	  mm = m;
	}
    }
  return mcv ? mcv : D_forecv;
}

void
SetCanvasWindow(cv, wi)
struct canvas *cv;
struct win *wi;
{
  struct win *p = 0, **pp;
  struct layer *l;
  struct canvas *cvp, **cvpp;

  l = cv->c_layer;
  display = cv->c_display;

  if (l)
    {
      /* remove old layer */
      for (cvpp = &l->l_cvlist; (cvp = *cvpp); cvpp = &cvp->c_lnext)
	if (cvp == cv)
	  break;
      ASSERT(cvp);
      *cvpp = cvp->c_lnext;

      p = Layer2Window(l);
      l = cv->c_layer;
      cv->c_layer = 0;

      if (p && cv == D_forecv)
	{
#ifdef MULTIUSER
	  ReleaseAutoWritelock(display, p);
#endif
	  if (p->w_silence)
	    {
	      SetTimeout(&p->w_silenceev, p->w_silencewait * 1000);
	      evenq(&p->w_silenceev);
	    }
	  D_other = fore;
	  D_fore = 0;
	}
      if (l->l_cvlist == 0 && (p == 0 || l != p->w_savelayer))
	KillLayerChain(l);
    }

  /* find right layer to display on canvas */
  if (wi && wi->w_type != W_TYPE_GROUP)
    {
      l = &wi->w_layer;
      if (wi->w_savelayer && (wi->w_blocked || wi->w_savelayer->l_cvlist == 0))
	l = wi->w_savelayer;
    }
  else
    {
      l = &cv->c_blank;
      if (wi)
	l->l_data = (char *)wi;
      else
	l->l_data = 0;
    }

  /* add our canvas to the layer's canvaslist */
  ASSERT(l->l_cvlist != cv);
  cv->c_lnext = l->l_cvlist;
  l->l_cvlist = cv;
  cv->c_layer = l;
  cv->c_xoff = cv->c_xs;
  cv->c_yoff = cv->c_ys;
  RethinkViewportOffsets(cv);

  if (flayer == 0)
    flayer = l;

  if (wi && wi->w_type == W_TYPE_GROUP)
    {
      /* auto-start windowlist on groups */
      struct display *d = display;
      struct layer *oldflayer = flayer;
      flayer = l;
      display_windows(0, 0, wi);
      flayer = oldflayer;
      display = d;
    }

  if (wi && D_other == wi)
    D_other = wi->w_next;	/* Might be 0, but that's OK. */
  if (cv == D_forecv)
    {
      D_fore = wi;
      fore = D_fore;	/* XXX ? */
      if (wi)
	{
#ifdef MULTIUSER
	  ObtainAutoWritelock(display, wi);
#endif
	  /*
	   * Place the window at the head of the most-recently-used list
	   */
	  if (windows != wi)
	    {
	      for (pp = &windows; (p = *pp); pp = &p->w_next)
		if (p == wi)
		  break;
	      ASSERT(p);
	      *pp = p->w_next;
	      p->w_next = windows;
	      windows = p;
	      WListLinkChanged();
	    }
	}
    }
}

static void
cv_winid_fn(ev, data)
struct event *ev;
char *data;
{
  int ox, oy;
  struct canvas *cv = (struct canvas *)data;

  display = cv->c_display;
  if (D_status == STATUS_ON_WIN)
    {
      SetTimeout(ev, 1);
      evenq(ev);
      return;
    }
  ox = D_x;
  oy = D_y;
  if (cv->c_ye + 1 < D_height)
    RefreshLine(cv->c_ye + 1, 0, D_width - 1, 0);
  if (ox != -1 && oy != -1)
    GotoPos(ox, oy);
}

int
MakeDefaultCanvas()
{
  struct canvas *cv;
 
  ASSERT(display);
  if ((cv = (struct canvas *)calloc(1, sizeof *cv)) == 0)
    return -1;
  cv->c_xs      = 0;
  cv->c_xe      = D_width - 1;
  cv->c_ys      = (D_has_hstatus == HSTATUS_FIRSTLINE);
  cv->c_ye      = D_height - 1 - (D_has_hstatus == HSTATUS_LASTLINE) - captionalways;
  debug2("MakeDefaultCanvas 0,0 %d,%d\n", cv->c_xe, cv->c_ye);
  cv->c_xoff    = 0;
  cv->c_yoff    = 0;
  cv->c_next = 0;
  cv->c_display = display;
  cv->c_vplist = 0;
  cv->c_slnext = 0;
  cv->c_slprev = 0;
  cv->c_slperp = 0;
  cv->c_slweight = 1;
  cv->c_slback = &D_canvas;
  D_canvas.c_slperp = cv;
  D_canvas.c_xs = cv->c_xs;
  D_canvas.c_xe = cv->c_xe;
  D_canvas.c_ys = cv->c_ys;
  D_canvas.c_ye = cv->c_ye;
  cv->c_slorient = SLICE_UNKN;
  cv->c_captev.type = EV_TIMEOUT;
  cv->c_captev.data = (char *)cv;
  cv->c_captev.handler = cv_winid_fn;

  CanvasInitBlank(cv);
  cv->c_lnext = 0;

  D_cvlist = cv;
  RethinkDisplayViewports();
  D_forecv = cv; 	      /* default input focus */
  return 0;
}

static struct canvas **
CreateCanvasChainRec(cv, cvp)
struct canvas *cv;
struct canvas **cvp;
{
  for (; cv; cv = cv->c_slnext)
    {
      if (cv->c_slperp)
	cvp = CreateCanvasChainRec(cv->c_slperp, cvp);
      else
	{
	  *cvp = cv;
	  cvp = &cv->c_next;
	}
    }
  return cvp;
}

void
RecreateCanvasChain()
{
  struct canvas **cvp;
  cvp = CreateCanvasChainRec(D_canvas.c_slperp, &D_cvlist);
  *cvp = 0;
}

void
EqualizeCanvas(cv, gflag)
struct canvas *cv;
int gflag;
{
  struct canvas *cv2;
  for (; cv; cv = cv->c_slnext)
    {
      if (cv->c_slperp && gflag)
	{
	  cv->c_slweight = CountCanvasPerp(cv);
	  for (cv2 = cv->c_slperp; cv2; cv2 = cv2->c_slnext)
	    if (cv2->c_slperp)
	      EqualizeCanvas(cv2->c_slperp, gflag);
	}
      else
	cv->c_slweight = 1;
    }
}

void
ResizeCanvas(cv)
struct canvas *cv;
{
  struct canvas *cv2, *cvn, *fcv;
  int nh, i, maxi, hh, m, w, wsum;
  int need, got;
  int xs, ys, xe, ye;
  int focusmin = 0;

  xs = cv->c_xs;
  ys = cv->c_ys;
  xe = cv->c_xe;
  ye = cv->c_ye;
  cv = cv->c_slperp;
  debug2("ResizeCanvas: %d,%d", xs, ys);
  debug2(" %d,%d\n", xe, ye);
  if (cv == 0)
    return;
  if (cv->c_slorient == SLICE_UNKN)
    {
      ASSERT(!cv->c_slnext && !cv->c_slperp);
      cv->c_xs = xs;
      cv->c_xe = xe;
      cv->c_ys = ys;
      cv->c_ye = ye;
      cv->c_xoff = cv->c_xs;
      cv->c_yoff = cv->c_ys;
      cv->c_blank.l_width = cv->c_xe - cv->c_xs + 1;
      cv->c_blank.l_height = cv->c_ye - cv->c_ys + 1;
      return;
    }

  fcv = 0;
  if (focusminwidth || focusminheight)
    {
      debug("searching for focus canvas\n");
      cv2 = D_forecv;
      while (cv2->c_slback)
        {
	  if (cv2->c_slback == cv->c_slback)
	    {
	      fcv = cv2;
	      focusmin = cv->c_slorient == SLICE_VERT ? focusminheight : focusminwidth;
	      if (focusmin > 0)
		focusmin--;
	      else if (focusmin < 0)
		focusmin = cv->c_slorient == SLICE_VERT ? ye - ys + 2 : xe - xs + 2;
	      debug1("found, focusmin=%d\n", focusmin);
	    }
          cv2 = cv2->c_slback;
        }
    }
  if (focusmin)
    {
      m = CountCanvas(cv) * 2;
      nh = cv->c_slorient == SLICE_VERT ? ye - ys + 2 : xe - xs + 2;
      nh -= m;
      if (nh < 0)
	nh = 0;
      if (focusmin > nh)
	focusmin = nh;
      debug1("corrected to %d\n", focusmin);
    }

  /* pass 1: calculate weight sum */
  for (cv2 = cv, wsum = 0; cv2; cv2 = cv2->c_slnext)
    {
      debug1("  weight %d\n", cv2->c_slweight);
      wsum += cv2->c_slweight;
    }
  debug1("wsum = %d\n", wsum);
  if (wsum == 0)
    wsum = 1;
  w = wsum;

  /* pass 2: calculate need/excess space */
  nh = cv->c_slorient == SLICE_VERT ? ye - ys + 2 : xe - xs + 2;
  for (cv2 = cv, need = got = 0; cv2; cv2 = cv2->c_slnext)
    {
      m = cv2->c_slperp ? CountCanvasPerp(cv2) * 2 - 1 : 1;
      if (cv2 == fcv)
	m += focusmin;
      hh = cv2->c_slweight ? nh * cv2->c_slweight / w : 0;
      w -= cv2->c_slweight;
      nh -= hh;
      debug2("  should %d min %d\n", hh, m);
      if (hh <= m + 1)
        need += m + 1 - hh;
      else
        got += hh - m - 1;
    }
  debug2("need: %d, got %d\n", need, got);
  if (need > got)
    need = got;

  /* pass 3: distribute space */
  nh = cv->c_slorient == SLICE_VERT ? ye - ys + 2 : xe - xs + 2;
  i = cv->c_slorient == SLICE_VERT ? ys : xs;
  maxi = cv->c_slorient == SLICE_VERT ? ye : xe;
  w = wsum;
  for (; cv; cv = cvn)
    {
      cvn = cv->c_slnext;
      if (i > maxi)
	{
	  if (cv->c_slprev && !cv->c_slback->c_slback && !cv->c_slprev->c_slperp && !cv->c_slprev->c_slprev)
	    {
	      cv->c_slprev->c_slorient = SLICE_UNKN;
	      if (!captionalways)
		{
	          cv->c_slback->c_ye++;
		  cv->c_slprev->c_ye++;
		}
	    }
	  SetCanvasWindow(cv, 0);
	  FreeCanvas(cv);
	  continue;
	}
      m = cv->c_slperp ? CountCanvasPerp(cv) * 2 - 1 : 1;
      if (cv == fcv)
	m += focusmin;
      hh = cv->c_slweight ? nh * cv->c_slweight / w : 0;
      w -= cv->c_slweight;
      nh -= hh;
      debug2("  should %d min %d\n", hh, m);
      if (hh <= m + 1)
	{
	  hh = m + 1;
	  debug1(" -> %d\n", hh);
	}
      else
	{
	  int hx = 1;       //FIXME Division by zero (got) is posible. "hx = 1" is random number here!!!
    if (got != 0)
      hx = need * (hh - m - 1) / got;
    else
      debug("   got = 0\n");

	  debug3(" -> %d - %d = %d\n", hh, hx, hh - hx);
	  got -= (hh - m - 1);
	  hh -= hx;
	  need -= hx;
          debug2("   now need=%d got=%d\n", need, got);
	}
      ASSERT(hh >= m + 1);
      /* hh is window size plus pation line */
      if (i + hh > maxi + 2)
	{
	  hh = maxi + 2 - i;
	  debug1("  not enough space, reducing to %d\n", hh);
	}
      if (i + hh == maxi + 1)
	{
	  hh++;
	  debug("  incrementing as no other canvas will fit\n");
	}
      if (cv->c_slorient == SLICE_VERT)
        {
          cv->c_xs = xs;
          cv->c_xe = xe;
          cv->c_ys = i;
          cv->c_ye = i + hh - 2;
          cv->c_xoff = xs;
          cv->c_yoff = i;
        }
      else
        {
          cv->c_xs = i;
          cv->c_xe = i + hh - 2;
          cv->c_ys = ys;
          cv->c_ye = ye;
          cv->c_xoff = i;
          cv->c_yoff = ys;
        }
      cv->c_xoff = cv->c_xs;
      cv->c_yoff = cv->c_ys;
      cv->c_blank.l_width = cv->c_xe - cv->c_xs + 1;
      cv->c_blank.l_height = cv->c_ye - cv->c_ys + 1;
      if (cv->c_slperp)
	{
          ResizeCanvas(cv);
	  if (!cv->c_slperp->c_slnext)
	    {
	      debug("deleting perp node\n");
	      FreePerp(cv->c_slperp);
	      FreePerp(cv);
	    }
	}
      i += hh;
    }
}

static struct canvas *
AddPerp(cv)
struct canvas *cv;
{
  struct canvas *pcv;
  debug("Creating new perp node\n");

  if ((pcv = (struct canvas *)calloc(1, sizeof *cv)) == 0)
    return 0;
  pcv->c_next = 0;
  pcv->c_display = cv->c_display;
  pcv->c_slnext = cv->c_slnext;
  pcv->c_slprev = cv->c_slprev;
  pcv->c_slperp = cv;
  pcv->c_slback = cv->c_slback;
  if (cv->c_slback && cv->c_slback->c_slperp == cv)
    cv->c_slback->c_slperp = pcv;
  pcv->c_slorient = cv->c_slorient;
  pcv->c_xoff = 0;
  pcv->c_yoff = 0;
  pcv->c_xs = cv->c_xs;
  pcv->c_xe = cv->c_xe;
  pcv->c_ys = cv->c_ys;
  pcv->c_ye = cv->c_ye;
  if (pcv->c_slnext)
    pcv->c_slnext->c_slprev = pcv;
  if (pcv->c_slprev)
    pcv->c_slprev->c_slnext = pcv;
  pcv->c_slweight = cv->c_slweight;
  CanvasInitBlank(pcv);
  cv->c_slweight = 1;
  cv->c_slnext = 0;
  cv->c_slprev = 0;
  cv->c_slperp = 0;
  cv->c_slback = pcv;
  cv->c_slorient = SLICE_UNKN;
  return pcv;
}

int
AddCanvas(orient)
int orient;
{
  struct canvas *cv;
  int xs, xe, ys, ye;
  int h, num;

  cv = D_forecv;
  debug2("AddCanvas orient %d, forecv is %d\n", orient, cv->c_slorient);

  if (cv->c_slorient != SLICE_UNKN && cv->c_slorient != orient)
    if (!AddPerp(cv))
      return -1;

  cv = D_forecv;
  xs = cv->c_slback->c_xs;
  xe = cv->c_slback->c_xe;
  ys = cv->c_slback->c_ys;
  ye = cv->c_slback->c_ye;
  if (!captionalways && cv == D_canvas.c_slperp && !cv->c_slnext)
    ye--;	/* need space for caption */
  debug2("Adding Canvas to slice %d,%d ", xs, ys);
  debug2("%d,%d\n", xe, ye);

  num = CountCanvas(cv->c_slback->c_slperp) + 1;
  debug1("Num = %d\n", num);
  if (orient == SLICE_VERT)
    h = ye - ys + 1;
  else
    h = xe - xs + 1;

  h -= 2 * num - 1;
  if (h < 0)
    return -1;		/* can't fit in */

  if ((cv = (struct canvas *)calloc(1, sizeof *cv)) == 0)
    return -1;

  D_forecv->c_slback->c_ye = ye;	/* in case we modified it above */
  D_forecv->c_slorient = orient;	/* in case it was UNKN */
  cv->c_slnext = D_forecv->c_slnext;
  cv->c_slprev = D_forecv;
  D_forecv->c_slnext = cv;
  if (cv->c_slnext)
    cv->c_slnext->c_slprev = cv;
  cv->c_slorient = orient;
  cv->c_slback = D_forecv->c_slback;

  cv->c_xs      = xs;
  cv->c_xe      = xe;
  cv->c_ys      = ys;
  cv->c_ye      = ye;
  cv->c_xoff    = 0;
  cv->c_yoff    = 0;
  cv->c_display = display;
  cv->c_vplist  = 0;
  cv->c_captev.type = EV_TIMEOUT;
  cv->c_captev.data = (char *)cv;
  cv->c_captev.handler = cv_winid_fn;

  CanvasInitBlank(cv);
  cv->c_lnext = 0;

  cv->c_next    = 0;

  cv = cv->c_slback;
  EqualizeCanvas(cv->c_slperp, 0);
  ResizeCanvas(cv);
  RecreateCanvasChain();
  RethinkDisplayViewports();
  ResizeLayersToCanvases();
  return 0;
}

void
RemCanvas()
{
  int ye;
  struct canvas *cv;

  debug("RemCanvas\n");
  cv = D_forecv;
  ye = cv->c_slback->c_ye;
  if (cv->c_slorient == SLICE_UNKN)
    return;
  while (cv->c_slprev)
    cv = cv->c_slprev;
  if (!cv->c_slnext)
    return;
  if (!cv->c_slnext->c_slnext && cv->c_slback->c_slback)
    {
      /* two canvases in slice, kill perp node */
      cv = D_forecv;
      debug("deleting perp node\n");
      FreePerp(cv->c_slprev ? cv->c_slprev : cv->c_slnext);
      FreePerp(cv->c_slback);
    }
  /* free canvas */
  cv = D_forecv;
  D_forecv = cv->c_slprev;
  if (!D_forecv)
    D_forecv = cv->c_slnext;
  FreeCanvas(cv);

  cv = D_forecv;
  while (D_forecv->c_slperp)
    D_forecv = D_forecv->c_slperp;

  /* if only one canvas left, set orient back to unknown */
  if (!cv->c_slnext && !cv->c_slprev && !cv->c_slback->c_slback && !cv->c_slperp)
    {
      cv->c_slorient = SLICE_UNKN;
      if (!captionalways)
	cv->c_slback->c_ye = ++ye;	/* caption line no longer needed */
    }
  cv = cv->c_slback;
  EqualizeCanvas(cv->c_slperp, 0);
  ResizeCanvas(cv);

  D_fore = Layer2Window(D_forecv->c_layer);
  flayer = D_forecv->c_layer;

  RecreateCanvasChain();
  RethinkDisplayViewports();
  ResizeLayersToCanvases();
}

void
OneCanvas()
{
  struct canvas *cv = D_forecv, *ocv = 0;

  if (cv->c_slprev)
    {
      ocv = cv->c_slprev;
      cv->c_slprev->c_slnext = cv->c_slnext;
    }
  if (cv->c_slnext)
    {
      ocv = cv->c_slnext;
      cv->c_slnext->c_slprev = cv->c_slprev;
    }
  if (!ocv)
    return;
  if (cv->c_slback && cv->c_slback->c_slperp == cv)
    cv->c_slback->c_slperp = ocv;
  cv->c_slorient = SLICE_UNKN;
  while (D_canvas.c_slperp)
    FreeCanvas(D_canvas.c_slperp);
  cv = D_forecv;
  D_canvas.c_slperp = cv;
  cv->c_slback = &D_canvas;
  cv->c_slnext = 0;
  cv->c_slprev = 0;
  ASSERT(!cv->c_slperp);
  if (!captionalways)
    D_canvas.c_ye++;	/* caption line no longer needed */
  ResizeCanvas(&D_canvas);
  RecreateCanvasChain();
  RethinkDisplayViewports();
  ResizeLayersToCanvases();
}

void
DupLayoutCv(cvf, cvt, save)
struct canvas *cvf, *cvt;
int save;
{
  while(cvf)
    {
      cvt->c_slorient = cvf->c_slorient;
      cvt->c_slweight = cvf->c_slweight;
      if (cvf == D_forecv)
        D_forecv = cvt;
      if (!save)
	{
	  cvt->c_display = display;
	  if (!cvf->c_slperp)
	    {
	      cvt->c_captev.type = EV_TIMEOUT;
	      cvt->c_captev.data = (char *)cvt;
	      cvt->c_captev.handler = cv_winid_fn;
	      cvt->c_blank.l_cvlist = 0;
	      cvt->c_blank.l_layfn = &BlankLf;
	      cvt->c_blank.l_bottom = &cvt->c_blank;
	    }
          cvt->c_layer = cvf->c_layer;
	}
      else
	{
	  struct win *p = cvf->c_layer ? Layer2Window(cvf->c_layer) : 0;
          cvt->c_layer = p ? &p->w_layer : 0;
	}
      if (cvf->c_slperp)
	{
	  cvt->c_slperp = (struct canvas *)calloc(1, sizeof(struct canvas));
	  cvt->c_slperp->c_slback = cvt;
	  CanvasInitBlank(cvt->c_slperp);
	  DupLayoutCv(cvf->c_slperp, cvt->c_slperp, save);
	}
      if (cvf->c_slnext)
	{
	  cvt->c_slnext = (struct canvas *)calloc(1, sizeof(struct canvas));
	  cvt->c_slnext->c_slprev = cvt;
	  cvt->c_slnext->c_slback = cvt->c_slback;
	  CanvasInitBlank(cvt->c_slnext);
	}
      cvf = cvf->c_slnext;
      cvt = cvt->c_slnext;
    }
}

void
PutWindowCv(cv)
struct canvas *cv;
{
  struct win *p;
  for (; cv; cv = cv->c_slnext)
    {
      if (cv->c_slperp)
	{
	  PutWindowCv(cv->c_slperp);
	  continue;
	}
      p = cv->c_layer ? (struct win *)cv->c_layer->l_data : 0;
      cv->c_layer = 0;
      SetCanvasWindow(cv, p);
    }
}

