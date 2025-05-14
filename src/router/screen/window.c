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
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"

#include "window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include "fileio.h"
#include "help.h"
#include "input.h"
#include "mark.h"
#include "misc.h"
#include "process.h"
#include "pty.h"
#include "resize.h"
#include "telnet.h"
#include "termcap.h"
#include "tty.h"
#include "utmp.h"
#include "winmsg.h"

static void WinProcess(char **, size_t *);
static void WinRedisplayLine(int, int, int, int);
static void WinClearLine(int, int, int, int);
static int WinResize(int, int);
static void WinRestore(void);
static int DoAutolf(char *, size_t *, int);
static void ZombieProcess(char **, size_t *);
static void win_readev_fn(Event *, void *);
static void win_writeev_fn(Event *, void *);
static void win_resurrect_zombie_fn(Event *, void *);
static int muchpending(Window *, Event *);
static void paste_slowev_fn(Event *, void *);
static void pseu_readev_fn(Event *, void *);
static void pseu_writeev_fn(Event *, void *);
static void win_silenceev_fn(Event *, void *);
static void win_destroyev_fn(Event *, void *);

static int ForkWindow(Window *, char **, char *);
static void zmodem_found(Window *, int, char *, size_t);
static void zmodemFin(char *, size_t, void *);
static int zmodem_parse(Window *, char *, size_t);

bool VerboseCreate = false;		/* XXX move this to user.h */

char DefaultShell[] = "/bin/sh";
#ifndef HAVE_EXECVPE
static char DefaultPath[] = ":/usr/ucb:/bin:/usr/bin";
#endif

struct NewWindow nwin_undef = {
	.StartAt             = -1,
	.aka                 = NULL,
	.args                = NULL,
	.dir                 = NULL,
	.term                = NULL,
	.aflag               = false,
	.dynamicaka          = false,
	.flowflag            = -1,
	.list_order          = false,
	.list_nested         = false,
	.lflag               = -1,
	.histheight          = -1,
	.monitor             = -1,
	.wlock               = -1,
	.silence             = -1,
	.wrap                = false,
	.Lflag               = false,
	.slow                = -1,
	.gr                  = -1,
	.c1                  = false,
	.bce                 = -1,
	.encoding            = -1,
	.hstatus             = NULL,
	.charset             = NULL,
	.poll_zombie_timeout = 0
};

struct NewWindow nwin_default = {
	.StartAt    = 0,
	.aka        = NULL,
	.args       = ShellArgs,
	.dir        = NULL,
	.term       = screenterm,
	.aflag      = false,
	.dynamicaka = true,
	.flowflag   = FLOW_ON,
	.list_order   = false,
	.list_nested = false,
	.lflag      = 1,
	.histheight = DEFAULTHISTHEIGHT,
	.monitor    = MON_OFF,
	.wlock      = WLOCK_OFF,
	.silence    = 0,
	.wrap       = true,
	.Lflag      = false,
	.slow       = 0,
	.gr         = 0,
	.c1         = true,
	.bce        = 0,
	.encoding   = 0,
	.hstatus    = NULL,
	.charset    = NULL
};

struct NewWindow nwin_options;

static int const_IOSIZE = IOSIZE;
static int const_one = 1;

void nwin_compose(struct NewWindow *def, struct NewWindow *new, struct NewWindow *res)
{
#define COMPOSE(x) res->x = new->x != nwin_undef.x ? new->x : def->x
	COMPOSE(StartAt);
	COMPOSE(aka);
	COMPOSE(args);
	COMPOSE(dir);
	COMPOSE(term);
	COMPOSE(aflag);
	COMPOSE(dynamicaka);
	COMPOSE(flowflag);
	COMPOSE(list_order);
	COMPOSE(list_nested);
	COMPOSE(lflag);
	COMPOSE(histheight);
	COMPOSE(monitor);
	COMPOSE(wlock);
	COMPOSE(silence);
	COMPOSE(wrap);
	COMPOSE(Lflag);
	COMPOSE(slow);
	COMPOSE(gr);
	COMPOSE(c1);
	COMPOSE(bce);
	COMPOSE(encoding);
	COMPOSE(hstatus);
	COMPOSE(charset);
	COMPOSE(poll_zombie_timeout);
#undef COMPOSE
}

/*****************************************************************
 *
 *  The window layer functions
 */

const struct LayFuncs WinLf = {
	WinProcess,
	NULL,
	WinRedisplayLine,
	WinClearLine,
	WinResize,
	WinRestore,
	NULL
};

static int DoAutolf(char *buf, size_t *lenp, int fr)
{
	char *p;
	size_t len = *lenp;
	int trunc = 0;

	for (p = buf; len > 0; p++, len--) {
		if (*p != '\r')
			continue;
		if (fr-- <= 0) {
			trunc++;
			len--;
		}
		if (len == 0)
			break;
		memmove(p + 1, p, len++);
		p[1] = '\n';
	}
	*lenp = p - buf;
	return trunc;
}

static void WinProcess(char **bufpp, size_t *lenp)
{
	size_t l2 = 0, f, *ilen, l = *lenp, trunc;
	char *ibuf;

	fore = (Window *)flayer->l_data;

	if (fore->w_type == W_TYPE_GROUP) {
		*bufpp += *lenp;
		*lenp = 0;
		return;
	}
	if (fore->w_ptyfd < 0) {	/* zombie? */
		ZombieProcess(bufpp, lenp);
		return;
	}
	/* a pending writelock is this:
	 * fore->w_wlock == WLOCK_AUTO, fore->w_wlockuse = NULL
	 * The user who wants to use this window next, will get the lock, if he can.
	 */
	if (display && fore->w_wlock == WLOCK_AUTO && !fore->w_wlockuser && !AclCheckPermWin(D_user, ACL_WRITE, fore)) {
		fore->w_wlockuser = D_user;
	}
	/* if w_wlock is set, only one user may write, else we check acls */
	if (display && ((fore->w_wlock == WLOCK_OFF) ?
			AclCheckPermWin(D_user, ACL_WRITE, fore) : (D_user != fore->w_wlockuser))) {
		Msg(0, "write: permission denied (user %s)", D_user->u_name);
		*bufpp += *lenp;
		*lenp = 0;
		return;
	}
#ifdef ENABLE_TELNET
	if (fore->w_type == W_TYPE_TELNET && TelIsline(fore) && *bufpp != fore->w_telbuf) {
		TelProcessLine(bufpp, lenp);
		return;
	}
#endif

	if (W_UWP(fore)) {
		/* we send the user input to our pseudowin */
		ibuf = fore->w_pwin->p_inbuf;
		ilen = &fore->w_pwin->p_inlen;
		f = ARRAY_SIZE(fore->w_pwin->p_inbuf) - *ilen;
	} else {
		/* we send the user input to the window */
		ibuf = fore->w_inbuf;
		ilen = &fore->w_inlen;
		f = ARRAY_SIZE(fore->w_inbuf) - *ilen;
	}

	if (l > f)
		l = f;
#ifdef ENABLE_TELNET
	while (l > 0)
#else
	if (l > 0)
#endif
	{
		l2 = l;
		memmove(ibuf + *ilen, *bufpp, l2);
		if (fore->w_autolf && (trunc = DoAutolf(ibuf + *ilen, &l2, f - l2)))
			l -= trunc;
#ifdef ENABLE_TELNET
		if (fore->w_type == W_TYPE_TELNET && (trunc = DoTelnet(ibuf + *ilen, &l2, f - l2))) {
			l -= trunc;
			if (fore->w_autolf)
				continue;	/* need exact value */
		}
#endif
		*ilen += l2;
		*bufpp += l;
		*lenp -= l;
		return;
	}
}

