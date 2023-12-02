/* Copyright (c) 2010
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
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

/* Deals with the list of windows */

/* NOTE: A 'struct win *' is used as the 'data' for each row. It might make more sense
 * to use 'struct win* ->w_number' as the 'data', instead, because that way, we can
 * verify that the window does exist (by looking at wtab[]).
 */

#include "config.h"
#include "screen.h"
#include "layer.h"
#include "extern.h"
#include "list_generic.h"

extern struct layer *flayer;
extern struct display *display, *displays;

extern char *wlisttit;
extern char *wliststr;

extern struct mchar mchar_blank, mchar_so;
extern int renditions[];

extern struct win **wtab, *windows, *fore;
extern int maxwin;

extern char *noargs[];

static char ListID[] = "window";

struct gl_Window_Data
{
  struct win *group;	/* Set only for a W_TYPE_GROUP window */
  int order;		/* MRU? NUM? */
  int onblank;
  int nested;
  struct win *fore;	/* The foreground window we had. */
};

/* Is this wdata for a group window? */
#define WLIST_FOR_GROUP(wdate)	((wdata)->group && !(wdata)->onblank && Layer2Window(flayer) && Layer2Window(flayer)->w_type == W_TYPE_GROUP)

/* This macro should not be used if 'fn' is expected to update the window list */
#define FOR_EACH_WINDOW(_wdata, _w, fn) do {	\
    if ((_wdata)->order == WLIST_MRU)	\
      {	\
	struct win *_ww;	\
	for (_ww = windows; _ww; _ww = _ww->w_next)	\
	  {	\
	    _w = _ww;	\
	    fn	\
	  }	\
      }	\
    else	\
      {	\
	struct win **_ww, *_witer;	\
	for (_ww = wtab, _witer = windows; _witer && _ww - wtab < maxwin; _ww++)	\
	  {	\
	    if (!(_w = *_ww))	continue;	\
	    fn	\
	    _witer = _witer->w_next;	\
	  }	\
      }	\
  } while (0)

/* Is 'a' an ancestor of 'd'? */
static int
window_ancestor(struct win *a, struct win *d)
{
  if (!a)
    return 1;	/* Every window is a descendant of the 'null' group */
  for (; d; d = d->w_group)
    if (d->w_group == a)
      return 1;
  return 0;
}

static void
window_kill_confirm(char *buf, int len, char *data)
{
  struct win *w = windows;
  struct action act;

  if (len || (*buf != 'y' && *buf != 'Y'))
    {
      memset(buf, 0, len);
      return;
    }

  /* Loop over the windows to make sure that the window actually still exists. */
  for (; w; w = w->w_next)
    if (w == (struct win *)data)
      break;

  if (!w)
    return;

  /* Pretend the selected window is the foreground window. Then trigger a non-interactive 'kill' */
  fore = w;
  act.nr = RC_KILL;
  act.args = noargs;
  act.argl = 0;
  act.quiet = 0;
  DoAction(&act, -1);
}

static struct ListRow *
gl_Window_add_group(struct ListData *ldata, struct ListRow *row)
{
  /* Right now, 'row' doesn't have any child. */
  struct gl_Window_Data *wdata = ldata->data;
  struct win *group = row->data, *w;
  struct ListRow *cur = row;

  ASSERT(wdata->nested);

  FOR_EACH_WINDOW(wdata, w,
    if (w->w_group != group)
      continue;

    cur = glist_add_row(ldata, w, cur);
    if (w == wdata->fore)
      ldata->selected = cur;

    if (w->w_type == W_TYPE_GROUP)
      cur = gl_Window_add_group(ldata, cur);
  );

  return cur;
}

static void
gl_Window_rebuild(struct ListData *ldata)
{
  struct ListRow *row = NULL;
  struct gl_Window_Data *wdata = ldata->data;
  struct win *w;

  FOR_EACH_WINDOW(wdata, w,
    if (w->w_group != wdata->group)
      continue;
    row = glist_add_row(ldata, w, row);
    if (w == wdata->fore)
      ldata->selected = row;
    if (w->w_type == W_TYPE_GROUP && wdata->nested)
      row = gl_Window_add_group(ldata, row);
  );
  glist_display_all(ldata);
}

static struct ListRow *
gl_Window_findrow(struct ListData *ldata, struct win *p)
{
  struct ListRow *row = ldata->root;
  for (; row; row = row->next)
    {
      if (row->data == p)
	break;
    }
  return row;
}

