//==========================================================================
//
//      ledit.c
//
//      Utterly simple line editor
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Udderly simple line editor
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <monitor.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_BSP
#include <bsp/bsp.h>
#endif
#include "ledit.h"

#ifndef NULL
#define NULL 0
#endif

static char *cutBuf = NULL;

#ifdef NO_MALLOC
static char linebufArray [MAX_HIST_ENTS + 2][MAXLINELEN + 1];
#endif

static struct termcap 
{
  char *relleft, *relright;
  char *oneleft;
  char *insertch;
  char *deletech;
  char *deleteonech;
  char *clreol;
  char *gobol;
  int width;
} terminal;

static struct linepos 
{
  char *prompt;
  char *buffer;
  char *ebuf;
  int cursor;
} linebuf;

static struct history 
{
  char *cmd;
  struct history *next, *prev;
} *list = NULL, *topl = NULL, *curl = NULL;

static int histlen = 0;
static int histLimit = MAX_HIST_ENTS;

static struct history histEnts[MAX_HIST_ENTS + 1], *histPtr = histEnts;
static struct history currLine = { NULL, NULL, NULL };

void
beep (void)
{
  xputchar ('\007');
}

void
printHistoryList () 
{
  struct history *hist = list;
  int hist_num = 1;

  if (hist != NULL)
  {
    while (hist->prev != NULL)
    {
      hist = hist->prev;
    }

    while (hist != NULL)
    {
      if (hist->cmd != NULL)
        xprintf(" %d %s\n", hist_num++, hist->cmd);
      hist = hist->next;
    }
  }
}

static void
outputParamStr (char *str, int val)
{
  char *i = strchr (str, '%');
  char *ptr;
  int dist = val;

  if (i == NULL)
    {
      while (dist-- > 0)
	xprintf (str);
    }
  else
    {
      for (ptr = str; *ptr && ptr < i; ptr++)
	xputchar (*ptr);
      if (dist > 99)
	{
	  xputchar ('0' + dist / 100);
	  dist = dist % 100;
	}
      if (dist > 9)
	{
	  xputchar ('0' + dist / 10);
	  dist = dist % 10;
	}
      xputchar ('0' + dist);
      if (*ptr)
	xprintf (ptr + 1);
    }
}

static void
absMoveCursor (int pos)
{
  int dist, oldpos = linebuf.cursor;
  int absdist;
  char *bigmove;

  if (pos > (linebuf.ebuf - linebuf.buffer))
    {
      beep ();
      pos = linebuf.ebuf - linebuf.buffer;
    }
  else if (pos < 0)
    pos = 0;
  dist = pos - linebuf.cursor;
  absdist = (dist < 0 ? -dist : dist);
  linebuf.cursor = pos;
  if (dist == 0)
    return;
  if (dist < 0)
    bigmove = terminal.relleft;
  else
    bigmove = terminal.relright;

  if ((absdist < 4) || (bigmove == NULL))
    {
      int x;
      int promptLen = strlen (linebuf.prompt);

      if (pos < (absdist - promptLen))
	{
	  xprintf (terminal.gobol);
	  xprintf (linebuf.prompt);
	  for (x = 0; x < pos; x++)
	    xputchar (linebuf.buffer[x]);
	  return;
	}

      if (dist < 0)
	{
	  for (x = 0; x < -dist ;x++)
	    xprintf (terminal.oneleft);
	}
      else
	{
	  for (x = 0; x < dist; x++)
	    xputchar (linebuf.buffer [oldpos + x]);
	}
    }
  else
    {
      outputParamStr (bigmove, absdist);
    }
}

static void
clrScrToEol (void)
{
  int len = linebuf.ebuf - linebuf.buffer;

  if (len < linebuf.cursor)
    return;

  if(terminal.clreol)
    {
      xprintf (terminal.clreol);
    }
  else if (terminal.deletech)
    {
      outputParamStr (terminal.deletech, len - linebuf.cursor);
    }
  else
    {
      int oldcur = linebuf.cursor;
      while (linebuf.cursor < len)
	{
	  xputchar (' ');
	  linebuf.cursor++;
	}
      
      absMoveCursor (oldcur);
    }
}

