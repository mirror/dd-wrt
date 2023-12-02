/*
 * A braille interface to unix tty terminals
 *
 * Authors:  Hadi Bargi Rangin  bargi@dots.physics.orst.edu
 *           Bill Barry         barryb@dots.physics.orst.edu
 *
 * Copyright (c) 1995 by Science Access Project, Oregon State University.
 *
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "config.h"
#include "screen.h"
#include "extern.h"
#include "braille.h"

#ifdef HAVE_BRAILLE


extern int bd_init_powerbraille_40 __P((void));
extern int bd_init_powerbraille_80 __P((void));
extern int bd_init_navigator_40 __P((void));

extern struct layer *flayer;
extern struct display *displays, *display; 
extern char *rc_name;




/* global variables */

struct braille_display bd;

struct bd_type {
  char *name;
  int (*init) __P((void));
};

static struct bd_type bd_typelist[] = 
{
  {"powerbraille_40", bd_init_powerbraille_40}, 
  {"powerbraille_80", bd_init_powerbraille_80}, 
  {"navigator_40"   , bd_init_navigator_40}
};

static void position_braille_cursor __P((void));
static int  initialize_braille_display_type __P((char *));
static int  open_braille_device __P(());
static int  load_braille_table __P((char *));
static void bd_signal __P((void));
static void bd_bc_left __P((void));
static void bd_bc_right __P((void));
static void bd_bc_up __P((void));
static void bd_bc_down __P((void));
static void bd_upper_left __P((void));
static void bd_upper_right __P((void));
static void bd_lower_left __P((void));
static void bd_lower_right __P((void));
static int  bd_do_search __P((int, int, int));
static void bd_normalize __P((int, int));
static void bd_readev_fn __P((struct event *, char *));
static void bd_writeev_fn __P((struct event *, char *));
static void bd_selectev_fn __P((struct event *, char *));

static unsigned char btable_local [] = 
{
  0xC8,0xC1,0xC3,0xC9,0xD9,0xD1,0xCB,0xDB, 
  0xD3,0xCA,0xDA,0xC5,0xC7,0xCD,0xDD,0xD5,
  0xCF,0xDF,0xD7,0xCE,0xDE,0xE5,0xE7,0xFA,
  0xED,0xFD,0xF5,0xEA,0xF3,0xFB,0xD8,0xF8,
  0x00,0x2E,0x10,0x3C,0x2B,0x29,0x2F,0x04,
  0x37,0x3E,0x21,0x2C,0x20,0x24,0x28,0x0C,
  0x34,0x02,0x06,0x12,0x32,0x22,0x16,0x36,
  0x26,0x14,0x31,0x30,0x23,0x3F,0x1C,0x39,
  0x48,0x41,0x43,0x49,0x59,0x51,0x4B,0x5B,
  0x53,0x4A,0x5A,0x45,0x47,0x4D,0x5D,0x55,
  0x4F,0x5F,0x57,0x4E,0x5E,0x65,0x67,0x7A,
  0x6D,0x7D,0x75,0x6A,0x73,0x7B,0x58,0x38,
  0x08,0x01,0x03,0x09,0x19,0x11,0x0B,0x1B,
  0x13,0x0A,0x1A,0x05,0x07,0x0D,0x1D,0x15,
  0x0F,0x1F,0x17,0x0E,0x1E,0x25,0x27,0x3A,
  0x2D,0x3D,0x35,0x2A,0x33,0x3B,0x18,0x78,
  0x88,0x81,0x83,0x89,0x99,0x91,0x8B,0x9B,
  0x93,0x8A,0x9A,0x85,0x87,0x8D,0x9D,0x95,
  0x8F,0x9F,0x97,0x8E,0x9E,0xA5,0xA7,0xBA,
  0xAD,0xBD,0xB5,0xAA,0xB3,0xBB,0x98,0xB8,
  0x40,0x6E,0x50,0x7C,0x6B,0x69,0x6F,0x44,
  0x77,0x7E,0x61,0x6C,0x60,0x64,0x68,0x4C,
  0x74,0x42,0x46,0x52,0x72,0x62,0x56,0x76,
  0x66,0x54,0x71,0x70,0x63,0x7F,0x5C,0x79,
  0xC0,0xEE,0xD0,0xFC,0xEB,0xE9,0xEF,0xC4,
  0xF7,0xFE,0xE1,0xEC,0xE0,0xE4,0xE8,0xCC,
  0xF4,0xC2,0xC6,0xD2,0xF2,0xE2,0xD6,0xF6,
  0xE6,0xD4,0xF1,0xF0,0xE3,0xFF,0xDC,0xF9,
  0x80,0xAE,0x90,0xBC,0xAB,0xA9,0xAF,0x84,
  0xB7,0xBE,0xA1,0xAC,0xA0,0xA4,0xA8,0x8C,
  0xB4,0x82,0x86,0x92,0xB2,0xA2,0x96,0xB6,
  0xA6,0x94,0xB1,0xB0,0xA3,0xBF,0x9C,0xB9
};

