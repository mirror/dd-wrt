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
 */

#include "config.h"

#include "display.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "screen.h"

#include "canvas.h"
#include "encoding.h"
#include "mark.h"
#include "misc.h"
#include "process.h"
#include "pty.h"
#include "resize.h"
#include "termcap.h"
#include "tty.h"
#include "winmsg.h"

/* CSI parsing status */
enum {
	CSI_PB=0, CSI_PX=1, CSI_PY=2, CSI_DONE=3,
	CSI_ESC_SEEN, CSI_BEGIN, CSI_INACTIVE, CSI_INVALID
};

static int CountChars(int);
static int DoAddChar(int);
static int BlankResize(int, int);
static void disp_readev_fn(Event *, void *);
static void disp_writeev_fn(Event *, void *);
static void disp_writeev_eagain(Event *, void *);
static void disp_status_fn(Event *, void *);
static void disp_hstatus_fn(Event *, void *);
static void disp_blocked_fn(Event *, void *);
static void disp_map_fn(Event *, void *);
static void disp_idle_fn(Event *, void *);
static void disp_blanker_fn(Event *, void *);
static void disp_mousetimeout_fn(Event *, void *);
static void disp_processinput (Display *, unsigned char *, size_t);
static void WriteLP(int, int);
static void INSERTCHAR(uint32_t);
static void RAW_PUTCHAR(uint32_t);
static void SetBackColor(int);
static void RemoveStatusMinWait(void);

Display *display, *displays;

/*
 *  The default values
 */
bool defautonuke = false;

int defobuflimit = OBUF_MAX;
int defnonblock = -1;
int defmousetrack = 0;
int defbracketed = 0;
int defcursorstyle = 0;
int captionalways = 0;
int captiontop = 0;
int hardstatusemu = HSTATUS_IGNORE;

struct statusposstr statuspos = { STATUS_BOTTOM, STATUS_LEFT };

int focusminwidth, focusminheight;

/*
 *  Default layer management
 */

void DefProcess(char **bufp, size_t *lenp)
{
	*bufp += *lenp;
	*lenp = 0;
}

void DefRedisplayLine(int y, int xs, int xe, int isblank)
{
	if (isblank == 0 && y >= 0)
		DefClearLine(y, xs, xe, 0);
}

void DefClearLine(int y, int xs, int xe, int bce)
{
	LClearLine(flayer, y, xs, xe, bce, NULL);
}

int DefResize(int width, int height)
{
	(void)width; /* unused */
	(void)height; /* unused */
	return -1;
}

void DefRestore(void)
{
	LAY_DISPLAYS(flayer, InsertMode(false));
	/* ChangeScrollRegion(0, D_height - 1); */
	LKeypadMode(flayer, 0);
	LCursorkeysMode(flayer, 0);
	LCursorVisibility(flayer, 0);
	LMouseMode(flayer, 0);
	LExtMouseMode(flayer, 0);
	LBracketedPasteMode(flayer, false);
	LCursorStyle(flayer, 0);
	LSetRendition(flayer, &mchar_null);
	LSetFlow(flayer, nwin_default.flowflag & FLOW_ON);
}

/*
 *  Blank layer management
 */

const struct LayFuncs BlankLf = {
	DefProcess,
	NULL,
	DefRedisplayLine,
	DefClearLine,
	BlankResize,
	DefRestore,
	NULL
};

static int BlankResize(int wi, int he)
{
	flayer->l_width = wi;
	flayer->l_height = he;
	return 0;
}

/*
 *  Generate new display, start with a blank layer.
 *  The termcap arrays are not initialised here.
 *  The new display is placed in the displays list.
 */

Display *MakeDisplay(char *uname, char *utty, char *term, int fd, pid_t pid, struct mode *Mode)
{
	struct acluser **u;

	if (!*(u = FindUserPtr(uname)) && UserAdd(uname, u))
		return NULL;	/* could not find or add user */

	if ((display = calloc(1, sizeof(Display))) == NULL)
		return NULL;
	display->d_next = displays;
	displays = display;
	D_flow = 1;
	D_nonblock = defnonblock;
	D_userfd = fd;
	D_readev.fd = D_writeev.fd = fd;
	D_readev.type = EV_READ;
	D_writeev.type = EV_WRITE;
	D_readev.data = D_writeev.data = (char *)display;
	D_readev.handler = disp_readev_fn;
	D_writeev.handler = disp_writeev_fn;
	evenq(&D_readev);
	D_writeev.condpos = &D_obuflen;
	D_writeev.condneg = &D_obuffree;
	evenq(&D_writeev);
	D_statusev.type = EV_TIMEOUT;
	D_statusev.data = (char *)display;
	D_statusev.handler = disp_status_fn;
	D_hstatusev.type = EV_TIMEOUT;
	D_hstatusev.data = (char *)display;
	D_hstatusev.handler = disp_hstatus_fn;
	D_blockedev.type = EV_TIMEOUT;
	D_blockedev.data = (char *)display;
	D_blockedev.handler = disp_blocked_fn;
	D_blockedev.condpos = &D_obuffree;
	D_blockedev.condneg = &D_obuflenmax;
	D_mapev.type = EV_TIMEOUT;
	D_mapev.data = (char *)display;
	D_mapev.handler = disp_map_fn;
	D_idleev.type = EV_TIMEOUT;
	D_idleev.data = (char *)display;
	D_idleev.handler = disp_idle_fn;
	D_blankerev.type = EV_READ;
	D_blankerev.data = (char *)display;
	D_blankerev.handler = disp_blanker_fn;
	D_blankerev.fd = -1;
	D_mousetimeoutev.type = EV_TIMEOUT;
	D_mousetimeoutev.data = (char *)display;
	D_mousetimeoutev.handler = disp_mousetimeout_fn;
	D_OldMode = *Mode;
	D_status_obuffree = -1;
	Resize_obuf();		/* Allocate memory for buffer */
	D_obufmax = defobuflimit;
	D_obuflenmax = D_obuflen - D_obufmax;
	D_auto_nuke = defautonuke;
	D_obufp = D_obuf;
	D_printfd = -1;
	D_userpid = pid;
	strncpy(D_usertty, utty, ARRAY_SIZE(D_usertty) - 1);
	D_usertty[ARRAY_SIZE(D_usertty) - 1] = 0;
	strncpy(D_termname, term, MAXTERMLEN);
	D_termname[MAXTERMLEN] = 0;
	D_user = *u;
	D_processinput = ProcessInput;
	D_mousetrack = defmousetrack;
	D_bracketed = defbracketed;
	D_cursorstyle = defcursorstyle;
	return display;
}

void FreeDisplay(void)
{
	Display *d, **dp;

	FreeTransTable();
	KillBlanker();
	if (D_userfd >= 0) {
		Flush(3);
		if (!display)
			return;
		SetTTY(D_userfd, &D_OldMode);
		fcntl(D_userfd, F_SETFL, 0);
	}
	freetty();
	if (D_tentry)
		free(D_tentry);
	D_tentry = NULL;
	if (D_processinputdata)
		free(D_processinputdata);
	D_processinputdata = NULL;
	D_tcinited = 0;
	evdeq(&D_hstatusev);
	evdeq(&D_statusev);
	evdeq(&D_readev);
	evdeq(&D_writeev);
	evdeq(&D_blockedev);
	evdeq(&D_mapev);
	if (D_kmaps) {
		free(D_kmaps);
		D_kmaps = NULL;
		D_aseqs = 0;
		D_nseqs = 0;
		D_seqp = NULL;
		D_seql = 0;
		D_seqh = NULL;
	}
	evdeq(&D_idleev);
	evdeq(&D_blankerev);

	for (dp = &displays; (d = *dp); dp = &d->d_next)
		if (d == display)
			break;
	if (D_status_lastmsg)
		free(D_status_lastmsg);
	if (D_obuf)
		free(D_obuf);
	*dp = display->d_next;

	while (D_canvas.c_slperp)
		FreeCanvas(D_canvas.c_slperp);
	D_cvlist = NULL;

	for (Window *p = mru_window; p; p = p->w_prev_mru) {
		if (p->w_pdisplay == display)
			p->w_pdisplay = NULL;
		if (p->w_lastdisp == display)
			p->w_lastdisp = NULL;
		if (p->w_readev.condneg == (int *)&D_status || p->w_readev.condneg == &D_obuflenmax)
			p->w_readev.condpos = p->w_readev.condneg = NULL;
	}
	for (Window *p = mru_window; p; p = p->w_prev_mru)
		if (p->w_zdisplay == display)
			zmodem_abort(p, NULL);
	if (D_mousetrack) {
		D_mousetrack = 0;
		MouseMode(0);
		ExtMouseMode(0);
	}
	free((char *)display);
	display = NULL;
}

/*
 * if the adaptflag is on, we keep the size of this display, else
 * we may try to restore our old window sizes.
 */
void InitTerm(int adapt)
{
	D_top = D_bot = -1;
	AddCStr(D_IS);
	AddCStr(D_TI);
	/* Check for toggle */
	if (D_IM && strcmp(D_IM, D_EI))
		AddCStr(D_EI);
	D_insert = 0;
	AddCStr(D_KS);
	AddCStr(D_CCS);
	D_keypad = 0;
	D_cursorkeys = 0;
	AddCStr(D_ME);
	AddCStr(D_EA);
	AddCStr(D_CE0);
	D_rend = mchar_null;
	D_atyp = 0;
	if (adapt == 0)
		ResizeDisplay(D_defwidth, D_defheight);
	ChangeScrollRegion(0, D_height - 1);
	D_x = D_y = 0;
	Flush(3);
	ClearAll();
	/* In case the size was changed by a init sequence */
	CheckScreenSize((adapt) ? 2 : 0);
}

void FinitTerm(void)
{
	KillBlanker();
	if (D_tcinited) {
		ResizeDisplay(D_defwidth, D_defheight);
		InsertMode(false);
		ChangeScrollRegion(0, D_height - 1);
		KeypadMode(0);
		CursorkeysMode(0);
		CursorVisibility(0);
		if (D_mousetrack)
			D_mousetrack = 0;
		MouseMode(0);
		ExtMouseMode(0);
		BracketedPasteMode(false);
		CursorStyle(0);
		SetRendition(&mchar_null);
		SetFlow(FLOW_ON);
		AddCStr(D_KE);
		AddCStr(D_CCE);
		if (D_hstatus)
			ShowHStatus(NULL);
		ClearAllXtermOSC();
		D_x = D_y = -1;
		GotoPos(0, D_height - 1);
		AddChar('\r');
		AddChar('\n');
		AddCStr(D_TE);
	}
	Flush(3);
}

static void INSERTCHAR(uint32_t c)
{
	if (!D_insert && D_x < D_width - 1) {
		if (D_IC || D_CIC) {
			if (D_IC)
				AddCStr(D_IC);
			else
				AddCStr2(D_CIC, 1);
			RAW_PUTCHAR(c);
			return;
		}
		InsertMode(true);
		if (!D_insert) {
			RefreshLine(D_y, D_x, D_width - 1, 0);
			return;
		}
	}
	RAW_PUTCHAR(c);
}

void PUTCHAR(uint32_t c)
{
	if (D_insert && D_x < D_width - 1)
		InsertMode(false);
	RAW_PUTCHAR(c);
}

