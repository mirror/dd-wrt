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

#include "tty.h"

#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "screen.h"
#include "fileio.h"
#include "misc.h"
#include "pty.h"
#include "telnet.h"
#include "tty.h"

static void consredir_readev_fn(Event *, void *);
static struct baud_values *lookup_baud (int);

bool separate_sids = true;

static void DoSendBreak(int, int, int);
static void SigAlrmDummy(int);
static int SttyMode(struct mode *, char *);
static int SetBaud (struct mode *, int, int);

/* Frank Schulz (fschulz@pyramid.com):
 * I have no idea why VSTART is not defined and my fix is probably not
 * the cleanest, but it works.
 */
#if !defined(VSTART) && defined(_VSTART)
#define VSTART _VSTART
#endif
#if !defined(VSTOP) && defined(_VSTOP)
#define VSTOP _VSTOP
#endif

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

#ifndef TTYVMIN
#define TTYVMIN 1
#endif
#ifndef TTYVTIME
#define TTYVTIME 0
#endif

static void SigAlrmDummy( __attribute__ ((unused))
			 int sigsig)
{
	return;
}

/*
 *  Carefully open a character device. Not used to open display ttys.
 *  The second parameter is parsed for a few stty style options.
 */

int OpenTTY(char *line, char *opt)
{
	int f;
	struct mode Mode;
	void (*sigalrm) (int);

	sigalrm = xsignal(SIGALRM, SigAlrmDummy);
	alarm(2);

	/* this open only succeeds, if real uid is allowed */
	if ((f = secopen(line, O_RDWR | O_NONBLOCK | O_NOCTTY, 0)) == -1) {
		if (errno == EINTR)
			Msg(0, "Cannot open line '%s' for R/W: open() blocked, aborted.", line);
		else
			Msg(errno, "Cannot open line '%s' for R/W", line);
		alarm(0);
		xsignal(SIGALRM, sigalrm);
		return -1;
	}
	if (!isatty(f)) {
		Msg(0, "'%s' is not a tty", line);
		alarm(0);
		xsignal(SIGALRM, sigalrm);
		close(f);
		return -1;
	}
	/*
	 * We come here exclusively. This is to stop all kermit and cu type things
	 * accessing the same tty line.
	 * Perhaps we should better create a lock in some /usr/spool/locks directory?
	 */
#if defined(TIOCEXCL) && defined(TIOCNXCL)
	errno = 0;
	if (ioctl(f, TIOCEXCL, NULL) < 0)
		Msg(errno, "%s: ioctl TIOCEXCL failed", line);
#endif	/* TIOCEXCL && TIOCNXCL */
	/*
	 * We create a sane tty mode. We do not copy things from the display tty
	 */
	InitTTY(&Mode, W_TYPE_PLAIN);


	if (SttyMode(&Mode, opt)) {
		Msg(errno, "%s: stty failed - invalid option?", line);
		alarm(0);
		xsignal(SIGALRM, sigalrm);
		close(f);
		return -1;
	}
	SetTTY(f, &Mode);

#if defined(TIOCMSET)
	{
		int mcs = 0;
		ioctl(f, TIOCMGET, &mcs);
		mcs |= TIOCM_RTS;
		ioctl(f, TIOCMSET, &mcs);
	}
#endif

	brktty(f);
	alarm(0);
	xsignal(SIGALRM, sigalrm);
	return f;
}

int CloseTTY(int fd)
{
#if defined(TIOCEXCL) && defined(TIOCNXCL)
	(void)ioctl(fd, TIOCNXCL, NULL);
#endif
	return close(fd);
}

/*
 *  Tty mode handling
 */