void
InitBraille()
{
  bd.bd_start_braille=0;
  bd.bd_port = 0;
  bd.bd_braille_table = SaveStr("internal us-braille.tbl"); 
  bd.bd_type = 0;
  bd.bd_baud = 9600;
  bd.bd_bell = 1;
  bd.bd_eightdot = 1;
  bd.bd_info = 0;
  bd.bd_link = 1;
  bd.bd_ncells = 0;
  bd.bd_width = 0;
  bd.bd_ncrc = 1;
  bd.bd_scroll = 1;
  bd.bd_skip = 0;
  bd.bd_using_braille = 0;
  bd.bd_obuflen = 0;
  bd.bd_fd = -1;
  bcopy((char *)btable_local, bd.bd_btable, 256);
}

static int 
initialize_braille_display_type(s)
char *s;
{
  int i;
  
  for (i = 0; i < sizeof(bd_typelist)/sizeof(*bd_typelist); i++)
    if (!strcmp(s, bd_typelist[i].name))
      break;
  if (i == sizeof(bd_typelist)/sizeof(*bd_typelist))
    {
      Msg(0, "No entry for bd_type: %s ", s);
      return -1;
    }
  bd.bd_type = bd_typelist[i].name;
  if ((*bd_typelist[i].init)())
    return -1;

  if (!bd.bd_width)
    bd.bd_width = bd.bd_ncells;

  return 0;
}

void
StartBraille()
{
  bd.bd_dpy = displays;

  debug("StartBraille called\n");
  evdeq(&bd.bd_readev);
  evdeq(&bd.bd_writeev);
  evdeq(&bd.bd_selectev);
  bd.bd_using_braille = 0;

  if (!bd.bd_start_braille)
    return;

  if (bd.bd_type == 0 || bd.bd_port == 0)
    return;

  if (bd.bd_fd < 0 && open_braille_device())
    {
      Msg(0, "bd_port turned off");
      free(bd.bd_port);
      bd.bd_port = 0;
      return;
    }

  /* check if braille display is connected and turned on */
  if (bd.bd_response_test())
    {
      Msg(0, "Make sure that braille display is connected and turned on. ");
      Msg(0, "start_braille turned off");
      bd.bd_start_braille = 0;
    }
  else
    {
      bd.bd_using_braille = 1;
      bd.bd_readev.fd = bd.bd_writeev.fd = bd.bd_fd;
      bd.bd_readev.type  = EV_READ;
      bd.bd_writeev.type = EV_WRITE;
      bd.bd_selectev.type = EV_ALWAYS;
      bd.bd_readev.data = bd.bd_writeev.data = bd.bd_selectev.data = (char *)&bd;
      bd.bd_readev.handler  = bd_readev_fn;
      bd.bd_writeev.handler = bd_writeev_fn;
      bd.bd_selectev.handler = bd_selectev_fn;
      evenq(&bd.bd_readev);
      bd.bd_writeev.condpos = &bd.bd_obuflen;
      bd.bd_writeev.condneg = 0;
      evenq(&bd.bd_writeev);
      bd.bd_selectev.pri = -20;
      evenq(&bd.bd_selectev);
    }
}