static void ZombieProcess(char **bufpp, size_t *lenp)
{
	size_t l = *lenp;
	char *buf = *bufpp, b1[10], b2[10];

	fore = (Window *)flayer->l_data;

	*bufpp += *lenp;
	*lenp = 0;
	for (; l-- > 0; buf++) {
		if (*(unsigned char *)buf == ZombieKey_destroy) {
			KillWindow(fore);
			return;
		}
		if (*(unsigned char *)buf == ZombieKey_resurrect) {
			WriteString(fore, "\r\n", 2);
			RemakeWindow(fore);
			return;
		}
	}
	b1[AddXChar(b1, ZombieKey_destroy)] = '\0';
	b2[AddXChar(b2, ZombieKey_resurrect)] = '\0';
	Msg(0, "Press %s to destroy or %s to resurrect window", b1, b2);
}

static void WinRedisplayLine(int y, int from, int to, int isblank)
{
	if (y < 0)
		return;
	fore = (Window *)flayer->l_data;
	if (from == 0 && y > 0 && fore->w_mlines[y - 1].image[fore->w_width] == 0)
		LCDisplayLineWrap(&fore->w_layer, &fore->w_mlines[y], y, from, to, isblank);
	else
		LCDisplayLine(&fore->w_layer, &fore->w_mlines[y], y, from, to, isblank);
}

static void WinClearLine(int y, int xs, int xe, int bce)
{
	fore = (Window *)flayer->l_data;
	LClearLine(flayer, y, xs, xe, bce, &fore->w_mlines[y]);
}

static int WinResize(int wi, int he)
{
	fore = (Window *)flayer->l_data;
	ChangeWindowSize(fore, wi, he, fore->w_histheight);
	return 0;
}

static void WinRestore(void)
{
	fore = (Window *)flayer->l_data;
	for (Canvas *cv = flayer->l_cvlist; cv; cv = cv->c_next) {
		display = cv->c_display;
		if (cv != D_forecv)
			continue;
		/* ChangeScrollRegion(fore->w_top, fore->w_bot); */
		KeypadMode(fore->w_keypad);
		CursorkeysMode(fore->w_cursorkeys);
		SetFlow(fore->w_flow & FLOW_ON);
		InsertMode(fore->w_insert);
		ReverseVideo(fore->w_revvid);
		CursorVisibility(fore->w_curinv ? -1 : fore->w_curvvis);
		MouseMode(fore->w_mouse);
		ExtMouseMode(fore->w_extmouse);
		BracketedPasteMode(fore->w_bracketed);
		CursorStyle(fore->w_cursorstyle);
	}
}

/*****************************************************************/

/*
 * DoStartLog constructs a path for the "want to be logfile" in buf and
 * attempts logfopen.
 *
 * returns 0 on success.
 */
int DoStartLog(Window *window, char *buf, int bufsize)
{
	int n;
	if (!window || !buf)
		return -1;

	strncpy(buf, MakeWinMsg(screenlogfile, window, '%'), bufsize - 1);
	buf[bufsize - 1] = 0;

	if (window->w_log != NULL)
		logfclose(window->w_log);

	if ((window->w_log = logfopen(buf, islogfile(buf) ? NULL : secfopen(buf, "a"))) == NULL)
		return -2;
	if (!logflushev.queued) {
		n = log_flush ? log_flush : (logtstamp_after + 4) / 5;
		if (n) {
			SetTimeout(&logflushev, n * 1000);
			evenq(&logflushev);
		}
	}
	return 0;
}

static void remove_window_from_list(Window *win)
{
	if (win) {
		Window *tmp = win->w_next;
		if (win->w_prev) {
			win->w_prev->w_next = tmp;
		} else {
			first_window = tmp;
		}
		if (tmp) {
			tmp->w_prev = win->w_prev;
		} else {
			last_window = win->w_prev;
		}
		win->w_next = NULL;
		win->w_prev = NULL;

		if (win == mru_window) {
			mru_window = win->w_prev_mru;
		} else {
			for (Window *w = mru_window; w; w = w->w_prev_mru) {
				if (w->w_prev_mru == win) {
					w->w_prev_mru = w->w_prev_mru->w_prev_mru;
					break;
				}
			}
		}
		win->w_prev_mru = NULL;
	}
}

/*
 * helper function to insert window into window list
 * we insert p before win, if win == NULL we are either at the end of list or list is empty
 */
static void add_window_to_list(Window *p, Window *win)
{
	if (!p)
		return; /* maybe we should just Panic? but we should always get window to place */

	/* most recently used list */

	p->w_prev_mru = mru_window;
	mru_window = p;
	/*
	 * place the new window in proper place on window list
	 */

	if (win) {
		/* if we are not at the end of list, we point after insertion point */
		if (win == first_window) {
			/* we insert at the beginning! */
			first_window->w_prev = p;
			p->w_next = first_window;
			p->w_prev = NULL;
			first_window = p;
		} else {
			/* we insert in the middle */
			p->w_next = win->w_prev->w_next;
			p->w_prev = win->w_prev;
			win->w_prev->w_next = p;
			win->w_prev = p;
		}
	} else {
		/* if win is NULL, then we are either at the end of the list or there are no windows */
		win = last_window;
		if (win) {
			/* we have last window, so add new window at the end of list */
			last_window->w_next = p;
			p->w_prev = last_window;
			p->w_next = NULL;
			last_window = p;
		} else {
			/* there are no windows */
			first_window = p;
			last_window = p;
			p->w_next = NULL;
			p->w_prev = NULL;
		}
	}
}

/*
 * Umask & wlock are set for the user of the display,
 * The display d (if specified) switches to that window.
 */
