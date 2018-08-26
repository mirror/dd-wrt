/* SLang_read_line interface --- uses SLang tty stuff */
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

typedef struct RL_History_Type
{
   struct RL_History_Type *prev, *next;
   char *buf;
   unsigned int len;
   unsigned int point;
} RL_History_Type;

/* Maximum size of display */
#define SLRL_DISPLAY_BUFFER_SIZE 4096  /* reasonable size for UTF-8 */

struct _pSLrline_Type
{
   RL_History_Type *root, *tail, *last;
   RL_History_Type *saved_line;	       /* used when browsing the history */
   char *name;			       /* the name of this object */
   unsigned char *buf;		       /* edit buffer */
   unsigned int buf_len;			       /* sizeof buffer */
   unsigned int point;			       /* current editing point */
   unsigned int tab;			       /* tab width */
   unsigned int len;			       /* current line size */

   /* display variables */
   unsigned int edit_width;		       /* length of display field */
   int curs_pos;			       /* current column */
   int start_column;		       /* column offset of display */
   unsigned int hscroll;		       /* amount to use for horiz scroll */
   SLFUTURE_CONST char *prompt;

   SLang_Key_Type last_key;	       /* last key executed by rl */

   /* These two contain an image of what is on the display.  They are
    * used by the default display handling functions. (update_hook is NULL)
    */
   unsigned char upd_buf1[SLRL_DISPLAY_BUFFER_SIZE];
   unsigned char upd_buf2[SLRL_DISPLAY_BUFFER_SIZE];
   unsigned char *old_upd, *new_upd;   /* pointers to previous two buffers */
   int new_upd_len, old_upd_len;       /* length of output buffers */
   unsigned int last_nonblank_column;  /* column of last non-blank char */

   SLKeyMap_List_Type *keymap;
   int eof_char;

   unsigned int flags;		       /*  */

   int state;
#define RLI_LINE_INVALID	0
#define RLI_LINE_SET		1
#define RLI_LINE_IN_PROGRESS	2
#define RLI_LINE_READ		3

   int is_modified;

   int quit;
#define RLINE_QUIT_DONE  1
#define RLINE_QUIT_ABORT 2

   /* tty variables */
   unsigned int (*getkey)(void);       /* getkey function -- required */
   void (*tt_goto_column)(int);
   void (*tt_insert)(char);
   void (*update_hook)(SLrline_Type *rli,
		       SLFUTURE_CONST char *prompt, SLFUTURE_CONST char *buf, unsigned int len, unsigned int point,
		       VOID_STAR client_data);
   VOID_STAR update_client_data;
   void (*update_free_update_data_hook)(SLrline_Type *rli, VOID_STAR client_data);
   void (*update_clear_hook)(SLrline_Type *rli, VOID_STAR client_data);
   void (*update_preread_hook)(SLrline_Type *rli, VOID_STAR client_data);
   void (*update_postread_hook)(SLrline_Type *rli, VOID_STAR client_data);
   void (*update_display_width_changed_hook)(SLrline_Type *rli, int width, VOID_STAR client_data);

   /* This function is only called when blinking matches */
   int (*input_pending)(int);

   SLang_Name_Type *completion_callback;
   SLang_Name_Type *list_completions_callback;
};

static SLang_Name_Type *Default_Completion_Callback;
static SLang_Name_Type *Default_List_Completions_Callback;

static unsigned char Char_Widths[256];
static void position_cursor (SLrline_Type *, int);

static void free_history_item (RL_History_Type *h)
{
   if (h == NULL)
     return;

   if (h->buf != NULL)
     SLang_create_slstring (h->buf);
   SLfree ((char *)h);
}

static void free_history (RL_History_Type *h)
{
   while (h != NULL)
     {
	RL_History_Type *next = h->next;
	free_history_item (h);
	h = next;
     }
}

static RL_History_Type *allocate_history (SLFUTURE_CONST char *str, int point)
{
   RL_History_Type *h;

   if (NULL == (h = (RL_History_Type *) SLcalloc (1, sizeof (RL_History_Type)))
       || (NULL == (h->buf = SLang_create_slstring (str))))
     {
	SLfree ((char *)h);	       /* NULL ok */
	return NULL;
     }

   h->len = strlen (str);
   if ((point < 0) || ((unsigned int)point > h->len))
     point = h->len;
   h->point = point;
   return h;
}

static void rl_beep (void)
{
   putc(7, stdout);
   fflush (stdout);
}

static int check_space (SLrline_Type *This_RLI, unsigned int dn)
{
   unsigned char *new_buf;
   unsigned int new_len;

   new_len = 1 + This_RLI->len + dn;

   if (new_len <= This_RLI->buf_len)
     return 0;

   if (NULL == (new_buf = (unsigned char *) SLrealloc ((char *)This_RLI->buf, new_len)))
     return -1;

   This_RLI->buf_len = new_len;
   This_RLI->buf = new_buf;
   return 0;
}

/* editing functions */
int SLrline_bol (SLrline_Type *This_RLI)
{
   This_RLI->point = 0;
   return 0;
}

int SLrline_eol (SLrline_Type *This_RLI)
{
   This_RLI->point = This_RLI->len;
   return 0;
}

static int rl_right (SLrline_Type *This_RLI)
{
   SLuchar_Type *s, *smax;
   int ignore_combining = 1;

   s = This_RLI->buf + This_RLI->point;
   smax = This_RLI->buf + This_RLI->len;

   if (s < smax)
     {
	if (This_RLI->flags & SL_RLINE_UTF8_MODE)
	  s = SLutf8_skip_chars (s, smax, 1, NULL, ignore_combining);
	else
	  s++;

	This_RLI->point = s - This_RLI->buf;
     }

   return 0;
}

static int rl_left (SLrline_Type *This_RLI)
{
   SLuchar_Type *s, *smin;
   int ignore_combining = 1;

   smin = This_RLI->buf;
   s = smin + This_RLI->point;

   if (s > smin)
     {
	if (This_RLI->flags & SL_RLINE_UTF8_MODE)
	  s = SLutf8_bskip_chars (smin, s, 1, NULL, ignore_combining);
	else
	  s--;

	This_RLI->point = s - This_RLI->buf;
     }

   return 0;
}

int SLrline_move (SLrline_Type *rli, int n)
{
   if (rli == NULL)
     return -1;

   if (n < 0)
     {
	n = -n;
	while (n && rli->point)
	  {
	     (void) rl_left (rli);
	     n--;
	  }
	return 0;
     }

   while (n && (rli->point != rli->len))
     {
	(void) rl_right (rli);
	n--;
     }
   return 0;
}

int SLrline_ins (SLrline_Type *This_RLI, SLFUTURE_CONST char *s, unsigned int n)
{
   unsigned char *pmin;

   if (-1 == check_space (This_RLI, n + 128))
     return -1;

   pmin = This_RLI->buf + This_RLI->point;
   if (This_RLI->len)
     {
	unsigned char *p = This_RLI->buf + This_RLI->len;
	while (p >= pmin)
	  {
	     *(p + n) = *p;
	     p--;
	  }
     }
   memcpy ((char *) pmin, s, n);

   This_RLI->len += n;
   This_RLI->point += n;
   This_RLI->is_modified = 1;

   return n;
}

static int rl_self_insert (SLrline_Type *This_RLI)
{
   char buf[8];

   buf[0] = SLang_Last_Key_Char;
   buf[1] = 0;

   return SLrline_ins (This_RLI, buf, 1);
}

int SLrline_del (SLrline_Type *This_RLI, unsigned int n)
{
   SLuchar_Type *pmax, *p, *pn;
   int ignore_combining = 1;

   p = This_RLI->buf + This_RLI->point;
   pmax = This_RLI->buf + This_RLI->len;

   if (This_RLI->flags & SL_RLINE_UTF8_MODE)
     {
	pn = SLutf8_skip_chars (p, pmax, n, NULL, ignore_combining);
	n = pn - p;
     }
   else
     {
	if (p + n > pmax) n = (pmax - p);
	pn = p + n;
     }
   This_RLI->len -= n;

   while (pn < pmax)
     {
	*p++ = *pn++;
     }
   This_RLI->is_modified = 1;
   return 0;
}

static int rl_del (SLrline_Type *This_RLI)
{
   return SLrline_del (This_RLI, 1);
}

