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

/* Support for UTF-8 combining characters added by Adrian Colley */

#include "slinclud.h"

#include <signal.h>
#include <errno.h>

#include "slang.h"
#include "_slang.h"
#include "slcurses.h"

/* This file is meant to implement a primitive curses implementation in
 * terms of SLsmg calls.  The fact is that the interfaces are sufficiently
 * different that a 100% emulation is not possible.
 */

#define SLCURSES_NULLATTR ((SLcurses_Char_Type)0)
#define SLCURSES_NULLCHAR ((SLwchar_Type)0)

#define SLCURSES_BUILD_CHAR(ch,wch,c) \
   ((ch) = ((SLcurses_Char_Type)(wch)) | (((SLcurses_Char_Type)(c)) << 24))

#define SLCURSES_BUILD_CELL(cellp,wch,c,isacs) \
   do \
     { \
	int slcurses_build_cell_i; \
	SLcurses_Cell_Type *slcurses_build_cell_c = (cellp); \
	SLCURSES_BUILD_CHAR(slcurses_build_cell_c->main, wch, c); \
	slcurses_build_cell_c->is_acs = isacs; \
	for (slcurses_build_cell_i = 0; \
	     slcurses_build_cell_i < SLSMG_MAX_CHARS_PER_CELL-1; \
	     slcurses_build_cell_i++) \
	  slcurses_build_cell_c->combining[slcurses_build_cell_i] = SLCURSES_NULLCHAR; \
     } \
   while (0)

#define SLCURSES_EXTRACT_COLOR(ch) (((ch >> 24)&0xFF))
#define SLCURSES_EXTRACT_CHAR(ch) ((ch)&A_CHARTEXT)

#define SLTT_EXTRACT_CHAR(ch) ((ch)&A_CHARTEXT)

SLcurses_Window_Type *SLcurses_Stdscr;
int SLcurses_Esc_Delay = 150;	       /* 0.15 seconds */
SLtt_Char_Type SLcurses_Acs_Map [128];
int SLcurses_Is_Endwin = 1;
int SLcurses_Num_Colors = 8;

static void blank_line (SLcurses_Cell_Type *b, unsigned int len, SLsmg_Color_Type color)
{
   SLcurses_Cell_Type *bmax = b + len;

   while (b < bmax)
     SLCURSES_BUILD_CELL(b++,' ',color, 0);
}

static int va_mvprintw (SLcurses_Window_Type *w, int r, int c, int do_move,
			char *fmt, va_list ap)
{
   char buf[1024];

   if (do_move) SLcurses_wmove (w, r, c);

   (void) SLvsnprintf (buf, sizeof(buf), fmt, ap);

   SLcurses_waddnstr (w, buf, -1);
   return 0;
}