void InitTTY(struct mode *m, int ttyflag)
{
	memset((char *)m, 0, sizeof(struct mode));
	/* struct termios tio
	 * defaults, as seen on SunOS 4.1.3
	 */
#if defined(BRKINT)
	m->tio.c_iflag |= BRKINT;
#endif				/* BRKINT */
#if defined(IGNPAR)
	m->tio.c_iflag |= IGNPAR;
#endif				/* IGNPAR */
/* #if defined(ISTRIP)
 * 	m->tio.c_iflag |= ISTRIP;
 * #endif
 * may be needed, let's try. jw. */
#if defined(IXON)
	m->tio.c_iflag |= IXON;
#endif				/* IXON */
/* #if defined(IMAXBEL)
 * 	m->tio.c_iflag |= IMAXBEL;
 * #endif
 * sorry, this one is ridiculus. jw */

	if (!ttyflag) {		/* may not even be good for ptys.. */
#if defined(ICRNL)
		m->tio.c_iflag |= ICRNL;
#endif				/* ICRNL */
#if defined(ONLCR)
		m->tio.c_oflag |= ONLCR;
#endif				/* ONLCR */
#if defined(TAB3)
		m->tio.c_oflag |= TAB3;
#endif				/* TAB3 */
#if defined(OXTABS)
		m->tio.c_oflag |= OXTABS;
#endif				/* OXTABS */
/* #if defined(PARENB)
 * 		m->tio.c_cflag |= PARENB;
 * #endif
 * nah! jw. */
#if defined(OPOST)
		m->tio.c_oflag |= OPOST;
#endif				/* OPOST */
	}

/*
 * Or-ing the speed into c_cflags is dangerous.
 * It breaks on bsdi, where c_ispeed and c_ospeed are extra longs.
 *
 * #if defined(B9600)
 * 	m->tio.c_cflag |= B9600;
 * #endif
 * #if defined(IBSHIFT) && defined(B9600)
 * 	m->tio.c_cflag |= B9600 << IBSHIFT;
 * #endif
 *
 * We hope that we have the posix calls to do it right:
 * If these are not available you might try the above.
 */
#if defined(B9600)
	cfsetospeed(&m->tio, B9600);
#endif				/* B9600 */
#if defined(B9600)
	cfsetispeed(&m->tio, B9600);
#endif				/* B9600 */

#if defined(CS8)
	m->tio.c_cflag |= CS8;
#endif				/* CS8 */
#if defined(CREAD)
	m->tio.c_cflag |= CREAD;
#endif				/* CREAD */
#if defined(CLOCAL)
	m->tio.c_cflag |= CLOCAL;
#endif				/* CLOCAL */

#if defined(ECHOCTL)
	m->tio.c_lflag |= ECHOCTL;
#endif				/* ECHOCTL */
#if defined(ECHOKE)
	m->tio.c_lflag |= ECHOKE;
#endif				/* ECHOKE */

	if (!ttyflag) {
#if defined(ISIG)
		m->tio.c_lflag |= ISIG;
#endif				/* ISIG */
#if defined(ICANON)
		m->tio.c_lflag |= ICANON;
#endif				/* ICANON */
#if defined(ECHO)
		m->tio.c_lflag |= ECHO;
#endif				/* ECHO */
	}
#if defined(ECHOE)
	m->tio.c_lflag |= ECHOE;
#endif				/* ECHOE */
#if defined(ECHOK)
	m->tio.c_lflag |= ECHOK;
#endif				/* ECHOK */
#if defined(IEXTEN)
	m->tio.c_lflag |= IEXTEN;
#endif				/* IEXTEN */

#if defined(VINTR)
#if (VINTR < MAXCC)
	m->tio.c_cc[VINTR] = Ctrl('C');
#endif
#endif				/* VINTR */
#if defined(VQUIT)
#if (VQUIT < MAXCC)
	m->tio.c_cc[VQUIT] = Ctrl('\\');
#endif
#endif				/* VQUIT */
#if defined(VERASE)
#if (VERASE < MAXCC)
	m->tio.c_cc[VERASE] = 0x7f;	/* DEL */
#endif
#endif				/* VERASE */
#if defined(VKILL)
#if (VKILL < MAXCC)
	m->tio.c_cc[VKILL] = Ctrl('U');
#endif
#endif				/* VKILL */
#if defined(VEOF)
#if (VEOF < MAXCC)
	m->tio.c_cc[VEOF] = Ctrl('D');
#endif
#endif				/* VEOF */
#if defined(VEOL)
#if (VEOL < MAXCC)
	m->tio.c_cc[VEOL] = VDISABLE;
#endif
#endif				/* VEOL */
#if defined(VEOL2)
#if (VEOL2 < MAXCC)
	m->tio.c_cc[VEOL2] = VDISABLE;
#endif
#endif				/* VEOL2 */
#if defined(VSWTCH)
#if (VSWTCH < MAXCC)
	m->tio.c_cc[VSWTCH] = VDISABLE;
#endif
#endif				/* VSWTCH */
#if defined(VSTART)
#if (VSTART < MAXCC)
	m->tio.c_cc[VSTART] = Ctrl('Q');
#endif
#endif				/* VSTART */
#if defined(VSTOP)
#if (VSTOP < MAXCC)
	m->tio.c_cc[VSTOP] = Ctrl('S');
#endif
#endif				/* VSTOP */
#if defined(VSUSP)
#if (VSUSP < MAXCC)
	m->tio.c_cc[VSUSP] = Ctrl('Z');
#endif
#endif				/* VSUSP */
#if defined(VDSUSP)
#if (VDSUSP < MAXCC)
	m->tio.c_cc[VDSUSP] = Ctrl('Y');
#endif
#endif				/* VDSUSP */
#if defined(VREPRINT)
#if (VREPRINT < MAXCC)
	m->tio.c_cc[VREPRINT] = Ctrl('R');
#endif
#endif				/* VREPRINT */
#if defined(VDISCARD)
#if (VDISCARD < MAXCC)
	m->tio.c_cc[VDISCARD] = Ctrl('O');
#endif
#endif				/* VDISCARD */
#if defined(VWERASE)
#if (VWERASE < MAXCC)
	m->tio.c_cc[VWERASE] = Ctrl('W');
#endif
#endif				/* VWERASE */
#if defined(VLNEXT)
#if (VLNEXT < MAXCC)
	m->tio.c_cc[VLNEXT] = Ctrl('V');
#endif
#endif				/* VLNEXT */
#if defined(VSTATUS)
#if (VSTATUS < MAXCC)
	m->tio.c_cc[VSTATUS] = Ctrl('T');
#endif
#endif				/* VSTATUS */

	if (ttyflag) {
		m->tio.c_cc[VMIN] = TTYVMIN;
		m->tio.c_cc[VTIME] = TTYVTIME;
	}
#if defined(TIOCKSET)
	m->m_jtchars.t_ascii = 'J';
	m->m_jtchars.t_kanji = 'B';
	m->m_knjmode = KM_ASCII | KM_SYSSJIS;
#endif
}