static int rl_quote_insert (SLrline_Type *This_RLI)
{
   /* FIXME.  We should not be messing with SLang_Error here */
   int err = _pSLang_Error;
   _pSLang_Error = 0;
   SLang_Last_Key_Char = (*This_RLI->getkey)();
   rl_self_insert (This_RLI);
   if (_pSLang_Error == SL_USER_BREAK)
     {
	SLKeyBoard_Quit = 0;
	_pSLang_Error = 0;
     }
   else _pSLang_Error = err;
   return 0;
}

static int rl_trim (SLrline_Type *This_RLI)
{
   unsigned char *p, *pmax, *p1;

   p = This_RLI->buf + This_RLI->point;
   pmax = This_RLI->buf + This_RLI->len;

   if (p == pmax)
     {
	if (p == This_RLI->buf) return 0;
	p--;
     }

   if ((*p != ' ') && (*p != '\t')) return 0;
   p1 = p;
   while ((p1 < pmax) && ((*p1 == ' ') || (*p1 == '\t'))) p1++;
   pmax = p1;
   p1 = This_RLI->buf;

   while ((p >= p1) && ((*p == ' ') || (*p == '\t'))) p--;
   if (p == pmax) return 0;
   p++;

   This_RLI->point = (int) (p - p1);
   return SLrline_del (This_RLI, (int) (pmax - p));
}

static int rl_bdel (SLrline_Type *This_RLI)
{
   if (This_RLI->point)
     {
	rl_left (This_RLI);
	rl_del(This_RLI);
     }
   return 0;
}

static int rl_delbol (SLrline_Type *This_RLI)
{
   while (This_RLI->point)
     {
	rl_left (This_RLI);
	rl_del(This_RLI);
     }
   return 0;
}

static int rl_deleol (SLrline_Type *This_RLI)
{
   if (This_RLI->point == This_RLI->len) return 0;
   *(This_RLI->buf + This_RLI->point) = 0;
   This_RLI->len = This_RLI->point;
   This_RLI->is_modified = 1;
   return 0;
}

#if 0
static int rl_delete_line (SLrline_Type *This_RLI)
{
   (void) SLrline_bol (This_RLI);
   rl_deleol (This_RLI);
   return 0;
}
#endif

static int rl_enter (SLrline_Type *This_RLI)
{
   if (-1 == check_space (This_RLI, 1))
     return -1;

   *(This_RLI->buf + This_RLI->len) = 0;
   This_RLI->quit = RLINE_QUIT_DONE;
   return 0;
}

static int rl_complete (SLrline_Type *rli)
{
   char *line;
   unsigned int i, n, nbytes;
   char **strings, *str0, ch0;
   int start_point, delta;
   SLang_Array_Type *at;
   SLang_Name_Type *completion_callback;
   SLang_Name_Type *list_completions_callback;

   if (NULL == (completion_callback = rli->completion_callback))
     {
	completion_callback = Default_Completion_Callback;
	if (completion_callback == NULL)
	  return SLrline_ins (rli, "\t", 1);
     }
   if (NULL == (list_completions_callback = rli->list_completions_callback))
     list_completions_callback = Default_List_Completions_Callback;

   if (NULL == (line = SLrline_get_line (rli)))
     return -1;

   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_push_string (line))
       || (-1 == SLang_push_int (rli->point))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (completion_callback)))
     {
	SLfree (line);
	return -1;
     }

   SLfree (line);

   if (-1 == SLang_pop_int (&start_point))
     return -1;

   if (start_point < 0)
     start_point = 0;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return -1;

   strings = (char **) at->data;
   n = at->num_elements;

   if (n == 0)
     {
	SLang_free_array (at);
	return 0;
     }

   if ((n != 1) && (list_completions_callback != NULL))
     {
	if ((-1 == SLang_start_arg_list ())
	    || (-1 == SLang_push_array (at, 0))
	    || (-1 == SLang_end_arg_list ())
	    || (-1 == SLexecute_function (list_completions_callback)))
	  {
	     SLang_free_array (at);
	     return -1;
	  }
	(void) SLrline_redraw (rli);
     }

   str0 = strings[0];
   nbytes = 0;
   while (0 != (ch0 = str0[nbytes]))
     {
	for (i = 1; i < n; i++)
	  {
	     char ch1 = strings[i][nbytes];
	     if (ch0 != ch1)
	       break;
	  }
	if (i != n)
	  break;
	nbytes++;
     }

   delta = start_point - rli->point;
   if (delta < 0)
     {
	(void) SLrline_move (rli, delta);
	delta = -delta;
     }
   (void) SLrline_del (rli, (unsigned int) delta);
   (void) SLrline_ins (rli, str0, nbytes);

   /* How should the completion be ended?
    *   "foo/     -->  "foo/
    *   "foo/bar  -->  "foo/bar"
    *   "foo      -->  "foo"
    *   foo       -->  fooSPACE
    *   foo/bar   -->  fooSPACE
    */
   if ((n == 1)
       && nbytes && (str0[nbytes-1] != '/') && (str0[nbytes-1] != '\\'))
     {
	char qch = ' ';

	if (start_point > 0)
	  {
	     ch0 = rli->buf[start_point-1];
	     if ((ch0 == '"') || (ch0 == '\''))
	       qch = ch0;
	  }
	if (qch != 0)
	  (void) SLrline_ins (rli, &qch, 1);
     }

   SLang_free_array (at);
   return 0;
}

static SLKeyMap_List_Type *RL_Keymap;

static SLuchar_Type *compute_char_width (SLuchar_Type *b, SLuchar_Type *bmax, int utf8_mode, unsigned int *wp, SLwchar_Type *wchp, int *illegalp)
{
   SLwchar_Type wch;

   if (illegalp != NULL) *illegalp = 0;

   if (b >= bmax)
     {
	*wp = 0;
	if (wchp != NULL) *wchp = 0;
	return b;
     }

   if (utf8_mode == 0)
     {
	*wp = Char_Widths[*b];
	if (wchp != NULL) *wchp = *b;
	return b + 1;
     }

   if (NULL == SLutf8_decode (b, bmax, &wch, NULL))
     {
	/* Illegal byte sequence */
	*wp = 4;   /* <XX> */
	if (wchp != NULL) *wchp = *b;
	if (illegalp != NULL) *illegalp = 1;
	return b + 1;
     }

   /* skip combining */
   if ((wch >= ' ') && (wch < 127))
     *wp = 1;
   else if (wch > 127)
     *wp = SLwchar_wcwidth (wch);
   else
     *wp = 2;			       /* ^X */

   if (wchp != NULL) *wchp = wch;
   /* skip combining chars too */
   return SLutf8_skip_chars (b, bmax, 1, NULL, 1);
}

/* This update is designed for dumb terminals.  It assumes only that the
 * terminal can backspace via ^H, and move cursor to start of line via ^M.
 * There is a hook so the user can provide a more sophisticated update if
 * necessary.
 */
static void position_cursor (SLrline_Type *This_RLI, int col)
{
   unsigned char *p, *pmax;
   unsigned int len, dlen;
   int curs_pos;
   int dc;
   int utf8_mode = This_RLI->flags & SL_RLINE_UTF8_MODE;

   if (col == This_RLI->curs_pos)
     {
	fflush (stdout);
	return;
     }

   if (This_RLI->tt_goto_column != NULL)
     {
	(*This_RLI->tt_goto_column)(col);
	This_RLI->curs_pos = col;
	fflush (stdout);
	return;
     }

   curs_pos = This_RLI->curs_pos;
   dc = This_RLI->curs_pos - col;
   if (dc < 0)
     {
	p = This_RLI->new_upd;
	pmax = p + SLRL_DISPLAY_BUFFER_SIZE;

	len = 0;
	while (((int)len < curs_pos) && (p < pmax))
	  {
	     p = compute_char_width (p, pmax, utf8_mode, &dlen, NULL, NULL);
	     len += dlen;
	  }
	while (((int)len < col) && (p < pmax))
	  {
	     SLuchar_Type *p1;
	     p1 = compute_char_width (p, pmax, utf8_mode, &dlen, NULL, NULL);
	     while (p < p1)
	       putc((char) *p++, stdout);

	     len += dlen;
	  }
     }
   else
     {
	if (dc < col)
	  {
	     while (dc--) putc(8, stdout);
	  }
	else
	  {
	     putc('\r', stdout);
	     p = This_RLI->new_upd;
	     pmax = p + SLRL_DISPLAY_BUFFER_SIZE;
	     len = 0;
	     while (((int)len < col) && (p < pmax))
	       {
		  SLuchar_Type *p1;
		  p1 = compute_char_width (p, pmax, utf8_mode, &dlen, NULL, NULL);
		  while (p < p1)
		    putc((char) *p++, stdout);
		  len += dlen;
	       }
	  }
     }
   This_RLI->curs_pos = col;
   fflush (stdout);
}

