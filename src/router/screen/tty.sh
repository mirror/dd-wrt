#! /bin/sh
# sh tty.sh tty.c
# This inserts all the needed #ifdefs for IF{} statements
# and generates tty.c

#
# Stupid cpp on A/UX barfs on ``#if defined(FOO) && FOO < 17'' when 
# FOO is undefined. Reported by Robert C. Tindall (rtindall@uidaho.edu)
#
rm -f $1
sed -e '1,26d' \
-e 's%^IF{\([^}]*\)}\(.*\)%#if defined(\1)\
\2\
#endif /* \1 */%' \
-e 's%^IFN{\([^}]*\)}\(.*\)%#if !defined(\1)\
\2\
#endif /* \1 */%' \
-e 's%^XIF{\([^}]*\)}\(.*\)%#if defined(\1)\
#if (\1 < MAXCC)\
\2\
#endif \
#endif /* \1 */%' \
 < $0 > $1
chmod -w $1
exit 0

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
 */

/*
 * NOTICE: tty.c is automatically generated from tty.sh
 * Do not change anything here. If you then change tty.sh.
 */

#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef sgi
# include <sys/file.h>
#endif
#if !defined(sun) || defined(SUNOS3)
# include <sys/ioctl.h> /* collosions with termios.h */
#else
# ifndef TIOCEXCL
#  include <sys/ttold.h>	/* needed for TIOCEXCL */
# endif
#endif
#ifdef __hpux
# include <sys/modem.h>
#endif
#include <limits.h>

#ifdef ISC
# include <sys/tty.h>
# include <sys/sioctl.h>
# include <sys/pty.h>
#endif

#include "config.h"
#ifdef HAVE_STROPTS_H
#include <sys/stropts.h>	/* for I_POP */
#endif

#include "screen.h"
#include "extern.h"

#if !defined(TIOCCONS) && defined(sun) && defined(SVR4)
# include <sys/strredir.h>
#endif

extern struct display *display, *displays;
extern int iflag;
#if (!defined(TIOCCONS) && defined(SRIOCSREDIR)) || defined(linux)
extern struct win *console_window;
static void consredir_readev_fn __P((struct event *, char *));
#endif

int separate_sids = 1;

static void DoSendBreak __P((int, int, int));
static sigret_t SigAlrmDummy __P(SIGPROTOARG);


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
# define O_NOCTTY 0
#endif

#ifndef TTYVMIN
# define TTYVMIN 1
#endif
#ifndef TTYVTIME
#define TTYVTIME 0
#endif


static sigret_t
SigAlrmDummy SIGDEFARG
{
  debug("SigAlrmDummy()\n");
  SIGRETURN;
}

/*
 *  Carefully open a charcter device. Not used to open display ttys.
 *  The second parameter is parsed for a few stty style options.
 */

int
OpenTTY(line, opt)
char *line, *opt;
{
  int f;
  struct mode Mode;
  sigret_t (*sigalrm)__P(SIGPROTOARG);

  sigalrm = signal(SIGALRM, SigAlrmDummy);
  alarm(2);

  /* this open only succeeds, if real uid is allowed */
  if ((f = secopen(line, O_RDWR | O_NONBLOCK | O_NOCTTY, 0)) == -1)
    {
      if (errno == EINTR)
        Msg(0, "Cannot open line '%s' for R/W: open() blocked, aborted.", line);
      else
        Msg(errno, "Cannot open line '%s' for R/W", line);
      alarm(0);
      signal(SIGALRM, sigalrm);
      return -1;
    }
  if (!isatty(f))
    {
      Msg(0, "'%s' is not a tty", line);
      alarm(0);
      signal(SIGALRM, sigalrm);
      close(f);
      return -1;
    }
#if defined(I_POP) && defined(POP_TTYMODULES)
  debug("OpenTTY I_POP\n");
  while (ioctl(f, I_POP, (char *)0) >= 0)
    ;
#endif
  /*
   * We come here exclusively. This is to stop all kermit and cu type things
   * accessing the same tty line.
   * Perhaps we should better create a lock in some /usr/spool/locks directory?
   */
#ifdef TIOCEXCL
 errno = 0;
 if (ioctl(f, TIOCEXCL, (char *) 0) < 0)
   Msg(errno, "%s: ioctl TIOCEXCL failed", line);
 debug3("%d %d %d\n", getuid(), geteuid(), getpid());
 debug2("%s TIOCEXCL errno %d\n", line, errno);
#endif  /* TIOCEXCL */
  /*
   * We create a sane tty mode. We do not copy things from the display tty
   */
#if WE_REALLY_WANT_TO_COPY_THE_TTY_MODE
  if (display)
    {
      debug1("OpenTTY: using mode of display for %s\n", line);
      Mode = D_NewMode;
    }
  else
#endif
    InitTTY(&Mode, W_TYPE_PLAIN);
  
  SttyMode(&Mode, opt);
#ifdef DEBUG
  DebugTTY(&Mode);
#endif
  SetTTY(f, &Mode);

#if defined(linux) && defined(TIOCMSET)
  {
    int mcs = 0;
    ioctl(f, TIOCMGET, &mcs);
    mcs |= TIOCM_RTS;
    ioctl(f, TIOCMSET, &mcs);
  }
#endif

  brktty(f);
  alarm(0);
  signal(SIGALRM, sigalrm);
  debug2("'%s' CONNECT fd=%d.\n", line, f);
  return f;
}


/*
 *  Tty mode handling
 */