static int
gl_Window_remove(struct ListData *ldata, struct win *p)
{
  struct ListRow *row = gl_Window_findrow(ldata, p);
  if (!row)
    return 0;

  /* Remove 'row'. Update 'selected', 'top', 'root' if necessary. */
  if (row->next)
    row->next->prev = row->prev;
  if (row->prev)
    row->prev->next = row->next;

  if (ldata->selected == row)
    ldata->selected = row->prev ? row->prev : row->next;
  if (ldata->top == row)
    ldata->top = row->prev ? row->prev : row->next;
  if (ldata->root == row)
    ldata->root = row->next;

  ldata->list_fn->gl_freerow(ldata, row);
  free(row);

  return 1;
}

static int
gl_Window_header(struct ListData *ldata)
{
  char *str;
  struct gl_Window_Data *wdata = ldata->data;
  int g;

  if ((g = (wdata->group != NULL)))
    {
      LPutWinMsg(flayer, "Group: ", 7, &mchar_blank, 0, 0);
      LPutWinMsg(flayer, wdata->group->w_title, strlen(wdata->group->w_title), &mchar_blank, 7, 0);
    }

  str = MakeWinMsgEv(wlisttit, (struct win *)0, '%', flayer->l_width, (struct event *)0, 0);

  LPutWinMsg(flayer, str, strlen(str), &mchar_blank, 0, g);
  return 2 + g;
}

static int
gl_Window_footer(struct ListData *ldata)
{
  return 0;
}

static int
gl_Window_row(struct ListData *ldata, struct ListRow *lrow)
{
  char *str;
  struct win *w, *g;
  int xoff;
  struct mchar *mchar;
  struct mchar mchar_rend = mchar_blank;
  struct gl_Window_Data *wdata = ldata->data;

  w = lrow->data;

  /* First, make sure we want to display this window in the list.
   * If we are showing a list for a group, and not on blank, then we must
   * only show the windows directly belonging to that group.
   * Otherwise, do some more checks. */

  for (xoff = 0, g = w->w_group; g != wdata->group; g = g->w_group)
    xoff += 2;
  str = MakeWinMsgEv(wliststr, w, '%', flayer->l_width - xoff, NULL, 0);
  if (ldata->selected == lrow)
    mchar = &mchar_so;
  else if (w->w_monitor == MON_DONE && renditions[REND_MONITOR] != -1)
    {
      mchar = &mchar_rend;
      ApplyAttrColor(renditions[REND_MONITOR], mchar);
    }
  else if ((w->w_bell == BELL_DONE || w->w_bell == BELL_FOUND) && renditions[REND_BELL] != -1)
    {
      mchar = &mchar_rend;
      ApplyAttrColor(renditions[REND_BELL], mchar);
    }
  else if ((w->w_silence == SILENCE_FOUND || w->w_silence == SILENCE_DONE) && renditions[REND_SILENCE] != -1)
    {
      mchar = &mchar_rend;
      ApplyAttrColor(renditions[REND_SILENCE], mchar);
    }
  else
    mchar = &mchar_blank;

  LPutWinMsg(flayer, str, flayer->l_width, mchar, xoff, lrow->y);
  if (xoff)
    LPutWinMsg(flayer, "", xoff, mchar, 0, lrow->y);

  return 1;
}