static void erase_eol (SLrline_Type *rli)
{
   int col = rli->curs_pos;
   int col_max = rli->last_nonblank_column;

   while (col < col_max)
     {
	putc(' ', stdout);
	col++;
     }
   rli->curs_pos = col_max;
}

static void spit_out(SLrline_Type *rli, SLuchar_Type *p, SLuchar_Type *pmax, int col)
{
   int utf8_mode = rli->flags & SL_RLINE_UTF8_MODE;
   position_cursor (rli, col);
   while (p < pmax)
     {
	SLuchar_Type *p1;
	unsigned int dcol;
	p1 = compute_char_width (p, pmax, utf8_mode, &dcol, NULL, NULL);
	while (p < p1)
	  putc((char) *p++, stdout);
	col += dcol;
     }
   rli->curs_pos = col;
}

static void really_update (SLrline_Type *rli, int new_curs_position)
{
   SLuchar_Type *b, *bmax, *p, *pmax;
   unsigned int col, max_col;
   int utf8_mode = rli->flags & SL_RLINE_UTF8_MODE;

   col = 0;
   max_col = rli->edit_width-1;
   b = rli->old_upd;
   p = rli->new_upd;
   bmax = b + rli->old_upd_len;
   pmax = p + rli->new_upd_len;

   while (col < max_col)
     {
	SLwchar_Type pch, bch;
	SLuchar_Type *p1, *b1;
	unsigned int plen, blen;

	b1 = compute_char_width (b, bmax, utf8_mode, &blen, &bch, NULL);
	p1 = compute_char_width (p, pmax, utf8_mode, &plen, &pch, NULL);

	if ((p1 != p) && ((b1-b) == (p1-p))
	    && (bch == pch))
	  {
	     col += plen;
	     b = b1;
	     p = p1;
	     continue;
	  }

	spit_out (rli, p, pmax, col);

	col = rli->curs_pos;
	if (col < rli->last_nonblank_column)
	  erase_eol (rli);
	rli->last_nonblank_column = col;

	break;
     }

   position_cursor (rli, new_curs_position);

   /* update finished, so swap */

   rli->old_upd_len = rli->new_upd_len;
   p = rli->old_upd;
   rli->old_upd = rli->new_upd;
   rli->new_upd = p;
}

static SLuchar_Type *
  compute_tabbed_char_width (SLuchar_Type *b, SLuchar_Type *bmax,
			     int utf8_mode, int col, int tab_width,
			     unsigned int *dlenp)
{
   if (b >= bmax)
     {
	*dlenp = 0;
	return bmax;
     }

   if ((*b == '\t') && tab_width)
     {
	*dlenp = tab_width * (col / tab_width + 1) - col;
	return b+1;
     }

   return compute_char_width (b, bmax, utf8_mode, dlenp, NULL, NULL);
}

static unsigned int compute_string_width (SLrline_Type *rli, SLuchar_Type *b, SLuchar_Type *bmax, unsigned int tab_width)
{
   int utf8_mode = rli->flags & SL_RLINE_UTF8_MODE;
   unsigned int len;

   if (b == NULL)
     return 0;

   len = 0;
   while (b < bmax)
     {
	unsigned int dlen;

	if ((*b == '\t') && tab_width)
	  {
	     dlen = tab_width * (len / tab_width + 1) - len;
	     b++;
	  }
	else b = compute_char_width (b, bmax, utf8_mode, &dlen, NULL, NULL);

	len += dlen;
     }
   return len;
}

static void RLupdate (SLrline_Type *rli)
{
   unsigned int len, dlen, prompt_len = 0, tw = 0, count;
   unsigned int start_len;
   int want_cursor_pos;
   unsigned char *b, *bmax, *b_point, *p, *pmax;
   int no_echo;
   int utf8_mode;
   unsigned int edit_width;

   no_echo = rli->flags & SL_RLINE_NO_ECHO;
   utf8_mode = rli->flags & SL_RLINE_UTF8_MODE;
   edit_width = rli->edit_width-1;

   *(rli->buf + rli->len) = 0;

   if (rli->update_hook != NULL)
     {
	if (no_echo)
	  (*rli->update_hook) (rli, rli->prompt, "", 0, 0, rli->update_client_data);
	else
	  (*rli->update_hook) (rli, rli->prompt, (char *)rli->buf, rli->len, rli->point, rli->update_client_data);
	return;
     }

   /* expand characters for output buffer --- handle prompt first.
    * Do two passes --- first to find out where to begin upon horiz
    * scroll and the second to actually fill the buffer. */
   len = 0;
   b = (unsigned char *) rli->prompt;
   if (b != NULL)
     {
	bmax = b + strlen ((char *)b);
	len += compute_string_width (rli, b, bmax, 0);
     }
   prompt_len = len;

   b = (unsigned char *) rli->buf;
   b_point = (unsigned char *) (rli->buf + rli->point);
   if (no_echo == 0)
     len += compute_string_width (rli, b, b_point, rli->tab);

   if (len + rli->hscroll < edit_width) start_len = 0;
   else if ((rli->start_column > (int)len)
	    || (rli->start_column + (int)edit_width <= (int)len))
     start_len = (len + rli->hscroll) - edit_width;
   else start_len = rli->start_column;
   rli->start_column = start_len;

   /* want_cursor_pos = len - start_len; */

   /* second pass */
   b = (unsigned char *) rli->prompt;
   if (b == NULL) b = (unsigned char *) "";
   bmax = b + strlen ((char *)b);
   len = 0;
   count = 2;
   while ((len < start_len) && (b < bmax))
     {
	b = compute_tabbed_char_width (b, bmax, utf8_mode, 0, 0, &dlen);
	len += dlen;
     }

   tw = 0;
   if (b == bmax)
     {
	b = (unsigned char *) rli->buf;
	bmax = b + strlen ((char *)b);
	tw = rli->tab;
	while ((len < start_len) && (b < bmax))
	  {
	     b = compute_tabbed_char_width (b, bmax, utf8_mode, 0, tw, &dlen);
	     len += dlen;
	  }
	count--;
     }

   len = 0;
   p = rli->new_upd;
   pmax = p + SLRL_DISPLAY_BUFFER_SIZE;
   want_cursor_pos = -1;

   while (count--)
     {
	if ((count == 0) && (no_echo))
	  break;

	while ((len < edit_width) && (b < bmax))
	  {
	     SLwchar_Type wch;
	     int is_illegal;
	     SLuchar_Type *b1;

	     if (b == b_point)
	       want_cursor_pos = len;

	     if ((*b == '\t') && tw)
	       {
		  dlen = tw * ((len + start_len - prompt_len) / tw + 1) - (len + start_len - prompt_len);
		  len += dlen;	       /* ok since dlen comes out 0  */
		  if (len > edit_width) dlen = len - edit_width;
		  while (dlen-- && (p < pmax)) *p++ = ' ';
		  b++;
		  continue;
	       }

	     b1 = compute_char_width (b, bmax, utf8_mode, &dlen, &wch, &is_illegal);
	     if (len + dlen > edit_width)
	       {
		  /* The character is double width and a portion of it exceeds
		   * the edit width
		   */
		  break;
	       }

	     if (is_illegal == 0)
	       {
		  if (wch < 32)
		    {
		       if (p < pmax) *p++ = '^';
		       if (p < pmax) *p++ = *b + '@';
		    }
		  else if (wch == 127)
		    {
		       if (p < pmax) *p++ = '^';
		       if (p < pmax) *p++ = '?';
		    }
		  else while (b < b1)
		    {
		       if (p < pmax) *p++ = *b++;
		    }
	       }
	     else
	       {
		  if (p + 4 < pmax)
		    {
		       sprintf ((char *)p, "<%02X>", *b);
		       p += 4;
		    }
	       }
	     b = b1;
	     len += dlen;
	  }
	/* if (start_len > prompt_len) break; */
	tw = rli->tab;
	b = (unsigned char *) rli->buf;
	bmax = b + strlen ((char *)b);
     }

   if (want_cursor_pos == -1)
     want_cursor_pos = len;

   rli->new_upd_len = (int) (p - rli->new_upd);
   while ((p < pmax) && (len < edit_width))
     {
	*p++ = ' ';
	len++;
     }
   really_update (rli, want_cursor_pos);
}