int MakeWindow(struct NewWindow *newwin)
{
	Window *p;
	int i;
	int f = -1;
	struct NewWindow nwin;
	int type, startat;
	char *TtyName;
	Window *win = first_window;

	nwin_compose(&nwin_default, newwin, &nwin);

	startat = nwin.StartAt;
	/* skip windows with lower number */
	while (win && win->w_number < startat)
		win = win->w_next;
	/* if there is no free spot, look for one (we can reach end of list) */
	while (win && win->w_number == startat) {
		win = win->w_next;
		startat++;
	}

#ifdef ENABLE_TELNET
	if (!strcmp(nwin.args[0], "//telnet")) {
		type = W_TYPE_TELNET;
		TtyName = "telnet";
	} else
#endif
	if ((f = OpenDevice(nwin.args, nwin.lflag, &type, &TtyName)) < 0)
		return -1;
	if (type == W_TYPE_GROUP)
		f = -1;

	if ((p = calloc(1, sizeof(Window))) == NULL) {
		if (type == W_TYPE_PTY)
			ClosePTY(f);
		else
			close(f);
		Msg(0, "%s", strnomem);
		return -1;
	}
#ifdef ENABLE_UTMP
	if (type != W_TYPE_PTY)
		nwin.lflag = 0;
#endif

	p->w_type = type;

	/* save the command line so that zombies can be resurrected */
	for (i = 0; nwin.args[i] && i < MAXARGS - 1; i++)
		p->w_cmdargs[i] = SaveStr(nwin.args[i]);
	p->w_cmdargs[i] = NULL;
	if (nwin.dir)
		p->w_dir = SaveStr(nwin.dir);
	if (nwin.term)
		p->w_term = SaveStr(nwin.term);

	p->w_number = startat;
	p->w_group = NULL;
	if (fore && fore->w_type == W_TYPE_GROUP)
		p->w_group = fore;
	else if (fore && fore->w_group)
		p->w_group = fore->w_group;
	/*
	 * This is dangerous: without a display we use creators umask
	 * This is intended to be useful for detached startup.
	 * But is still better than default bits with a NULL user.
	 */
	if (NewWindowAcl(p, display ? D_user : users)) {
		free((char *)p);
		if (type == W_TYPE_PTY)
			ClosePTY(f);
		else
			close(f);
		Msg(0, "%s", strnomem);
		return -1;
	}
	p->w_layer.l_next = NULL;
	p->w_layer.l_bottom = &p->w_layer;
	p->w_layer.l_layfn = &WinLf;
	p->w_layer.l_data = (char *)p;
	p->w_savelayer = &p->w_layer;
	p->w_pdisplay = NULL;
	p->w_lastdisp = NULL;
	p->w_list_order = nwin.list_order;
	p->w_list_nested = nwin.list_nested;

	if (display && !AclCheckPermWin(D_user, ACL_WRITE, p))
		p->w_wlockuser = D_user;
	p->w_wlock = nwin.wlock;
	p->w_ptyfd = f;
	p->w_aflag = nwin.aflag;
	p->w_dynamicaka = nwin.dynamicaka;
	p->w_flow = nwin.flowflag | ((nwin.flowflag & FLOW_AUTOFLAG) ? (FLOW_AUTO | FLOW_ON) : FLOW_AUTO);
	if (!nwin.aka)
		nwin.aka = Filename(nwin.args[0]);
	strncpy(p->w_akabuf, nwin.aka, ARRAY_SIZE(p->w_akabuf) - 1);
	if ((nwin.aka = strrchr(p->w_akabuf, '|')) != NULL) {
		p->w_autoaka = 0;
		*nwin.aka++ = 0;
		p->w_title = nwin.aka;
		p->w_akachange = nwin.aka + strlen(nwin.aka);
	} else
		p->w_title = p->w_akachange = p->w_akabuf;
	if (nwin.hstatus)
		p->w_hstatus = SaveStr(nwin.hstatus);
	p->w_monitor = nwin.monitor;
	if (p->w_monitor == MON_ON) {
		/* always tell all users */
		for (i = 0; i < maxusercount; i++)
			ACLBYTE(p->w_mon_notify, i) |= ACLBIT(i);
	}
	/*
	 * defsilence by Lloyd Zusman (zusman_lloyd@jpmorgan.com)
	 */
	p->w_silence = nwin.silence;
	p->w_silencewait = SilenceWait;
	if (p->w_silence == SILENCE_ON) {
		/* always tell all users */
		for (i = 0; i < maxusercount; i++)
			ACLBYTE(p->w_lio_notify, i) |= ACLBIT(i);
	}
	p->w_slowpaste = nwin.slow;

	p->w_norefresh = 0;
	strncpy(p->w_tty, TtyName, MAXSTR - 1);

	if (ChangeWindowSize(p, display ? D_forecv->c_xe - D_forecv->c_xs + 1 : 80,
			     display ? D_forecv->c_ye - D_forecv->c_ys + 1 : 24, nwin.histheight)) {
		FreeWindow(p);
		return -1;
	}

	p->w_encoding = nwin.encoding;
	ResetWindow(p);		/* sets w_wrap, w_c1, w_gr */

	if (nwin.charset)
		SetCharsets(p, nwin.charset);

	if (VerboseCreate && type != W_TYPE_GROUP) {
		Display *d = display;	/* WriteString zaps display */

		WriteString(p, ":screen (", 9);
		WriteString(p, p->w_title, strlen(p->w_title));
		WriteString(p, "):", 2);
		for (f = 0; p->w_cmdargs[f]; f++) {
			WriteString(p, " ", 1);
			WriteString(p, p->w_cmdargs[f], strlen(p->w_cmdargs[f]));
		}
		WriteString(p, "\r\n", 2);
		display = d;
	}

	p->w_deadpid = 0;
	p->w_pid = 0;
	p->w_pwin = NULL;

#ifdef ENABLE_TELNET
	if (type == W_TYPE_TELNET) {
		if (TelOpenAndConnect(p)) {
			FreeWindow(p);
			return -1;
		}
	} else
#endif
	if (type == W_TYPE_PTY) {
		p->w_pid = ForkWindow(p, nwin.args, TtyName);
		if (p->w_pid < 0) {
			FreeWindow(p);
			return -1;
		}
	}

	/*
	 * Place the new window at the head of the most-recently-used list.
	 */
	if (display && D_fore)
		D_other = D_fore;

	add_window_to_list(p, win);

	if (type == W_TYPE_GROUP) {
		SetForeWindow(p);
		Activate(p->w_norefresh);
		WindowChanged(NULL, WINESC_WIN_NAMES);
		WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
		WindowChanged(NULL, 0);
		return startat;
	}

#ifdef ENABLE_UTMP
	p->w_lflag = nwin.lflag;
	p->w_slot = (slot_t) - 1;
	if (nwin.lflag & 1) {
		p->w_slot = (slot_t) 0;
		if (display || (p->w_lflag & 2))
			SetUtmp(p);
	}
#ifdef CAREFULUTMP
	CarefulUtmp();		/* If all 've been zombies, we've had no slot */
#endif
#endif				/* ENABLE_UTMP */

	if (nwin.Lflag) {
		char buf[1024];
		DoStartLog(p, buf, ARRAY_SIZE(buf));
	}

	/* Is this all where I have to init window poll timeout? */
	if (nwin.poll_zombie_timeout)
		p->w_poll_zombie_timeout = nwin.poll_zombie_timeout;

	p->w_zombieev.type = EV_TIMEOUT;
	p->w_zombieev.data = (char *)p;
	p->w_zombieev.handler = win_resurrect_zombie_fn;

	p->w_readev.fd = p->w_writeev.fd = p->w_ptyfd;
	p->w_readev.type = EV_READ;
	p->w_writeev.type = EV_WRITE;
	p->w_readev.data = p->w_writeev.data = (char *)p;
	p->w_readev.handler = win_readev_fn;
	p->w_writeev.handler = win_writeev_fn;
	p->w_writeev.condpos = (int *)&p->w_inlen;
	evenq(&p->w_readev);
	evenq(&p->w_writeev);
	p->w_paster.pa_slowev.type = EV_TIMEOUT;
	p->w_paster.pa_slowev.data = (char *)&p->w_paster;
	p->w_paster.pa_slowev.handler = paste_slowev_fn;
	p->w_silenceev.type = EV_TIMEOUT;
	p->w_silenceev.data = (char *)p;
	p->w_silenceev.handler = win_silenceev_fn;
	if (p->w_silence > 0) {
		SetTimeout(&p->w_silenceev, p->w_silencewait * 1000);
		evenq(&p->w_silenceev);
	}
	p->w_destroyev.type = EV_TIMEOUT;
	p->w_destroyev.data = NULL;
	p->w_destroyev.handler = win_destroyev_fn;

	SetForeWindow(p);
	Activate(p->w_norefresh);
	WindowChanged(NULL, WINESC_WIN_NAMES);
	WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
	WindowChanged(NULL, 0);
	return startat;
}

/*
 * Resurrect a window from Zombie state.
 * The command vector is therefore stored in the window structure.
 * Note: The terminaltype defaults to screenterm again, the current
 * working directory is lost.
 */
int RemakeWindow(Window *window)
{
	char *TtyName;
	int lflag;
	int fd = -1;

	lflag = nwin_default.lflag;
#ifdef ENABLE_TELNET
	if (!strcmp(window->w_cmdargs[0], "//telnet")) {
		window->w_type = W_TYPE_TELNET;
		TtyName = "telnet";
	} else
#endif
	{ /* see above #ifdef */
		if ((fd = OpenDevice(window->w_cmdargs, lflag, &window->w_type, &TtyName)) < 0)
			return -1;
	}

	evdeq(&window->w_destroyev);	/* no re-destroy of resurrected zombie */

	strncpy(window->w_tty, *TtyName ? TtyName : window->w_title, MAXSTR - 1);

	window->w_ptyfd = fd;
	window->w_readev.fd = fd;
	window->w_writeev.fd = fd;
	evenq(&window->w_readev);
	evenq(&window->w_writeev);

	if (VerboseCreate) {
		Display *d = display;	/* WriteString zaps display */

		WriteString(window, ":screen (", 9);
		WriteString(window, window->w_title, strlen(window->w_title));
		WriteString(window, "):", 2);
		for (int i = 0; window->w_cmdargs[i]; i++) {
			WriteString(window, " ", 1);
			WriteString(window, window->w_cmdargs[i], strlen(window->w_cmdargs[i]));
		}
		WriteString(window, "\r\n", 2);
		display = d;
	}

	window->w_deadpid = 0;
	window->w_pid = 0;
#ifdef ENABLE_TELNET
	if (window->w_type == W_TYPE_TELNET) {
		if (TelOpenAndConnect(window))
			return -1;
	} else
#endif
	if (window->w_type == W_TYPE_PTY) {
		window->w_pid = ForkWindow(window, window->w_cmdargs, TtyName);
		if (window->w_pid < 0)
			return -1;
	}
#ifdef ENABLE_UTMP
	if (window->w_slot == (slot_t) 0 && (display || (window->w_lflag & 2)))
		SetUtmp(window);
#ifdef CAREFULUTMP
	CarefulUtmp();		/* If all 've been zombies, we've had no slot */
#endif
#endif
	WindowChanged(window, WINESC_WFLAGS);
	return window->w_number;
}