void SetTTY(int fd, struct mode *mp)
{
	errno = 0;
	tcsetattr(fd, TCSADRAIN, &mp->tio);
#if defined(TIOCKSET)
	ioctl(fd, TIOCKSETC, &mp->m_jtchars);
	ioctl(fd, TIOCKSET, &mp->m_knjmode);
#endif
	if (errno)
		Msg(errno, "SetTTY (fd %d): ioctl failed", fd);
}

void GetTTY(int fd, struct mode *mp)
{
	errno = 0;
	tcgetattr(fd, &mp->tio);
#if defined(TIOCKSET)
	ioctl(fd, TIOCKGETC, &mp->m_jtchars);
	ioctl(fd, TIOCKGET, &mp->m_knjmode);
#endif
	if (errno)
		Msg(errno, "GetTTY (fd %d): ioctl failed", fd);
}

/*
 * needs interrupt = iflag and flow = d->d_flow
 */
void SetMode(struct mode *op, struct mode *np, int flow, int interrupt)
{
	*np = *op;

#ifdef CYTERMIO
	np->m_mapkey = NOMAPKEY;
	np->m_mapscreen = NOMAPSCREEN;
	np->tio.c_line = 0;
#endif
#if defined(ICRNL)
	np->tio.c_iflag &= ~ICRNL;
#endif				/* ICRNL */
#if defined(ISTRIP)
	np->tio.c_iflag &= ~ISTRIP;
#endif				/* ISTRIP */
#if defined(ONLCR)
	np->tio.c_oflag &= ~ONLCR;
#endif				/* ONLCR */
	np->tio.c_lflag &= ~(ICANON | ECHO);
	/*
	 * From Andrew Myers (andru@tonic.lcs.mit.edu)
	 * to avoid ^V^V-Problem on OSF1
	 */
#if defined(IEXTEN)
	np->tio.c_lflag &= ~IEXTEN;
#endif				/* IEXTEN */

	/*
	 * Unfortunately, the master process never will get SIGINT if the real
	 * terminal is different from the one on which it was originaly started
	 * (process group membership has not been restored or the new tty could not
	 * be made controlling again). In my solution, it is the attacher who
	 * receives SIGINT (because it is always correctly associated with the real
	 * tty) and forwards it to the master [kill(MasterPid, SIGINT)].
	 * Marc Boucher (marc@CAM.ORG)
	 */
	if (interrupt)
		np->tio.c_lflag |= ISIG;
	else
		np->tio.c_lflag &= ~ISIG;
	/*
	 * careful, careful catche monkey..
	 * never set VMIN and VTIME to zero, if you want blocking io.
	 *
	 * We may want to do a VMIN > 0, VTIME > 0 read on the ptys too, to
	 * reduce interrupt frequency.  But then we would not know how to
	 * handle read returning 0. jw.
	 */
	np->tio.c_cc[VMIN] = 1;
	np->tio.c_cc[VTIME] = 0;
	if (!interrupt || !flow)
		np->tio.c_cc[VINTR] = VDISABLE;
	np->tio.c_cc[VQUIT] = VDISABLE;
	if (flow == 0) {
#if defined(VSTART)
#if (VSTART < MAXCC)
		np->tio.c_cc[VSTART] = VDISABLE;
#endif
#endif				/* VSTART */
#if defined(VSTOP)
#if (VSTOP < MAXCC)
		np->tio.c_cc[VSTOP] = VDISABLE;
#endif
#endif				/* VSTOP */
		np->tio.c_iflag &= ~IXON;
	}
#if defined(VDISCARD)
#if (VDISCARD < MAXCC)
	np->tio.c_cc[VDISCARD] = VDISABLE;
#endif
#endif				/* VDISCARD */
#if defined(VLNEXT)
#if (VLNEXT < MAXCC)
	np->tio.c_cc[VLNEXT] = VDISABLE;
#endif
#endif				/* VLNEXT */
#if defined(VSTATUS)
#if (VSTATUS < MAXCC)
	np->tio.c_cc[VSTATUS] = VDISABLE;
#endif
#endif				/* VSTATUS */
#if defined(VSUSP)
#if (VSUSP < MAXCC)
	np->tio.c_cc[VSUSP] = VDISABLE;
#endif
#endif				/* VSUSP */
	/* Set VERASE to DEL, rather than VDISABLE, to avoid libvte
	   "autodetect" issues. */
#if defined(VERASE)
#if (VERASE < MAXCC)
	np->tio.c_cc[VERASE] = 0x7f;
#endif
#endif				/* VERASE */
#if defined(VKILL)
#if (VKILL < MAXCC)
	np->tio.c_cc[VKILL] = VDISABLE;
#endif
#endif				/* VKILL */
#if defined(VDSUSP)
#if (VDSUSP < MAXCC)
	np->tio.c_cc[VDSUSP] = VDISABLE;
#endif
#endif				/* VDSUSP */
#if defined(VREPRINT)
#if (VREPRINT < MAXCC)
	np->tio.c_cc[VREPRINT] = VDISABLE;
#endif
#endif				/* VREPRINT */
#if defined(VWERASE)
#if (VWERASE < MAXCC)
	np->tio.c_cc[VWERASE] = VDISABLE;
#endif
#endif				/* VWERASE */
}