static int 
load_braille_table(tablename)
char *tablename;
{
  int i, j, c, p;
  FILE *fp;
  char buffer[80], a[10];

  if ((fp = secfopen(tablename, "r")) == 0) 
    {
      Msg(errno, "Braille table not found: %s ", tablename);
      return -1;
    }
  bzero(bd.bd_btable, 256);
  /* format:
   * Dec  Hex    Braille      Description
   *  7   07    (12-45--8)    BEL
   */
  while (fgets(buffer, sizeof(buffer), fp))
    {
      if (buffer[0] == '#') 
	continue;
      sscanf(buffer,"%d %x %8s", &i, &j, a);
      if (i < 0 || i > 255)
	continue;
      for (j=1, p=1, c=0; j<9; j++, p*=2)
	if (a[j] == '0' + j)
	  c += p;
      bd.bd_btable[i] = c;
    }
  fclose(fp);
  return 0;
}


static int 
open_braille_device(s)
char *s;
{
  char str[256];

  sprintf(str, "%d cs8 -istrip ixon ixoff", bd.bd_baud);
  bd.bd_fd = OpenTTY(bd.bd_port, str);
  if (bd.bd_fd == -1)
    {
      Msg(errno, "open comm port failed: %s ", bd.bd_port);
      return -1;
    }
  fcntl(bd.bd_fd, F_SETFL, FNBLOCK);
  return 0;
}


static void
position_braille_cursor()
{
  int  sx = bd.bd_sx;
  int  bx = BD_FORE->w_bd_x;
  int  eol = BD_FORE->w_width;
  int  w = bd.bd_width;

  if (bd.bd_scroll)
    bx = sx - w + bd.bd_ncrc;	/* keep rc centered in window */
  else
    bx = w * (int)(sx / w);	/* increase bc in integral steps */

  if (bx > eol - w)
    bx = eol - w;
  if (bx < 0)
    bx = 0;
  BD_FORE->w_bd_x = bx;
  BD_FORE->w_bd_y = bd.bd_sy;
}


void
RefreshBraille()
{
  int i, y, xs, xe;
  int cursor_pos;

  if (!bd.bd_using_braille)
    return;
  if (!BD_FORE)
    return;
  bcopy(bd.bd_line, bd.bd_oline, bd.bd_ncells);
  bd.bd_refreshing = 1;
  flayer = bd.bd_dpy->d_forecv->c_layer;
  bd.bd_sx = flayer->l_x;
  bd.bd_sy = flayer->l_y;
  display = bd.bd_dpy;
  if ((D_obufp != D_obuf) && bd.bd_link)
    {
      /* jump to real cursor */
      debug("calling position_braille_cursor\n");
      position_braille_cursor();
    }
  bclear(bd.bd_line, bd.bd_ncells);
  
  y = BD_FORE->w_bd_y;
  xs = BD_FORE->w_bd_x;

  if (bd.bd_info & 1)
    {
      sprintf(bd.bd_line, "%02d%02d", (BD_FORE->w_bd_x + 1) % 100, (BD_FORE->w_bd_y + 1) % 100);
      bd.bd_line[4] = ' ';
    }
  if (bd.bd_info & 2)
    {
      sprintf(bd.bd_line + bd.bd_ncells - 4, "%02d%02d",(bd.bd_sx +1) % 100, (bd.bd_sy +1) % 100);
    }

  xe = xs + bd.bd_width - 1;

  if (xs > flayer->l_width - 1)
    xs = flayer->l_width - 1;
  if (xe > flayer->l_width - 1)
    xe = flayer->l_width - 1;

  if (D_status)
    {
      sprintf(bd.bd_line, "**%-*.*s", bd.bd_ncells - 2, bd.bd_ncells - 2, D_status_lastmsg ? D_status_lastmsg : "unknown msg");
      xs = xe = -1;
    }
  else if (xs <= xe)
    {
      LayRedisplayLine(-1, xs, xe, 1);
      LayRedisplayLine(y, xs, xe, 1);
    }

  debug1("Braille: got >%s<\n", bd.bd_line);

  bd.bd_refreshing = 0;

  if (y == bd.bd_sy && xs <= bd.bd_sx && bd.bd_sx <= xe)
    cursor_pos = bd.bd_sx - xs + (bd.bd_info & 1 ? 4 : 0);
  else
    cursor_pos = bd.bd_ncells;
  for (i = 0; i < bd.bd_ncells; i++)
    if (bd.bd_line[i] != bd.bd_oline[i])
      break;
  if (bd.bd_cursorpos != cursor_pos || i < bd.bd_ncells)
    bd.write_line_braille(bd.bd_line, bd.bd_ncells, cursor_pos); 
  bd.bd_cursorpos = cursor_pos;
}


