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

#ifndef SCREEN_LAYER_H
#define SCREEN_LAYER_H

/*
 * This is the overlay structure. It is used to create a seperate
 * layer over the current windows.
 */

struct mchar;	/* forward declaration */

struct LayFuncs
{
  void	(*lf_LayProcess) __P((char **, int *));
  void	(*lf_LayAbort) __P((void));
  void	(*lf_LayRedisplayLine) __P((int, int, int, int));
  void	(*lf_LayClearLine) __P((int, int, int, int));
  int	(*lf_LayRewrite) __P((int, int, int, struct mchar *, int));
  int	(*lf_LayResize) __P((int, int));
  void	(*lf_LayRestore) __P((void));
  void  (*lf_LayFree) __P((void *));	/* Should only free any data kept in
					   flayer->l_data (but not flayer->l_data itself). */
};

struct layer
{
  struct canvas *l_cvlist;	/* list of canvases displaying layer */
  int	 l_width;
  int	 l_height;
  int    l_x;			/* cursor position */
  int    l_y;
  int    l_encoding;
  struct LayFuncs *l_layfn;
  void	*l_data;

  struct layer *l_next;		/* layer stack, should be in data? */
  struct layer *l_bottom;	/* bottom element of layer stack */
  int	 l_blocking;
  int	 l_mode;		/* non-zero == edit mode */

  struct {
    unsigned char buffer[3];	/* [0]: the button
				   [1]: x
				   [2]: y
				*/
    int len;
    int start;
  } l_mouseevent;

  struct {
    int d : 1;		/* Is the output for the layer blocked? */

    /* After unpausing, what region should we refresh? */
    int *left, *right;
    int top, bottom;
    int lines;
  } l_pause;
};

#define LayProcess		(*flayer->l_layfn->lf_LayProcess)
#define LayAbort		(*flayer->l_layfn->lf_LayAbort)
#define LayRedisplayLine	(*flayer->l_layfn->lf_LayRedisplayLine)
#define LayClearLine		(*flayer->l_layfn->lf_LayClearLine)
#define LayRewrite		(*flayer->l_layfn->lf_LayRewrite)
#define LayResize		(*flayer->l_layfn->lf_LayResize)
#define LayRestore		(*flayer->l_layfn->lf_LayRestore)
#define LayFree		(*flayer->l_layfn->lf_LayFree)

#define LaySetCursor()	LGotoPos(flayer, flayer->l_x, flayer->l_y)
#define LayCanResize(l)	(l->l_layfn->LayResize != DefResize)

/* XXX: AArgh! think again! */

#define LAY_CALL_UP(fn) do				\
	{ 						\
	  struct layer *oldlay = flayer; 		\
	  struct canvas *oldcvlist, *cv;		\
	  debug("LayCallUp\n");				\
	  flayer = flayer->l_next;			\
	  oldcvlist = flayer->l_cvlist;			\
	  debug1("oldcvlist: %x\n", oldcvlist);		\
	  flayer->l_cvlist = oldlay->l_cvlist;		\
	  for (cv = flayer->l_cvlist; cv; cv = cv->c_lnext)	\
		cv->c_layer = flayer;			\
	  fn;						\
	  flayer = oldlay;				\
	  for (cv = flayer->l_cvlist; cv; cv = cv->c_lnext)	\
		cv->c_layer = flayer;			\
	  flayer->l_next->l_cvlist = oldcvlist;		\
	} while(0)

#define LAY_DISPLAYS(l, fn) do				\
	{ 						\
	  struct display *olddisplay = display;		\
	  struct canvas *cv;				\
	  for (display = displays; display; display = display->d_next) \
	    {						\
	      for (cv = D_cvlist; cv; cv = cv->c_next)	\
		if (cv->c_layer == l)			\
		  break;				\
	      if (cv == 0)				\
		continue;				\
	      fn;					\
	    }						\
	  display = olddisplay;				\
	} while(0)

#endif /* SCREEN_LAYER_H */

/**
 * (Un)Pauses a layer.
 *
 * @param layer The layer that should be (un)paused.
 * @param pause Should we pause the layer?
 */
void LayPause __P((struct layer *layer, int pause));

/**
 * Update the region to refresh after a layer is unpaused.
 *
 * @param layer The layer.
 * @param xs	The left-end of the region.
 * @param xe	The right-end of the region.
 * @param ys	The top-end of the region.
 * @param ye	The bottom-end of the region.
 */
void LayPauseUpdateRegion __P((struct layer *layer, int xs, int xe, int ys, int ye));

/**
 * Free any internal memory for the layer.
 *
 * @param layer The layer.
 */
void LayerCleanupMemory __P((struct layer *layer));