void PUTCHARLP(uint32_t c)
{
	if (D_x < D_width - 1) {
		if (D_insert)
			InsertMode(false);
		RAW_PUTCHAR(c);
		return;
	}
	if (D_CLP || D_y != D_bot) {
		int y = D_y;
		RAW_PUTCHAR(c);
		if (D_AM && !D_CLP)
			GotoPos(D_width - 1, y);
		return;
	}
	D_lp_missing = 1;
	D_rend.image = c;
	D_lpchar = D_rend;
	/* XXX -> PutChar ? */
	if (D_mbcs) {
		D_lpchar.mbcs = c;
		D_lpchar.image = D_mbcs;
		D_mbcs = 0;
		D_x--;
	}
}

/*
 * RAW_PUTCHAR() is for all text that will be displayed.
 * NOTE: charset Nr. 0 has a conversion table, but c1, c2, ... don't.
 */

static void RAW_PUTCHAR(uint32_t c)
{

	if (D_encoding == UTF8) {
		if (D_mbcs) {
			c = D_mbcs;
			if (D_x == D_width)
				D_x += D_AM ? 1 : -1;
			D_mbcs = 0;
		} else if (utf8_isdouble(c)) {
			D_mbcs = c;
			D_x++;
			return;
		}
		if (c < 32) {
			AddCStr2(D_CS0, '0');
			AddChar(c + 0x5f);
			AddCStr(D_CE0);
			goto addedutf8;
		}
		if (c < 0x80) {
			if (D_xtable && D_xtable[(int)(unsigned char)D_rend.font]
			    && D_xtable[(int)(unsigned char)D_rend.font][(int)(unsigned char)c])
				AddStr(D_xtable[(int)(unsigned char)D_rend.font][(int)(unsigned char)c]);
			else
				AddChar(c);
		} else
			AddUtf8(c);
		goto addedutf8;
	}
	if (is_dw_font(D_rend.font)) {
		int t = c;
		if (D_mbcs == 0) {
			D_mbcs = c;
			D_x++;
			return;
		}
		D_x--;
		if (D_x == D_width - 1)
			D_x += D_AM ? 1 : -1;
		c = D_mbcs;
		D_mbcs = t;
	}
	if (D_encoding)
		c = PrepareEncodedChar(c);
 kanjiloop:
	if (D_xtable && D_xtable[(int)(unsigned char)D_rend.font]
	    && D_xtable[(int)(unsigned char)D_rend.font][(int)(unsigned char)c])
		AddStr(D_xtable[(int)(unsigned char)D_rend.font][(int)(unsigned char)c]);
	else
		AddChar(D_rend.font != '0' ? c : (uint32_t)D_c0_tab[(int)(unsigned char)c]);

 addedutf8:
	if (++D_x >= D_width) {
		if (D_AM == 0)
			D_x = D_width - 1;
		else if (!D_CLP || D_x > D_width) {
			D_x -= D_width;
			if (D_y < D_height - 1 && D_y != D_bot)
				D_y++;
		}
	}
	if (D_mbcs) {
		c = D_mbcs;
		D_mbcs = 0;
		goto kanjiloop;
	}
}

static int DoAddChar(int c)
{
	/* this is for ESC-sequences only (AddChar is a macro) */
	AddChar(c);
	return c;
}

void AddCStr(char *s)
{
	if (display && s && *s) {
		tputs(s, 1, DoAddChar);
	}
}

void AddCStr2(char *s, int c)
{
	if (display && s && *s) {
		tputs(tgoto(s, 0, c), 1, DoAddChar);
	}
}

/* Insert mode is a toggle on some terminals, so we need this hack:
 */
void InsertMode(bool on)
{
	if (display && on != D_insert && D_IM) {
		D_insert = on;
		if (on)
			AddCStr(D_IM);
		else
			AddCStr(D_EI);
	}
}

/* ...and maybe keypad application mode is a toggle, too:
 */
void KeypadMode(int on)
{
	if (display)
		D_keypad = on;
}

void CursorkeysMode(int on)
{
	if (display)
		D_cursorkeys = on;
}

void ReverseVideo(bool on)
{
	if (display && D_revvid != on && D_CVR) {
		D_revvid = on;
		if (D_revvid)
			AddCStr(D_CVR);
		else
			AddCStr(D_CVN);
	}
}

void CursorVisibility(int v)
{
	if (display && D_curvis != v) {
		if (D_curvis)
			AddCStr(D_VE);	/* do this always, just to be safe */
		D_curvis = 0;
		if (v == -1 && D_VI)
			AddCStr(D_VI);
		else if (v == 1 && D_VS)
			AddCStr(D_VS);
		else
			return;
		D_curvis = v;
	}
}

void MouseMode(int mode)
{
	if (!display)
		return;

	if (mode < D_mousetrack)
		mode = D_mousetrack;

	if (D_mouse != mode) {
		char mousebuf[20];
		if (!D_CXT)
			return;
		if (D_mouse) {
			sprintf(mousebuf, "\033[?%dl", D_mouse);
			AddStr(mousebuf);
		}
		if (mode) {
			sprintf(mousebuf, "\033[?%dh", mode);
			AddStr(mousebuf);
		}
		D_mouse = mode;
		D_mouse_parse.state = CSI_INACTIVE;
	}
}

void ExtMouseMode(int mode)
{
	if (display && D_extmouse != mode) {
		char mousebuf[20];
		if (!D_CXT)
			return;
		if (D_extmouse) {
			sprintf(mousebuf, "\033[?%dl", D_extmouse);
			AddStr(mousebuf);
		}
		if (mode) {
			sprintf(mousebuf, "\033[?%dh", mode);
			AddStr(mousebuf);
		}
		D_extmouse = mode;
		D_mouse_parse.state = CSI_INACTIVE;
	}
}


void BracketedPasteMode(bool mode)
{
	if (!display)
		return;

	if (D_bracketed != mode) {
		if (!D_CXT)
			return;
		if (D_bracketed) {
			AddStr("\033[?2004l");
		}
		if (mode) {
			AddStr("\033[?2004h");
		}
		D_bracketed = mode;
	}
}

void CursorStyle(int mode)
{
	char buf[32];

	if (!display)
		return;

	if (D_cursorstyle != mode) {
		if (!D_CXT)
			return;
		if (mode < 0)
			return;
		sprintf(buf, "\033[%d q", mode);
		AddStr(buf);
		D_cursorstyle = mode;
	}
}

static int StrCost;

static int CountChars(int c)
{
	StrCost++;
	return c;
}

int CalcCost(char *s)
{
	if (s) {
		StrCost = 0;
		tputs(s, 1, CountChars);
		return StrCost;
	} else
		return EXPENSIVE;
}

void GotoPos(int x2, int y2)
{
	int dy, dx, x1, y1;
	int costx, costy;
	int m;
	char *s;
	int CMcost;
	enum move_t xm = M_NONE, ym = M_NONE;

	if (!display)
		return;

	x1 = D_x;
	y1 = D_y;

	if (x1 == D_width) {
		if (D_CLP && D_AM)
			x1 = -1;	/* don't know how the terminal treats this */
		else
			x1--;
	}
	if (x2 == D_width)
		x2--;
	dx = x2 - x1;
	dy = y2 - y1;
	if (dy == 0 && dx == 0)
		return;
	if (!D_MS)		/* Safe to move ? */
		SetRendition(&mchar_null);
	if (y1 < 0		/* don't know the y position */
	    || (y2 > D_bot && y1 <= D_bot)	/* have to cross border */
	    ||(y2 < D_top && y1 >= D_top)) {	/* of scrollregion ?    */
 DoCM:
		if (D_HO && !x2 && !y2)
			AddCStr(D_HO);
		else
			AddCStr(tgoto(D_CM, x2, y2));
		D_x = x2;
		D_y = y2;
		return;
	}

	/* some scrollregion implementations don't allow movements
	 * away from the region. sigh.
	 */
	if ((y1 > D_bot && y2 > y1) || (y1 < D_top && y2 < y1))
		goto DoCM;

	/* Calculate CMcost */
	if (D_HO && !x2 && !y2)
		s = D_HO;
	else
		s = tgoto(D_CM, x2, y2);
	CMcost = CalcCost(s);

	/* Calculate the cost to move the cursor to the right x position */
	costx = EXPENSIVE;
	if (x1 >= 0) {		/* relativ x positioning only if we know where we are */
		if (dx > 0) {
			if (D_CRI && (dx > 1 || !D_ND)) {
				costx = CalcCost(tgoto(D_CRI, 0, dx));
				xm = M_CRI;
			}
			if ((m = D_NDcost * dx) < costx) {
				costx = m;
				xm = M_RI;
			}
		} else if (dx < 0) {
			if (D_CLE && (dx < -1 || !D_BC)) {
				costx = CalcCost(tgoto(D_CLE, 0, -dx));
				xm = M_CLE;
			}
			if ((m = -dx * D_LEcost) < costx) {
				costx = m;
				xm = M_LE;
			}
		} else
			costx = 0;
	}
	if (x2 + D_CRcost < costx && (m = (D_CRcost < costx))) {
		costx = EXPENSIVE;
		xm = M_CR;
	}

	/* Check if it is already cheaper to do CM */
	if (costx >= CMcost)
		goto DoCM;

	/* Calculate the cost to move the cursor to the right y position */
	costy = EXPENSIVE;
	if (dy > 0) {
		if (D_CDO && dy > 1) {	/* DO & NL are always != 0 */
			costy = CalcCost(tgoto(D_CDO, 0, dy));
			ym = M_CDO;
		}
		if ((m = dy * ((x2 == 0) ? D_NLcost : D_DOcost)) < costy) {
			costy = m;
			ym = M_DO;
		}
	} else if (dy < 0) {
		if (D_CUP && (dy < -1 || !D_UP)) {
			costy = CalcCost(tgoto(D_CUP, 0, -dy));
			ym = M_CUP;
		}
		if ((m = -dy * D_UPcost) < costy) {
			costy = m;
			ym = M_UP;
		}
	} else
		costy = 0;

	/* Finally check if it is cheaper to do CM */
	if (costx + costy >= CMcost)
		goto DoCM;

	switch (xm) {
	case M_LE:
		while (dx++ < 0)
			AddCStr(D_BC);
		break;
	case M_CLE:
		AddCStr2(D_CLE, -dx);
		break;
	case M_RI:
		while (dx-- > 0)
			AddCStr(D_ND);
		break;
	case M_CRI:
		AddCStr2(D_CRI, dx);
		break;
	case M_CR:
		AddCStr(D_CR);
		D_x = 0;
		/* FALLTHROUGH */
	case M_RW:
		break;
	default:
		break;
	}

	switch (ym) {
	case M_UP:
		while (dy++ < 0)
			AddCStr(D_UP);
		break;
	case M_CUP:
		AddCStr2(D_CUP, -dy);
		break;
	case M_DO:
		s = (x2 == 0) ? D_NL : D_DO;
		while (dy-- > 0)
			AddCStr(s);
		break;
	case M_CDO:
		AddCStr2(D_CDO, dy);
		break;
	default:
		break;
	}
	D_x = x2;
	D_y = y2;
}

void ClearAll(void)
{
	ClearArea(0, 0, 0, D_width - 1, D_width - 1, D_height - 1, 0, 0);
}

