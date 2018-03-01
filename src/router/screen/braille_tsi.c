/*    bd-tsi.c, TSI specific key bindings and display commands
 *
 *	dotscreen
 *	A braille interface to unix tty terminals
 *      Authors:  Hadi Bargi Rangin  bargi@dots.physics.orst.edu
 *                Bill Barry         barryb@dots.physics.orst.edu
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
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"
#include "screen.h"
#include "extern.h"
#include "braille.h"

#ifdef HAVE_BRAILLE

extern struct display *display;

struct key2rc {
  int key;
  int nr;
  char *arg1;
  char *arg2;
};


static int tsi_ctype = 1;        /* cursor type, 0,1,2 */

static int tsi_line_type; /* indicates number of cells on powerbraille
                          display 01=20 cells 02=40 cells 03=80 cells */

static int  display_status_tsi __P((void));
static int  write_line_tsi __P((char *, int, int));
static void buttonpress_tsi __P((struct key2rc *));
static void buttonpress_navigator_40 __P((void));
static void buttonpress_powerbraille_40 __P((void));
static void buttonpress_powerbraille_80 __P((void));

int
bd_init_powerbraille_40()
{
  bd.write_line_braille = write_line_tsi;
  bd.buttonpress = buttonpress_powerbraille_40;
  bd.bd_response_test = display_status_tsi;
  bd.bd_ncells = 40;
  tsi_line_type = 2;
  return 0;
}

int
bd_init_powerbraille_80()
{
  bd.write_line_braille = write_line_tsi;
  bd.buttonpress = buttonpress_powerbraille_80;
  bd.bd_response_test = display_status_tsi;
  bd.bd_ncells = 80;
  tsi_line_type = 3;
  return 0;
}

int
bd_init_navigator_40()
{
  bd.write_line_braille = write_line_tsi;
  bd.buttonpress = buttonpress_navigator_40;
  bd.bd_response_test = display_status_tsi;
  bd.bd_ncells = 40;
  tsi_line_type = 2;
  return 0;
}

static int 
display_status_tsi()
{
  char obuf[3],ibuf[20];
  int r;

  obuf[0] = 0xff;
  obuf[1] = 0xff;
  obuf[2] = 0x0a;
  r = read(bd.bd_fd, ibuf, 20);		/* flush the input port */
  r = write(bd.bd_fd, obuf, 3);
  if (r != 3)
    return -1;

  /* we have written to the display asking for a response
     we wait 1 second for the response, read it and if no 
     response we wait 2 seconds, if still no response,
     return -1 to indicate no braille  display available */
  sleep(1);
  r = read(bd.bd_fd, ibuf, 2);
  if (r == -1)
    {
      sleep(2);
      r = read(bd.bd_fd, ibuf, 2);
    }
  debug2("first chars from braille display %d %d\n",ibuf[0],ibuf[1]);
  if (r != 2 || ibuf[0] != 0 || ibuf[1] != 5)
    return -1;
  
  r= read(bd.bd_fd,ibuf,2);
  if (r != 2)
    return -1;
  debug2("braille display size:%d dots:%d\n", ibuf[0], ibuf[1]);
  bd.bd_ncells = (unsigned char)ibuf[0];
  if (bd.bd_ncells <= 1)
    return -1;
  r = read(bd.bd_fd,ibuf,1);
  if (r != 1)
    return -1;
  if (ibuf[0] == 'V')
    r = read(bd.bd_fd, ibuf, 3);
  else
    r = read(bd.bd_fd, ibuf + 1, 2) + 1;
  if (r != 3)
    return -1;
  ibuf[3] = 0;
  debug1("braille display version %s\n", ibuf); 
  bd.bd_version = atof(ibuf);
  return 0;
}
		

static int 
write_line_tsi (bstr,line_length,  cursor_pos) 
char *bstr;
int line_length, cursor_pos; 
{
  int obp, i;
  bd.bd_obuf[0] = 0xff;
  bd.bd_obuf[1] = 0xff;
  bd.bd_obuf[2] = tsi_line_type;
  bd.bd_obuf[3] = 0x07;
  bd.bd_obuf[4] = cursor_pos;
  bd.bd_obuf[5] = tsi_ctype;
  obp=6;

  for (i=0; i < line_length; i++) 
    {
      bd.bd_obuf[2*i+obp] = 0; 
      bd.bd_obuf[2*i+1+obp] = bd.bd_btable[(int)(unsigned char)bstr[i]];
    }      
  for (i=line_length; i < bd.bd_ncells; i++) 
    {
      bd.bd_obuf[2*i+obp] = 0; 
      bd.bd_obuf[2*i+1+obp] = bd.bd_btable[(int)' '];
    }      

  bd.bd_obuflen = 2*bd.bd_ncells + obp ;  
  return 0;
}