int SLcurses_mvprintw (int r, int c, char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (SLcurses_Stdscr, r, c, 1, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_mvwprintw (SLcurses_Window_Type *w, int r, int c, char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (w, r, c, 1, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_wprintw (SLcurses_Window_Type *w, char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (w, 0, 0, 0, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_printw (char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (SLcurses_Stdscr, 0, 0, 0, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_nil (void)
{
   return 0;
}

int SLcurses_has_colors(void)
{
   return SLtt_Use_Ansi_Colors;
}

int SLcurses_nodelay (SLcurses_Window_Type *w, int onoff)
{
   w->delay_off = (onoff ? 0 : -1);
   return 0;
}

static unsigned char Keyboard_Buffer[256];
static unsigned char *Keyboard_Buffer_Start = Keyboard_Buffer;
static unsigned char *Keyboard_Buffer_Stop = Keyboard_Buffer;

static int getkey_function (void)
{
   int ch;

   ch = SLang_getkey ();
   if (ch != SLANG_GETKEY_ERROR)
     {
	*Keyboard_Buffer_Stop++ = (unsigned char) ch;
	if (Keyboard_Buffer_Stop == Keyboard_Buffer + sizeof (Keyboard_Buffer))
	  Keyboard_Buffer_Stop = Keyboard_Buffer;
     }

   return ch;
}

static int get_buffered_key (void)
{
   int ch;

   if (Keyboard_Buffer_Stop == Keyboard_Buffer_Start)
     return SLANG_GETKEY_ERROR;

   ch = *Keyboard_Buffer_Start++;
   if (Keyboard_Buffer_Start == Keyboard_Buffer + sizeof (Keyboard_Buffer))
     Keyboard_Buffer_Start = Keyboard_Buffer;

   return ch;
}

static int get_keypad_key (void)
{
   int ch;

   if (Keyboard_Buffer_Stop != Keyboard_Buffer_Start)
     return get_buffered_key ();

   ch = SLang_getkey ();
   if (ch == '\033')
     {
	if (0 == SLang_input_pending (ESCDELAY / 100))
	  return ch;
     }
   else if (ch == SLANG_GETKEY_ERROR) return ERR;
   SLang_ungetkey (ch);
   ch = SLkp_getkey ();
   if (ch == SL_KEY_ERR)
     ch = get_buffered_key ();
   else
     Keyboard_Buffer_Stop = Keyboard_Buffer_Start;
   return ch;
}

int SLcurses_wgetch (SLcurses_Window_Type *w)
{
   if (w == NULL)
     return ERR;

   SLcurses_wrefresh (w);

   if ((Keyboard_Buffer_Start != Keyboard_Buffer_Stop)
       || (w->delay_off == -1)
       || SLang_input_pending (w->delay_off))
     {
	if (w->use_keypad)
	  return get_keypad_key ();

	return SLang_getkey ();
     }

   return ERR;
}

int SLcurses_getch (void)
{
   return SLcurses_wgetch (SLcurses_Stdscr);
}

/* This is a super hack.  That fact is that SLsmg and curses
 * are incompatible.
 */
static unsigned char Color_Objects[256];

static unsigned int map_attr_to_object (SLtt_Char_Type attr)
{
   unsigned int obj;
   SLtt_Char_Type at;

   obj = SLCURSES_EXTRACT_COLOR(attr);

   if (SLtt_Use_Ansi_Colors)
     {
	if (Color_Objects[obj] != 0) return obj;

	at = SLtt_get_color_object (obj & 0xF);

	if (attr & A_BOLD) at |= SLTT_BOLD_MASK;
	if (attr & A_UNDERLINE) at |= SLTT_ULINE_MASK;
	if (attr & A_REVERSE) at |= SLTT_REV_MASK;
	if (attr & A_ALTCHARSET) at |= SLTT_ALTC_MASK;

	SLtt_set_color_object (obj, at);

	Color_Objects[obj] = 1;
     }
   else obj = obj & 0xF0;

   return obj;

}

int SLcurses_start_color (void)
{
   int f, b;
   int obj;

   if (SLtt_Use_Ansi_Colors == 0) return -1;

   obj = 0;
   for (f = 0; f < 16; f++)
     {
	for (b = 0; b < 16; b++)
	  {
	     obj++;
	     SLtt_set_color_fgbg (obj, f, b);
	  }
     }
   return 0;
}

#ifdef SIGINT
static void sigint_handler (int sig)
{
   SLang_reset_tty ();
   SLsmg_reset_smg ();
   exit (sig);
}
#endif

/* Values are assumed to be 0, 1, 2.  This fact is exploited */
static int TTY_State;

static int init_tty (int suspend_ok)
{
   if (-1 == SLang_init_tty (-1, 1, 0))
     return -1;

#ifdef REAL_UNIX_SYSTEM
   if (suspend_ok) SLtty_set_suspend_state (1);
#endif
   return 0;
}

int SLcurses_raw (void)
{
   TTY_State = 1;
   return init_tty (0);
}

int SLcurses_cbreak (void)
{
   TTY_State = 2;
   return init_tty (1);
}

#if defined(SIGTSTP) && defined(SIGSTOP)
static void sigtstp_handler (int sig)
{
   sig = errno;

   SLsmg_suspend_smg ();

   if (TTY_State)
     SLang_reset_tty ();

   kill(getpid(),SIGSTOP);

   SLsmg_resume_smg ();

   if (TTY_State) init_tty (TTY_State - 1);

   signal (SIGTSTP, sigtstp_handler);
   errno = sig;
}
#endif

SLcurses_Window_Type *SLcurses_initscr (void)
{
   SLcurses_Is_Endwin = 0;
   SLsmg_Newline_Behavior = SLSMG_NEWLINE_MOVES;
   SLtt_get_terminfo ();

#if !defined(IBMPC_SYSTEM) && !defined(VMS)
   if (-1 == (SLcurses_Num_Colors = SLtt_tgetnum ("Co")))
#endif
     SLcurses_Num_Colors = 8;

   /* Enable UTF-8 support if used on the terminal */
   (void) SLutf8_enable (-1);

   if ((-1 == SLkp_init ())
       || (-1 == SLcurses_cbreak ())
       || (NULL == (SLcurses_Stdscr = SLcurses_newwin (0, 0, 0, 0)))
       || (-1 == SLsmg_init_smg ()))
     {
	SLang_exit_error ("SLcurses_initscr: init failed\n");
	return NULL;
     }
   SLkp_set_getkey_function (getkey_function);

#ifdef SIGINT
   signal (SIGINT, sigint_handler);
#endif

#if defined(SIGTSTP) && defined(SIGSTOP)
   signal (SIGTSTP, sigtstp_handler);
#endif

   SLtt_set_mono (SLCURSES_EXTRACT_COLOR(A_BOLD), NULL, SLTT_BOLD_MASK);
   SLtt_set_mono (SLCURSES_EXTRACT_COLOR(A_UNDERLINE), NULL, SLTT_ULINE_MASK);
   SLtt_set_mono (SLCURSES_EXTRACT_COLOR(A_REVERSE), NULL, SLTT_REV_MASK);
   /* SLtt_set_mono (SLCURSES_EXTRACT_COLOR(A_BLINK), NULL, SLTT_BLINK_MASK); */
   SLtt_set_mono ((SLCURSES_EXTRACT_COLOR(A_BOLD|A_UNDERLINE)), NULL, SLTT_ULINE_MASK|SLTT_BOLD_MASK);
   SLtt_set_mono ((SLCURSES_EXTRACT_COLOR(A_REVERSE|A_UNDERLINE)), NULL, SLTT_ULINE_MASK|SLTT_REV_MASK);

   if (SLtt_Has_Alt_Charset)
     {
       SLcurses_Acs_Map[SLSMG_ULCORN_CHAR] = SLSMG_ULCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_URCORN_CHAR] = SLSMG_URCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LLCORN_CHAR] = SLSMG_LLCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LRCORN_CHAR] = SLSMG_LRCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_UTEE_CHAR] = SLSMG_UTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_DTEE_CHAR] = SLSMG_DTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LTEE_CHAR] = SLSMG_LTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_RTEE_CHAR] = SLSMG_RTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_VLINE_CHAR] = SLSMG_VLINE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_HLINE_CHAR] = SLSMG_HLINE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_PLUS_CHAR] = SLSMG_PLUS_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_CKBRD_CHAR] = SLSMG_CKBRD_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_DIAMOND_CHAR] = SLSMG_DIAMOND_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_DEGREE_CHAR] = SLSMG_DEGREE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_PLMINUS_CHAR] = SLSMG_PLMINUS_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_BULLET_CHAR] = SLSMG_BULLET_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LARROW_CHAR] = SLSMG_LARROW_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_RARROW_CHAR] = SLSMG_RARROW_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_DARROW_CHAR] = SLSMG_DARROW_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_UARROW_CHAR] = SLSMG_UARROW_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_BOARD_CHAR] = SLSMG_BOARD_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_BLOCK_CHAR] = SLSMG_BLOCK_CHAR | A_ALTCHARSET;
     }
   else
     {
       /* ugly defaults to use on terminals which don't support graphics */
	SLcurses_Acs_Map[SLSMG_ULCORN_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_URCORN_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_LLCORN_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_LRCORN_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_UTEE_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_DTEE_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_LTEE_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_RTEE_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_VLINE_CHAR] = '|';
	SLcurses_Acs_Map[SLSMG_HLINE_CHAR] = '-';
	SLcurses_Acs_Map[SLSMG_PLUS_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_CKBRD_CHAR] = '#';
	SLcurses_Acs_Map[SLSMG_DIAMOND_CHAR] = '+';
	SLcurses_Acs_Map[SLSMG_DEGREE_CHAR] = '\\';
	SLcurses_Acs_Map[SLSMG_PLMINUS_CHAR] = '#';
	SLcurses_Acs_Map[SLSMG_BULLET_CHAR] = 'o';
	SLcurses_Acs_Map[SLSMG_LARROW_CHAR] = '<';
	SLcurses_Acs_Map[SLSMG_RARROW_CHAR] = '>';
	SLcurses_Acs_Map[SLSMG_DARROW_CHAR] = 'v';
	SLcurses_Acs_Map[SLSMG_UARROW_CHAR] = '^';
	SLcurses_Acs_Map[SLSMG_BOARD_CHAR] = '#';
	SLcurses_Acs_Map[SLSMG_BLOCK_CHAR] = '#';
     }

   return SLcurses_Stdscr;
}

