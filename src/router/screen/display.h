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

#ifndef SCREEN_DISPLAY_H
#define SCREEN_DISPLAY_H

#include "layout.h"
#include "canvas.h"
#include "viewport.h"
#include "comm.h"
#include "image.h"
#include "screen.h"

#define KMAP_KEYS (T_OCAPS-T_CAPS)
#define KMAP_AKEYS (T_OCAPS-T_CURSOR)

#define KMAP_NOTIMEOUT 0x4000

struct kmap_ext {
	char *str;
	int fl;
	struct action um;
	struct action dm;
	struct action mm;
};

typedef enum {
	STATUS_OFF	= 0,
	STATUS_ON_WIN	= 1,
	STATUS_ON_HS	= 2
} DisplayStatus;

typedef enum {
	HSTATUS_IGNORE		= 0,
	HSTATUS_LASTLINE	= 1,
	HSTATUS_MESSAGE		= 2,
	HSTATUS_HS		= 3,
	HSTATUS_FIRSTLINE	= 4,
	HSTATUS_ALWAYS		= (1<<3)
} HardStatus;

struct statusposstr {
	int row, col;
};

#define STATUS_TOP		1
#define STATUS_BOTTOM		0
#define STATUS_LEFT		0
#define STATUS_RIGHT		1


struct mouse_parse {
	char sgrmode;			/* non-zero if parsing an SGR sequence */
	char state;			/* current state of parsing */
	int params[3];			/* parsed params: button, x, y */
};

