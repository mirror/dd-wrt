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
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 * $Id$ GNU
 */

#ifndef SCREEN_WINDOW_H
#define SCREEN_WINDOW_H

/* keep this in sync with the initialisations in window.c */
struct NewWindow
{
  int	StartAt;	/* where to start the search for the slot */
  char	*aka;		/* aka string */
  char	**args;		/* argv vector */
  char	*dir;		/* directory for chdir */
  char	*term;		/* TERM to be set instead of "screen" */
  int	aflag;
  int   dynamicaka;
  int	flowflag;
  int	lflag;
  int	histheight;
  int	monitor;
  int   wlock;		/* default writelock setting */
  int	silence;
  int   wrap;
  int	Lflag;		/* logging */
  int	slow;		/* inter character milliseconds */
  int   gr;
  int   c1;
  int   bce;
  int   encoding;
  char	*hstatus;
  char	*charset;
  int	poll_zombie_timeout;
};

#ifdef PSEUDOS

struct pseudowin
{
  int	p_fdpat;
  int	p_pid;
  int	p_ptyfd;
  struct event p_readev;
  struct event p_writeev;
  char	p_cmd[MAXSTR];
  char	p_tty[MAXSTR];
  char	p_inbuf[IOSIZE];	/* buffered writing to p_ptyfd */
  int	p_inlen;
};

/* bits for fdpat: */
#define F_PMASK 	0x0003
#define F_PSHIFT	2
#define F_PFRONT	0x0001			/* . */
#define F_PBACK 	0x0002			/* ! */
#define F_PBOTH 	(F_PFRONT | F_PBACK)	/* : */

#define F_UWP		0x1000			/* | */

/* The screen process ...)
 * ... wants to write to pseudo */
#define W_WP(w) ((w)->w_pwin && ((w)->w_pwin->p_fdpat & F_PFRONT))

/* ... wants to write to window: user writes to window 
 * or stdout/stderr of pseudo are duplicated to window */
#define W_WW(w) (!((w)->w_pwin) || \
(((w)->w_pwin->p_fdpat & F_PMASK) == F_PBACK) || \
((((w)->w_pwin->p_fdpat >> F_PSHIFT) & F_PMASK) == F_PBOTH) || \
((((w)->w_pwin->p_fdpat >> (F_PSHIFT * 2)) & F_PMASK) == F_PBOTH))

/* ... wants to read from pseudowin */
#define W_RP(w) ((w)->w_pwin && ((w)->w_pwin->p_fdpat & \
((F_PFRONT << (F_PSHIFT * 2)) | (F_PFRONT << F_PSHIFT)) ))

/* ... wants to read from window */
#define W_RW(w) (!((w)->w_pwin) || ((w)->w_pwin->p_fdpat & F_PFRONT))

/* user input is written to pseudo */
#define W_UWP(w) ((w)->w_pwin && ((w)->w_pwin->p_fdpat & F_UWP))

/* pseudo output has to be stuffed in window */
#define W_PTOW(w) (\
((w)->w_pwin->p_fdpat & F_PMASK << F_PSHIFT) == F_PBOTH << F_PSHIFT || \
((w)->w_pwin->p_fdpat & F_PMASK << F_PSHIFT * 2) == F_PBOTH << F_PSHIFT * 2 )

/* window output has to be stuffed in pseudo */
#define W_WTOP(w) (((w)->w_pwin->p_fdpat & F_PMASK) == F_PBOTH)

#endif /* PSEUDOS */

/* definitions for wlocktype */
#define WLOCK_OFF	0	/* all in w_userbits can write */
#define WLOCK_AUTO	1	/* who selects first, can write */
#define WLOCK_ON	2	/* user writes even if deselected */


#ifdef COPY_PASTE
struct paster
{
  char	*pa_pastebuf;		/* this gets pasted in the window */
  char	*pa_pasteptr;		/* pointer in pastebuf */
  int	 pa_pastelen;		/* bytes left to paste */
  struct layer *pa_pastelayer;	/* layer to paste into */
  struct event pa_slowev;	/* slowpaste event */
};
#else
struct paster;
#endif