void
InitTTY(m, ttyflag)
struct mode *m;
int ttyflag;
{
  bzero((char *)m, sizeof(*m));
#ifdef POSIX
  /* struct termios tio 
   * defaults, as seen on SunOS 4.1.3
   */
  debug1("InitTTY: POSIX: termios defaults based on SunOS 4.1.3, but better (%d)\n", ttyflag);
IF{BRKINT}	m->tio.c_iflag |= BRKINT;
IF{IGNPAR}	m->tio.c_iflag |= IGNPAR;
/* IF{ISTRIP}	m->tio.c_iflag |= ISTRIP;  may be needed, let's try. jw. */
IF{IXON}	m->tio.c_iflag |= IXON;
/* IF{IMAXBEL}	m->tio.c_iflag |= IMAXBEL; sorry, this one is ridiculus. jw */

  if (!ttyflag)	/* may not even be good for ptys.. */
    {
IF{ICRNL}	m->tio.c_iflag |= ICRNL;
IF{ONLCR}	m->tio.c_oflag |= ONLCR; 
IF{TAB3}	m->tio.c_oflag |= TAB3; 
IF{OXTABS}      m->tio.c_oflag |= OXTABS;
/* IF{PARENB}	m->tio.c_cflag |= PARENB;	nah! jw. */
IF{OPOST}	m->tio.c_oflag |= OPOST;
    }


/*
 * Or-ing the speed into c_cflags is dangerous.
 * It breaks on bsdi, where c_ispeed and c_ospeed are extra longs.
 *
 * IF{B9600}    m->tio.c_cflag |= B9600;
 * IF{IBSHIFT) && defined(B9600}        m->tio.c_cflag |= B9600 << IBSHIFT;
 *
 * We hope that we have the posix calls to do it right:
 * If these are not available you might try the above.
 */
IF{B9600}       cfsetospeed(&m->tio, B9600);
IF{B9600}       cfsetispeed(&m->tio, B9600);

IF{CS8} 	m->tio.c_cflag |= CS8;
IF{CREAD}	m->tio.c_cflag |= CREAD;
IF{CLOCAL}	m->tio.c_cflag |= CLOCAL;

IF{ECHOCTL}	m->tio.c_lflag |= ECHOCTL;
IF{ECHOKE}	m->tio.c_lflag |= ECHOKE;

  if (!ttyflag)
    {
IF{ISIG}	m->tio.c_lflag |= ISIG;
IF{ICANON}	m->tio.c_lflag |= ICANON;
IF{ECHO}	m->tio.c_lflag |= ECHO;
    }
IF{ECHOE}	m->tio.c_lflag |= ECHOE;
IF{ECHOK}	m->tio.c_lflag |= ECHOK;
IF{IEXTEN}	m->tio.c_lflag |= IEXTEN;

XIF{VINTR}	m->tio.c_cc[VINTR]    = Ctrl('C');
XIF{VQUIT}	m->tio.c_cc[VQUIT]    = Ctrl('\\');
XIF{VERASE}	m->tio.c_cc[VERASE]   = 0x7f; /* DEL */
XIF{VKILL}	m->tio.c_cc[VKILL]    = Ctrl('U');
XIF{VEOF}	m->tio.c_cc[VEOF]     = Ctrl('D');
XIF{VEOL}	m->tio.c_cc[VEOL]     = VDISABLE;
XIF{VEOL2}	m->tio.c_cc[VEOL2]    = VDISABLE;
XIF{VSWTCH}	m->tio.c_cc[VSWTCH]   = VDISABLE;
XIF{VSTART}	m->tio.c_cc[VSTART]   = Ctrl('Q');
XIF{VSTOP}	m->tio.c_cc[VSTOP]    = Ctrl('S');
XIF{VSUSP}	m->tio.c_cc[VSUSP]    = Ctrl('Z');
XIF{VDSUSP}	m->tio.c_cc[VDSUSP]   = Ctrl('Y');
XIF{VREPRINT}	m->tio.c_cc[VREPRINT] = Ctrl('R');
XIF{VDISCARD}	m->tio.c_cc[VDISCARD] = Ctrl('O');
XIF{VWERASE}	m->tio.c_cc[VWERASE]  = Ctrl('W');
XIF{VLNEXT}	m->tio.c_cc[VLNEXT]   = Ctrl('V');
XIF{VSTATUS}	m->tio.c_cc[VSTATUS]  = Ctrl('T');

  if (ttyflag)
    {
      m->tio.c_cc[VMIN] = TTYVMIN;
      m->tio.c_cc[VTIME] = TTYVTIME;
    }

# ifdef HPUX_LTCHARS_HACK
  m->m_ltchars.t_suspc =  Ctrl('Z');
  m->m_ltchars.t_dsuspc = Ctrl('Y');
  m->m_ltchars.t_rprntc = Ctrl('R');
  m->m_ltchars.t_flushc = Ctrl('O');
  m->m_ltchars.t_werasc = Ctrl('W');
  m->m_ltchars.t_lnextc = Ctrl('V');
# endif /* HPUX_LTCHARS_HACK */

#else /* POSIX */

# ifdef TERMIO
  debug1("InitTTY: nonPOSIX, struct termio a la Motorola SYSV68 (%d)\n", ttyflag);
  /* struct termio tio 
   * defaults, as seen on Mototola SYSV68:
   * input: 7bit, CR->NL, ^S/^Q flow control 
   * output: POSTprocessing: NL->NL-CR, Tabs to spaces
   * control: 9600baud, 8bit CSIZE, enable input
   * local: enable signals, erase/kill processing, echo on.
   */
IF{ISTRIP}	m->tio.c_iflag |= ISTRIP;
IF{IXON}	m->tio.c_iflag |= IXON;

  if (!ttyflag)	/* may not even be good for ptys.. */
    {
IF{OPOST}	m->tio.c_oflag |= OPOST;
IF{ICRNL}	m->tio.c_iflag |= ICRNL;
IF{ONLCR}	m->tio.c_oflag |= ONLCR;
IF{TAB3}	m->tio.c_oflag |= TAB3;
    }

#ifdef __bsdi__
		)-: cannot handle BSDI without POSIX
#else
IF{B9600}	m->tio.c_cflag  = B9600;
#endif
IF{CS8} 	m->tio.c_cflag |= CS8;
IF{CREAD}	m->tio.c_cflag |= CREAD;

  if (!ttyflag)
    {
IF{ISIG}	m->tio.c_lflag |= ISIG;
IF{ICANON}	m->tio.c_lflag |= ICANON;
IF{ECHO}	m->tio.c_lflag |= ECHO;
    }
IF{ECHOE}	m->tio.c_lflag |= ECHOE;
IF{ECHOK}	m->tio.c_lflag |= ECHOK;

XIF{VINTR}	m->tio.c_cc[VINTR]  = Ctrl('C');
XIF{VQUIT}	m->tio.c_cc[VQUIT]  = Ctrl('\\');
XIF{VERASE}	m->tio.c_cc[VERASE] = 0177; /* DEL */
XIF{VKILL}	m->tio.c_cc[VKILL]  = Ctrl('U');
XIF{VEOF}	m->tio.c_cc[VEOF]   = Ctrl('D');
XIF{VEOL}	m->tio.c_cc[VEOL]   = VDISABLE;
XIF{VEOL2}	m->tio.c_cc[VEOL2]  = VDISABLE;
XIF{VSWTCH}	m->tio.c_cc[VSWTCH] = VDISABLE;

  if (ttyflag)
   {
      m->tio.c_cc[VMIN] = TTYVMIN;
      m->tio.c_cc[VTIME] = TTYVTIME;
    } 

# else /* TERMIO */
  debug1("InitTTY: BSD: defaults a la SunOS 4.1.3 (%d)\n", ttyflag);
  m->m_ttyb.sg_ispeed = B9600;
  m->m_ttyb.sg_ospeed = B9600;
  m->m_ttyb.sg_erase  = 0177; /*DEL */
  m->m_ttyb.sg_kill   = Ctrl('U');
  if (!ttyflag)
    m->m_ttyb.sg_flags = CRMOD | ECHO