typedef struct Display Display;
struct Display {
	Display *d_next;		/* linked list */
	struct acluser *d_user;		/* user who owns that display */
	Canvas d_canvas;		/* our canvas slice */
	Canvas *d_cvlist;		/* the canvases of this display */
	Canvas *d_forecv;		/* current input focus */
	Layout *d_layout;	/* layout we're using */
	void (*d_processinput) (char *, size_t);
	struct pwdata *d_processinputdata;	/* data for processinput */
	int d_vpxmin, d_vpxmax;		/* min/max used position on display */
	Window *d_fore;		/* pointer to fore window */
	Window *d_other;		/* pointer to other window */
	int   d_nonblock;		/* -1 don't block if obufmax reached */
					/* >0: block after nonblock secs */
	char  d_termname[MAXTERMLEN + 1]; /* $TERM */
	char	*d_tentry;		/* buffer for tgetstr */
	char	d_tcinited;		/* termcap inited flag */
	int	d_width, d_height;	/* width/height of the screen */
	int	d_defwidth, d_defheight;	/* default width/height of windows */
	int	d_top, d_bot;		/* scrollregion start/end */
	int	d_x, d_y;		/* cursor position */
	struct mchar d_rend;		/* current rendition */
	char	d_atyp;			/* current attribute types */
	int   d_mbcs;			/* saved char for multibytes charset */
	int   d_encoding;		/* what encoding type the display is */
	int   d_decodestate;		/* state of our decoder */
	int   d_realfont;		/* real font of terminal */
	bool	d_insert;		/* insert mode flag */
	int	d_keypad;		/* application keypad flag */
	int	d_cursorkeys;		/* application cursorkeys flag */
	bool	d_revvid;		/* reverse video */
	int	d_curvis;		/* cursor visibility */
	HardStatus	d_has_hstatus;		/* display has hardstatus line */
	bool d_hstatus;		/* hardstatus used */
	int	d_lp_missing;		/* last character on bot line missing */
	int	d_mouse;			/* mouse mode */
	int	d_extmouse;		/* extended mouse mode */
	struct mouse_parse d_mouse_parse;	/* state of mouse code parsing */
	int	d_mousetrack;		/* set when user wants to use mouse even when the window
					   does not */
	int   d_bracketed;		/* bracketed paste mode */
	int   d_cursorstyle;		/* cursor style */
	int   d_xtermosc[5];		/* osc used */
	struct mchar d_lpchar;		/* missing char */
	struct timeval d_status_time;	/* time of status display */
	DisplayStatus   d_status;			/* is status displayed? */
	char	d_status_bell;		/* is it only a vbell? */
	int	d_status_len;		/* length of status line */
	char *d_status_lastmsg;		/* last displayed message */
	int   d_status_buflen;		/* last message buffer len */
	int	d_status_lastx;		/* position of the cursor */
	int	d_status_lasty;		/*   before status was displayed */
	int   d_status_obuflen;		/* saved obuflen */ 
	int   d_status_obuffree;	/* saved obuffree */ 
	int	d_status_obufpos;	/* end of status position in obuf */
	Event d_statusev;	/* timeout event */
	Event d_hstatusev;	/* hstatus changed event */
	int	d_kaablamm;		/* display kaablamm msg */
	struct action *d_ESCseen;	/* Was the last char an ESC (^a) */
	pid_t	d_userpid;		/* pid of attacher */
	char	d_usertty[MAXPATHLEN];	/* tty we are attached to */
	int	d_userfd;		/* fd of the tty */
	Event d_readev;		/* userfd read event */
	Event d_writeev;		/* userfd write event */
	Event d_blockedev;	/* blocked timeout */
	struct mode d_OldMode;		/* tty mode when screen was started */
	struct mode d_NewMode;		/* New tty mode */
	int	d_flow;			/* tty's flow control on/off flag*/
	int   d_intrc;			/* current intr when flow is on */
	char *d_obuf;			/* output buffer */
	int   d_obuflen;		/* len of buffer */
	int	d_obufmax;		/* len where we are blocking the pty */
	int	d_obuflenmax;		/* len - max */
	char *d_obufp;			/* pointer in buffer */
	int   d_obuffree;		/* free bytes in buffer */
	bool	d_auto_nuke;		/* autonuke flag */
	int	d_nseqs;		/* number of valid mappings */
	int	d_aseqs;		/* number of allocated mappings */
	unsigned char  *d_kmaps;	/* keymaps */
	unsigned char *d_seqp;		/* pointer into keymap array */
	int	d_seql;			/* number of parsed chars */
	unsigned char *d_seqh;		/* last hit */
	Event d_mapev;		/* timeout event */
	int	d_dontmap;		/* do not map next */
	int	d_mapdefault;		/* do map next to default */
	union	tcu d_tcs[T_N];		/* terminal capabilities */
	char *d_attrtab[NATTR];		/* attrib emulation table */
	char  d_attrtyp[NATTR];		/* attrib group table */
	int   d_hascolor;		/* do we support color */
	char	d_c0_tab[256];		/* conversion for C0 */
	char ***d_xtable;		/* char translation table */
	int	d_UPcost, d_DOcost, d_LEcost, d_NDcost;
	int	d_CRcost, d_IMcost, d_EIcost, d_NLcost;
	int   d_printfd;		/* fd for vt100 print sequence */
#ifdef ENABLE_UTMP
	slot_t d_loginslot;		/* offset, where utmp_logintty belongs */
	struct utmpx d_utmp_logintty;	/* here the original utmp structure is stored */
	int   d_loginttymode;
#endif
	int   d_blocked;
	int   d_blocked_fuzz;
	Event d_idleev;		/* screen blanker */
	pid_t   d_blankerpid;
	Event d_blankerev;
	Event d_mousetimeoutev;		/* mouse sequence timeout event */
};

#define DISPLAY(x) display->x