int SLcurses_wattrset (SLcurses_Window_Type *w, SLtt_Char_Type ch)
{
   unsigned int obj;

   obj = map_attr_to_object (ch);
   w->color = obj;
   w->attr = ch;
   return 0;
}

int SLcurses_wattroff (SLcurses_Window_Type *w, SLtt_Char_Type ch)
{
   if (SLtt_Use_Ansi_Colors)
     return SLcurses_wattrset (w, 0);

   w->attr &= ~ch;
   return SLcurses_wattrset (w, w->attr);
}

int SLcurses_wattron (SLcurses_Window_Type *w, SLtt_Char_Type ch)
{
   if (SLtt_Use_Ansi_Colors)
     return SLcurses_wattrset (w, ch);

   w->attr |= ch;
   return SLcurses_wattrset (w, w->attr);
}

int SLcurses_delwin (SLcurses_Window_Type *w)
{
   if (w == NULL) return 0;
   if (w->lines != NULL)
     {
	SLcurses_Cell_Type **lines = w->lines;
	if (w->is_subwin == 0)
	  {
	     unsigned int r, rmax;

	     rmax = w->nrows;
	     for (r = 0; r < rmax; r++)
	       {
		  SLfree ((char *)lines[r]);
	       }
	  }

	SLfree ((char *)lines);
     }

   SLfree ((char *)w);
   if (w == SLcurses_Stdscr)
     SLcurses_Stdscr = NULL;
   return 0;
}