void CloseDevice(Window *window)
{
	if (window->w_ptyfd < 0) {
		return;
	}
	switch (window->w_type) {
	case W_TYPE_PTY:
		/* pty 4 SALE */
		(void)chmod(window->w_tty, 0666);
		(void)chown(window->w_tty, 0, 0);
		ClosePTY(window->w_ptyfd);
		break;
	case W_TYPE_PLAIN:
		CloseTTY(window->w_ptyfd);
		break;
	default:
		close(window->w_ptyfd);
		break;
	}
	window->w_ptyfd = -1;
	window->w_tty[0] = 0;
	evdeq(&window->w_readev);
	evdeq(&window->w_writeev);
#ifdef ENABLE_TELNET
	evdeq(&window->w_telconnev);
#endif
	window->w_readev.fd = window->w_writeev.fd = -1;
}

void FreeWindow(Window *window)
{
	if (window->w_pwin)
		FreePseudowin(window);
#ifdef ENABLE_UTMP
	RemoveUtmp(window);
#endif
	CloseDevice(window);

	if (window == console_window) {
		TtyGrabConsole(-1, false, "free");
		console_window = NULL;
	}
	if (window->w_log != NULL)
		logfclose(window->w_log);
	ChangeWindowSize(window, 0, 0, 0);

	if (window->w_type == W_TYPE_GROUP) {
		for (Window *win = mru_window; win; win = win->w_prev_mru)
			if (win->w_group == window)
				win->w_group = window->w_group;
	}

	if (window->w_hstatus)
		free(window->w_hstatus);
	for (int i = 0; window->w_cmdargs[i]; i++)
		free(window->w_cmdargs[i]);
	if (window->w_dir)
		free(window->w_dir);
	if (window->w_term)
		free(window->w_term);
	for (Display *display = displays; display; display = display->d_next) {
		if (display->d_other == window)
			display->d_other = display->d_fore && display->d_fore->w_prev_mru != window ? display->d_fore->w_prev_mru : window->w_prev_mru;
		if (display->d_fore == window)
			display->d_fore = NULL;
		for (Canvas *canvas = display->d_cvlist; canvas; canvas = canvas->c_next) {
			Layer *layer;
			for (layer = canvas->c_layer; layer; layer = layer->l_next)
				if (layer->l_layfn == &WinLf)
					break;
			if (!layer)
				continue;
			if ((Window *)layer->l_data != window)
				continue;
			if (canvas->c_layer == window->w_savelayer)
				window->w_savelayer = NULL;
			KillLayerChain(canvas->c_layer);
		}
	}
	if (window->w_savelayer)
		KillLayerChain(window->w_savelayer);
	for (Canvas *canvas = window->w_layer.l_cvlist; canvas; canvas = canvas->c_lnext) {
		canvas->c_layer = &canvas->c_blank;
		canvas->c_blank.l_cvlist = canvas;
		canvas->c_lnext = NULL;
		canvas->c_xoff = canvas->c_xs;
		canvas->c_yoff = canvas->c_ys;
		RethinkViewportOffsets(canvas);
	}
	window->w_layer.l_cvlist = NULL;
	if (flayer == &window->w_layer)
		flayer = NULL;
	LayerCleanupMemory(&window->w_layer);

	FreeWindowAcl(window);
	evdeq(&window->w_readev);	/* just in case */
	evdeq(&window->w_writeev);	/* just in case */
	evdeq(&window->w_silenceev);
	evdeq(&window->w_zombieev);
	evdeq(&window->w_destroyev);
	FreePaster(&window->w_paster);
	free((char *)window);
}

int OpenDevice(char **args, int lflag, int *typep, char **namep)
{
	char *arg = args[0];
	struct stat st;
	int fd;

#ifndef ENABLE_UTMP
	(void)lflag; /* unused */
#endif

	if (!arg)
		return -1;
	if (strcmp(arg, "//group") == 0) {
		*typep = W_TYPE_GROUP;
		*namep = "telnet";
		return 0;
	}
	if (strncmp(arg, "//", 2) == 0) {
		Msg(0, "Invalid argument '%s'", arg);
		return -1;
	} else if ((stat(arg, &st)) == 0 && S_ISCHR(st.st_mode)) {
		if (access(arg, R_OK | W_OK) == -1) {
			Msg(errno, "Cannot access line '%s' for R/W", arg);
			return -1;
		}
		if ((fd = OpenTTY(arg, args[1])) < 0)
			return -1;
#ifdef ENABLE_UTMP
		lflag = 0;
#endif
		*typep = W_TYPE_PLAIN;
		*namep = arg;
	} else {
		*typep = W_TYPE_PTY;
		fd = OpenPTY(namep);
		if (fd == -1) {
			Msg(0, "No more PTYs.");
			return -1;
		}
#ifdef TIOCPKT
		{
			int flag = 1;
			if (ioctl(fd, TIOCPKT, (char *)&flag)) {
				Msg(errno, "TIOCPKT ioctl");
				ClosePTY(fd);
				return -1;
			}
		}
#endif				/* TIOCPKT */
	}
	(void)fcntl(fd, F_SETFL, FNBLOCK);
	/*
	 * Tenebreux (zeus@ns.acadiacom.net) has Linux 1.3.70 where select
	 * gets confused in the following condition:
	 * Open a pty-master side, request a flush on it, then set packet
	 * mode and call select(). Select will return a possible read, where
	 * the one byte response to the flush can be found. Select will
	 * thereafter return a possible read, which yields I/O error.
	 *
	 * If we request another flush *after* switching into packet mode,
	 * this I/O error does not occur. We receive a single response byte
	 * although we send two flush requests now.
	 *
	 * Maybe we should not flush at all.
	 *
	 * 10.5.96 jw.
	 */
	if (*typep == W_TYPE_PTY || *typep == W_TYPE_PLAIN)
		tcflush(fd, TCIOFLUSH);

	if (*typep != W_TYPE_PTY)
		return fd;

#ifndef PTY_ROFS
#ifdef PTY_GROUP
	if (chown(*namep, real_uid, PTY_GROUP) && !eff_uid)
#else
	if (chown(*namep, real_uid, real_gid) && !eff_uid)
#endif
	{
		Msg(errno, "chown tty");
		if (*typep == W_TYPE_PTY)
			ClosePTY(fd);
		else
			close(fd);
		return -1;
	}
#ifdef ENABLE_UTMP
	if (chmod(*namep, lflag ? TtyMode : (TtyMode & ~022)) && !eff_uid)
#else
	if (chmod(*namep, TtyMode) && !eff_uid)
#endif
	{
		Msg(errno, "chmod tty");
		if (*typep == W_TYPE_PTY)
			ClosePTY(fd);
		else
			close(fd);
		return -1;
	}
#endif
	return fd;
}

/*
 * Fields w_width, w_height, aflag, number (and w_tty)
 * are read from Window *win. No fields written.
 * If pwin is nonzero, filedescriptors are distributed
 * between win->w_tty and open(ttyn)
 *
 */
