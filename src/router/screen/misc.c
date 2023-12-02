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

#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>	/* mkdir() declaration */
#include <signal.h>

#include "config.h"
#include "screen.h"
#include "extern.h"

#ifdef SVR4
# include <sys/resource.h>
#endif

extern struct layer *flayer;

extern int eff_uid, real_uid;
extern int eff_gid, real_gid;
extern struct mline mline_old;
extern struct mchar mchar_blank;
extern unsigned char *null, *blank;

#ifdef HAVE_FDWALK
static int close_func __P((void *, int));
#endif

char *
SaveStr(register const char *str)
{
	register char *cp;

	if (str == NULL)
		Panic(0, "SaveStr() received NULL - possibly failed crypt()");
	if ((cp = malloc(strlen(str) + 1)) == NULL)
		Panic(0, "%s", strnomem);
	else
		strcpy(cp, str);
	return cp;
}

char *
SaveStrn(register const char *str, int n)
{
	register char *cp;

	if ((cp = malloc(n + 1)) == NULL)
		Panic(0, "%s", strnomem);
	else {
		bcopy((char *)str, cp, n);
		cp[n] = 0;
	}
	return cp;
}

/* cheap strstr replacement */
char *
InStr(char *str, const char *pat)
{
	int npat = strlen(pat);
	for (; *str; str++)
		if (!strncmp(str, pat, npat))
			return str;
	return 0;
}

#ifndef HAVE_STRERROR
char *
strerror(int err)
{
	extern int sys_nerr;
	extern char *sys_errlist[];

	static char er[20];
	if (err > 0 && err < sys_nerr)
		return sys_errlist[err];
	sprintf(er, "Error %d", err);
	return er;
}
#endif

void
centerline(char *str, int y)
{
	int l, n;

	ASSERT(flayer);
	n = strlen(str);
	if (n > flayer->l_width - 1)
		n = flayer->l_width - 1;
	l = (flayer->l_width - 1 - n) / 2;
	LPutStr(flayer, str, n, &mchar_blank, l, y);
}

void
leftline(char *str, int y, struct mchar *rend)
{
	int l, n;
	struct mchar mchar_dol;

	mchar_dol = mchar_blank;
	mchar_dol.image = '$';

	ASSERT(flayer);
	l = n = strlen(str);
	if (n > flayer->l_width - 1)
		n = flayer->l_width - 1;
	LPutStr(flayer, str, n, rend ? rend : &mchar_blank, 0, y);
	if (n != l)
		LPutChar(flayer, &mchar_dol, n, y);
}

char *
Filename(char *s)
{
	register char *p = s;

	if (p)
		while (*p)
			if (*p++ == '/')
				s = p;
	return s;
}

char *
stripdev(char *nam)
{
#ifdef apollo
	char *p;

	if (nam == NULL)
		return NULL;
# ifdef SVR4
  /* unixware has /dev/pts012 as synonym for /dev/pts/12 */
	if (!strncmp(nam, "/dev/pts", 8) && nam[8] >= '0' && nam[8] <= '9') {
		static char b[13];
		sprintf(b, "pts/%d", atoi(nam + 8));
		return b;
	}
# endif /* SVR4 */
	if (p = strstr(nam, "/dev/"))
		return p + 5;
#else /* apollo */
	if (nam == NULL)
		return NULL;
	if (strncmp(nam, "/dev/", 5) == 0)
		return nam + 5;
#endif /* apollo */
	return nam;
}

/*
 *    Signal handling
 */

#ifdef POSIX
sigret_t (*xsignal(sig, func))
# ifndef __APPLE__
 __P(SIGPROTOARG)
# else
()
# endif
int sig;
sigret_t (*func) __P(SIGPROTOARG);
{
  struct sigaction osa, sa;
  sa.sa_handler = func;
  (void)sigemptyset(&sa.sa_mask);
#ifdef SA_RESTART
  sa.sa_flags = (sig == SIGCHLD ? SA_RESTART : 0);
#else
  sa.sa_flags = 0;
#endif
  if (sigaction(sig, &sa, &osa))
    return (sigret_t (*)__P(SIGPROTOARG))-1;
  return osa.sa_handler;
}