SLcurses_Window_Type *SLcurses_newwin (unsigned int nrows, unsigned int ncols,
				       unsigned int r, unsigned int c)
{
   SLcurses_Window_Type *win;
   SLcurses_Cell_Type **lines;

   if (r >= (unsigned int) SLtt_Screen_Rows)
     return NULL;
   if (c >= (unsigned int) SLtt_Screen_Cols)
     return NULL;

   if (NULL == (win = (SLcurses_Window_Type *) SLmalloc (sizeof (SLcurses_Window_Type))))
     return NULL;

   SLMEMSET ((char *) win, 0, sizeof (SLcurses_Window_Type));

   if (nrows == 0)
     nrows = (unsigned int) SLtt_Screen_Rows - r;
   if (ncols == 0)
     ncols = (unsigned int) SLtt_Screen_Cols - c;

   lines = (SLcurses_Cell_Type **) _SLcalloc (nrows, sizeof (SLcurses_Cell_Type *));
   if (lines == NULL)
     {
	SLcurses_delwin (win);
	return NULL;
     }

   SLMEMSET ((char *) lines, 0, nrows * sizeof (SLcurses_Cell_Type *));

   win->lines = lines;
   win->scroll_max = win->nrows = nrows;
   win->ncols = ncols;
   win->_begy = r;
   win->_begx = c;
   win->_maxx = (c + ncols) - 1;
   win->_maxy = (r + nrows) - 1;
   win->modified = 1;
   win->delay_off = -1;

   for (r = 0; r < nrows; r++)
     {
	SLcurses_Cell_Type *b;

	b = (SLcurses_Cell_Type *) _SLcalloc (ncols, sizeof (SLcurses_Cell_Type));
	if (b == NULL)
	  {
	     SLcurses_delwin (win);
	     return NULL;
	  }
	lines [r] = b;
	blank_line (b, ncols, 0);
     }

   return win;
}

int SLcurses_wmove (SLcurses_Window_Type *win, unsigned int r, unsigned int c)
{
   if (win == NULL) return -1;
   win->_cury = r;
   win->_curx = c;
   win->modified = 1;
   return 0;
}

static int do_newline (SLcurses_Window_Type *w)
{
   w->_curx = 0;
   w->_cury += 1;
   if (w->_cury >= w->scroll_max)
     {
	w->_cury = w->scroll_max - 1;
	if (w->scroll_ok)
	  SLcurses_wscrl (w, 1);
     }

   return 0;
}

/* SLcurses_placechar(w, wch, width, color) places character wch at the current
 * position in w.  If any multicolumn characters are partially overwritten as a
 * result, the remains of those characters are blanked out.  The caller is
 * responsible for ensuring that there is room at the current position for
 * (width) columns.  The current position is not advanced by this function.  If
 * width is zero, this function adds the character to the combining[] array of
 * the current position, if there's room (the character is dropped if there's
 * no room).
 */
static void SLcurses_placechar (SLcurses_Window_Type *w, SLwchar_Type wch,
				int width, int color, int is_acs)
{
   SLcurses_Cell_Type *b;
   unsigned int i;

   if (width <= 0)
     {
	/* Backtrack to the start of any multicolumn character. */
	for (i = w->_curx; i > 0; i--)
	  if (w->lines[w->_cury][i].main != SLCURSES_NULLATTR)
	    break;
	b = &w->lines[w->_cury][i];
	/* Find a place for the new combining character. */
	for (i = 0; i < SLSMG_MAX_CHARS_PER_CELL-1; i++)
	  if (b->combining[i] == SLCURSES_NULLCHAR)
	    break;
	/* Add character if it fits. */
	if (i < SLSMG_MAX_CHARS_PER_CELL-1)
	  b->combining[i] = wch;
	return;
     }

   b = &w->lines[w->_cury][w->_curx];
   if (b->main == SLCURSES_NULLATTR)
     {
	/* wch will overwrite part of a multicolumn character, so
	 * blank out the left half of the character.
	 */
	int blankcolor = w->color;
	i = w->_curx;
	while (1)
          {
	     if (w->lines[w->_cury][i].main != SLCURSES_NULLATTR)
	       {
		  blankcolor = SLCURSES_EXTRACT_COLOR(w->lines[w->_cury][i].main);
		  break;
	       }
	     if (i == 0)
	       break;
	     i--;
	  }
	while (i < w->_curx)
	  {
	     SLCURSES_BUILD_CELL(&w->lines[w->_cury][i], ' ', blankcolor, is_acs);
	     i++;
	  }
     }
   /* Place wch itself. */
   SLCURSES_BUILD_CELL(b, wch, w->color, is_acs);
   /* If wch is multicolumn, reserve the following cell(s). */
   for (i = 1; i < (unsigned int) width; i++)
     {
	b[i].main = SLCURSES_NULLATTR;
	/* It's unnecessary to clear b[i].combined when b[i].main==0. */
     }
   /* If wch is followed by the tail of a previous, overwritten, multicolumn
    * character, then blank it out completely.
    */
   for (i = w->_curx + width; i < w->ncols; i++)
     {
	b = &w->lines[w->_cury][i];
	if (b->main != SLCURSES_NULLATTR)
	  break;
	SLCURSES_BUILD_CELL(b, ' ', color, is_acs);
     }
}