IF{ANYP}	| ANYP
    ;
  else
    m->m_ttyb.sg_flags = CBREAK
IF{ANYP}	| ANYP
    ;

  m->m_tchars.t_intrc   = Ctrl('C');
  m->m_tchars.t_quitc   = Ctrl('\\');
  m->m_tchars.t_startc  = Ctrl('Q');
  m->m_tchars.t_stopc   = Ctrl('S');
  m->m_tchars.t_eofc    = Ctrl('D');
  m->m_tchars.t_brkc    = -1;

  m->m_ltchars.t_suspc  = Ctrl('Z');
  m->m_ltchars.t_dsuspc = Ctrl('Y');
  m->m_ltchars.t_rprntc = Ctrl('R');
  m->m_ltchars.t_flushc = Ctrl('O');
  m->m_ltchars.t_werasc = Ctrl('W');
  m->m_ltchars.t_lnextc = Ctrl('V');

IF{NTTYDISC}	m->m_ldisc = NTTYDISC;

  m->m_lmode = 0
IF{LDECCTQ}	| LDECCTQ
IF{LCTLECH}	| LCTLECH
IF{LPASS8}	| LPASS8
IF{LCRTKIL}	| LCRTKIL
IF{LCRTERA}	| LCRTERA
IF{LCRTBS}	| LCRTBS
  ;
# endif /* TERMIO */
#endif /* POSIX */

#if defined(ENCODINGS) && defined(TIOCKSET)
  m->m_jtchars.t_ascii = 'J';
  m->m_jtchars.t_kanji = 'B';
  m->m_knjmode = KM_ASCII | KM_SYSSJIS;
#endif
}

void 
SetTTY(fd, mp)
int fd;
struct mode *mp;
{
  errno = 0;
#ifdef POSIX
  tcsetattr(fd, TCSADRAIN, &mp->tio);
# ifdef HPUX_LTCHARS_HACK
  ioctl(fd, TIOCSLTC, (char *)&mp->m_ltchars);
# endif
#else
# ifdef TERMIO
  ioctl(fd, TCSETAW, (char *)&mp->tio);
#  ifdef CYTERMIO
  if (mp->tio.c_line == 3)
    {
      ioctl(fd, LDSETMAPKEY, (char *)&mp->m_mapkey);
      ioctl(fd, LDSETMAPSCREEN, (char *)&mp->m_mapscreen);
      ioctl(fd, LDSETBACKSPACE, (char *)&mp->m_backspace);
    }
#  endif
# else
  /* ioctl(fd, TIOCSETP, (char *)&mp->m_ttyb); */
  ioctl(fd, TIOCSETC, (char *)&mp->m_tchars);
  ioctl(fd, TIOCLSET, (char *)&mp->m_lmode);
  ioctl(fd, TIOCSETD, (char *)&mp->m_ldisc);
  ioctl(fd, TIOCSETP, (char *)&mp->m_ttyb);
  ioctl(fd, TIOCSLTC, (char *)&mp->m_ltchars); /* moved here for apollo. jw */
# endif
#endif
#if defined(ENCODINGS) && defined(TIOCKSET)
  ioctl(fd, TIOCKSETC, &mp->m_jtchars);
  ioctl(fd, TIOCKSET, &mp->m_knjmode);
#endif
  if (errno)
    Msg(errno, "SetTTY (fd %d): ioctl failed", fd);
}

void
GetTTY(fd, mp)
int fd;
struct mode *mp;
{
  errno = 0;
#ifdef POSIX
  tcgetattr(fd, &mp->tio);
# ifdef HPUX_LTCHARS_HACK
  ioctl(fd, TIOCGLTC, (char *)&mp->m_ltchars);
# endif
#else
# ifdef TERMIO
  ioctl(fd, TCGETA, (char *)&mp->tio);
#  ifdef CYTERMIO
  if (mp->tio.c_line == 3)
    {
      ioctl(fd, LDGETMAPKEY, (char *)&mp->m_mapkey);
      ioctl(fd, LDGETMAPSCREEN, (char *)&mp->m_mapscreen);
      ioctl(fd, LDGETBACKSPACE, (char *)&mp->m_backspace);
    }
  else
    {
      mp->m_mapkey = NOMAPKEY;
      mp->m_mapscreen = NOMAPSCREEN;
      mp->m_backspace = '\b';
    }
#  endif
# else
  ioctl(fd, TIOCGETP, (char *)&mp->m_ttyb);
  ioctl(fd, TIOCGETC, (char *)&mp->m_tchars);
  ioctl(fd, TIOCGLTC, (char *)&mp->m_ltchars);
  ioctl(fd, TIOCLGET, (char *)&mp->m_lmode);
  ioctl(fd, TIOCGETD, (char *)&mp->m_ldisc);
# endif
#endif
#if defined(ENCODINGS) && defined(TIOCKSET)
  ioctl(fd, TIOCKGETC, &mp->m_jtchars);
  ioctl(fd, TIOCKGET, &mp->m_knjmode);
#endif
  if (errno)
    Msg(errno, "GetTTY (fd %d): ioctl failed", fd);
}

/*
 * needs interrupt = iflag and flow = d->d_flow
 */