void ClearArea(int x1, int y1, int xs, int xe, int x2, int y2, int bce, int uselayfn)
{
	int xxe;
	Canvas *cv;
	Viewport *vp;

	if (x1 == D_width)
		x1--;
	if (x2 == D_width)
		x2--;
	if (xs == -1)
		xs = x1;
	if (xe == -1)
		xe = x2;
	if (D_UT)		/* Safe to erase ? */
		SetRendition(&mchar_null);
	if (D_BE)
		SetBackColor(bce);
	if (D_lp_missing && y1 <= D_bot && xe >= D_width - 1) {
		if (y2 > D_bot || (y2 == D_bot && x2 >= D_width - 1))
			D_lp_missing = 0;
	}
	if (x2 == D_width - 1 && (xs == 0 || y1 == y2) && xe == D_width - 1 && y2 == D_height - 1 && (!bce || D_BE)) {
		if (x1 == 0 && y1 == 0 && D_auto_nuke)
			NukePending();
		if (x1 == 0 && y1 == 0 && D_CL) {
			AddCStr(D_CL);
			D_y = D_x = 0;
			return;
		}
		/*
		 * Workaround a hp700/22 terminal bug. Do not use CD where CE
		 * is also appropriate.
		 */
		if (D_CD && (y1 < y2 || !D_CE)) {
			GotoPos(x1, y1);
			AddCStr(D_CD);
			return;
		}
	}
	if (x1 == 0 && xs == 0 && (xe == D_width - 1 || y1 == y2) && y1 == 0 && D_CCD && (!bce || D_BE)) {
		GotoPos(x1, y1);
		AddCStr(D_CCD);
		return;
	}
	xxe = xe;
	for (int y = y1; y <= y2; y++, x1 = xs) {
		if (y == y2)
			xxe = x2;
		if (x1 == 0 && D_CB && (xxe != D_width - 1 || (D_x == xxe && D_y == y)) && (!bce || D_BE)) {
			GotoPos(xxe, y);
			AddCStr(D_CB);
			continue;
		}
		if (xxe == D_width - 1 && D_CE && (!bce || D_BE)) {
			GotoPos(x1, y);
			AddCStr(D_CE);
			continue;
		}
		if (uselayfn) {
			vp = NULL;
			for (cv = D_cvlist; cv; cv = cv->c_next) {
				if (y < cv->c_ys || y > cv->c_ye || xxe < cv->c_xs || x1 > cv->c_xe)
					continue;
				for (vp = cv->c_vplist; vp; vp = vp->v_next)
					if (y >= vp->v_ys && y <= vp->v_ye && xxe >= vp->v_xs && x1 <= vp->v_xe)
						break;
				if (vp)
					break;
			}
			if (cv && cv->c_layer && x1 >= vp->v_xs && xxe <= vp->v_xe &&
			    y - vp->v_yoff >= 0 && y - vp->v_yoff < cv->c_layer->l_height &&
			    xxe - vp->v_xoff >= 0 && x1 - vp->v_xoff < cv->c_layer->l_width) {
				Layer *oldflayer = flayer;
				Canvas *cvlist, *cvlnext;
				flayer = cv->c_layer;
				cvlist = flayer->l_cvlist;
				cvlnext = cv->c_lnext;
				flayer->l_cvlist = cv;
				cv->c_lnext = NULL;
				LayClearLine(y - vp->v_yoff, x1 - vp->v_xoff, xxe - vp->v_xoff, bce);
				flayer->l_cvlist = cvlist;
				cv->c_lnext = cvlnext;
				flayer = oldflayer;
				continue;
			}
		}
		ClearLine(NULL, y, x1, xxe, bce);
	}
}

/*
 * if cur_only > 0, we only redisplay current line, as a full refresh is
 * too expensive over a low baud line.
 */
void Redisplay(int cur_only)
{
	/* XXX do em all? */
	InsertMode(false);
	ChangeScrollRegion(0, D_height - 1);
	KeypadMode(0);
	CursorkeysMode(0);
	CursorVisibility(0);
	MouseMode(0);
	ExtMouseMode(0);
	BracketedPasteMode(false);
	CursorStyle(0);
	SetRendition(&mchar_null);
	SetFlow(FLOW_ON);

	ClearAll();
	RefreshXtermOSC();
	if (cur_only > 0 && D_fore)
		RefreshArea(0, D_fore->w_y, D_width - 1, D_fore->w_y, 1);
	else
		RefreshAll(1);
	RefreshHStatus();
	CV_CALL(D_forecv, LayRestore();
		LaySetCursor());
}

void RedisplayDisplays(int cur_only)
{
	Display *olddisplay = display;
	for (display = displays; display; display = display->d_next)
		Redisplay(cur_only);
	display = olddisplay;
}

/* XXX: use oml! */
void ScrollH(int y, int xs, int xe, int n, int bce, struct mline *oml)
{
	(void)oml; /* unused */

	if (n == 0)
		return;
	if (xe != D_width - 1) {
		RefreshLine(y, xs, xe, 0);
		/* UpdateLine(oml, y, xs, xe); */
		return;
	}
	GotoPos(xs, y);
	if (D_UT)
		SetRendition(&mchar_null);
	if (D_BE)
		SetBackColor(bce);
	if (n > 0) {
		if (n >= xe - xs + 1)
			n = xe - xs + 1;
		if (D_CDC && !(n == 1 && D_DC))
			AddCStr2(D_CDC, n);
		else if (D_DC) {
			for (int i = n; i--;)
				AddCStr(D_DC);
		} else {
			RefreshLine(y, xs, xe, 0);
			/* UpdateLine(oml, y, xs, xe); */
			return;
		}
	} else {
		if (-n >= xe - xs + 1)
			n = -(xe - xs + 1);
		if (!D_insert) {
			if (D_CIC && !(n == -1 && D_IC))
				AddCStr2(D_CIC, -n);
			else if (D_IC) {
				for (int i = -n; i--;)
					AddCStr(D_IC);
			} else if (D_IM) {
				InsertMode(true);
				SetRendition(&mchar_null);
				SetBackColor(bce);
				for (int i = -n; i--;)
					INSERTCHAR(' ');
				bce = 0;	/* all done */
			} else {
				/* UpdateLine(oml, y, xs, xe); */
				RefreshLine(y, xs, xe, 0);
				return;
			}
		} else {
			SetRendition(&mchar_null);
			SetBackColor(bce);
			for (int i = -n; i--;)
				INSERTCHAR(' ');
			bce = 0;	/* all done */
		}
	}
	if (bce && !D_BE) {
		if (n > 0)
			ClearLine(NULL, y, xe - n + 1, xe, bce);
		else
			ClearLine(NULL, y, xs, xs - n - 1, bce);
	}
	if (D_lp_missing && y == D_bot) {
		if (n > 0)
			WriteLP(D_width - 1 - n, y);
		D_lp_missing = 0;
	}
}

void ScrollV(int xs, int ys, int xe, int ye, int n, int bce)
{
	int up;
	int /*oldtop,*/ oldbot;
	int alok, dlok, aldlfaster;
	int missy = 0;

	if (n == 0)
		return;
	if (n >= ye - ys + 1 || -n >= ye - ys + 1) {
		ClearArea(xs, ys, xs, xe, xe, ye, bce, 0);
		return;
	}
	if (xs > D_vpxmin || xe < D_vpxmax) {
		RefreshArea(xs, ys, xe, ye, 0);
		return;
	}

	if (D_lp_missing) {
		if (D_bot > ye || D_bot < ys)
			missy = D_bot;
		else {
			missy = D_bot - n;
			if (missy > ye || missy < ys)
				D_lp_missing = 0;
		}
	}

	up = 1;
	if (n < 0) {
		up = 0;
		n = -n;
	}
	if (n >= ye - ys + 1)
		n = ye - ys + 1;

	/* oldtop = D_top; */
	oldbot = D_bot;
	if (ys < D_top || D_bot != ye)
		ChangeScrollRegion(ys, ye);
	alok = (D_AL || D_CAL || (ys >= D_top && ye == D_bot && up));
	dlok = (D_DL || D_CDL || (ys >= D_top && ye == D_bot && !up));
	if (D_top != ys && !(alok && dlok))
		ChangeScrollRegion(ys, ye);

	if (D_lp_missing && (oldbot != D_bot || (oldbot == D_bot && up && D_top == ys && D_bot == ye))) {
		WriteLP(D_width - 1, oldbot);
		if (oldbot == D_bot) {	/* have scrolled */
			if (--n == 0) {
/* XXX
	      ChangeScrollRegion(oldtop, oldbot);
*/
				if (bce && !D_BE)
					ClearLine(NULL, ye, xs, xe, bce);
				return;
			}
		}
	}

	if (D_UT)
		SetRendition(&mchar_null);
	if (D_BE)
		SetBackColor(bce);

	aldlfaster = (n > 1 && ys >= D_top && ye == D_bot && ((up && D_CDL) || (!up && D_CAL)));

	if ((up || D_SR) && D_top == ys && D_bot == ye && !aldlfaster) {
		if (up) {
			GotoPos(0, ye);
			for (int i = n; i-- > 0;)
				AddCStr(D_NL);	/* was SF, I think NL is faster */
		} else {
			GotoPos(0, ys);
			for (int i = n; i-- > 0;)
				AddCStr(D_SR);
		}
	} else if (alok && dlok) {
		if (up || ye != D_bot) {
			GotoPos(0, up ? ys : ye + 1 - n);
			if (D_CDL && !(n == 1 && D_DL))
				AddCStr2(D_CDL, n);
			else
				for (int i = n; i--;)
					AddCStr(D_DL);
		}
		if (!up || ye != D_bot) {
			GotoPos(0, up ? ye + 1 - n : ys);
			if (D_CAL && !(n == 1 && D_AL))
				AddCStr2(D_CAL, n);
			else
				for (int i = n; i--;)
					AddCStr(D_AL);
		}
	} else {
		RefreshArea(xs, ys, xe, ye, 0);
		return;
	}
	if (bce && !D_BE) {
		if (up)
			ClearArea(xs, ye - n + 1, xs, xe, xe, ye, bce, 0);
		else
			ClearArea(xs, ys, xs, xe, xe, ys + n - 1, bce, 0);
	}
	if (D_lp_missing && missy != D_bot)
		WriteLP(D_width - 1, missy);
/* XXX
  ChangeScrollRegion(oldtop, oldbot);
  if (D_lp_missing && missy != D_bot)
    WriteLP(D_width - 1, missy);
*/
}

void SetAttr(int new)
{
	int i, j, old, typ;
	old = D_rend.attr;

	if (!display)
		return;
	if (old == new)
		return;
	D_rend.attr = new;
	typ = D_atyp;
	if ((new & old) != old) {
		if ((typ & ATYP_U))
			AddCStr(D_UE);
		if ((typ & ATYP_S))
			AddCStr(D_SE);
		if ((typ & ATYP_M)) {
			AddCStr(D_ME);
			/* ansi attrib handling: \E[m resets color, too */
			if (D_hascolor)
				D_rend.colorbg = D_rend.colorfg = 0;
			if (!D_CG0) {
				/* D_ME may also reset the alternate charset */
				D_rend.font = 0;
				D_realfont = 0;
			}
		}
		old = 0;
		typ = 0;
	}
	old ^= new;
	for (i = 0, j = 1; old && i < NATTR; i++, j <<= 1) {
		if ((old & j) == 0)
			continue;
		old ^= j;
		if (D_attrtab[i]) {
			AddCStr(D_attrtab[i]);
			typ |= D_attrtyp[i];
		}
	}
	D_atyp = typ;
}

void SetFont(int new)
{
	int old = D_rend.font;
	if (!display || old == new)
		return;
	D_rend.font = new;
	if (D_encoding && CanEncodeFont(D_encoding, new))
		return;
	if (new == D_realfont)
		return;
	D_realfont = new;
	if (D_xtable && D_xtable[(int)(unsigned char)new] && D_xtable[(int)(unsigned char)new][256]) {
		AddCStr(D_xtable[(int)(unsigned char)new][256]);
		return;
	}

	if (!D_CG0 && new != '0') {
		new = ASCII;
		if (old == new)
			return;
	}

	if (new == ASCII)
		AddCStr(D_CE0);
	else if (new < ' ') {
		AddStr("\033$");
		if (new > 2)
			AddChar('(');
		AddChar(new + '@');
	} else
		AddCStr2(D_CS0, new);
}