static int
gl_Window_input(struct ListData *ldata, char **inp, int *len)
{
  struct win *win;
  unsigned char ch;
  struct display *cd = display;
  struct gl_Window_Data *wdata = ldata->data;

  if (!ldata->selected)
    return 0;

  ch = (unsigned char) **inp;
  ++*inp;
  --*len;

  win = ldata->selected->data;
  switch (ch)
    {
    case ' ':
    case '\n':
    case '\r':
      if (!win)
	break;
#ifdef MULTIUSER
      if (display && AclCheckPermWin(D_user, ACL_READ, win))
	return 0;		/* Not allowed to switch to this window. */
#endif
      if (WLIST_FOR_GROUP(wdata))
	SwitchWindow(win->w_number);
      else
	{
	  /* Abort list only when not in a group window. */
	  glist_abort();
	  display = cd;
	  if (D_fore != win)
	    SwitchWindow(win->w_number);
	}
      *len = 0;
      break;

    case 'm':
      /* Toggle MRU-ness */
      wdata->order = wdata->order == WLIST_MRU ? WLIST_NUM : WLIST_MRU;
      glist_remove_rows(ldata);
      gl_Window_rebuild(ldata);
      break;

    case 'g':
      /* Toggle nestedness */
      wdata->nested = !wdata->nested;
      glist_remove_rows(ldata);
      gl_Window_rebuild(ldata);
      break;

    case 'a':
      /* All-window view */
      if (wdata->group)
	{
	  int order = wdata->order | (wdata->nested ? WLIST_NESTED : 0);
	  glist_abort();
	  display = cd;
	  display_windows(1, order, NULL);
	  *len = 0;
	}
      else if (!wdata->nested)
	{
	  wdata->nested = 1;
	  glist_remove_rows(ldata);
	  gl_Window_rebuild(ldata);
	}
      break;

    case 010:	/* ^H */
    case 0177:	/* Backspace */
      if (!wdata->group)
	break;
      if (wdata->group->w_group)
	{
	  /* The parent is another group window. So switch to that window. */
	  struct win *g = wdata->group->w_group;
	  glist_abort();
	  display = cd;
	  SetForeWindow(g);
	  *len = 0;
	}
      else
	{
	  /* We were in a group view. Now we are moving to an all-window view.
	   * So treat it as 'windowlist on blank'. */
	  int order = wdata->order | (wdata->nested ? WLIST_NESTED : 0);
	  glist_abort();
	  display = cd;
	  display_windows(1, order, NULL);
	  *len = 0;
	}
      break;

    case ',':	/* Switch numbers with the previous window. */
      if (wdata->order == WLIST_NUM && ldata->selected->prev)
	{
	  struct win *pw = ldata->selected->prev->data;
	  if (win->w_group != pw->w_group)
	    break;	/* Do not allow switching with the parent group */

	  /* When a windows's number is successfully changed, it triggers a WListUpdatecv
	   * with NULL window. So that causes a redraw of the entire list. So reset the
	   * 'selected' after that. */
	  wdata->fore = win;
	  WindowChangeNumber(win->w_number, pw->w_number);
	}
      break;

    case '.':	/* Switch numbers with the next window. */
      if (wdata->order == WLIST_NUM && ldata->selected->next)
	{
	  struct win *nw = ldata->selected->next->data;
	  if (win->w_group != nw->w_group)
	    break;	/* Do not allow switching with the parent group */

	  wdata->fore = win;
	  WindowChangeNumber(win->w_number, nw->w_number);
	}
      break;

    case 'K':	/* Kill a window */
      {
	char str[MAXSTR];
	snprintf(str, sizeof(str) - 1, "Really kill window %d (%s) [y/n]",
	    win->w_number, win->w_title);
	Input(str, 1, INP_RAW, window_kill_confirm, (char *)win, 0);
      }
      break;

    case 033:	/* escape */
    case 007:	/* ^G */
      if (!WLIST_FOR_GROUP(wdata))
	{
	  int fnumber = wdata->onblank ? wdata->fore->w_number : -1;
	  glist_abort();
	  display = cd;
	  if (fnumber >= 0)
	    SwitchWindow(fnumber);
	  *len = 0;
	}
      break;
    default:
      if (ch >= '0' && ch <= '9')
	{
	  struct ListRow *row = ldata->root;
	  for (; row; row = row->next)
	    {
	      struct win *w = row->data;
	      if (w->w_number == ch - '0')
		{
		  struct ListRow *old = ldata->selected;
		  if (old == row)
		    break;
		  ldata->selected = row;
		  if (ldata->selected->y == -1)
		    {
		      /* We need to list all the rows, since we are scrolling down. But first,
		       * find the top of the visible list. */
		      ldata->top = row;
		      glist_display_all(ldata);
		    }
		  else
		    {
		      /* just redisplay the two lines. */
		      ldata->list_fn->gl_printrow(ldata, old);
		      ldata->list_fn->gl_printrow(ldata, ldata->selected);
		      flayer->l_y = ldata->selected->y;
		      LaySetCursor();
		    }
		  break;
		}
	    }
	  break;
	}
      --*inp;
      ++*len;
      return 0;
    }
  return 1;
}

static int
gl_Window_freerow(struct ListData *ldata, struct ListRow *row)
{
  return 0;
}

static int
gl_Window_free(struct ListData *ldata)
{
  Free(ldata->data);
  return 0;
}

static int
gl_Window_match(struct ListData *ldata, struct ListRow *row, const char *needle)
{
  struct win *w = row->data;
  if (InStr(w->w_title, needle))
    return 1;
  return 0;
}

static struct GenericList gl_Window =
{
  gl_Window_header,
  gl_Window_footer,
  gl_Window_row,
  gl_Window_input,
  gl_Window_freerow,
  gl_Window_free,
  gl_Window_match
};