void SLrline_redraw (SLrline_Type *rli)
{
   if (rli == NULL)
     return;

   if (rli->update_hook != NULL)
     {
	(*rli->update_hook) (rli, "", "", 0, 0, rli->update_client_data);
     }
   else
     {
	unsigned char *p;
	unsigned char *pmax;

	p = rli->new_upd;
	pmax = p + rli->edit_width;
	while (p < pmax) *p++ = ' ';
	rli->new_upd_len = rli->edit_width;
	rli->last_nonblank_column = rli->edit_width-1;
	really_update (rli, 0);
	rli->last_nonblank_column = 0;
     }
   RLupdate (rli);
}

static int rl_eof_insert (SLrline_Type *This_RLI)
{
   return rl_enter (This_RLI);
}

/* This is very naive.  It knows very little about nesting and nothing
 * about quoting.
 */
static void blink_match (SLrline_Type *rli)
{
   unsigned char bra, ket;
   unsigned int delta_column;
   unsigned char *p, *pmin;
   int dq_level, sq_level;
   int level;

   pmin = rli->buf;
   p = pmin + rli->point;
   if (pmin == p)
     return;

   ket = SLang_Last_Key_Char;
   switch (ket)
     {
      case ')':
	bra = '(';
	break;
      case ']':
	bra = '[';
	break;
      case '}':
	bra = '{';
	break;
      default:
	return;
     }

   level = 0;
   sq_level = dq_level = 0;

   delta_column = 0;
   while (p > pmin)
     {
	char ch;

	p--;
	delta_column++;
	ch = *p;

	if (ch == ket)
	  {
	     if ((dq_level == 0) && (sq_level == 0))
	       level++;
	  }
	else if (ch == bra)
	  {
	     if ((dq_level != 0) || (sq_level != 0))
	       continue;

	     level--;
	     if (level == 0)
	       {
		  rli->point -= delta_column;
		  RLupdate (rli);
		  if (rli->input_pending != NULL)
		    (*rli->input_pending)(10);
		  rli->point += delta_column;
		  RLupdate (rli);
		  break;
	       }
	     if (level < 0)
	       break;
	  }
	else if (ch == '"') dq_level = !dq_level;
	else if (ch == '\'') sq_level = !sq_level;
     }
}

static void free_last_key (SLrline_Type *rli)
{
   SLang_Key_Type *last_key;

   if (rli == NULL)
     return;

   last_key = &rli->last_key;
   switch (last_key->type)
     {
      case SLKEY_F_INTERPRET:
	if (last_key->f.s != NULL)
	  {
	     SLang_free_slstring (last_key->f.s);
	     last_key->f.s = NULL;
	  }
	break;

      case SLKEY_F_SLANG:
	if (NULL != last_key->f.slang_fun)
	  {
	     SLang_free_function (last_key->f.slang_fun);
	     last_key->f.slang_fun = NULL;
	  }
	break;
     }

   last_key->type = 0;
}

static int save_last_key (SLrline_Type *rli, SLang_Key_Type *key)
{
   SLang_Key_Type *last_key;

   if ((rli == NULL) || (key == NULL))
     return 0;

   free_last_key (rli);

   last_key = &rli->last_key;
   last_key->type = 0;

   switch (key->type)
     {
      case SLKEY_F_INTERPRET:
	if (NULL == (last_key->f.s = SLang_create_slstring (key->f.s)))
	  return -1;
	break;

      case SLKEY_F_KEYSYM:
	last_key->f.keysym = key->f.keysym;
	break;

      case SLKEY_F_SLANG:
	if (NULL == (last_key->f.slang_fun = SLang_copy_function (key->f.slang_fun)))
	  return -1;
	break;

      default:
      case SLKEY_F_INTRINSIC:
	last_key->f.f = key->f.f;
     }

   last_key->type = key->type;
   memcpy (last_key->str, key->str, sizeof(key->str));
   return 0;
}

static SLrline_Type *Active_Rline_Info = NULL;

char *SLrline_read_line (SLrline_Type *rli, SLFUTURE_CONST char *prompt, unsigned int *lenp)
{
   unsigned char *p, *pmax;
   SLang_Key_Type *key;
   int last_input_char;
   unsigned int dummy_len_buf;

   if (lenp == NULL)
     lenp = &dummy_len_buf;

   *lenp = 0;

   if (rli == NULL)
     return NULL;

   if (rli->state == RLI_LINE_IN_PROGRESS)
     {
	*lenp = 0;
	return NULL;
     }

   if (prompt == NULL)
     prompt = "";

   if ((rli->prompt == NULL)
       || strcmp (rli->prompt, prompt))
     {
	if (NULL == (prompt = SLmake_string (prompt)))
	  return NULL;

	SLfree ((char *)rli->prompt);
	rli->prompt = prompt;
     }

   rli->quit = 0;
   p = rli->old_upd; pmax = p + rli->edit_width;
   while (p < pmax) *p++ = ' ';

   if (rli->state != RLI_LINE_SET)
     {
	rli->len = 0;
	rli->point = 0;
	*rli->buf = 0;
     }
   rli->state = RLI_LINE_IN_PROGRESS;

   if (rli->update_preread_hook != NULL)
     (*rli->update_preread_hook)(rli, rli->update_client_data);

   rli->curs_pos = rli->start_column = 0;
   rli->new_upd_len = rli->old_upd_len = 0;

   free_last_key (rli);
   if (rli->update_hook == NULL)
     putc ('\r', stdout);

   rli->is_modified = 0;
   rli->last = NULL;

   RLupdate (rli);

   last_input_char = 0;
   while (1)
     {
	SLrline_Type *save_rli = Active_Rline_Info;

	key = SLang_do_key (RL_Keymap, (int (*)(void)) rli->getkey);

	if ((key == NULL) || (key->f.f == NULL))
	  {
	     rl_beep ();
	     continue;
	  }

	if ((*key->str != 2) || (key->str[1] != rli->eof_char))
	  last_input_char = 0;
	else
	  {
	     if ((rli->len == 0) && (last_input_char != rli->eof_char))
	       {
		  rli->buf[rli->len] = 0;
		  rli->state = RLI_LINE_READ;
		  *lenp = 0;

		  if (rli->update_postread_hook != NULL)
		    (*rli->update_postread_hook)(rli, rli->update_client_data);

		  return NULL;	       /* EOF */
	       }

	     last_input_char = rli->eof_char;
	  }

	Active_Rline_Info = rli;
	if (key->type == SLKEY_F_INTRINSIC)
	  {
	     int (*func)(SLrline_Type *);
	     func = (int (*)(SLrline_Type *)) key->f.f;

	     (void) (*func)(rli);

	     RLupdate (rli);

	     if ((rli->flags & SL_RLINE_BLINK_MATCH)
		 && (rli->input_pending != NULL))
	       blink_match (rli);
	  }
	else if (key->type == SLKEY_F_SLANG)
	  {
	     (void) SLexecute_function (key->f.slang_fun);
	     RLupdate (rli);
	  }
	Active_Rline_Info = save_rli;

	if (_pSLang_Error) rli->quit = RLINE_QUIT_ABORT;

	if (rli->quit)
	  {
	     if (rli->quit == RLINE_QUIT_ABORT)
	       {
		  rli->len = 0;
	       }
	     rli->buf[rli->len] = 0;
	     rli->state = RLI_LINE_READ;
	     *lenp = rli->len;

	     free_history_item (rli->saved_line);
	     rli->saved_line = NULL;

	     if (rli->update_postread_hook != NULL)
	       (*rli->update_postread_hook)(rli, rli->update_client_data);

	     if (rli->quit == RLINE_QUIT_ABORT)
	       {
		  return NULL;
	       }

	     return SLmake_nstring ((char *)rli->buf, rli->len);
	  }
	if (key != NULL)
	  (void) save_last_key (rli, key);
     }
}