int SLcurses_waddch (SLcurses_Window_Type *win, SLtt_Char_Type attr)
{
   SLtt_Char_Type ch;
   SLsmg_Color_Type color;
   int width;
   int is_acs;

   if (win == NULL) return -1;

   if (win->_cury >= win->nrows)
     {
	/* Curses seems to move current position to top of window. */
	win->_cury = win->_curx = 0;
	return -1;
     }

   win->modified = 1;

   ch = SLTT_EXTRACT_CHAR(attr);
   if (ch == 0) return -1;

   if (attr == ch)
     color = win->color;
   else
     {
	/* hack to pick up the default color for graphics chars */
	if (((attr & A_COLOR) == 0) && ((attr & A_ALTCHARSET) != 0))
	  {
	     SLCURSES_BUILD_CHAR(attr, attr, win->color);
	  }
	color = map_attr_to_object (attr);
     }

   is_acs = attr & A_ALTCHARSET;

   if (SLwchar_iscntrl((SLwchar_Type)ch))
     {
	if (ch == '\n')
	  {
	     SLcurses_wclrtoeol (win);
	     return do_newline (win);
	  }

	if (ch == '\r')
	  {
	     win->_curx = 0;
	     return 0;
	  }

	if (ch == '\b')
	  {
	     if (win->_curx > 0)
	       win->_curx--;

	     return 0;
	  }

	if (ch == '\t')
	  {
	     int err;
	     do
	       err = SLcurses_waddch (win, (SLtt_Char_Type)' ');
	     while (err == 0 && win->_curx % SLsmg_Tab_Width != 0);
	     return err;
	  }
     }

   width = SLwchar_isprint (ch)
	? (SLsmg_is_utf8_mode ()
		? SLwchar_wcwidth (ch)
		: 1)
	: 0;
   if (win->_curx + width > win->ncols)
     {
	SLcurses_wclrtoeol (win);
	do_newline (win);
     }

   SLcurses_placechar (win, ch, width, color, is_acs);
   win->_curx += width;

   return 0;
}

static void write_color_chars (SLcurses_Cell_Type *p, unsigned int len)
{
   int color = -1;
   unsigned int i;
   for (i = 0; i < len; i++)
     {
	SLwchar_Type ch;
	int this_color;
	int j;
	if (p[i].main == SLCURSES_NULLATTR)
	  continue;	/* prev char was multicolumn */
	ch = SLCURSES_EXTRACT_CHAR(p[i].main);
	this_color = SLCURSES_EXTRACT_COLOR(p[i].main);
	if (this_color != color)
	  {
	     SLsmg_set_color (this_color);
	     color = this_color;
	  }
	if (p[i].is_acs)
	  SLsmg_set_char_set (1);
	SLsmg_write_char (ch);
	for (j = 0; j < SLSMG_MAX_CHARS_PER_CELL-1; j++)
	  {
	     SLwchar_Type combining = p[i].combining[j];
	     if (combining == SLCURSES_NULLCHAR)
	       break;	/* no more combining chars */
	     SLsmg_write_char (combining);
	  }
	if (p[i].is_acs)
	  SLsmg_set_char_set (0);
     }
}

int SLcurses_wnoutrefresh (SLcurses_Window_Type *w)
{
   unsigned int len;
   unsigned int r, c;
   unsigned int i, imax;

   if (SLcurses_Is_Endwin)
     {
	if (TTY_State) init_tty (TTY_State - 1);
       	SLsmg_resume_smg ();
	SLcurses_Is_Endwin = 0;
     }

   if (w == NULL)
     {
	SLsmg_refresh ();
	return -1;
     }

   if (w->modified == 0)
     return 0;

   r = w->_begy;
   c = w->_begx;

   len = w->ncols;
   imax = w->nrows;

   for (i = 0; i < imax; i++)
     {
	SLsmg_gotorc (r, c);
	write_color_chars (w->lines[i], len);
	r++;
     }

   if (w->has_box)
     SLsmg_draw_box(w->_begy, w->_begx, w->nrows, w->ncols);

   SLsmg_gotorc (w->_begy + w->_cury, w->_begx + w->_curx);
   w->modified = 0;
   return 0;
}

int SLcurses_wrefresh (SLcurses_Window_Type *w)
{
   if (w == NULL)
     return -1;

   if (w->modified == 0)
     return 0;

   SLcurses_wnoutrefresh (w);
   SLsmg_refresh ();
   return 0;
}

int SLcurses_wclrtoeol (SLcurses_Window_Type *w)
{
   SLcurses_Cell_Type *b, *bmax;
   int color;

   if (w == NULL) return -1;
   if (w->_cury >= w->nrows)
     return 0;

   w->modified = 1;

   b = w->lines[w->_cury];
   bmax = b + w->ncols;
   b += w->_curx;

   color = w->color;
   while (b < bmax)
     SLCURSES_BUILD_CELL(b++,' ',color, 0);

   return 0;
}