static int ForkWindow(Window *win, char **args, char *ttyn)
{
	pid_t pid;
	char tebuf[MAXTERMLEN + 5 + 1]; /* MAXTERMLEN + strlen("TERM=") + '\0' */
	char ebuf[20];
	char shellbuf[7 + MAXPATHLEN];
	char *proc;
	int newfd;
	int w = win->w_width;
	int h = win->w_height;
	int pat, wfdused;
	struct pseudowin *pwin = win->w_pwin;
	int slave = -1;

#ifdef O_NOCTTY
	if (pty_preopen) {
		if ((slave = open(ttyn, O_RDWR | O_NOCTTY)) == -1) {
			Msg(errno, "ttyn");
			return -1;
		}
	}
#endif
	proc = *args;
	if (proc == NULL) {
		args = ShellArgs;
		proc = *args;
	}
	fflush(stdout);
	fflush(stderr);
	switch (pid = fork()) {
	case -1:
		Msg(errno, "fork");
		break;
	case 0:
		xsignal(SIGHUP, SIG_DFL);
		xsignal(SIGINT, SIG_DFL);
		xsignal(SIGQUIT, SIG_DFL);
		xsignal(SIGTERM, SIG_DFL);
		xsignal(SIGTTIN, SIG_DFL);
		xsignal(SIGTTOU, SIG_DFL);
#ifdef SIGXFSZ
		xsignal(SIGXFSZ, SIG_DFL);
#endif

		displays = NULL;	/* beware of Panic() */
		ServerSocket = -1;
		if (setgid(real_gid) || setuid(real_uid))
			Panic(errno, "Setuid/gid");
		eff_uid = real_uid;
		eff_gid = real_gid;
		if (!pwin)	/* ignore directory if pseudo */
			if (win->w_dir && *win->w_dir && chdir(win->w_dir))
				Panic(errno, "Cannot chdir to %s", win->w_dir);

		if (display) {
			brktty(D_userfd);
			freetty();
		} else
			brktty(-1);
		if (slave != -1) {
			close(0);
			if(dup(slave) < 0)
				Panic(errno, "Cannot duplicate file descriptor");
			close(slave);
			closeallfiles(win->w_ptyfd);
			slave = dup(0);
		} else
			closeallfiles(win->w_ptyfd);
		/* Close the three /dev/null descriptors */
		close(0);
		close(1);
		close(2);
		newfd = -1;
		/*
		 * distribute filedescriptors between the ttys
		 */
		pat = pwin ? pwin->p_fdpat : ((F_PFRONT << (F_PSHIFT * 2)) | (F_PFRONT << F_PSHIFT) | F_PFRONT);
		wfdused = 0;
		for (int i = 0; i < 3; i++) {
			if (pat & F_PFRONT << F_PSHIFT * i) {
				if (newfd < 0) {
#ifdef O_NOCTTY
					if (separate_sids)
						newfd = open(ttyn, O_RDWR);
					else
						newfd = open(ttyn, O_RDWR | O_NOCTTY);
#else
					newfd = open(ttyn, O_RDWR);
#endif
					if (newfd < 0)
						Panic(errno, "Cannot open %s", ttyn);
				} else {
					if (dup(newfd) < 0)
						Panic(errno, "Cannot duplicate file descriptor");
				}
				if (fgtty(newfd))
					Msg(errno, "fgtty");
			} else {
				if(dup(win->w_ptyfd) < 0)
					Panic(errno, "Cannot duplicate file descriptor");
				wfdused = 1;
			}
		}
		if (wfdused) {
			/*
			 * the pseudo window process should not be surprised with a
			 * nonblocking filedescriptor. Poor Backend!
			 */
			if (fcntl(win->w_ptyfd, F_SETFL, 0))
				Msg(errno, "Warning: clear NBLOCK fcntl failed");
		}
		close(win->w_ptyfd);
		if (slave != -1)
			close(slave);
		if (newfd >= 0) {
			struct mode fakemode, *modep;
			if (display) {
				modep = &D_OldMode;
			} else {
				modep = &fakemode;
				InitTTY(modep, 0);
			}
			/* We only want echo if the users input goes to the pseudo
			 * and the pseudo's stdout is not send to the window.
			 */
			if (pwin && (!(pat & F_UWP) || (pat & F_PBACK << F_PSHIFT))) {
				modep->tio.c_lflag &= ~ECHO;
				modep->tio.c_iflag &= ~ICRNL;
			}
			SetTTY(newfd, modep);
			glwz.ws_col = w;
			glwz.ws_row = h;
			(void)ioctl(newfd, TIOCSWINSZ, (char *)&glwz);
			/* Always turn off nonblocking mode */
			(void)fcntl(newfd, F_SETFL, 0);
		}
		NewEnv[2] = MakeTermcap(display == NULL || win->w_aflag);
		strcpy(shellbuf, "SHELL=");
		strncpy(shellbuf + 6, ShellProg + (*ShellProg == '-'), ARRAY_SIZE(shellbuf) - 7);
		shellbuf[ARRAY_SIZE(shellbuf) - 1] = 0;
		NewEnv[4] = shellbuf;
		if (win->w_term && *win->w_term && strcmp(screenterm, win->w_term) && (strlen(win->w_term) < MAXTERMLEN)) {
			char *s1, *s2, tl;

			snprintf(tebuf, ARRAY_SIZE(tebuf), "TERM=%s", win->w_term);
			tl = strlen(win->w_term);
			NewEnv[1] = tebuf;
			if ((s1 = strchr(NewEnv[2], '|'))) {
				if ((s2 = strchr(++s1, '|'))) {
					if (strlen(NewEnv[2]) - (s2 - s1) + tl < 1024) {
						memmove(s1 + tl, s2, strlen(s2) + 1);
						memmove(s1, win->w_term, tl);
					}
				}
			}
		}
		snprintf(ebuf, ARRAY_SIZE(ebuf), "WINDOW=%d", win->w_number);
		NewEnv[3] = ebuf;

		if (*proc == '-')
			proc++;
		if (!*proc)
			proc = DefaultShell;
		execvpe(proc, args, NewEnv);
		Panic(errno, "Cannot exec '%s'", proc);
	default:
		break;
	}
	if (slave != -1)
		close(slave);
	return pid;
}

#ifndef HAVE_EXECVPE
void execvpe(char *prog, char **args, char **env)
{
	char *path = NULL;
	char buf[1024];
	char *shargs[MAXARGS + 1];
	int eaccess = 0;

	if (strrchr(prog, '/'))
		path = "";
	if (!path && !(path = getenv("PATH")))
		path = DefaultPath;
	do {
		char *p;
		for (p = buf; *path && *path != ':'; path++)
			if (p - buf < (int)ARRAY_SIZE(buf) - 2)
				*p++ = *path;
		if (p > buf)
			*p++ = '/';
		if (p - buf + strlen(prog) >= ARRAY_SIZE(buf) - 1)
			continue;
		strcpy(p, prog);
		execve(buf, args, env);
		switch (errno) {
		case ENOEXEC:
			shargs[0] = DefaultShell;
			shargs[1] = buf;
			for (int i = 1; (shargs[i + 1] = args[i]) != NULL; ++i) ;
			execve(DefaultShell, shargs, env);
			return;
		case EACCES:
			eaccess = 1;
			break;
		case ENOMEM:
		case E2BIG:
		case ETXTBSY:
			return;
		}
	} while (*path++);
	if (eaccess)
		errno = EACCES;
}
#endif

