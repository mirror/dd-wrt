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

#ifndef SCREEN_CANVAS_H
#define SCREEN_CANVAS_H

#define SLICE_UNKN 0
#define SLICE_VERT (1 << 0)
#define SLICE_HORI (1 << 1)

#define SLICE_THIS (1 << 2)	/* used in equal test */
#define SLICE_GLOBAL (1 << 3)

struct canvas
{
  struct canvas   *c_next;	/* next canvas on display */
  struct display  *c_display;	/* back pointer to display */

  struct canvas   *c_slnext;	/* next canvas in display slice */
  struct canvas   *c_slprev;	/* prev canvas in display slice */
  struct canvas   *c_slperp;	/* perpendicular slice */
  struct canvas   *c_slback;	/* perpendicular slice back pointer */
  int              c_slorient;  /* our slice orientation */
  int              c_slweight;	/* size ratio */

  struct viewport *c_vplist;
  struct layer    *c_layer;	/* layer on this canvas */
  struct canvas   *c_lnext;	/* next canvas that displays layer */
  struct layer     c_blank;	/* bottom layer, always blank */
  int              c_xoff;	/* canvas x offset on display */
  int              c_yoff;	/* canvas y offset on display */
  int              c_xs;
  int              c_xe;
  int              c_ys;
  int              c_ye;
  struct event     c_captev;	/* caption changed event */
};

struct win;			/* forward declaration */

extern void  SetCanvasWindow __P((struct canvas *, struct win *));
extern void  SetForeCanvas __P((struct display *, struct canvas *));
extern struct canvas *FindCanvas __P((int, int));
extern int   MakeDefaultCanvas __P((void));
extern int   AddCanvas __P((int));
extern void  RemCanvas __P((void));
extern void  OneCanvas __P((void));
extern void  FreeCanvas __P((struct canvas *));
extern void  ResizeCanvas __P((struct canvas *));
extern void  RecreateCanvasChain __P((void));
extern void  RethinkViewportOffsets __P((struct canvas *));
extern int   CountCanvasPerp __P((struct canvas *));
extern void  EqualizeCanvas __P((struct canvas *, int));
extern void  DupLayoutCv __P((struct canvas *, struct canvas *, int));
extern void  PutWindowCv __P((struct canvas *));

#define CV_CALL(cv, cmd)			\
{						\
  struct display *olddisplay = display;		\
  struct layer *oldflayer = flayer;		\
  struct layer *l = cv->c_layer;		\
  struct canvas *cvlist = l->l_cvlist;		\
  struct canvas *cvlnext = cv->c_lnext;		\
  flayer = l;					\
  l->l_cvlist = cv;				\
  cv->c_lnext = 0;				\
  cmd;						\
  flayer = oldflayer;				\
  l->l_cvlist = cvlist;				\
  cv->c_lnext = cvlnext;			\
  display = olddisplay;				\
}

#endif /* SCREEN_CANVAS_H */

