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

#ifndef SCREEN_ANSI_H
#define SCREEN_ANSI_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define NATTR		7

				/*			Capname	TCapCode */
#define ATTR_DI		0	/* Dim mode		dim	mh	*/
#define ATTR_US		1	/* Underscore mode	smul	us	*/
#define ATTR_BD		2	/* Bold mode		bold	md	*/
#define ATTR_RV		3	/* Reverse mode		rev	mr	*/
#define ATTR_SO		4	/* Standout mode	smso	so	*/
#define ATTR_BL		5	/* Blinking		blink	mb	*/
#define ATTR_IT		6	/* Italicized		simt	ZH	*/

#define A_DI	(1<<ATTR_DI)
#define A_US	(1<<ATTR_US)
#define A_BD	(1<<ATTR_BD)
#define A_RV	(1<<ATTR_RV)
#define A_SO	(1<<ATTR_SO)
#define A_BL	(1<<ATTR_BL)
#define A_IT	(1<<ATTR_IT)

#define ATYP_M		(1<<0)
#define ATYP_S		(1<<1)
#define ATYP_U		(1<<2)

/*
 *  Parser state
 */
/* keep state_t and state_t_string in sync! */
enum state_t {
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
enum string_t {
	NONE,
	DCS,				/* Device control string */
	OSC,				/* Operating system command */
	APC,				/* Application program command */
					/*  - used for status change */
	PM,				/* Privacy message */
	AKA,				/* title for current screen */
	GM,				/* Global message to every display */
	STATUS				/* User hardstatus line */
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

#define G0		 0
#define G1		 1
#define G2		 2
#define G3		 3

#define ASCII		 0

#define STATLINE()	 	(statuspos.row == STATUS_BOTTOM ? D_height-1 : 0)
#define	STATCOL(width, len)	(statuspos.col == STATUS_LEFT ? 0 : D_width - D_status_len - 2)

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
#define KOI8U	21

#define EUC	EUC_JP


#define UTF8	8

#define UCS_REPL    0xfffd  /* character for illegal codes */
#define UCS_REPL_DW 0xff1f  /* character for illegal codes */
#define UCS_HIDDEN  0xffff

#define is_dw_font(f) ((f) && ((f) & 0x60) == 0)

#define dw_left(ml, x, enc) ((enc == UTF8) ? \
	(ml)->font[(x) + 1] == 0xff && (ml)->image[(x) + 1] == 0xff : \
	((ml)->font[x] & 0x1f) != 0 && ((ml)->font[x] & 0xe0) == 0 \
	)
#define dw_right(ml, x, enc) ((enc == UTF8) ? \
	(ml)->font[x] == 0xff && (ml)->image[x] == 0xff : \
	((ml)->font[x] & 0xe0) == 0x80 \
	)

typedef struct Window Window;

void  ResetAnsiState (Window *);
void  ResetCharsets (Window *);
void  WriteString (Window *, char *, size_t);
void  ChangeAKA (Window *, char *, size_t);
void  SetCharsets (Window *, char *);
int   GetAnsiStatus (Window *, char *);
void  WNewAutoFlow (Window *, int);
void  WBell (Window *, bool);
void  WMsg (Window *, int, char *);
int   MFindUsedLine (Window *, int, int);

/* global variables */

extern bool visual_bell;
extern bool use_altscreen;
extern bool use_hardstatus;

extern char *printcmd;

extern uint32_t *blank;
extern uint32_t *null;

extern uint64_t renditions[];

extern const int Z0width;
extern const int Z1width;

extern struct mline mline_blank;
extern struct mline mline_null;
extern struct mline mline_old;

extern struct mchar mchar_so;
extern struct mchar mchar_blank;
extern struct mchar mchar_null;

#endif /* SCREEN_ANSI_H */