/* operates on display */
void SetFlow(bool on)
{
	if (D_flow == on)
		return;
	if (on) {
		D_NewMode.tio.c_cc[VINTR] = iflag ? D_OldMode.tio.c_cc[VINTR] : VDISABLE;
#if defined(VSTART)
#if (VSTART < MAXCC)
		D_NewMode.tio.c_cc[VSTART] = D_OldMode.tio.c_cc[VSTART];
#endif
#endif				/* VSTART */
#if defined(VSTOP)
#if (VSTOP < MAXCC)
		D_NewMode.tio.c_cc[VSTOP] = D_OldMode.tio.c_cc[VSTOP];
#endif
#endif				/* VSTOP */
		D_NewMode.tio.c_iflag |= D_OldMode.tio.c_iflag & IXON;
	} else {
		D_NewMode.tio.c_cc[VINTR] = VDISABLE;
#if defined(VSTART)
#if (VSTART < MAXCC)
		D_NewMode.tio.c_cc[VSTART] = VDISABLE;
#endif
#endif				/* VSTART */
#if defined(VSTOP)
#if (VSTOP < MAXCC)
		D_NewMode.tio.c_cc[VSTOP] = VDISABLE;
#endif
#endif				/* VSTOP */
		D_NewMode.tio.c_iflag &= ~IXON;
	}
#ifdef TCOON
	if (!on)
		tcflow(D_userfd, TCOON);
#endif
	tcsetattr(D_userfd, TCSANOW, &D_NewMode.tio);
	D_flow = on;
}

/* parse commands from opt and modify m */
static int SttyMode(struct mode *m, char *opt)
{
	static const char sep[] = " \t:;,";

	if (!opt)
		return 0;

	while (*opt) {
		while (strchr(sep, *opt))
			opt++;
		if (*opt >= '0' && *opt <= '9') {
			if (SetBaud(m, atoi(opt), atoi(opt)))
				return -1;
		} else if (!strncmp("cs7", opt, 3)) {
			m->tio.c_cflag &= ~CSIZE;
			m->tio.c_cflag |= CS7;
		} else if (!strncmp("cs8", opt, 3)) {
			m->tio.c_cflag &= ~CSIZE;
			m->tio.c_cflag |= CS8;
		} else if (!strncmp("istrip", opt, 6)) {
			m->tio.c_iflag |= ISTRIP;
		} else if (!strncmp("-istrip", opt, 7)) {
			m->tio.c_iflag &= ~ISTRIP;
		} else if (!strncmp("ixon", opt, 4)) {
			m->tio.c_iflag |= IXON;
		} else if (!strncmp("-ixon", opt, 5)) {
			m->tio.c_iflag &= ~IXON;
		} else if (!strncmp("ixoff", opt, 5)) {
			m->tio.c_iflag |= IXOFF;
		} else if (!strncmp("-ixoff", opt, 6)) {
			m->tio.c_iflag &= ~IXOFF;
		} else if (!strncmp("crtscts", opt, 7)) {
#if defined(CRTSCTS)
			m->tio.c_cflag |= CRTSCTS;
#endif
		} else if (!strncmp("-crtscts", opt, 8)) {
#if defined(CRTSCTS)
			m->tio.c_cflag &= ~CRTSCTS;
#endif
		} else {
			return -1;
		}
		while (*opt && !strchr(sep, *opt))
			opt++;
	}
	return 0;
}