static int rl_abort (SLrline_Type *This_RLI)
{
   This_RLI->quit = RLINE_QUIT_ABORT;
   return 0;
}

/* TTY interface --- ANSI */

static void ansi_goto_column (int n)
{
   putc('\r', stdout);
   if (n) fprintf(stdout, "\033[%dC", n);
}

static int rl_select_line (SLrline_Type *rli, RL_History_Type *p)
{
   unsigned int len;

   len = p->len;
   if (-1 == check_space (rli, len))
     return -1;

   rli->last = p;
   strcpy ((char *) rli->buf, p->buf);
   rli->point = p->point;
   rli->len = len;
   rli->is_modified = 0;
   return 0;
}

static int rl_redraw (SLrline_Type *This_RLI)
{
   SLrline_redraw (This_RLI);
   return 0;
}

static int rl_prev_line (SLrline_Type *This_RLI)
{
   RL_History_Type *prev;

   if ((This_RLI->is_modified)
       || (This_RLI->last == NULL))
     {
	prev = This_RLI->tail;
     }
   else
     prev = This_RLI->last->prev;

   if (prev == NULL)
     {
	rl_beep ();
	return 0;
     }

   if (prev == This_RLI->tail)
     {
	This_RLI->buf[This_RLI->len] = 0;
	free_history_item (This_RLI->saved_line);
	This_RLI->saved_line = allocate_history ((char *)This_RLI->buf, This_RLI->point);
	if (This_RLI->saved_line == NULL)
	  return -1;
     }

   return rl_select_line (This_RLI, prev);
}

static int rl_next_line (SLrline_Type *This_RLI)
{
   RL_History_Type *next;

   if ((This_RLI->is_modified)
       || (This_RLI->last == NULL))
     {
	rl_beep ();
	return 0;
     }

   next = This_RLI->last->next;

   if (next == NULL)
     {
	int status = 0;

	if (This_RLI->saved_line != NULL)
	  {
	     status = rl_select_line (This_RLI, This_RLI->saved_line);
	     free_history_item (This_RLI->saved_line);
	     This_RLI->saved_line = NULL;
	     This_RLI->is_modified = 1;
	     if (status == 0)
	       return 0;
	  }
	This_RLI->len = This_RLI->point = 0;
	*This_RLI->buf = 0;
	This_RLI->last = NULL;
	This_RLI->is_modified = 0;
	return status;
     }

   return rl_select_line (This_RLI, next);
}

#define AKEY(name,func) {name,(int (*)(void))func}

static SLKeymap_Function_Type SLReadLine_Functions[] =
{
   AKEY("kbd_quit",rl_abort),
   AKEY("redraw",rl_redraw),
   AKEY("up", rl_prev_line),
   AKEY("down", rl_next_line),
   AKEY("bol", SLrline_bol),
   AKEY("eol", SLrline_eol),
   AKEY("right", rl_right),
   AKEY("left", rl_left),
   AKEY("self_insert", rl_self_insert),
   AKEY("bdel", rl_bdel),
   AKEY("del", rl_del),
   AKEY("deleol", rl_deleol),
   AKEY("delbol", rl_delbol),
   AKEY("enter", rl_enter),
   AKEY("trim", rl_trim),
   AKEY("quoted_insert", rl_quote_insert),
   AKEY("complete", rl_complete),
   AKEY(NULL, NULL),
};

void SLrline_close (SLrline_Type *rli)
{
   if (rli == NULL)
     return;

   if (rli->name != NULL)
     {
	char hookname[1024];
	SLrline_Type *arli = Active_Rline_Info;
	Active_Rline_Info = rli;
	SLsnprintf (hookname, sizeof(hookname), "%s_rline_close_hook", rli->name);
	if (0 == SLang_run_hooks (hookname, 0))
	  (void) SLang_run_hooks ("rline_close_hook", 1, rli->name);
	Active_Rline_Info = arli;
	SLang_free_slstring (rli->name);
     }

   if ((rli->update_free_update_data_hook != NULL)
       && (rli->update_client_data != NULL))
     (*rli->update_free_update_data_hook)(rli, rli->update_client_data);

   free_last_key (rli);
   free_history (rli->root);
   free_history_item (rli->saved_line);
   SLang_free_function (rli->list_completions_callback);
   SLang_free_function (rli->completion_callback);
   SLfree ((char *)rli->prompt);
   SLfree ((char *)rli->buf);
   SLfree ((char *)rli);
}

static int init_keymap (void)
{
   int ch;
   char simple[2];
   SLkeymap_Type *km;

   if (RL_Keymap != NULL)
     return 0;

   simple[1] = 0;
   if (NULL == (km = SLang_create_keymap ("ReadLine", NULL)))
     return -1;

   km->functions = SLReadLine_Functions;

   /* This breaks under some DEC ALPHA compilers (scary!) */
#ifndef __DECC
   for (ch = ' '; ch < 256; ch++)
     {
	simple[0] = (char) ch;
	SLkm_define_key (simple, (FVOID_STAR) rl_self_insert, km);
     }
#else
   ch = ' ';
   while (1)
     {
	simple[0] = (char) ch;
	SLkm_define_key (simple, (FVOID_STAR) rl_self_insert, km);
	ch = ch + 1;
	if (ch == 256) break;
     }
#endif				       /* NOT __DECC */

   simple[0] = SLang_Abort_Char;
   SLkm_define_key (simple, (FVOID_STAR) rl_abort, km);
#ifdef REAL_UNIX_SYSTEM
   simple[0] = (char) 4;
#else
   simple[0] = (char) 26;
#endif
   SLkm_define_key (simple, (FVOID_STAR) rl_eof_insert, km);

#ifndef IBMPC_SYSTEM
   SLkm_define_key  ("^[[A", (FVOID_STAR) rl_prev_line, km);
   SLkm_define_key  ("^[[B", (FVOID_STAR) rl_next_line, km);
   SLkm_define_key  ("^[[C", (FVOID_STAR) rl_right, km);
   SLkm_define_key  ("^[[D", (FVOID_STAR) rl_left, km);
   SLkm_define_key  ("^[OA", (FVOID_STAR) rl_prev_line, km);
   SLkm_define_key  ("^[OB", (FVOID_STAR) rl_next_line, km);
   SLkm_define_key  ("^[OC", (FVOID_STAR) rl_right, km);
   SLkm_define_key  ("^[OD", (FVOID_STAR) rl_left, km);
#else
   SLkm_define_key  ("^@H", (FVOID_STAR) rl_prev_line, km);
   SLkm_define_key  ("^@P", (FVOID_STAR) rl_next_line, km);
   SLkm_define_key  ("^@M", (FVOID_STAR) rl_right, km);
   SLkm_define_key  ("^@K", (FVOID_STAR) rl_left, km);
   SLkm_define_key  ("^@S", (FVOID_STAR) rl_del, km);
   SLkm_define_key  ("^@O", (FVOID_STAR) SLrline_eol, km);
   SLkm_define_key  ("^@G", (FVOID_STAR) SLrline_bol, km);

   SLkm_define_key  ("\xE0H", (FVOID_STAR) rl_prev_line, km);
   SLkm_define_key  ("\xE0P", (FVOID_STAR) rl_next_line, km);
   SLkm_define_key  ("\xE0M", (FVOID_STAR) rl_right, km);
   SLkm_define_key  ("\xE0K", (FVOID_STAR) rl_left, km);
   SLkm_define_key  ("\xE0S", (FVOID_STAR) rl_del, km);
   SLkm_define_key  ("\xE0O", (FVOID_STAR) SLrline_eol, km);
   SLkm_define_key  ("\xE0G", (FVOID_STAR) SLrline_bol, km);
#endif
   SLkm_define_key  ("^C", (FVOID_STAR) rl_abort, km);
   SLkm_define_key  ("^E", (FVOID_STAR) SLrline_eol, km);
   SLkm_define_key  ("^G", (FVOID_STAR) rl_abort, km);
   SLkm_define_key  ("^I", (FVOID_STAR) rl_complete, km);
   SLkm_define_key  ("^A", (FVOID_STAR) SLrline_bol, km);
   SLkm_define_key  ("\r", (FVOID_STAR) rl_enter, km);
   SLkm_define_key  ("\n", (FVOID_STAR) rl_enter, km);
   SLkm_define_key  ("^K", (FVOID_STAR) rl_deleol, km);
   SLkm_define_key  ("^L", (FVOID_STAR) rl_deleol, km);
   SLkm_define_key  ("^U", (FVOID_STAR) rl_delbol, km);
   SLkm_define_key  ("^V", (FVOID_STAR) rl_del, km);
   SLkm_define_key  ("^D", (FVOID_STAR) rl_del, km);
   SLkm_define_key  ("^F", (FVOID_STAR) rl_right, km);
   SLkm_define_key  ("^B", (FVOID_STAR) rl_left, km);
   SLkm_define_key  ("^?", (FVOID_STAR) rl_bdel, km);
   SLkm_define_key  ("^H", (FVOID_STAR) rl_bdel, km);
   SLkm_define_key  ("^P", (FVOID_STAR) rl_prev_line, km);
   SLkm_define_key  ("^N", (FVOID_STAR) rl_next_line, km);
   SLkm_define_key  ("^R", (FVOID_STAR) rl_redraw, km);
   SLkm_define_key  ("`", (FVOID_STAR) rl_quote_insert, km);
   SLkm_define_key  ("\033\\", (FVOID_STAR) rl_trim, km);
   if (_pSLang_Error)
     return -1;
   RL_Keymap = km;
   return 0;
}

