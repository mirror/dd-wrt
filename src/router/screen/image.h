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

#ifndef SCREEN_IMAGE_H
#define SCREEN_IMAGE_H

#include <stdint.h>

/* structure representing single cell of terminal */
struct mchar {
	uint32_t image;		/* actual letter like a, b, c ... as Unicode code point */
	uint32_t attr;		/* attributes - bold, standout etc. */
	uint32_t font;		/* font :) */
	uint32_t colorbg;	/* background color */
	uint32_t colorfg;	/* foreground color */
	uint32_t mbcs;		/* used for multi byte character sets; TODO: possible to remove? use image now that it has 32 bits*/
};

struct mline {
	uint32_t *image;
	uint32_t *attr;
	uint32_t *font;
	uint32_t *colorbg;
	uint32_t *colorfg;
};



#define save_mline(ml, n) {					\
	memmove(mline_old.image,   (ml)->image,   (n) * 4);	\
	memmove(mline_old.attr,    (ml)->attr,    (n) * 4);	\
	memmove(mline_old.font,    (ml)->font,    (n) * 4);	\
	memmove(mline_old.colorbg, (ml)->colorbg, (n) * 4);	\
	memmove(mline_old.colorfg, (ml)->colorfg, (n) * 4);	\
}

#define copy_mline(ml, xf, xt, n) {					\
	memmove((ml)->image   + (xt), (ml)->image   + (xf), (n) * 4);	\
	memmove((ml)->attr    + (xt), (ml)->attr    + (xf), (n) * 4);	\
	memmove((ml)->font    + (xt), (ml)->font    + (xf), (n) * 4);	\
	memmove((ml)->colorbg + (xt), (ml)->colorbg + (xf), (n) * 4);	\
	memmove((ml)->colorfg + (xt), (ml)->colorfg + (xf), (n) * 4);	\
}

#define clear_mline(ml, x, n) {							\
	memmove((ml)->image + (x), blank, (n) * 4);				\
	if ((ml)->attr    != null) memset((ml)->attr    + (x), 0, (n) * 4);	\
	if ((ml)->font    != null) memset((ml)->font    + (x), 0, (n) * 4);	\
	if ((ml)->colorbg != null) memset((ml)->colorbg + (x), 0, (n) * 4);	\
	if ((ml)->colorfg != null) memset((ml)->colorfg + (x), 0, (n) * 4);	\
}

#define cmp_mline(ml1, ml2, x) (			\
	   (ml1)->image[x]   == (ml2)->image[x]		\
	&& (ml1)->attr[x]    == (ml2)->attr[x]		\
	&& (ml1)->font[x]    == (ml2)->font[x]		\
	&& (ml1)->colorbg[x] == (ml2)->colorbg[x]	\
	&& (ml1)->colorfg[x] == (ml2)->colorfg[x]	\
)

#define cmp_mchar(mc1, mc2) (				\
	    (mc1)->image  == (mc2)->image		\
	&& (mc1)->attr    == (mc2)->attr		\
	&& (mc1)->font    == (mc2)->font		\
	&& (mc1)->colorbg == (mc2)->colorbg		\
	&& (mc1)->colorfg == (mc2)->colorfg		\
)

#define cmp_mchar_mline(mc, ml, x) (			\
	   (mc)->image   == (ml)->image[x]		\
	&& (mc)->attr    == (ml)->attr[x]		\
	&& (mc)->font    == (ml)->font[x]		\
	&& (mc)->colorbg == (ml)->colorbg[x]		\
	&& (mc)->colorfg == (ml)->colorfg[x]		\
)

#define copy_mchar2mline(mc, ml, x) {			\
	(ml)->image[x]   = (mc)->image;			\
	(ml)->attr[x]    = (mc)->attr;			\
	(ml)->font[x]    = (mc)->font;			\
	(ml)->colorbg[x] = (mc)->colorbg;		\
	(ml)->colorfg[x] = (mc)->colorfg;		\
}

#define copy_mline2mchar(mc, ml, x) {			\
	(mc)->image   = (ml)->image[x];			\
	(mc)->attr    = (ml)->attr[x];			\
	(mc)->font    = (ml)->font[x];			\
	(mc)->colorbg = (ml)->colorbg[x];		\
	(mc)->colorfg = (ml)->colorfg[x];		\
	(mc)->mbcs    = 0;				\
}

enum {
	REND_BELL = 0,
	REND_MONITOR,
	REND_SILENCE,
	NUM_RENDS
};

#endif /* SCREEN_IMAGE_H */