int winexec(char **av)
{
	char *p, *s, *t;
	int r = 0, l = 0;
	Window *w;
	struct pseudowin *pwin;
	int type;

	if ((w = display ? fore : mru_window) == NULL)
		return -1;
	if (!*av || w->w_pwin) {
		Msg(0, "Filter running: %s", w->w_pwin ? w->w_pwin->p_cmd : "(none)");
		return -1;
	}
	if (w->w_ptyfd < 0) {
		Msg(0, "You feel dead inside.");
		return -1;
	}
	if (!(pwin = calloc(1, sizeof(struct pseudowin)))) {
		Msg(0, "%s", strnomem);
		return -1;
	}

	/* allow ^a:!!./ttytest as a short form for ^a:exec !.. ./ttytest */
	for (s = *av; *s == ' '; s++) ;
	for (p = s; *p == ':' || *p == '.' || *p == '!'; p++) ;
	if (*p != '|')
		while (*p && p > s && p[-1] == '.')
			p--;
	if (*p == '|') {
		l = F_UWP;
		p++;
	}
	if (*p)
		av[0] = p;
	else
		av++;

	t = pwin->p_cmd;
	for (int i = 0; i < 3; i++) {
		*t = (s < p) ? *s++ : '.';
		switch (*t++) {
		case '.':
		case '|':
			l |= F_PFRONT << (i * F_PSHIFT);
			break;
		case '!':
			l |= F_PBACK << (i * F_PSHIFT);
			break;
		case ':':
			l |= F_PBOTH << (i * F_PSHIFT);
			break;
		}
	}

	if (l & F_UWP) {
		*t++ = '|';
		if ((l & F_PMASK) == F_PFRONT) {
			*pwin->p_cmd = '!';
			l ^= F_PFRONT | F_PBACK;
		}
	}
	if (!(l & F_PBACK))
		l |= F_UWP;
	*t++ = ' ';
	pwin->p_fdpat = l;

	l = MAXSTR - 4;
	for (char **pp = av; *pp; pp++) {
		p = *pp;
		while (*p && l-- > 0)
			*t++ = *p++;
		if (l <= 0)
			break;
		*t++ = ' ';
	}
	*--t = '\0';

	if ((pwin->p_ptyfd = OpenDevice(av, 0, &type, &t)) < 0) {
		free((char *)pwin);
		return -1;
	}
	strncpy(pwin->p_tty, t, MAXSTR - 1);
	w->w_pwin = pwin;
	if (type != W_TYPE_PTY) {
		FreePseudowin(w);
		Msg(0, "Cannot only use commands as pseudo win.");
		return -1;
	}
	if (!(pwin->p_fdpat & F_PFRONT))
		evdeq(&w->w_readev);
#ifdef TIOCPKT
	{
		int flag = 0;

		if (ioctl(pwin->p_ptyfd, TIOCPKT, (char *)&flag)) {
			Msg(errno, "TIOCPKT pwin ioctl");
			FreePseudowin(w);
			return -1;
		}
		if (w->w_type == W_TYPE_PTY && !(pwin->p_fdpat & F_PFRONT)) {
			if (ioctl(w->w_ptyfd, TIOCPKT, (char *)&flag)) {
				Msg(errno, "TIOCPKT win ioctl");
				FreePseudowin(w);
				return -1;
			}
		}
	}
#endif				/* TIOCPKT */

	pwin->p_readev.fd = pwin->p_writeev.fd = pwin->p_ptyfd;
	pwin->p_readev.type = EV_READ;
	pwin->p_writeev.type = EV_WRITE;
	pwin->p_readev.data = pwin->p_writeev.data = (char *)w;
	pwin->p_readev.handler = pseu_readev_fn;
	pwin->p_writeev.handler = pseu_writeev_fn;
	pwin->p_writeev.condpos = (int *)&pwin->p_inlen;
	if (pwin->p_fdpat & (F_PFRONT << F_PSHIFT * 2 | F_PFRONT << F_PSHIFT))
		evenq(&pwin->p_readev);
	evenq(&pwin->p_writeev);
	r = pwin->p_pid = ForkWindow(w, av, t);
	if (r < 0)
		FreePseudowin(w);
	return r;
}

void FreePseudowin(Window *w)
{
	struct pseudowin *pwin = w->w_pwin;

	if (fcntl(w->w_ptyfd, F_SETFL, FNBLOCK))
		Msg(errno, "Warning: FreePseudowin: NBLOCK fcntl failed");
#ifdef TIOCPKT
	if (w->w_type == W_TYPE_PTY && !(pwin->p_fdpat & F_PFRONT)) {
		int flag = 1;
		if (ioctl(w->w_ptyfd, TIOCPKT, (char *)&flag))
			Msg(errno, "Warning: FreePseudowin: TIOCPKT win ioctl");
	}
#endif
	/* should be able to use CloseDevice() here */
	(void)chmod(pwin->p_tty, 0666);
	(void)chown(pwin->p_tty, 0, 0);
	if (pwin->p_ptyfd >= 0) {
		if (w->w_type == W_TYPE_PTY)
			ClosePTY(pwin->p_ptyfd);
		else
			close(pwin->p_ptyfd);
	}
	evdeq(&pwin->p_readev);
	evdeq(&pwin->p_writeev);
	if (w->w_readev.condneg == (int *)&pwin->p_inlen)
		w->w_readev.condpos = w->w_readev.condneg = NULL;
	evenq(&w->w_readev);
	free((char *)pwin);
	w->w_pwin = NULL;
}

/*
 * returns 0, if the lock really has been released
 */
int ReleaseAutoWritelock(Display *dis, Window *w)
{
	/* release auto writelock when user has no other display here */
	if (w->w_wlock == WLOCK_AUTO && w->w_wlockuser == dis->d_user) {
		Display *d;
		for (d = displays; d; d = d->d_next)
			if ((d != dis) && (d->d_fore == w) && (d->d_user == dis->d_user))
				break;
		if (!d) {
			w->w_wlockuser = NULL;
			return 0;
		}
	}
	return 1;
}

/*
 * returns 0, if the lock really could be obtained
 */
int ObtainAutoWritelock(Display *d, Window *w)
{
	if ((w->w_wlock == WLOCK_AUTO) && !AclCheckPermWin(d->d_user, ACL_WRITE, w) && !w->w_wlockuser) {
		w->w_wlockuser = d->d_user;
		return 0;
	}
	return 1;
}

/********************************************************************/

static void paste_slowev_fn(Event *event, void *data)
{
	struct paster *pa = (struct paster *)data;
	Window *p;

	(void)event; /* unused */

	size_t len = 1;
	flayer = pa->pa_pastelayer;
	if (!flayer)
		pa->pa_pastelen = 0;
	if (!pa->pa_pastelen)
		return;
	p = Layer2Window(flayer);
	DoProcess(p, &pa->pa_pasteptr, &len, pa);
	pa->pa_pastelen -= 1 - len;
	if (pa->pa_pastelen > 0) {
		SetTimeout(&pa->pa_slowev, p->w_slowpaste);
		evenq(&pa->pa_slowev);
	}
}

static int muchpending(Window *p, Event *event)
{
	for (Canvas *cv = p->w_layer.l_cvlist; cv; cv = cv->c_lnext) {
		display = cv->c_display;
		if (D_status == STATUS_ON_WIN && !D_status_bell) {
			/* wait 'til status is gone */
			event->condpos = &const_one;
			event->condneg = (int *)&D_status;
			return 1;
		}
		if (D_blocked)
			continue;
		if (D_obufp - D_obuf > D_obufmax + D_blocked_fuzz) {
			if (D_nonblock == 0) {
				D_blocked = 1;
				continue;
			}
			event->condpos = &D_obuffree;
			event->condneg = &D_obuflenmax;
			if (D_nonblock > 0 && !D_blockedev.queued) {
				SetTimeout(&D_blockedev, D_nonblock);
				evenq(&D_blockedev);
			}
			return 1;
		}
	}
	return 0;
}