void
display_windows(int onblank, int order, struct win *group)
{
  struct win *p;
  struct ListData *ldata;
  struct gl_Window_Data *wdata;

  if (flayer->l_width < 10 || flayer->l_height < 6)
    {
      LMsg(0, "Window size too small for window list page");
      return;
    }

  if (group)
    onblank = 0;	/* When drawing a group window, ignore 'onblank' */

  if (onblank)
    {
      debug3("flayer %lx %d %x\n", (long)flayer, flayer->l_width, flayer->l_height);
      if (!display)
	{
	  LMsg(0, "windowlist -b: display required");
	  return;
	}
      p = D_fore;
      if (p)
	{
	  SetForeWindow((struct win *)0);
          if (p->w_group)
	    {
	      D_fore = p->w_group;
	      flayer->l_data = (char *)p->w_group;
	    }
	  Activate(0);
	}
      if (flayer->l_width < 10 || flayer->l_height < 6)
	{
	  LMsg(0, "Window size too small for window list page");
	  return;
	}
    }
  else
    p = Layer2Window(flayer);
  if (!group && p)
    group = p->w_group;

  ldata = glist_display(&gl_Window, ListID);
  if (!ldata)
    {
      if (onblank && p)
	{
	  /* Could not display the list. So restore the window. */
	  SetForeWindow(p);
	  Activate(1);
	}
      return;
    }

  wdata = calloc(1, sizeof(struct gl_Window_Data));
  wdata->group = group;
  wdata->order = (order & ~WLIST_NESTED);
  wdata->nested = !!(order & WLIST_NESTED);
  wdata->onblank = onblank;

  /* Set the most recent window as selected. */
  wdata->fore = windows;
  while (wdata->fore && wdata->fore->w_group != group)
    wdata->fore = wdata->fore->w_next;

  ldata->data = wdata;

  gl_Window_rebuild(ldata);
}

static void
WListUpdate(struct win *p, struct ListData *ldata)
{
  struct gl_Window_Data *wdata = ldata->data;
  struct ListRow *row, *rbefore;
  struct win *before;
  int d = 0, sel = 0;

  if (!p)
    {
      if (ldata->selected)
	wdata->fore = ldata->selected->data;	/* Try to retain the current selection */
      glist_remove_rows(ldata);
      gl_Window_rebuild(ldata);
      return;
    }

  /* First decide if this window should be displayed at all. */
  d = 1;
  if (wdata->order == WLIST_NUM || wdata->order == WLIST_MRU)
    {
      if (p->w_group != wdata->group)
	{
	  if (!wdata->nested)
	    d = 0;
	  else
	    d = window_ancestor(wdata->group, p);
	}
    }

  if (!d)
    {
      if (gl_Window_remove(ldata, p))
	glist_display_all(ldata);
      return;
    }

  /* OK, so we keep the window in the list. Update the ordering.
   * First, find the row where this window should go to. Then, either create
   * a new row for that window, or move the exising row for the window to the
   * correct place. */
  before = NULL;
  if (wdata->order == WLIST_MRU)
    {
      if (windows != p)
	for (before = windows; before; before = before->w_next)
	  if (before->w_next == p)
	    break;
    }
  else if (wdata->order == WLIST_NUM)
    {
      if (p->w_number != 0)
	{
	  struct win **w = wtab + p->w_number - 1;
	  for (; w >= wtab; w--)
	    {
	      if (*w && (*w)->w_group == wdata->group)
		{
		  before = *w;
		  break;
		}
	    }
	}
    }

  /* Now, find the row belonging to 'before' */
  if (before)
    rbefore = gl_Window_findrow(ldata, before);
  else if (wdata->nested && p->w_group)	/* There's no 'before'. So find the group window */
    rbefore = gl_Window_findrow(ldata, p->w_group);
  else
    rbefore = NULL;

  /* For now, just remove the row containing 'p' if it is not already in the right place . */
  row = gl_Window_findrow(ldata, p);
  if (row)
    {
      if (row->prev != rbefore)
	{
	  sel = ldata->selected->data == p;
	  gl_Window_remove(ldata, p);
	}
      else
	p = NULL;		/* the window is in the correct place */
    }
  if (p)
    {
      row = glist_add_row(ldata, p, rbefore);
      if (sel)
	ldata->selected = row;
    }
  glist_display_all(ldata);
}

void
WListUpdatecv(cv, p)
struct canvas *cv;
struct win *p;
{
  struct ListData *ldata;

  if (cv->c_layer->l_layfn != &ListLf)
    return;
  ldata = cv->c_layer->l_data;
  if (ldata->name != ListID)
    return;
  CV_CALL(cv, WListUpdate(p, ldata));
}

void
WListLinkChanged()
{
  struct display *olddisplay = display;
  struct canvas *cv;
  struct ListData *ldata;
  struct gl_Window_Data *wdata;

  for (display = displays; display; display = display->d_next)
    for (cv = D_cvlist; cv; cv = cv->c_next)
      {
        if (!cv->c_layer || cv->c_layer->l_layfn != &ListLf)
	  continue;
	ldata = cv->c_layer->l_data;
	if (ldata->name != ListID)
	  continue;
	wdata = ldata->data;
	if (!(wdata->order & WLIST_MRU))
	  continue;
        CV_CALL(cv, WListUpdate(0, ldata));
      }
  display = olddisplay;
}