static void
redrawCmd (void)
{
  xprintf (terminal.gobol);
  xprintf (linebuf.prompt);
  linebuf.buffer[linebuf.cursor] = 0;
  xprintf (linebuf.buffer);
  clrScrToEol ();
}

static void
instCmd (char *ncmd)
{
  linebuf.cursor = strlen (ncmd);
  strcpy (linebuf.buffer, ncmd);
  redrawCmd ();
  linebuf.ebuf = linebuf.buffer + linebuf.cursor;
}

static void
prevCmd (void) 
{
  if (curl == &currLine)
    {
      if (list != NULL)
	{
	  *linebuf.ebuf = 0;
#ifdef NO_MALLOC
	  currLine.cmd = linebufArray[MAX_HIST_ENTS];
	  strcpy (currLine.cmd, linebuf.buffer);
#else
	  if (currLine.cmd != NULL)
	    free (currLine.cmd);
	  currLine.cmd = strdup (linebuf.buffer);
#endif
	  curl = list;
	}
    }
  else
    {
      if (curl->prev != NULL)
	curl = curl->prev;
    }
  if (curl != NULL && curl->cmd != NULL)
    instCmd (curl->cmd);
  else
    beep ();
}

static void
nextCmd (void)
{
  if (curl->next == NULL)
    {
      beep ();
    }
  else
    {
      curl = curl->next;
      instCmd (curl->cmd);
    }

}

static int initted = 0;

void
initVt100 (void) 
{
  terminal.gobol = "\r";
  terminal.oneleft = "\010";
  terminal.relleft = "\033[%D";
  terminal.relright = "\033[%C";
  terminal.insertch = "\033[%@";
  terminal.deletech = "\033[%P";
  terminal.deleteonech = "\033[P";
  terminal.clreol = "\033[K";
  terminal.width = 80;
  initted = 1;
}

void
initDumb (void)
{
  terminal.gobol = "\r";
  terminal.oneleft = "\010";
  terminal.relleft = NULL;
  terminal.relright = NULL;
  terminal.insertch = NULL;
  terminal.deletech = NULL;
  terminal.deleteonech = NULL;
  terminal.clreol = NULL;
  terminal.width = 80;
  initted = 1;
}

static void
insertChar (char *chars, int num)
{
  int len = linebuf.ebuf - linebuf.buffer + strlen (linebuf.prompt);
  int n = 0;

  if ((len + num) >= terminal.width)
    {
      beep ();
      return;
    }
  if ((linebuf.ebuf - linebuf.buffer) > linebuf.cursor)
    {
      char *ptr, *eptr = linebuf.buffer + linebuf.cursor;

      for (ptr = linebuf.ebuf; ptr >= eptr; ptr--)
	*(ptr+num) = *ptr;

      if (terminal.insertch != NULL)
	outputParamStr (terminal.insertch, num);
    }
  for (n = 0; n < num; n++)
    {
      xputchar (*chars);
      linebuf.buffer[linebuf.cursor++] = *(chars++);
    }

  linebuf.ebuf += num;

  if (terminal.insertch == NULL)
    {
      char *ptr = linebuf.buffer + linebuf.cursor;
      int oldcur = linebuf.cursor;
      for (; ptr < linebuf.ebuf; ptr++)
	xputchar (*ptr);
      linebuf.cursor = linebuf.ebuf - linebuf.buffer;
      absMoveCursor (oldcur);
    }
}

static void
deleteEol (int putInCutBuffer)
{
  int len = linebuf.ebuf - linebuf.buffer;
  if (linebuf.cursor < len)
    {
      clrScrToEol ();

      if (putInCutBuffer)
	{
	  *linebuf.ebuf = 0;
#ifdef NO_MALLOC
	  cutBuf = linebufArray[MAX_HIST_ENTS + 1];
	  strcpy (cutBuf, linebuf.buffer + linebuf.cursor);
#else
	  if (cutBuf != NULL)
	    free (cutBuf);
	  cutBuf = strdup (linebuf.buffer + linebuf.cursor);
#endif
	}
      linebuf.ebuf = linebuf.buffer + linebuf.cursor;
    }
}