#if SLANG_HAS_MULTILINE_RLINE
typedef struct
{
   int max_row;
   int max_col;
   int num_screen_cols;
   int num_screen_rows;
}
RLine_SMG_Update_Type;

static RLine_SMG_Update_Type *alloc_smg_update_data (void)
{
   return (RLine_SMG_Update_Type *)SLcalloc (1, sizeof(RLine_SMG_Update_Type));
}

static void free_smg_update_data (SLrline_Type *rli, VOID_STAR cd)
{
   (void) rli;
   if (cd != NULL)
     SLfree ((char *)cd);
}

static int check_window_size_and_redraw (SLrline_Type *rli, RLine_SMG_Update_Type *s)
{
   if ((s->num_screen_cols != SLtt_Screen_Cols)
       || (s->num_screen_rows != SLtt_Screen_Rows))
     {
	SLsmg_reinit_smg ();
	s->num_screen_cols = SLtt_Screen_Cols;
	s->num_screen_rows = SLtt_Screen_Rows;
	SLrline_redraw (rli);
	return 1;
     }
   return 0;
}

static void rline_smg_update (SLrline_Type *rli, SLFUTURE_CONST char *prompt,
			      SLFUTURE_CONST char *buf, unsigned int buflen,
			      unsigned int point, VOID_STAR cd)
{
   int r0, c0, r1, c1;
   RLine_SMG_Update_Type *s = (RLine_SMG_Update_Type *) cd;
   (void) rli;

   if (check_window_size_and_redraw (rli, s))
     return;

   if ((*buf == 0) && (*prompt == 0))
     _pSLtt_cmdline_mode_reset ();

   SLsmg_gotorc (0, 0);
   SLsmg_write_string (prompt);
   r0 = SLsmg_get_row ();
   c0 = SLsmg_get_column ();
   SLsmg_write_nchars (buf, buflen);
   r1 = SLsmg_get_row ();
   c1 = SLsmg_get_column ();
   if (r1 <= s->max_row)
     {
	int r = r1, c = c1;
	while (r <= s->max_row)
	  {
	     SLsmg_gotorc (r, c);
	     SLsmg_erase_eol ();
	     c = 0;
	     r++;
	  }
     }
   s->max_row = r1;
   s->max_col = c1;
   SLsmg_gotorc (r0, c0);
   SLsmg_write_nchars (buf, point);
   SLsmg_refresh ();
}

static void rline_smg_preread (SLrline_Type *rli, VOID_STAR cd)
{
   RLine_SMG_Update_Type *s = (RLine_SMG_Update_Type *) cd;

   (void) rli;
   _pSLtt_cmdline_mode_reset ();
   SLsmg_gotorc (0,0);
   SLsmg_erase_eos ();
   s->max_row = 0;
   s->max_col = 0;
   SLsmg_refresh ();
}

static void rline_smg_postread (SLrline_Type *rli, VOID_STAR cd)
{
   RLine_SMG_Update_Type *s = (RLine_SMG_Update_Type *) cd;

   (void) rli;
   SLsmg_gotorc (s->max_row, s->max_col);
   SLsmg_refresh ();
}

static void rline_smg_clear (SLrline_Type *rli, VOID_STAR cd)
{
   RLine_SMG_Update_Type *s = (RLine_SMG_Update_Type *) cd;

   (void) rli;
   SLsmg_gotorc (0,0);
   SLsmg_cls ();
   s->max_row = 0;
   s->max_col = 0;
}

static void rline_smg_display_width_changed (SLrline_Type *rli, int w, VOID_STAR cd)
{
   RLine_SMG_Update_Type *s = (RLine_SMG_Update_Type *) cd;
   (void) w;

   SLtt_get_screen_size ();
   (void) check_window_size_and_redraw (rli, s);
}

static int try_smg_multiline_mode (SLrline_Type *rli)
{
   int status;
   RLine_SMG_Update_Type *cd;

   status = _pSLtt_init_cmdline_mode ();
   if (status <= 0)
     return status;

   if (NULL == (cd = alloc_smg_update_data ()))
     return -1;

   (void) SLrline_set_update_hook (rli, rline_smg_update, cd);
   rli->update_free_update_data_hook = free_smg_update_data;
   rli->update_clear_hook = rline_smg_clear;
   rli->update_preread_hook = rline_smg_preread;
   rli->update_postread_hook = rline_smg_postread;
   rli->update_display_width_changed_hook = rline_smg_display_width_changed;

   cd->num_screen_cols = SLtt_Screen_Cols;
   cd->num_screen_rows = SLtt_Screen_Rows;

   if (-1 == _pSLsmg_init_smg_cmdline ())
     return -1;

   return 1;
}
#endif				       /* SLANG_HAS_MULTILINE_RLINE */

SLrline_Type *SLrline_open (unsigned int width, unsigned int flags)
{
   SLrline_Type *rli;

   if (_pSLutf8_mode)
     flags |= SL_RLINE_UTF8_MODE;

   if (NULL == (rli = (SLrline_Type *)SLcalloc (1, sizeof (SLrline_Type))))
     return NULL;

   if (width == 0)
     width = 80;

   if (width < 256) rli->buf_len = 256;
   else rli->buf_len = width;

   if (NULL == (rli->buf = (unsigned char *)SLmalloc (rli->buf_len)))
     {
	SLrline_close (rli);
	return NULL;
     }
   *rli->buf = 0;
#ifdef REAL_UNIX_SYSTEM
   rli->eof_char = 4;
#else
   rli->eof_char = 26;
#endif

   rli->point = 0;
   rli->flags = flags;
   rli->edit_width = width;
   rli->hscroll = width/4;
   rli->tab = 8;
   rli->getkey = SLang_getkey;
   rli->input_pending = SLang_input_pending;
   rli->state = RLI_LINE_INVALID;

   if (rli->flags & SL_RLINE_USE_ANSI)
     {
	if (rli->tt_goto_column == NULL) rli->tt_goto_column = ansi_goto_column;
     }

   if (-1 == init_keymap ())
     {
	SLrline_close (rli);
	return NULL;
     }
   rli->keymap = RL_Keymap;
   rli->old_upd = rli->upd_buf1;
   rli->new_upd = rli->upd_buf2;

   if (Char_Widths[0] == 0)
     {
	int ch;
	/* FIXME: This does not support UTF-8 */
	for (ch = 0; ch < 32; ch++) Char_Widths[ch] = 2;
	for (ch = 32; ch < 256; ch++) Char_Widths[ch] = 1;
	Char_Widths[127] = 2;
#ifndef IBMPC_SYSTEM
	for (ch = 128; ch < 160; ch++) Char_Widths[ch] = 3;
#endif
     }
#if SLANG_HAS_MULTILINE_RLINE
   if (flags & SL_RLINE_USE_MULTILINE)
     {
	if (-1 == try_smg_multiline_mode (rli))
	  {
	     SLrline_close (rli);
	     return NULL;
	  }
     }
#endif
   return rli;
}

