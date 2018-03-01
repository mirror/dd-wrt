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

#include <sys/types.h>
#include <ctype.h>

#include "config.h"
#include "screen.h"
#include "mark.h"
#include "extern.h"

#ifdef COPY_PASTE

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * WARNING: these routines use the global variables "fore" and
 * "flayer" to make things easier.
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

static int  is_letter __P((int));
static void nextword __P((int *, int *, int, int));
static int  linestart __P((int));
static int  lineend __P((int));
static int  rem __P((int, int , int , int , int , char *, int));
static int  eq __P((int, int ));
static int  MarkScrollDownDisplay __P((int));
static int  MarkScrollUpDisplay __P((int));

static void MarkProcess __P((char **, int *));
static void MarkAbort __P((void));
static void MarkRedisplayLine __P((int, int, int, int));
static int  MarkRewrite __P((int, int, int, struct mchar *, int));

extern struct layer *flayer;
extern struct display *display, *displays;
extern struct win *fore;
extern struct mline mline_blank, mline_null;
extern struct mchar mchar_so;

#ifdef FONT
int pastefont = 1;
#endif

struct LayFuncs MarkLf =
{
  MarkProcess,
  MarkAbort,
  MarkRedisplayLine,
  DefClearLine,
  MarkRewrite,
  DefResize,
  DefRestore,
  0
};

int join_with_cr =  0;
int compacthist = 0;

unsigned char mark_key_tab[256]; /* this array must be initialised first! */

static struct markdata *markdata;


/*
 * VI like is_letter: 0 - whitespace
 *                    1 - letter
 *		      2 - other
 */