/*
 *  Job control handling
 *
 *  Somehow the ultrix session handling is broken, so use
 *  the bsdish variant.
 */

void brktty(int fd)
{
	if (separate_sids)
		setsid();	/* will break terminal affiliation */

	ioctl(fd, TIOCSCTTY, NULL);
}

int fgtty(int fd)
{
	int mypid;

	mypid = getpid();

	ioctl(fd, TIOCSCTTY, NULL);

	if (separate_sids)
		if (tcsetpgrp(fd, mypid))
			return -1;
	return 0;
}

/*
 * The alm boards on our sparc center 1000 have a lousy driver.
 * We cannot generate long breaks unless we use the most ugly form
 * of ioctls. jw.
 */
int breaktype = 2;

/*
 * type:
 *  0:	TIOCSBRK / TIOCCBRK
 *  1:	TCSBRK
 *  2:	tcsendbreak()
 * n: approximate duration in 1/4 seconds.
 */
static void DoSendBreak(int fd, int n, int type)
{
	switch (type) {
	case 2:		/* tcsendbreak() =============================== */
#ifdef HAVE_SUPER_TCSENDBREAK
		/* There is one rare case that I have tested, where tcsendbreak works
		 * really great: this was an alm driver that came with SunOS 4.1.3
		 * If you have this one, define the above symbol.
		 * here we can use the second parameter to specify the duration.
		 */
		if (tcsendbreak(fd, n) < 0)
			Msg(errno, "cannot send BREAK (tcsendbreak)");
#else
		/*
		 * here we hope, that multiple calls to tcsendbreak() can
		 * be concatenated to form a long break, as we do not know
		 * what exact interpretation the second parameter has:
		 *
		 * - sunos 4: duration in quarter seconds
		 * - sunos 5: 0 a short break, nonzero a tcdrain()
		 * - hpux, irix: ignored
		 * - mot88: duration in milliseconds
		 * - aix: duration in milliseconds, but 0 is 25 milliseconds.
		 */
		if (!n)
			n++;
		for (int i = 0; i < n; i++)
			if (tcsendbreak(fd, 0) < 0) {
				Msg(errno, "cannot send BREAK (tcsendbreak SVR4)");
				return;
			}
#endif
		break;

	case 1:		/* TCSBRK ======================================= */
#ifdef TCSBRK
		if (!n)
			n++;
		/*
		 * Here too, we assume that short breaks can be concatenated to
		 * perform long breaks. But for SOLARIS, this is not true, of course.
		 */
		for (int i = 0; i < n; i++)
			if (ioctl(fd, TCSBRK, NULL) < 0) {
				Msg(errno, "Cannot send BREAK (TCSBRK)");
				return;
			}
#else				/* TCSBRK */
		Msg(0, "TCSBRK not available, change breaktype");
#endif				/* TCSBRK */
		break;

	case 0:		/* TIOCSBRK / TIOCCBRK ========================== */
#if defined(TIOCSBRK) && defined(TIOCCBRK)
		/*
		 * This is very rude. Screen actively celebrates the break.
		 * But it may be the only save way to issue long breaks.
		 */
		if (ioctl(fd, TIOCSBRK, NULL) < 0) {
			Msg(errno, "Can't send BREAK (TIOCSBRK)");
			return;
		}
		usleep(1000 * (n ? n * 250 : 250));
		if (ioctl(fd, TIOCCBRK, NULL) < 0) {
			Msg(errno, "BREAK stuck!!! -- HELP! (TIOCCBRK)");
			return;
		}
#else				/* TIOCSBRK && TIOCCBRK */
		Msg(0, "TIOCSBRK/CBRK not available, change breaktype");
#endif				/* TIOCSBRK && TIOCCBRK */
		break;

	default:		/* unknown ========================== */
		Msg(0, "Internal SendBreak error: method %d unknown", type);
	}
}

/*
 * Send a break for n * 0.25 seconds. Tty must be PLAIN.
 * The longest possible break allowed here is 15 seconds.
 */