int SLcurses_wclrtobot (SLcurses_Window_Type *w)
{
   unsigned int r;
   int color;

   if (w == NULL) return -1;

   w->modified = 1;
   color = w->color;
   SLcurses_wclrtoeol (w);
   for (r = w->_cury + 1; r < w->nrows; r++)
     {
	SLcurses_Cell_Type *b, *bmax;
	b = w->lines [r];
	bmax = b + w->ncols;

	while (b < bmax)
	  SLCURSES_BUILD_CELL(b++,' ',color, 0);
     }

   return 0;
}

int SLcurses_wscrl (SLcurses_Window_Type *w, int n)
{
   SLcurses_Cell_Type **lines;
   unsigned int r0, r1, rmax, rmin, ncols;
   SLsmg_Color_Type color;

   if ((w == NULL) || (w->scroll_ok == 0))
     return -1;

   w->modified = 1;
#if 0
   if (w->is_subwin)
     {
	SLang_reset_tty ();
	SLsmg_reset_smg ();
	fprintf (stderr, "\rAttempt to scroll a subwindow\n");
	exit (1);
     }
#endif

   color = w->color;
   ncols = w->ncols;
   lines = w->lines;
   rmax = w->scroll_max;
   rmin = w->scroll_min;
   if (rmax > w->nrows)
     rmax = w->nrows;
   if ((rmin >= rmax) || (rmax == 0))
     return 0;

   if (n == 0)
     return 0;

   if (n > 0)
     {
	r0 = rmin;
	r1 = rmin + n;

	while (r1 < rmax)
	  {
	     if (w->is_subwin)
	       memcpy ((char *)lines[r0], (char *)lines[r1],
		       sizeof (SLcurses_Cell_Type) * ncols);
	     else
	       {
		  SLcurses_Cell_Type *swap = lines[r0];
		  lines[r0] = lines[r1];
		  lines[r1] = swap;
	       }
	     r0++;
	     r1++;
	  }
	while (r0 < rmax)
	  {
	     blank_line (lines[r0], ncols, color);
	     r0++;
	  }
	return 0;
     }

   /* else n < 0 */
   n = -n;

   r1 = rmax - 1;
   if (r1 < (unsigned int) n)
     n = r1;
   r0 = r1 - n;

   while (r0 >= rmin)
     {
	if (w->is_subwin)
	  memcpy ((char *)lines[r1], (char *)lines[r0],
		  sizeof (SLcurses_Cell_Type) * ncols);
	else
	  {
	     SLcurses_Cell_Type *swap = lines[r1];
	     lines[r1] = lines[r0];
	     lines[r0] = swap;
	  }
	r1--;
	if (r0 == 0)
	  break;
	r0--;
     }
   r0 = rmin;
   while (r0 <= r1)
     {
	blank_line (lines[r0], ncols, color);
	r0++;
     }

   return 0;
}

/* Note: if len is < 0, entire string will be used.
 */
int SLcurses_waddnstr (SLcurses_Window_Type *w, char *str, int len)
{
   unsigned int nrows, ncols, crow, ccol;
   SLuchar_Type *u, *umax;
   int is_acs = 0;

   if ((w == NULL)
       || (str == NULL))
     return -1;

   w->modified = 1;
   nrows = w->nrows;
   ncols = w->ncols;
   crow = w->_cury;
   ccol = w->_curx;

   if (w->scroll_max <= nrows)
     nrows = w->scroll_max;

   if (crow >= nrows)
     crow = 0;			       /* wrap back to top */

   u = (SLuchar_Type *)str;
   umax = &u[len >= 0 ? (unsigned int)len : strlen(str)];
   while (u < umax)
     {
	SLwchar_Type ch;
	SLstrlen_Type nconsumed;
	int width = 1;

	if (SLsmg_is_utf8_mode () && SLutf8_decode (u, umax, &ch, &nconsumed))
	  {
	     u += nconsumed;
	     if ((ch & A_CHARTEXT) != ch)
	       {
		  ch = (SLwchar_Type)0xFFFDL;	/* Unicode replacement character */
		  width = 1;
	       }
	     else if (SLwchar_isprint (ch))
	       width = SLwchar_wcwidth (ch);
	     else
	       width = 0;	/* FIXME: cope with <%02X> printstrings. */
	  }
	else
	  {
	     ch = (SLwchar_Type)*u++;
	     if (ch < 0x20 || (ch >= 0x7f && ch < 0xa0))
	       width = 0;	/* FIXME: use display_8bit */
	  }
	if (ch == '\t')
	  width = 1;	/* HACK forcing linewrap if ccol==ncols */
	if (ch == 0)
	  continue;	/* Avoid adding a literal SLCURSES_NULLCHAR. */

	/* FIXME; should this function be defined in terms of waddch? */
	if (ch == '\n')
	  {
	     w->_cury = crow;
	     w->_curx = ccol;
	     SLcurses_wclrtoeol (w);
	     do_newline (w);
	     crow = w->_cury;
	     ccol = w->_curx;
	     continue;
	  }

	if (ccol + width > ncols)
	  {
	     w->_curx = ccol;
	     w->_cury = crow;
	     SLcurses_wclrtoeol(w);	/* no-op if width<=1 */
	     w->_curx = ccol = 0;
	     w->_cury = ++crow;
	     if (crow >= nrows)
	       {
		  do_newline (w);
		  crow = w->_cury;
		  ccol = w->_curx;
	       }
	  }

	if (ch == '\t')
	  {
	     /* assert (ccol < ncols); */
	     w->_curx = ccol;
	     w->_cury = crow;
	     do
	       {
		  SLcurses_placechar (w, (SLwchar_Type)' ', 1, w->color, is_acs);
		  w->_curx = ++ccol;
	       }
	     while (ccol < ncols && ccol % SLsmg_Tab_Width != 0);
	     continue;
	  }

	SLcurses_placechar (w, ch, width, w->color, is_acs);
	w->_curx = (ccol += width);
     }

   w->_curx = ccol;
   w->_cury = crow;

   return 0;
}