SLrline_Type *SLrline_open2 (SLFUTURE_CONST char *name, unsigned int width, unsigned int flags)
{
   SLrline_Type *rli;
   SLrline_Type *arli;
   char hookname [1024];

   if (NULL == (rli = SLrline_open (width, flags)))
     return NULL;

   if (NULL != rli->name)
     SLang_free_slstring (rli->name);
   if (NULL == (rli->name = SLang_create_slstring (name)))
     {
	SLrline_close (rli);
	return NULL;
     }

   arli = Active_Rline_Info;
   Active_Rline_Info = rli;
   SLsnprintf (hookname, sizeof(hookname), "%s_rline_open_hook", name);
   if (0 == SLang_run_hooks (hookname, 0))
     (void) SLang_run_hooks ("rline_open_hook", 1, name);
   Active_Rline_Info = arli;
   return rli;
}

int SLrline_add_to_history (SLrline_Type *rli, SLFUTURE_CONST char *hist)
{
   RL_History_Type *h;

   if ((rli == NULL) || (hist == NULL))
     return -1;

   h = allocate_history (hist, -1);

   if (rli->root == NULL)
     rli->root = h;

   if (rli->tail != NULL)
     rli->tail->next = h;

   h->prev = rli->tail;
   rli->tail = h;
   h->next = NULL;

   return 0;
}

int SLrline_save_line (SLrline_Type *rli)
{
   if (rli == NULL)
     return -1;

   return SLrline_add_to_history (rli, (char *) rli->buf);
}

SLkeymap_Type *SLrline_get_keymap (SLrline_Type *rli)
{
   if (rli == NULL)
     return NULL;
   return rli->keymap;
}

int SLrline_set_update_hook (SLrline_Type *rli,
			     void (*fun)(SLrline_Type *, SLFUTURE_CONST char *, SLFUTURE_CONST char *, unsigned int, unsigned int, VOID_STAR),
			     VOID_STAR client_data)
{
   if (rli == NULL)
     return -1;

   rli->update_hook = fun;
   rli->update_client_data = client_data;
   return 0;
}

char *SLrline_get_line (SLrline_Type *rli)
{
   if (rli == NULL)
     return NULL;
   return SLmake_nstring ((char *)rli->buf, rli->len);
}

int SLrline_get_point (SLrline_Type *rli, unsigned int *pointp)
{
   if (rli == NULL)
     return -1;
   *pointp = rli->point;
   return 0;
}

int SLrline_set_point (SLrline_Type *rli, unsigned int point)
{
   if (rli == NULL)
     return -1;

   if (rli->state == RLI_LINE_INVALID)
     return -1;

   if (point > rli->len)
     point = rli->len;

   rli->point = point;
   return 0;
}

int SLrline_get_tab (SLrline_Type *rli, unsigned int *p)
{
   if (rli == NULL)
     return -1;
   *p = rli->tab;
   return 0;
}

int SLrline_set_tab (SLrline_Type *rli, unsigned int tab)
{
   if (rli == NULL)
     return -1;

   rli->tab = tab;
   return 0;
}

int SLrline_get_hscroll (SLrline_Type *rli, unsigned int *p)
{
   if (rli == NULL)
     return -1;
   *p = rli->hscroll;
   return 0;
}

int SLrline_set_hscroll (SLrline_Type *rli, unsigned int p)
{
   if (rli == NULL)
     return -1;
   rli->hscroll = p;
   return 0;
}

int SLrline_set_line (SLrline_Type *rli, SLFUTURE_CONST char *buf)
{
   unsigned int len;

   if (rli == NULL)
     return -1;

   if (buf == NULL)
     buf = "";

   len = strlen (buf);

   buf = SLmake_string (buf);
   if (buf == NULL)
     return -1;

   SLfree ((char *)rli->buf);
   rli->buf = (unsigned char *)buf;
   rli->buf_len = len;

   rli->point = len;
   rli->len = len;

   rli->state = RLI_LINE_SET;
   return 0;
}

int SLrline_set_echo (SLrline_Type *rli, int state)
{
   if (rli == NULL)
     return -1;

   if (state == 0)
     rli->flags |= SL_RLINE_NO_ECHO;
   else
     rli->flags &= ~SL_RLINE_NO_ECHO;

   return 0;
}

int SLrline_get_echo (SLrline_Type *rli, int *statep)
{
   if (rli == NULL)
     return -1;

   *statep = (0 == (rli->flags & SL_RLINE_NO_ECHO));
   return 0;
}

int SLrline_set_display_width (SLrline_Type *rli, unsigned int w)
{
   unsigned int old_width;

   if (rli == NULL)
     return -1;
   if (w < 1)
     w = 80;

   old_width = rli->edit_width;
   rli->edit_width = w;

   if (rli->update_display_width_changed_hook != NULL)
     (*rli->update_display_width_changed_hook) (rli, w, rli->update_client_data);
   else
     {
	if (w != old_width)
	  SLrline_redraw (rli);
     }
   return 0;
}

/* SLang Interface */

static void rline_ins_intrinsic (char *str)
{
   if (Active_Rline_Info != NULL)
     (void) SLrline_ins (Active_Rline_Info, str, strlen (str));
}

static void rline_del_intrinsic (int *np)
{
   int n = *np;

   if (Active_Rline_Info == NULL)
     return;

   if (n < 0)
     {
	(void) SLrline_move (Active_Rline_Info, n);
	n = -n;
     }

   (void) SLrline_del (Active_Rline_Info, (unsigned int) n);
}

static SLkeymap_Type *get_keymap (void)
{
   SLkeymap_Type *kmap;

   if (Active_Rline_Info != NULL)
     kmap = SLrline_get_keymap (Active_Rline_Info);
   else
     kmap = RL_Keymap;

   if (kmap != NULL)
     return kmap;

   _pSLang_verror (SL_APPLICATION_ERROR, "No keymap available for rline interface");
   return NULL;
}

static void rline_setkey_intrinsic (char *keyseq)
{
   char *str;
   SLkeymap_Type *kmap;

   if (NULL == (kmap = get_keymap ()))
     return;

   if (SLang_peek_at_stack () == SLANG_REF_TYPE)
     {
	SLang_Name_Type *nt;

	if (NULL == (nt = SLang_pop_function ()))
	  return;

	(void) SLkm_define_slkey (keyseq, nt, kmap);
	return;
     }

   if (-1 == SLang_pop_slstring (&str))
     return;

   (void) SLang_define_key (keyseq, str, kmap);
   SLang_free_slstring (str);
}

static void rline_unsetkey_intrinsic (char *keyseq)
{
   SLkeymap_Type *kmap;

   if (NULL != (kmap = get_keymap ()))
     SLang_undefine_key (keyseq, kmap);
}

static int rline_get_point_intrinsic (void)
{
   unsigned int p;

   if ((Active_Rline_Info == NULL)
       || (-1 == SLrline_get_point (Active_Rline_Info, &p)))
     return 0;

   return (int) p;
}

static void rline_set_point_intrinsic (int *pp)
{
   int p;
   SLrline_Type *rli;

   if (NULL == (rli = Active_Rline_Info))
     return;

   p = *pp;
   if (p < 0)
     {
	p = (int)rli->len + (p + 1);
	if (p < 0)
	  p = 0;
     }

   if ((unsigned int)p > rli->len)
     p = (int)rli->len;

   (void) SLrline_set_point (rli, (unsigned int) p);
}

static void rline_call_intrinsic (char *fun)
{
   int (*f)(SLrline_Type *);

   if (Active_Rline_Info == NULL)
     return;

   if (NULL == (f = (int (*)(SLrline_Type *)) (SLang_find_key_function(fun, Active_Rline_Info->keymap))))
     {
	_pSLang_verror (SL_UndefinedName_Error, "rline internal function %s does not exist", fun);
	return;
     }

   (void) (*f)(Active_Rline_Info);
   /* Active_Rline_Info->last_fun = (FVOID_STAR) f; */
}

static void rline_get_line_intrinsic (void)
{
   char *s;

   if ((Active_Rline_Info == NULL)
       || (NULL == (s = SLrline_get_line (Active_Rline_Info))))
     {
	(void) SLang_push_string ("");
	return;
     }
   (void) SLang_push_malloced_string (s);
}