/**********************************************************************
 *
 */

/*
 * So, why is there a Flush() down below? The reason is simple: the
 * cursor warp (if bd_link is on) checks the obuf to see if something
 * happened. If there would be no Flush, screen would warp the
 * bd cursor if a bd movement command tries to ring the bell.
 * (In other words: this is a gross hack!)
 */
static void
bd_signal()
{
  if (!bd.bd_bell) 
    return;
  display = bd.bd_dpy;
  if (D_obufp != D_obuf)
    AddCStr(D_BL);
  else
    {
      AddCStr(D_BL);
      Flush(0);
    }
}

static int
bd_do_search(y, xs, xe)
int y, xs, xe;
{
  int oy = BD_FORE->w_bd_y;

  if (!bd.bd_skip)	/* no skip mode, found it */
    {
      if (xs > xe)
	return 0;
      bd.bd_searchmin = xs;
      bd.bd_searchmax = xe;
      return 1;
    }
  flayer = bd.bd_dpy->d_forecv->c_layer;
  bd.bd_searchmax = -1;
  bd.bd_searchmin = flayer->l_width;
  if (xs <= xe)
    {
      BD_FORE->w_bd_y = y;	/* stupid hack */
      bd.bd_refreshing = bd.bd_searching = 1;
      bd.bd_searchstart = xs;
      bd.bd_searchend   = xe;
      LayRedisplayLine(-1, xs, xe, 1);
      LayRedisplayLine(y, xs, xe, 1);
      bd.bd_refreshing = bd.bd_searching = 0;
      BD_FORE->w_bd_y = oy;
    }
  return bd.bd_searchmax >= 0;
}

static void
bd_normalize(x, y)
int x, y;
{
  if (x > BD_FORE->w_width - bd.bd_width)
    x = BD_FORE->w_width - bd.bd_width;
  if (x < 0)
    x = 0;
  if (y < 0)
    {
      bd_signal();
      y = 0;
    }
  if (y >= BD_FORE->w_height)
    {
      bd_signal();
      y = BD_FORE->w_height - 1;
    }
  if (x != BD_FORE->w_bd_x || y != BD_FORE->w_bd_y)
    bd.bd_moved = 1;
  BD_FORE->w_bd_x = x;
  BD_FORE->w_bd_y = y;
}

static void
bd_bc_left()
{
  int bx = BD_FORE->w_bd_x, by = BD_FORE->w_bd_y;
  int ex;

  ex = bx - 1;
  bx = 0;
  for (; by >= 0; by--)
    {
      if (bd_do_search(by, 0, ex))
	{
	  if (!bd.bd_skip && by != BD_FORE->w_bd_y)
	    bd_signal();
	  bx = bd.bd_searchmax + 1 - bd.bd_width;
	  break;
	}
      ex = BD_FORE->w_width - 1;
    }
  bd_normalize(bx, by);
}

