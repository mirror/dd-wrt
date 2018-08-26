/* SLang Scrolling Window Routines */
/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

static void find_window_bottom (SLscroll_Window_Type *win)
{
   unsigned int nrows;
   unsigned int hidden_mask;
   SLscroll_Type *bot, *cline, *last_bot;
   unsigned int row;

   nrows = win->nrows;
   hidden_mask = win->hidden_mask;
   cline = win->current_line;

   win->window_row = row = 0;
   last_bot = bot = win->top_window_line;

   while (row < nrows)
     {
	if (bot == cline)
	  win->window_row = row;

	last_bot = bot;

	if (bot == NULL)
	  break;

	bot = bot->next;

	if (hidden_mask)
	  {
	     while ((bot != NULL) && (bot->flags & hidden_mask))
	       bot = bot->next;
	  }

	row++;
     }

   win->bot_window_line = last_bot;
}

static int find_top_to_recenter (SLscroll_Window_Type *win)
{
   unsigned int nrows;
   unsigned int hidden_mask;
   SLscroll_Type *prev, *last_prev, *cline;

   nrows = win->nrows;
   cline = win->current_line;
   hidden_mask = win->hidden_mask;

   nrows = nrows / 2;

   last_prev = prev = cline;

   while (nrows && (prev != NULL))
     {
	nrows--;
	last_prev = prev;
	do
	  {
	     prev = prev->prev;
	  }
	while (hidden_mask
	       && (prev != NULL)
	       && (prev->flags & hidden_mask));
     }

   if (prev == NULL) prev = last_prev;

   win->top_window_line = prev;
   find_window_bottom (win);

   return 0;
}

#define HAS_BORDER_CODE 1
int SLscroll_find_top (SLscroll_Window_Type *win)
{
   unsigned int i;
   SLscroll_Type *cline, *prev, *next;
   SLscroll_Type *top_window_line;
   unsigned int nrows;
   unsigned int hidden_mask;
   int scroll_mode;
   unsigned int border;

   cline = win->current_line;
   nrows = win->nrows;
   scroll_mode = win->cannot_scroll;
   border = win->border;
   if (scroll_mode == 2)
     border = 0;

   if ((cline == NULL) || (nrows <= 1))
     {
	win->top_window_line = cline;
	find_window_bottom (win);
	return 0;
     }

   hidden_mask = win->hidden_mask;

   /* Note: top_window_line might be a bogus pointer.  This means that I cannot
    * access it unless it really corresponds to a pointer in the buffer.
    */
   top_window_line = win->top_window_line;

   if (top_window_line == NULL)
     return find_top_to_recenter (win);

   /* Chances are that the current line is visible in the window.  This means
    * that the top window line should be above it.
    */
   prev = cline;

   i = 0;

   while ((i < nrows) && (prev != NULL))
     {
	if (prev == top_window_line)
	  {
	     SLscroll_Type *twl = top_window_line;
	     int dir = 0;

	     if (i < border) dir = -1; else if (i + border >= nrows) dir = 1;

	     if (dir) while (border)
	       {
		  if (dir < 0) twl = twl->prev;
		  else twl = twl->next;

		  if (twl == NULL)
		    {
		       twl = top_window_line;
		       break;
		    }
		  if ((hidden_mask == 0)
		      || (0 == (twl->flags & hidden_mask)))
		    border--;
	       }

	     win->top_window_line = twl;
	     find_window_bottom (win);
	     return 0;
	  }

	do
	  {
	     prev = prev->prev;
	  }
	while (hidden_mask
	       && (prev != NULL)
	       && (prev->flags & hidden_mask));
	i++;
     }

   /* Now check the borders of the window.  Perhaps the current line lies
    * outsider the border by a line.  Only do this if terminal can scroll.
    */

   if (scroll_mode == 1)
     return find_top_to_recenter (win);
   else if (scroll_mode == -1)
     scroll_mode = 0;

   next = cline->next;
   while (hidden_mask
	  && (next != NULL)
	  && (next->flags & hidden_mask))
     next = next->next;

   if ((next != NULL)
       && (next == top_window_line))
     {
	/* The current line is one line above the window.  This means user
	 * has moved up past the top of the window.  If scroll_mode is set
	 * to scroll by pages, we need to do a page up.
	 */

	win->top_window_line = cline;
	find_window_bottom (win);

	if (scroll_mode) return SLscroll_pageup (win);

	return 0;
     }

   prev = cline->prev;

   while (hidden_mask
	  && (prev != NULL)
	  && (prev->flags & hidden_mask))
     prev = prev->prev;

   if ((prev == NULL)
       || (prev != win->bot_window_line))
     return find_top_to_recenter (win);

   /* It looks like cline is below window by one line.  See what line should
    * be at top to scroll it into view.  Only do this unless we are scrolling
    * by pages.
    */
   if (scroll_mode)
     {
	win->top_window_line = cline;
	find_window_bottom (win);
	return 0;
     }

   i = 2;
   while ((i < nrows) && (prev != NULL))
     {
	do
	  {
	     prev = prev->prev;
	  }
	while (hidden_mask
	       && (prev != NULL)
	       && (prev->flags & hidden_mask));
	i++;
     }

   if (prev != NULL)
     {
	win->top_window_line = prev;
	find_window_bottom (win);
	return 0;
     }

   return find_top_to_recenter (win);
}