void SendBreak(Window * wp, int n, int closeopen)
{
	void (*sigalrm) (int);

#ifdef ENABLE_TELNET
	if (wp->w_type == W_TYPE_TELNET) {
		TelBreak(wp);
		return;
	}
#endif
	if (wp->w_type != W_TYPE_PLAIN)
		return;

	(void)tcflush(wp->w_ptyfd, TCIOFLUSH);

	if (closeopen) {
		/* if we got exclusive access on tty, remove it */
#if defined(TIOCEXCL) && defined(TIOCNXCL)
		errno = 0;
		if (ioctl(wp->w_ptyfd, TIOCNXCL, NULL) < 0)
			Msg(errno, "%s: ioctl TIOCNXCL failed", wp->w_tty);
#endif	/* TIOCEXCL && TIOCNXCL */
		close(wp->w_ptyfd);

		usleep(1000 * (n ? n * 250 : 250));
		if ((wp->w_ptyfd = OpenTTY(wp->w_tty, wp->w_cmdargs[1])) < 1) {
			Msg(0, "Ouch, cannot reopen line %s, please try harder", wp->w_tty);
			return;
		}
		(void)fcntl(wp->w_ptyfd, F_SETFL, FNBLOCK);
	} else {
		sigalrm = xsignal(SIGALRM, SigAlrmDummy);
		alarm(15);

		DoSendBreak(wp->w_ptyfd, n, breaktype);

		alarm(0);
		xsignal(SIGALRM, sigalrm);
	}
}

/*
 *  Console grabbing
 */

static Event consredir_ev;
static int consredirfd[2] = { -1, -1 };

static void consredir_readev_fn(Event * event, void *data)
{
	char *p, *n, buf[256];
	size_t l;

	(void)data;		/* unused */

	if (!console_window || (l = read(consredirfd[0], buf, ARRAY_SIZE(buf))) <= 0) {
		close(consredirfd[0]);
		close(consredirfd[1]);
		consredirfd[0] = consredirfd[1] = -1;
		evdeq(event);
		return;
	}
	for (p = n = buf; l > 0; n++, l--)
		if (*n == '\n') {
			if (n > p)
				WriteString(console_window, p, n - p);
			WriteString(console_window, "\r\n", 2);
			p = n + 1;
		}
	if (n > p)
		WriteString(console_window, p, n - p);
}

int TtyGrabConsole(int fd, bool on, char *rc_name)
{
	Display *d;
#ifdef SRIOCSREDIR
	int cfd;
#else
	struct mode new1, new2;
	char *slave;
#endif

	(void)fd;		/* unused */

	if (on) {
		if (displays == NULL) {
			Msg(0, "I need a display");
			return -1;
		}
		for (d = displays; d; d = d->d_next)
			if (strcmp(d->d_usertty, "/dev/console") == 0)
				break;
		if (d) {
			Msg(0, "too dangerous - screen is running on /dev/console");
			return -1;
		}
	}
	if (consredirfd[0] >= 0) {
		evdeq(&consredir_ev);
		close(consredirfd[0]);
		close(consredirfd[1]);
		consredirfd[0] = consredirfd[1] = -1;
	}
	if (!on)
		return 0;
#ifdef SRIOCSREDIR
	if ((cfd = secopen("/dev/console", O_RDWR | O_NOCTTY, 0)) == -1) {
		Msg(errno, "/dev/console");
		return -1;
	}
	if (pipe(consredirfd)) {
		Msg(errno, "pipe");
		close(cfd);
		consredirfd[0] = consredirfd[1] = -1;
		return -1;
	}
	if (ioctl(cfd, SRIOCSREDIR, consredirfd[1])) {
		Msg(errno, "SRIOCSREDIR ioctl");
		close(cfd);
		close(consredirfd[0]);
		close(consredirfd[1]);
		consredirfd[0] = consredirfd[1] = -1;
		return -1;
	}
	close(cfd);
#else
	/* special linux workaround for a too restrictive kernel */
	if ((consredirfd[0] = OpenPTY(&slave)) < 0) {
		Msg(errno, "%s: could not open detach pty master", rc_name);
		return -1;
	}
	if ((consredirfd[1] = open(slave, O_RDWR | O_NOCTTY)) < 0) {
		Msg(errno, "%s: could not open detach pty slave", rc_name);
		close(consredirfd[0]);
		return -1;
	}
	InitTTY(&new1, 0);
	SetMode(&new1, &new2, 0, 0);
	SetTTY(consredirfd[1], &new2);
	if (UserContext() == 1)
		UserReturn(ioctl(consredirfd[1], TIOCCONS, (char *)&on));
	if (UserStatus()) {
		Msg(errno, "%s: ioctl TIOCCONS failed", rc_name);
		close(consredirfd[0]);
		close(consredirfd[1]);
		return -1;
	}
#endif
	consredir_ev.fd = consredirfd[0];
	consredir_ev.type = EV_READ;
	consredir_ev.handler = consredir_readev_fn;
	evenq(&consredir_ev);
	return 0;
}

/*
 * Read modem control lines of a physical tty and write them to buf
 * in a readable format.
 * Will not write more than 256 characters to buf.
 * Returns buf;
 */