static void
deleteCurrChar (void)
{
  int len = linebuf.ebuf - linebuf.buffer;
  char *ptr;
  if (len == linebuf.cursor || len == 0)
    return;
  for (ptr = linebuf.buffer + linebuf.cursor; ptr < (linebuf.ebuf - 1); ptr++)
    {
      *ptr = *(ptr + 1);
      if (terminal.deleteonech == NULL)
	xputchar (*ptr);
    }
  linebuf.ebuf--;
  if (terminal.deleteonech && (len - 1) != linebuf.cursor)
    xprintf (terminal.deleteonech);
  else
    {
      int oldcur = linebuf.cursor;
      xputchar (' ');
      linebuf.cursor = linebuf.ebuf - linebuf.buffer + 1;
      absMoveCursor (oldcur);
    }
}

static void
deleteChar (void)
{
  if (linebuf.cursor == 0)
    {
      beep ();
      return;
    }
  absMoveCursor (linebuf.cursor - 1);
  deleteCurrChar ();
}

int
lineedit (char *prompt, char *buffer, int maxLen)
{
  int c;

  curl = &currLine;

  if (!initted)
    {
      initted = 1;
      /*initVt100 ();*/
      initDumb();
    }
  linebuf.prompt = prompt;
  linebuf.buffer = buffer;
  linebuf.ebuf = buffer;
  buffer[0] = 0;
  linebuf.cursor = 0;
  redrawCmd ();
  while ((c=input_char ()) > 0) 
    {
      switch (c)
	{
	case PREVCMD:
	  prevCmd ();
	  break;
	case NEXTCMD:
	  nextCmd ();
	  break;
	case LF:
	case CR:
	  *linebuf.ebuf = 0;
#ifdef NO_MALLOC
	  cutBuf = NULL;
	  currLine.cmd = NULL;
#else
	  if (cutBuf != NULL)
	    {
	      free (cutBuf);
	      cutBuf = NULL;
	    }
	  if (currLine.cmd != NULL)
	    {
	      free (currLine.cmd);
	      currLine.cmd = NULL;
	    }
#endif
	  return linebuf.ebuf - linebuf.buffer;
	  break;
	case BOLCMD:
	  absMoveCursor (0);
	  break;
	case EOLCMD:
	  absMoveCursor (linebuf.ebuf-linebuf.buffer);
	  break;
	case FORWCMD:
	  absMoveCursor (linebuf.cursor + 1);
	  break;
	case BACKCMD:
	  absMoveCursor (linebuf.cursor - 1);
	  break;
	case DELBACK:
	case '\177':
	  deleteChar ();
	  break;
	case ERASELINE:
	  absMoveCursor (0);
	  deleteEol (0);
	  break;
	case DELEOL:
	  deleteEol (1);
	  break;
	case DELCURRCH:
	  deleteCurrChar ();
	  break;
	case YANKCH:
	  if (cutBuf != NULL)
	    insertChar (cutBuf,strlen (cutBuf));
	  break;
	default:
	  if (c >= 32 && c < 127)
	    {
	      char ch = c;
	      insertChar (&ch, 1);
	    }
	  break;
	}
    }
  return -1;
}

void
addHistoryCmd (char *cmd)
{
  struct history *newent = NULL;

  if (histlen >= histLimit)
    {
      newent = topl;
      topl = topl->next;
      topl->prev = NULL;
#ifdef NO_MALLOC
      newent->prev = NULL;
      newent->next = NULL;
#else
      free (newent->cmd);
      newent->cmd = NULL;
#endif
      histlen = histLimit - 1;
    }

  histlen++;

  if (newent == NULL)
    {
      newent = histPtr++;
#ifdef NO_MALLOC
      newent->cmd = linebufArray[histlen - 1];
#endif
    }

  if (list == NULL)
    {
      list = newent;
      list->prev = NULL;
      topl = list;
    }
  else
    {
      list->next = newent;
      list->next->prev = list;
      list = list->next;
    }
  currLine.prev = list;
  list->next = &currLine;
#ifdef NO_MALLOC
  strcpy (list->cmd, cmd);
#else
  list->cmd = strdup (cmd);
#endif
  curl = &currLine;
}

void
set_term_name (char *name)
{
  if (! strcmp (name, "vt100"))
    {
      initVt100 ();
    }
  else if (! strcmp (name, "dumb"))
    {
      initDumb ();
    }
  else
    {
      xprintf ("Unknown terminal name %s\n", name);
    }  
}
