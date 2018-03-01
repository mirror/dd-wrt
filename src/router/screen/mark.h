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
 * $Id$ GNU
 */

struct markdata
{
  struct win *md_window;/* pointer to window we are working on */
  struct acluser *md_user;	/* The user who brought us up */
  int	cx, cy;		/* cursor Position in WIN coords*/
  int	x1, y1;		/* first mark in WIN coords */
  int	second;		/* first mark dropped flag */
  int	left_mar, right_mar, nonl;
  int	rep_cnt;	/* number of repeats */
  int	append_mode;	/* shall we overwrite or append to copybuffer */
  int	write_buffer;	/* shall we do a KEY_WRITE_EXCHANGE right away? */
  int	hist_offset;	/* how many lines are on top of the screen */
  char	isstr[100];	/* string we are searching for */
  int	isstrl;
  char	isistr[200];	/* string of chars user has typed */
  int	isistrl;
  int	isdir;		/* current search direction */
  int	isstartpos;	/* position where isearch was started */
  int	isstartdir;	/* direction when isearch was started */
  struct {              /* bookkeeping for fFtT;, commands */
    int flag, target, direction;
  } f_cmd;
    
};


#define W2D(y) ((y) - markdata->hist_offset)
#define D2W(y) ((y) + markdata->hist_offset)

