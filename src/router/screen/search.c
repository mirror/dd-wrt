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

#include "config.h"
#include "screen.h"
#include "mark.h"
#include "extern.h"

#define INPUTLINE (flayer->l_height - 1)

extern struct layer *flayer;
extern struct win *fore;

#ifdef COPY_PASTE

int search_ic;

/********************************************************************
 *  VI style Search
 */

static int  matchword __P((char *, int, int, int));
static void searchend __P((char *, int, char *));
static void backsearchend __P((char *, int, char *));

void
Search(dir)
int dir;
{
  struct markdata *markdata;
  if (dir == 0)
    {
      markdata = (struct markdata *)flayer->l_data;
      if (markdata->isdir > 0)
	searchend(0, 0, NULL);
      else if (markdata->isdir < 0)
	backsearchend(0, 0, NULL);
      else
	LMsg(0, "No previous pattern");
    }
  else
    Input((dir > 0 ? "/" : "?"), sizeof(markdata->isstr)-1, INP_COOKED,
          (dir > 0 ? searchend : backsearchend), NULL, 0);
}

static void
searchend(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  int x = 0, sx, ex, y;
  struct markdata *markdata;
  struct win *p;

  markdata = (struct markdata *)flayer->l_data;
  p = markdata->md_window;
  markdata->isdir = 1;
  if (len)
    strcpy(markdata->isstr, buf);
  sx = markdata->cx + 1;
  ex = flayer->l_width - 1;
  for (y = markdata->cy; y < p->w_histheight + flayer->l_height; y++, sx = 0)
    {
      if ((x = matchword(markdata->isstr, y, sx, ex)) >= 0)
        break;
    }
  if (y >= p->w_histheight + flayer->l_height)
    {
      LGotoPos(flayer, markdata->cx, W2D(markdata->cy));
      LMsg(0, "Pattern not found");
    }
  else
    revto(x, y);
}

static void
backsearchend(buf, len, data)
char *buf;
int len;
char *data;	/* dummy */
{
  int sx, ex, x = -1, y;
  struct markdata *markdata;

  markdata = (struct markdata *)flayer->l_data;
  markdata->isdir = -1;
  if (len)
    strcpy(markdata->isstr, buf);
  ex = markdata->cx - 1;
  for (y = markdata->cy; y >= 0; y--, ex = flayer->l_width - 1)
    {
      sx = 0;
      while ((sx = matchword(markdata->isstr, y, sx, ex)) >= 0)
	x = sx++;
      if (x >= 0)
	break;
    }
  if (y < 0)
    {
      LGotoPos(flayer, markdata->cx, W2D(markdata->cy));
      LMsg(0, "Pattern not found");
    }
  else
    revto(x, y);
}


/*
 * Search for a string that matches pattern.  The first character of the
 * match must be on line y, between columns sx and ex (inclusive), but
 * the rest of the match can extend beyond column ex and even onto the
 * following line.  Returns the starting column of the first match found,
 * or -1 if there's no match.
 */
static int
matchword(pattern, y, sx, ex)
char *pattern;
int y, sx, ex;
{
  unsigned char *ip, *cp, *pp, *cpe;
  int cy;

  fore = ((struct markdata *)flayer->l_data)->md_window;

  for (;sx <= ex; sx++)
    {
			cy = y;
			cp = WIN(cy)->image + sx;
			cpe = WIN(cy)->image + flayer->l_width;
      pp = (unsigned char *)pattern;
      for (;;)
	{
	  if (*cp != *pp)
	    if (!search_ic || ((*cp ^ *pp) & 0xdf) || (*cp | 0x20) < 'a' || (*cp | 0x20) > 'z')
	      break;
	  cp++;
	  pp++;
	  if (*pp == 0)
	    return sx;
		if (cp == cpe) {
		/*
		* We have a partial match, but we've hit
		* the end of this line.  Does it wrap onto
		* the following line?  If not, we're done.
		*/
		if (*cp == ' ' || cy >= fore->w_histheight + flayer->l_height - 1)
			break;

		/*
		* This line does wrap, so look on the next
		* line for the rest of our match.
		*/
		cy++;
		cp = WIN(cy)->image;
		cpe = WIN(cy)->image + flayer->l_width;
		}
	}
    }
  return -1;
}


/********************************************************************
 *  Emacs style ISearch
 */

static char *isprompts[] = {
  "I-search backward: ", "failing I-search backward: ",
  "I-search: ", "failing I-search: "
};


static int  is_redo __P((struct markdata *));
static void is_process __P((char *, int, char *));
static int  is_bm __P((char *, int, int, int, int));


static int
is_bm(str, l, p, end, dir)
char *str;
int l, p, end, dir;
{
  int tab[256];
  int i, q;
  unsigned char *s, c;
  int w = flayer->l_width;

  /* *sigh* to make WIN work */
  fore = ((struct markdata *)flayer->l_next->l_data)->md_window;
  debug2("is_bm: searching for %s len %d\n", str, l);
  debug3("start at %d end %d dir %d\n", p, end, dir);
  if (p < 0 || p + l > end)
    return -1;
  if (l == 0)
    return p;
  if (dir < 0)
    str += l - 1;
  for (i = 0; i < 256; i++)
    tab[i] = l * dir;
  for (i = 0; i < l - 1; i++, str += dir)
    {
      q = *(unsigned char *)str;
      tab[q] = (l - 1 - i) * dir;
      if (search_ic && (q | 0x20) >= 'a' && ((q | 0x20) <= 'z'))
        tab[q ^ 0x20] = (l - 1 - i) * dir;
    }
  if (dir > 0)
    p += l - 1;
  debug1("first char to match: %c\n", *str);
  while (p >= 0 && p < end)
    {
      q = p;
      s = (unsigned char *)str;
      for (i = 0;;)
	{
          c = (WIN(q / w))->image[q % w];
	  if (i == 0)
            p += tab[(int)(unsigned char) c];
	  if (c != *s)
	    if (!search_ic || ((c ^ *s) & 0xdf) || (c | 0x20) < 'a' || (c | 0x20) > 'z')
	      break;
	  q -= dir;
	  s -= dir;
	  if (++i == l)
	    return q + (dir > 0 ? 1 : -l);
	}
    }
  return -1;
}