char *TtyGetModemStatus(int fd, char *buf)
{
	char *p = buf;
#ifdef TIOCGSOFTCAR
	unsigned int softcar;
#endif
#if defined(TIOCMGET) || defined(TIOCMODG)
	unsigned int mflags;
#else
#ifdef MCGETA
	/* this is yet another interface, found on hpux. grrr */
	mflag mflags;
#if defined(MDTR)
#define TIOCM_DTR MDTR
#endif				/* MDTR */
#if defined(MRTS)
#define TIOCM_RTS MRTS
#endif				/* MRTS */
#if defined(MDSR)
#define TIOCM_DSR MDSR
#endif				/* MDSR */
#if defined(MDCD)
#define TIOCM_CAR MDCD
#endif				/* MDCD */
#if defined(MRI)
#define TIOCM_RNG MRI
#endif				/* MRI */
#if defined(MCTS)
#define TIOCM_CTS MCTS
#endif				/* MCTS */
#endif
#endif
#if defined(CLOCAL) || defined(CRTSCTS)
	struct mode mtio;	/* screen.h */
#endif
#if defined(CRTSCTS) || defined(TIOCM_CTS)
	int rtscts;
#endif
	int clocal;

#if defined(CLOCAL) || defined(CRTSCTS)
	GetTTY(fd, &mtio);
#endif
	clocal = 0;
#ifdef CLOCAL
	if (mtio.tio.c_cflag & CLOCAL) {
		clocal = 1;
		*p++ = '{';
	}
#endif

#ifdef TIOCM_CTS
#ifdef CRTSCTS
	if (!(mtio.tio.c_cflag & CRTSCTS))
		rtscts = 0;
	else
#endif				/* CRTSCTS */
		rtscts = 1;
#endif				/* TIOCM_CTS */

#ifdef TIOCGSOFTCAR
	if (ioctl(fd, TIOCGSOFTCAR, (char *)&softcar) < 0)
		softcar = 0;
#endif

#if defined(TIOCMGET) || defined(TIOCMODG) || defined(MCGETA)
#ifdef TIOCMGET
	if (ioctl(fd, TIOCMGET, (char *)&mflags) < 0)
#else
#ifdef TIOCMODG
	if (ioctl(fd, TIOCMODG, (char *)&mflags) < 0)
#else
	if (ioctl(fd, MCGETA, &mflags) < 0)
#endif
#endif
	{
#ifdef TIOCGSOFTCAR
		sprintf(p, "NO-TTY? %s", softcar ? "(CD)" : "CD");
#else
		sprintf(p, "NO-TTY?");
#endif
		p += strlen(p);
	} else {
		char *s;
#ifdef FANCY_MODEM
#ifdef TIOCM_LE
		if (!(mflags & TIOCM_LE))
			for (s = "!LE "; *s; *p++ = *s++) ;
#endif
#endif				/* FANCY_MODEM */

#ifdef TIOCM_RTS
		s = "!RTS ";
		if (mflags & TIOCM_RTS)
			s++;
		while (*s)
			*p++ = *s++;
#endif
#ifdef TIOCM_CTS
		s = "!CTS ";
		if (!rtscts) {
			*p++ = '(';
			s = "!CTS) ";
		}
		if (mflags & TIOCM_CTS)
			s++;
		while (*s)
			*p++ = *s++;
#endif

#ifdef TIOCM_DTR
		s = "!DTR ";
		if (mflags & TIOCM_DTR)
			s++;
		while (*s)
			*p++ = *s++;
#endif
#ifdef TIOCM_DSR
		s = "!DSR ";
		if (mflags & TIOCM_DSR)
			s++;
		while (*s)
			*p++ = *s++;
#endif
#if defined(TIOCM_CD) || defined(TIOCM_CAR)
		s = "!CD ";
#ifdef TIOCGSOFTCAR
		if (softcar) {
			*p++ = '(';
			s = "!CD) ";
		}
#endif
#ifdef TIOCM_CD
		if (mflags & TIOCM_CD)
			s++;
#else
		if (mflags & TIOCM_CAR)
			s++;
#endif
		while (*s)
			*p++ = *s++;
#endif
#if defined(TIOCM_RI) || defined(TIOCM_RNG)
#ifdef TIOCM_RI
		if (mflags & TIOCM_RI)
#else
		if (mflags & TIOCM_RNG)
#endif
			for (s = "RI "; *s; *p++ = *s++) ;
#endif
#ifdef FANCY_MODEM
#ifdef TIOCM_ST
		s = "!ST ";
		if (mflags & TIOCM_ST)
			s++;
		while (*s)
			*p++ = *s++;
#endif
#ifdef TIOCM_SR
		s = "!SR ";
		if (mflags & TIOCM_SR)
			s++;
		while (*s)
			*p++ = *s++;
#endif
#endif				/* FANCY_MODEM */
		if (p > buf && p[-1] == ' ')
			p--;
		*p = '\0';
	}
#else
#ifdef TIOCGSOFTCAR
	sprintf(p, " %s", softcar ? "(CD)", "CD");
	p += strlen(p);
#endif
#endif
	if (clocal)
		*p++ = '}';
	*p = '\0';
	return buf;
}