void
SetMode(op, np, flow, interrupt)
struct mode *op, *np;
int flow, interrupt;
{
  *np = *op;

  ASSERT(display);
#if defined(TERMIO) || defined(POSIX)
# ifdef CYTERMIO
  np->m_mapkey = NOMAPKEY;
  np->m_mapscreen = NOMAPSCREEN;
  np->tio.c_line = 0;
# endif
IF{ICRNL}  np->tio.c_iflag &= ~ICRNL;
IF{ISTRIP}  np->tio.c_iflag &= ~ISTRIP;
IF{ONLCR}  np->tio.c_oflag &= ~ONLCR;
  np->tio.c_lflag &= ~(ICANON | ECHO);
  /*
   * From Andrew Myers (andru@tonic.lcs.mit.edu)
   * to avoid ^V^V-Problem on OSF1
   */
IF{IEXTEN}  np->tio.c_lflag &= ~IEXTEN;

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
  if (flow == 0)
    {
XIF{VSTART}	np->tio.c_cc[VSTART] = VDISABLE;
XIF{VSTOP}	np->tio.c_cc[VSTOP] = VDISABLE;
      np->tio.c_iflag &= ~IXON;
    }
XIF{VDISCARD}	np->tio.c_cc[VDISCARD] = VDISABLE;
XIF{VLNEXT}	np->tio.c_cc[VLNEXT] = VDISABLE;
XIF{VSTATUS}	np->tio.c_cc[VSTATUS] = VDISABLE;
XIF{VSUSP}	np->tio.c_cc[VSUSP] = VDISABLE;
 /* Set VERASE to DEL, rather than VDISABLE, to avoid libvte
    "autodetect" issues. */
XIF{VERASE}	np->tio.c_cc[VERASE] = 0x7f;
XIF{VKILL}	np->tio.c_cc[VKILL] = VDISABLE;
# ifdef HPUX_LTCHARS_HACK
  np->m_ltchars.t_suspc  = VDISABLE;
  np->m_ltchars.t_dsuspc = VDISABLE;
  np->m_ltchars.t_rprntc = VDISABLE;
  np->m_ltchars.t_flushc = VDISABLE;
  np->m_ltchars.t_werasc = VDISABLE;
  np->m_ltchars.t_lnextc = VDISABLE;
# else /* HPUX_LTCHARS_HACK */
XIF{VDSUSP}	np->tio.c_cc[VDSUSP] = VDISABLE;
XIF{VREPRINT}	np->tio.c_cc[VREPRINT] = VDISABLE;
XIF{VWERASE}	np->tio.c_cc[VWERASE] = VDISABLE;
# endif /* HPUX_LTCHARS_HACK */
#else /* TERMIO || POSIX */
  if (!interrupt || !flow)
    np->m_tchars.t_intrc = -1;
  np->m_ttyb.sg_flags &= ~(CRMOD | ECHO);
  np->m_ttyb.sg_flags |= CBREAK;
# if defined(CYRILL) && defined(CSTYLE) && defined(CS_8BITS)
  np->m_ttyb.sg_flags &= ~CSTYLE;
  np->m_ttyb.sg_flags |= CS_8BITS;
# endif
  np->m_tchars.t_quitc = -1;
  if (flow == 0)
    {
      np->m_tchars.t_startc = -1;
      np->m_tchars.t_stopc = -1;
    }
  np->m_ltchars.t_suspc = -1;
  np->m_ltchars.t_dsuspc = -1;
  np->m_ltchars.t_flushc = -1;
  np->m_ltchars.t_lnextc = -1;
#endif /* defined(TERMIO) || defined(POSIX) */
}

/* operates on display */
void
SetFlow(on)
int on;
{
  ASSERT(display);
  if (D_flow == on)
    return;
#if defined(TERMIO) || defined(POSIX)
  if (on)
    {
      D_NewMode.tio.c_cc[VINTR] = iflag ? D_OldMode.tio.c_cc[VINTR] : VDISABLE;
XIF{VSTART}	D_NewMode.tio.c_cc[VSTART] = D_OldMode.tio.c_cc[VSTART];
XIF{VSTOP}	D_NewMode.tio.c_cc[VSTOP] = D_OldMode.tio.c_cc[VSTOP];
      D_NewMode.tio.c_iflag |= D_OldMode.tio.c_iflag & IXON;
    }
  else
    {
      D_NewMode.tio.c_cc[VINTR] = VDISABLE;
XIF{VSTART}	D_NewMode.tio.c_cc[VSTART] = VDISABLE;
XIF{VSTOP}	D_NewMode.tio.c_cc[VSTOP] = VDISABLE;
      D_NewMode.tio.c_iflag &= ~IXON;
    }
# ifdef POSIX
#  ifdef TCOON
  if (!on)
    tcflow(D_userfd, TCOON);
#  endif
  if (tcsetattr(D_userfd, TCSANOW, &D_NewMode.tio))
# else
  if (ioctl(D_userfd, TCSETAW, (char *)&D_NewMode.tio) != 0)
# endif
    debug1("SetFlow: ioctl errno %d\n", errno);
#else /* POSIX || TERMIO */
  if (on)
    {
      D_NewMode.m_tchars.t_intrc = iflag ? D_OldMode.m_tchars.t_intrc : -1;
      D_NewMode.m_tchars.t_startc = D_OldMode.m_tchars.t_startc;
      D_NewMode.m_tchars.t_stopc = D_OldMode.m_tchars.t_stopc;
    }
  else
    {
      D_NewMode.m_tchars.t_intrc = -1;
      D_NewMode.m_tchars.t_startc = -1;
      D_NewMode.m_tchars.t_stopc = -1;
    }
  if (ioctl(D_userfd, TIOCSETC, (char *)&D_NewMode.m_tchars) != 0)
    debug1("SetFlow: ioctl errno %d\n", errno);
#endif /* POSIX || TERMIO */
  D_flow = on;
}

/* parse commands from opt and modify m */
int
SttyMode(m, opt)
struct mode *m;
char *opt;
{
  static const char sep[] = " \t:;,";

  if (!opt)
    return 0;

  while (*opt)
    {
      while (index(sep, *opt)) opt++;
      if (*opt >= '0' && *opt <= '9')
        {
	  if (SetBaud(m, atoi(opt), atoi(opt)))
	    return -1;
	}
      else if (!strncmp("cs7", opt, 3))
        {
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_cflag &= ~CSIZE;
	  m->tio.c_cflag |= CS7;
#else
	  m->m_lmode &= ~LPASS8;
#endif
	}
      else if (!strncmp("cs8", opt, 3))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_cflag &= ~CSIZE;
	  m->tio.c_cflag |= CS8;
#else
	  m->m_lmode |= LPASS8;
#endif
	}
      else if (!strncmp("istrip", opt, 6))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_iflag |= ISTRIP;
#else
	  m->m_lmode &= ~LPASS8;
#endif
        }
      else if (!strncmp("-istrip", opt, 7))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_iflag &= ~ISTRIP;
#else
	  m->m_lmode |= LPASS8;
#endif
        }
      else if (!strncmp("ixon", opt, 4))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_iflag |= IXON;
#else
	  debug("SttyMode: no ixon in old bsd land.\n");
#endif
        }
      else if (!strncmp("-ixon", opt, 5))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_iflag &= ~IXON;
#else
	  debug("SttyMode: no -ixon in old bsd land.\n");
#endif
        }
      else if (!strncmp("ixoff", opt, 5))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_iflag |= IXOFF;
#else
	  m->m_ttyb.sg_flags |= TANDEM;
#endif
        }
      else if (!strncmp("-ixoff", opt, 6))
	{
#if defined(POSIX) || defined(TERMIO)
	  m->tio.c_iflag &= ~IXOFF;
#else
	  m->m_ttyb.sg_flags &= ~TANDEM;
#endif
        }
      else if (!strncmp("crtscts", opt, 7))
	{
#if (defined(POSIX) || defined(TERMIO)) && defined(CRTSCTS)
	  m->tio.c_cflag |= CRTSCTS;
#endif
	}
      else if (!strncmp("-crtscts", opt, 8))
        {
#if (defined(POSIX) || defined(TERMIO)) && defined(CRTSCTS)
	  m->tio.c_cflag &= ~CRTSCTS;
#endif
	}
      else
        return -1;
      while (*opt && !index(sep, *opt)) opt++;
    }
  return 0;
}

