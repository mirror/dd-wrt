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


#undef IFFONT
#undef IFCOLOR

#ifdef FONT
# define IFFONT(x) x
# define IFFONTX(x) x
#else
# define IFFONT(x)
# define IFFONTX(x)
#endif

#ifdef COLOR
# define IFCOLOR(x) x
#else
# define IFCOLOR(x)
#endif

#if defined(COLOR) && defined(COLORS16) && defined(COLORS256)
# define IFCOLORX(x) x
#else
# define IFCOLORX(x)
#endif

#ifdef DW_CHARS
# define IFDWCHAR(x) x
#else
# define IFDWCHAR(x)
#endif

struct mchar {
	 unsigned char image;
	 unsigned char attr;
IFFONT(  unsigned char font; )
IFFONTX( unsigned char fontx; )
IFCOLOR( unsigned char color; )
IFCOLORX(unsigned char colorx; )
IFDWCHAR(unsigned char mbcs; )
};

struct mline {
	 unsigned char *image;
	 unsigned char *attr;
IFFONT(  unsigned char *font; )
IFFONTX( unsigned char *fontx; )
IFCOLOR( unsigned char *color; )
IFCOLORX(unsigned char *colorx; )
};



#define save_mline(ml, n) do {						\
	 bcopy((char *)(ml)->image, (char *)mline_old.image, (n));	\
	 bcopy((char *)(ml)->attr,  (char *)mline_old.attr,  (n));	\
IFFONT(	 bcopy((char *)(ml)->font,  (char *)mline_old.font,  (n));    ) \
IFFONTX( bcopy((char *)(ml)->fontx, (char *)mline_old.fontx, (n));    ) \
IFCOLOR( bcopy((char *)(ml)->color, (char *)mline_old.color, (n));    ) \
IFCOLORX(bcopy((char *)(ml)->colorx, (char *)mline_old.colorx, (n));  ) \
} while (0)

#define bcopy_mline(ml, xf, xt, n) do {					       \
	 bcopy((char *)(ml)->image + (xf), (char *)(ml)->image + (xt), (n));   \
	 bcopy((char *)(ml)->attr  + (xf), (char *)(ml)->attr  + (xt), (n));   \
IFFONT(	 bcopy((char *)(ml)->font  + (xf), (char *)(ml)->font  + (xt), (n)); ) \
IFFONTX( bcopy((char *)(ml)->fontx + (xf), (char *)(ml)->fontx + (xt), (n)); ) \
IFCOLOR( bcopy((char *)(ml)->color + (xf), (char *)(ml)->color + (xt), (n)); ) \
IFCOLORX(bcopy((char *)(ml)->colorx + (xf), (char *)(ml)->colorx + (xt), (n));) \
} while (0)

#define clear_mline(ml, x, n) do {					       \
	 bclear((char *)(ml)->image + (x), (n));			       \
	 if ((ml)->attr != null) bzero((char *)(ml)->attr  + (x), (n));	       \
IFFONT(  if ((ml)->font != null) bzero((char *)(ml)->font  + (x), (n));      ) \
IFFONTX( if ((ml)->fontx != null) bzero((char *)(ml)->fontx + (x), (n));      ) \
IFCOLOR( if ((ml)->color!= null) bzero((char *)(ml)->color + (x), (n));      ) \
IFCOLORX(if ((ml)->colorx!= null) bzero((char *)(ml)->colorx + (x), (n));    ) \
} while (0)

#define cmp_mline(ml1, ml2, x) (				\
	    (ml1)->image[x] == (ml2)->image[x]			\
	 && (ml1)->attr[x]  == (ml2)->attr[x]			\
IFFONT(	 && (ml1)->font[x]  == (ml2)->font[x]		      ) \
IFFONTX( && (ml1)->fontx[x] == (ml2)->fontx[x]		      ) \
IFCOLOR( && (ml1)->color[x] == (ml2)->color[x]		      ) \
IFCOLORX(&& (ml1)->colorx[x] == (ml2)->colorx[x]	      ) \
)

#define cmp_mchar(mc1, mc2) (					\
	    (mc1)->image == (mc2)->image			\
	 && (mc1)->attr  == (mc2)->attr				\
IFFONT(	 && (mc1)->font  == (mc2)->font			      ) \
IFFONTX( && (mc1)->fontx == (mc2)->fontx		      ) \
IFCOLOR( && (mc1)->color == (mc2)->color		      ) \
IFCOLORX(&& (mc1)->colorx == (mc2)->colorx		      ) \
)

#define cmp_mchar_mline(mc, ml, x) (				\
	    (mc)->image == (ml)->image[x]			\
	 && (mc)->attr  == (ml)->attr[x]			\