static struct key2rc keys_navigator_40[] = {
  {0x4000000,	RC_STUFF, "-k", "kl"},		/* 1 */
  {0x10000000, 	RC_STUFF, "-k", "kr"},		/* 3 */
  {0x8000000,	RC_STUFF, "-k", "ku"},		/* 2 */
  {0x20000000,	RC_STUFF, "-k", "kd"},		/* 4 */
  {0x2000,	RC_BD_BC_LEFT, 0, 0},		/* 6 */
  {0x8000,	RC_BD_BC_RIGHT, 0, 0},		/* 8 */
  {0x4000,	RC_BD_BC_UP, 0, 0},		/* 7 */
  {0x10000,	RC_BD_BC_DOWN, 0, 0},		/* 9 */
  {0x6000,	RC_BD_UPPER_LEFT, 0, 0},	/* 6, 7 */
  {0xc000,	RC_BD_UPPER_RIGHT, 0, 0},	/* 7, 8 */
  {0x12000,	RC_BD_LOWER_LEFT, 0, 0},	/* 6, 9 */
  {0x18000,	RC_BD_LOWER_RIGHT, 0, 0},	/* 8, 9 */
  {0xa000,	RC_BD_INFO, "1032", 0},		/* bc 6, 8 */
  {0x14000000,	RC_BD_INFO, "2301", 0},		/* sc 1, 3 */
  {0x4008000,	RC_BD_INFO, "3330", 0},		/* bc+sc 1, 8 */
  {0x8010000,	RC_BD_BELL, 0, 0},		/* 2, 9 */
  {0x8004000,	RC_BD_EIGHTDOT, 0, 0},		/* 2, 7 */
  {0x40000000,	RC_STUFF, "\015", 0},		/* 5 */
  {0x20000,	RC_BD_LINK, 0, 0},		/* 10 */
  {0x10002000,	RC_BD_SCROLL, 0, 0},		/* 3, 6 */
  {0x20010000,	RC_BD_NCRC, "+", 0},		/* 4, 9 */
  {0x14000,	RC_BD_SKIP, 0, 0},		/* 7, 9*/
  {-1,		RC_ILLEGAL, 0, 0}
};

static struct key2rc keys_powerbraille_40[] = {
  {0x4000000,	RC_STUFF, "-k", "kl"},		/* 1 */
  {0x10000000, 	RC_STUFF, "-k", "kr"},		/* 3 */
  {0x8000000,	RC_STUFF, "-k", "ku"},		/* 2 */
  {0x20000000,	RC_STUFF, "-k", "kd"},		/* 4 */
  {0x2000,	RC_BD_BC_LEFT, 0, 0},		/* 6 */
  {0x8000,	RC_BD_BC_RIGHT, 0, 0},		/* 8 */
  {0x4000,	RC_BD_BC_UP, 0, 0},		/* 7 */
  {0x10000,	RC_BD_BC_DOWN, 0, 0},		/* 9 */
  {0x8002000,	RC_BD_UPPER_LEFT, 0, 0},	/* 2, 6 */
  {0xc000,	RC_BD_UPPER_RIGHT, 0, 0},	/* 7, 8 */
  {0x20002000,	RC_BD_LOWER_LEFT, 0, 0},	/* 3, 6 */
  {0x18000,	RC_BD_LOWER_RIGHT, 0, 0},	/* 8, 9 */
  {0x8008000,	RC_BD_INFO, "1032", 0},		/* bc 2, 8 */
  {0x6000,	RC_BD_INFO, "2301", 0},		/* 6, 7 */
  {0x8004000,	RC_BD_INFO, "3330", 0},		/* bc+sc 2, 7 */
  {0x8010000,	RC_BD_BELL, 0, 0},		/* 2, 9 */
  {0x20008000,	RC_BD_EIGHTDOT, 0, 0},		/* 4, 6 */
  {0x40000000,	RC_STUFF, "\015", 0},		/* 5 */
  {0x20000,	RC_BD_LINK, 0, 0},		/* 10 */
  {0xa000,	RC_BD_SCROLL, 0, 0},		/* 6, 8 */
  {0x20010000,	RC_BD_NCRC, "+", 0},		/* 4, 9 */
  {0x20004000,	RC_BD_SKIP, 0, 0},		/* 4, 7 */
  {-1,		RC_ILLEGAL, 0, 0}
};