int SLscroll_find_line_num (SLscroll_Window_Type *win)
{
   SLscroll_Type *cline, *l;
   unsigned int n;
   unsigned int hidden_mask;

   if (win == NULL) return -1;

   hidden_mask = win->hidden_mask;
   cline = win->current_line;

   n = 1;

   l = win->lines;
   while (l != cline)
     {
	if ((hidden_mask == 0)
	    || (0 == (l->flags & hidden_mask)))
	  n++;

	l = l->next;
     }

   win->line_num = n;
   n--;

   while (l != NULL)
     {
	if ((hidden_mask == 0)
	    || (0 == (l->flags & hidden_mask)))
	  n++;
	l = l->next;
     }
   win->num_lines = n;

   return 0;
}

unsigned int SLscroll_next_n (SLscroll_Window_Type *win, unsigned int n)
{
   unsigned int i;
   unsigned int hidden_mask;
   SLscroll_Type *l, *cline;

   if ((win == NULL)
       || (NULL == (cline = win->current_line)))
     return 0;

   hidden_mask = win->hidden_mask;
   l = cline;
   i = 0;
   while (i < n)
     {
	l = l->next;
	while (hidden_mask
	       && (l != NULL) && (l->flags & hidden_mask))
	  l = l->next;

	if (l == NULL)
	  break;

	i++;
	cline = l;
     }

   win->current_line = cline;
   win->line_num += i;
   return i;
}

unsigned int SLscroll_prev_n (SLscroll_Window_Type *win, unsigned int n)
{
   unsigned int i;
   unsigned int hidden_mask;
   SLscroll_Type *l, *cline;

   if ((win == NULL)
       || (NULL == (cline = win->current_line)))
     return 0;

   hidden_mask = win->hidden_mask;
   l = cline;
   i = 0;
   while (i < n)
     {
	l = l->prev;
	while (hidden_mask
	       && (l != NULL) && (l->flags & hidden_mask))
	  l = l->prev;

	if (l == NULL)
	  break;

	i++;
	cline = l;
     }

   win->current_line = cline;
   win->line_num -= i;
   return i;
}

int SLscroll_pageup (SLscroll_Window_Type *win)
{
   SLscroll_Type *l, *top;
   unsigned int nrows, hidden_mask;
   unsigned int n;

   if (win == NULL)
     return -1;

   (void) SLscroll_find_top (win);

   nrows = win->nrows;

   if ((NULL != (top = win->top_window_line))
       && (nrows > 2))
     {
	n = 0;
	hidden_mask = win->hidden_mask;
	l = win->current_line;
	while ((l != NULL) && (l != top))
	  {
	     l = l->prev;
	     if ((hidden_mask == 0)
		 || ((l != NULL) && (0 == (l->flags & hidden_mask))))
	       n++;
	  }

	if (l != NULL)
	  {
	     unsigned int save_line_num;
	     int ret = 0;

	     win->current_line = l;
	     win->line_num -= n;

	     /* Compute a new top/bottom header */
	     save_line_num = win->line_num;

	     if ((0 == SLscroll_prev_n (win, nrows - 1))
		 && (n == 0))
	       ret = -1;

	     win->top_window_line = win->current_line;
	     win->current_line = l;
	     win->line_num = save_line_num;

	     find_window_bottom (win);
	     return ret;
	  }
     }

   if (nrows < 2) nrows++;
   if (0 == SLscroll_prev_n (win, nrows - 1))
     return -1;
   return 0;
}

int SLscroll_pagedown (SLscroll_Window_Type *win)
{
   SLscroll_Type *l, *bot;
   unsigned int nrows, hidden_mask;
   unsigned int n;

   if (win == NULL)
     return -1;

   (void) SLscroll_find_top (win);

   nrows = win->nrows;

   if ((NULL != (bot = win->bot_window_line))
       && (nrows > 2))
     {
	n = 0;
	hidden_mask = win->hidden_mask;
	l = win->current_line;
	while ((l != NULL) && (l != bot))
	  {
	     l = l->next;
	     if ((hidden_mask == 0)
		 || ((l != NULL) && (0 == (l->flags & hidden_mask))))
	       n++;
	  }

	if (l != NULL)
	  {
	     win->current_line = l;
	     win->top_window_line = l;
	     win->line_num += n;

	     find_window_bottom (win);

	     if (n || (bot != win->bot_window_line))
	       return 0;

	     return -1;
	  }
     }

   if (nrows < 2) nrows++;
   if (0 == SLscroll_next_n (win, nrows - 1))
     return -1;
   return 0;
}