/* This routine IS NOT CORRECT.  It needs to compute the proper overlap
 * and copy accordingly.  Here, I just assume windows are same size.
 */
#if 0
int SLcurses_overlay (SLcurses_Window_Type *swin, SLcurses_Window_Type *dwin)
{
   SLcurses_Char_Type *s, *smax, *d, *dmax;

   if ((swin == NULL) || (dwin == NULL))
     return -1;

   s = swin->buf;
   smax = swin->bufmax;
   d = dwin->buf;
   dmax = dwin->bufmax;

   while ((s < smax) && (d < dmax))
     {
	SLcurses_Char_Type ch = *s++;
	if (SLSMG_EXTRACT_CHAR(ch) != ' ')
	  *d = ch;
	d++;
     }

   return -1;			       /* not implemented */
}

#endif

SLcurses_Window_Type *SLcurses_subwin (SLcurses_Window_Type *orig,
				       unsigned int nlines, unsigned int ncols,
				       unsigned int begin_y, unsigned int begin_x)
{
   SLcurses_Window_Type *sw;
   int r, c;
   unsigned int i;

   if (orig == NULL)
     return NULL;

   sw = (SLcurses_Window_Type *) SLmalloc (sizeof (SLcurses_Window_Type));
   if (sw == NULL)
     return NULL;

   SLMEMSET ((char *)sw, 0, sizeof (SLcurses_Window_Type));
#if 1
   r = begin_y - orig->_begy;
#else
   r = 1 + ((int)orig->nrows - (int)nlines) / 2;
#endif
   if (r < 0) r = 0;
   if (r + nlines > orig->nrows) nlines = orig->nrows - r;

   c = ((int)orig->ncols - (int)ncols) / 2;
   if (c < 0) c = 0;
   if (c + ncols > orig->ncols) ncols = orig->ncols - c;

   sw->scroll_min = 0;
   sw->scroll_max = sw->nrows = nlines;
   sw->ncols = ncols;
   sw->_begy = begin_y;
   sw->_begx = begin_x;
   sw->_maxx = (begin_x + ncols) - 1;
   sw->_maxy = (begin_y + nlines) - 1;

   sw->lines = (SLcurses_Cell_Type **) _SLcalloc (nlines, sizeof (SLcurses_Cell_Type *));
   if (sw->lines == NULL)
     {
	SLcurses_delwin (sw);
	return NULL;
     }

   for (i = 0; i < nlines; i++)
     {
	sw->lines [i] = orig->lines [r + i] + c;
     }

   sw->is_subwin = 1;
   return sw;
}

int SLcurses_wclear (SLcurses_Window_Type *w)
{
   unsigned int i;

   if (w != NULL) w->modified = 1;
   for (i=0; i < w->nrows; i++)
     blank_line (w->lines[i], w->ncols, w->color);
   return 0;
}

int SLcurses_wdelch (SLcurses_Window_Type *w)
{
   int src, dest, limit;
   SLcurses_Cell_Type *line = w->lines[w->_cury];

   dest = w->_curx;
   /* Backtrack to start of this character. */
   while (dest > 0 && line[dest].main == SLCURSES_NULLATTR)
     dest--;
   w->_curx = dest;	/* Point to start of character, not the middle. */

   limit = w->ncols;
   src = dest + 1;
   /* Advance to start of next character. */
   while (src < limit && line[src].main == SLCURSES_NULLATTR)
     src++;

   /* line[dest..dest+limit-1-src] = line[src..limit-1]; */
#ifdef HAVE_MEMMOVE
   memmove(&line[dest], &line[src], (limit-src) * sizeof SLcurses_Cell_Type);
   dest += (limit-src);
#else
   while (src < limit)
     line[dest++] = line[src++];
#endif

   while (dest < limit)
     SLCURSES_BUILD_CELL(&line[dest++], ' ', w->color, 0);

   w->modified = 1;
   return 0;
}