/*
 *  Job control handling
 *
 *  Somehow the ultrix session handling is broken, so use
 *  the bsdish variant.
 */

/*ARGSUSED*/
void
brktty(fd)
int fd;
{
#if defined(POSIX) && !defined(ultrix)
  if (separate_sids)
    setsid();		/* will break terminal affiliation */
  /* GNU added for Hurd systems 2001-10-10 */
# if defined(BSD) && defined(TIOCSCTTY) && !defined(__GNU__)
  ioctl(fd, TIOCSCTTY, (char *)0);
# endif /* BSD && TIOCSCTTY */
#else /* POSIX */
# ifdef SYSV
  if (separate_sids)
    setpgrp();		/* will break terminal affiliation */
# else /* SYSV */
#  ifdef BSDJOBS
  int devtty;

  if ((devtty = open("/dev/tty", O_RDWR | O_NONBLOCK)) >= 0)
    {
      if (ioctl(devtty, TIOCNOTTY, (char *)0))
        debug2("brktty: ioctl(devtty=%d, TIOCNOTTY, 0) = %d\n", devtty, errno);
      close(devtty);
    }
#  endif /* BSDJOBS */
# endif /* SYSV */
#endif /* POSIX */
}

int
fgtty(fd)
int fd;
{
#ifdef BSDJOBS
  int mypid;

  mypid = getpid();

  /*
   * Under BSD we have to set the controlling terminal again explicitly.
   */
# if (defined(__FreeBSD_kernel__) || defined(__DragonFly__) || defined(__GNU__)) && defined(TIOCSCTTY)
  ioctl(fd, TIOCSCTTY, (char *)0);
# endif

# ifdef POSIX
  if (separate_sids)
    if (tcsetpgrp(fd, mypid))
      {
        debug1("fgtty: tcsetpgrp: %d\n", errno);
        return -1;
      }
# else /* POSIX */
  if (ioctl(fd, TIOCSPGRP, (char *)&mypid) != 0)
    debug1("fgtty: TIOSETPGRP: %d\n", errno);
#  ifndef SYSV	/* Already done in brktty():setpgrp() */
  if (separate_sids)
    if (setpgrp(fd, mypid))
      debug1("fgtty: setpgrp: %d\n", errno);
#  endif
# endif /* POSIX */
#endif /* BSDJOBS */
  return 0;
}

/* 
 * The alm boards on our sparc center 1000 have a lousy driver.
 * We cannot generate long breaks unless we use the most ugly form
 * of ioctls. jw.
 */
#ifdef POSIX
int breaktype = 2;
#else /* POSIX */
# ifdef TCSBRK
int breaktype = 1;
# else
int breaktype = 0;
# endif
#endif /* POSIX */

#if defined(sun) && !defined(SVR4)
# define HAVE_SUPER_TCSENDBREAK
#endif

/*
 * type:
 *  0:	TIOCSBRK / TIOCCBRK
 *  1:	TCSBRK
 *  2:	tcsendbreak()
 * n: approximate duration in 1/4 seconds.
 */
static void
DoSendBreak(fd, n, type)
int fd, n, type;
{
  switch (type)
    {
    case 2:	/* tcsendbreak() =============================== */
#ifdef POSIX
# ifdef HAVE_SUPER_TCSENDBREAK
      /* There is one rare case that I have tested, where tcsendbreak works
       * really great: this was an alm driver that came with SunOS 4.1.3
       * If you have this one, define the above symbol.
       * here we can use the second parameter to specify the duration.
       */
      debug2("tcsendbreak(fd=%d, %d)\n", fd, n);
      if (tcsendbreak(fd, n) < 0)
        Msg(errno, "cannot send BREAK (tcsendbreak)");
# else
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
      debug2("%d * tcsendbreak(fd=%d, 0)\n", n, fd);
	{
	  int i;

	  if (!n)
	    n++;
	  for (i = 0; i < n; i++)
	    if (tcsendbreak(fd, 0) < 0)
	      {
		Msg(errno, "cannot send BREAK (tcsendbreak SVR4)");
		return;
	      }
	}
# endif
#else /* POSIX */
      Msg(0, "tcsendbreak() not available, change breaktype");
#endif /* POSIX */
      break;

    case 1:	/* TCSBRK ======================================= */
#ifdef TCSBRK
      if (!n)
        n++;
      /*
       * Here too, we assume that short breaks can be concatenated to 
       * perform long breaks. But for SOLARIS, this is not true, of course.
       */
      debug2("%d * TCSBRK fd=%d\n", n, fd);
	{
	  int i;

	  for (i = 0; i < n; i++)
	    if (ioctl(fd, TCSBRK, (char *)0) < 0)
	      {
		Msg(errno, "Cannot send BREAK (TCSBRK)");
		return;
	      }
	}
#else /* TCSBRK */
      Msg(0, "TCSBRK not available, change breaktype");
#endif /* TCSBRK */
      break;

    case 0:	/* TIOCSBRK / TIOCCBRK ========================== */
#if defined(TIOCSBRK) && defined(TIOCCBRK)
      /*
       * This is very rude. Screen actively celebrates the break.
       * But it may be the only save way to issue long breaks.
       */
      debug("TIOCSBRK TIOCCBRK\n");
      if (ioctl(fd, TIOCSBRK, (char *)0) < 0)
        {
	  Msg(errno, "Can't send BREAK (TIOCSBRK)");
	  return;
	}
      sleep1000(n ? n * 250 : 250);
      if (ioctl(fd, TIOCCBRK, (char *)0) < 0)
        {
	  Msg(errno, "BREAK stuck!!! -- HELP! (TIOCCBRK)");
	  return;
	}
#else /* TIOCSBRK && TIOCCBRK */
      Msg(0, "TIOCSBRK/CBRK not available, change breaktype");
#endif /* TIOCSBRK && TIOCCBRK */
      break;

    default:	/* unknown ========================== */
      Msg(0, "Internal SendBreak error: method %d unknown", type);
    }
}

/* 
 * Send a break for n * 0.25 seconds. Tty must be PLAIN.
 * The longest possible break allowed here is 15 seconds.
 */