#define D_user		DISPLAY(d_user)
#define D_username	(DISPLAY(d_user) ? DISPLAY(d_user)->u_name : 0)
#define D_bracketed	DISPLAY(d_bracketed)
#define D_cursorstyle	DISPLAY(d_cursorstyle)
#define D_canvas	DISPLAY(d_canvas)
#define D_cvlist	DISPLAY(d_cvlist)
#define D_layout	DISPLAY(d_layout)
#define D_forecv	DISPLAY(d_forecv)
#define D_processinput	DISPLAY(d_processinput)
#define D_processinputdata	DISPLAY(d_processinputdata)
#define D_vpxmin	DISPLAY(d_vpxmin)
#define D_vpxmax	DISPLAY(d_vpxmax)
#define D_fore		DISPLAY(d_fore)
#define D_other		DISPLAY(d_other)
#define D_nonblock      DISPLAY(d_nonblock)
#define D_termname	DISPLAY(d_termname)
#define D_tentry	DISPLAY(d_tentry)
#define D_tcinited	DISPLAY(d_tcinited)
#define D_width		DISPLAY(d_width)
#define D_height	DISPLAY(d_height)
#define D_defwidth	DISPLAY(d_defwidth)
#define D_defheight	DISPLAY(d_defheight)
#define D_top		DISPLAY(d_top)
#define D_bot		DISPLAY(d_bot)
#define D_x		DISPLAY(d_x)
#define D_y		DISPLAY(d_y)
#define D_rend		DISPLAY(d_rend)
#define D_atyp		DISPLAY(d_atyp)
#define D_mbcs		DISPLAY(d_mbcs)
#define D_encoding	DISPLAY(d_encoding)
#define D_decodestate	DISPLAY(d_decodestate)
#define D_realfont	DISPLAY(d_realfont)
#define D_insert	DISPLAY(d_insert)
#define D_keypad	DISPLAY(d_keypad)
#define D_cursorkeys	DISPLAY(d_cursorkeys)
#define D_revvid	DISPLAY(d_revvid)
#define D_curvis	DISPLAY(d_curvis)
#define D_has_hstatus	DISPLAY(d_has_hstatus)
#define D_hstatus	DISPLAY(d_hstatus)
#define D_lp_missing	DISPLAY(d_lp_missing)
#define D_mouse		DISPLAY(d_mouse)
#define D_mouse_parse	DISPLAY(d_mouse_parse)
#define D_extmouse	DISPLAY(d_extmouse)
#define D_mousetrack	DISPLAY(d_mousetrack)
#define D_xtermosc	DISPLAY(d_xtermosc)
#define D_lpchar	DISPLAY(d_lpchar)
#define D_status	DISPLAY(d_status)
#define D_status_time	DISPLAY(d_status_time)
#define D_status_bell	DISPLAY(d_status_bell)
#define D_status_len	DISPLAY(d_status_len)
#define D_status_lastmsg	DISPLAY(d_status_lastmsg)
#define D_status_buflen	DISPLAY(d_status_buflen)
#define D_status_lastx	DISPLAY(d_status_lastx)
#define D_status_lasty	DISPLAY(d_status_lasty)
#define D_status_obuflen	DISPLAY(d_status_obuflen)
#define D_status_obuffree	DISPLAY(d_status_obuffree)
#define D_status_obufpos	DISPLAY(d_status_obufpos)
#define D_statusev	DISPLAY(d_statusev)
#define D_hstatusev	DISPLAY(d_hstatusev)
#define D_kaablamm	DISPLAY(d_kaablamm)
#define D_ESCseen	DISPLAY(d_ESCseen)
#define D_userpid	DISPLAY(d_userpid)
#define D_usertty	DISPLAY(d_usertty)
#define D_userfd	DISPLAY(d_userfd)
#define D_OldMode	DISPLAY(d_OldMode)
#define D_NewMode	DISPLAY(d_NewMode)
#define D_flow		DISPLAY(d_flow)
#define D_intr		DISPLAY(d_intr)
#define D_obuf		DISPLAY(d_obuf)
#define D_obuflen	DISPLAY(d_obuflen)
#define D_obufmax	DISPLAY(d_obufmax)
#define D_obuflenmax	DISPLAY(d_obuflenmax)
#define D_obufp		DISPLAY(d_obufp)
#define D_obuffree	DISPLAY(d_obuffree)
#define D_auto_nuke	DISPLAY(d_auto_nuke)
#define D_nseqs		DISPLAY(d_nseqs)
#define D_aseqs		DISPLAY(d_aseqs)
#define D_seqp		DISPLAY(d_seqp)
#define D_seql		DISPLAY(d_seql)
#define D_seqh		DISPLAY(d_seqh)
#define D_dontmap	DISPLAY(d_dontmap)
#define D_mapdefault	DISPLAY(d_mapdefault)
#define D_kmaps		DISPLAY(d_kmaps)
#define D_tcs		DISPLAY(d_tcs)
#define D_attrtab	DISPLAY(d_attrtab)
#define D_attrtyp	DISPLAY(d_attrtyp)
#define D_hascolor	DISPLAY(d_hascolor)
#define D_c0_tab	DISPLAY(d_c0_tab)
#define D_xtable	DISPLAY(d_xtable)
#define D_UPcost	DISPLAY(d_UPcost)
#define D_DOcost	DISPLAY(d_DOcost)
#define D_LEcost	DISPLAY(d_LEcost)
#define D_NDcost	DISPLAY(d_NDcost)
#define D_CRcost	DISPLAY(d_CRcost)
#define D_IMcost	DISPLAY(d_IMcost)
#define D_EIcost	DISPLAY(d_EIcost)
#define D_NLcost	DISPLAY(d_NLcost)
#define D_printfd	DISPLAY(d_printfd)
#define D_loginslot	DISPLAY(d_loginslot)
#define D_utmp_logintty	DISPLAY(d_utmp_logintty)
#define D_loginttymode	DISPLAY(d_loginttymode)
#define D_loginhost	DISPLAY(d_loginhost)
#define D_readev	DISPLAY(d_readev)
#define D_writeev	DISPLAY(d_writeev)
#define D_blockedev	DISPLAY(d_blockedev)
#define D_mapev		DISPLAY(d_mapev)
#define D_blocked	DISPLAY(d_blocked)
#define D_blocked_fuzz	DISPLAY(d_blocked_fuzz)
#define D_idleev	DISPLAY(d_idleev)
#define D_blankerev	DISPLAY(d_blankerev)
#define D_blankerpid	DISPLAY(d_blankerpid)
#define D_mousetimeoutev	DISPLAY(d_mousetimeoutev)


