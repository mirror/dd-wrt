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

#include "config.h"
#include "screen.h"
#include "list_generic.h"
#include "layer.h"
#include "extern.h"

/* Deals with a generic list display */

extern struct layer *flayer;

static void ListProcess __P((char **, int *));
static void ListAbort __P((void));
static void ListRedisplayLine __P((int, int, int, int));
static void ListClearLine __P((int, int, int, int));
static int  ListRewrite __P((int, int, int, struct mchar *, int));
static int  ListResize __P((int, int));
static void ListRestore __P((void));
static void ListFree __P((void *));

struct LayFuncs ListLf =
{
  ListProcess,
  ListAbort,
  ListRedisplayLine,
  ListClearLine,
  ListRewrite,
  ListResize,
  ListRestore,
  ListFree
};

/** Returns non-zero on success. */
struct ListData *
glist_display(struct GenericList *list, const char *name)
{
  struct ListData *ldata;

  if (InitOverlayPage(sizeof(struct ListData), &ListLf, 0))
    return NULL;
  ldata = flayer->l_data;

  ldata->name = name;	/* We do not SaveStr, since the strings should be all static literals */
  ldata->list_fn = list;

  flayer->l_mode = 1;
  flayer->l_x = 0;
  flayer->l_y = flayer->l_height - 1;

  return ldata;
}

static void
glist_decide_top(struct ListData *ldata)
{
  int count = flayer->l_height - 5;	/* 2 for header, 1 for footer */
  struct ListRow *top = ldata->selected;
  for (; count && top != ldata->root; top = top->prev, count--)
    ;
  ldata->top = top;
}

static struct ListRow *
glist_search_dir(struct ListData *ldata, struct ListRow *start, int dir)
{
  struct ListRow *row = (dir == 1) ? start->next : start->prev;
  for (; row; row = (dir == 1) ? row->next : row->prev)
    if (ldata->list_fn->gl_matchrow(ldata, row, ldata->search))
      return row;

  if (dir == 1)
    row = ldata->root;
  else
    {
      /* First, go to the end */
      if (!start->next)
	row = start;
      else
	for (row = start->next; row->next; row = row->next)
	  ;
    }

  for (; row != start; row = (dir == 1) ? row->next : row->prev)
    if (ldata->list_fn->gl_matchrow(ldata, row, ldata->search))
      break;

  return row;
}

static void
glist_search(char *buf, int len, char *data)
{
  struct ListData *ldata = (struct ListData *)data;
  struct ListRow *row;

  if (ldata->search)
    Free(ldata->search);
  if (len > 0)
    ldata->search = SaveStr(buf);
  else
    return;

  for (row = ldata->selected; row; row = row->next)
    if (ldata->list_fn->gl_matchrow(ldata, row, ldata->search))
      break;

  if (!row)
    for (row = ldata->root; row != ldata->selected; row = row->next)
      if (ldata->list_fn->gl_matchrow(ldata, row, ldata->search))
	break;

  if (row == ldata->selected)
    return;

  ldata->selected = row;
  if (ldata->selected->y == -1)
    glist_decide_top(ldata);
  glist_display_all(ldata);
}