static void win_readev_fn(Event *event, void *data)
{
	Window *p = (Window *)data;
	char buf[IOSIZE], *bp;
	int size, len;
	int wtop;

	bp = buf;
	size = IOSIZE;

	wtop = p->w_pwin && W_WTOP(p);
	if (wtop) {
		size = IOSIZE - p->w_pwin->p_inlen;
		if (size <= 0) {
			event->condpos = &const_IOSIZE;
			event->condneg = (int *)&p->w_pwin->p_inlen;
			return;
		}
	}
	if (p->w_layer.l_cvlist && muchpending(p, event))
		return;
	if (!p->w_zdisplay)
		if (p->w_blocked) {
			event->condpos = &const_one;
			event->condneg = &p->w_blocked;
			return;
		}
	if (event->condpos)
		event->condpos = event->condneg = NULL;

	if ((len = p->w_outlen)) {
		p->w_outlen = 0;
		WriteString(p, p->w_outbuf, len);
		return;
	}

	if ((len = read(event->fd, buf, size)) <= 0) {
		if (errno == EINTR || errno == EAGAIN)
			return;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
		if (errno == EWOULDBLOCK)
			return;
#endif
		WindowDied(p, 0, 0);
		return;
	}
#ifdef TIOCPKT
	if (p->w_type == W_TYPE_PTY) {
		if (buf[0]) {
			if (buf[0] & TIOCPKT_NOSTOP)
				WNewAutoFlow(p, 0);
			if (buf[0] & TIOCPKT_DOSTOP)
				WNewAutoFlow(p, 1);
		}
		bp++;
		len--;
	}
#endif
#ifdef ENABLE_TELNET
	if (p->w_type == W_TYPE_TELNET)
		len = TelIn(p, bp, len, buf + ARRAY_SIZE(buf) - (bp + len));
#endif
	if (len == 0)
		return;
	if (zmodem_mode && zmodem_parse(p, bp, len))
		return;
	if (wtop) {
		memmove(p->w_pwin->p_inbuf + p->w_pwin->p_inlen, bp, len);
		p->w_pwin->p_inlen += len;
	}

	LayPause(&p->w_layer, 1);
	WriteString(p, bp, len);
	LayPause(&p->w_layer, 0);

	return;
}

static void win_resurrect_zombie_fn(Event *event, void *data)
{
	Window *p = (Window *)data;

	(void)event; /* unused */

	/* Already reconnected? */
	if (p->w_deadpid != p->w_pid)
		return;
	WriteString(p, "\r\n", 2);
	RemakeWindow(p);
}

static void win_writeev_fn(Event *event, void *data)
{
	Window *p = (Window *)data;
	size_t len;
	if (p->w_inlen) {
		if ((len = write(event->fd, p->w_inbuf, p->w_inlen)) <= 0)
			len = p->w_inlen;	/* dead window */

		if (p->w_miflag) { /* don't loop if not needed */
			for (Window *win = mru_window; win; win = win->w_prev_mru) {
				if (win != p && win->w_miflag)
					write(win->w_ptyfd, p->w_inbuf, p->w_inlen);
			}
		}

		if ((p->w_inlen -= len))
			memmove(p->w_inbuf, p->w_inbuf + len, p->w_inlen);
	}
	if (p->w_paster.pa_pastelen && !p->w_slowpaste) {
		struct paster *pa = &p->w_paster;
		flayer = pa->pa_pastelayer;
		if (flayer)
			DoProcess(p, &pa->pa_pasteptr, &pa->pa_pastelen, pa);
	}
	return;
}

static void pseu_readev_fn(Event *event, void *data)
{
	Window *p = (Window *)data;
	char buf[IOSIZE];
	int size, ptow, len;

	size = IOSIZE;

	ptow = W_PTOW(p);
	if (ptow) {
		size = IOSIZE - p->w_inlen;
		if (size <= 0) {
			event->condpos = &const_IOSIZE;
			event->condneg = (int *)&p->w_inlen;
			return;
		}
	}
	if (p->w_layer.l_cvlist && muchpending(p, event))
		return;
	if (p->w_blocked) {
		event->condpos = &const_one;
		event->condneg = &p->w_blocked;
		return;
	}
	if (event->condpos)
		event->condpos = event->condneg = NULL;

	if ((len = p->w_outlen)) {
		p->w_outlen = 0;
		WriteString(p, p->w_outbuf, len);
		return;
	}

	if ((len = read(event->fd, buf, size)) <= 0) {
		if (errno == EINTR || errno == EAGAIN)
			return;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
		if (errno == EWOULDBLOCK)
			return;
#endif
		FreePseudowin(p);
		return;
	}
	/* no packet mode on pseudos! */
	if (ptow) {
		memmove(p->w_inbuf + p->w_inlen, buf, len);
		p->w_inlen += len;
	}
	WriteString(p, buf, len);
	return;
}

static void pseu_writeev_fn(Event *event, void *data)
{
	Window *p = (Window *)data;
	struct pseudowin *pw = p->w_pwin;
	size_t len;

	if (pw->p_inlen == 0)
		return;
	if ((len = write(event->fd, pw->p_inbuf, pw->p_inlen)) <= 0)
		len = pw->p_inlen;	/* dead pseudo */
	if ((p->w_pwin->p_inlen -= len))
		memmove(p->w_pwin->p_inbuf, p->w_pwin->p_inbuf + len, p->w_pwin->p_inlen);
}

static void win_silenceev_fn(Event *event, void *data)
{
	Window *p = (Window *)data;
	Canvas *cv;
	
	(void)event; /* unused */

	for (display = displays; display; display = display->d_next) {
		for (cv = D_cvlist; cv; cv = cv->c_next)
			if (cv->c_layer->l_bottom == &p->w_layer)
				break;
		if (cv)
			continue;	/* user already sees window */
		if (!(ACLBYTE(p->w_lio_notify, D_user->u_id) & ACLBIT(D_user->u_id)))
			continue;
		Msg(0, "Window %d: silence for %d seconds", p->w_number, p->w_silencewait);
		p->w_silence = SILENCE_FOUND;
		WindowChanged(p, WINESC_WFLAGS);
	}
}

static void win_destroyev_fn(Event *event, void *data)
{
	Window *p = (Window *)event->data;

	(void)data; /* unused */

	WindowDied(p, p->w_exitstatus, 1);
}

static int zmodem_parse(Window *p, char *bp, size_t len)
{
	char *b2 = bp;
	for (size_t i = 0; i < len; i++, b2++) {
		if (p->w_zauto == 0) {
			for (; i < len; i++, b2++)
				if (*b2 == 030)
					break;
			if (i == len)
				break;
			if (i > 1 && b2[-1] == '*' && b2[-2] == '*')
				p->w_zauto = 3;
			continue;
		}
		if (p->w_zauto > 5 || *b2 == "**\030B00"[p->w_zauto] || (p->w_zauto == 5 && *b2 == '1')
		    || (p->w_zauto == 5 && p->w_zdisplay && *b2 == '8')) {
			if (++p->w_zauto < 6)
				continue;
			if (p->w_zauto == 6)
				p->w_zauto = 0;
			if (!p->w_zdisplay) {
				if (i > 6)
					WriteString(p, bp, i + 1 - 6);
				WriteString(p, "\r\n", 2);
				zmodem_found(p, *b2 == '1', b2 + 1, len - i - 1);
				return 1;
			} else if (p->w_zauto == 7 || *b2 == '8') {
				int se = p->w_zdisplay->d_blocked == 2 ? 'O' : '\212';
				for (; i < len; i++, b2++)
					if (*b2 == se)
						break;
				if (i < len) {
					zmodem_abort(p, NULL);
					D_blocked = 0;
					D_readev.condpos = D_readev.condneg = NULL;
					while (len-- > 0)
						AddChar(*bp++);
					Flush(0);
					Activate(D_fore ? D_fore->w_norefresh : 0);
					return 1;
				}
				p->w_zauto = 6;
			}
		} else
			p->w_zauto = *b2 == '*' ? (p->w_zauto == 2 ? 2 : 1) : 0;
	}
	if (p->w_zauto == 0 && bp[len - 1] == '*')
		p->w_zauto = len > 1 && bp[len - 2] == '*' ? 2 : 1;
	if (p->w_zdisplay) {
		display = p->w_zdisplay;
		while (len-- > 0)
			AddChar(*bp++);
		return 1;
	}
	return 0;
}

static void zmodemFin(char *buf, size_t len, void *data)
{
	char *s;
	size_t l;

	(void)data; /* unused */

	if (len)
		RcLine(buf, strlen(buf) + 1);
	else {
		s = "\030\030\030\030\030\030\030\030\030\030";
		l = strlen(s);
		LayProcess(&s, &l);
	}
}