/*ARGSUSED*/
static void
is_process(p, n, data)	/* i-search */
char *p;
int n;
char *data;	/* dummy */
{
  int pos, x, y, dir;
  struct markdata *markdata;

  if (n == 0)
    return;
  ASSERT(p);
  markdata = (struct markdata *)flayer->l_next->l_data;

  pos = markdata->cx + markdata->cy * flayer->l_width;
  LGotoPos(flayer, markdata->cx, W2D(markdata->cy));

  switch (*p)
    {
    case '\007':	/* CTRL-G */
      pos = markdata->isstartpos;
      /*FALLTHROUGH*/
    case '\033':	/* ESC */
      *p = 0;
      break;
    case '\013':	/* CTRL-K */
    case '\027':	/* CTRL-W */
      markdata->isistrl = 1;
      /*FALLTHROUGH*/
    case '\b':
    case '\177':
      if (markdata->isistrl == 0)
	return;
      markdata->isistrl--;
      pos = is_redo(markdata);
      *p = '\b';
      break;
    case '\023':	/* CTRL-S */
    case '\022': 	/* CTRL-R */
      if (markdata->isistrl >= (int)sizeof(markdata->isistr))
	return;
      dir = (*p == '\023') ? 1 : -1;
      pos += dir;
      if (markdata->isdir == dir && markdata->isistrl == 0)
	{
	  strcpy(markdata->isistr, markdata->isstr);
	  markdata->isistrl = markdata->isstrl = strlen(markdata->isstr);
	  break;
	}
      markdata->isdir = dir;
      markdata->isistr[markdata->isistrl++] = *p;
      break;
    default:
      if (*p < ' ' || markdata->isistrl >= (int)sizeof(markdata->isistr)
	  || markdata->isstrl >= (int)sizeof(markdata->isstr) - 1)
	return;
      markdata->isstr[markdata->isstrl++] = *p;
      markdata->isistr[markdata->isistrl++] = *p;
      markdata->isstr[markdata->isstrl] = 0;
      debug2("New char: %c - left %d\n", *p, (int)sizeof(markdata->isistr) - markdata->isistrl);
    }
  if (*p && *p != '\b')
    pos = is_bm(markdata->isstr, markdata->isstrl, pos, flayer->l_width * (markdata->md_window->w_histheight + flayer->l_height), markdata->isdir);
  if (pos >= 0)
    {
      x = pos % flayer->l_width;
      y = pos / flayer->l_width;
      LAY_CALL_UP
	(
          LayRedisplayLine(INPUTLINE, 0, flayer->l_width - 1, 0);
          revto(x, y);
          if (W2D(markdata->cy) == INPUTLINE)
	    revto_line(markdata->cx, markdata->cy, INPUTLINE > 0 ? INPUTLINE - 1 : 1);
        );
    }
  if (*p)
    inp_setprompt(isprompts[markdata->isdir + (pos < 0) + 1], markdata->isstrl ? markdata->isstr : "");
  flayer->l_x = markdata->cx;
  flayer->l_y = W2D(markdata->cy);
  LGotoPos(flayer, flayer->l_x, flayer->l_y);
  if (!*p)
    {
      /* we are about to finish, keep cursor position */
      flayer->l_next->l_x = markdata->cx;
      flayer->l_next->l_y = W2D(markdata->cy);
    }
}

static int
is_redo(markdata)
struct markdata *markdata;
{
  int i, pos, npos, dir;
  char c;

  npos = pos = markdata->isstartpos;
  dir = markdata->isstartdir;
  markdata->isstrl = 0;
  for (i = 0; i < markdata->isistrl; i++)
    {
      c = markdata->isistr[i];
      if (c == '\022')		/* ^R */
	pos += (dir = -1);
      else if (c == '\023')	/* ^S */
	pos += (dir = 1);
      else
	markdata->isstr[markdata->isstrl++] = c;
      if (pos >= 0)
	{
          npos = is_bm(markdata->isstr, markdata->isstrl, pos, flayer->l_width * (markdata->md_window->w_histheight + flayer->l_height), dir);
	  if (npos >= 0)
	    pos = npos;
	}
    }
  markdata->isstr[markdata->isstrl] = 0;
  markdata->isdir = dir;
  return npos;
}

void
ISearch(dir)
int dir;
{
  struct markdata *markdata;

  markdata = (struct markdata *)flayer->l_data;
  markdata->isdir = markdata->isstartdir = dir;
  markdata->isstartpos = markdata->cx + markdata->cy * flayer->l_width;
  markdata->isistrl = markdata->isstrl = 0;
  if (W2D(markdata->cy) == INPUTLINE)
    revto_line(markdata->cx, markdata->cy, INPUTLINE > 0 ? INPUTLINE - 1 : 1);
  Input(isprompts[dir + 1], sizeof(markdata->isstr) - 1, INP_RAW,
        is_process, NULL, 0);
  LGotoPos(flayer, markdata->cx, W2D(markdata->cy));
  flayer->l_x = markdata->cx;
  flayer->l_y = W2D(markdata->cy);
}

#endif /* COPY_PASTE */