void 
SendBreak(wp, n, closeopen)
struct win *wp;
int n, closeopen;
{
  sigret_t (*sigalrm)__P(SIGPROTOARG);

#ifdef BUILTIN_TELNET
  if (wp->w_type == W_TYPE_TELNET)
    {
      TelBreak(wp);
      return;
    }
#endif
  if (wp->w_type != W_TYPE_PLAIN)
    return;

  debug3("break(%d, %d) fd %d\n", n, closeopen, wp->w_ptyfd);

#ifdef POSIX
  (void) tcflush(wp->w_ptyfd, TCIOFLUSH);
#else
# ifdef TIOCFLUSH
  (void) ioctl(wp->w_ptyfd, TIOCFLUSH, (char *)0);
# endif /* TIOCFLUSH */
#endif /* POSIX */

  if (closeopen)
    {
      close(wp->w_ptyfd);
      sleep1000(n ? n * 250 : 250);
      if ((wp->w_ptyfd = OpenTTY(wp->w_tty, wp->w_cmdargs[1])) < 1)
	{
	  Msg(0, "Ouch, cannot reopen line %s, please try harder", wp->w_tty);
	  return;
	}
      (void) fcntl(wp->w_ptyfd, F_SETFL, FNBLOCK);
    }
  else
    {
      sigalrm = signal(SIGALRM, SigAlrmDummy);
      alarm(15);

      DoSendBreak(wp->w_ptyfd, n, breaktype);

      alarm(0);
      signal(SIGALRM, sigalrm);
    }
  debug("            broken.\n");
}

/*
 *  Console grabbing
 */

#if (!defined(TIOCCONS) && defined(SRIOCSREDIR)) || defined(linux)

static struct event consredir_ev;
static int consredirfd[2] = {-1, -1};

static void
consredir_readev_fn(ev, data)
struct event *ev;
char *data;
{
  char *p, *n, buf[256];
  int l;

  if (!console_window || (l = read(consredirfd[0], buf, sizeof(buf))) <= 0)
    {
      close(consredirfd[0]);
      close(consredirfd[1]);
      consredirfd[0] = consredirfd[1] = -1;
      evdeq(ev);
      return;
    }
  for (p = n = buf; l > 0; n++, l--)
    if (*n == '\n')
      {
        if (n > p)
	  WriteString(console_window, p, n - p);
        WriteString(console_window, "\r\n", 2);
        p = n + 1;
      }
  if (n > p)
    WriteString(console_window, p, n - p);
}

#endif

/*ARGSUSED*/
int
TtyGrabConsole(fd, on, rc_name)
int fd, on;
char *rc_name;
{
#if defined(TIOCCONS) && !defined(linux)
  struct display *d;
  int ret = 0;
  int sfd = -1;

  if (on < 0)
    return 0;		/* pty close will ungrab */
  if (on)
    {
      if (displays == 0)
	{
	  Msg(0, "I need a display");
	  return -1;
	}
      for (d = displays; d; d = d->d_next)
	if (strcmp(d->d_usertty, "/dev/console") == 0)
	  break;
      if (d)
	{
	  Msg(0, "too dangerous - screen is running on /dev/console");
	  return -1;
	}
    }

  if (!on)
    {
      char *slave;
      if ((fd = OpenPTY(&slave)) < 0)
	{
	  Msg(errno, "%s: could not open detach pty master", rc_name);
	  return -1;
	}
      if ((sfd = open(slave, O_RDWR | O_NOCTTY)) < 0)
	{
	  Msg(errno, "%s: could not open detach pty slave", rc_name);
	  close(fd);
	  return -1;
	}
    }
  if (UserContext() == 1)
    UserReturn(ioctl(fd, TIOCCONS, (char *)&on));
  ret = UserStatus();
  if (ret)
    Msg(errno, "%s: ioctl TIOCCONS failed", rc_name);
  if (!on)
    {
      close(sfd);
      close(fd);
    }
  return ret;

#else
# if defined(SRIOCSREDIR) || defined(linux)
  struct display *d;
#  ifdef SRIOCSREDIR
  int cfd;
#  else
  struct mode new1, new2;
  char *slave;
#  endif

  if (on > 0)
    {
      if (displays == 0)
	{
	  Msg(0, "I need a display");
	  return -1;
	}
      for (d = displays; d; d = d->d_next)
	if (strcmp(d->d_usertty, "/dev/console") == 0)
	  break;
      if (d)
	{
	  Msg(0, "too dangerous - screen is running on /dev/console");
	  return -1;
	}
    }
  if (consredirfd[0] >= 0)
    {
      evdeq(&consredir_ev);
      close(consredirfd[0]);
      close(consredirfd[1]);
      consredirfd[0] = consredirfd[1] = -1;
    }
  if (on <= 0)
    return 0;
#  ifdef SRIOCSREDIR
  if ((cfd = secopen("/dev/console", O_RDWR|O_NOCTTY, 0)) == -1)
    {
      Msg(errno, "/dev/console");
      return -1;
    }
  if (pipe(consredirfd))
    {
      Msg(errno, "pipe");
      close(cfd);
      consredirfd[0] = consredirfd[1] = -1;
      return -1;
    }
  if (ioctl(cfd, SRIOCSREDIR, consredirfd[1]))
    {
      Msg(errno, "SRIOCSREDIR ioctl");
      close(cfd);
      close(consredirfd[0]);
      close(consredirfd[1]);
      consredirfd[0] = consredirfd[1] = -1;
      return -1;
    }
  close(cfd);
#  else
  /* special linux workaround for a too restrictive kernel */
  if ((consredirfd[0] = OpenPTY(&slave)) < 0)
    {
      Msg(errno, "%s: could not open detach pty master", rc_name);
      return -1;
    }
  if ((consredirfd[1] = open(slave, O_RDWR | O_NOCTTY)) < 0)
    {
      Msg(errno, "%s: could not open detach pty slave", rc_name);
      close(consredirfd[0]);
      return -1;
    }
  InitTTY(&new1, 0);
  SetMode(&new1, &new2, 0, 0);
  SetTTY(consredirfd[1], &new2);
  if (UserContext() == 1)
    UserReturn(ioctl(consredirfd[1], TIOCCONS, (char *)&on));
  if (UserStatus())
    {
      Msg(errno, "%s: ioctl TIOCCONS failed", rc_name);
      close(consredirfd[0]);
      close(consredirfd[1]);
      return -1;
    }
#  endif
  consredir_ev.fd = consredirfd[0];
  consredir_ev.type = EV_READ;
  consredir_ev.handler = consredir_readev_fn;
  evenq(&consredir_ev);
  return 0;
# else
  if (on > 0)
    Msg(0, "%s: don't know how to grab the console", rc_name);
  return -1;
# endif
#endif
}

/*
 * Read modem control lines of a physical tty and write them to buf
 * in a readable format.
 * Will not write more than 256 characters to buf.
 * Returns buf;
 */
