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
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 * $Id$ GNU
 */

#ifndef SCREEN_CANVAS_H
#define SCREEN_CANVAS_H

#include "layer.h"
#include "sched.h"

#define SLICE_UNKN 0
#define SLICE_VERT (1 << 0)
#define SLICE_HORI (1 << 1)

#define SLICE_THIS (1 << 2)	/* used in equal test */
#define SLICE_GLOBAL (1 << 3)

typedef struct Display Display;
typedef struct Viewport Viewport;
typedef struct Window Window;

typedef struct Canvas Canvas;
struct Canvas {
	Canvas   *c_next;		/* next canvas on display */
	Display  *c_display;		/* back pointer to display */

	Canvas   *c_slnext;		/* next canvas in display slice */
	Canvas   *c_slprev;		/* prev canvas in display slice */
	Canvas   *c_slperp;		/* perpendicular slice */
	Canvas   *c_slback;		/* perpendicular slice back pointer */
	int              c_slorient;	/* our slice orientation */
	int              c_slweight;	/* size ratio */

	Viewport	*c_vplist;
	Layer    *c_layer;		/* layer on this canvas */
	Canvas   *c_lnext;		/* next canvas that displays layer */
	Layer     c_blank;		/* bottom layer, always blank */
	int              c_xoff;	/* canvas x offset on display */
	int              c_yoff;	/* canvas y offset on display */
	int              c_xs;
	int              c_xe;
	int              c_ys;
	int              c_ye;
	Event     c_captev;		/* caption changed event */
};

void  SetCanvasWindow (Canvas *, Window *);
void  SetForeCanvas (Display *, Canvas *);
Canvas *FindCanvas (int, int);
int   MakeDefaultCanvas (void);
int   AddCanvas (int);
void  RemCanvas (void);
void  OneCanvas (void);
void  FreeCanvas (Canvas *);
int   CountCanvas (Canvas *);
void  ResizeCanvas (Canvas *);
void  RecreateCanvasChain (void);
void  RethinkViewportOffsets (Canvas *);
int   CountCanvasPerp (Canvas *);
void  EqualizeCanvas (Canvas *, bool);
void  DupLayoutCv (Canvas *, Canvas *, bool);
void  PutWindowCv (Canvas *);

#define CV_CALL(cv, cmd)		\
{					\
  Display *olddisplay = display;	\
  Layer *oldflayer = flayer;		\
  Layer *l = cv->c_layer;		\
  Canvas *cvlist = l->l_cvlist;		\
  Canvas *cvlnext = cv->c_lnext;	\
  flayer = l;				\
  l->l_cvlist = cv;			\
  cv->c_lnext = NULL;			\
  cmd;					\
  flayer = oldflayer;			\
  l->l_cvlist = cvlist;			\
  cv->c_lnext = cvlnext;		\
  display = olddisplay;			\
}

#endif /* SCREEN_CANVAS_H */