struct win
{
  struct win *w_next;		/* next window */
  int    w_type;		/* type of window */
  void  *w_data;
  struct layer w_layer;		/* our layer */
  struct layer *w_savelayer;	/* the layer to keep */
  int    w_blocked;		/* block input */
#ifdef PSEUDOS
  struct pseudowin *w_pwin;	/* ptr to pseudo */
#endif
  struct display *w_pdisplay;	/* display for printer relay */
  struct display *w_lastdisp;	/* where the last input was made */
  int	 w_number;		/* window number */
  struct event w_readev;
  struct event w_writeev;
  struct event w_silenceev;	/* silence event */
  struct event w_zombieev;	/* event to try to resurrect window */
  int	 w_poll_zombie_timeout;
  int	 w_ptyfd;		/* fd of the master pty */
  char	 w_inbuf[IOSIZE];
  int	 w_inlen;
  char	 w_outbuf[IOSIZE];
  int	 w_outlen;
  int	 w_aflag;		/* (-a option) */
  int	 w_dynamicaka;		/* should we change name */
  char  *w_title;		/* name of the window */
  char  *w_akachange;		/* autoaka hack */
  char	 w_akabuf[MAXSTR];	/* aka buffer */
  int	 w_autoaka;		/* autoaka hack */
  struct win *w_group;		/* window group we belong to */
  int	 w_intermediate;	/* char used while parsing ESC-seq */
  int	 w_args[MAXARGS];	/* emulator args */
  int	 w_NumArgs;

#ifdef MULTIUSER
  int    w_wlock;		/* WLOCK_AUTO, WLOCK_OFF, WLOCK_ON */
  struct acluser *w_wlockuser;	/* NULL when unlocked or user who writes */
  AclBits w_userbits[ACL_BITS_PER_WIN];
  AclBits w_lio_notify;		/* whom to tell when lastio+seconds < time() */
  AclBits w_mon_notify;		/* whom to tell monitor statis */
#endif

  enum state_t w_state;		/* parser state */
  enum string_t w_StringType;
  struct mline *w_mlines;
  struct mchar w_rend;		/* current rendition */
#ifdef FONT
  char	 w_FontL;		/* character font GL */
  char	 w_FontR;		/* character font GR */
# ifdef ENCODINGS
  char	 w_FontE;		/* character font GR locked */
# endif
  int	 w_Charset;		/* charset number GL */
  int	 w_CharsetR;		/* charset number GR */
  int	 w_charsets[4];		/* Font = charsets[Charset] */
#endif
  int	 w_ss;
  struct cursor {
    int on;
    int	 x, y;
    struct mchar Rend;
#ifdef FONT
    int	 Charset;
    int	 CharsetR;
    int	 Charsets[4];
#endif
  } w_saved;
  int	 w_top, w_bot;		/* scrollregion */
  int	 w_wrap;		/* autowrap */
  int	 w_origin;		/* origin mode */
  int	 w_insert;		/* window is in insert mode */
  int	 w_keypad;		/* keypad mode */
  int	 w_cursorkeys;		/* appl. cursorkeys mode */
  int	 w_revvid;		/* reverse video */
  int	 w_curinv;		/* cursor invisible */
  int	 w_curvvis;		/* cursor very visible */
  int	 w_autolf;		/* automatic linefeed */
  char  *w_hstatus;		/* hardstatus line */
  int	 w_gr;			/* enable GR flag */
  int	 w_c1;			/* enable C1 flag */
  int	 w_bce;			/* enable backcol erase */
#if 0
  int    w_encoding;		/* for input and paste */
#endif
  int    w_decodestate;		/* state of our input decoder */
#ifdef DW_CHARS
  int    w_mbcs;		/* saved char for multibytes charset */
#endif
  char	 w_string[MAXSTR];
  char	*w_stringp;
  char	*w_tabs;		/* line with tabs */
  int	 w_bell;		/* bell status of this window */
  int	 w_flow;		/* flow flags */
  struct logfile *w_log;	/* log to file */
  int    w_logsilence;		/* silence in secs */
  int	 w_monitor;		/* monitor status */
  int	 w_silencewait;		/* wait for silencewait secs */
  int	 w_silence;		/* silence status (Lloyd Zusman) */
  char	 w_vbwait;
  char	 w_norefresh;		/* dont redisplay when switching to that win */
#ifdef RXVT_OSC
  char	 w_xtermosc[4][MAXSTR];	/* special xterm/rxvt escapes */
#endif
  int    w_mouse;		/* mouse mode 0,9,1000 */
#ifdef HAVE_BRAILLE
  int	 w_bd_x, w_bd_y;	/* Braille cursor position */
#endif

#ifdef COPY_PASTE
  int    w_slowpaste;		/* do careful writes to the window */
  int	 w_histheight;		/* all histbases are malloced with width * histheight */
  int	 w_histidx;		/* 0 <= histidx < histheight; where we insert lines */
  int	 w_scrollback_height;	/* number of lines of output stored, to be updated with w_histidx, w_histheight */
  struct mline *w_hlines;	/* history buffer */
  struct paster w_paster;	/* paste info */
#else
  int	 w_histheight;		/* always 0 */
#endif
  int	 w_pid;			/* process at the other end of ptyfd */
  int	 w_deadpid;		/* saved w_pid of a process that closed the ptyfd to us */