char *
TtyGetModemStatus(fd, buf)
int fd;
char *buf;
{
  char *p = buf;
#ifdef TIOCGSOFTCAR
  unsigned int softcar;
#endif
#if defined(TIOCMGET) || defined(TIOCMODG)
  unsigned int mflags;
#else
# ifdef MCGETA
  /* this is yet another interface, found on hpux. grrr */
  mflag mflags;
IF{MDTR}#  define TIOCM_DTR MDTR
IF{MRTS}#  define TIOCM_RTS MRTS
IF{MDSR}#  define TIOCM_DSR MDSR
IF{MDCD}#  define TIOCM_CAR MDCD
IF{MRI}#  define TIOCM_RNG MRI
IF{MCTS}#  define TIOCM_CTS MCTS
# endif
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
  if (mtio.tio.c_cflag & CLOCAL)
    {
      clocal = 1;
      *p++ = '{';
    }
#endif

#ifdef TIOCM_CTS
# ifdef CRTSCTS
  if (!(mtio.tio.c_cflag & CRTSCTS))
    rtscts = 0;
  else
# endif /* CRTSCTS */
    rtscts = 1;
#endif /* TIOCM_CTS */

#ifdef TIOCGSOFTCAR
  if (ioctl(fd, TIOCGSOFTCAR, (char *)&softcar) < 0)
    softcar = 0;
#endif

#if defined(TIOCMGET) || defined(TIOCMODG) || defined(MCGETA)
# ifdef TIOCMGET
  if (ioctl(fd, TIOCMGET, (char *)&mflags) < 0)
# else
#  ifdef TIOCMODG
  if (ioctl(fd, TIOCMODG, (char *)&mflags) < 0)
#  else
  if (ioctl(fd, MCGETA, &mflags) < 0)
#  endif
# endif
    {
#ifdef TIOCGSOFTCAR
      sprintf(p, "NO-TTY? %s", softcar ? "(CD)" : "CD");
#else
      sprintf(p, "NO-TTY?");
#endif
      p += strlen(p);
    }
  else
    {
      char *s;
# ifdef FANCY_MODEM
#  ifdef TIOCM_LE
      if (!(mflags & TIOCM_LE))
        for (s = "!LE "; *s; *p++ = *s++);
#  endif
# endif /* FANCY_MODEM */

# ifdef TIOCM_RTS
      s = "!RTS "; if (mflags & TIOCM_RTS) s++;
      while (*s) *p++ = *s++;
# endif
# ifdef TIOCM_CTS
      s = "!CTS "; 
      if (!rtscts)
        {
          *p++ = '(';
          s = "!CTS) "; 
	}
      if (mflags & TIOCM_CTS) s++;
      while (*s) *p++ = *s++;
# endif

# ifdef TIOCM_DTR
      s = "!DTR "; if (mflags & TIOCM_DTR) s++;
      while (*s) *p++ = *s++;
# endif
# ifdef TIOCM_DSR
      s = "!DSR "; if (mflags & TIOCM_DSR) s++;
      while (*s) *p++ = *s++;
# endif
# if defined(TIOCM_CD) || defined(TIOCM_CAR)
      s = "!CD "; 
#  ifdef TIOCGSOFTCAR
      if (softcar)
	 {
	  *p++ = '(';
	  s = "!CD) ";
	 }
#  endif
#  ifdef TIOCM_CD
      if (mflags & TIOCM_CD) s++;
#  else
      if (mflags & TIOCM_CAR) s++;
#  endif
      while (*s) *p++ = *s++;
# endif
# if defined(TIOCM_RI) || defined(TIOCM_RNG)
#  ifdef TIOCM_RI
      if (mflags & TIOCM_RI)
#  else
      if (mflags & TIOCM_RNG)
#  endif
	for (s = "RI "; *s; *p++ = *s++);
# endif
# ifdef FANCY_MODEM
#  ifdef TIOCM_ST
      s = "!ST "; if (mflags & TIOCM_ST) s++;
      while (*s) *p++ = *s++;
#  endif
#  ifdef TIOCM_SR
      s = "!SR "; if (mflags & TIOCM_SR) s++;
      while (*s) *p++ = *s++;
#  endif
# endif /* FANCY_MODEM */
      if (p > buf && p[-1] == ' ')
        p--;
      *p = '\0';
    }
#else
# ifdef TIOCGSOFTCAR
  sprintf(p, " %s", softcar ? "(CD)", "CD");
  p += strlen(p);
# endif
#endif
  if (clocal)
    *p++ = '}';
  *p = '\0';
  return buf;
}

/*
 * Old bsd-ish machines may not have any of the baudrate B... symbols.
 * We hope to detect them here, so that the btable[] below always has
 * many entries.
 */
#ifndef POSIX
# ifndef TERMIO
#  if !defined(B9600) && !defined(B2400) && !defined(B1200) && !defined(B300)
IFN{B0}#define		B0	0
IFN{B50}#define  	B50	1
IFN{B75}#define  	B75	2
IFN{B110}#define 	B110	3
IFN{B134}#define 	B134	4
IFN{B150}#define 	B150	5
IFN{B200}#define 	B200	6
IFN{B300}#define 	B300	7
IFN{B600}#define 	B600	8
IFN{B1200}#define	B1200	9
IFN{B1800}#define	B1800	10
IFN{B2400}#define	B2400	11
IFN{B4800}#define	B4800	12
IFN{B9600}#define	B9600	13
IFN{EXTA}#define	EXTA	14
IFN{EXTB}#define	EXTB	15
#  endif
# endif
#endif

/*
 * On hpux, idx and sym will be different. 
 * Rumor has it that, we need idx in D_dospeed to make tputs
 * padding correct. 
 * Frequently used entries come first.
 */
static struct baud_values btable[] =
{
IF{B4000000}	{	33,	4000000,	B4000000},
IF{B3500000}	{	32,	3500000,	B3500000},
IF{B3000000}	{	31,	3000000,	B3000000},
IF{B2500000}	{	30,	2500000,	B2500000},
IF{B2000000}	{	29,	2000000,	B2000000},
IF{B1500000}	{	28,	1500000,	B1500000},
IF{B1152000}	{	27,	1152000,	B1152000},
IF{B1000000}	{	26,	1000000,	B1000000},
IF{B921600}	{	25,	921600,		B921600	},
IF{B576000}	{	24,	576000,		B576000	},
IF{B500000}	{	23,	500000,		B500000	},
IF{B460800}	{	22,	460800,		B460800	},
IF{B230400}	{	21,	230400,		B230400	},
IF{B115200}	{	20,	115200,		B115200	},
IF{B57600}	{	19,	57600,		B57600	},
IF{EXTB}	{	18,	38400,		EXTB	},
IF{B38400}	{	18,	38400,		B38400	},
IF{EXTA}	{	17,	19200,		EXTA	},
IF{B19200}	{	17,	19200,		B19200	},
IF{B9600}	{	16,	9600,		B9600	},
IF{B7200}	{	15,	7200,		B7200	},
IF{B4800}	{	14,	4800,		B4800	},
IF{B3600}	{	13,	3600,		B3600	},
IF{B2400}	{	12,	2400,		B2400	},
IF{B1800}	{	11,	1800,		B1800	},
IF{B1200}	{	10,	1200,		B1200	},
IF{B900} 	{	9,	900,		B900	},
IF{B600} 	{	8,	600,		B600	},
IF{B300} 	{	7,	300, 		B300	},
IF{B200} 	{	6,	200, 		B200	},
IF{B150} 	{	5,	150,		B150	},
IF{B134} 	{	4,	134,		B134	},
IF{B110} 	{	3,	110,		B110	},
IF{B75}  	{	2,	75,		B75	},
IF{B50}  	{	1,	50,		B50	},
IF{B0}   	{	0,	0,		B0	},
		{	-1,	-1,		-1	}
};