int SLcurses_winsch (SLcurses_Window_Type *w, int ch)
{
   int is_acs = 0;
   int colsneeded;
#ifndef HAVE_MEMMOVE
   int dest, src;
#endif
   SLcurses_Cell_Type *line = w->lines[w->_cury];

   /* Backtrack to start of this character. */
   while (w->_curx > 0 && line[w->_curx].main == SLCURSES_NULLATTR)
     w->_curx--;

   if (ch == (SLcurses_Char_Type)'\t')
     ch = ' ';	/* FIXME */

   if (SLwchar_isprint (ch))
     {
	if (SLsmg_is_utf8_mode ())
	  colsneeded = SLwchar_wcwidth (ch);
	else
	  colsneeded = 1;
     }
   else colsneeded = 0;

   if (colsneeded == 0)
     {
	int x, i;
	SLcurses_Cell_Type *b = NULL;
	/* Back one character. */
	for (x = w->_curx - 1; x >= 0; x--)
	  {
	     b = &line[x];
	     if (b->main != SLCURSES_NULLATTR)
	       break;
	  }
	if (x < 0)
	  {	/* Back to end of previous line. */
	     if (w->_cury == 0)
	       return (-1);
	     line = w->lines[w->_cury - 1];
	     for (x = w->ncols - 1; x >= 0; x--)
	       {
		  b = &line[x];
		  if (b->main != SLCURSES_NULLATTR)
		    break;
	       }
	     if (x < 0)
	       return (-1);
	  }
	/* b is previous character; insert combining character ch in *b. */
	for (i = 0; i < SLSMG_MAX_CHARS_PER_CELL-1; i++)
	  if (b->combining[i] == SLCURSES_NULLCHAR)
	    break;
	if (i < SLSMG_MAX_CHARS_PER_CELL-1)
	  b->combining[i] = ch;
	return 0;
     }

   /* Remove trailing character(s) from line. */
   if ((colsneeded > 0) && ((unsigned int)colsneeded <= w->ncols))
     {
	int i;
	for (i = w->ncols - colsneeded; i > 0; i--)
	  if (line[i].main != SLCURSES_NULLATTR)
	    break;
	/* line[w->ncols-colsneeded..w->ncols-1] will be shifted off the end,
	 * but if i < w->ncols-colsneeded, then line[i..w->ncols-colsneeded-1]
	 * must be blanked because they are part of a multicolumn character
	 * which is partially shifted off the end.
	 */
	while (i + colsneeded < (int)w->ncols)
	  SLCURSES_BUILD_CELL(&line[i++], ' ', w->color, is_acs);
     }

   /* line[_curx+colsneeded..ncols-1] = line[_curx..ncols-1-colsneeded]; */
#ifdef HAVE_MEMMOVE
   memmove(&line[w->_curx+colsneeded], &line[w->_curx], ncols-(w->_curx+colsneeded));
#else
   dest = w->ncols - 1;
   for (src = dest - colsneeded; src >= (int)w->_curx; src--, dest--)
     line[dest] = line[src];
#endif
   if (w->_curx + colsneeded <= w->ncols)
     SLcurses_placechar (w, ch, colsneeded, w->color, is_acs);

   w->modified = 1;
   return 0;
}

int SLcurses_endwin (void)
{
   SLcurses_Is_Endwin = 1;
   SLsmg_suspend_smg ();
   SLang_reset_tty ();
   return 0;
}

#if 0
int SLcurses_mvwscanw (SLcurses_Window_Type *w, unsigned int r, unsigned int c,
		       char *fmt, ...)
{
#if HAVE_VFSCANF
   int ret;
   va_list ap;

   SLcurses_wmove (w, r, c);
   SLcurses_wrefresh (w);

   va_start(ap, fmt);
   ret = vfscanf (stdin, fmt, ap);
   va_end(ap);
   return ret;
#else
   return 0;
#endif
}

int SLcurses_wscanw (SLcurses_Window_Type *w, char *fmt, ...)
{
#if HAVE_VFSCANF
  va_list ap;
   int ret;

   SLcurses_wrefresh (w);

   va_start(ap, fmt);
   ret = vfscanf (stdin, fmt, ap);
   va_end(ap);

   return ret;
#else
   return 0;
#endif
}

int SLcurses_scanw (char *fmt, ...)
{
#ifdef HAVE_VFSCANF
   va_list ap;
   int ret;

   SLcurses_wrefresh (SLcurses_Stdscr);

   va_start(ap, fmt);
   ret = vfscanf (stdin, fmt, ap);
   va_end(ap);

   return ret;
#else
   return 0;
#endif
}
#endif

int SLcurses_clearok (SLcurses_Window_Type *w, int bf)
{
   if (bf)
     {
	SLsmg_cls ();
	w->modified = 1;
     }
   return 0;
}
/* vim:set noautoindent cinoptions=>5,n-3,{2,^-2,+4 cindent ruler: */