int color256to16(int color)
{
	int min, max;
	int r, g, b;

	if (color >= 232) {
		color = (color - 232) / 6;
		color = (color & 1) << 3 | (color & 2 ? 7 : 0);
	} else if (color >= 16) {
		color -= 16;
		r = color / 36;
		g = (color / 6) % 6;
		b = color % 6;
		min = r < g ? (r < b ? r : b) : (g < b ? g : b);
		max = r > g ? (r > b ? r : b) : (g > b ? g : b);
		if (min == max)
			color = ((max + 1) & 2) << 2 | ((max + 1) & 4 ? 7 : 0);
		else
			color = (b - min) / (max - min) << 2 |
				(g - min) / (max - min) << 1 |
				(r - min) / (max - min) |
				(max > 3 ? 8: 0);
	}
	return color;
}

int color256to88(int color)
{
	int r, g, b;

	if (color >= 232)
		return (color - 232) / 3 + 80;
	if (color >= 16) {
		color -= 16;
		r = color / 36;
		g = (color / 6) % 6;
		b = color % 6;
		return ((r + 1) / 2) * 16 + ((g + 1) / 2) * 4 + ((b + 1) / 2) + 16;
	}
	return color;
}

/*
 * SetColor - Sets foreground and background color
 * 0x00000000 <- default color ("transparent")
 * 	one note here that "null" variable is pointer to array of 0 and that's one of reasons to use it this way 
 * 0x0100000x <- 16 base color
 * 0x020000xx <- 256 color
 * 0x04xxxxxx <- truecolor
 */