#define GRAIN 4096	/* Allocation grain size for output buffer */
#define OBUF_MAX 256	/* default for obuflimit */

#define OUTPUT_BLOCK_SIZE 256  /* Block size of output to tty */

#define AddChar(c)		\
do				\
  {				\
    if (--D_obuffree <= 0)	\
      Resize_obuf();		\
    *D_obufp++ = (c);		\
  }				\
while (0)

Display *MakeDisplay (char *, char *, char *, int, pid_t, struct mode *);
void  FreeDisplay (void);
void  DefProcess (char **, size_t *);
void  DefRedisplayLine (int, int, int, int);
void  DefClearLine (int, int, int, int);
int   DefResize (int, int);
void  DefRestore (void);
void  AddCStr (char *);
void  AddCStr2 (char *, int);
void  InitTerm (int);
void  FinitTerm (void);
void  PUTCHAR (uint32_t);
void  PUTCHARLP (uint32_t);
void  ClearAll (void);
void  ClearArea (int, int, int, int, int, int, int, int);
void  ClearLine (struct mline *, int, int, int, int);
void  RefreshAll (int);
void  RefreshArea (int, int, int, int, int);
void  RefreshLine (int, int, int, int);
void  Redisplay (int);
void  RedisplayDisplays (int);
void  ShowHStatus (char *);
void  RefreshHStatus (void);
void  DisplayLine (struct mline *, struct mline *, int, int, int);
void  GotoPos (int, int);
int   CalcCost (char *);
void  ScrollH (int, int, int, int, int, struct mline *);
void  ScrollV (int, int, int, int, int, int);
void  PutChar (struct mchar *, int, int);
void  InsChar (struct mchar *, int, int, int, struct mline *);
void  WrapChar (struct mchar *, int, int, int, int, int, int, bool);
void  ChangeScrollRegion (int, int);
void  InsertMode (bool);
void  KeypadMode (int);
void  CursorkeysMode (int);
void  ReverseVideo (bool);
void  CursorVisibility (int);
void  MouseMode (int);
void  ExtMouseMode (int);
void  BracketedPasteMode (bool);
void  CursorStyle (int);
void  SetRendition (struct mchar *);
void  SetRenditionMline (struct mline *, int);
void  MakeStatus (char *);
void  RemoveStatus (void);
int   ResizeDisplay (int, int);
void  AddStr (char *);
void  AddStrn (char *, int);
void  Flush (int);
void  freetty (void);
void  Resize_obuf (void);
void  NukePending (void);
void  ClearAllXtermOSC (void);
void  SetXtermOSC (int, char *, char *);
void  ResetIdle (void);
void  RunBlanker (char **);
void  KillBlanker (void);
void  DisplaySleep1000 (int, int);
void  ClearScrollbackBuffer (void);

/* global variables */

extern bool defautonuke;

extern int captionalways;
extern int captiontop;
extern int defmousetrack;
extern int defnonblock;
extern int defobuflimit;
extern int focusminheight;
extern int focusminwidth;
extern int hardstatusemu;

extern Display *display, *displays;

extern const struct LayFuncs BlankLf;

extern struct statusposstr statuspos;

#endif /* SCREEN_DISPLAY_H */