static int is_letter(c)
char c;
{
  if ((c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '_' || c == '.' ||
      c == '@' || c == ':' ||
      c == '%' || c == '!' ||
      c == '-' || c == '+')
    /* thus we can catch email-addresses as a word :-) */
    return 1;
  else if (c != ' ')
    return 2;
  return 0;
}

static int
linestart(y)
int y;
{
  register int x;
  register unsigned char *i;

  for (x = markdata->left_mar, i = WIN(y)->image + x; x < fore->w_width - 1; x++)
    if (*i++ != ' ')
      break;
  if (x == fore->w_width - 1)
    x = markdata->left_mar;
  return x;
}

static int
lineend(y)
int y;
{
  register int x;
  register unsigned char *i;

  for (x = markdata->right_mar, i = WIN(y)->image + x; x >= 0; x--)
    if (*i-- != ' ')
      break;
  if (x < 0)
    x = markdata->left_mar;
  return x;
}

/*
 * nextchar sets *xp to the num-th occurrence of the target in the line.
 *
 * Returns -1 if the target doesn't appear num times, 0 otherwise.
 */
static int
nextchar(int *xp, int *yp, int direction, char target, int num)
{
  int width;  /* width of the current window. */
  int x;      /* x coordinate of the current cursor position. */
  int step;   /* amount to increment x (+1 or -1) */
  int adjust; /* Final adjustment of cursor position. */
  char *displayed_line; /* Line in which search takes place. */
 
  debug("nextchar\n");
 
  x = *xp;
  step = 1;
  adjust = 0;
  width = fore->w_width;
  displayed_line = (char *)WIN(*yp) -> image;
 
  switch(direction) {
    case 't':
      adjust = -1; /* fall through */
    case 'f':
      step = 1;
      break;
    case 'T':
      adjust = 1; /* fall through */
    case 'F':
      step = -1;
      break;
    default:
      ASSERT(0);
  }
 
  x += step;
 
  debug1("ml->image = %s\n", displayed_line);
  debug2("num = %d, width = %d\n",num, width);
  debug2("x = %d target = %c\n", x, target );
 
  for ( ;x>=0 && x <= width; x += step) {
    if (displayed_line[x] == target) {
      if (--num == 0) {
        *xp = x + adjust;
        return 0;
      }
    }
  }
  return -1;
}

/*
 *  nextword calculates the cursor position of the num'th word.
 *  If the cursor is on a word, it counts as the first.
 *  NW_BACK:		search backward
 *  NW_ENDOFWORD:	find the end of the word
 *  NW_MUSTMOVE:	move at least one char
 *  NW_BIG:             match WORDs not words
 */

#define NW_BACK		(1<<0)
#define NW_ENDOFWORD	(1<<1)
#define NW_MUSTMOVE	(1<<2)
#define NW_BIG		(1<<3)



static void
nextword(xp, yp, flags, num)
int *xp, *yp, flags, num;
{
  int xx = fore->w_width, yy = fore->w_histheight + fore->w_height;
  register int sx, oq, q, x, y;
  struct mline *ml;

  x = *xp;
  y = *yp;
  sx = (flags & NW_BACK) ? -1 : 1;
  if ((flags & NW_ENDOFWORD) && (flags & NW_MUSTMOVE))
    x += sx;
  ml = WIN(y);
  for (oq = -1; ; x += sx, oq = q)
    {
      if (x >= xx || x < 0)
	q = 0;
      else if (flags & NW_BIG)
        q = ml->image[x] == ' ';
      else
        q = is_letter(ml->image[x]);
      if (oq >= 0 && oq != q)
	{
	  if (oq == 0 || !(flags & NW_ENDOFWORD))
	    *xp = x;
	  else
	    *xp = x-sx;
	  *yp = y;
	  if ((!(flags & NW_ENDOFWORD) && q) ||
	      ((flags & NW_ENDOFWORD) && oq))
	    {
	      if (--num <= 0)
	        return;
	    }
	}
      if (x == xx)
	{
	  x = -1;
	  if (++y >= yy)
	    return;
	  ml = WIN(y);
	}
      else if (x < 0)
	{
	  x = xx;
	  if (--y < 0)
	    return;
	  ml = WIN(y);
	}
    }
}


/*
 * y1, y2 are WIN coordinates
 *
 * redisplay:	0  -  just copy
 * 		1  -  redisplay + copy
 *		2  -  count + copy, don't redisplay
 */

static int
rem(x1, y1, x2, y2, redisplay, pt, yend)
int x1, y1, x2, y2, redisplay, yend;
char *pt;
{
  int i, j, from, to, ry, c;
  int l = 0;
  unsigned char *im;
  struct mline *ml;
#ifdef FONT
  int cf, cfx, font;
  unsigned char *fo, *fox;
#endif

  markdata->second = 0;
  if (y2 < y1 || ((y2 == y1) && (x2 < x1)))
    {
      i = y2;
      y2 = y1;
      y1 = i;
      i = x2;
      x2 = x1;
      x1 = i;
    }
  ry = y1 - markdata->hist_offset;

  i = y1;
  if (redisplay != 2 && pt == 0 && ry <0)
    {
      i -= ry;
      ry = 0;
    }
  for (; i <= y2; i++, ry++)
    {
      if (redisplay != 2 && pt == 0 && ry > yend)
	break;
      ml = WIN(i);
      from = (i == y1) ? x1 : 0;
      if (from < markdata->left_mar)
	from = markdata->left_mar;
      for (to = fore->w_width, im = ml->image + to; to >= 0; to--)
        if (*im-- != ' ')
	  break;
      if (i == y2 && x2 < to)
	to = x2;
      if (to > markdata->right_mar)
	to = markdata->right_mar;
      if (redisplay == 1 && from <= to && ry >=0 && ry <= yend)
	MarkRedisplayLine(ry, from, to, 0);
      if (redisplay != 2 && pt == 0)	/* don't count/copy */
	continue;
      j = from;
#ifdef DW_CHARS
      if (dw_right(ml, j, fore->w_encoding))
	j--;
#endif
      im = ml->image + j;
#ifdef FONT
      fo = ml->font + j;
      fox = ml->fontx + j;
      font = ASCII;
#endif
      for (; j <= to; j++)
	{
	  c = (unsigned char)*im++;
#ifdef FONT
	  cf = (unsigned char)*fo++;
	  cfx = (unsigned char)*fox++;
# ifdef UTF8
	  if (fore->w_encoding == UTF8)
	    {
	      c |= cf << 8 | cfx << 16;
	      if (c == UCS_HIDDEN)
		continue;
	      c = ToUtf8_comb(pt, c);
	      l += c;
	      if (pt)
	        pt += c;
	      continue;
	    }
# endif
# ifdef DW_CHARS
	  if (is_dw_font(cf))
	    {
	      c = c << 8 | (unsigned char)*im++;
	      fo++;
	      j++;
	    }
# endif
	  if (pastefont)
	    {
	      c = EncodeChar(pt, c | cf << 16, fore->w_encoding, &font);
	      l += c;
	      if (pt)
		pt += c;
	      continue;
	    }
#endif /* FONT */
	  if (pt)
	    *pt++ = c;
	  l++;
	}
#ifdef FONT
      if (pastefont && font != ASCII)
	{
	  if (pt)
	    {
	      strcpy(pt, "\033(B");
	      pt += 3;
	    }
	  l += 3;
	}
#endif
      if (i != y2 && (to != fore->w_width - 1 || ml->image[to + 1] == ' '))
	{
	  /*
	   * this code defines, what glues lines together
	   */
	  switch (markdata->nonl)
	    {
	    case 0:		/* lines separated by newlines */
	      if (pt)
		*pt++ = '\r';
	      l++;
	      if (join_with_cr)
		{
		  if (pt)
		    *pt++ = '\n';
		  l++;
		}
	      break;
	    case 1:		/* nothing to separate lines */
	      break;
	    case 2:		/* lines separated by blanks */
	      if (pt)
		*pt++ = ' ';
	      l++;
	      break;
	    case 3:		/* seperate by comma, for csh junkies */
	      if (pt)
	        *pt++ = ',';
	      l++;
	      break;
	    }
	}
    }
  return l;
}

/* Check if two chars are identical. All digits are treated
 * as same. Used for GetHistory()
 */

static int
eq(a, b)
int a, b;
{
  if (a == b)
    return 1;
  if (a == 0 || b == 0)
    return 1;
  if (a <= '9' && a >= '0' && b <= '9' && b >= '0')
    return 1;
  return 0;
}


/**********************************************************************/

int
GetHistory()	/* return value 1 if copybuffer changed */
{
  int i = 0, q = 0, xx, yy, x, y;
  unsigned char *linep;
  struct mline *ml;

  ASSERT(display && fore);
  x = fore->w_x;
  if (x >= fore->w_width)
    x = fore->w_width - 1;
  y = fore->w_y + fore->w_histheight;
  debug2("cursor is at x=%d, y=%d\n", x, y);
  ml = WIN(y);
  for (xx = x - 1, linep = ml->image + xx; xx >= 0; xx--)
    if ((q = *linep--) != ' ' )
      break;
  debug3("%c at (%d,%d)\n", q, xx, y);
  for (yy = y - 1; yy >= 0; yy--)
    {
      ml = WIN(yy);
      linep = ml->image;
      if (xx < 0 || eq(linep[xx], q))
	{		/* line is matching... */
	  for (i = fore->w_width - 1, linep += i; i >= x; i--)
	    if (*linep-- != ' ')
	      break;
	  if (i >= x)
	    break;
	}
    }
  if (yy < 0)
    return 0;
  if (D_user->u_plop.buf)
    UserFreeCopyBuffer(D_user);
  if ((D_user->u_plop.buf = (char *)malloc((unsigned) (i - x + 2))) == NULL)
    {
      LMsg(0, "Not enough memory... Sorry.");
      return 0;
    }
  bcopy((char *)linep - i + x + 1, D_user->u_plop.buf, i - x + 1);
  D_user->u_plop.len = i - x + 1;
#ifdef ENCODINGS
  D_user->u_plop.enc = fore->w_encoding;
#endif
  return 1;
}

/**********************************************************************/


void
MarkRoutine()
{
  int x, y;

  ASSERT(fore && display && D_user);

  debug2("MarkRoutine called: fore nr %d, display %s\n",
         fore->w_number, D_usertty);

  if (InitOverlayPage(sizeof(*markdata), &MarkLf, 1))
    return;
  flayer->l_encoding = fore->w_encoding;
  flayer->l_mode = 1;
  markdata = (struct markdata *)flayer->l_data;
  markdata->md_user = D_user;	/* XXX: Correct? */
  markdata->md_window = fore;
  markdata->second = 0;
  markdata->rep_cnt = 0;
  markdata->append_mode = 0;
  markdata->write_buffer = 0;
  markdata->nonl = 0;
  markdata->left_mar  = 0;
  markdata->right_mar = fore->w_width - 1;
  markdata->hist_offset = fore->w_histheight;
  x = fore->w_x;
  y = D2W(fore->w_y);
  if (x >= fore->w_width)
    x = fore->w_width - 1;

  LGotoPos(flayer, x, W2D(y));
  LMsg(0, "Copy mode - Column %d Line %d(+%d) (%d,%d)",
      x + 1, W2D(y + 1), fore->w_histheight, fore->w_width, fore->w_height);
  markdata->cx = markdata->x1 = x;
  markdata->cy = markdata->y1 = y;
  flayer->l_x = x;
  flayer->l_y = W2D(y);
}

static void
MarkProcess(inbufp,inlenp)
char **inbufp;
int *inlenp;
{
  char *inbuf, *pt;
  int inlen;
  int cx, cy, x2, y2, j, yend;
  int newcopylen = 0, od;
  int in_mark;
  int rep_cnt;
  struct acluser *md_user;

/*
  char *extrap = 0, extrabuf[100];
*/

  markdata = (struct markdata *)flayer->l_data;
  fore = markdata->md_window;
  md_user = markdata->md_user;
  if (inbufp == 0)
    {
      MarkAbort();
      return;
    }

  LGotoPos(flayer, markdata->cx, W2D(markdata->cy));
  inbuf= *inbufp;
  inlen= *inlenp;
  pt = inbuf;
  in_mark = 1;
  while (in_mark && (inlen /* || extrap */))
    {
      unsigned char ch = (unsigned char )*pt++;
      inlen--;
      if (flayer->l_mouseevent.start)
	{
	  int r = LayProcessMouse(flayer, ch);
	  if (r == -1)
	    LayProcessMouseSwitch(flayer, 0);
	  else
	    {
	      if (r)
		ch = 0222;
	      else
		continue;
	    }
	}
      od = mark_key_tab[(int)ch];
      rep_cnt = markdata->rep_cnt;
      if (od >= '0' && od <= '9' && !markdata->f_cmd.flag)
        {
	  if (rep_cnt < 1001 && (od != '0' || rep_cnt != 0))
	    {
	      markdata->rep_cnt = 10 * rep_cnt + od - '0';
	      continue;
 	      /*
	       * Now what is that 1001 here? Well, we have a screen with
	       * 25 * 80 = 2000 characters. Movement is at most across the full
	       * screen. This we do with word by word movement, as character by
	       * character movement never steps over line boundaries. The most words
	       * we can place on the screen are 1000 single letter words. Thus 1001
	       * is sufficient. Users with bigger screens never write in single letter
	       * words, as they should be more advanced. jw.
	       * Oh, wrong. We still give even the experienced user a factor of ten.
	       */
	    }
	}
      cx = markdata->cx;
      cy = markdata->cy;

      if (markdata -> f_cmd.flag) {
        debug2("searching for %c:%d\n",od,rep_cnt);
        markdata->f_cmd.flag = 0;
        markdata->rep_cnt = 0;

	if (isgraph (od)) {
	  markdata->f_cmd.target = od;
	  rep_cnt = (rep_cnt) ? rep_cnt : 1;
	  nextchar(&cx, &cy, markdata->f_cmd.direction, od, rep_cnt );
	  revto(cx, cy);
	  continue;
	}
      }

processchar:
      switch (od)
        {
	case 'f': /* fall through */
	case 'F': /* fall through */
	case 't': /* fall through */
	case 'T': /* fall through */
	  /*
	   * Set f_cmd to do a search on the next key stroke.
	   * If we break, rep_cnt will be reset, so we
	   * continue instead. It might be cleaner to
	   * store the rep_count in f_cmd and
	   * break here so later followon code will be
	   * hit.
	   */
	  markdata->f_cmd.flag = 1;
	  markdata->f_cmd.direction = od;
	  debug("entering char search\n");
	  continue;
	case ';':
	case ',':
	  if (!markdata->f_cmd.target)
	    break;
	  if (!rep_cnt)
	    rep_cnt = 1;
	  nextchar(&cx, &cy,
	      od == ';' ? markdata->f_cmd.direction : (markdata->f_cmd.direction ^ 0x20),
	      markdata->f_cmd.target, rep_cnt );
	  revto(cx, cy);
	  break;
	case 'o':
	case 'x':
	  if (!markdata->second)
	    break;
	  markdata->cx = markdata->x1;
	  markdata->cy = markdata->y1;
	  markdata->x1 = cx;
	  markdata->y1 = cy;
	  revto(markdata->cx, markdata->cy);
	  break;
	case '\014':	/* CTRL-L Redisplay */
	  Redisplay(0);
	  LGotoPos(flayer, cx, W2D(cy));
	  break;
	case 0202:	/* M-C-b */
	case '\010':	/* CTRL-H Backspace */
	case 'h':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  revto(cx - rep_cnt, cy);
	  break;
	case 0216:	/* M-C-p */
	case '\016':	/* CTRL-N */
	case 'j':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  revto(cx, cy + rep_cnt);
	  break;
	case '+':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  j = cy + rep_cnt;
	  if (j > fore->w_histheight + fore->w_height - 1)
	    j = fore->w_histheight + fore->w_height - 1;
	  revto(linestart(j), j);
	  break;
	case '-':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  cy -= rep_cnt;
	  if (cy < 0)
	    cy = 0;
	  revto(linestart(cy), cy);
	  break;
	case '^':
	  revto(linestart(cy), cy);
	  break;
	case '\n':
	  revto(markdata->left_mar, cy + 1);
	  break;
	case 0220:	/* M-C-p */
	case '\020':	/* CTRL-P */
	case 'k':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  revto(cx, cy - rep_cnt);
	  break;
	case 0206:	/* M-C-f */
	case 'l':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  revto(cx + rep_cnt, cy);
	  break;
	case '\001':	/* CTRL-A from tcsh/emacs */
	case '0':
	  revto(markdata->left_mar, cy);
	  break;
	case '\004':    /* CTRL-D down half screen */
	  if (rep_cnt == 0)
	    rep_cnt = (fore->w_height + 1) >> 1;
	  revto_line(cx, cy + rep_cnt, W2D(cy));
	  break;
	case '$':
	  revto(lineend(cy), cy);
	  break;
        case '\022':	/* CTRL-R emacs style backwards search */
	  ISearch(-1);
	  in_mark = 0;
	  break;
        case '\023':	/* CTRL-S emacs style search */
	  ISearch(1);
	  in_mark = 0;
	  break;
	case '\025':	/* CTRL-U up half screen */
	  if (rep_cnt == 0)
	    rep_cnt = (fore->w_height + 1) >> 1;
	  revto_line(cx, cy - rep_cnt, W2D(cy));
	  break;
	case '\007':	/* CTRL-G show cursorpos */
	  if (markdata->left_mar == 0 && markdata->right_mar == fore->w_width - 1)
	    LMsg(0, "Column %d Line %d(+%d)", cx+1, W2D(cy)+1,
		markdata->hist_offset);
	  else
	    LMsg(0, "Column %d(%d..%d) Line %d(+%d)", cx+1,
		markdata->left_mar+1, markdata->right_mar+1, W2D(cy)+1, markdata->hist_offset);
	  break;
	case '\002':	/* CTRL-B  back one page */
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  rep_cnt *= fore->w_height;
	  revto(cx, cy - rep_cnt);
	  break;
	case '\006':	/* CTRL-F  forward one page */
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  rep_cnt *= fore->w_height;
	  revto(cx, cy + rep_cnt);
	  break;
	case '\005':	/* CTRL-E  scroll up */
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  rep_cnt = MarkScrollUpDisplay(rep_cnt);
	  if (cy < D2W(0))
            revto(cx, D2W(0));
	  else
            LGotoPos(flayer, cx, W2D(cy));
	  break;
	case '\031': /* CTRL-Y  scroll down */
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  rep_cnt = MarkScrollDownDisplay(rep_cnt);
	  if (cy > D2W(fore->w_height-1))
            revto(cx, D2W(fore->w_height-1));
	  else
            LGotoPos(flayer, cx, W2D(cy));
	  break;
	case '@':
	  /* it may be useful to have a key that does nothing */
	  break;
	case '%':
	  /* rep_cnt is a percentage for the history buffer */
	  if (rep_cnt < 0)
	    rep_cnt = 0;
	  if (rep_cnt > 100)
	    rep_cnt = 100;
	  revto_line(markdata->left_mar,
		     fore->w_histheight - fore->w_scrollback_height +
		     (int)(rep_cnt * (fore->w_scrollback_height + fore->w_height) / 100.0),
		     (fore->w_height - 1) / 2);
	  break;
	case 0201:
	case 'g':
	  rep_cnt = 1;
	  /* FALLTHROUGH */
	case 0205:
	case 'G':
	  /* rep_cnt is here the WIN line number */
	  if (rep_cnt == 0)
	    rep_cnt = fore->w_histheight + fore->w_height;
	  revto_line(markdata->left_mar, --rep_cnt, (fore->w_height - 1) / 2);
	  break;
	case 'H':
	  revto(markdata->left_mar, D2W(0));
	  break;
	case 'M':
	  revto(markdata->left_mar, D2W((fore->w_height - 1) / 2));
	  break;
	case 'L':
	  revto(markdata->left_mar, D2W(fore->w_height - 1));
	  break;
	case '|':
	  revto(--rep_cnt, cy);
	  break;
	case 'w':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  nextword(&cx, &cy, NW_MUSTMOVE, rep_cnt);
	  revto(cx, cy);
	  break;
	case 'e':
	case 'E':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  nextword(&cx, &cy, NW_ENDOFWORD|NW_MUSTMOVE | (od == 'E' ? NW_BIG : 0), rep_cnt);
	  revto(cx, cy);
	  break;
	case 'b':
	case 'B':
	  if (rep_cnt == 0)
	    rep_cnt = 1;
	  nextword(&cx, &cy, NW_BACK|NW_ENDOFWORD|NW_MUSTMOVE | (od == 'B' ? NW_BIG : 0), rep_cnt);
	  revto(cx, cy);
	  break;
	case 'a':
	  markdata->append_mode = 1 - markdata->append_mode;
	  debug1("append mode %d--\n", markdata->append_mode);
	  LMsg(0, (markdata->append_mode) ? ":set append" : ":set noappend");
	  break;
	case 'v':
	case 'V':
	  /* this sets start column to column 9 for VI :set nu users */
	  if (markdata->left_mar == 8)
	    rep_cnt = 1;
	  else
	    rep_cnt = 9;
	  /* FALLTHROUGH */
	case 'c':
	case 'C':
	  /* set start column (c) and end column (C) */
	  if (markdata->second)
	    {
	      rem(markdata->x1, markdata->y1, cx, cy, 1, (char *)0, fore->w_height-1); /* Hack */
	      markdata->second = 1;	/* rem turns off second */
	    }
	  rep_cnt--;
	  if (rep_cnt < 0)
	    rep_cnt = cx;
	  if (od != 'C')
	    {
	      markdata->left_mar = rep_cnt;
	      if (markdata->left_mar > markdata->right_mar)
		markdata->left_mar = markdata->right_mar;
	    }
	  else
	    {
	      markdata->right_mar = rep_cnt;
	      if (markdata->left_mar > markdata->right_mar)
		markdata->right_mar = markdata->left_mar;
	    }
	  if (markdata->second)
	    {
	      markdata->cx = markdata->x1; markdata->cy = markdata->y1;
	      revto(cx, cy);
	    }
	  if (od == 'v' || od == 'V')
	    LMsg(0, (markdata->left_mar != 8) ? ":set nonu" : ":set nu");
	  break;
	case 'J':
	  /* how do you join lines in VI ? */
	  markdata->nonl = (markdata->nonl + 1) % 4;
	  switch (markdata->nonl)
	    {
	    case 0:
	      if (join_with_cr)
		LMsg(0, "Multiple lines (CR/LF)");
	      else
		LMsg(0, "Multiple lines (LF)");
	      break;
	    case 1:
	      LMsg(0, "Lines joined");
	      break;
	    case 2:
	      LMsg(0, "Lines joined with blanks");
	      break;
	    case 3:
	      LMsg(0, "Lines joined with comma");
	      break;
	    }
	  break;
	case '/':
	  Search(1);
	  in_mark = 0;
	  break;
	case '?':
	  Search(-1);
	  in_mark = 0;
	  break;
	case 'n':
	  Search(0);
	  break;
	case 'N':
	  markdata->isdir = -markdata->isdir;
	  Search(0);
	  markdata->isdir = -markdata->isdir;
	  break;
	case 'y':
	case 'Y':
	  if (markdata->second == 0)
	    {
	      revto(linestart(cy), cy);
	      markdata->second++;
	      cx = markdata->x1 = markdata->cx;
	      cy = markdata->y1 = markdata->cy;
	    }
	  if (--rep_cnt > 0)
	    revto(cx, cy + rep_cnt);
	  revto(lineend(markdata->cy), markdata->cy);
	  if (od == 'y')
	    break;
	  /* FALLTHROUGH */
	case 'W':
	  if (od == 'W')
	    {
	      if (rep_cnt == 0)
		rep_cnt = 1;
	      if (!markdata->second)
		{
		  nextword(&cx, &cy, NW_BACK|NW_ENDOFWORD, 1);
		  revto(cx, cy);
		  markdata->second++;
		  cx = markdata->x1 = markdata->cx;
		  cy = markdata->y1 = markdata->cy;
		}
	      nextword(&cx, &cy, NW_ENDOFWORD, rep_cnt);
	      revto(cx, cy);
	    }
	  cx = markdata->cx;
	  cy = markdata->cy;
	  /* FALLTHROUGH */
	case 'A':
	  if (od == 'A')
	    markdata->append_mode = 1;
	  /* FALLTHROUGH */
	case '>':
	  if (od == '>')
	    markdata->write_buffer = 1;
	  /* FALLTHROUGH */
	case ' ':
	case '\r':
	  if (!markdata->second)
	    {
	      markdata->second++;
	      markdata->x1 = cx;
	      markdata->y1 = cy;
	      revto(cx, cy);
	      LMsg(0, "First mark set - Column %d Line %d", cx+1, W2D(cy)+1);
	      break;
	    }
	  else
	    {
	      int append_mode = markdata->append_mode;
	      int write_buffer = markdata->write_buffer;

	      x2 = cx;
	      y2 = cy;
	      newcopylen = rem(markdata->x1, markdata->y1, x2, y2, 2, (char *)0, 0); /* count */
	      if (md_user->u_plop.buf && !append_mode)
		UserFreeCopyBuffer(md_user);
	      yend = fore->w_height - 1;
	      if (fore->w_histheight - markdata->hist_offset < fore->w_height)
		{
		  markdata->second = 0;
		  yend -= MarkScrollUpDisplay(fore->w_histheight - markdata->hist_offset);
		}
	      if (newcopylen > 0)
		{
		  /* the +3 below is for : cr + lf + \0 */
		  if (md_user->u_plop.buf)
		    md_user->u_plop.buf = realloc(md_user->u_plop.buf,
			(unsigned) (md_user->u_plop.len + newcopylen + 3));
		  else
		    {
		      md_user->u_plop.len = 0;
		      md_user->u_plop.buf = malloc((unsigned) (newcopylen + 3));
		    }
		  if (!md_user->u_plop.buf)
		    {
		      MarkAbort();
		      in_mark = 0;
		      LMsg(0, "Not enough memory... Sorry.");
		      md_user->u_plop.len = 0;
		      md_user->u_plop.buf = 0;
		      break;
		    }
		  if (append_mode)
		    {
		      switch (markdata->nonl)
			{
			/*
			 * this code defines, what glues lines together
			 */
			case 0:
			  if (join_with_cr)
			    {
			      md_user->u_plop.buf[md_user->u_plop.len] = '\r';
			      md_user->u_plop.len++;
			    }
			  md_user->u_plop.buf[md_user->u_plop.len] = '\n';
			  md_user->u_plop.len++;
			  break;
			case 1:
			  break;
			case 2:
			  md_user->u_plop.buf[md_user->u_plop.len] = ' ';
			  md_user->u_plop.len++;
			  break;
			case 3:
			  md_user->u_plop.buf[md_user->u_plop.len] = ',';
			  md_user->u_plop.len++;
			  break;
			}
		    }
		  md_user->u_plop.len += rem(markdata->x1, markdata->y1, x2, y2,
		    markdata->hist_offset == fore->w_histheight,
		    md_user->u_plop.buf + md_user->u_plop.len, yend);
#ifdef ENCODINGS
		  md_user->u_plop.enc = fore->w_encoding;
#endif
		}
	      if (markdata->hist_offset != fore->w_histheight)
		{
		  LAY_CALL_UP(LRefreshAll(flayer, 0));
		}
	      ExitOverlayPage();
	      WindowChanged(fore, 'P');
	      if (append_mode)
		LMsg(0, "Appended %d characters to buffer",
		    newcopylen);
	      else
		LMsg(0, "Copied %d characters into buffer", md_user->u_plop.len);
	      if (write_buffer)
		WriteFile(md_user, (char *)0, DUMP_EXCHANGE);
	      in_mark = 0;
	      break;
	    }

	case 0222:
	  if (flayer->l_mouseevent.start)
	    {
	      int button = flayer->l_mouseevent.buffer[0];
	      if (button == 'a')
		{
		  /* Scroll down */
		  od = 'j';
		}
	      else if (button == '`')
		{
		  /* Scroll up */
		  od = 'k';
		}
	      else if (button == ' ')
		{
		  /* Left click */
		  cx = flayer->l_mouseevent.buffer[1];
		  cy = D2W(flayer->l_mouseevent.buffer[2]);
		  revto(cx, cy);
		  od = ' ';
		}
	      else
		od = 0;
	      LayProcessMouseSwitch(flayer, 0);
	      if (od)
		goto processchar;
	    }
	  else
	    LayProcessMouseSwitch(flayer, 1);
	  break;

	default:
	  MarkAbort();
	  LMsg(0, "Copy mode aborted");
	  in_mark = 0;
	  break;
	}
      if (in_mark)	/* markdata may be freed */
        markdata->rep_cnt = 0;
    }
  if (in_mark)
    {
      flayer->l_x = markdata->cx;
      flayer->l_y = W2D(markdata->cy);
    }
  *inbufp = pt;
  *inlenp = inlen;
}

void revto(tx, ty)
int tx, ty;
{
  revto_line(tx, ty, -1);
}

/* tx, ty: WINDOW,  line: DISPLAY */
void revto_line(tx, ty, line)
int tx, ty, line;
{
  int fx, fy;
  int x, y, t, revst, reven, qq, ff, tt, st, en, ce = 0;
  int ystart = 0, yend = fore->w_height-1;
  int i, ry;
  unsigned char *wi;
  struct mline *ml;
  struct mchar mc;

  if (tx < 0)
    tx = 0;
  else if (tx > fore->w_width - 1)
    tx = fore->w_width -1;
  if (ty < fore->w_histheight - fore->w_scrollback_height)
    ty = fore->w_histheight - fore->w_scrollback_height;
  else if (ty > fore->w_histheight + fore->w_height - 1)
    ty = fore->w_histheight + fore->w_height - 1;

  fx = markdata->cx; fy = markdata->cy;

#ifdef DW_CHARS
  /* don't just move inside of a kanji, the user wants to see something */
  ml = WIN(ty);
  if (ty == fy && fx + 1 == tx && dw_right(ml, tx, fore->w_encoding) && tx < D_width - 1)
    tx++;
  if (ty == fy && fx - 1 == tx && dw_right(ml, fx, fore->w_encoding) && tx)
    tx--;
#endif

  markdata->cx = tx; markdata->cy = ty;

  /*
   * if we go to a position that is currently offscreen
   * then scroll the screen
   */
  i = 0;
  if (line >= 0 && line < fore->w_height)
    i = W2D(ty) - line;
  else if (ty < markdata->hist_offset)
    i = ty - markdata->hist_offset;
  else if (ty > markdata->hist_offset + (fore->w_height - 1))
    i = ty - markdata->hist_offset - (fore->w_height - 1);
  if (i > 0)
    yend -= MarkScrollUpDisplay(i);
  else if (i < 0)
    ystart += MarkScrollDownDisplay(-i);

  if (markdata->second == 0)
    {
      flayer->l_x = tx;
      flayer->l_y = W2D(ty);
      LGotoPos(flayer, tx, W2D(ty));
      return;
    }

  qq = markdata->x1 + markdata->y1 * fore->w_width;
  ff = fx + fy * fore->w_width; /* "from" offset in WIN coords */
  tt = tx + ty * fore->w_width; /* "to" offset  in WIN coords*/

  if (ff > tt)
    {
      st = tt; en = ff;
      x = tx; y = ty;
    }
  else
    {
      st = ff; en = tt;
      x = fx; y = fy;
    }
  if (st > qq)
    {
      st++;
      x++;
    }
  if (en < qq)
    en--;
  if (tt > qq)
    {
      revst = qq; reven = tt;
    }
  else
    {
      revst = tt; reven = qq;
    }
  ry = y - markdata->hist_offset;
  if (ry < ystart)
    {
      y += (ystart - ry);
      x = 0;
      st = y * fore->w_width;
      ry = ystart;
    }
  ml = WIN(y);
  for (t = st; t <= en; t++, x++)
    {
      if (x >= fore->w_width)
	{
	  x = 0;
	  y++, ry++;
	  ml = WIN(y);
	}
      if (ry > yend)
	break;
      if (t == st || x == 0)
	{
	  wi = ml->image + fore->w_width;
	  for (ce = fore->w_width; ce >= 0; ce--, wi--)
	    if (*wi != ' ')
	      break;
	}
      if (x <= ce && x >= markdata->left_mar && x <= markdata->right_mar)
	{
#ifdef DW_CHARS
	  if (dw_right(ml, x, fore->w_encoding))
	      {
		if (t == revst)
		  revst--;
	        t--;
		x--;
	      }
#endif
	  if (t >= revst && t <= reven)
	    {
	      mc = mchar_so;
#ifdef FONT
	      if (pastefont)
		{
		  mc.font = ml->font[x];
		  mc.fontx = ml->fontx[x];
		}
#endif
	      mc.image = ml->image[x];
	    }
	  else
	    copy_mline2mchar(&mc, ml, x);
#ifdef DW_CHARS
	  if (dw_left(ml, x, fore->w_encoding))
	    {
	      mc.mbcs = ml->image[x + 1];
	      LPutChar(flayer, &mc, x, W2D(y));
	      t++;
	    }
#endif
	  LPutChar(flayer, &mc, x, W2D(y));
#ifdef DW_CHARS
	  if (dw_left(ml, x, fore->w_encoding))
	    x++;
#endif
	}
    }
  flayer->l_x = tx;
  flayer->l_y = W2D(ty);
  LGotoPos(flayer, tx, W2D(ty));
}

static void
MarkAbort()
{
  int yend, redisp;

  debug("MarkAbort\n");
  markdata = (struct markdata *)flayer->l_data;
  fore = markdata->md_window;
  yend = fore->w_height - 1;
  redisp = markdata->second;
  if (fore->w_histheight - markdata->hist_offset < fore->w_height)
    {
      markdata->second = 0;
      yend -= MarkScrollUpDisplay(fore->w_histheight - markdata->hist_offset);
    }
  if (markdata->hist_offset != fore->w_histheight)
    {
      LAY_CALL_UP(LRefreshAll(flayer, 0));
    }
  else
    {
      rem(markdata->x1, markdata->y1, markdata->cx, markdata->cy, redisp, (char *)0, yend);
    }
  ExitOverlayPage();
  WindowChanged(fore, 'P');
}


static void
MarkRedisplayLine(y, xs, xe, isblank)
int y;	/* NOTE: y is in DISPLAY coords system! */
int xs, xe;
int isblank;
{
  int wy, x, i, rm;
  int sta, sto, cp;	/* NOTE: these 3 are in WINDOW coords system */
  unsigned char *wi;
  struct mline *ml;
  struct mchar mchar_marked;

  if (y < 0)	/* No special full page handling */
    return;

  markdata = (struct markdata *)flayer->l_data;
  fore = markdata->md_window;

  mchar_marked = mchar_so;

  wy = D2W(y);
  ml = WIN(wy);

  if (markdata->second == 0)
    {
      if (dw_right(ml, xs, fore->w_encoding) && xs > 0)
	xs--;
      if (dw_left(ml, xe, fore->w_encoding) && xe < fore->w_width - 1)
	xe++;
      if (xs == 0 && y > 0 && wy > 0 && WIN(wy - 1)->image[flayer->l_width] == 0)
        LCDisplayLineWrap(flayer, ml, y, xs, xe, isblank);
      else
        LCDisplayLine(flayer, ml, y, xs, xe, isblank);
      return;
    }

  sta = markdata->y1 * fore->w_width + markdata->x1;
  sto = markdata->cy * fore->w_width + markdata->cx;
  if (sta > sto)
    {
      i=sta; sta=sto; sto=i;
    }
  cp = wy * fore->w_width + xs;

  rm = markdata->right_mar;
  for (x = fore->w_width, wi = ml->image + fore->w_width; x >= 0; x--, wi--)
    if (*wi != ' ')
      break;
  if (x < rm)
    rm = x;

  for (x = xs; x <= xe; x++, cp++)
    if (cp >= sta && x >= markdata->left_mar)
      break;
#ifdef DW_CHARS
  if (dw_right(ml, x, fore->w_encoding))
    x--;
#endif
  if (x > xs)
    LCDisplayLine(flayer, ml, y, xs, x - 1, isblank);
  for (; x <= xe; x++, cp++)
    {
      if (cp > sto || x > rm)
	break;
#ifdef FONT
      if (pastefont)
	{
	  mchar_marked.font = ml->font[x];
	  mchar_marked.fontx = ml->fontx[x];
	}
#endif
      mchar_marked.image = ml->image[x];
#ifdef DW_CHARS
      mchar_marked.mbcs = 0;
      if (dw_left(ml, x, fore->w_encoding))
	{
	  mchar_marked.mbcs = ml->image[x + 1];
	  cp++;
	}
#endif
      LPutChar(flayer, &mchar_marked, x, y);
#ifdef DW_CHARS
      if (dw_left(ml, x, fore->w_encoding))
	x++;
#endif
    }
  if (x <= xe)
    LCDisplayLine(flayer, ml, y, x, xe, isblank);
}


/*
 * This ugly routine is to speed up GotoPos()
 */
static int
MarkRewrite(ry, xs, xe, rend, doit)
int ry, xs, xe, doit;
struct mchar *rend;
{
  int dx, x, y, st, en, t, rm;
  unsigned char *i;
  struct mline *ml;
  struct mchar mchar_marked;

  mchar_marked = mchar_so;

  debug3("MarkRewrite %d, %d-%d\n", ry, xs, xe);
  markdata = (struct markdata *)flayer->l_data;
  fore = markdata->md_window;
  y = D2W(ry);
  ml = WIN(y);
#ifdef UTF8
  if (fore->w_encoding && fore->w_encoding != UTF8 && D_encoding == UTF8 && ContainsSpecialDeffont(ml, xs, xe, fore->w_encoding))
    return EXPENSIVE;
#endif
  dx = xe - xs + 1;
  if (doit)
    {
      i = ml->image + xs;
      while (dx--)
        PUTCHAR(*i++);
      return 0;
    }

  if (markdata->second == 0)
    st = en = -1;
  else
    {
      st = markdata->y1 * fore->w_width + markdata->x1;
      en = markdata->cy * fore->w_width + markdata->cx;
      if (st > en)
        {
          t = st; st = en; en = t;
        }
    }
  t = y * fore->w_width + xs;
  for (rm = fore->w_width, i = ml->image + fore->w_width; rm >= 0; rm--)
    if (*i-- != ' ')
      break;
  if (rm > markdata->right_mar)
    rm = markdata->right_mar;
  x = xs;
  while (dx--)
    {
      if (t >= st && t <= en && x >= markdata->left_mar && x <= rm)
        {
#ifdef FONT
	  if (pastefont)
	    {
	      mchar_marked.font = ml->font[x];
	      mchar_marked.fontx = ml->fontx[x];
	    }
#endif
	  rend->image = mchar_marked.image;
	  if (!cmp_mchar(rend, &mchar_marked))
	    return EXPENSIVE;
        }
      else
        {
	  rend->image = ml->image[x];
	  if (!cmp_mchar_mline(rend, ml, x))
	    return EXPENSIVE;
        }
      x++;
    }
  return xe - xs + 1;
}


/*
 * scroll the screen contents up/down.
 */
static int MarkScrollUpDisplay(n)
int n;
{
  int i;

  debug1("MarkScrollUpDisplay(%d)\n", n);
  if (n <= 0)
    return 0;
  if (n > fore->w_histheight - markdata->hist_offset)
    n = fore->w_histheight - markdata->hist_offset;
  markdata->hist_offset += n;
  i = (n < flayer->l_height) ? n : (flayer->l_height);
  LScrollV(flayer, i, 0, flayer->l_height - 1, 0);
  while (i-- > 0)
    MarkRedisplayLine(flayer->l_height - i - 1, 0, flayer->l_width - 1, 1);
  return n;
}

static int
MarkScrollDownDisplay(n)
int n;
{
  int i;

  debug1("MarkScrollDownDisplay(%d)\n", n);
  if (n <= 0)
    return 0;
  if (n > markdata->hist_offset)
    n = markdata->hist_offset;
  markdata->hist_offset -= n;
  i = (n < flayer->l_height) ? n : (flayer->l_height);
  LScrollV(flayer, -i, 0, fore->w_height - 1, 0);
  while (i-- > 0)
    MarkRedisplayLine(i, 0, flayer->l_width - 1, 1);
  return n;
}

void
MakePaster(pa, buf, len, bufiscopy)
struct paster *pa;
char *buf;
int len;
int bufiscopy;
{
  FreePaster(pa);
  pa->pa_pasteptr = buf;
  pa->pa_pastelen = len;
  if (bufiscopy)
    pa->pa_pastebuf = buf;
  pa->pa_pastelayer = flayer;
  DoProcess(Layer2Window(flayer), &pa->pa_pasteptr, &pa->pa_pastelen, pa);
}

void
FreePaster(pa)
struct paster *pa;
{
  if (pa->pa_pastebuf)
    free(pa->pa_pastebuf);
  pa->pa_pastebuf = 0;
  pa->pa_pasteptr = 0;
  pa->pa_pastelen = 0;
  pa->pa_pastelayer = 0;
  evdeq(&pa->pa_slowev);
}

#endif /* COPY_PASTE */