static void zmodem_found(Window *p, int send, char *bp, size_t len)
{
	char *s;
	size_t n;

	/* check for abort sequence */
	n = 0;
	for (size_t i = 0; i < len; i++)
		if (bp[i] != 030)
			n = 0;
		else if (++n > 4)
			return;
	if (zmodem_mode == 3 || (zmodem_mode == 1 && p->w_type != W_TYPE_PLAIN)) {
		Display *d, *olddisplay;

		olddisplay = display;
		d = p->w_lastdisp;
		if (!d || d->d_fore != p)
			for (d = displays; d; d = d->d_next)
				if (d->d_fore == p)
					break;
		if (!d && p->w_layer.l_cvlist)
			d = p->w_layer.l_cvlist->c_display;
		if (!d)
			d = displays;
		if (!d)
			return;
		display = d;
		RemoveStatus();
		p->w_zdisplay = display;
		D_blocked = 2 + send;
		flayer = &p->w_layer;
		ZmodemPage();
		display = d;
		evdeq(&D_blockedev);
		D_readev.condpos = &const_IOSIZE;
		D_readev.condneg = (int *)&p->w_inlen;
		ClearAll();
		GotoPos(0, 0);
		SetRendition(&mchar_blank);
		AddStr("Zmodem active\r\n\r\n");
		AddStr(send ? "**\030B01" : "**\030B00");
		while (len-- > 0)
			AddChar(*bp++);
		display = olddisplay;
		return;
	}
	flayer = &p->w_layer;
	Input(":", MAXSTR, INP_COOKED, zmodemFin, NULL, 0);
	s = send ? zmodem_sendcmd : zmodem_recvcmd;
	n = strlen(s);
	LayProcess(&s, &n);
}

void zmodem_abort(Window *p, Display *d)
{
	Layer *oldflayer = flayer;
	if (p) {
		if (p->w_savelayer && p->w_savelayer->l_next) {
			if (oldflayer == p->w_savelayer)
				oldflayer = flayer->l_next;
			flayer = p->w_savelayer;
			ExitOverlayPage();
		}
		p->w_zdisplay = NULL;
		p->w_zauto = 0;
		LRefreshAll(&p->w_layer, 0);
	}
	if (d) {
		display = d;
		D_blocked = 0;
		D_readev.condpos = D_readev.condneg = NULL;
		Activate(D_fore ? D_fore->w_norefresh : 0);
	}
	flayer = oldflayer;
}

int SwapWindows(int old, int dest)
{
	Window *win_a, *win_b;
	Window *tmp;

	if (dest < 0) {
		Msg(0, "Given window position is invalid.");
		return 0;
	}

	if (old == dest)
		return 1;

	win_a = GetWindowByNumber(old);
	win_b = GetWindowByNumber(dest);

	remove_window_from_list(win_a);
	win_a->w_number = dest;
	if (win_b) {
		remove_window_from_list(win_b);
		win_b->w_number = old;
	}

	tmp = first_window;
	while (tmp && tmp->w_number < win_a->w_number)
		tmp = tmp->w_next;
	add_window_to_list(win_a, tmp);

	if (win_b) {
		tmp = first_window;
		while (tmp && tmp->w_number < win_b->w_number)
			tmp = tmp->w_next;
		add_window_to_list(win_b, tmp);
	}

	/* exchange the acls for these windows. */
#ifdef ENABLE_UTMP
	/* exchange the utmp-slots for these windows */
	if ((win_a->w_slot != (slot_t) - 1) && (win_a->w_slot != (slot_t) 0)) {
		RemoveUtmp(win_a);
		SetUtmp(win_a);
	}
	if (win_b && (win_b->w_slot != (slot_t) - 1) && (win_b->w_slot != (slot_t) 0)) {
		display = win_a->w_layer.l_cvlist ? win_a->w_layer.l_cvlist->c_display : NULL;
		RemoveUtmp(win_b);
		SetUtmp(win_b);
	}
#endif

	WindowChanged(win_a, WINESC_WIN_NUM);
	WindowChanged(NULL, WINESC_WIN_NAMES);
	WindowChanged(NULL, WINESC_WIN_NAMES_NOCUR);
	WindowChanged(NULL, 0);
	return 1;
}

void WindowDied(Window *p, int wstat, int wstat_valid)
{
	int killit = 0;

	if (p->w_destroyev.data == (char *)p) {
		wstat = p->w_exitstatus;
		wstat_valid = 1;
		evdeq(&p->w_destroyev);
		p->w_destroyev.data = NULL;
	}
	if (!wstat_valid && p->w_pid > 0) {
		/* EOF on file descriptor. The process is probably also dead.
		 * try a waitpid */
		if (waitpid(p->w_pid, &wstat, WNOHANG | WUNTRACED) == p->w_pid) {
			p->w_pid = 0;
			wstat_valid = 1;
		}
	}
	if (ZombieKey_destroy && ZombieKey_onerror && wstat_valid && WIFEXITED(wstat) && WEXITSTATUS(wstat) == 0)
		killit = 1;

	if (ZombieKey_destroy && !killit) {
		char buf[100], *s, reason[60];
		time_t now;

		if (wstat_valid) {
			if (WIFEXITED(wstat))
				if (WEXITSTATUS(wstat))
					sprintf(reason, "terminated with exit status %d", WEXITSTATUS(wstat));
				else
					sprintf(reason, "terminated normally");
			else if (WIFSIGNALED(wstat))
				sprintf(reason, "terminated with signal %d%s", WTERMSIG(wstat),
#ifdef WCOREDUMP
					WCOREDUMP(wstat) ? " (core file generated)" : "");
#else
					"");
#endif
		} else
			sprintf(reason, "detached from window");

		(void)time(&now);
		s = ctime(&now);
		if (s && *s)
			s[strlen(s) - 1] = '\0';
#ifdef ENABLE_UTMP
		if (p->w_slot != (slot_t) 0 && p->w_slot != (slot_t) - 1) {
			RemoveUtmp(p);
			p->w_slot = NULL;	/* "detached" */
		}
#endif
		CloseDevice(p);

		p->w_deadpid = p->w_pid;
		p->w_pid = 0;
		ResetWindow(p);
		/* p->w_y = p->w_bot; */
		p->w_y = MFindUsedLine(p, p->w_bot, 1);
		sprintf(buf, "\n\r=== Command %s (%s) ===", reason, s ? s : "?");
		WriteString(p, buf, strlen(buf));
		if (p->w_poll_zombie_timeout) {
			SetTimeout(&p->w_zombieev, p->w_poll_zombie_timeout * 1000);
			evenq(&p->w_zombieev);
		}
		WindowChanged(p, WINESC_WFLAGS);
	} else
		KillWindow(p);
#ifdef ENABLE_UTMP
	CarefulUtmp();
#endif
}

void ResetWindow(Window *win)
{
	win->w_wrap = nwin_default.wrap;
	win->w_origin = 0;
	win->w_insert = false;
	win->w_revvid = 0;
	win->w_mouse = 0;
	win->w_bracketed = false;
	win->w_cursorstyle = 0;
	win->w_curinv = 0;
	win->w_curvvis = 0;
	win->w_autolf = 0;
	win->w_keypad = 0;
	win->w_cursorkeys = 0;
	win->w_top = 0;
	win->w_bot = win->w_height - 1;
	win->w_saved.on = 0;
	win->w_x = win->w_y = 0;
	win->w_state = LIT;
	win->w_StringType = NONE;
	memset(win->w_tabs, 0, win->w_width);
	for (int i = 8; i < win->w_width; i += 8)
		win->w_tabs[i] = 1;
	win->w_rend = mchar_null;
	ResetCharsets(win);
}

Window *GetWindowByNumber(uint16_t n)
{
	if (first_window->w_number <= n &&
	    n <= last_window->w_number)
	{
		Window *w;
		if ((first_window->w_number + (last_window->w_number - first_window->w_number) / 2) > n) {
			/* look from the start */
			w = first_window;
			while (w && w->w_number < n)
				w = w->w_next;
		} else {
			/* look from the end */
			w = last_window;
			while (w && w->w_number > n)
				w = w->w_prev;
		}
		if (w->w_number == n)
			return w;
	}
	return NULL;
}