/*
 * baud may either be a bits-per-second value or a symbolic
 * value as returned by cfget?speed() 
 */
struct baud_values *
lookup_baud(baud)
int baud;
{
  struct baud_values *p;

  for (p = btable; p->idx >= 0; p++)
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
int
SetBaud(m, ibaud, obaud)
struct mode *m;
int ibaud, obaud;
{
  struct baud_values *ip, *op;

  if ((!(ip = lookup_baud(ibaud)) && ibaud != -1) ||
      (!(op = lookup_baud(obaud)) && obaud != -1))
    return -1;

#ifdef POSIX
  if (ip) cfsetispeed(&m->tio, ip->sym);
  if (op) cfsetospeed(&m->tio, op->sym);
#else /* POSIX */
# ifdef TERMIO
  if (ip)
    {
#  ifdef IBSHIFT
      m->tio.c_cflag &= ~(CBAUD << IBSHIFT);
      m->tio.c_cflag |= (ip->sym & CBAUD) << IBSHIFT;
#  else /* IBSHIFT */
      if (ibaud != obaud)
        return -1;
#  endif /* IBSHIFT */
    }
  if (op)
    {
      m->tio.c_cflag &= ~CBAUD;
      m->tio.c_cflag |= op->sym & CBAUD;
    }
# else /* TERMIO */
  if (ip) m->m_ttyb.sg_ispeed = ip->idx;
  if (op) m->m_ttyb.sg_ospeed = op->idx;
# endif /* TERMIO */
#endif /* POSIX */
  return 0;
}


int
CheckTtyname (tty)
char *tty;
{
  struct stat st;
  char realbuf[PATH_MAX];
  const char *real;
  int rc;

  real = realpath(tty, realbuf);
  if (!real)
    return -1;
  realbuf[sizeof(realbuf)-1]='\0';

  if (lstat(real, &st) || !S_ISCHR(st.st_mode) ||
    (st.st_nlink > 1 && strncmp(real, "/dev", 4)))
    rc = -1;
  else
    rc = 0;

  return rc;
}

/*
 *  Write out the mode struct in a readable form
 */

#ifdef DEBUG
void
DebugTTY(m)
struct mode *m;
{
  int i;

#ifdef POSIX
  debug("struct termios tio:\n");
  debug1("c_iflag = %#x\n", (unsigned int)m->tio.c_iflag);
  debug1("c_oflag = %#x\n", (unsigned int)m->tio.c_oflag);
  debug1("c_cflag = %#x\n", (unsigned int)m->tio.c_cflag);
  debug1("c_lflag = %#x\n", (unsigned int)m->tio.c_lflag);
  debug1("cfgetospeed() = %d\n", (int)cfgetospeed(&m->tio));
  debug1("cfgetispeed() = %d\n", (int)cfgetispeed(&m->tio));
  for (i = 0; i < sizeof(m->tio.c_cc)/sizeof(*m->tio.c_cc); i++)
    {
      debug2("c_cc[%d] = %#x\n", i, m->tio.c_cc[i]);
    }
# ifdef HPUX_LTCHARS_HACK
  debug1("suspc     = %#02x\n", m->m_ltchars.t_suspc);
  debug1("dsuspc    = %#02x\n", m->m_ltchars.t_dsuspc);
  debug1("rprntc    = %#02x\n", m->m_ltchars.t_rprntc);
  debug1("flushc    = %#02x\n", m->m_ltchars.t_flushc);
  debug1("werasc    = %#02x\n", m->m_ltchars.t_werasc);
  debug1("lnextc    = %#02x\n", m->m_ltchars.t_lnextc);
# endif /* HPUX_LTCHARS_HACK */
#else /* POSIX */
# ifdef TERMIO
  debug("struct termio tio:\n");
  debug1("c_iflag = %04o\n", m->tio.c_iflag);
  debug1("c_oflag = %04o\n", m->tio.c_oflag);
  debug1("c_cflag = %04o\n", m->tio.c_cflag);
  debug1("c_lflag = %04o\n", m->tio.c_lflag);
  for (i = 0; i < sizeof(m->tio.c_cc)/sizeof(*m->tio.c_cc); i++)
    {
      debug2("c_cc[%d] = %04o\n", i, m->tio.c_cc[i]);
    }
# else /* TERMIO */
  debug1("sg_ispeed = %d\n",    m->m_ttyb.sg_ispeed);
  debug1("sg_ospeed = %d\n",    m->m_ttyb.sg_ospeed);
  debug1("sg_erase  = %#02x\n", m->m_ttyb.sg_erase);
  debug1("sg_kill   = %#02x\n", m->m_ttyb.sg_kill);
  debug1("sg_flags  = %#04x\n", (unsigned short)m->m_ttyb.sg_flags);
  debug1("intrc     = %#02x\n", m->m_tchars.t_intrc);
  debug1("quitc     = %#02x\n", m->m_tchars.t_quitc);
  debug1("startc    = %#02x\n", m->m_tchars.t_startc);
  debug1("stopc     = %#02x\n", m->m_tchars.t_stopc);
  debug1("eofc      = %#02x\n", m->m_tchars.t_eofc);
  debug1("brkc      = %#02x\n", m->m_tchars.t_brkc);
  debug1("suspc     = %#02x\n", m->m_ltchars.t_suspc);
  debug1("dsuspc    = %#02x\n", m->m_ltchars.t_dsuspc);
  debug1("rprntc    = %#02x\n", m->m_ltchars.t_rprntc);
  debug1("flushc    = %#02x\n", m->m_ltchars.t_flushc);
  debug1("werasc    = %#02x\n", m->m_ltchars.t_werasc);
  debug1("lnextc    = %#02x\n", m->m_ltchars.t_lnextc);
  debug1("ldisc     = %d\n",    m->m_ldisc);
  debug1("lmode     = %#x\n",   m->m_lmode);
# endif /* TERMIO */
#endif /* POSIX */
}
#endif /* DEBUG */