#else
# ifdef hpux
/*
 * hpux has berkeley signal semantics if we use sigvector,
 * but not, if we use signal, so we define our own signal() routine.
 */
void (*xsignal(sig, func)) __P(SIGPROTOARG)
int sig;
void (*func) __P(SIGPROTOARG);
{
  struct sigvec osv, sv;

  sv.sv_handler = func;
  sv.sv_mask = sigmask(sig);
  sv.sv_flags = SV_BSDSIG;
  if (sigvector(sig, &sv, &osv) < 0)
    return (void (*)__P(SIGPROTOARG))(BADSIG);
  return osv.sv_handler;
}
# endif	/* hpux */
#endif	/* POSIX */

/*
 *    uid/gid handling
 */

#ifdef HAVE_SETEUID

void
xseteuid(int euid)
{
	if (seteuid(euid) == 0)
		return;
	seteuid(0);
	if (seteuid(euid))
		Panic(errno, "seteuid");
}

void
xsetegid(int egid)
{
	if (setegid(egid))
		Panic(errno, "setegid");
}

#else /* HAVE_SETEUID */
# ifdef HAVE_SETREUID

void
xseteuid(int euid)
{
	int oeuid;

	oeuid = geteuid();
	if (oeuid == euid)
		return;
	if ((int)getuid() != euid)
		oeuid = getuid();
	if (setreuid(oeuid, euid))
		Panic(errno, "setreuid");
}

void
xsetegid(int egid)
{
	int oegid;

	oegid = getegid();
	if (oegid == egid)
		return;
	if ((int)getgid() != egid)
		oegid = getgid();
	if (setregid(oegid, egid))
		Panic(errno, "setregid");
}

# endif /* HAVE_SETREUID */
#endif /* HAVE_SETEUID */

#ifdef NEED_OWN_BCOPY
void
xbcopy(register char *s1, register char *s2, register int len)
{
	if (s1 < s2 && s2 < s1 + len) {
		s1 += len;
		s2 += len;
		while (len-- > 0)
			*--s2 = *--s1;
	} else
		while (len-- > 0)
			*s2++ = *s1++;
}
#endif	/* NEED_OWN_BCOPY */

void
bclear(char *p, int n)
{
	bcopy((char *)blank, p, n);
}

void
Kill(int pid, int sig)
{
	if (pid < 2)
		return;
	(void)kill(pid, sig);
}

#ifdef HAVE_FDWALK
/*
 * Modern versions of Solaris include fdwalk(3c) which allows efficient
 * implementation of closing open descriptors; this is helpful because
 * the default file descriptor limit has risen to 65k.
 */
static int
close_func(void *cb_data, int fd)
{
	int except = *(int *)cb_data;
	if (fd > 2 && fd != except)
		(void)close(fd);
	return (0);
}

void
closeallfiles(int except)
{
	(void)fdwalk(close_func, &except);
}

#else /* HAVE_FDWALK */

void
closeallfiles(int except)
{
	int f;
#ifdef SVR4
	struct rlimit rl;

	if ((getrlimit(RLIMIT_NOFILE, &rl) == 0) &&
	    rl.rlim_max != RLIM_INFINITY)
		f = rl.rlim_max;
	else
#endif /* SVR4 */
#if defined(SYSV) && defined(NOFILE) && !defined(ISC)
		f = NOFILE;
	while (--f > 2)
		if (f != except)
			close(f);
#else /* SYSV && !ISC */
	{
		struct pollfd pfd[1024];
		int maxfd, i, ret, z;

		i = 3; /* skip stdin, stdout and stderr */
		maxfd = getdtablesize();

		while (i < maxfd) {
			memset(pfd, 0, sizeof(pfd));

			z = 0;
			for (f = i; f < maxfd && f < i + 1024; f++)
				pfd[z++].fd = f;

			ret = poll(pfd, f - i, 0);
			if (ret < 0)
				Panic(errno, "poll");

			z = 0;
			for (f = i; f < maxfd && f < i + 1024; f++)
				if (!(pfd[z++].revents & POLLNVAL) &&
				    f != except)
					close(f);

			i = f;
		}
	}
#endif /* SYSV && !ISC */
}