void SetColor(uint32_t foreground, uint32_t background)
{
	uint32_t f, b, of, ob;
	static unsigned char sftrans[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

	if (!display)
		return;

	f = foreground;
	b = background;
	of = D_rend.colorfg;
	ob = D_rend.colorbg;
	D_rend.colorfg = f;
	D_rend.colorbg = b;

	if (!D_CAX && D_hascolor && ((f == 0 && f != of) || (b == 0 && b != ob))) {
		if (D_OP)
			AddCStr(D_OP);
		else {
			int oattr;
			oattr = D_rend.attr;
			AddCStr(D_ME ? D_ME : "\033[m");
			if (D_ME && !D_CG0) {
				/* D_ME may also reset the alternate charset */
				D_rend.font = 0;
				D_realfont = 0;
			}
			D_atyp = 0;
			D_rend.attr = 0;
			SetAttr(oattr);
		}
		of = ob = 0;
	}
	if (!D_hascolor)
		return;

/* FOREGROUND */
	if (f != of && f == 0) {
		AddCStr("\033[39m");	/* works because AX is set */
	}
	if (f != of && (f & 0x01000000)) {
		f &= 0x0f;
		if (f < 8) {
			if (D_CAF)
				AddCStr2(D_CAF, f);
			else if (D_CSF)
				AddCStr2(D_CSF, sftrans[f]);
		}
		if (D_CXT && f >= 8 && f <= 15) {
			AddCStr2("\033[9%p1%dm", f & 7);
		}
	}
	if (f != of && (f & 0x02000000)) {
		f &= 0x0ff;
		if (f > 15 && D_CCO != 256) {
			f = D_CCO == 88 && D_CAF ? color256to88(f) : color256to16(f);
		}
		if (D_CAF) {
			//AddCStr2(D_CAF, f); //FIXME
			tputs(tparm("\033[38;5;%dm", f), 1, DoAddChar);
		}
		
	}
	if (f != of && (f & 0x04000000) && hastruecolor) {
		uint8_t _r, _g, _b;

		_r = (f & 0x00ff0000) >> 16;
		_g = (f & 0x0000ff00) >> 8;
		_b = (f & 0x000000ff);

		/* TODO - properly get escape code */
		tputs(tparm("\033[38;2;%p1%d;%p2%d;%p3%dm", _r, _g, _b), 1, DoAddChar);
	}

/* BACKGROUND */
	if (b != ob && b == 0) {
		AddCStr("\033[49m");	/* works because AX is set */
	}
	if (b != ob && (b & 0x01000000)) {
		b &= 0x0f;
		if (b < 8) {
			if (D_CAB)
				AddCStr2(D_CAB, b);
			else if (D_CSB)
				AddCStr2(D_CSB, sftrans[b]);
		}
		if (D_CXT && b >= 8 && b <= 15) {
			AddCStr2("\033[10%p1%dm", b & 7);
		}
	}
	if (b != ob && (b & 0x02000000)) {
		b &= 0x0ff;
		if (b > 15 && D_CCO != 256) {
			b = D_CCO == 88 && D_CAB ? color256to88(b) : color256to16(b);
		}
		if (D_CAB) {
		//	AddCStr2(D_CAB, b); // FIXME
			tputs(tparm("\033[48;5;%dm", b), 1, DoAddChar);
		}
	}
	if (b != ob && (b & 0x04000000) && hastruecolor) {
		uint8_t _r, _g, _b;

		_r = (b & 0x00ff0000) >> 16;
		_g = (b & 0x0000ff00) >> 8;
		_b = (b & 0x000000ff);

		/* TODO - properly get escape code */
		tputs(tparm("\033[48;2;%p1%d;%p2%d;%p3%dm", _r, _g, _b), 1, DoAddChar);
	}
}

static void SetBackColor(int new)
{
	if (!display)
		return;
	SetColor(D_rend.colorfg, new);
}

void SetRendition(struct mchar *mc)
{
	if (!display)
		return;
	if (D_rend.attr != mc->attr)
		SetAttr(mc->attr);
	if (D_rend.colorbg != mc->colorbg || D_rend.colorfg != mc->colorfg)
		SetColor(mc->colorfg, mc->colorbg);
	if (D_rend.font != mc->font)
		SetFont(mc->font);
}

void SetRenditionMline(struct mline *ml, int x)
{
	if (!display)
		return;
	if (D_rend.attr != ml->attr[x])
		SetAttr(ml->attr[x]);
	if (D_rend.colorbg != ml->colorbg[x]
	    || D_rend.colorfg != ml->colorfg[x]) {
		struct mchar mc;
		copy_mline2mchar(&mc, ml, x);
		SetColor(mc.colorfg, mc.colorbg);
	}
	if (D_rend.font != ml->font[x])
		SetFont(ml->font[x]);
}

void MakeStatus(char *msg)
{
	char *s, *t;
	int max;

	if (!display)
		return;

	if (D_blocked)
		return;
	if (!D_tcinited) {
		if (D_processinputdata)
			return;	/* XXX: better */
		AddStr(msg);
		AddStr("\r\n");
		Flush(0);
		return;
	}
	if (!use_hardstatus || !D_HS) {
		max = D_width;
		if (D_CLP == 0)
			max--;
	} else
		max = D_WS > 0 ? D_WS : (D_width - !D_CLP);
	if (D_status) {
		/* same message? */
		if (strcmp(msg, D_status_lastmsg) == 0) {
			if (!D_status_obufpos)
				SetTimeout(&D_statusev, MsgWait);
			return;
		}
		RemoveStatusMinWait();
	}
	for (s = t = msg; *s && t - msg < max; ++s)
		if (*s == BELL)
			AddCStr(D_BL);
		else if ((unsigned char)*s >= ' ' && *s != 0177)
			*t++ = *s;
	*t = '\0';
	if (t == msg)
		return;
	if (t - msg >= D_status_buflen) {
		char *buf;
		if (D_status_lastmsg)
			buf = realloc(D_status_lastmsg, t - msg + 1);
		else
			buf = malloc(t - msg + 1);
		if (buf) {
			D_status_lastmsg = buf;
			D_status_buflen = t - msg + 1;
		}
	}
	if (t - msg < D_status_buflen)
		strncpy(D_status_lastmsg, msg, D_status_buflen);
	D_status_len = t - msg;
	D_status_lastx = D_x;
	D_status_lasty = D_y;
	if (!use_hardstatus || D_has_hstatus == HSTATUS_IGNORE || D_has_hstatus == HSTATUS_MESSAGE) {
		D_status = STATUS_ON_WIN;
		GotoPos(STATCOL(D_width, D_status_len), STATLINE());
		SetRendition(&mchar_so);
		InsertMode(false);
		AddStr(msg);
		if (D_status_len < max) {
			/* Wayne Davison: add extra space for readability */
			D_status_len++;
			SetRendition(&mchar_null);
			AddChar(' ');
			if (D_status_len < max) {
				D_status_len++;
				AddChar(' ');
				AddChar('\b');
			}
			AddChar('\b');
		}
		D_x = -1;
	} else {
		D_status = STATUS_ON_HS;
		ShowHStatus(msg);
	}

	D_status_obufpos = D_obufp - D_obuf;

	if (D_status == STATUS_ON_WIN) {
		Display *olddisplay = display;
		Layer *oldflayer = flayer;

		/* this is copied over from RemoveStatus() */
		D_status = 0;
		GotoPos(STATCOL(D_width, D_status_len), STATLINE());
		RefreshLine(STATLINE(), STATCOL(D_width, D_status_len), STATCOL(D_width, D_status_len) + D_status_len - 1, 0);
		GotoPos(D_status_lastx, D_status_lasty);
		flayer = D_forecv ? D_forecv->c_layer : NULL;
		if (flayer)
			LaySetCursor();
		display = olddisplay;
		flayer = oldflayer;
		D_status = STATUS_ON_WIN;
	}
}

void RemoveStatus(void)
{
	Display *olddisplay;
	Layer *oldflayer;
	int where;

	if (!display)
		return;
	if (!(where = D_status))
		return;

	if (D_status_obuffree >= 0) {
		D_obuflen = D_status_obuflen;
		D_obuffree = D_status_obuffree;
		D_status_obuffree = -1;
	}
	D_status = 0;
	D_status_obufpos = 0;
	D_status_bell = 0;
	evdeq(&D_statusev);
	olddisplay = display;
	oldflayer = flayer;
	if (where == STATUS_ON_WIN) {
		if (captionalways || (D_canvas.c_slperp && D_canvas.c_slperp->c_slnext)) {
			GotoPos(STATCOL(D_width, D_status_len), STATLINE());
			RefreshLine(STATLINE(), STATCOL(D_width, D_status_len), STATCOL(D_width, D_status_len) + D_status_len - 1, 0);
			GotoPos(D_status_lastx, D_status_lasty);
		}
	} else
		RefreshHStatus();
	flayer = D_forecv ? D_forecv->c_layer : NULL;
	if (flayer)
		LaySetCursor();
	display = olddisplay;
	flayer = oldflayer;
}

/* Remove the status but make sure that it is seen for MsgMinWait ms */
static void RemoveStatusMinWait(void)
{
	/* XXX: should flush output first if D_status_obufpos is set */
	if (!D_status_bell && !D_status_obufpos) {
		struct timeval now;
		int ti;
		gettimeofday(&now, NULL);
		ti = (now.tv_sec - D_status_time.tv_sec) * 1000 + (now.tv_usec - D_status_time.tv_usec) / 1000;
		if (ti < MsgMinWait)
			DisplaySleep1000(MsgMinWait - ti, 0);
	}
	RemoveStatus();
}

static int strlen_onscreen(char *c, char *end)
{
	int len = 0;
	while (*c && (!end || c < end)) {
		int v, dec = 0;
		do {
			v = FromUtf8(*c++, &dec);
			if (v == -2)
				c--;
		}
		while (v < 0 && (!end || c < end));
		if (!utf8_iscomb(v)) {
			if (utf8_isdouble(v))
				len++;
			len++;
		}
	}

	return len;
}

static int PrePutWinMsg(char *s, int start, int max)
{
	/* Avoid double-encoding problem for a UTF-8 message on a UTF-8 locale.
	   Ideally, this would not be necessary. But fixing it the Right Way will
	   probably take way more time. So this will have to do for now. */
	if (D_encoding == UTF8) {
		int chars = strlen_onscreen((s + start), (s + max));
		D_encoding = 0;
		PutWinMsg(s, start, max + ((max - start) - chars));	/* Multibyte count */
		D_encoding = UTF8;
		D_x -= (max - chars);	/* Yak! But this is necessary to count for
					   the fact that not every byte represents a
					   character. */
		return start + chars;
	} else {
		PutWinMsg(s, start, max);
		return max;
	}
}

/* refresh the display's hstatus line */
void ShowHStatus(char *str)
{
	int l, ox, oy, max;

	if (D_status == STATUS_ON_WIN && (D_has_hstatus == HSTATUS_FIRSTLINE || D_has_hstatus == HSTATUS_LASTLINE) && STATLINE() == D_height - 1)
		return;		/* sorry, in use */
	if (D_blocked)
		return;

	if (D_HS && D_has_hstatus == HSTATUS_HS) {
		if (!D_hstatus && (str == NULL || *str == 0))
			return;
		SetRendition(&mchar_null);
		InsertMode(false);
		if (D_hstatus)
			AddCStr(D_DS);
		D_hstatus = false;
		if (str == NULL || *str == 0)
			return;
		AddCStr2(D_TS, 0);
		max = D_WS > 0 ? D_WS : (D_width - !D_CLP);
		if ((int)strlen(str) > max)
			AddStrn(str, max);
		else
			AddStr(str);
		AddCStr(D_FS);
		D_hstatus = true;
	} else if (D_has_hstatus == HSTATUS_LASTLINE) {
		ox = D_x;
		oy = D_y;
		str = str ? str : "";
		l = strlen(str);
		if (l > D_width)
			l = D_width;
		GotoPos(0, D_height - 1);
		SetRendition(&mchar_null);
		l = PrePutWinMsg(str, 0, l);
		if (!captionalways && D_cvlist && !D_cvlist->c_next)
			while (l++ < D_width)
				PUTCHARLP(' ');
		if (l < D_width)
			ClearArea(l, D_height - 1, l, D_width - 1, D_width - 1, D_height - 1, 0, 0);
		if (ox != -1 && oy != -1)
			GotoPos(ox, oy);
		D_hstatus = (str != NULL);
		SetRendition(&mchar_null);
	} else if (D_has_hstatus == HSTATUS_FIRSTLINE) {
		ox = D_x;
		oy = D_y;
		str = str ? str : "";
		l = strlen(str);
		if (l > D_width)
			l = D_width;
		GotoPos(0, 0);
		SetRendition(&mchar_null);
		l = PrePutWinMsg(str, 0, l);
		if (!captionalways || (D_cvlist && !D_cvlist->c_next))
			while (l++ < D_width)
				PUTCHARLP(' ');
		if (l < D_width)
			ClearArea(l, 0, l, D_width - 1, D_width - 1, 0, 0, 0);
		if (ox != -1 && oy != -1)
			GotoPos(ox, oy);
		D_hstatus = (str != NULL);
		SetRendition(&mchar_null);
	} else if (str && *str && D_has_hstatus == HSTATUS_MESSAGE) {
		Msg(0, "%s", str);
	}
}

/*
 *  Refreshes the harstatus of the fore window. Shouldn't be here...
 */
void RefreshHStatus(void)
{
	char *buf;
	int extrabytes = strlen(hstatusstring) - strlen_onscreen(hstatusstring, NULL);
	evdeq(&D_hstatusev);
	if (D_status == STATUS_ON_HS)
		return;
	buf =
	    MakeWinMsgEv(NULL, hstatusstring, D_fore, '%',
			 (D_HS && D_has_hstatus == HSTATUS_HS && D_WS > 0) ? D_WS : D_width - !D_CLP + extrabytes, &D_hstatusev, 0);
	if (buf && *buf) {
		ShowHStatus(buf);
		if (D_has_hstatus != HSTATUS_IGNORE && D_hstatusev.timeout)
			evenq(&D_hstatusev);
	} else
		ShowHStatus(NULL);
}

/*********************************************************************/
/*
 *  Here come the routines that refresh an arbitrary part of the screen.
 */

void RefreshAll(int isblank)
{
	for (Canvas *cv = D_cvlist; cv; cv = cv->c_next) {
		CV_CALL(cv, LayRedisplayLine(-1, -1, -1, isblank));
		display = cv->c_display;	/* just in case! */
	}
	RefreshArea(0, 0, D_width - 1, D_height - 1, isblank);
}

void RefreshArea(int xs, int ys, int xe, int ye, int isblank)
{
	if (!isblank && xs == 0 && xe == D_width - 1 && ye == D_height - 1 && (ys == 0 || D_CD)) {
		ClearArea(xs, ys, xs, xe, xe, ye, 0, 0);
		isblank = 1;
	}
	for (int y = ys; y <= ye; y++)
		RefreshLine(y, xs, xe, isblank);
}

void RefreshLine(int y, int from, int to, int isblank)
{
	Viewport *vp, *lvp;
	Canvas *cv, *lcv, *cvlist, *cvlnext;
	Layer *oldflayer;
	int xx, yy, l;
	char *buf;
	Window *p;

	if (D_status == STATUS_ON_WIN && y == STATLINE()) {
		if (to >= D_status_len)
			D_status_len = to + 1;
		return;		/* can't refresh status */
	}

	if (isblank == 0 && D_CE && to == D_width - 1 && from < to && D_status != STATUS_ON_HS) {
		GotoPos(from, y);
		if (D_UT || D_BE)
			SetRendition(&mchar_null);
		AddCStr(D_CE);
		isblank = 1;
	}

	if ((y == D_height - 1 && D_has_hstatus == HSTATUS_LASTLINE) || (y == 0 && D_has_hstatus == HSTATUS_FIRSTLINE)) {
		RefreshHStatus();
		return;
	}

	while (from <= to) {
		lcv = NULL;
		lvp = NULL;
		for (cv = display->d_cvlist; cv; cv = cv->c_next) {
			if (y == (captiontop ? cv->c_ys - 1 : cv->c_ye + 1) && from >= cv->c_xs && from <= cv->c_xe) {
				int extrabytes = strlen(captionstring) - strlen_onscreen(captionstring, NULL);
				p = Layer2Window(cv->c_layer);
				buf =
				    MakeWinMsgEv(NULL, captionstring, p, '%',
						 cv->c_xe - cv->c_xs + (cv->c_xe + 1 < D_width
									|| D_CLP) + extrabytes, &cv->c_captev, 0);
				if (cv->c_captev.timeout)
					evenq(&cv->c_captev);
				xx = to > cv->c_xe ? cv->c_xe : to;
				l = strlen(buf);
				GotoPos(from, y);
				SetRendition(&mchar_so);
				if (l > xx - cv->c_xs + 1)
					l = xx - cv->c_xs + 1;
				l = PrePutWinMsg(buf, from - cv->c_xs, l + extrabytes);
				from = cv->c_xs + l;
				for (; from <= xx; from++)
					PUTCHARLP(' ');
				break;
			}
			if (from == cv->c_xe + 1 && (y >= cv->c_ys - captiontop) && (y <= cv->c_ye + !captiontop)) {
				GotoPos(from, y);
				SetRendition(&mchar_so);
				PUTCHARLP(' ');
				from++;
				break;
			}
			if (y < cv->c_ys || y > cv->c_ye || to < cv->c_xs || from > cv->c_xe)
				continue;
			for (vp = cv->c_vplist; vp; vp = vp->v_next) {
				/* find leftmost overlapping vp */
				if (y >= vp->v_ys && y <= vp->v_ye && from <= vp->v_xe && to >= vp->v_xs
				    && (lvp == NULL || lvp->v_xs > vp->v_xs)) {
					lcv = cv;
					lvp = vp;
				}
			}
		}
		if (cv)
			continue;	/* we advanced from */
		if (lvp == NULL)
			break;
		if (from < lvp->v_xs) {
			if (!isblank)
				DisplayLine(&mline_null, &mline_blank, y, from, lvp->v_xs - 1);
			from = lvp->v_xs;
		}

		/* call LayRedisplayLine on canvas lcv viewport lvp */
		yy = y - lvp->v_yoff;
		xx = to < lvp->v_xe ? to : lvp->v_xe;

		if (lcv->c_layer && lcv->c_xoff + lcv->c_layer->l_width == from) {
			GotoPos(from, y);
			SetRendition(&mchar_blank);
			PUTCHARLP('|');
			from++;
		}
		if (lcv->c_layer && yy == lcv->c_layer->l_height) {
			GotoPos(from, y);
			SetRendition(&mchar_blank);
			while (from <= lvp->v_xe && from - lvp->v_xoff < lcv->c_layer->l_width) {
				PUTCHARLP('-');
				from++;
			}
			if (from >= lvp->v_xe + 1)
				continue;
		}
		if (lcv->c_layer == NULL || yy >= lcv->c_layer->l_height || from - lvp->v_xoff >= lcv->c_layer->l_width) {
			if (!isblank)
				DisplayLine(&mline_null, &mline_blank, y, from, lvp->v_xe);
			from = lvp->v_xe + 1;
			continue;
		}

		if (xx - lvp->v_xoff >= lcv->c_layer->l_width)
			xx = lcv->c_layer->l_width + lvp->v_xoff - 1;
		oldflayer = flayer;
		flayer = lcv->c_layer;
		cvlist = flayer->l_cvlist;
		cvlnext = lcv->c_lnext;
		flayer->l_cvlist = lcv;
		lcv->c_lnext = NULL;
		LayRedisplayLine(yy, from - lvp->v_xoff, xx - lvp->v_xoff, isblank);
		flayer->l_cvlist = cvlist;
		lcv->c_lnext = cvlnext;
		flayer = oldflayer;

		from = xx + 1;
	}
	if (!isblank && from <= to)
		DisplayLine(&mline_null, &mline_blank, y, from, to);
}

/*********************************************************************/

/* clear lp_missing by writing the char on the screen. The
 * position must be safe.
 */
static void WriteLP(int x2, int y2)
{
	struct mchar oldrend;

	oldrend = D_rend;
	if (D_lpchar.mbcs) {
		if (x2 > 0)
			x2--;
		else
			D_lpchar = mchar_blank;
	}
	/* Can't use PutChar */
	GotoPos(x2, y2);
	SetRendition(&D_lpchar);
	PUTCHAR(D_lpchar.image);
	if (D_lpchar.mbcs)
		PUTCHAR(D_lpchar.mbcs);
	D_lp_missing = 0;
	SetRendition(&oldrend);
}

void ClearLine(struct mline *oml, int y, int from, int to, int bce)
{
	int x;
	struct mchar bcechar;

	if (D_UT)		/* Safe to erase ? */
		SetRendition(&mchar_null);
	if (D_BE)
		SetBackColor(bce);
	if (from == 0 && D_CB && (to != D_width - 1 || (D_x == to && D_y == y)) && (!bce || D_BE)) {
		GotoPos(to, y);
		AddCStr(D_CB);
		return;
	}
	if (to == D_width - 1 && D_CE && (!bce || D_BE)) {
		GotoPos(from, y);
		AddCStr(D_CE);
		return;
	}
	if (oml == NULL)
		oml = &mline_null;
	if (!bce) {
		DisplayLine(oml, &mline_blank, y, from, to);
		return;
	}
	bcechar = mchar_null;
	bcechar.colorbg = bce;
	for (x = from; x <= to; x++)
		copy_mchar2mline(&bcechar, &mline_old, x);
	DisplayLine(oml, &mline_old, y, from, to);
}

void DisplayLine(struct mline *oml, struct mline *ml, int y, int from, int to)
{
	int x;
	int last2flag = 0, delete_lp = 0;

	if (!D_CLP && y == D_bot && to == D_width - 1) {
		if (D_lp_missing || !cmp_mline(oml, ml, to)) {
			if ((D_IC || D_IM) && from < to && !dw_left(ml, to, D_encoding)) {
				last2flag = 1;
				D_lp_missing = 0;
				to--;
			} else {
				delete_lp = !cmp_mchar_mline(&mchar_blank, oml, to) && (D_CE || D_DC || D_CDC);
				D_lp_missing = !cmp_mchar_mline(&mchar_blank, ml, to);
				copy_mline2mchar(&D_lpchar, ml, to);
			}
		}
		to--;
	}
	if (D_mbcs) {
		/* finish dw-char (can happen after a wrap) */
		SetRenditionMline(ml, from);
		PUTCHAR(ml->image[from]);
		from++;
	}
	for (x = from; x <= to; x++) {
		if (ml != NULL) {
			if ((x < to || x != D_width - 1 || ml->image[x + 1]))
				if (cmp_mline(oml, ml, x))
					continue;
			GotoPos(x, y);
			if (dw_right(ml, x, D_encoding)) {
				if (x > 0) {
					x--;
				} else {
					x++;
				}
				GotoPos(x, y);
			}
			if (x == to && dw_left(ml, x, D_encoding))
				break;	/* don't start new kanji */
			SetRenditionMline(ml, x);
			PUTCHAR(ml->image[x]);
			if (dw_left(ml, x, D_encoding))
				PUTCHAR(ml->image[++x]);
		}
	}
	if (last2flag) {
		GotoPos(x, y);
		SetRenditionMline(ml, x + 1);
		PUTCHAR(ml->image[x + 1]);
		GotoPos(x, y);
		SetRenditionMline(ml, x);
		INSERTCHAR(ml->image[x]);
	} else if (delete_lp) {
		if (D_UT)
			SetRendition(&mchar_null);
		if (D_DC)
			AddCStr(D_DC);
		else if (D_CDC)
			AddCStr2(D_CDC, 1);
		else if (D_CE)
			AddCStr(D_CE);
	}
}

void PutChar(struct mchar *c, int x, int y)
{
	GotoPos(x, y);
	SetRendition(c);
	PUTCHARLP(c->image);
	if (c->mbcs) {
		if (D_encoding == UTF8)
			D_rend.font = 0;
		PUTCHARLP(c->mbcs);
	}
}

void InsChar(struct mchar *c, int x, int xe, int y, struct mline *oml)
{
	(void)oml; /* unused */
	GotoPos(x, y);
	if (y == D_bot && !D_CLP) {
		if (x == D_width - 1) {
			D_lp_missing = 1;
			D_lpchar = *c;
			return;
		}
		if (xe == D_width - 1)
			D_lp_missing = 0;
	}
	if (x == xe) {
		SetRendition(c);
		PUTCHARLP(c->image);
		return;
	}
	if (!(D_IC || D_CIC || D_IM) || xe != D_width - 1) {
		RefreshLine(y, x, xe, 0);
		GotoPos(x + 1, y);
		/* UpdateLine(oml, y, x, xe); */
		return;
	}
	InsertMode(true);
	if (!D_insert) {
		if (c->mbcs && D_IC)
			AddCStr(D_IC);
		if (D_IC)
			AddCStr(D_IC);
		else
			AddCStr2(D_CIC, c->mbcs ? 2 : 1);
	}
	SetRendition(c);
	RAW_PUTCHAR(c->image);
	if (c->mbcs) {
		if (D_encoding == UTF8)
			D_rend.font = 0;
		if (D_x == D_width - 1)
			PUTCHARLP(c->mbcs);
		else
			RAW_PUTCHAR(c->mbcs);
	}
}

void WrapChar(struct mchar *c, int x, int y, int xs, int ys, int xe, int ye, bool ins)
{
	int bce;

	bce = c->colorbg;
	if (xs != 0 || x != D_width || !D_AM) {
		if (y == ye)
			ScrollV(xs, ys, xe, ye, 1, bce);
		else if (y < D_height - 1)
			y++;
		if (ins)
			InsChar(c, xs, xe, y, NULL);
		else
			PutChar(c, xs, y);
		return;
	}
	if (y == ye) {		/* we have to scroll */
		ChangeScrollRegion(ys, ye);
		if (D_bot != y || D_x != D_width || (!bce && !D_BE)) {
			ScrollV(xs, ys, xe, ye, 1, bce);
			y--;
		}
	} else if (y == D_bot)	/* remove unusable region? */
		ChangeScrollRegion(0, D_height - 1);
	if (D_x != D_width || D_y != y) {
		if (D_CLP && y >= 0)	/* don't even try if !LP */
			RefreshLine(y, D_width - 1, D_width - 1, 0);
		if (D_x != D_width || D_y != y) {	/* sorry, no bonus */
			if (y == ye)
				ScrollV(xs, ys, xe, ye, 1, bce);
			GotoPos(xs, y == ye || y == D_height - 1 ? y : y + 1);
		}
	}
	if (y != ye && y < D_height - 1)
		y++;
	if (ins != D_insert)
		InsertMode(ins);
	if (ins && !D_insert) {
		InsChar(c, 0, xe, y, NULL);
		return;
	}
	D_y = y;
	D_x = 0;
	SetRendition(c);
	RAW_PUTCHAR(c->image);
	if (c->mbcs) {
		if (D_encoding == UTF8)
			D_rend.font = 0;
		RAW_PUTCHAR(c->mbcs);
	}
}

int ResizeDisplay(int wi, int he)
{
	if (D_width == wi && D_height == he) {
		return 0;
	}
	if (D_width != wi && (D_height == he || !D_CWS) && D_CZ0 && (wi == Z0width || wi == Z1width)) {
		AddCStr(wi == Z0width ? D_CZ0 : D_CZ1);
		ChangeScreenSize(wi, D_height, 0);
		return (he == D_height) ? 0 : -1;
	}
	if (D_CWS) {
		AddCStr(tgoto(D_CWS, wi, he));
		ChangeScreenSize(wi, he, 0);
		return 0;
	}
	return -1;
}

void ChangeScrollRegion(int newtop, int newbot)
{
	if (display == NULL)
		return;
	if (newtop == newbot)
		return;		/* xterm etc can't do it */
	if (newtop == -1)
		newtop = 0;
	if (newbot == -1)
		newbot = D_height - 1;
	if (D_CS == NULL) {
		D_top = 0;
		D_bot = D_height - 1;
		return;
	}
	if (D_top == newtop && D_bot == newbot)
		return;
	AddCStr(tgoto(D_CS, newbot, newtop));
	D_top = newtop;
	D_bot = newbot;
	D_y = D_x = -1;		/* Just in case... */
}

#define WT_FLAG "2"		/* change to "0" to set both title and icon */

void SetXtermOSC(int i, char *s, char *t)
{
	static char *oscs[][2] = {
		{WT_FLAG ";", "screen"},	/* set window title */
		{"11;", ""},	/* background RGB */
		{"20;", ""},	/* background */
		{"39;", "black"},	/* default foreground (black?) */
		{"49;", "white"}	/* default background (white?) */
	};

	if (!D_CXT)
		return;
	if (!s)
		s = "";
	if (!D_xtermosc[i] && !*s)
		return;
	if (i == 0 && !D_xtermosc[0])
		AddStr("\033[22;" WT_FLAG "t");	/* stack titles (xterm patch #251) */
	if (!*s)
		s = oscs[i][1];
	D_xtermosc[i] = 1;
	AddStr("\033]");
	AddStr(oscs[i][0]);
	AddStr(s);
	AddStr(t);
}

void ClearAllXtermOSC(void)
{
	for (int i = 4; i >= 0; i--)
		SetXtermOSC(i, NULL, "\a");
	if (D_xtermosc[0])
		AddStr("\033[23;" WT_FLAG "t");	/* unstack titles (xterm patch #251) */
}

#undef WT_FLAG

/*
 *  Output buffering routines
 */

void AddStr(char *str)
{
	char c;

	if (D_encoding == UTF8) {
		while ((c = *str++))
			AddUtf8((unsigned char)c);
		return;
	}
	while ((c = *str++))
		AddChar(c);
}

void AddStrn(char *str, int n)
{
	char c;

	if (D_encoding == UTF8) {
		while ((c = *str++) && n-- > 0)
			AddUtf8((unsigned char)c);
	} else
		while ((c = *str++) && n-- > 0)
			AddChar(c);
	while (n-- > 0)
		AddChar(' ');
}

void Flush(int progress)
{
	int l;
	int wr;
	char *p;

	l = D_obufp - D_obuf;
	if (l == 0)
		return;
	if (D_userfd < 0) {
		D_obuffree += l;
		D_obufp = D_obuf;
		return;
	}
	p = D_obuf;
	if (!progress) {
		fcntl(D_userfd, F_SETFL, 0);
	}
	while (l) {
		if (progress) {
			struct pollfd pfd[1];

			pfd[0].fd = D_userfd;
			pfd[0].events = POLLOUT;

			wr = poll(pfd, ARRAY_SIZE(pfd), progress / 1000);

			if (wr == -1) {
				if (errno == EINTR)
					continue;
				break;
			}
			if (wr == 0) {
				/* no progress after 3 seconds. sorry. */
				break;
			}
		}
		wr = write(D_userfd, p, l);
		if (wr <= 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		D_obuffree += wr;
		p += wr;
		l -= wr;
	}
	D_obuffree += l;
	D_obufp = D_obuf;
	if (!progress) {
		fcntl(D_userfd, F_SETFL, FNBLOCK);
	}
	if (D_blocked == 1)
		D_blocked = 0;
	D_blocked_fuzz = 0;
}

void freetty(void)
{
	if (D_userfd >= 0)
		close(D_userfd);
	D_userfd = -1;
	D_obufp = NULL;
	D_obuffree = 0;
	if (D_obuf)
		free(D_obuf);
	D_obuf = NULL;
	D_obuflen = 0;
	D_obuflenmax = -D_obufmax;
	D_blocked = 0;
	D_blocked_fuzz = 0;
}

/*
 *  Asynchronous output routines by
 *  Tim MacKenzie (tym@dibbler.cs.monash.edu.au)
 */

void Resize_obuf(void)
{
	int ind;

	if (D_status_obuffree >= 0) {
		RemoveStatusMinWait();
		if (--D_obuffree > 0)	/* redo AddChar decrement */
			return;
	}
	if (D_obuflen && D_obuf) {
		ind = D_obufp - D_obuf;
		D_obuflen += GRAIN;
		D_obuffree += GRAIN;
		D_obuf = realloc(D_obuf, D_obuflen);
	} else {
		ind = 0;
		D_obuflen = GRAIN;
		D_obuffree = GRAIN;
		D_obuf = malloc(D_obuflen);
	}
	if (!D_obuf)
		Panic(0, "Out of memory");
	D_obufp = D_obuf + ind;
	D_obuflenmax = D_obuflen - D_obufmax;
}

void DisplaySleep1000(int n, int eat)
{
	struct pollfd pfd[1];
	char buf;

	if (n <= 0)
		return;
	if (!display) {
		usleep(1000 * n);
		return;
	}

	pfd[0].fd = D_userfd;
	pfd[0].events = POLLIN;

	if (poll(pfd, ARRAY_SIZE(pfd), n) > 0) {
		if (eat)
			read(D_userfd, &buf, 1);
	}
}

void NukePending(void)
{				/* Nuke pending output in current display, clear screen */
	int len;
	int oldtop = D_top, oldbot = D_bot;
	struct mchar oldrend;
	int oldkeypad = D_keypad, oldcursorkeys = D_cursorkeys;
	int oldcurvis = D_curvis;
	int oldmouse = D_mouse;
	int extoldmouse = D_extmouse;
	bool oldbracketed = D_bracketed;
	int oldcursorstyle = D_cursorstyle;

	oldrend = D_rend;
	len = D_obufp - D_obuf;

	/* Throw away any output that we can... */
	tcflush(D_userfd, TCOFLUSH);

	D_obufp = D_obuf;
	D_obuffree += len;
	D_top = D_bot = -1;
	AddCStr(D_IS);
	AddCStr(D_TI);
	/* Turn off all attributes. (Tim MacKenzie) */
	if (D_ME)
		AddCStr(D_ME);
	else {
		if (D_hascolor)
			AddStr("\033[m");	/* why is D_ME not set? */
		AddCStr(D_SE);
		AddCStr(D_UE);
	}
	/* Check for toggle */
	if (D_IM && strcmp(D_IM, D_EI))
		AddCStr(D_EI);
	D_insert = 0;
	/* Check for toggle */
	if (D_KS && strcmp(D_KS, D_KE))
		AddCStr(D_KS);
	if (D_CCS && strcmp(D_CCS, D_CCE))
		AddCStr(D_CCS);
	AddCStr(D_CE0);
	D_rend = mchar_null;
	D_atyp = 0;
	AddCStr(D_DS);
	D_hstatus = false;
	AddCStr(D_VE);
	D_curvis = 0;
	ChangeScrollRegion(oldtop, oldbot);
	SetRendition(&oldrend);
	KeypadMode(oldkeypad);
	CursorkeysMode(oldcursorkeys);
	CursorVisibility(oldcurvis);
	MouseMode(oldmouse);
	ExtMouseMode(extoldmouse);
	BracketedPasteMode(oldbracketed);
	CursorStyle(oldcursorstyle);
	if (D_CWS) {
		AddCStr(tgoto(D_CWS, D_width, D_height));
	} else if (D_CZ0 && (D_width == Z0width || D_width == Z1width)) {
		AddCStr(D_width == Z0width ? D_CZ0 : D_CZ1);
	}
}

/* linux' select can't handle flow control, so wait 100ms if
 * we get EAGAIN
 */
static void disp_writeev_eagain(Event *event, void *data)
{
	(void)event; /* unused */

	display = (Display *)data;
	evdeq(&D_writeev);
	D_writeev.type = EV_WRITE;
	D_writeev.handler = disp_writeev_fn;
	evenq(&D_writeev);
}

static void disp_writeev_fn(Event *event, void *data)
{
	int len, size = OUTPUT_BLOCK_SIZE;

	(void)event; /* unused */

	display = (Display *)data;
	len = D_obufp - D_obuf;
	if (len < size)
		size = len;
	if (D_status_obufpos && size > D_status_obufpos)
		size = D_status_obufpos;
	size = write(D_userfd, D_obuf, size);
	if (size >= 0) {
		len -= size;
		if (len) {
			memmove(D_obuf, D_obuf + size, len);
		}
		D_obufp -= size;
		D_obuffree += size;
		if (D_status_obufpos) {
			D_status_obufpos -= size;
			if (!D_status_obufpos) {
				/* we're finished displaying the message! */
				if (D_status == STATUS_ON_WIN) {
					/* setup continue trigger */
					D_status_obuflen = D_obuflen;
					D_status_obuffree = D_obuffree;
					/* setting obbuffree to 0 will make AddChar call
					 * ResizeObuf */
					D_obuffree = D_obuflen = 0;
				}
				gettimeofday(&D_status_time, NULL);
				SetTimeout(&D_statusev, MsgWait);
				evenq(&D_statusev);
			}
		}
		if (D_blocked_fuzz) {
			D_blocked_fuzz -= size;
			if (D_blocked_fuzz < 0)
				D_blocked_fuzz = 0;
		}
		if (D_blockedev.queued) {
			if (D_obufp - D_obuf > D_obufmax / 2) {
				SetTimeout(&D_blockedev, D_nonblock);
			} else {
				evdeq(&D_blockedev);
			}
		}
		if (D_blocked == 1 && D_obuf == D_obufp) {
			/* empty again, restart output */
			D_blocked = 0;
			Activate(D_fore ? D_fore->w_norefresh : 0);
			D_blocked_fuzz = D_obufp - D_obuf;
		}
	} else {
		/* linux flow control is badly broken */
		if (errno == EAGAIN) {
			evdeq(&D_writeev);
			D_writeev.type = EV_TIMEOUT;
			D_writeev.handler = disp_writeev_eagain;
			SetTimeout(&D_writeev, 100);
			evenq(&D_writeev);
		}
		if (errno != EINTR && errno != EAGAIN)
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
			if (errno != EWOULDBLOCK)
#endif
				Msg(errno, "Error writing output to display");
	}
}

/* maximum mouse sequence length is SGR: ESC [ < b ; x ; y M */
#define MAX_MOUSE_SEQUENCE (3+10+1+10+1+10+1)

static void disp_readev_fn(Event *event, void *data)
{
	/* We have to intercept mouse sequences in order to translate them
	 * properly, and an incoming sequence could be spread over multiple
	 * I/O reads. When this occurs, we buffer intermediate state in
	 * D_mouse_params, and then use the reservation at the front of
	 * bufspace to prepend the completed and translated mouse sequence.
	 */
	ssize_t size;
	unsigned char bufspace[MAX_MOUSE_SEQUENCE + IOSIZE];
	unsigned char *buf = bufspace + MAX_MOUSE_SEQUENCE;
	Canvas *cv;

	(void)event; /* unused */

	display = (Display *)data;

	/* Hmmmm... a bit ugly... */
	if (D_forecv)
		for (cv = D_forecv->c_layer->l_cvlist; cv; cv = cv->c_lnext) {
			display = cv->c_display;
			if (D_status == STATUS_ON_WIN)
				RemoveStatus();
		}

	display = (Display *)data;
	if (D_fore == NULL)
		size = IOSIZE;
	else {
		if (W_UWP(D_fore))
			size = ARRAY_SIZE(D_fore->w_pwin->p_inbuf) - D_fore->w_pwin->p_inlen;
		else
			size = ARRAY_SIZE(D_fore->w_inbuf) - D_fore->w_inlen;
	}

	if (size > IOSIZE)
		size = IOSIZE;
	if (size <= 0)
		size = 1;	/* Always allow one char for command keys */

	size = read(D_userfd, buf, size);
	if (size < 0) {
		if (errno == EINTR || errno == EAGAIN)
			return;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
		if (errno == EWOULDBLOCK)
			return;
#endif
		Hangup();
		sleep(1);
		return;
	} else if (size == 0) {
		Hangup();
		sleep(1);
		return;
	}
	if (D_blocked == 4) {
		D_blocked = 0;
		KillBlanker();
		Activate(D_fore ? D_fore->w_norefresh : 0);
		ResetIdle();
		return;
	}
	if (D_blocked > 1) {	/* 2, 3 */
		flayer = NULL;
		for (Window *p = mru_window; p; p = p->w_prev_mru)
			if (p->w_zdisplay == display) {
				char *bufp = (char *)buf;
				flayer = &p->w_layer;
				while (size > 0)
					LayProcess(&bufp, (size_t*)&size);
				return;
			}
		zmodem_abort(NULL, display);
	}
	if (idletimo > 0)
		ResetIdle();
	if (D_fore)
		D_fore->w_lastdisp = display;

	if (D_mouse && D_forecv) {
		unsigned char *bp = (unsigned char *)buf;
		unsigned char *end = bp + size;
		unsigned char *mark = NULL;

		/* When mouse mode is enabled, buffer up incoming CSI until we
		 * know whether it is a mouse sequence. If not a mouse event,
		 * emit the CSI unchanged; if an invalid mouse event, swallow
		 * it; otherwise, translate the sequence and emit it.
		 *
		 * Supported mouse events take two flavors.
		 *
		 * VT200: CSI M Cb Cx Cy
		 * SGR:   CSI < Ps ; Ps ; Ps M|m
		 *
		 * UTF-8 and UXRVT modes are explicitly rejected because they
		 * introduce incompatibilities with other control sequences.
		 *
		 * NOTE: applications wishing to use SGR mode will normally
		 * enable VT200 mode first as a fallback in case the terminal
		 * does not support SGR mode. Thus, we must expect to receive
		 * either mode's sequences when mouse mode is enabled. We will
		 * dutifully translate whatever arrives, without attempting to
		 * convert between modes on behalf of the application.
		 */
		switch (D_mouse_parse.state) {
		case CSI_PY:
		case CSI_PX:
		case CSI_PB:
			/* Partial mouse sequence; do not restore suppressed
			 * characters. We will emit a translated version (if valid)
			 * or swallow them (otherwise).
			 */
			break;
		case CSI_BEGIN:
			/* Partial CSI; restore suppressed characters in case it
			 * turns out not to be a mouse sequence after all.
			 */
			*(--buf) = '[';
			++size;
			/* fall through */
		case CSI_ESC_SEEN:
			/* Escape character; restore it in case this turns out not
			 * to be the start of a mouse sequence after all.
			 */
			*(--buf) = '\033';
			++size;
			break;
		default:
			break;
		};

		while (bp != end) {
			unsigned char c = *(bp++);

			switch (D_mouse_parse.state) {
			case CSI_INACTIVE:
				if (c == '\033') {
					/* potential escape sequence */
					mark = bp - 1;
					D_mouse_parse.state = CSI_ESC_SEEN;
					SetTimeout(&D_mousetimeoutev, maptimeout);
					evenq(&D_mousetimeoutev);
				}
				break;
			case CSI_ESC_SEEN:
				if (c == '[') {
					/* continue buffering an escape sequence */
					D_mouse_parse.state = CSI_BEGIN;
					evdeq(&D_mousetimeoutev);
					SetTimeout(&D_mousetimeoutev, maptimeout);
					evenq(&D_mousetimeoutev);
				} else
					D_mouse_parse.state = CSI_INACTIVE;
				break;
			case CSI_BEGIN:
				evdeq(&D_mousetimeoutev);
				if (c == 'M') {
					/* VT200 mouse sequence */
					D_mouse_parse.state = CSI_PB;
					D_mouse_parse.sgrmode = 0;
				} else if (c == '<') {
					/* SGR mouse sequence */
					D_mouse_parse.state = CSI_PB;
					D_mouse_parse.params[(int)D_mouse_parse.state] = 0;
					D_mouse_parse.sgrmode = 1;
				} else
					D_mouse_parse.state = CSI_INACTIVE;
				break;
			case CSI_PB:
			case CSI_PX:
			case CSI_PY:
				if (D_mouse_parse.sgrmode) {
					/* SGR mode: parse decimal numbers */
					if ('0' <= c && c <= '9') {
						D_mouse_parse.params[(int)D_mouse_parse.state] *= 10;
						D_mouse_parse.params[(int)D_mouse_parse.state] += c - '0';
					} else if (D_mouse_parse.state == CSI_PY) {
						if (c == 'M' || c == 'm')
							D_mouse_parse.state = CSI_DONE;
						else
							D_mouse_parse.state = CSI_INVALID;
					} else if (c == ';') {
						D_mouse_parse.state++;
						D_mouse_parse.params[(int)D_mouse_parse.state] = 0;
					} else
						D_mouse_parse.state = CSI_INVALID;
				} else {
					/* VT200 mode: read raw binary values */
					D_mouse_parse.params[(int)D_mouse_parse.state++] = c;
				}
				break;
			default:
				break;
			}

			if (D_mouse_parse.state == CSI_INVALID) {
				/* swallow invalid sequence, but emit whatever came before */
				if (buf < mark)
					disp_processinput(display, buf, mark - buf);

				buf = bp;
				size = end - bp;
				D_mouse_parse.state = CSI_INACTIVE;
			} else if (D_mouse_parse.state == CSI_DONE) {
				/* emit whatever came before this sequence */
				if (buf < mark)
					disp_processinput(display, buf, mark - buf);

				buf = bp;
				size = end - bp;

				int x = D_mouse_parse.params[CSI_PX];
				int y = D_mouse_parse.params[CSI_PY];
				int bias = D_mouse_parse.sgrmode? 1 : 33;

				x -= bias;
				y -= bias;

				if (x >= D_forecv->c_xs && x <= D_forecv->c_xe && y >= D_forecv->c_ys && y <= D_forecv->c_ye) {
					if ((D_fore && D_fore->w_mouse) || (D_mousetrack && D_forecv->c_layer->l_mode == 1)) {
						/* Send clicks only if the window is expecting clicks */
						x -= D_forecv->c_xoff;
						y -= D_forecv->c_yoff;

						if (x >= 0 && x < D_forecv->c_layer->l_width && y >= 0 && y < D_forecv->c_layer->l_height) {
							char tmp[MAX_MOUSE_SEQUENCE+1];
							int n;

							x += bias;
							y += bias;

							if (D_mouse_parse.sgrmode) {
								n = snprintf(
								tmp, MAX_MOUSE_SEQUENCE, "\033[<%d;%d;%d%c",
								D_mouse_parse.params[CSI_PB], x, y, c);
							} else {
								n = snprintf(
								tmp, MAX_MOUSE_SEQUENCE, "\033[M%c%c%c",
								D_mouse_parse.params[CSI_PB], x, y);
							}

							if (n > MAX_MOUSE_SEQUENCE)
								n = MAX_MOUSE_SEQUENCE;

							/* emit sequence */

							buf -= n;
							size += n;
							memmove(buf, tmp, n);
						}
					}
				} else if (D_mousetrack) {
					/* 'focus' to the clicked region, only on mouse up */
					int focus = 0;
					if (D_mouse_parse.sgrmode)
						focus = (c == 'm');
					else
						focus = (D_mouse_parse.params[CSI_PB] == '#');

					Canvas *cv = FindCanvas(x, y);
					if (focus && cv) {
						SetForeCanvas(display, cv);
						/* XXX: Do we want to reset the input buffer? */
					}
				}

				D_mouse_parse.state = CSI_INACTIVE;
			}
		}

		if (D_mouse_parse.state != CSI_INACTIVE) {
			/* suppress partial sequence at end */
			size = mark? mark - buf : 0;
		}
	}

	if (size > 0)
		disp_processinput(display, buf, size);
}

static void disp_processinput(Display * display, unsigned char *buf, size_t size)
{
	if (D_encoding != (D_forecv ? D_forecv->c_layer->l_encoding : 0)) {
		int c, enc;
		ptrdiff_t i, j;
		char buf2[IOSIZE * 2 + 10];
		enc = D_forecv ? D_forecv->c_layer->l_encoding : 0;
		for (i = j = 0; i < (ptrdiff_t)size; i++) {
			c = ((unsigned char *)buf)[i];
			c = DecodeChar(c, D_encoding, &D_decodestate);
			if (c == -2)
				i--;	/* try char again */
			if (c < 0)
				continue;
			if (pastefont) {
				int font = 0;
				j += EncodeChar(buf2 + j, c, enc, &font);
				j += EncodeChar(buf2 + j, -1, enc, &font);
			} else
				j += EncodeChar(buf2 + j, c, enc, NULL);
			if (j > (int)ARRAY_SIZE(buf2) - 10)	/* just in case... */
				break;
		}
		(*D_processinput) (buf2, j);
		return;
	}
	(*D_processinput) ((char *)buf, size);
}

static void disp_mousetimeout_fn(Event *event, void *data)
{
	unsigned char buf[2] = {'\033', '['};
	int size = 0;

	(void)event; /* unused */

	display = (Display *)data;

	switch (D_mouse_parse.state) {
	case CSI_PY:
	case CSI_PX:
	case CSI_PB:
		/* Do not restore */
		break;
	case CSI_BEGIN:
		++size;
		/* fall through */
	case CSI_ESC_SEEN:
		++size;
		disp_processinput(display, buf, size);
		break;
	default:
		break;
	};
	D_mouse_parse.state = CSI_INACTIVE;
}

static void disp_status_fn(Event *event, void *data)
{
	(void)event; /* unused */

	display = (Display *)data;
	if (D_status)
		RemoveStatus();
}

static void disp_hstatus_fn(Event *event, void *data)
{
	display = (Display *)data;
	if (D_status == STATUS_ON_HS) {
		SetTimeout(event, 1);
		evenq(event);
		return;
	}
	RefreshHStatus();
}

static void disp_blocked_fn(Event *event, void *data)
{
	(void)event; /* unused */

	display = (Display *)data;
	if (D_obufp - D_obuf > D_obufmax + D_blocked_fuzz) {
		D_blocked = 1;
		/* re-enable all windows */
		for (Window *p = mru_window; p; p = p->w_prev_mru)
			if (p->w_readev.condneg == &D_obuflenmax) {
				p->w_readev.condpos = p->w_readev.condneg = NULL;
			}
	}
}

static void disp_map_fn(Event *event, void *data)
{
	char *p;
	int l, i;
	unsigned char *q;

	(void)event; /* unused */

	display = (Display *)data;
	if (!(l = D_seql))
		return;
	p = (char *)D_seqp - l;
	D_seqp = D_kmaps + 3;
	D_seql = 0;
	if ((q = D_seqh) != NULL) {
		D_seqh = NULL;
		i = q[0] << 8 | q[1];
		i &= ~KMAP_NOTIMEOUT;
		if (StuffKey(i))
			ProcessInput2((char *)q + 3, q[2]);
		if (display == NULL)
			return;
		l -= q[2];
		p += q[2];
	} else
		D_dontmap = 1;
	ProcessInput(p, l);
}

static void disp_idle_fn(Event *event, void *data)
{
	Display *olddisplay;

	(void)event; /* unused */

	display = (Display *)data;
	if (idletimo <= 0 || idleaction.nr == RC_ILLEGAL)
		return;
	olddisplay = display;
	flayer = D_forecv->c_layer;
	fore = D_fore;
	DoAction(&idleaction);
	if (idleaction.nr == RC_BLANKER)
		return;
	for (display = displays; display; display = display->d_next)
		if (olddisplay == display)
			break;
	if (display)
		ResetIdle();
}

void ResetIdle(void)
{
	if (idletimo > 0) {
		SetTimeout(&D_idleev, idletimo);
		if (!D_idleev.queued)
			evenq(&D_idleev);
	} else
		evdeq(&D_idleev);
}

static void disp_blanker_fn(Event *event, void *data)
{
	char buf[IOSIZE], *b;
	int size;

	(void)event; /* unused */

	display = (Display *)data;
	size = read(D_blankerev.fd, buf, IOSIZE);
	if (size <= 0) {
		evdeq(&D_blankerev);
		ClosePTY(D_blankerev.fd);
		D_blankerev.fd = -1;
		return;
	}
	for (b = buf; size; size--)
		AddChar(*b++);
}

void KillBlanker(void)
{
	int oldtop = D_top, oldbot = D_bot;
	struct mchar oldrend;

	if (D_blankerev.fd == -1)
		return;
	if (D_blocked == 4)
		D_blocked = 0;
	evdeq(&D_blankerev);
	ClosePTY(D_blankerev.fd);
	D_blankerev.fd = -1;
	Kill(D_blankerpid, SIGHUP);
	D_top = D_bot = -1;
	oldrend = D_rend;
	if (D_ME) {
		AddCStr(D_ME);
		AddCStr(D_ME);
	} else {
		if (D_hascolor)
			AddStr("\033[m\033[m");	/* why is D_ME not set? */
		AddCStr(D_SE);
		AddCStr(D_UE);
	}
	AddCStr(D_VE);
	AddCStr(D_CE0);
	D_rend = mchar_null;
	D_atyp = 0;
	D_curvis = 0;
	D_x = D_y = -1;
	ChangeScrollRegion(oldtop, oldbot);
	SetRendition(&oldrend);
	ClearAll();
}

void RunBlanker(char **cmdv)
{
	char *m;
	pid_t pid;
	int slave = -1;
	int ptype = 0;
	char termname[MAXTERMLEN + 6];
	char **np;

	strncpy(termname, "TERM=", 6);
	strncpy(termname + 5, D_termname, MAXTERMLEN - 6);
	termname[ARRAY_SIZE(termname) - 1] = 0;
	KillBlanker();
	D_blankerpid = -1;
	if ((D_blankerev.fd = OpenDevice(cmdv, 0, &ptype, &m)) == -1) {
		Msg(0, "OpenDevice failed");
		return;
	}
#ifdef O_NOCTTY
	if (pty_preopen) {
		if ((slave = open(m, O_RDWR | O_NOCTTY)) == -1) {
			Msg(errno, "%s", m);
			ClosePTY(D_blankerev.fd);
			D_blankerev.fd = -1;
			return;
		}
	}
#endif
	switch (pid = fork()) {
	case -1:
		Msg(errno, "fork");
		ClosePTY(D_blankerev.fd);
		D_blankerev.fd = -1;
		if (slave != -1)
			close(slave);
		return;
	case 0:
		displays = NULL;
		ServerSocket = -1;
		if (setgid(real_gid) || setuid(real_uid))
			Panic(errno, "setuid/setgid");
		eff_uid = real_uid;
		eff_gid = real_gid;
		brktty(D_userfd);
		freetty();
		if (slave != -1) {
			close(0);
			dup(slave);
			close(slave);
			closeallfiles(D_blankerev.fd);
			slave = dup(0);
		} else
			closeallfiles(D_blankerev.fd);
		close(0);
		close(1);
		close(2);
		if (open(m, O_RDWR))
			Panic(errno, "Cannot open %s", m);
		if (dup(0) < 0)
			Panic(errno, "Cannot dup() %s", m);
		if (dup(0) < 0)
			Panic(errno, "Cannot dup() %s", m);
		if (slave != -1)
			close(slave);
		close(D_blankerev.fd);
		fgtty(0);
		SetTTY(0, &D_OldMode);
		np = NewEnv + 3;
		*np++ = NewEnv[0];
		*np++ = termname;
		glwz.ws_col = D_width;
		glwz.ws_row = D_height;
		(void)ioctl(0, TIOCSWINSZ, (char *)&glwz);
		display = NULL;
		execvpe(*cmdv, cmdv, NewEnv + 3);
		Panic(errno, "Cannot exec '%s'", *cmdv);
	default:
		break;
	}
	D_blankerpid = pid;
	evenq(&D_blankerev);
	D_blocked = 4;
	ClearAll();
	if (slave != -1)
		close(slave);
}

void ClearScrollbackBuffer(void)
{
	if (D_CE3)
		AddCStr(D_CE3);
}