static void
bd_bc_right()
{
  int bx = BD_FORE->w_bd_x, by = BD_FORE->w_bd_y;
  int sx;

  sx = bx + bd.bd_width;
  bx = BD_FORE->w_width - bd.bd_width;
  for (; by < BD_FORE->w_height; by++)
    {
      if (bd_do_search(by, sx, BD_FORE->w_width - 1))
	{
	  if (!bd.bd_skip && by != BD_FORE->w_bd_y)
	    bd_signal();
	  bx = bd.bd_searchmin;
	  break;
	}
      sx = 0;
    }
  bd_normalize(bx, by);
}

static void
bd_bc_up()
{
  int bx = BD_FORE->w_bd_x, by = BD_FORE->w_bd_y;

  for (by--; by >= 0; by--)
    if (bd_do_search(by, bx, bx + bd.bd_width - 1))
      break;
  bd_normalize(bx, by);
}

static void
bd_bc_down()
{
  int bx = BD_FORE->w_bd_x, by = BD_FORE->w_bd_y;

  for (by++; by < BD_FORE->w_height; by++)
    if (bd_do_search(by, bx, bx + bd.bd_width - 1))
      break;
  bd_normalize(bx, by);
}


static void
bd_upper_left()
{
  bd_normalize(0, 0);
}


static void
bd_upper_right()
{
  bd_normalize(BD_FORE->w_width - bd.bd_width, 0);
}


static void
bd_lower_left()
{
  bd_normalize(0, BD_FORE->w_height - 1);
}


static void
bd_lower_right()
{
  bd_normalize(BD_FORE->w_width - bd.bd_width, BD_FORE->w_height -1);
}

/**********************************************************************
 *
 */


static void
bd_check(x, c)
int x, c;
{
  if (c == ' ')
    return;
  if (x < bd.bd_searchstart || x > bd.bd_searchend)
    return;
  if (x > bd.bd_searchmax)
    bd.bd_searchmax = x;
  if (x < bd.bd_searchmin)
    bd.bd_searchmin = x;
}



/*ARGSUSED*/
void
BGotoPos(la, x, y)
struct layer *la;
int x, y;
{
}

/*ARGSUSED*/
void
BCDisplayLine(la, ml, y, xs, xe, isblank)
struct layer *la;
struct mline *ml;
int y, xs, xe;
int isblank;
{
  int x;
  int sx, ex;
  char *l;

  if (y != BD_FORE->w_bd_y)
    return;
  if (bd.bd_searching)
    {
      for (x = xs; x <= xe; x++)
	bd_check(x, ml->image[x]);
      return;
    }
  l = bd.bd_line;
  sx = BD_FORE->w_bd_x;
  ex = sx + bd.bd_width - 1;
  if (bd.bd_info & 1)
    l += 4;
  for (x = xs; x <= xe; x++)
    if (x >= sx && x <= ex)
      l[x - sx] = ml->image[x];
}

/*ARGSUSED*/
void
BPutChar(la, c, x, y)
struct layer *la;
struct mchar *c;
int x, y;
{
  int sx, ex;
  char *l;

  if (y != BD_FORE->w_bd_y)
    return;
  if (bd.bd_searching)
    {
      bd_check(x, c->image);
      return;
    }
  l = bd.bd_line;
  sx = BD_FORE->w_bd_x;
  ex = sx + bd.bd_width - 1;
  if (bd.bd_info & 1)
    l += 4;
  if (x >= sx && x <= ex)
    l[x - sx] = c->image;
}

/*ARGSUSED*/
void
BPutStr(la, s, n, r, x, y)
struct layer *la;
char *s;
int n;
struct mchar *r;
int x, y;
{
  int sx, ex;
  char *l;

  if (y != BD_FORE->w_bd_y)
    return;
  if (bd.bd_searching)
    {
      for (; n > 0; n--, s++, x++)
	bd_check(x, *s);
      return;
    }
  l = bd.bd_line;
  sx = BD_FORE->w_bd_x;
  ex = sx + bd.bd_width - 1;
  if (bd.bd_info & 1)
    l += 4;
  for (; n > 0; n--, s++, x++)
    if (x >= sx && x <= ex)
      l[x - sx] = *s;
}