#endif /* HAVE_FDWALK */

/*
 *  Security - switch to real uid
 */

#ifndef USE_SETEUID
static int UserPID;
static sigret_t (*Usersigcld) __P(SIGPROTOARG);
#endif
static int UserSTAT;

int
UserContext()
{
#ifndef USE_SETEUID
	if (eff_uid == real_uid && eff_gid == real_gid)
		return 1;
	Usersigcld = signal(SIGCHLD, SIG_DFL);
	debug("UserContext: forking.\n");
	switch (UserPID = fork()) {
	case -1:
		Msg(errno, "fork");
		return -1;
	case 0:
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
# ifdef BSDJOBS
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
# endif
		setuid(real_uid);
		setgid(real_gid);
		return 1;
	default:
		return 0;
	}
#else
	xseteuid(real_uid);
	xsetegid(real_gid);
	return 1;
#endif
}

void
UserReturn(int val)
{
#ifndef USE_SETEUID
	if (eff_uid == real_uid && eff_gid == real_gid)
		UserSTAT = val;
	else
		_exit(val);
#else
	    xseteuid(eff_uid);
	xsetegid(eff_gid);
	UserSTAT = val;
#endif
}

int
UserStatus()
{
#ifndef USE_SETEUID
	int i;
# ifdef BSDWAIT
	union wait wstat;
# else
	int wstat;
# endif

	if (eff_uid == real_uid && eff_gid == real_gid)
		return UserSTAT;
	if (UserPID < 0)
		return -1;
	while ((errno = 0, i = wait(&wstat)) != UserPID)
		if (i < 0 && errno != EINTR)
			break;
	(void)signal(SIGCHLD, Usersigcld);
	if (i == -1)
		return -1;
	return WEXITSTATUS(wstat);
#else
	return UserSTAT;
#endif
}

#ifndef HAVE_RENAME
int
rename(char *old, char *new)
{
	if (link(old, new) < 0)
		return -1;
	return unlink(old);
}
#endif

int
AddXChar(char *buf, int ch)
{
	char *p = buf;

	if (ch < ' ' || ch == 0x7f) {
		*p++ = '^';
		*p++ = ch ^ 0x40;
	} else if (ch >= 0x80) {
		*p++ = '\\';
		*p++ = (ch >> 6 & 7) + '0';
		*p++ = (ch >> 3 & 7) + '0';
		*p++ = (ch >> 0 & 7) + '0';
	} else
		*p++ = ch;
	return p - buf;
}

int
AddXChars(char *buf, int len, char *str)
{
	char *p;

	if (str == 0) {
		*buf = 0;
		return 0;
	}
	len -= 4;     /* longest sequence produced by AddXChar() */
	for (p = buf; p < buf + len && *str; str++) {
		if (*str == ' ')
			*p++ = *str;
		else
			p += AddXChar(p, *str);
	}
	*p = 0;
	return p - buf;
}

#ifdef DEBUG
void
opendebug(int new, int shout)
{
	char buf[256];

#ifdef _MODE_T
	mode_t oumask = umask(0);
#else
	int oumask = umask(0);
#endif

	ASSERT(!dfp);

	(void)mkdir(DEBUGDIR, 0777);
	sprintf(buf, shout ?
	    "%s/SCREEN.%d" : "%s/screen.%d", DEBUGDIR, getpid());
	if (!(dfp = fopen(buf, new ? "w" : "a")))
		dfp = stderr;
	else
		(void)chmod(buf, 0666);

	(void)umask(oumask);
	debug("opendebug: done.\n");
}
#endif /* DEBUG */

void
sleep1000(int msec)
{
	struct timeval t;

	t.tv_sec = (long)(msec / 1000);
	t.tv_usec = (long)((msec % 1000) * 1000);
	select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &t);
}

/*
 * This uses either setenv() or putenv(). If it is putenv() we cannot dare
 * to free the buffer after putenv(), unless it it the one found in putenv.c
 */
