/* Copyright (c) 2010
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 2008, 2009
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
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

/* Deals with the list of displays */

#include "config.h"

#include "screen.h"
#include "extern.h"
#include "list_generic.h"

#ifdef MULTI

extern struct layer *flayer;
extern struct display *display, *displays;
extern struct mchar mchar_blank, mchar_so;

static char ListID[] = "display";

/*
 * layout of the displays page is as follows:

xterm 80x42      jnweiger@/dev/ttyp4    0(m11)    &rWx
facit 80x24 nb   mlschroe@/dev/ttyhf   11(tcsh)    rwx
xterm 80x42      jnhollma@/dev/ttyp5    0(m11)    &R.x

  |     |    |      |         |         |   |     | ¦___ window permissions
  |     |    |      |         |         |   |     |      (R. is locked r-only,
  |     |    |      |         |         |   |     |       W has wlock)
  |     |    |      |         |         |   |     |___ Window is shared
  |     |    |      |         |         |   |___ Name/Title of window
  |     |    |      |         |         |___ Number of window
  |     |    |      |         |___ Name of the display (the attached device)
  |     |    |      |___ Username who is logged in at the display
  |     |    |___ Display is in nonblocking mode. Shows 'NB' if obuf is full.
  |     |___ Displays geometry as width x height.
  |___ the terminal type known by screen for this display.

 */

static int
gl_Display_header(struct ListData *ldata)
{
  leftline("term-type   size         user interface           window       Perms", 0, 0);
  leftline("---------- ------- ---------- ----------------- ----------     -----", 1, 0);
  return 2;
}

static int
gl_Display_footer(struct ListData *ldata)
{
  centerline("[Press Space to refresh; Return to end.]", flayer->l_height - 1);
  return 1;
}

static int
gl_Display_row(struct ListData *ldata, struct ListRow *lrow)
{
  struct display *d = lrow->data;
  char tbuf[80];
  static char *blockstates[5] = {"nb", "NB", "Z<", "Z>", "BL"};
  struct win *w = d->d_fore;
  struct mchar m_current = mchar_blank;
  m_current.attr = A_BD;

  sprintf(tbuf, " %-10.10s%4dx%-4d%10.10s@%-16.16s%s",
      d->d_termname, d->d_width, d->d_height, d->d_user->u_name,
      d->d_usertty,
      (d->d_blocked || d->d_nonblock >= 0) && d->d_blocked <= 4 ? blockstates[d->d_blocked] : "  ");

  if (w)
    {
      int l = 10 - strlen(w->w_title);
      if (l < 0)
	l = 0;
      sprintf(tbuf + strlen(tbuf), "%3d(%.10s)%*s%c%c%c%c",
	  w->w_number, w->w_title, l, "",
	  /* w->w_dlist->next */ 0 ? '&' : ' ',
	  /*
	   * The rwx triple:
	   * -,r,R	no read, read, read only due to foreign wlock
	   * -,.,w,W	no write, write suppressed by foreign wlock,
	   *            write, own wlock
	   * -,x	no execute, execute
	   */
#ifdef MULTIUSER
	  (AclCheckPermWin(d->d_user, ACL_READ, w) ? '-' :
	   ((w->w_wlock == WLOCK_OFF || d->d_user == w->w_wlockuser) ?
	    'r' : 'R')),
	  (AclCheckPermWin(d->d_user, ACL_READ, w) ? '-' :
	   ((w->w_wlock == WLOCK_OFF) ? 'w' :
	    ((d->d_user == w->w_wlockuser) ? 'W' : 'v'))),
	  (AclCheckPermWin(d->d_user, ACL_READ, w) ? '-' : 'x')
#else
	  'r', 'w', 'x'
#endif
	  );
    }
  leftline(tbuf, lrow->y, lrow == ldata->selected ? &mchar_so : d == display ? &m_current : 0);

  return 1;
}

static void
gl_Display_rebuild(struct ListData *ldata)
{
  /* recreate the rows */
  struct display *d;
  struct ListRow *row = NULL;
  for (d = displays; d; d = d->d_next)
    {
      row = glist_add_row(ldata, d, row);
      if (d == display)
	ldata->selected = row;
    }

  glist_display_all(ldata);
}

static int
gl_Display_input(struct ListData *ldata, char **inp, int *len)
{
  struct display *cd = display;
  unsigned char ch;

  if (!ldata->selected)
    return 0;

  ch = (unsigned char) **inp;
  ++*inp;
  --*len;

  switch (ch)
    {
    case ' ':	/* Space to refresh */
      glist_remove_rows(ldata);
      gl_Display_rebuild(ldata);
      break;

    case '\r':
    case '\n':
      glist_abort();
      *len = 0;
      break;

#ifdef REMOTE_DETACH
    case 'd': /* Detach */
    case 'D': /* Power detach */
      display = ldata->selected->data;
      if (display == cd)	/* We do not allow detaching the current display */
	break;
      Detach(
#ifdef POW_DETACH
	  ch == 'D' ? D_REMOTE_POWER : D_REMOTE
#else
	  D_REMOTE
#endif
	  );
      display = cd;
      glist_remove_rows(ldata);
      gl_Display_rebuild(ldata);
      break;
#endif

    default:
      /* We didn't actually process the input. */
      --*inp;
      ++*len;
      return 0;
    }
  return 1;
}

static int
gl_Display_freerow(struct ListData *ldata, struct ListRow *row)
{
  /* There was no allocation when row->data was set. So nothing to do here. */
  return 0;
}

static int
gl_Display_free(struct ListData *ldata)
{
  /* There was no allocation in ldata->data. So nothing to do here. */
  return 0;
}

static struct GenericList gl_Display =
{
  gl_Display_header,
  gl_Display_footer,
  gl_Display_row,
  gl_Display_input,
  gl_Display_freerow,
  gl_Display_free,
  NULL	/* We do not allow searching in the display list, at the moment */
};

void
display_displays()
{
  struct ListData *ldata;
  if (flayer->l_width < 10 || flayer->l_height < 5)
    {
      LMsg(0, "Window size too small for displays page");
      return;
    }

  ldata = glist_display(&gl_Display, ListID);
  if (!ldata)
    return;

  gl_Display_rebuild(ldata);
}

#endif /* MULTI */