/**********************************************************************
 *
 */

static char *infonames[] = {"none", "bc", "sc", "bc+sc"};

void
DoBrailleAction(act, msgok)
struct action *act;
int msgok;
{
  int nr, dosig;
  int n, l, o;
  char *s, **args;
  struct stat st;

  nr = act->nr;
  args = act->args;
  dosig = display && !*rc_name;

  switch(nr)
    {
    case RC_BD_BELL:
      if (ParseSwitch(act, &bd.bd_bell) || !msgok)
	{
	  bd_signal();
	  break;
	}
      Msg(0, bd.bd_bell ? "bd_bell is on." : "bd_bell is off.");
      break;

    case RC_BD_EIGHTDOT:
      if (ParseSwitch(act, &bd.bd_eightdot) || !msgok)
	break;
      Msg(0, "switched to %d-dots system.", bd.bd_eightdot ? 8 : 6);
      break;

    case RC_BD_INFO:
      n = bd.bd_info;
      if (*args)
	{
	  if (strlen(*args) == 4)
	    n = args[0][n] - '0';
	  else if (ParseNum(act, &n))
	    break;
	}
      if (n < 0 || n > 3)
	{
          Msg(0, "Out of range; 0 <= bd_info >= 3 ");
	  break;
	}
      /* bd_width at the beginning is unknown */
      if (bd.bd_width == 0)
        break;
      
      o = (bd.bd_info * 2 + 2) & 12;
      l = (n * 2 + 2) & 12;
      if (l >= bd.bd_ncells)
	{
	  Msg(0, "bd_info is too large for braille display.");
	  break;
	}
      if (l >= bd.bd_width + o)
	{
	  Msg(0, "bd_info is too large for bd_width.");
	  break;
	}
      bd.bd_width += o - l;
      bd.bd_info = n;

      if (msgok)
	Msg(0, "bd_info is %s.", infonames[n]);
      position_braille_cursor();
      break;

    case RC_BD_LINK:
      if (*args == 0 && bd.bd_moved)
	bd.bd_link = 0;
      if (ParseSwitch(act, &bd.bd_link))
	break;
      if (bd.bd_link)
	{
	  bd.bd_moved = 0;
	  if (dosig)
	    bd_signal();
	  position_braille_cursor();
	}
      if (msgok)
        Msg(0, bd.bd_link ? "bd_link is on." : "bd_link is off.");
      break;

    case RC_BD_SKIP:
      if (ParseSwitch(act, &bd.bd_skip))
	break;
      if (bd.bd_skip && dosig)
	bd_signal();
      if (msgok)
        Msg(0, bd.bd_skip ? "bd_skip is on." : "bd_skip is off.");
      break; 

    case RC_BD_SCROLL:
      if (ParseSwitch(act, &bd.bd_scroll) || !msgok)
	{
	  position_braille_cursor();
          break;
	}
      Msg(0, bd.bd_scroll ? "bd_scroll is on." : "bd_scroll is off.");
      break;

    case RC_BD_NCRC: 
      n = bd.bd_ncrc;
      if (*args)
	{
	  if (args[0][0] == '+')
	    n = (n + atoi(*args + 1)) % bd.bd_width + 1;
	  else if (args[0][0] == '-')
	    n = (n - atoi(*args + 1)) % bd.bd_width + 1;
	  else if (ParseNum(act, &n))
	    break;
	}
      if (n < 1 ||  n > bd.bd_width)
	{
	  Msg(0, "Out of range; 1 <= bd_ncrc >= %d", bd.bd_width);
	  break;
	}
      bd.bd_ncrc = n;
      if (msgok) 
	Msg(0, "bd_ncrc status is: %d ", bd.bd_ncrc);
      position_braille_cursor();
      break;

    case RC_BD_BRAILLE_TABLE:
      s = 0;
      if (*args)
	{
	  if (ParseSaveStr(act, &s))
	    break;
	  if (load_braille_table(s))
	    {
	      free(s);
	      break;
	    }
	  if (bd.bd_braille_table)
	    free(bd.bd_braille_table);
	  bd.bd_braille_table = s;
	}
      if (msgok)
	Msg(0, "bd_braille_table is: %s ", bd.bd_braille_table);
      break;

    case RC_BD_PORT:
      s = 0;
      if (*args)
	{
          if (ParseSaveStr(act, &s))
	    break;

	  if (stat(s, &st) || !S_ISCHR(st.st_mode) || access(s, R_OK|W_OK))
	    {
	      Msg(0, "Cannot access braille device port %s", s);
	      free(s);
	      break;
	    }
	  if (bd.bd_fd >= 0)
	    close(bd.bd_fd);
	  bd.bd_fd = -1;
	  if (bd.bd_port)
	    free(bd.bd_port);
	  bd.bd_port = s;
	}
      if (msgok)
	Msg(0, "bd_port is: %s ", bd.bd_port ? bd.bd_port : "not set");
      StartBraille();
      break;

    case RC_BD_TYPE:
      s = 0;
      if (*args)
        if (ParseSaveStr(act, &s) || initialize_braille_display_type(s))
	  break;
      if (msgok)
	Msg(0, "bd_type is: %s ", bd.bd_type ? bd.bd_type : "not set");
      StartBraille();
      break;

    case RC_BD_START_BRAILLE:
      if (ParseSwitch(act, &bd.bd_start_braille))
	break;
      if (msgok)
        Msg(0, bd.bd_start_braille ? "bd_start_braille is on." : "bd_start_braille is off.");
      StartBraille();
      break;

    case RC_BD_WIDTH:
      n = bd.bd_width;
      if (*args)
	{
	  if (ParseNum(act, &n))
	    break;
	}
      if (n <= 0)
	{
	  Msg(0, "Invalid value for bd_width: %d ", n);
	  break;
	}
      l = (bd.bd_info * 2 + 2) & 12;
      if (n > bd.bd_ncells - l || n < l)
	{
	  Msg(0, "bd_info is too large for bd_width.");
	  break;
	}
      bd.bd_width = n;
      if (msgok)
	Msg(0, "bd_width is: %d ", bd.bd_width);
      break;

    case RC_BD_BC_LEFT:
      bd_bc_left();
      break;

    case RC_BD_BC_RIGHT:
      bd_bc_right();
      break;

    case RC_BD_BC_UP:
      bd_bc_up();
      break;

    case RC_BD_BC_DOWN:
      bd_bc_down();
      break;

    case RC_BD_UPPER_LEFT:
      bd_upper_left();
      break;

    case RC_BD_UPPER_RIGHT:
      bd_upper_right();
      break;

    case RC_BD_LOWER_LEFT:
      bd_lower_left();
      break;

    case RC_BD_LOWER_RIGHT:
      bd_lower_right();
      break;

    default:
      break;
    }
}

static void
bd_readev_fn(ev, data)
struct event *ev;
char *data;
{
  bd.buttonpress();
}

static void
bd_writeev_fn(ev, data)
struct event *ev;
char *data;
{
  int len;

  if (bd.bd_obuflen == 0)
    return;
  if ((len = write(bd.bd_fd, bd.bd_obuf, bd.bd_obuflen)) < 0)
    len = bd.bd_obuflen;	/* dead braille display */
  if ((bd.bd_obuflen -= len))
    bcopy(bd.bd_obuf + len, bd.bd_obuf, bd.bd_obuflen);
}

static void
bd_selectev_fn(ev, data)
struct event *ev;
char *data;
{
  RefreshBraille();
}

#endif /* HAVE_BRAILLE */