static void ListProcess(char **ppbuf, int *plen)
{
  struct ListData *ldata = flayer->l_data;
  int count = 0;

  while (*plen > 0)
    {
      struct ListRow *old;
      unsigned char ch;

      if (!flayer->l_mouseevent.start && ldata->list_fn->gl_pinput &&
	  ldata->list_fn->gl_pinput(ldata, ppbuf, plen))
	continue;

      ch = **ppbuf;
      ++*ppbuf;
      --*plen;

      if (flayer->l_mouseevent.start)
	{
	  int r = LayProcessMouse(flayer, ch);
	  if (r == -1)
	    {
	      LayProcessMouseSwitch(flayer, 0);
	      continue;
	    }
	  else
	    {
	      if (r)
		ch = 0222;
	      else
		continue;
	    }
	}

      if (!ldata->selected)
	{
	  *plen = 0;
	  break;
	}

      old = ldata->selected;

processchar:
      switch (ch)
	{
	case ' ':
	  break;

	case '\r':
	case '\n':
	  break;

	case 0220:	/* up */
	case 16:	/* ^P */
	case 'k':
	  if (!ldata->selected->prev)	/* There's no where to go */
	    break;
	  ldata->selected = old->prev;
	  break;

	case 0216:	/* down */
	case 14:	/* ^N like emacs */
	case 'j':
	  if (!ldata->selected->next)	/* Nothing to do */
	    break;
	  ldata->selected = old->next;
	  break;

	case 033:	/* escape */
	case 007:	/* ^G */
	  ListAbort();
	  *plen = 0;
	  return;

	case 0201:	/* home */
	case 0001:	/* ^A */
	  ldata->selected = ldata->root;
	  break;

	case 0205:	/* end */
	case 0005:	/* ^E */
	  while (ldata->selected->next)
	    ldata->selected = ldata->selected->next;
	  if (ldata->selected->y != -1)
	    {
	      /* Both old and current selections are on the screen. So we can just
	       * redraw these two affected rows. */
	    }
	  break;

	case 0004:	/* ^D (half-page down) */
	case 0006:	/* page-down, ^F */
	  count = (flayer->l_height - 4) >> (ch == 0004);
	  for (; ldata->selected->next && --count;
	      ldata->selected = ldata->selected->next)
	    ;
	  break;

	case 0025:	/* ^U (half-page up) */
	case 0002:	/* page-up, ^B */
	  count = (flayer->l_height - 4) >> (ch == 0025);
	  for (; ldata->selected->prev && --count;
	      ldata->selected = ldata->selected->prev)
	    ;
	  break;

	case '/':	/* start searching */
	  if (ldata->list_fn->gl_matchrow)
	    {
	      char *s;
	      Input("Search: ", 80, INP_COOKED, glist_search, (char *)ldata, 0);
	      if ((s = ldata->search))
		{
		  for (; *s; s++)
		    {
		      char *ss = s;
		      int n = 1;
		      LayProcess(&ss, &n);
		    }
		}
	    }
	  break;

	/* The following deal with searching. */

	case 'n':	/* search next */
	  if (ldata->list_fn->gl_matchrow && ldata->search)
	    ldata->selected = glist_search_dir(ldata, ldata->selected, 1);
	  break;

	case 'N':	/* search prev */
	  if (ldata->list_fn->gl_matchrow && ldata->search)
	    ldata->selected = glist_search_dir(ldata, ldata->selected, -1);
	  break;

	/* Now, mouse events. */
	case 0222:
	  if (flayer->l_mouseevent.start)
	    {
	      int button = flayer->l_mouseevent.buffer[0];
	      if (button == 'a') /* Scroll down */
		ch = 'j';
	      else if (button == '`') /* Scroll up */
		ch = 'k';
	      else if (button == ' ') /* Left click */
		{
		  int y = flayer->l_mouseevent.buffer[2];
		  struct ListRow *r = ldata->top;
		  for (r = ldata->top; r && r->y != -1 && r->y != y; r = r->next)
		    ;
		  if (r && r->y == y)
		    ldata->selected = r;
		  ch = 0;
		}
	      else
		ch = 0;
	      LayProcessMouseSwitch(flayer, 0);
	      if (ch)
		goto processchar;
	    }
	  else
	    LayProcessMouseSwitch(flayer, 1);
	  break;
	}

      if (old == ldata->selected)	/* The selection didn't change */
	continue;

      if (ldata->selected->y == -1)
	{
	  /* We need to list all the rows, since we are scrolling down. But first,
	   * find the top of the visible list. */
	  glist_decide_top(ldata);
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
    }
}

static void ListAbort(void)
{
  LAY_CALL_UP(LRefreshAll(flayer, 0));
  ExitOverlayPage();
}

static void ListFree(void *d)
{
  struct ListData *ldata = d;
  glist_remove_rows(ldata);
  if (ldata->list_fn->gl_free)
    ldata->list_fn->gl_free(ldata);
  if (ldata->search)
    Free(ldata->search);
}

static void ListRedisplayLine(int y, int xs, int xe, int isblank)
{
  struct ListData *ldata;
  ASSERT(flayer);

  ldata = flayer->l_data;
  if (y < 0)
    {
      glist_display_all(ldata);
      return;
    }

  if (!isblank)
    LClearArea(flayer, xs, y, xe, y, 0, 0);

  if (ldata->top && y < ldata->top->y)
    ldata->list_fn->gl_printheader(ldata);
  else if (y + 1 == flayer->l_height)
    ldata->list_fn->gl_printfooter(ldata);
  else
    {
      struct ListRow *row;
      for (row = ldata->top; row && row->y != -1; row = row->next)
	if (row->y == y)
	  {
	    ldata->list_fn->gl_printrow(ldata, row);
	    break;
	  }
    }
}

static void ListClearLine(int y, int xs, int xe, int bce)
{
  DefClearLine(y, xs, xe, bce);
}

static int  ListRewrite(int y, int xs, int xe, struct mchar *rend, int doit)
{
  return EXPENSIVE;
}

static int ListResize (int wi, int he)
{
  if (wi < 10 || he < 5)
    return -1;

  flayer->l_width = wi;
  flayer->l_height = he;
  flayer->l_y = he - 1;

  return 0;
}

static void ListRestore (void)
{
  DefRestore();
}

struct ListRow *
glist_add_row(struct ListData *ldata, void *data, struct ListRow *after)
{
  struct ListRow *r = calloc(1, sizeof(struct ListRow));
  r->data = data;

  if (after)
    {
      r->next = after->next;
      r->prev = after;
      after->next = r;
      if (r->next)
	r->next->prev = r;
    }
  else
    {
      r->next = ldata->root;
      if (ldata->root)
	ldata->root->prev = r;
      ldata->root = r;
    }

  return r;
}

void
glist_remove_rows(struct ListData *ldata)
{
  struct ListRow *row;
  for (row = ldata->root; row; )
    {
      struct ListRow *r = row;
      row = row->next;
      ldata->list_fn->gl_freerow(ldata, r);
      free(r);
    }
  ldata->root = ldata->selected = ldata->top = NULL;
}

void
glist_display_all(struct ListData *list)
{
  int y;
  struct ListRow *row;

  LClearAll(flayer, 0);

  y = list->list_fn->gl_printheader(list);

  if (!list->top)
    list->top = list->root;
  if (!list->selected)
    list->selected = list->root;

  for (row = list->root; row != list->top; row = row->next)
    row->y = -1;

  for (row = list->top; row; row = row->next)
    {
      row->y = y++;
      if (!list->list_fn->gl_printrow(list, row))
	{
	  row->y = -1;
	  y--;
	}
      if (y + 1 == flayer->l_height)
	break;
    }
  for (; row; row = row->next)
    row->y = -1;

  list->list_fn->gl_printfooter(list);
  if (list->selected && list->selected->y != -1)
    flayer->l_y = list->selected->y;
  else
    flayer->l_y = flayer->l_height - 1;
  LaySetCursor();
}

void glist_abort(void)
{
  ListAbort();
}