static struct key2rc keys_powerbraille_80[] = {
  {0x4000000,	RC_STUFF, "-k", "kl"},		/* 1 */
  {0x10000000, 	RC_STUFF, "-k", "kr"},		/* 3 */
  {0x8000000,	RC_STUFF, "-k", "ku"},		/* 2 */
  {0x20000000,	RC_STUFF, "-k", "kd"},		/* 4 */
  {0x40000,	RC_BD_BC_LEFT, 0, 0},		/* 6 */
  {0x100000,	RC_BD_BC_RIGHT, 0, 0},		/* 8 */
  {0x4000,	RC_BD_BC_UP, 0, 0},		/* 7 */
  {0x10000,	RC_BD_BC_DOWN, 0, 0},		/* 9 */
  {0x44000,	RC_BD_UPPER_LEFT, 0, 0},	/* 6, 7 */
  {0x104000,	RC_BD_UPPER_RIGHT, 0, 0},	/* 7, 8 */
  {0x50000,	RC_BD_LOWER_LEFT, 0, 0},	/* 6, 9 */
  {0x110000,	RC_BD_LOWER_RIGHT, 0, 0},	/* 8, 9 */
  {0x8100000,	RC_BD_INFO, "1032", 0},		/* 2, 8 */
  {0x8040000,	RC_BD_INFO, "2301", 0},		/* 2, 6 */
  {0x140000,	RC_BD_INFO, "3330", 0},		/* 6, 8 */
  {0x8010000,	RC_BD_BELL, 0, 0},		/* 2, 9 */
  {0x8004000,	RC_BD_EIGHTDOT, 0, 0},		/* 2, 7 */
  {0x40000000,	RC_STUFF, "\015", 0},		/* 5 */
  {0x20000,	RC_BD_LINK, 0, 0},		/* 10 */
  {0x20004000,	RC_BD_SCROLL, 0, 0},		/* 4, 7 */
  {0x20010000,	RC_BD_NCRC, "+", 0},		/* 4, 9 */
  {0x40010000,	RC_BD_SKIP, 0, 0},		/* 5, 9 */
  {-1,		RC_ILLEGAL, 0, 0}
};

static void
buttonpress_tsi(tab)
struct key2rc *tab;
{
  int i, nb;
  int bkeys;
  unsigned char buf[10];
  nb = read(bd.bd_fd, buf, 10);
  debug1("buttonpress_tsi: read %d bytes\n", nb);
  for (i=0, bkeys=0; i < nb; i++)
    {   
      switch (buf[i] & 0xE0)
	{
	case 0x00: bkeys += ((int)(buf[i] & 0x1f)       );  break;
	case 0x20: bkeys += ((int)(buf[i] & 0x1f) <<  5 );  break;
	case 0x40: bkeys += ((int)(buf[i] & 0x1f) <<  9 );  break;
	case 0x60: bkeys += ((int)(buf[i] & 0x1f) <<  13 ); break;
	case 0xA0: bkeys += ((int)(buf[i] & 0x1f) <<  18 ); break;
	case 0xC0: bkeys += ((int)(buf[i] & 0x1f) <<  22 ); break;
	case 0xE0: bkeys += ((int)(buf[i] & 0x1f) <<  26 ); break;
	default: break;
	}
    }
  debug1("bkeys %x\n", bkeys);
  for (i = 0; tab[i].key != -1; i++)
    if (bkeys == tab[i].key)
      break;
  debug1("bkey index %d\n", i);
  if (tab[i].key != -1 && tab[i].nr != RC_ILLEGAL)
    {
      char *args[3];
      int argl[2];
      
      struct action act;
      args[0] = tab[i].arg1;
      args[1] = tab[i].arg2;
      args[2] = 0;
      argl[0] = args[0] ? strlen(args[0]) : 0;
      argl[1] = args[1] ? strlen(args[1]) : 0;
      act.nr = tab[i].nr;
      act.args = args;
      act.argl = argl;
      display = bd.bd_dpy;
      DoAction(&act, -2);
    }
}


static void
buttonpress_navigator_40()
{
  buttonpress_tsi(keys_navigator_40);
}

static void
buttonpress_powerbraille_40()
{
  buttonpress_tsi(keys_powerbraille_40);
}

static void
buttonpress_powerbraille_80()
{
  buttonpress_tsi(keys_powerbraille_80);
}

#endif /* HAVE_BRAILLE */