static struct baud_values btable[] = {
#if defined(B4000000)
	{4000000, B4000000},
#endif
#if defined(B3500000)
	{3500000, B3500000},
#endif
#if defined(B3000000)
	{3000000, B3000000},
#endif
#if defined(B2500000)
	{2500000, B2500000},
#endif
#if defined(B2000000)
	{2000000, B2000000},
#endif
#if defined(B1500000)
	{1500000, B1500000},
#endif
#if defined(B1152000)
	{1152000, B1152000},
#endif
#if defined(B1000000)
	{1000000, B1000000},
#endif
#if defined(B921600)
	{921600, B921600},
#endif
#if defined(B576000)
	{576000, B576000},
#endif
#if defined(B500000)
	{500000, B500000},
#endif
#if defined(B460800)
	{460800, B460800},
#endif
#if defined(B230400)
	{230400, B230400},
#endif
#if defined(B115200)
	{115200, B115200},
#endif
#if defined(B57600)
	{57600, B57600},
#endif
#if defined(EXTB)
	{38400, EXTB},
#endif
#if defined(B38400)
	{38400, B38400},
#endif
#if defined(EXTA)
	{19200, EXTA},
#endif
#if defined(B19200)
	{19200, B19200},
#endif
#if defined(B9600)
	{9600, B9600},
#endif
#if defined(B7200)
	{7200, B7200},
#endif
#if defined(B4800)
	{4800, B4800},
#endif
#if defined(B3600)
	{3600, B3600},
#endif
#if defined(B2400)
	{2400, B2400},
#endif
#if defined(B1800)
	{1800, B1800},
#endif
#if defined(B1200)
	{1200, B1200},
#endif
#if defined(B900)
	{900, B900},
#endif
#if defined(B600)
	{600, B600},
#endif
#if defined(B300)
	{300, B300},
#endif
#if defined(B200)
	{200, B200},
#endif
#if defined(B150)
	{150, B150},
#endif
#if defined(B134)
	{134, B134},
#endif
#if defined(B110)
	{110, B110},
#endif
#if defined(B75)
	{75, B75},
#endif
#if defined(B50)
	{50, B50},
#endif
#if defined(B0)
	{0, B0},
#endif
	{-1, -1}
};

/*
 * baud may either be a bits-per-second value or a symbolic
 * value as returned by cfget?speed()
 */
static struct baud_values *lookup_baud(int baud)
{
	for (struct baud_values *p = btable; p->bps >= 0; p++)
		if (baud == p->bps || baud == p->sym)
			return p;
	return NULL;
}

/*
 * change the baud rate in a mode structure.
 * ibaud and obaud are given in bit/second, or at your option as
 * termio B... symbols as defined in e.g. suns sys/ttydev.h
 * -1 means don't change.
 */
static int SetBaud(struct mode *m, int ibaud, int obaud)
{
	struct baud_values *ip, *op;

	if ((!(ip = lookup_baud(ibaud)) && ibaud != -1) || (!(op = lookup_baud(obaud)) && obaud != -1))
		return -1;

	if (ip)
		cfsetispeed(&m->tio, ip->sym);
	if (op)
		cfsetospeed(&m->tio, op->sym);
	return 0;
}

int CheckTtyname(char *tty)
{
	struct stat st;
	char realbuf[PATH_MAX];
	const char *real;
	int rc;

	real = realpath(tty, realbuf);
	if (!real)
		return -1;
	realbuf[ARRAY_SIZE(realbuf) - 1] = 0;

	if (lstat(real, &st) || !S_ISCHR(st.st_mode) || (st.st_nlink > 1 && strncmp(real, "/dev", 4)))
		rc = -1;
	else
		rc = 0;

	return rc;
}

/* len(/proc/self/fd/) + len(max 64 bit int) */
#define MAX_PTS_SYMLINK (14 + 21)
char *GetPtsPathOrSymlink(int fd)
{
	int ret;
	char *tty_name;
	static char tty_symlink[MAX_PTS_SYMLINK];

	errno = 0;
	tty_name = ttyname(fd);
	if (!tty_name && errno == ENODEV) {
		ret = snprintf(tty_symlink, MAX_PTS_SYMLINK, "/proc/self/fd/%d", fd);
		if (ret < 0 || ret >= MAX_PTS_SYMLINK)
			return NULL;
		/* We are setting errno to ENODEV to allow callers to check
		 * whether the pts device exists in another namespace.
		 */
		errno = ENODEV;
		return tty_symlink;
	}

	return tty_name;
}
