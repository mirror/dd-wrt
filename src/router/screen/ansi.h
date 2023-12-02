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

#define NATTR		6

#define ATTR_DI		0	/* Dim mode */
#define ATTR_US		1	/* Underscore mode */
#define ATTR_BD		2	/* Bold mode */
#define ATTR_RV		3	/* Reverse mode */
#define ATTR_SO		4	/* Standout mode */
#define ATTR_BL		5	/* Blinking */

#define A_DI	(1<<ATTR_DI)
#define A_US	(1<<ATTR_US)
#define A_BD	(1<<ATTR_BD)
#define A_RV	(1<<ATTR_RV)
#define A_SO	(1<<ATTR_SO)
#define A_BL	(1<<ATTR_BL)
#define A_MAX	(1<<(NATTR-1))

#define ATYP_M		(1<<0)
#define ATYP_S		(1<<1)
#define ATYP_U		(1<<2)

#ifdef COLORS16
/* pseudo attributes */
# define ATTR_BFG	6	/* bright foreground */
# define ATTR_BBG	7	/* bright background */
# define A_BFG	(1<<ATTR_BFG)
# define A_BBG	(1<<ATTR_BBG)
#endif

/*
 *  Parser state
 */
/* keep state_t and state_t_string in sync! */
enum state_t 
{
  LIT,				/* Literal input */
  ESC,				/* Start of escape sequence */
  ASTR,				/* Start of control string */
  STRESC,			/* ESC seen in control string */
  CSI,				/* Reading arguments in "CSI Pn ;...*/
  PRIN,				/* Printer mode */
  PRINESC,			/* ESC seen in printer mode */
  PRINCSI,			/* CSI seen in printer mode */
  PRIN4				/* CSI 4 seen in printer mode */
};

/* keep string_t and string_t_string in sync! */
enum string_t 
{
  NONE,
  DCS,				/* Device control string */
  OSC,				/* Operating system command */
  APC,				/* Application program command */
				/*  - used for status change */
  PM,				/* Privacy message */
  AKA,				/* title for current screen */
  GM,				/* Global message to every display */
  STATUS			/* User hardstatus line */
};

/*
 *  Types of movement used by GotoPos()
 */
enum move_t {
	M_NONE,
	M_UP,
	M_CUP,
	M_DO,
	M_CDO,
	M_LE,
	M_CLE,
	M_RI,
	M_CRI,
	M_RW,
	M_CR	/* CR and rewrite */
};

#define EXPENSIVE	 1000

#define G0			 0
#define G1			 1
#define G2			 2
#define G3			 3

#define ASCII		 0

#ifdef TOPSTAT
#define STATLINE	 (0)
#else
#define STATLINE	 (D_height-1)
#endif

#ifdef ENCODINGS

#define KANJI		('B' & 037)
#define KANJI0212	('D' & 037)
#define KANA    'I'

#define EUC_JP	1
#define SJIS	2
#define EUC_KR	3
#define EUC_CN	4
#define BIG5	5
#define KOI8R	6
#define CP1251	7
#define GBK	20

#define EUC	EUC_JP

#endif

#ifdef UTF8
#undef UTF8
#define UTF8	8
#endif

#ifdef UTF8
# define UCS_REPL    0xfffd  /* character for illegal codes */
# define UCS_REPL_DW 0xff1f  /* character for illegal codes */
# define UCS_HIDDEN 0xffff
#endif

#ifdef DW_CHARS
# define is_dw_font(f) ((f) && ((f) & 0x60) == 0)

# ifdef UTF8
#  define dw_left(ml, x, enc) ((enc == UTF8) ? \
	(unsigned char)(ml)->font[(x) + 1] == 0xff && (unsigned char)(ml)->image[(x) + 1] == 0xff : \
	((unsigned char)(ml)->font[x] & 0x1f) != 0 && ((unsigned char)(ml)->font[x] & 0xe0) == 0 \
	)
#  define dw_right(ml, x, enc) ((enc == UTF8) ? \
	(unsigned char)(ml)->font[x] == 0xff && (unsigned char)(ml)->image[x] == 0xff : \
	((unsigned char)(ml)->font[x] & 0xe0) == 0x80 \
	)
# else
#  define dw_left(ml, x, enc) ( \
	((unsigned char)(ml)->font[x] & 0x1f) != 0 && ((unsigned char)(ml)->font[x] & 0xe0) == 0 \
	)
#  define dw_right(ml, x, enc) ( \
	((unsigned char)(ml)->font[x] & 0xe0) == 0x80 \
	)
# endif /* UTF8 */
#else
# define dw_left(ml, x, enc) 0
# define dw_right(ml, x, enc) 0
#endif