  char  *w_cmdargs[MAXARGS];	/* command line argument vector */
  char	*w_dir;			/* directory for chdir */
  char	*w_term;		/* TERM to be set instead of "screen" */

  int    w_lflag;		/* login flag */
  slot_t w_slot;		/* utmp slot */
#if defined (UTMPOK)
  struct utmp w_savut;		/* utmp entry of this window */
#endif

  char	 w_tty[MAXSTR];

  int    w_zauto;
#ifdef ZMODEM
  struct display *w_zdisplay;
#endif
#ifdef BUILTIN_TELNET
  struct sockaddr_storage w_telsa;
  char   w_telbuf[IOSIZE];
  int    w_telbufl;
  char   w_telmopts[256];
  char   w_telropts[256];
  int    w_telstate;
  char   w_telsubbuf[128];
  int    w_telsubidx;
  struct event w_telconnev;
#endif
  struct {
    int    on;    /* Is the alternate buffer currently being used? */
    struct mline *mlines;
    int    width;
    int    height;
#ifdef COPY_PASTE
    int    histheight;
    struct mline *hlines;
    int    histidx;
#else
    int histheight;	/* 0 */
#endif
    struct cursor cursor;
  } w_alt;

  struct event w_destroyev;	/* window destroy event */
#ifdef BSDWAIT
  union wait w_exitstatus;	/* window exit status */
#else
  int w_exitstatus;
#endif
};


#define w_encoding   w_layer.l_encoding
#define w_width  w_layer.l_width
#define w_height w_layer.l_height
#define w_x      w_layer.l_x
#define w_y      w_layer.l_y

/* definitions for w_type */
#define W_TYPE_PTY		0
#define W_TYPE_PLAIN		1
#define W_TYPE_TELNET		2
#define W_TYPE_GROUP		3


/*
 * Definitions for flow
 *   000  -(-)	flow off, auto would be off
 *   001  +(-)	flow on , auto would be off
 *   010  -(+)	flow off, auto would be on
 *   011  +(+)	flow on , auto would be on
 *   100  -	flow auto, currently off
 *   111  +	flow auto, currently on
 * Application controls auto_flow via TIOCPKT, if available, 
 * else via application keypad mode.
 */
#define FLOW_NOW	(1<<0)
#define FLOW_AUTO	(1<<1)
#define FLOW_AUTOFLAG	(1<<2)


/*
 * WIN gives us a reference to line y of the *whole* image
 * where line 0 is the oldest line in our history.
 * y must be in whole image coordinate system, not in display.
 */

#define WIN(y) ((y < fore->w_histheight) ? \
      &fore->w_hlines[(fore->w_histidx + y) % fore->w_histheight] \
    : &fore->w_mlines[y - fore->w_histheight])

#define Layer2Window(l) ((struct win *)(l)->l_bottom->l_data)

int WindowChangeNumber __P((int, int));

#endif /* SCREEN_WINDOW_H */