IFFONT(	 && (mc)->font  == (ml)->font[x]		      ) \
IFFONTX( && (mc)->fontx == (ml)->fontx[x]		      ) \
IFCOLOR( && (mc)->color == (ml)->color[x]		      ) \
IFCOLORX(&& (mc)->colorx == (ml)->colorx[x]		      ) \
)

#define copy_mchar2mline(mc, ml, x) do {			\
	 (ml)->image[x] = (mc)->image;				\
	 (ml)->attr[x]  = (mc)->attr;				\
IFFONT(	 (ml)->font[x]  = (mc)->font;			      ) \
IFFONTX( (ml)->fontx[x] = (mc)->fontx;			      ) \
IFCOLOR( (ml)->color[x] = (mc)->color;			      ) \
IFCOLORX((ml)->colorx[x] = (mc)->colorx;		      ) \
} while (0)

#define copy_mline2mchar(mc, ml, x) do {			\
	 (mc)->image = (ml)->image[x];				\
	 (mc)->attr  = (ml)->attr[x];				\
IFFONT(	 (mc)->font  = (ml)->font[x];			      ) \
IFFONTX( (mc)->fontx = (ml)->fontx[x];			      ) \
IFCOLOR( (mc)->color = (ml)->color[x];			      ) \
IFCOLORX((mc)->colorx = (ml)->colorx[x];		      ) \
IFDWCHAR((mc)->mbcs  = 0;				      ) \
} while (0)

#ifdef COLOR
# ifdef COLORS16
#  ifdef COLORS256
#   define rend_getbg(mc) (((mc)->color & 0xf0) >> 4 | ((mc)->attr & A_BBG ? 0x100 : 0) | ((mc)->colorx & 0xf0))
#   define rend_setbg(mc, c) ((mc)->color = ((mc)->color & 0x0f) | (c << 4 & 0xf0), (mc)->colorx = ((mc)->colorx & 0x0f) | (c & 0xf0), (mc)->attr = ((mc)->attr | A_BBG) ^ (c & 0x100 ? 0 : A_BBG))
#   define rend_getfg(mc) (((mc)->color & 0x0f) | ((mc)->attr & A_BFG ? 0x100 : 0) | (((mc)->colorx & 0x0f) << 4))
#   define rend_setfg(mc, c) ((mc)->color = ((mc)->color & 0xf0) | (c & 0x0f), (mc)->colorx = ((mc)->colorx & 0xf0) | ((c & 0xf0) >> 4), (mc)->attr = ((mc)->attr | A_BFG) ^ (c & 0x100 ? 0 : A_BFG))
#   define rend_setdefault(mc) ((mc)->color = (mc)->colorx = 0, (mc)->attr &= ~(A_BBG|A_BFG))
#  else
#   define rend_getbg(mc) (((mc)->color & 0xf0) >> 4 | ((mc)->attr & A_BBG ? 0x100 : 0))
#   define rend_setbg(mc, c) ((mc)->color = ((mc)->color & 0x0f) | (c << 4 & 0xf0), (mc)->attr = ((mc)->attr | A_BBG) ^ (c & 0x100 ? 0 : A_BBG))
#   define rend_getfg(mc) (((mc)->color & 0x0f) | ((mc)->attr & A_BFG ? 0x100 : 0))
#   define rend_setfg(mc, c) ((mc)->color = ((mc)->color & 0xf0) | (c & 0x0f), (mc)->attr = ((mc)->attr | A_BFG) ^ (c & 0x100 ? 0 : A_BFG))
#   define rend_setdefault(mc) ((mc)->color = 0, (mc)->attr &= ~(A_BBG|A_BFG))
#  endif
#  define coli2e(c) ((((c) & 0x1f8) == 0x108 ? (c) ^ 0x108 : (c & 0xff)) ^ 9)
#  define cole2i(c) ((c) >= 8 && (c) < 16 ? (c) ^ 0x109 : (c) ^ 9)
# else
#  define rend_getbg(mc) (((mc)->color & 0xf0) >> 4)
#  define rend_setbg(mc, c) ((mc)->color = ((mc)->color & 0x0f) | (c << 4 & 0xf0))
#  define rend_getfg(mc) ((mc)->color & 0x0f)
#  define rend_setfg(mc, c) ((mc)->color = ((mc)->color & 0xf0) | (c & 0x0f))
#  define rend_setdefault(mc) ((mc)->color = 0)
#  define coli2e(c) ((c) ^ 9)
#  define cole2i(c) ((c) ^ 9)
# endif
#endif

enum
{
  REND_BELL = 0,
  REND_MONITOR,
  REND_SILENCE,
  NUM_RENDS
};