static void rline_set_line_intrinsic (char *str)
{
   if (Active_Rline_Info == NULL)
     return;

   (void) SLrline_set_line (Active_Rline_Info, str);
}

static void rline_set_completion_callback (void)
{
   SLang_Name_Type *nt;

   if (NULL == (nt = SLang_pop_function ()))
     return;

   if (Active_Rline_Info == NULL)
     {
	SLang_free_function (Default_Completion_Callback);
	Default_Completion_Callback = nt;
	return;
     }
   SLang_free_function (Active_Rline_Info->completion_callback);
   Active_Rline_Info->completion_callback = nt;
}

static void rline_set_list_completions_callback (void)
{
   SLang_Name_Type *nt;

   if (NULL == (nt = SLang_pop_function ()))
     return;

   if (Active_Rline_Info == NULL)
     {
	SLang_free_function (Default_List_Completions_Callback);
	Default_List_Completions_Callback = nt;
	return;
     }
   SLang_free_function (Active_Rline_Info->list_completions_callback);
   Active_Rline_Info->list_completions_callback = nt;
}

static int rline_input_pending_intrinsic (int *tsecsp)
{
   int tsecs = *tsecsp;
   if (tsecs < 0)
     tsecs = 0;

   if (Active_Rline_Info == NULL)
     return 0;

   if (Active_Rline_Info->input_pending == NULL)
     return 1;

   return Active_Rline_Info->input_pending (tsecs);
}

static int rline_getkey_intrinsic (void)
{
   if (Active_Rline_Info == NULL)
     return -1;

   return (int) Active_Rline_Info->getkey ();
}

static int rline_bolp_intrinsic (void)
{
   if (Active_Rline_Info == NULL)
     return 0;
   return Active_Rline_Info->point == 0;
}

static int rline_eolp_intrinsic (void)
{
   if (Active_Rline_Info == NULL)
     return 0;
   return Active_Rline_Info->point == Active_Rline_Info->len;
}

static int rline_get_edit_width_intrinsic (void)
{
   if (Active_Rline_Info == NULL)
     return 80;
   return Active_Rline_Info->edit_width;
}

static void rline_get_history_intrinsic (void)
{
   SLindex_Type i, num;
   RL_History_Type *h;
   char **data;
   SLang_Array_Type *at;

   if (Active_Rline_Info == NULL)
     {
	SLang_push_null ();
	return;
     }

   num = 0;
   h = Active_Rline_Info->root;
   while (h != NULL)
     {
	h = h->next;
	num++;
     }
   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1)))
     return;

   data = (char **)at->data;
   h = Active_Rline_Info->root;
   for (i = 0; i < num; i++)
     {
	if (NULL == (data[i] = SLang_create_slstring (h->buf)))
	  {
	     SLang_free_array (at);
	     return;
	  }
	h = h->next;
     }

   (void) SLang_push_array (at, 1);
}

static void rline_set_history_intrinsic (void)
{
   int i, num;
   char **data;
   SLrline_Type *rli;
   SLang_Array_Type *at;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;

   if (NULL == (rli = Active_Rline_Info))
     {
	SLang_free_array (at);
	return;
     }

   free_history (rli->root);
   rli->tail = rli->root = rli->last = NULL;

   data = (char **)at->data;
   num = at->num_elements;
   for (i = 0; i < num; i++)
     {
	if (-1 == SLrline_add_to_history (rli, data[i]))
	  break;
     }
   SLang_free_array (at);
}

static char *find_function_string (SLrline_Type *rli, FVOID_STAR f)
{
   SLKeymap_Function_Type *fp;

   if ((rli == NULL) || (rli->keymap == NULL))
     return NULL;

   fp = rli->keymap->functions;
   while ((fp != NULL) && (fp->name != NULL))
     {
	if ((FVOID_STAR) fp->f == f) return (char *) fp->name;
	fp++;
     }
   return NULL;
}

static void rline_get_last_key_function_intrinsic (void)
{
   SLang_Key_Type *last_key;
   char *s;

   if (Active_Rline_Info == NULL)
     {
	(void) SLang_push_null ();
	return;
     }

   last_key = &Active_Rline_Info->last_key;
   switch (last_key->type)
     {
      case SLKEY_F_INTERPRET:
	if (last_key->f.s != NULL)
	  {
	     (void) SLang_push_string (last_key->f.s);
	     return;
	  }
	break;

      case SLKEY_F_INTRINSIC:
	if (NULL != (s = find_function_string (Active_Rline_Info, last_key->f.f)))
	  {
	     (void) SLang_push_string (s);
	     return;
	  }
	break;

      case SLKEY_F_SLANG:
	if (NULL != last_key->f.slang_fun)
	  {
	     (void) SLang_push_function (last_key->f.slang_fun);
	     return;
	  }
      case SLKEY_F_KEYSYM:
	(void) SLang_push_uint (last_key->f.keysym);
	return;

      default:
	break;
     }

   (void) SLang_push_null ();
}

static SLang_Intrin_Fun_Type Intrinsics [] =
{
   MAKE_INTRINSIC_S("rline_call", rline_call_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_S("rline_ins", rline_ins_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_I("rline_del", rline_del_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_0("rline_bolp", rline_bolp_intrinsic, INT_TYPE),
   MAKE_INTRINSIC_0("rline_eolp", rline_eolp_intrinsic, INT_TYPE),
   MAKE_INTRINSIC_0("rline_get_point", rline_get_point_intrinsic, INT_TYPE),
   MAKE_INTRINSIC_I("rline_set_point", rline_set_point_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_0("rline_get_edit_width", rline_get_edit_width_intrinsic, INT_TYPE),
   MAKE_INTRINSIC_0("rline_get_line", rline_get_line_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_S("rline_set_line", rline_set_line_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_S("rline_setkey", rline_setkey_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_S("rline_unsetkey", rline_unsetkey_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_0("rline_getkey", rline_getkey_intrinsic, INT_TYPE),
   MAKE_INTRINSIC_0("rline_set_history", rline_set_history_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_0("rline_get_history", rline_get_history_intrinsic, VOID_TYPE),
   MAKE_INTRINSIC_I("rline_input_pending", rline_input_pending_intrinsic, INT_TYPE),
   MAKE_INTRINSIC_0("rline_set_completion_callback", rline_set_completion_callback, VOID_TYPE),
   MAKE_INTRINSIC_0("rline_set_list_completions_callback", rline_set_list_completions_callback, VOID_TYPE),
   MAKE_INTRINSIC_0("rline_get_last_key_function", rline_get_last_key_function_intrinsic, VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int SLrline_init (SLFUTURE_CONST char *appname, SLFUTURE_CONST char *user_initfile, SLFUTURE_CONST char *sys_initfile)
{
#ifdef __WIN32__
   char *home_dir = getenv ("USERPROFILE");
#else
# ifdef VMS
   char *home_dir = "SYS$LOGIN:";
# else
   char *home_dir = getenv ("HOME");
# endif
#endif
   char *file = NULL;
   int status;
   static char *appname_malloced;

   if (sys_initfile == NULL)
     sys_initfile = SLRLINE_SYS_INIT_FILE;
   if (user_initfile == NULL)
     user_initfile = SLRLINE_USER_INIT_FILE;

   if (appname == NULL)
     appname = "Unknown";

   if (NULL == (appname_malloced = SLmake_string (appname)))
     return -1;

   if (-1 == SLadd_intrinsic_variable ("__RL_APP__", &appname_malloced, SLANG_STRING_TYPE, 1))
     return -1;

   if (-1 == SLadd_intrin_fun_table (Intrinsics, NULL))
     return -1;

   if (-1 == init_keymap ())
     return -1;
#if SLANG_HAS_MULTILINE_RLINE
   /* Initialize the terminal here to allow the user to use termcap sequences */
   (void) SLtt_initialize (NULL);
#endif
   if (user_initfile != NULL)
     {
	file = SLpath_find_file_in_path (home_dir, user_initfile);
	if (file != NULL)
	  {
	     status = SLns_load_file (file, NULL);
	     SLfree (file);
	     return status;
	  }
     }

   if (sys_initfile != NULL)
     {
	file = _pSLpath_find_file (sys_initfile, 0);
	if (file != NULL)
	  {
	     status = SLns_load_file (file, NULL);
	     SLang_free_slstring (file);
	     return status;
	  }
     }

   return 0;
}