void
xsetenv(char *var, char *value)
{
#ifndef USESETENV
	char *buf;
	int l;

	if ((buf = (char *)malloc((l = strlen(var)) +
	    strlen(value) + 2)) == NULL) {
		Msg(0, strnomem);
		return;
	}
	strcpy(buf, var);
	buf[l] = '=';
	strcpy(buf + l + 1, value);
	putenv(buf);
# ifdef NEEDPUTENV
  /*
   * we use our own putenv(), knowing that it does a malloc()
   * the string space, we can free our buf now.
   */
	free(buf);
# else /* NEEDPUTENV */
  /*
   * For all sysv-ish systems that link a standard putenv()
   * the string-space buf is added to the environment and must not
   * be freed, or modified.
   * We are sorry to say that memory is lost here, when setting
   * the same variable again and again.
   */
# endif /* NEEDPUTENV */
#else /* USESETENV */
# if HAVE_SETENV_3
	setenv(var, value, 1);
# else
	setenv(var, value);
# endif /* HAVE_SETENV_3 */
#endif /* USESETENV */
}

#ifdef TERMINFO
/*
 * This is a replacement for the buggy _delay function from the termcap
 * emulation of libcurses, which ignores ospeed.
 */
int
_delay(register int delay, int (*outc) __P((int)))
{
  int pad;
  extern short ospeed;
  static short osp2pad[] = {
    0,2000,1333,909,743,666,500,333,166,83,55,41,20,10,5,2,1,1
  };

  if (ospeed <= 0 || ospeed >= (int)(sizeof(osp2pad)/sizeof(*osp2pad)))
    return 0;
  pad =osp2pad[ospeed];
  delay = (delay + pad / 2) / pad;
  while (delay-- > 0)
    (*outc)(0);
  return 0;
}

#endif /* TERMINFO */

#ifndef USEVARARGS

# define xva_arg(s, t, tn) (*(t *)(s += xsnoff(tn, 0, 0), s - xsnoff(tn, 0, 0)))
# define xva_list char *

static int
xsnoff(int a, char *b, int c)
{
  return a ? (char *)&c  - (char *)&b : (char *)&b - (char *)&a;
}

int
xsnprintf(char *s, int n, char *fmt, unsigned long p1, p2, p3, p4, p5, p6)
{
  int xvsnprintf __P((char *, int, char *, xva_list));
  return xvsnprintf(s, n, fmt, (char *)&fmt + xsnoff(1, 0, 0));
}

#else

# define xva_arg(s, t, tn) va_arg(s, t)
# define xva_list va_list

#endif

#if !defined(USEVARARGS) || !defined(HAVE_VSNPRINTF)

int
xvsnprintf(char *s, int n, char *fmt, xva_list stack)
{
	char *f, *sf = 0;
	int i, on, argl = 0;
	char myf[10], buf[20];
	char *arg, *myfp;

	on = n;
	f = fmt;
	arg = 0;
	while (arg || (sf = index(f, '%')) || (sf = f + strlen(f))) {
		if (arg == 0) {
			arg = f;
			argl = sf - f;
		}
		if (argl) {
			i = argl > n - 1 ? n - 1 : argl;
			strncpy(s, arg, i);
			s += i;
			n -= i;
			if (i < argl) {
				*s = 0;
				return on;
			}
		}
		arg = 0;
		if (sf == 0)
			continue;
		f = sf;
		sf = 0;
		if (!*f)
			break;
		myfp = myf;
		*myfp++ = *f++;
		while (((*f >= '0' && *f <= '9') || *f == '#') &&
		    myfp - myf < 8)
			*myfp++ = *f++;
		*myfp++ = *f;
		*myfp = 0;
		if (!*f++)
			break;
		switch (f[-1]) {
		case '%':
			arg = "%";
			break;
		case 'c':
		case 'o':
		case 'd':
		case 'x':
			i = xva_arg(stack, int, 0);
			sprintf(buf, myf, i);
			arg = buf;
			break;
		case 's':
			arg = xva_arg(stack, char *, 1);
			if (arg == 0)
				arg = "NULL";
			break;
		default:
			arg = "";
			break;
		}
		argl = strlen(arg);
	}
	*s = 0;
	return on - n;
}

#endif
