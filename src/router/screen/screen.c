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
#ifdef HAVE_BRAILLE
 * Modified by:
 *      Authors:  Hadi Bargi Rangin  bargi@dots.physics.orst.edu
 *                Bill Barry         barryb@dots.physics.orst.edu
 *                Randy Lundquist    randyl@dots.physics.orst.edu
 *
 * Modifications Copyright (c) 1995 by
 * Science Access Project, Oregon State University.
#endif
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

#include <sys/types.h>
#ifdef _AIX
#include <sys/socket.h>
#endif
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#if defined(__sun)
# include <limits.h>
#endif

#ifdef sgi
# include <sys/sysmacros.h>
#endif

#include <sys/stat.h>
#ifndef sun
# include <sys/ioctl.h>
#endif

#ifndef SIGINT
# include <signal.h>
#endif

#include "config.h"

#ifdef HAVE_STROPTS_H
# include <sys/stropts.h>
#endif

#if defined(SYSV) && !defined(ISC)
# include <sys/utsname.h>
#endif

#if defined(sequent) || defined(SVR4)
# include <sys/resource.h>
#endif /* sequent || SVR4 */

#ifdef ISC
# include <sys/tty.h>
# include <sys/sioctl.h>
# include <sys/pty.h>
#endif /* ISC */

#if (defined(AUX) || defined(_AUX_SOURCE)) && defined(POSIX)
# include <compat.h>
#endif
#if defined(USE_LOCALE) || defined(ENCODINGS)
# include <locale.h>
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ENCODINGS)
# include <langinfo.h>
#endif

#include "screen.h"
#ifdef HAVE_BRAILLE
# include "braille.h"
#endif

#include "patchlevel.h"

/*
 *  At the moment we only need the real password if the
 *  builtin lock is used. Therefore disable SHADOWPW if
 *  we do not really need it (kind of security thing).
 */
#ifndef LOCK
# undef SHADOWPW
#endif

#include <pwd.h>
#ifdef SHADOWPW
# include <shadow.h>
#endif /* SHADOWPW */

#include "logfile.h" /* islogfile, logfflush, logfopen/logfclose */

#ifdef DEBUG
FILE *dfp;
#endif


extern char Term[], screenterm[], **environ, Termcap[];
int force_vt = 1;
int VBellWait, MsgWait, MsgMinWait, SilenceWait;

extern struct acluser *users;
extern struct display *displays, *display;
extern struct LayFuncs MarkLf;

extern int visual_bell;
#ifdef COPY_PASTE
extern unsigned char mark_key_tab[];
#endif
extern char version[];
extern char DefaultShell[];
#ifdef ZMODEM
extern char *zmodem_sendcmd;
extern char *zmodem_recvcmd;
#endif
extern struct layout *layout_last;

char *ShellProg;
char *ShellArgs[2];

extern struct NewWindow nwin_undef, nwin_default, nwin_options;
struct backtick;

static struct passwd *getpwbyname __P((char *, struct passwd *));
static void  SigChldHandler __P((void));
static sigret_t SigChld __P(SIGPROTOARG);
static sigret_t SigInt __P(SIGPROTOARG);
static sigret_t CoreDump __P(SIGPROTOARG);
static sigret_t FinitHandler __P(SIGPROTOARG);
static void  DoWait __P((void));
static void  serv_read_fn __P((struct event *, char *));
static void  serv_select_fn __P((struct event *, char *));
static void  logflush_fn __P((struct event *, char *));
static void  backtick_filter __P((struct backtick *));
static void  backtick_fn __P((struct event *, char *));
static char *runbacktick __P((struct backtick *, int *, time_t));
static int   IsSymbol __P((char *, char *));
static char *ParseChar __P((char *, char *));
static int   ParseEscape __P((char *));
static char *pad_expand __P((char *, char *, int, int));
static void SetTtyname __P((bool, struct stat *));
#ifdef DEBUG
static void  fds __P((void));
#endif

int nversion;	/* numerical version, used for secondary DA */

/* the attacher */
struct passwd *ppp;
char *attach_tty;
/* Indicator whether the current tty exists in another namespace. */
bool attach_tty_is_in_new_ns = false;
/* Content of the tty symlink when attach_tty_is_in_new_ns == true. */
char attach_tty_name_in_ns[MAXPATHLEN];
int attach_fd = -1;
char *attach_term;
char *LoginName;
struct mode attach_Mode;

char SockPath[MAXPATHLEN + 2 * MAXSTR];
char *SockName;			/* SockName is pointer in SockPath */
char *SockMatch = NULL;		/* session id command line argument */
int ServerSocket = -1;
struct event serv_read;
struct event serv_select;
struct event logflushev;

char **NewEnv = NULL;

char *RcFileName = NULL;
char *home;

char *screenlogfile;			/* filename layout */
int log_flush = 10;           		/* flush interval in seconds */
int logtstamp_on = 0;			/* tstamp disabled */
char *logtstamp_string;			/* stamp layout */
int logtstamp_after = 120;		/* first tstamp after 120s */
char *hardcopydir = NULL;
char *BellString;
char *VisualBellString;
char *ActivityString;
#ifdef COPY_PASTE
char *BufferFile;
#endif
#ifdef POW_DETACH
char *PowDetachString;
#endif
char *hstatusstring;
char *captionstring;
char *timestring;
char *wliststr;
char *wlisttit;
int auto_detach = 1;
int iflag, rflag, dflag, lsflag, quietflag, wipeflag, xflag;
int cmdflag;
int queryflag = -1;
int adaptflag;

#ifdef MULTIUSER
char *multi;
char *multi_home;
int multi_uid;
int own_uid;
int multiattach;
int tty_mode;
int tty_oldmode = -1;
#endif

char HostName[MAXSTR];
int MasterPid, PanicPid;
int real_uid, real_gid, eff_uid, eff_gid;
int default_startup;
int ZombieKey_destroy, ZombieKey_resurrect, ZombieKey_onerror;
char *preselect = NULL;		/* only used in Attach() */

#ifdef UTF8
char *screenencodings;
#endif

#ifdef DW_CHARS
int cjkwidth;
#endif

#ifdef NETHACK
int nethackflag = 0;
#endif
int maxwin;

struct layer *flayer;
struct win *fore;
struct win *windows;
struct win *console_window;

#ifdef BUILTIN_TELNET
int af;
#endif

/*
 * Do this last
 */
#include "extern.h"

char strnomem[] = "Out of memory.";

static int InterruptPlease;
static int GotSigChld;

static int lf_secreopen(name, wantfd, l)
char *name;
int wantfd;

struct logfile *l;
{
  int got_fd;
  close(wantfd);
  if (((got_fd = secopen(name, O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0) || lf_move_fd(got_fd, wantfd) < 0) {
      logfclose(l);
      debug1("lf_secreopen: failed for %s\n", name);
      return -1;
  }
  l->st->st_ino = l->st->st_dev = 0;
  debug2("lf_secreopen: %d = %s\n", wantfd, name);
  return 0;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

static struct passwd * getpwbyname(name, ppp)
char *name;

struct passwd *ppp;
{
  int n;
#ifdef SHADOWPW
  struct spwd *sss = NULL;
  static char *spw = NULL;
#endif

  if (!ppp && !(ppp = getpwnam(name)))
    return NULL;

  /* Do password sanity check..., allow ##user for SUN_C2 security */
#ifdef SHADOWPW
pw_try_again:
#endif
  n = 0;
  if (ppp->pw_passwd[0] == '#' && ppp->pw_passwd[1] == '#' && strcmp(ppp->pw_passwd + 2, ppp->pw_name) == 0)
    n = 13;
  for (; n < 13; n++) {
    char c = ppp->pw_passwd[n];
    if (!(c == '.' || c == '/'  || c == '$' ||
         (c >= '0' && c <= '9') ||
	     (c >= 'a' && c <= 'z') ||
	     (c >= 'A' && c <= 'Z')))
	break;
  }

#ifdef SHADOWPW
  /* try to determine real password */
  if (n < 13 && sss == 0) {
    sss = getspnam(ppp->pw_name);
    if (sss) {
	  if (spw)
	    free(spw);
	  ppp->pw_passwd = spw = SaveStr(sss->sp_pwdp);
	  endspent();	/* this should delete all buffers ... */
	  goto pw_try_again;
	}
      endspent();	/* this should delete all buffers ... */
  }
#endif
  if (n < 13)
    ppp->pw_passwd = 0;
#ifdef linux
  if (ppp->pw_passwd && strlen(ppp->pw_passwd) == 13 + 11)
    ppp->pw_passwd[13] = 0;	/* beware of linux's long passwords */
#endif

  return ppp;
}

static char *locale_name(void)
{
  static char *s;
  if (!s) {
    s = getenv("LC_ALL");
    if (s == NULL)
      s = getenv("LC_CTYPE");
    if (s == NULL)
      s = getenv("LANG");
    }
  return s;
}

int main(int ac, char** av)
{
  register int n;
  char *ap;
  char *av0;
  char socknamebuf[2 * MAXSTR];
  int mflag = 0;
  char *myname = (ac == 0) ? "screen" : av[0];
  char *SockDir;
  struct stat st;
#ifdef _MODE_T			/* (jw) */
  mode_t oumask;
#else
  int oumask;
#endif
#if defined(SYSV) && !defined(ISC)
  struct utsname utsnam;
#endif
  struct NewWindow nwin;
  int detached = 0;		/* start up detached */
#ifdef MULTIUSER
  char *sockp;
#endif
  char *sty = 0;

#if (defined(AUX) || defined(_AUX_SOURCE)) && defined(POSIX)
  setcompat(COMPAT_POSIX|COMPAT_BSDPROT); /* turn on seteuid support */
#endif
#if defined(sun) && defined(SVR4)
  {
    /* Solaris' login blocks SIGHUP! This is _very bad_ */
    sigset_t sset;
    sigemptyset(&sset);
    sigprocmask(SIG_SETMASK, &sset, 0);
  }
#endif

  /*
   *  First, close all unused descriptors
   *  (otherwise, we might have problems with the select() call)
   */
  closeallfiles(0);
#ifdef DEBUG
  opendebug(1, 0);
#endif
  snprintf(version, 59, "%d.%.2d.%.2d%s (%s%s) %s", REV, VERS,
	  PATCHLEVEL, STATE, ORIGIN, GIT_REV, DATE);
  nversion = REV * 10000 + VERS * 100 + PATCHLEVEL;
  debug2("-- screen debug started %s (%s)\n", *av, version);
#ifdef POSIX
  debug("POSIX\n");
#endif
#ifdef TERMIO
  debug("TERMIO\n");
#endif
#ifdef SYSV
  debug("SYSV\n");
#endif
#ifdef SYSVSIGS
  debug("SYSVSIGS\n");
#endif
#if defined(SIGWINCH) && defined(TIOCGWINSZ)
  debug("Window size changing enabled\n");
#endif
#ifdef HAVE_SETREUID
  debug("SETREUID\n");
#endif
#ifdef HAVE_SETEUID
  debug("SETEUID\n");
#endif
#ifdef hpux
  debug("hpux\n");
#endif
#ifdef USEBCOPY
  debug("USEBCOPY\n");
#endif
#ifdef UTMPOK
  debug("UTMPOK\n");
#endif
#ifdef LOADAV
  debug("LOADAV\n");
#endif
#ifdef NETHACK
  debug("NETHACK\n");
#endif
#ifdef TERMINFO
  debug("TERMINFO\n");
#endif
#ifdef SHADOWPW
  debug("SHADOWPW\n");
#endif
#ifdef NAME_MAX
  debug1("NAME_MAX = %d\n", NAME_MAX);
#endif

  BellString = SaveStr("Bell in window %n");
  VisualBellString = SaveStr("   Wuff,  Wuff!!  ");
  ActivityString = SaveStr("Activity in window %n");
  screenlogfile = SaveStr("screenlog.%n");
  logtstamp_string = SaveStr("-- %n:%t -- time-stamp -- %M/%d/%y %c:%s --\n");
  hstatusstring = SaveStr("%h");
  captionstring = SaveStr("%4n %t");
  timestring = SaveStr("%c:%s %M %d %H%? %l%?");
  wlisttit = SaveStr(" Num Name%=Flags");
  wliststr = SaveStr("%4n %t%=%f");
#ifdef COPY_PASTE
  BufferFile = SaveStr(DEFAULT_BUFFERFILE);
#endif
  ShellProg = NULL;
#ifdef POW_DETACH
  PowDetachString = 0;
#endif
  default_startup = (ac > 1) ? 0 : 1;
  adaptflag = 0;
  VBellWait = VBELLWAIT * 1000;
  MsgWait = MSGWAIT * 1000;
  MsgMinWait = MSGMINWAIT * 1000;
  SilenceWait = SILENCEWAIT;
#ifdef HAVE_BRAILLE
  InitBraille();
#endif
#ifdef ZMODEM
  zmodem_sendcmd = SaveStr("!!! sz -vv -b ");
  zmodem_recvcmd = SaveStr("!!! rz -vv -b -E");
#endif

#ifdef COPY_PASTE
  CompileKeys((char *)0, 0, mark_key_tab);
#endif
#ifdef UTF8
  InitBuiltinTabs();
  screenencodings = SaveStr(SCREENENCODINGS);
#endif
#ifdef DW_CHARS
  cjkwidth = 0;
#endif
  nwin = nwin_undef;
  nwin_options = nwin_undef;
  strncpy(screenterm, "screen", MAXTERMLEN);
  screenterm[MAXTERMLEN] = '\0';
#ifdef BUILTIN_TELNET
  af = AF_UNSPEC;
#endif

  real_uid = getuid();
  real_gid = getgid();
  eff_uid = geteuid();
  eff_gid = getegid();

  logreopen_register(lf_secreopen);

  av0 = *av;   /* if this is a login screen, assume -RR */
  if (*av0 == '-') {
    rflag = 4;
#ifdef MULTI
    xflag = 1;
#else
    dflag = 1;
#endif
    ShellProg = SaveStr(DefaultShell); /* to prevent nasty circles */
  }

  while (ac > 0){
    ap = *++av;
    if (--ac > 0 && *ap == '-') {
	  if (ap[1] == '-' && ap[2] == 0) {
	    av++;
	    ac--;
	    break;
	  }

	  if (ap[1] == '-' && !strcmp(ap, "--version"))
	    Panic(0, "Screen version %s", version);
	  if (ap[1] == '-' && !strcmp(ap, "--help"))
	    exit_with_usage(myname, NULL, NULL);

	  while (ap && *ap && *++ap) {
        switch (*ap) {

#ifdef BUILTIN_TELNET
          case '4':
            af = AF_INET;
			break;

          case '6':
            af = AF_INET6;
            break;
#endif

          case 'a':
            nwin_options.aflag = 1;
            break;

          case 'A':
            adaptflag = 1;
            break;

          case 'p':       /* preselect */
            if (*++ap)
              preselect = ap;
            else {
              if (!--ac)
                exit_with_usage(myname, "Specify a window to preselect with -p", NULL);
              preselect = *++av;
            }
            ap = NULL;
            break;

#ifdef HAVE_BRAILLE
          case 'B':
            bd.bd_start_braille = 1;
            break;
#endif

          case 'c':
            if (*++ap)
              RcFileName = ap;
            else {
              if (--ac == 0)
                exit_with_usage(myname, "Specify an alternate rc-filename with -c", NULL);
              RcFileName = *++av;
            }
            ap = NULL;
            break;

          case 'e':
		    if (!*++ap) {
              if (--ac == 0)
                exit_with_usage(myname, "Specify command characters with -e", NULL);
              ap = *++av;
            }
            if (ParseEscape(ap))
              Panic(0, "Two characters are required with -e option, not '%s'.", ap);
            ap = NULL;
            break;

          case 'f':
            ap++;
            switch (*ap++) {
              case 'n':
              case '0':
                nwin_options.flowflag = FLOW_NOW * 0;
                break;
              case '\0':
                ap--;
		        /* FALLTHROUGH */
              case 'y':
              case '1':
                nwin_options.flowflag = FLOW_NOW * 1;
                break;
              case 'a':
                nwin_options.flowflag = FLOW_AUTOFLAG;
                break;
              default:
                exit_with_usage(myname, "Unknown flow option -%s", --ap);
            }
            break;

          case 'h':
            if (--ac == 0)
              exit_with_usage(myname, NULL, NULL);
              nwin_options.histheight = atoi(*++av);
            if (nwin_options.histheight < 0)
              exit_with_usage(myname, "-h: %s: negative scrollback size?", *av);
            break;

          case 'i':
            iflag = 1;
            break;

          case 't': /* title, the former AkA == -k */
            if (--ac == 0)
              exit_with_usage(myname, "Specify a new window-name with -t", NULL);
            nwin_options.aka = *++av;
            break;

          case 'l':
            ap++;
            switch (*ap++) {
              case 'n':
              case '0':
                nwin_options.lflag = 0;
                break;
              case '\0':
                ap--;
                /* FALLTHROUGH */
              case 'y':
              case '1':
                nwin_options.lflag = 1;
                break;
              case 'a':
                nwin_options.lflag = 3;
                break;
              case 's':	/* -ls */
              case 'i':	/* -list */
                lsflag = 1;
                if (ac > 1 && !SockMatch) {
                  SockMatch = *++av;
                  ac--;
                }
                ap = NULL;
                break;
              default:
                exit_with_usage(myname, "%s: Unknown suboption to -l", --ap);
              }
            break;

          case 'w':
            if (strcmp(ap+1, "ipe"))
              exit_with_usage(myname, "Unknown option %s", --ap);
            lsflag = 1;
            wipeflag = 1;
            if (ac > 1 && !SockMatch) {
              SockMatch = *++av;
              ac--;
            }
            break;

          case 'L':
	    if (!strcmp(ap + 1, "ogfile")) {
              if (--ac == 0)
                exit_with_usage(myname, "Specify logfile path with -Logfile", NULL);

              if (strlen(*++av) > PATH_MAX)
                Panic(1, "-Logfile name too long. (max. %d char)", PATH_MAX);

              free(screenlogfile); /* we already set it up while starting */
              screenlogfile = SaveStr(*av);

              ap = NULL;
            } else if (!strcmp(ap, "L"))
              nwin_options.Lflag = 1;

            break;

          case 'm':
            mflag = 1;
            break;

          case 'O':		/* to be (or not to be?) deleted. jw. */
            force_vt = 0;
            break;

          case 'T':
            if (--ac == 0)
              exit_with_usage(myname, "Specify terminal-type with -T", NULL);
            if (strlen(*++av) < MAXTERMLEN) {
              strncpy(screenterm, *av, MAXTERMLEN);
              screenterm[MAXTERMLEN] = '\0';
            } else
              Panic(0, "-T: terminal name too long. (max. %d char)", MAXTERMLEN);
            nwin_options.term = screenterm;
            break;

          case 'q':
            quietflag = 1;
            break;

          case 'Q':
            queryflag = 1;
            cmdflag = 1;
            break;

          case 'r':
          case 'R':
#ifdef MULTI
          case 'x':
#endif
            if (ac > 1 && *av[1] != '-' && !SockMatch) {
              SockMatch = *++av;
              ac--;
              debug2("rflag=%d, SockMatch=%s\n", dflag, SockMatch);
            }
#ifdef MULTI
            if (*ap == 'x')
              xflag = 1;
#endif
            if (rflag)
              rflag = 2;
            rflag += (*ap == 'R') ? 2 : 1;
            break;

#ifdef REMOTE_DETACH
          case 'd':
            dflag = 1;
            /* FALLTHROUGH */

          case 'D':
            if (!dflag)
              dflag = 2;
            if (ac == 2) {
              if (*av[1] != '-' && !SockMatch) {
                SockMatch = *++av;
                ac--;
                debug2("dflag=%d, SockMatch=%s\n", dflag, SockMatch);
              }
            }
            break;
#endif

          case 's':
            if (--ac == 0)
              exit_with_usage(myname, "Specify shell with -s", NULL);
            if (ShellProg)
              free(ShellProg);
            ShellProg = SaveStr(*++av);
            debug1("ShellProg: '%s'\n", ShellProg);
            break;

          case 'S':
            if (!SockMatch) {
              if (--ac == 0)
                exit_with_usage(myname, "Specify session-name with -S", NULL);
              SockMatch = *++av;
            }
            if (!*SockMatch)
              exit_with_usage(myname, "Empty session-name?", NULL);
            break;

          case 'X':
            cmdflag = 1;
            break;

          case 'v':
            Panic(0, "Screen version %s", version);
            /* NOTREACHED */

#ifdef UTF8
          case 'U':
            nwin_options.encoding = nwin_options.encoding == -1 ? UTF8 : 0;
            break;
#endif

          default:
            exit_with_usage(myname, "Unknown option %s", --ap);
		}
      }
    }
    else
      break;
  }

#ifdef SIGBUS /* OOPS, linux has no bus errors! */
  signal(SIGBUS, CoreDump);
#endif /* SIGBUS */
  signal(SIGSEGV, CoreDump);


#ifdef USE_LOCALE
  setlocale(LC_ALL, "");
#endif
#ifdef ENCODINGS
  if (nwin_options.encoding == -1) {
  /* ask locale if we should start in UTF-8 mode */
# ifdef HAVE_NL_LANGINFO
#  ifndef USE_LOCALE
    setlocale(LC_CTYPE, "");
#  endif
    nwin_options.encoding = FindEncoding(nl_langinfo(CODESET));
    debug1("locale says encoding = %d\n", nwin_options.encoding);
# else
#  ifdef UTF8
    char *s;
    if ((s = locale_name()) && InStr(s, "UTF-8"))
      nwin_options.encoding = UTF8;
#  endif
    debug1("environment says encoding=%d\n", nwin_options.encoding);
#endif
  }

# ifdef DW_CHARS
  {
    char *s;
    if ((s = locale_name())) {
      if(!strncmp(s, "zh_", 3) || !strncmp(s, "ja_", 3) || !strncmp(s, "ko_", 3)) {
        cjkwidth = 1;
      }
    }
  }
#endif
#endif

  if (nwin_options.aka) {
#ifdef ENCODINGS
    if (nwin_options.encoding > 0) {
      size_t len = strlen(nwin_options.aka);
      size_t newsz;
      char *newbuf = malloc(3 * len);
      if (!newbuf)
        Panic(0, "%s", strnomem);
      newsz = RecodeBuf((unsigned char *)nwin_options.aka, len,
                           nwin_options.encoding, 0, (unsigned char *)newbuf);
      newbuf[newsz] = '\0';
      nwin_options.aka = newbuf;
    } else
#endif
   {
     /* If we just use the original value from av,
        subsequent shelltitle invocations will attempt to free
        space we don't own... */
      nwin_options.aka = SaveStr(nwin_options.aka);
    }
  }

  if (SockMatch && strlen(SockMatch) >= MAXSTR)
    Panic(0, "Ridiculously long socketname - try again.");
  if (cmdflag && !rflag && !dflag && !xflag)
    xflag = 1;
  if (!cmdflag && dflag && mflag && !(rflag || xflag))
    detached = 1;
  nwin = nwin_options;

#ifdef ENCODINGS
  nwin.encoding = nwin_undef.encoding;	/* let screenrc overwrite it */
#endif
  if (ac)
    nwin.args = av;

  /* make the write() calls return -1 on all errors */

#ifdef SIGXFSZ
  /*
   * Ronald F. Guilmette, Oct 29 '94, bug-gnu-utils@prep.ai.mit.edu:
   * It appears that in System V Release 4, UNIX, if you are writing
   * an output file and you exceed the currently set file size limit,
   * you _don't_ just get the call to `write' returning with a
   * failure code.  Rather, you get a signal called `SIGXFSZ' which,
   * if neither handled nor ignored, will cause your program to crash
   * with a core dump.
   */
  signal(SIGXFSZ, SIG_IGN);
#endif /* SIGXFSZ */

#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif

  if (!ShellProg) {
    register char *sh;
    sh = getenv("SHELL");
    ShellProg = SaveStr(sh ? sh : DefaultShell);
  }
  ShellArgs[0] = ShellProg;
  home = getenv("HOME");
  if (!mflag && !SockMatch) {
    sty = getenv("STY");
    if (sty && *sty == 0)
      sty = 0;
  }

#ifdef NETHACK
  if (!(nethackflag = (getenv("NETHACKOPTIONS") != NULL))) {
    char nethackrc[MAXPATHLEN];

    if (home && (strlen(home) < (MAXPATHLEN - 20))) {
      sprintf(nethackrc,"%s/.nethackrc", home);
      nethackflag = !access(nethackrc, F_OK);
    }
  }
#endif

#ifdef MULTIUSER
  own_uid = multi_uid = real_uid;
  if (SockMatch && (sockp = index(SockMatch, '/'))) {
    *sockp = 0;
    multi = SockMatch;
    SockMatch = sockp + 1;
    if (*multi) {
      struct passwd *mppp;
      if ((mppp = getpwnam(multi)) == (struct passwd *)0)
        Panic(0, "Cannot identify account '%s'.", multi);
      multi_uid = mppp->pw_uid;
      multi_home = SaveStr(mppp->pw_dir);
      if (strlen(multi_home) > MAXPATHLEN - 10)
        Panic(0, "home directory path too long");

# ifdef MULTI
      /* always fake multi attach mode */
      if (rflag || lsflag)
        xflag = 1;
# endif /* MULTI */
      detached = 0;
      multiattach = 1;
    }
    /* Special case: effective user is multiuser. */
    if (eff_uid && (multi_uid != eff_uid))
      Panic(0, "Must run suid root for multiuser support.");
  }
  if (SockMatch && *SockMatch == 0)
    SockMatch = 0;
#endif /* MULTIUSER */

  if ((LoginName = getlogin()) && LoginName[0] != '\0') {
    if ((ppp = getpwnam(LoginName)) != (struct passwd *) 0)
      if ((int)ppp->pw_uid != real_uid)
        ppp = (struct passwd *) 0;
  }
  if (ppp == 0) {
    if ((ppp = getpwuid(real_uid)) == 0) {
      Panic(0, "getpwuid() can't identify your account!");
    exit(1);
    }
    LoginName = ppp->pw_name;
  }
  LoginName = SaveStr(LoginName);
  ppp = getpwbyname(LoginName, ppp);

#if !defined(SOCKDIR) && defined(MULTIUSER)
  if (multi && !multiattach) {
    if (home && strcmp(home, ppp->pw_dir))
       Panic(0, "$HOME must match passwd entry for multiuser screens.");
  }
#endif

#define SET_GUID() do \
  { \
    setgid(real_gid); \
    setuid(real_uid); \
    eff_uid = real_uid; \
    eff_gid = real_gid; \
  } while (0)

  if (home == 0 || *home == '\0')
    home = ppp->pw_dir;
  if (strlen(LoginName) > MAXLOGINLEN)
    Panic(0, "LoginName too long - sorry.");

#ifdef MULTIUSER
  if (multi && strlen(multi) > MAXLOGINLEN)
    Panic(0, "Screen owner name too long - sorry.");
#endif
  if (strlen(home) > MAXPATHLEN - 25)
    Panic(0, "$HOME too long - sorry.");

  attach_tty = "";
  if (!detached && !lsflag && !cmdflag && !(dflag && !mflag && !rflag && !xflag) &&
      !(sty && !SockMatch && !mflag && !rflag && !xflag)) {
    int fl;

    /* ttyname implies isatty */
    SetTtyname(true, &st);
#ifdef MULTIUSER
    tty_mode = (int)st.st_mode & 0777;
#endif

    fl = fcntl(0, F_GETFL, 0);
    if (fl != -1 && (fl & (O_RDWR|O_RDONLY|O_WRONLY)) == O_RDWR)
      attach_fd = 0;


    if (attach_fd == -1) {
      if ((n = secopen(attach_tty, O_RDWR | O_NONBLOCK, 0)) < 0)
        Panic(0, "Cannot open your terminal '%s' - please check.", attach_tty);
      /* If the server uses a socket we need an open fd. */
      attach_fd = n;
    }

    debug2("attach_tty is %s, attach_fd is %d\n", attach_tty, attach_fd);

    if ((attach_term = getenv("TERM")) == 0 || *attach_term == 0)
      Panic(0, "Please set a terminal type.");
    if (strlen(attach_term) > MAXTERMLEN)
      Panic(0, "$TERM too long - sorry.");

    GetTTY(0, &attach_Mode);
#ifdef DEBUGGGGGGGGGGGGGGG
    DebugTTY(&attach_Mode);
#endif /* DEBUG */
    }

#ifdef _MODE_T
    oumask = umask(0);		/* well, unsigned never fails? jw. */
#else
    if ((oumask = (int)umask(0)) == -1)
      Panic(errno, "Cannot change umask to zero");
#endif

    SockDir = getenv("SCREENDIR");
    if (SockDir) {
      if (strlen(SockDir) >= MAXPATHLEN - 1)
        Panic(0, "Ridiculously long $SCREENDIR - try again.");

#ifdef MULTIUSER
      if (multi)
        Panic(0, "No $SCREENDIR with multi screens, please.");
#endif
    }

#ifdef MULTIUSER
    if (multiattach) {
# ifndef SOCKDIR
      sprintf(SockPath, "%s/.screen", multi_home);
      SockDir = SockPath;
# else
      SockDir = SOCKDIR;
      sprintf(SockPath, "%s/S-%s", SockDir, multi);
# endif
    } else
#endif

    {
#ifndef SOCKDIR
    if (SockDir == 0) {
      sprintf(SockPath, "%s/.screen", home);
      SockDir = SockPath;
    }
#endif

    if (SockDir) {
      if (access(SockDir, F_OK)) {
        debug1("SockDir '%s' missing ...\n", SockDir);
        if (UserContext() > 0) {
          if (mkdir(SockDir, 0700))
            UserReturn(0);
          UserReturn(1);
        }

        if (UserStatus() <= 0)
          Panic(0, "Cannot make directory '%s'.", SockDir);
      }
      if (SockDir != SockPath)
        strcpy(SockPath, SockDir);
    }

#ifdef SOCKDIR
    else {
      SockDir = SOCKDIR;
      if (stat(SockDir, &st)) {
        n = (eff_uid == 0 && (real_uid || eff_gid == real_gid)) ? 0755 :
            (eff_gid != real_gid) ? 0775 :
#ifdef S_ISVTX
            0777|S_ISVTX;
#else
            0777;
#endif

        if (mkdir(SockDir, n) == -1)
          Panic(errno, "Cannot make directory '%s'", SockDir);
	  }
      else {
        if (!S_ISDIR(st.st_mode))
          Panic(0, "'%s' must be a directory.", SockDir);
        if (eff_uid == 0 && real_uid && (int)st.st_uid != eff_uid)
          Panic(0, "Directory '%s' must be owned by root.", SockDir);
        n = (eff_uid == 0 && (real_uid || (st.st_mode & 0775) != 0775)) ? 0755 :
            (eff_gid == (int)st.st_gid && eff_gid != real_gid) ? 0775 : 0777;
        if (((int)st.st_mode & 0777) != n)
          Panic(0, "Directory '%s' must have mode %03o.", SockDir, n);
      }
      sprintf(SockPath, "%s/S-%s", SockDir, LoginName);
      if (access(SockPath, F_OK)) {
        if (mkdir(SockPath, 0700) == -1 && errno != EEXIST)
          Panic(errno, "Cannot make directory '%s'", SockPath);
        (void) chown(SockPath, real_uid, real_gid);
      }
    }
#endif
  }

  if (stat(SockPath, &st) == -1)
    Panic(errno, "Cannot access %s", SockPath);
  else
    if (!S_ISDIR(st.st_mode))
      Panic(0, "%s is not a directory.", SockPath);
#ifdef MULTIUSER
  if (multi) {
    if ((int)st.st_uid != multi_uid)
      Panic(0, "%s is not the owner of %s.", multi, SockPath);
  }
  else
#endif

  {
#ifdef SOCKDIR
/* if SOCKDIR is not defined, the socket is in $HOME.
   in that case it does not make sense to compare uids. */

    if ((int)st.st_uid != real_uid)
      Panic(0, "You are not the owner of %s.", SockPath);
#endif
  }

  if ((st.st_mode & 0777) != 0700)
    Panic(0, "Directory %s must have mode 700.", SockPath);
  if (SockMatch && index(SockMatch, '/'))
    Panic(0, "Bad session name '%s'", SockMatch);
  SockName = SockPath + strlen(SockPath) + 1;
  *SockName = 0;
  (void) umask(oumask);
  debug2("SockPath: %s  SockMatch: %s\n", SockPath, SockMatch ? SockMatch : "NULL");

#if defined(SYSV) && !defined(ISC)
  if (uname(&utsnam) == -1)
    Panic(errno, "uname");
  strncpy(HostName, utsnam.nodename, sizeof(utsnam.nodename) < MAXSTR ? sizeof(utsnam.nodename) : MAXSTR - 1);
  HostName[sizeof(utsnam.nodename) < MAXSTR ? sizeof(utsnam.nodename) : MAXSTR - 1] = '\0';
#else

  (void) gethostname(HostName, MAXSTR);
  HostName[MAXSTR - 1] = '\0';
#endif
  if ((ap = index(HostName, '.')) != NULL)
    *ap = '\0';

  if (lsflag) {
    int i, fo, oth;
    bool sock;

#ifdef MULTIUSER
    if (multi)
      real_uid = multi_uid;
#endif

    SET_GUID();
    i = FindSocket((int *)NULL, &fo, &oth, SockMatch, &sock);
    if (quietflag) {
      if (rflag)
        exit(10 + i);
      else
        exit(9 + (fo || oth ? 1 : 0) + fo);
    }
    if (fo == 0)
      Panic(0, "No Sockets found in %s.\n", SockPath);
    Msg(0, "%d Socket%s in %s.", fo, fo > 1 ? "s" : "", SockPath);
    eexit(0);
  }
  signal(SIG_BYE, AttacherFinit);	/* prevent races */
  if (cmdflag) {
    /* attach_tty is not mandatory */
    SetTtyname(false, &st);
    if (!*av)
      Panic(0, "Please specify a command.");
    SET_GUID();
    SendCmdMessage(sty, SockMatch, av, queryflag >= 0);
    exit(0);
  }
  else if (rflag || xflag) {
    debug("screen -r: - is there anybody out there?\n");
    if (Attach(MSG_ATTACH)) {
	  Attacher();
	  /* NOTREACHED */
    }
#ifdef MULTIUSER
    if (multiattach)
      Panic(0, "Can't create sessions of other users.");
#endif

    debug("screen -r: backend not responding -- still crying\n");
  }
  else if (dflag && !mflag) {
    SetTtyname(false, &st);
    Attach(MSG_DETACH);
    Msg(0, "[%s %sdetached.]\n", SockName, (dflag > 1 ? "power " : ""));
    eexit(0);
    /* NOTREACHED */
  }
  if (!SockMatch && !mflag && sty) {
    /* attach_tty is not mandatory */
    SetTtyname(false, &st);
    SET_GUID();
    nwin_options.args = av;
    SendCreateMsg(sty, &nwin);
    exit(0);
    /* NOTREACHED */
  }
  nwin_compose(&nwin_default, &nwin_options, &nwin_default);

  if (!detached || dflag != 2)
    MasterPid = fork();
  else
    MasterPid = 0;

  switch (MasterPid) {
    case -1:
      Panic(errno, "fork");
      /* NOTREACHED */
    case 0:
      break;
    default:
      if (detached)
        exit(0);
      if (SockMatch)
        sprintf(socknamebuf, "%d.%s", MasterPid, SockMatch);
      else
        sprintf(socknamebuf, "%d.%s.%s", MasterPid, stripdev(attach_tty), HostName);
    for (ap = socknamebuf; *ap; ap++)
      if (*ap == '/')
        *ap = '-';
#ifdef NAME_MAX
    if (strlen(socknamebuf) > NAME_MAX)
      socknamebuf[NAME_MAX] = 0;
#endif
    sprintf(SockPath + strlen(SockPath), "/%s", socknamebuf);
    SET_GUID();
    Attacher();
    /* NOTREACHED */
  }

  if (!detached)
    PanicPid = getppid();

  if (DefaultEsc == -1)
    DefaultEsc = Ctrl('a');
  if (DefaultMetaEsc == -1)
    DefaultMetaEsc = 'a';

  ap = av0 + strlen(av0) - 1;
  while (ap >= av0) {
    if (!strncmp("screen", ap, 6)) {
      strncpy(ap, "SCREEN", 6); /* name this process "SCREEN-BACKEND" */
      break;
    }
    ap--;
  }
  if (ap < av0)
    *av0 = 'S';

#ifdef DEBUG
  {
    char buf[256];

    if (dfp && dfp != stderr)
      fclose(dfp);
    sprintf(buf, "%s/SCREEN.%d", DEBUGDIR, (int)getpid());
    if ((dfp = fopen(buf, "w")) == NULL)
      dfp = stderr;
    else
      (void) chmod(buf, 0666);
  }
#endif

  if (!detached) {
    if (attach_fd == -1) {
      if ((n = secopen(attach_tty, O_RDWR | O_NONBLOCK, 0)) < 0)
        Panic(0, "Cannot reopen '%s' - please check.", attach_tty);
    }
    else
      n = dup(attach_fd);
  }
  else
    n = -1;
  freopen("/dev/null", "r", stdin);
  freopen("/dev/null", "w", stdout);

#ifdef DEBUG
  if (dfp != stderr)
#endif
  freopen("/dev/null", "w", stderr);
  debug("-- screen.back debug started\n");

  /*  This guarantees that the session owner is listed, even when we
   *  start detached. From now on we should not refer to 'LoginName'
   *  any more, use users->u_name instead.
   */
  if (UserAdd(LoginName, (char *)0, (struct acluser **)0) < 0)
    Panic(0, "Could not create user info");
  if (!detached) {
    if (MakeDisplay(LoginName, attach_tty, attach_term, n, getppid(), &attach_Mode) == 0)
      Panic(0, "Could not alloc display");
    PanicPid = 0;
#ifdef ENCODINGS
    D_encoding = nwin_options.encoding > 0 ? nwin_options.encoding : 0;
    debug1("D_encoding = %d\n", D_encoding);
#endif
  }

  if (SockMatch) {
    /* user started us with -S option */
    sprintf(socknamebuf, "%d.%s", (int)getpid(), SockMatch);
  }
  else {
    sprintf(socknamebuf, "%d.%s.%s", (int)getpid(), stripdev(attach_tty), HostName);
  }
  for (ap = socknamebuf; *ap; ap++)
    if (*ap == '/')
      *ap = '-';

#ifdef NAME_MAX
  if (strlen(socknamebuf) > NAME_MAX) {
    debug2("Socketname %s truncated to %d chars\n", socknamebuf, NAME_MAX);
    socknamebuf[NAME_MAX] = 0;
  }
#endif

  sprintf(SockPath + strlen(SockPath), "/%s", socknamebuf);
  /* Always create sockets. We only allow attaching to fifos not creating
   * new ones.
   */
  ServerSocket = MakeServerSocket(true);
  InitKeytab();

#ifdef ETCSCREENRC
# ifdef ALLOW_SYSSCREENRC
  if ((ap = getenv("SYSSCREENRC")))
    (void)StartRc(ap, 0);
  else
# endif
    (void)StartRc(ETCSCREENRC, 0);
#endif
  (void)StartRc(RcFileName, 0);
# ifdef UTMPOK
#  ifndef UTNOKEEP
  InitUtmp();
#  endif /* UTNOKEEP */
# endif /* UTMPOK */
  if (display) {
    if (InitTermcap(0, 0)) {
      debug("Could not init termcap - exiting\n");
      fcntl(D_userfd, F_SETFL, 0);	/* Flush sets FNBLOCK */
      freetty();
      if (D_userpid)
        Kill(D_userpid, SIG_BYE);
      eexit(1);
    }
    MakeDefaultCanvas();
    InitTerm(0);
#ifdef UTMPOK
    RemoveLoginSlot();
#endif
  }
  else
    MakeTermcap(1);

#ifdef LOADAV
  InitLoadav();
#endif /* LOADAV */

  MakeNewEnv();
  signal(SIGHUP, SigHup);
  signal(SIGINT, FinitHandler);
  signal(SIGQUIT, FinitHandler);
  signal(SIGTERM, FinitHandler);
#ifdef BSDJOBS
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
#endif

  if (display) {
    brktty(D_userfd);
    SetMode(&D_OldMode, &D_NewMode, D_flow, iflag);
    /* Note: SetMode must be called _before_ FinishRc. */
    SetTTY(D_userfd, &D_NewMode);
    if (fcntl(D_userfd, F_SETFL, FNBLOCK))
      Msg(errno, "Warning: NBLOCK fcntl failed");
  }
  else
    brktty(-1);		/* just try */
  signal(SIGCHLD, SigChld);
#ifdef ETCSCREENRC
# ifdef ALLOW_SYSSCREENRC
  if ((ap = getenv("SYSSCREENRC")))
    FinishRc(ap);
  else
# endif
    FinishRc(ETCSCREENRC);
#endif
  FinishRc(RcFileName);

  debug2("UID %d  EUID %d\n", (int)getuid(), (int)geteuid());
  if (windows == NULL) {
    debug("We open one default window, as screenrc did not specify one.\n");
    if (MakeWindow(&nwin) == -1) {
      fd_set rfd;
      struct timeval tv = { MsgWait/1000, 1000*(MsgWait%1000) };
      FD_SET(0, &rfd);

      Msg(0, "Sorry, could not find a PTY or TTY.");
      // allow user to exit early by pressing any key.
      select(1, &rfd, NULL, NULL, &tv);
      Finit(0);
      /* NOTREACHED */
    }
  }
  else if (ac) /* Screen was invoked with a command */
    MakeWindow(&nwin);

#ifdef HAVE_BRAILLE
  StartBraille();
#endif

  if (display && default_startup)
    display_copyright();
  signal(SIGINT, SigInt);
  if (rflag && (rflag & 1) == 0 && !quietflag) {
    Msg(0, "New screen...");
    rflag = 0;
  }

  serv_read.type = EV_READ;
  serv_read.fd = ServerSocket;
  serv_read.handler = serv_read_fn;
  evenq(&serv_read);

  serv_select.pri = -10;
  serv_select.type = EV_ALWAYS;
  serv_select.handler = serv_select_fn;
  evenq(&serv_select);

  logflushev.type = EV_TIMEOUT;
  logflushev.handler = logflush_fn;

  sched();
  /* NOTREACHED */
  return 0;
}

void
WindowDied(p, wstat, wstat_valid)
struct win *p;
#ifdef BSDWAIT
  union wait wstat;
#else
  int wstat;
#endif


int wstat_valid;
{
  int killit = 0;
  if (p->w_destroyev.data == (char *)p) {
    wstat = p->w_exitstatus;
    wstat_valid = 1;
    evdeq(&p->w_destroyev);
    p->w_destroyev.data = 0;
  }

#if defined(BSDJOBS) && !defined(BSDWAIT)
  if (!wstat_valid && p->w_pid > 0) {
      /* EOF on file descriptor. The process is probably also dead.
       * try a waitpid */
    if (waitpid(p->w_pid, &wstat, WNOHANG | WUNTRACED) == p->w_pid) {
      p->w_pid = 0;
      wstat_valid = 1;
    }
  }
#endif

  if (ZombieKey_destroy && ZombieKey_onerror && wstat_valid &&
      WIFEXITED(wstat) && WEXITSTATUS(wstat) == 0)
    killit = 1;

  if (ZombieKey_destroy && !killit) {
    char buf[100], *s, reason[100];
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

    (void) time(&now);
    s = ctime(&now);
    if (s && *s)
      s[strlen(s) - 1] = '\0';
    debug3("window %d (%s) going into zombie state fd %d", p->w_number, p->w_title, p->w_ptyfd);

#ifdef UTMPOK
    if (p->w_slot != (slot_t)0 && p->w_slot != (slot_t)-1) {
      RemoveUtmp(p);
      p->w_slot = 0;   /* "detached" */
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
      debug2("Set zombie poll timeout for window %s to %d\n", p->w_title,
      p->w_poll_zombie_timeout);
      SetTimeout(&p->w_zombieev, p->w_poll_zombie_timeout * 1000);
      evenq(&p->w_zombieev);
    }
    WindowChanged(p, 'f');
  }
  else
    KillWindow(p);

#ifdef UTMPOK
  CarefulUtmp();
#endif
}

static void SigChldHandler()
{
  struct stat st;
#ifdef DEBUG
  fds();
#endif
  while (GotSigChld) {
    GotSigChld = 0;
    DoWait();
#ifdef SYSVSIGS
    signal(SIGCHLD, SigChld);
#endif
  }
  if (stat(SockPath, &st) == -1) {
    debug1("SigChldHandler: Yuck! cannot stat '%s'\n", SockPath);
    if (!RecoverSocket()) {
      debug("SCREEN cannot recover from corrupt Socket, bye\n");
      Finit(1);
    }
    else
      debug1("'%s' reconstructed\n", SockPath);
  }
  else
    debug2("SigChldHandler: stat '%s' o.k. (%03o)\n", SockPath, (int)st.st_mode);
}

static sigret_t SigChld SIGDEFARG
{
  debug("SigChld()\n");
  GotSigChld = 1;
  SIGRETURN;
}

sigret_t SigHup SIGDEFARG
{
  /* Hangup all displays */
  while ((display = displays) != 0)
    Hangup();
  SIGRETURN;
}

/*
 * the backend's Interrupt handler
 * we cannot insert the intrc directly, as we never know
 * if fore is valid.
 */
static sigret_t SigInt SIGDEFARG
{
#if HAZARDOUS
  char ibuf;
  debug("SigInt()\n");
  if (fore && displays) {
# if defined(TERMIO) || defined(POSIX)
    ibuf = displays->d_OldMode.tio.c_cc[VINTR];
# else
    ibuf = displays->d_OldMode.m_tchars.t_intrc;
# endif
    fore->w_inlen = 0;
    write(fore->w_ptyfd, &ibuf, 1);
  }
#else
  signal(SIGINT, SigInt);
  debug("SigInt() careful\n");
  InterruptPlease = 1;
#endif
  SIGRETURN;
}

static sigret_t CoreDump SIGDEFARG
{
  /* if running with s-bit, we must reset the s-bit, so that we get a
   * core file anyway.
   */
  struct display *disp;
  char buf[80];
  char *dump_msg = " (core dumped)";
  int running_w_s_bit = getuid() != geteuid();

#if defined(SHADOWPW) && !defined(DEBUG) && !defined(DUMPSHADOW)
  if (running_w_s_bit)
    dump_msg = "";
#endif

#if defined(SYSVSIGS) && defined(SIGHASARG)
  signal(sigsig, SIG_IGN);
#endif
  setgid(getgid());
  setuid(getuid());
  unlink("core");

#ifdef SIGHASARG
  sprintf(buf, "\r\n[screen caught signal %d.%s]\r\n", sigsig, dump_msg);
#else
  sprintf(buf, "\r\n[screen caught a fatal signal.%s]\r\n", dump_msg);
#endif

  for (disp = displays; disp; disp = disp->d_next) {
    if (disp->d_nonblock < -1 || disp->d_nonblock > 1000000)
    continue;
      fcntl(disp->d_userfd, F_SETFL, 0);
      SetTTY(disp->d_userfd, &D_OldMode);
      write(disp->d_userfd, buf, strlen(buf));
      Kill(disp->d_userpid, SIG_BYE);
  }

  if (running_w_s_bit) {
#if defined(SHADOWPW) && !defined(DEBUG) && !defined(DUMPSHADOW)
    Kill(getpid(), SIGKILL);
    eexit(11);
#else /* SHADOWPW && !DEBUG */
    abort();
#endif /* SHADOWPW  && !DEBUG */
  }
  else
    abort();

  SIGRETURN;
}

static void DoWait()
{
  register int pid;
  struct win *p, *next;
#ifdef BSDWAIT
  union wait wstat;
#else
  int wstat;
#endif

#ifdef BSDJOBS

# ifndef BSDWAIT
  while ((pid = waitpid(-1, &wstat, WNOHANG | WUNTRACED)) > 0)
# else

# ifdef USE_WAIT2
  /*
   * From: rouilj@sni-usa.com (John Rouillard)
   * note that WUNTRACED is not documented to work, but it is defined in
   * /usr/include/sys/wait.h, so it may work
   */
  while ((pid = wait2(&wstat, WNOHANG | WUNTRACED )) > 0)
#  else /* USE_WAIT2 */
  while ((pid = wait3(&wstat, WNOHANG | WUNTRACED, (struct rusage *) 0)) > 0)
#  endif /* USE_WAIT2 */
# endif

#else	/* BSDJOBS */
  while ((pid = wait(&wstat)) < 0)
    if (errno != EINTR)
      break;
  if (pid > 0)

#endif	/* BSDJOBS */
  {
    for (p = windows; p; p = next) {
      next = p->w_next;
      if ((p->w_pid && pid == p->w_pid) || (p->w_deadpid && pid == p->w_deadpid)) {
      /* child has ceased to exist */
        p->w_pid = 0;

#ifdef BSDJOBS
        if (WIFSTOPPED(wstat)) {
          debug3("Window %d pid %d: WIFSTOPPED (sig %d)\n", p->w_number, pid, WSTOPSIG(wstat));

#ifdef SIGTTIN
        if (WSTOPSIG(wstat) == SIGTTIN) {
          Msg(0, "Suspended (tty input)");
          continue;
        }
#endif


#ifdef SIGTTOU
        if (WSTOPSIG(wstat) == SIGTTOU) {
          Msg(0, "Suspended (tty output)");
          continue;
        }
#endif

        /* Try to restart process */
        Msg(0, "Child has been stopped, restarting.");
        if (killpg(pid, SIGCONT))
          kill(pid, SIGCONT);
        }
        else
#endif
        {
          /* Screen will detect the window has died when the window's
           * file descriptor signals EOF (which it will do when the process in
           * the window terminates). So do this in a timeout of 10 seconds.
           * (not doing this at all might also work)
           * See #27061 for more details.
           */
           p->w_destroyev.data = (char *)p;
           p->w_exitstatus = wstat;
           SetTimeout(&p->w_destroyev, 10 * 1000);
           evenq(&p->w_destroyev);
         }
         break;
      }

#ifdef PSEUDOS
      if (p->w_pwin && pid == p->w_pwin->p_pid) {
        debug2("pseudo of win Nr %d died. pid == %d\n", p->w_number, p->w_pwin->p_pid);
        FreePseudowin(p);
        break;
      }

#endif
    }

    if (p == 0) {
      debug1("pid %d not found - hope that's ok\n", pid);
    }
  }
}

static sigret_t FinitHandler SIGDEFARG
{
#ifdef SIGHASARG
  debug1("FinitHandler called, sig %d.\n", sigsig);
#else
  debug("FinitHandler called.\n");
#endif
  Finit(1);
  SIGRETURN;
}

void Finit(int i)
{
  signal(SIGCHLD, SIG_DFL);
  signal(SIGHUP, SIG_IGN);
  debug1("Finit(%d);\n", i);
  while (windows) {
    struct win *p = windows;
    windows = windows->w_next;
    FreeWindow(p);
  }

  if (ServerSocket != -1) {
    debug1("we unlink(%s)\n", SockPath);
#ifdef USE_SETEUID
    xseteuid(real_uid);
    xsetegid(real_gid);
#endif
    (void) unlink(SockPath);
#ifdef USE_SETEUID
    xseteuid(eff_uid);
    xsetegid(eff_gid);
#endif
  }

  for (display = displays; display; display = display->d_next) {
    if (D_status)
      RemoveStatus();
      FinitTerm();
#ifdef UTMPOK
      RestoreLoginSlot();
#endif
      AddStr("[screen is terminating]\r\n");
      Flush(3);
      SetTTY(D_userfd, &D_OldMode);
      fcntl(D_userfd, F_SETFL, 0);
      freetty();
      Kill(D_userpid, SIG_BYE);
  }
  /*
   * we _cannot_ call eexit(i) here,
   * instead of playing with the Socket above. Sigh.
   */
  exit(i);
}

void eexit(int e)
{
  debug("eexit\n");
  if (ServerSocket != -1) {
    debug1("we unlink(%s)\n", SockPath);
    setgid(real_gid);
    setuid(real_uid);
    (void) unlink(SockPath);
  }
  exit(e);
}

void Hangup()
{
  if (display == 0)
    return;
  debug1("Hangup %x\n", display);
  if (D_userfd >= 0) {
    close(D_userfd);
    D_userfd = -1;
  }
  if (auto_detach || displays->d_next)
    Detach(D_HANGUP);
  else
    Finit(0);
}

/*
 * Detach now has the following modes:
 *D_DETACH	 SIG_BYE	detach backend and exit attacher
 *D_HANGUP	 SIG_BYE	detach backend and exit attacher
 *D_STOP	 SIG_STOP	stop attacher (and detach backend)
 *D_REMOTE	 SIG_BYE	remote detach -- reattach to new attacher
 *D_POWER 	 SIG_POWER_BYE 	power detach -- attacher kills his parent
 *D_REMOTE_POWER SIG_POWER_BYE	remote power detach -- both
 *D_LOCK	 SIG_LOCK	lock the attacher
 * (jw)
 * we always remove our utmp slots. (even when "lock" or "stop")
 * Note: Take extra care here, we may be called by interrupt!
 */
void Detach(int mode)
{
  int sign = 0, pid;
  struct canvas *cv;
  struct win *p;

  if (display == 0)
    return;

#define AddStrSock(msg) do { \
   if (SockName) \
      { \
	AddStr("[" msg " from "); \
	AddStr(SockName); \
	AddStr("]\r\n"); \
      } \
    else \
      AddStr("[" msg "]\r\n"); \
  } while (0)

  signal(SIGHUP, SIG_IGN);
  debug1("Detach(%d)\n", mode);
  if (D_status)
    RemoveStatus();
  FinitTerm();
  if (!display)
    return;
  switch (mode) {

    case D_HANGUP:
      sign = SIG_BYE;
      break;

    case D_DETACH:
      AddStrSock("detached");
      sign = SIG_BYE;
      break;

#ifdef BSDJOBS
    case D_STOP:
      sign = SIG_STOP;
      break;
#endif

#ifdef REMOTE_DETACH
    case D_REMOTE:
      AddStrSock("remote detached");
      sign = SIG_BYE;
      break;
#endif

#ifdef POW_DETACH
    case D_POWER:
      AddStrSock("power detached");
      if (PowDetachString) {
        AddStr(PowDetachString);
        AddStr("\r\n");
      }
      sign = SIG_POWER_BYE;
      break;

#ifdef REMOTE_DETACH
    case D_REMOTE_POWER:
      AddStrSock("remote power detached");
      if (PowDetachString) {
        AddStr(PowDetachString);
        AddStr("\r\n");
      }
      sign = SIG_POWER_BYE;
      break;
#endif
#endif

    case D_LOCK:
      ClearAll();
      sign = SIG_LOCK;
      /* tell attacher to lock terminal with a lockprg. */
      break;
  }

#ifdef UTMPOK
  if (displays->d_next == 0) {
    for (p = windows; p; p = p->w_next) {
      if (p->w_slot != (slot_t) -1 && !(p->w_lflag & 2)) {
        RemoveUtmp(p);

	    /* Set the slot to 0 to get the window logged in again. */
	    p->w_slot = (slot_t) 0;
      }
    }
  }
  if (mode != D_HANGUP)
    RestoreLoginSlot();
#endif

  if (displays->d_next == 0 && console_window) {
    if (TtyGrabConsole(console_window->w_ptyfd, 0, "detach")) {
      debug("could not release console - killing window\n");
      KillWindow(console_window);
      display = displays;                  /* restore display */
	}
  }
  if (D_fore) {
#ifdef MULTIUSER
    ReleaseAutoWritelock(display, D_fore);
#endif
    D_user->u_detachwin = D_fore->w_number;
    D_user->u_detachotherwin = D_other ? D_other->w_number : -1;
  }

  AutosaveLayout(D_layout);
  layout_last = D_layout;
  for (cv = D_cvlist; cv; cv = cv->c_next) {
    p = Layer2Window(cv->c_layer);
    SetCanvasWindow(cv, 0);
    if (p)
      WindowChanged(p, 'u');
  }

  pid = D_userpid;
  debug2("display: %#x displays: %#x\n", (unsigned int)display, (unsigned int)displays);
  FreeDisplay();

  if (displays == 0) /* Flag detached-ness */
    (void) chsock();

  /*
   * tell father what to do. We do that after we
   * freed the tty, thus getty feels more comfortable on hpux
   * if it was a power detach.
   */
  Kill(pid, sign);
  debug2("Detach: Signal %d to Attacher(%d)!\n", sign, pid);
  debug("Detach returns, we are successfully detached.\n");
  signal(SIGHUP, SigHup);
#undef AddStrSock
}

static int IsSymbol(char *e, char *s)
{
  register int l;

  l = strlen(s);
  return strncmp(e, s, l) == 0 && e[l] == '=';
}

void MakeNewEnv()
{
  register char **op, **np;
  static char stybuf[MAXSTR];

  for (op = environ; *op; ++op)
    ;
  if (NewEnv)
    free((char *)NewEnv);
  NewEnv = np = (char **) malloc((unsigned) (op - environ + 7 + 1) * sizeof(char **));
  if (!NewEnv)
    Panic(0, "%s", strnomem);
  sprintf(stybuf, "STY=%s", strlen(SockName) <= MAXSTR - 5 ? SockName : "?");
  *np++ = stybuf;	   /* NewEnv[0] */
  *np++ = Term;        /* NewEnv[1] */
  np++;		           /* room for SHELL */
#ifdef TIOCSWINSZ
  np += 2;	/* room for TERMCAP and WINDOW */
#else
  np += 4;	/* room for TERMCAP WINDOW LINES COLUMNS */
#endif

  for (op = environ; *op; ++op) {
    if (!IsSymbol(*op, "TERM") && !IsSymbol(*op, "TERMCAP")
      && !IsSymbol(*op, "STY") && !IsSymbol(*op, "WINDOW")
      && !IsSymbol(*op, "SCREENCAP") && !IsSymbol(*op, "SHELL")
      && !IsSymbol(*op, "LINES") && !IsSymbol(*op, "COLUMNS"))
      *np++ = *op;
  }
  *np = 0;
}

#if defined(USEVARARGS) && defined(__STDC__)
  #define DEFINE_VARARGS_FN(fnname)	void fnname (int err, const char *fmt, VA_DOTS)
#else
  #define DEFINE_VARARGS_FN(fnname)	void fnname(err, fmt, VA_DOTS) \
  int err;	\
  const char *fmt;	\
  VA_DECL
#endif

#define	PROCESS_MESSAGE(B) do { \
    char *p = B;	\
    VA_LIST(ap)	\
    VA_START(ap, fmt);	\
    fmt = DoNLS(fmt);	\
    (void)vsnprintf(p, sizeof(B) - 100, fmt, VA_ARGS(ap));	\
    VA_END(ap);	\
    if (err)	\
      {	\
	p += strlen(p);	\
	*p++ = ':';	\
	*p++ = ' ';	\
	strncpy(p, strerror(err), B + sizeof(B) - p - 1);	\
	B[sizeof(B) - 1] = 0;	\
      }	\
  } while (0)

DEFINE_VARARGS_FN(Msg)
{
  char buf[MAXPATHLEN*2];
  PROCESS_MESSAGE(buf);

  debug2("Msg('%s') (%#x);\n", buf, (unsigned int)display);

  if (display && displays)
    MakeStatus(buf);
  else if (displays) {
    for (display = displays; display; display = display->d_next)
      MakeStatus(buf);
  }
  else if (display) {
      /* no displays but a display - must have forked.
       * send message to backend!
       */
    char *tty = D_usertty;
    struct display *olddisplay = display;
    display = 0;	/* only send once */
    SendErrorMsg(tty, buf);
    display = olddisplay;
  }
  else
    printf("%s\r\n", buf);

  if (queryflag >= 0)
    write(queryflag, buf, strlen(buf));
}

/*
 * Call FinitTerm for all displays, write a message to each and call eexit();
 */
DEFINE_VARARGS_FN(Panic)
{
  char buf[MAXPATHLEN*2];
  PROCESS_MESSAGE(buf);

  debug3("Panic('%s'); display=%x displays=%x\n", buf, display, displays);
  if (displays == 0 && display == 0) {
    printf("%s\r\n", buf);
    if (PanicPid)
      Kill(PanicPid, SIG_BYE);
  }
  else if (displays == 0) {
      /* no displays but a display - must have forked.
       * send message to backend!
       */
    char *tty = D_usertty;
    display = 0;
    SendErrorMsg(tty, buf);
    sleep(2);
    _exit(1);
  }
  else
    for (display = displays; display; display = display->d_next) {
      if (D_status)
        RemoveStatus();
      FinitTerm();
      Flush(3);
#ifdef UTMPOK
      RestoreLoginSlot();
#endif
      SetTTY(D_userfd, &D_OldMode);
      fcntl(D_userfd, F_SETFL, 0);
      write(D_userfd, buf, strlen(buf));
      write(D_userfd, "\n", 1);
      freetty();
      if (D_userpid)
        Kill(D_userpid, SIG_BYE);
    }
#ifdef MULTIUSER
  if (tty_oldmode >= 0) {

# ifdef USE_SETEUID
    if (setuid(own_uid))
      xseteuid(own_uid);	/* may be a loop. sigh. */
# else
      setuid(own_uid);
# endif

    debug1("Panic: changing back modes from %s\n", attach_tty);
    chmod(attach_tty, tty_oldmode);
  }
#endif
  eexit(1);
}

DEFINE_VARARGS_FN(QueryMsg)
{
  char buf[MAXPATHLEN*2];

  if (queryflag < 0)
    return;

  PROCESS_MESSAGE(buf);
  write(queryflag, buf, strlen(buf));
}

DEFINE_VARARGS_FN(Dummy)
{}

#undef PROCESS_MESSAGE
#undef DEFINE_VARARGS_FN

/*
 * '^' is allowed as an escape mechanism for control characters. jw.
 *
 * Added time insertion using ideas/code from /\ndy Jones
 *   (andy@lingua.cltr.uq.OZ.AU) - thanks a lot!
 *
 */

#ifndef USE_LOCALE
static const char days[]   = "SunMonTueWedThuFriSat";
static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
#endif

static char winmsg_buf[MAXSTR];
#define MAX_WINMSG_REND 256	/* rendition changes */
static int winmsg_rend[MAX_WINMSG_REND];
static int winmsg_rendpos[MAX_WINMSG_REND];
static int winmsg_numrend;

static char *pad_expand(char *buf, char *p, int numpad, int padlen)
{
  char *pn, *pn2;
  int i, r;

  padlen = padlen - (p - buf);	/* space for rent */
  if (padlen < 0)
    padlen = 0;
  pn2 = pn = p + padlen;
  r = winmsg_numrend;
  while (p >= buf) {
    if (r && *p != 127 && p - buf == winmsg_rendpos[r - 1]) {
      winmsg_rendpos[--r] = pn - buf;
      continue;
    }
    *pn-- = *p;
    if (*p-- == 127) {
      pn[1] = ' ';
      i = numpad > 0 ? (padlen + numpad - 1) / numpad : 0;
      padlen -= i;

      while (i-- > 0)
	    *pn-- = ' ';

      numpad--;
      if (r && p - buf + 1== winmsg_rendpos[r - 1])
        winmsg_rendpos[--r] = pn - buf + 1;
	}
  }
  return pn2;
}

struct backtick {
  struct backtick *next;
  int num;
  int tick;
  int lifespan;
  time_t bestbefore;
  char result[MAXSTR];
  char **cmdv;
  struct event ev;
  char *buf;
  int bufi;
};

struct backtick *backticks;

static void backtick_filter(struct backtick *bt)
{
  char *p, *q;
  int c;

  for (p = q = bt->result; (c = (unsigned char)*p++) != 0;) {
    if (c == '\t')
    c = ' ';
    if (c >= ' ' || c == '\005')
      *q++ = c;
  }
  *q = 0;
}

static void backtick_fn(struct event *ev, char *data)
{
  struct backtick *bt;
  int i, j, k, l;

  bt = (struct backtick *)data;
  debug1("backtick_fn for #%d\n", bt->num);
  i = bt->bufi;
  l = read(ev->fd, bt->buf + i, MAXSTR - i);

  if (l <= 0) {
    debug1("EOF on backtick #%d\n", bt->num);
    evdeq(ev);
    close(ev->fd);
    ev->fd = -1;
    return;
  }

  debug1("read %d bytes\n", l);
  i += l;
  for (j = 0; j < l; j++)
    if (bt->buf[i - j - 1] == '\n')
      break;

  if (j < l) {
    for (k = i - j - 2; k >= 0; k--)
      if (bt->buf[k] == '\n')
	  break;
      k++;
      bcopy(bt->buf + k, bt->result, i - j - k);
      bt->result[i - j - k - 1] = 0;
      backtick_filter(bt);
      WindowChanged(0, '`');
  }

  if (j == l && i == MAXSTR) {
    j = MAXSTR/2;
    l = j + 1;
  }

  if (j < l){
    if (j)
      bcopy(bt->buf + i - j, bt->buf, j);
    i = j;
  }
  bt->bufi = i;
}

void setbacktick(int num, int lifespan, int tick, char **cmdv)
{
  struct backtick **btp, *bt;
  char **v;

  debug1("setbacktick called for backtick #%d\n", num);
  for (btp = &backticks; (bt = *btp) != 0; btp = &bt->next)
    if (bt->num == num)
      break;

  if (!bt && !cmdv)
    return;

  if (bt) {
    for (v = bt->cmdv; *v; v++)
      free(*v);
    free(bt->cmdv);
    if (bt->buf)
	  free(bt->buf);
    if (bt->ev.fd >= 0)
	  close(bt->ev.fd);
    evdeq(&bt->ev);
  }

  if (bt && !cmdv) {
    *btp = bt->next;
    free(bt);
    return;
  }

  if (!bt){
    bt = (struct backtick *)malloc(sizeof *bt);
    if (!bt){
      Msg(0, "%s", strnomem);
      return;
	}
    bzero(bt, sizeof(*bt));
    bt->next = 0;
    *btp = bt;
  }

  bt->num = num;
  bt->tick = tick;
  bt->lifespan = lifespan;
  bt->bestbefore = 0;
  bt->result[0] = 0;
  bt->buf = 0;
  bt->bufi = 0;
  bt->cmdv = cmdv;
  bt->ev.fd = -1;

  if (bt->tick == 0 && bt->lifespan == 0) {
    debug("setbacktick: continuous mode\n");
    bt->buf = (char *)malloc(MAXSTR);
    if (bt->buf == 0) {
      Msg(0, "%s", strnomem);
      setbacktick(num, 0, 0, (char **)0);
      return;
	}

    bt->ev.type = EV_READ;
    bt->ev.fd = readpipe(bt->cmdv);
    bt->ev.handler = backtick_fn;
    bt->ev.data = (char *)bt;
    if (bt->ev.fd >= 0)
      evenq(&bt->ev);
  }
}

static char * runbacktick(struct backtick *bt, int *tickp, time_t now)
{
  int f, i, l, j;
  time_t now2;

  debug1("runbacktick called for backtick #%d\n", bt->num);
  if (bt->tick && (!*tickp || bt->tick < *tickp))
    *tickp = bt->tick;
  if ((bt->lifespan == 0 && bt->tick == 0) || now < bt->bestbefore) {
    debug1("returning old result (%d)\n", bt->lifespan);
    return bt->result;
  }

  f = readpipe(bt->cmdv);
  if (f == -1)
    return bt->result;
  i = 0;
  while ((l = read(f, bt->result + i, sizeof(bt->result) - i)) > 0) {
    debug1("runbacktick: read %d bytes\n", l);
    i += l;
    for (j = 1; j < l; j++)
	  if (bt->result[i - j - 1] == '\n')
	    break;

    if (j == l && i == sizeof(bt->result)) {
	  j = sizeof(bt->result) / 2;
	  l = j + 1;
	}

    if (j < l) {
	  bcopy(bt->result + i - j, bt->result, j);
	  i = j;
	}
  }

  close(f);
  bt->result[sizeof(bt->result) - 1] = '\n';
  if (i && bt->result[i - 1] == '\n')
    i--;
  debug1("runbacktick: finished, %d bytes\n", i);
  bt->result[i] = 0;
  backtick_filter(bt);
  (void)time(&now2);
  bt->bestbefore = now2 + bt->lifespan;
  return bt->result;
}

int AddWinMsgRend(const char *str, int r)
{
  if (winmsg_numrend >= MAX_WINMSG_REND || str < winmsg_buf || str >= winmsg_buf + MAXSTR)
    return -1;

  winmsg_rend[winmsg_numrend] = r;
  winmsg_rendpos[winmsg_numrend] = str - winmsg_buf;
  winmsg_numrend++;
  return 0;
}

char *MakeWinMsgEv(char *str, struct win *win, int esc, int padlen, struct event *ev, int rec)
{
  static int tick;
  char *s = str;
  register char *p = winmsg_buf;
  register int ctrl;
  struct timeval now;
  struct tm *tm;
  int l, i, r;
  int num;
  int zeroflg;
  int longflg;
  int minusflg;
  int plusflg;
  int qmflag = 0, omflag = 0, qmnumrend = 0;
  char *qmpos = 0;
  int numpad = 0;
  int lastpad = 0;
  int truncpos = -1;
  int truncper = 0;
  int trunclong = 0;
  struct backtick *bt = NULL;

  if (winmsg_numrend >= 0)
    winmsg_numrend = 0;
  else
    winmsg_numrend = -winmsg_numrend;

  tick = 0;
  tm = 0;
  ctrl = 0;
  gettimeofday(&now, NULL);
  for (; *s && (l = winmsg_buf + MAXSTR - 1 - p) > 0; s++, p++) {
    *p = *s;
    if (ctrl) {
      ctrl = 0;
      if (*s != '^' && *s >= 64)
        *p &= 0x1f;
      continue;
	}

    if (*s != esc) {
      if (esc == '%') {
        switch (*s) {
#if 0
          case '~':
            *p = BELL;
            break;
#endif
          case '^':
            ctrl = 1;
            *p-- = '^';
            break;

          default:
            break;
		}
	  }
	  continue;
	}

    if (*++s == esc)	/* double escape ? */
      continue;

    if ((plusflg = *s == '+') != 0)
      s++;

    if ((minusflg = *s == '-') != 0)
      s++;

    if ((zeroflg = *s == '0') != 0)
      s++;

    num = 0;
    while(*s >= '0' && *s <= '9')
	  num = num * 10 + (*s++ - '0');

    if ((longflg = *s == 'L') != 0)
      s++;

    switch (*s) {
      case '?':
        p--;
        if (qmpos) {
          if ((!qmflag && !omflag) || omflag == 1){
            p = qmpos;
            if (qmnumrend < winmsg_numrend)
              winmsg_numrend = qmnumrend;
		  }
	      qmpos = 0;
	      break;
	    }
	    qmpos = p;
	    qmnumrend = winmsg_numrend;
	    qmflag = omflag = 0;
	    break;

      case ':':
        p--;
        if (!qmpos)
          break;
        if (qmflag && omflag != 1) {
          omflag = 1;
          qmpos = p;
          qmnumrend = winmsg_numrend;
        }
        else {
          p = qmpos;
          if (qmnumrend < winmsg_numrend)
            winmsg_numrend = qmnumrend;
          omflag = -1;
	    }
        break;

      case 'd': case 'D': case 'm': case 'M': case 'y': case 'Y':
      case 'a': case 'A': case 's': case 'c': case 'C':

        if (l < 4)
          break;
        if (tm == 0) {
          time_t nowsec = now.tv_sec;
          tm = localtime(&nowsec);
	    }

        qmflag = 1;
        if (!tick || tick > 3600)
        tick = 3600;

        switch (*s) {
          case 'd':
            sprintf(p, "%02d", tm->tm_mday % 100);
            break;

          case 'D':
#ifdef USE_LOCALE
            strftime(p, l, (longflg ? "%A" : "%a"), tm);
#else
            sprintf(p, "%3.3s", days + 3 * tm->tm_wday);
#endif
            break;

          case 'm':
            sprintf(p, "%02d", tm->tm_mon + 1);
            break;

          case 'M':
#ifdef USE_LOCALE
            strftime(p, l, (longflg ? "%B" : "%b"), tm);
#else
            sprintf(p, "%3.3s", months + 3 * tm->tm_mon);
#endif
            break;

          case 'y':
            sprintf(p, "%02d", tm->tm_year % 100);
            break;

          case 'Y':
            sprintf(p, "%04d", tm->tm_year + 1900);
            break;

          case 'a':
            sprintf(p, tm->tm_hour >= 12 ? "pm" : "am");
            break;

          case 'A':
            sprintf(p, tm->tm_hour >= 12 ? "PM" : "AM");
            break;

          case 's':
            sprintf(p, "%02d", tm->tm_sec);
            tick = 1;
            break;

          case 'c':
            sprintf(p, zeroflg ? "%02d:%02d" : "%2d:%02d", tm->tm_hour, tm->tm_min);
            if (!tick || tick > 60)
              tick = 60;
            break;

          case 'C':
            sprintf(p, zeroflg ? "%02d:%02d" : "%2d:%02d", (tm->tm_hour + 11) % 12 + 1, tm->tm_min);
            if (!tick || tick > 60)
              tick = 60;
            break;

          default:
            break;
	    }

        p += strlen(p) - 1;
        break;

      case 'X': case 'x':
        *p = 0;
        for (i = 0; win && win->w_cmdargs[i]; i++) {
          if (l < strlen(win->w_cmdargs[i]) + 1)
            break;
          sprintf(p, i ? " %s" : "%s", win->w_cmdargs[i]);
          l -= strlen(p);
          p += strlen(p);
          if (i == 0 && *s == 'X')
            break;
		}
		p--;
		break;

      case 'l':
#ifdef LOADAV
        *p = 0;
        if (l > 20)
          AddLoadav(p);
        if (*p) {
          qmflag = 1;
          p += strlen(p) - 1;
        }
        else
          *p = '?';

        if (!tick || tick > 60)
          tick = 60;
#else
        *p = '?';
#endif
        p += strlen(p) - 1;
        break;

      case '`':
      case 'h':
        if (rec >= 10 || (*s == 'h' && (win == 0 || win->w_hstatus == 0 || *win->w_hstatus == 0))) {
          p--;
          break;
        }
        if (*s == '`') {
          for (bt = backticks; bt; bt = bt->next)
            if (bt->num == num)
              break;
            if (bt == 0) {
              p--;
              break;
		    }
	    }
	    {
          char savebuf[sizeof(winmsg_buf)];
          int oldtick = tick;
          int oldnumrend = winmsg_numrend;

          *p = 0;
          strcpy(savebuf, winmsg_buf);
          winmsg_numrend = -winmsg_numrend;
          MakeWinMsgEv(*s == 'h' ?
            win->w_hstatus : runbacktick(bt, &oldtick, now.tv_sec), win, '\005', 0, (struct event *)0, rec + 1);
          debug2("oldtick=%d tick=%d\n", oldtick, tick);
          if (!tick || oldtick < tick)
            tick = oldtick;
          if ((int)strlen(winmsg_buf) < l)
            strcat(savebuf, winmsg_buf);
          strcpy(winmsg_buf, savebuf);
          while (oldnumrend < winmsg_numrend)
            winmsg_rendpos[oldnumrend++] += p - winmsg_buf;
          if (*p)
            qmflag = 1;
          p += strlen(p) - 1;
	    }
        break;

      case 'w':
      case 'W':
      {
        struct win *oldfore = 0;
        char *ss;
        if (display) {
          oldfore = D_fore;
          D_fore = win;
        }
        ss = AddWindows(p, l - 1, (*s == 'w' ? 0 : 1) |
             (longflg ? 0 : 2) | (plusflg ? 4 : 0) |
             (minusflg ? 8 : 0), win ? win->w_number : -1);
        if (display)
        D_fore = oldfore;
	  }
	  if (*p)
        qmflag = 1;
      p += strlen(p) - 1;
      break;

	case 'u':
      *p = 0;
      if (win)
        AddOtherUsers(p, l - 1, win);
      if (*p)
        qmflag = 1;
      p += strlen(p) - 1;
      break;

    case 'f':
      *p = 0;
      if (win)
        AddWindowFlags(p, l - 1, win);
      if (*p)
        qmflag = 1;
      p += strlen(p) - 1;
      break;

    case 't':
      *p = 0;
      if (win && (int)strlen(win->w_title) < l) {
        strcpy(p, win->w_title);
        if (*p)
          qmflag = 1;
      }
      p += strlen(p) - 1;
      break;

    case '{':
    {
      char rbuf[128];
      s++;

      for (i = 0; i < 127; i++)
        if (s[i] && s[i] != '}')
          rbuf[i] = s[i];
        else
          break;

      if (s[i] == '}' && winmsg_numrend < MAX_WINMSG_REND) {
        r = -1;
        rbuf[i] = 0;
        debug1("MakeWinMsg attrcolor %s\n", rbuf);

        if (i != 1 || rbuf[0] != '-')
          r = ParseAttrColor(rbuf, (char *)0, 0);
          if (r != -1 || (i == 1 && rbuf[0] == '-')) {
            winmsg_rend[winmsg_numrend] = r;
            winmsg_rendpos[winmsg_numrend] = p - winmsg_buf;
            winmsg_numrend++;
          }
       }
       s += i;
       p--;
    }
	break;

    case 'H':
      *p = 0;
      if ((int)strlen(HostName) < l) {
        strcpy(p, HostName);
        if (*p)
          qmflag = 1;
      }
      p += strlen(p) - 1;
      break;

    case 'S':
    {
      char *session_name;
      *p = 0;
      session_name = strchr(SockName, '.') + 1;
      if ((int)strlen(session_name) < l) {
        strcpy(p, session_name);
        if (*p)
          qmflag = 1;
      }
      p += strlen(p) - 1;
	}
    break;

    case 'p':
    {
      sprintf(p, "%d", (plusflg && display) ? D_userpid : getpid());
      p += strlen(p) - 1;
    }
    break;

    case 'F':
      p--;
      /* small hack */
      if (display && ((ev && ev == &D_forecv->c_captev) || (!ev && win && win == D_fore)))
        minusflg = !minusflg;
      if (minusflg)
        qmflag = 1;
      break;

    case 'P':
      p--;
#ifdef COPY_PASTE
      if (display && ev && ev != &D_hstatusev) {                   /* Hack */
        /* Is the layer in the current canvas in copy mode? */
        struct canvas *cv = (struct canvas *)ev->data;
        if (ev == &cv->c_captev && cv->c_layer->l_layfn == &MarkLf)
          qmflag = 1;
      }
#endif
      break;


    case 'E':
      p--;
      if (display && D_ESCseen)
        qmflag = 1;
      break;

    case '>':
      truncpos = p - winmsg_buf;
      truncper = num > 100 ? 100 : num;
      trunclong = longflg;
      p--;
      break;

    case '=':
    case '<':
      *p = ' ';
      if (num || zeroflg || plusflg || longflg || (*s != '=')) {
        /* expand all pads */
        if (minusflg) {
          num = (plusflg ? lastpad : padlen) - num;
          if (!plusflg && padlen == 0)
            num = p - winmsg_buf;
          plusflg = 0;
        }
        else if (!zeroflg) {
          if (*s != '=' && num == 0 && !plusflg)
            num = 100;
          if (num > 100)
            num = 100;
          if (padlen == 0)
            num = p - winmsg_buf;
          else
            num = (padlen - (plusflg ? lastpad : 0)) * num / 100;
        }

        if (num < 0)
          num = 0;

        if (plusflg)
          num += lastpad;

        if (num > MAXSTR - 1)
          num = MAXSTR - 1;

        if (numpad)
          p = pad_expand(winmsg_buf, p, numpad, num);

        numpad = 0;
        if (p - winmsg_buf > num && !longflg) {
          int left, trunc;
          if (truncpos == -1) {
            truncpos = lastpad;
            truncper = 0;
          }

          trunc = lastpad + truncper * (num - lastpad) / 100;
          if (trunc > num)
            trunc = num;
          if (trunc < lastpad)
            trunc = lastpad;
          left = truncpos - trunc;
          if (left > p - winmsg_buf - num)
            left = p - winmsg_buf - num;
          debug1("lastpad = %d, ", lastpad);
          debug3("truncpos = %d, trunc = %d, left = %d\n", truncpos, trunc, left);

          if (left > 0) {
            if (left + lastpad > p - winmsg_buf)
              left = p - winmsg_buf - lastpad;
            if (p - winmsg_buf - lastpad - left > 0)
              bcopy(winmsg_buf + lastpad + left, winmsg_buf + lastpad,  p - winmsg_buf - lastpad - left);
            p -= left;
            r = winmsg_numrend;
            while (r && winmsg_rendpos[r - 1] > lastpad) {
              r--;
              winmsg_rendpos[r] -= left;
              if (winmsg_rendpos[r] < lastpad)
                winmsg_rendpos[r] = lastpad;
            }

            if (trunclong) {
              if (p - winmsg_buf > lastpad)
                winmsg_buf[lastpad] = '.';
              if (p - winmsg_buf > lastpad + 1)
                winmsg_buf[lastpad + 1] = '.';
              if (p - winmsg_buf > lastpad + 2)
                winmsg_buf[lastpad + 2] = '.';
            }
          }

          if (p - winmsg_buf > num) {
            p = winmsg_buf + num;
            if (trunclong) {
              if (num - 1 >= lastpad)
                p[-1] = '.';
              if (num - 2 >= lastpad)
                p[-2] = '.';
              if (num - 3 >= lastpad)
                p[-3] = '.';
            }
            r = winmsg_numrend;
            while (r && winmsg_rendpos[r - 1] > num)
              winmsg_rendpos[--r] = num;
          }
          truncpos = -1;
          trunclong = 0;
          if (lastpad > p - winmsg_buf)
            lastpad = p - winmsg_buf;
          debug1("lastpad now %d\n", lastpad);
        }

        if (*s == '=') {
          while (p - winmsg_buf < num)
            *p++ = ' ';
          lastpad = p - winmsg_buf;
          truncpos = -1;
          trunclong = 0;
          debug1("lastpad2 now %d\n", lastpad);
        }
        p--;
	  }
      else if (padlen) {
        *p = 127;		/* internal pad representation */
        numpad++;
      }
	  break;

    case 'n':
      s++;
      /* FALLTHROUGH */

    default:
      s--;
      if (l > 10 + num) {
        if (num == 0)
          num = 1;
        if (!win)
          sprintf(p, "%*s", num, num > 1 ? "--" : "-");
        else
          sprintf(p, "%*d", num, win->w_number);
        qmflag = 1;
        p += strlen(p) - 1;
      }
      break;
    }
  }

  if (qmpos && !qmflag)
    p = qmpos + 1;
  *p = '\0';
  if (numpad) {
    if (padlen > MAXSTR - 1)
      padlen = MAXSTR - 1;
    p = pad_expand(winmsg_buf, p, numpad, padlen);
  }
  if (ev) {
    evdeq(ev);		/* just in case */
    ev->timeout.tv_sec = 0;
    ev->timeout.tv_usec = 0;
  }
  if (ev && tick) {
    now.tv_usec = 100000;
    if (tick == 1)
      now.tv_sec++;
    else
      now.tv_sec += tick - (now.tv_sec % tick);
    ev->timeout = now;
    debug2("NEW timeout %d %d\n", ev->timeout.tv_sec, tick);
  }
  return winmsg_buf;
}


char *MakeWinMsg(char *s, struct win *win, int esc)
{
  return MakeWinMsgEv(s, win, esc, 0, (struct event *)0, 0);
}

void PutWinMsg(char *s, int start, int max)
{
  int i, p, l, r, n;
  struct mchar rend;
  struct mchar rendstack[MAX_WINMSG_REND];
  int rendstackn = 0;

  if (s != winmsg_buf) {
    /* sorry, no fancy coloring available */
    debug1("PutWinMsg %s plain\n", s);
    l = strlen(s);
    if (l > max)
      l = max;
    l -= start;
    s += start;
    while (l-- > 0)
      PUTCHARLP(*s++);
    return;
  }
  rend = D_rend;
  p = 0;
  l = strlen(s);
  debug2("PutWinMsg %s start attr %x\n", s, rend.attr);
  for (i = 0; i < winmsg_numrend && max > 0; i++) {
    if (p > winmsg_rendpos[i] || winmsg_rendpos[i] > l)
      break;
    if (p < winmsg_rendpos[i]) {
      n = winmsg_rendpos[i] - p;
      if (n > max)
        n = max;
      max -= n;
      p += n;
      while(n-- > 0) {
        if (start-- > 0)
          s++;
        else
          PUTCHARLP(*s++);
      }
	}

    r = winmsg_rend[i];
    if (r == -1) {
      if (rendstackn > 0)
        rend = rendstack[--rendstackn];
	}
    else {
      rendstack[rendstackn++] = rend;
      ApplyAttrColor(r, &rend);
    }

    SetRendition(&rend);
  }
  if (p < l){
    n = l - p;
    if (n > max)
      n = max;
    while(n-- > 0) {
      if (start-- > 0)
        s++;
      else
        PUTCHARLP(*s++);
    }
  }
}

#ifdef DEBUG
static void fds1(int i, int j)
{
  while (i < j) {
    debug1("%d ", i);
    i++;
  }
  if ((j = open("/dev/null", 0)) >= 0) {
    fds1(i + 1, j);
    close(j);
  }
  else {
    while (dup(++i) < 0 && errno != EBADF)
      debug1("%d ", i);
    debug1(" [%d]\n", i);
  }
}

static void fds()
{
  debug("fds: ");
  fds1(-1, -1);
}
#endif

static void serv_read_fn(struct event *ev, char *data)
{
  debug("Knock - knock!\n");
  ReceiveMsg();
}

static void serv_select_fn(struct event *ev, char *data)
{
  struct win *p;
  debug("serv_select_fn called\n");
  /* XXX: messages?? */
  if (GotSigChld)
      SigChldHandler();

  if (InterruptPlease) {
    debug("Backend received interrupt\n");
    /* This approach is rather questionable in a multi-display
     * environment */
    if (fore && displays) {
#if defined(TERMIO) || defined(POSIX)
      char ibuf = displays->d_OldMode.tio.c_cc[VINTR];
#else
      char ibuf = displays->d_OldMode.m_tchars.t_intrc;
#endif


#ifdef PSEUDOS
      write(W_UWP(fore) ? fore->w_pwin->p_ptyfd : fore->w_ptyfd, &ibuf, 1);
      debug1("Backend wrote interrupt to %d", fore->w_number);
      debug1("%s\n", W_UWP(fore) ? " (pseudowin)" : "");
#else
      write(fore->w_ptyfd, &ibuf, 1);
      debug1("Backend wrote interrupt to %d\n", fore->w_number);
#endif
    }
    InterruptPlease = 0;
  }

  for (p = windows; p; p = p->w_next){
    if (p->w_bell == BELL_FOUND || p->w_bell == BELL_VISUAL) {
      struct canvas *cv;
      int visual = p->w_bell == BELL_VISUAL || visual_bell;
      p->w_bell = BELL_ON;
      for (display = displays; display; display = display->d_next) {
        for (cv = D_cvlist; cv; cv = cv->c_next)
          if (cv->c_layer->l_bottom == &p->w_layer)
            break;

        if (cv == 0) {
		  p->w_bell = BELL_DONE;
		  Msg(0, "%s", MakeWinMsg(BellString, p, '%'));
        }
	    else if (visual && !D_VB && (!D_status || !D_status_bell)) {
          Msg(0, "%s", VisualBellString);

          if (D_status) {
            D_status_bell = 1;
            debug1("using vbell timeout %d\n", VBellWait);
            SetTimeout(&D_statusev, VBellWait );
          }
        }
      }

      /* don't annoy the user with two messages */
      if (p->w_monitor == MON_FOUND)
        p->w_monitor = MON_DONE;
      WindowChanged(p, 'f');
    }

    if (p->w_monitor == MON_FOUND) {
      struct canvas *cv;
      p->w_monitor = MON_ON;
      for (display = displays; display; display = display->d_next) {
        for (cv = D_cvlist; cv; cv = cv->c_next)
          if (cv->c_layer->l_bottom == &p->w_layer)
            break;
          if (cv)
            continue;   /* user already sees window */

#ifdef MULTIUSER
          if (!(ACLBYTE(p->w_mon_notify, D_user->u_id) & ACLBIT(D_user->u_id)))
            continue;   /* user doesn't care */
#endif

          Msg(0, "%s", MakeWinMsg(ActivityString, p, '%'));
          p->w_monitor = MON_DONE;
      }
      WindowChanged(p, 'f');
    }

    if (p->w_silence == SILENCE_FOUND) {
      /* Unset the flag if the user switched to this window. */
      if (p->w_layer.l_cvlist) {
        p->w_silence = SILENCE_ON;
        WindowChanged(p, 'f');
      }
    }
  }

  for (display = displays; display; display = display->d_next) {
    struct canvas *cv;
    if (D_status == STATUS_ON_WIN)
      continue;
      /* XXX: should use display functions! */
      for (cv = D_cvlist; cv; cv = cv->c_next) {
        int lx, ly;

      /* normalize window, see resize.c */
      lx = cv->c_layer->l_x;
      ly = cv->c_layer->l_y;
      if (lx == cv->c_layer->l_width)
        lx--;
      if (ly + cv->c_yoff < cv->c_ys) {
        int i, n = cv->c_ys - (ly + cv->c_yoff);
	    cv->c_yoff = cv->c_ys - ly;
        RethinkViewportOffsets(cv);
        if (n > cv->c_layer->l_height)
          n = cv->c_layer->l_height;
        CV_CALL(cv,
            LScrollV(flayer, -n, 0, flayer->l_height - 1, 0);
            LayRedisplayLine(-1, -1, -1, 1);
            for (i = 0; i < n; i++)
              LayRedisplayLine(i, 0, flayer->l_width - 1, 1);
            if (cv == cv->c_display->d_forecv)
              LaySetCursor();
	    );
	  }
      else if (ly + cv->c_yoff > cv->c_ye) {
        int i, n = ly + cv->c_yoff - cv->c_ye;
        cv->c_yoff = cv->c_ye - ly;
        RethinkViewportOffsets(cv);
        if (n > cv->c_layer->l_height)
          n = cv->c_layer->l_height;
        CV_CALL(cv,
           LScrollV(flayer, n, 0, cv->c_layer->l_height - 1, 0);
           LayRedisplayLine(-1, -1, -1, 1);
           for (i = 0; i < n; i++)
             LayRedisplayLine(i + flayer->l_height - n, 0, flayer->l_width - 1, 1);
             if (cv == cv->c_display->d_forecv)
               LaySetCursor();
        );
      }
      if (lx + cv->c_xoff < cv->c_xs) {
        int i, n = cv->c_xs - (lx + cv->c_xoff);
        if (n < (cv->c_xe - cv->c_xs + 1) / 2)
          n = (cv->c_xe - cv->c_xs + 1) / 2;
        if (cv->c_xoff + n > cv->c_xs)
          n = cv->c_xs - cv->c_xoff;
        cv->c_xoff += n;
        RethinkViewportOffsets(cv);
        if (n > cv->c_layer->l_width)
          n = cv->c_layer->l_width;
        CV_CALL(cv,
           LayRedisplayLine(-1, -1, -1, 1);
           for (i = 0; i < flayer->l_height; i++) {
              LScrollH(flayer, -n, i, 0, flayer->l_width - 1, 0, 0);
              LayRedisplayLine(i, 0, n - 1, 1);
           }
           if (cv == cv->c_display->d_forecv)
             LaySetCursor();
        );
      }
      else if (lx + cv->c_xoff > cv->c_xe) {
        int i, n = lx + cv->c_xoff - cv->c_xe;
        if (n < (cv->c_xe - cv->c_xs + 1) / 2)
          n = (cv->c_xe - cv->c_xs + 1) / 2;
        if (cv->c_xoff - n + cv->c_layer->l_width - 1 < cv->c_xe)
          n = cv->c_xoff + cv->c_layer->l_width - 1 - cv->c_xe;
        cv->c_xoff -= n;
        RethinkViewportOffsets(cv);
        if (n > cv->c_layer->l_width)
          n = cv->c_layer->l_width;
        CV_CALL(cv,
           LayRedisplayLine(-1, -1, -1, 1);
           for (i = 0; i < flayer->l_height; i++) {
             LScrollH(flayer, n, i, 0, flayer->l_width - 1, 0, 0);
             LayRedisplayLine(i, flayer->l_width - n, flayer->l_width - 1, 1);
           }
           if (cv == cv->c_display->d_forecv)
             LaySetCursor();
        );
      }
    }
  }
  for (display = displays; display; display = display->d_next) {
    if (D_status == STATUS_ON_WIN || D_cvlist == 0 || D_cvlist->c_next == 0)
      continue;
    debug1("serv_select_fn: Restore on cv %#x\n", (int)D_forecv);
    CV_CALL(D_forecv, LayRestore();LaySetCursor());
  }
}

static void logflush_fn(struct event *ev, char *data)
{
  struct win *p;
  char *buf;
  int n;

  if (!islogfile(NULL))
    return;          /* no more logfiles */
  logfflush(NULL);
  n = log_flush ? log_flush : (logtstamp_after + 4) / 5;

  if (n) {
    SetTimeout(ev, n * 1000);
    evenq(ev);       /* re-enqueue ourself */
  }

  if (!logtstamp_on)
    return;

  /* write fancy time-stamp */
  for (p = windows; p; p = p->w_next) {
    if (!p->w_log)
      continue;
    p->w_logsilence += n;
    if (p->w_logsilence < logtstamp_after)
      continue;
    if (p->w_logsilence - n >= logtstamp_after)
      continue;
    buf = MakeWinMsg(logtstamp_string, p, '%');
    logfwrite(p->w_log, buf, strlen(buf));
  }
}

/*
 * Interprets ^?, ^@ and other ^-control-char notation.
 * Interprets \ddd octal notation
 *
 * The result is placed in *cp, p is advanced behind the parsed expression and
 * returned.
 */
static char *ParseChar(char *p, char *cp)
{
  if (*p == 0)
    return 0;

  if (*p == '^' && p[1]) {
    if (*++p == '?')
      *cp = '\177';
    else if (*p >= '@')
      *cp = Ctrl(*p);
    else
      return 0;
      ++p;
  }
  else if (*p == '\\' && *++p <= '7' && *p >= '0') {
    *cp = 0;
    do
      *cp = *cp * 8 + *p - '0';
      while (*++p <= '7' && *p >= '0');
  }
  else
    *cp = *p++;
  return p;
}

static int ParseEscape(char *p)
{
  unsigned char buf[2];

  if (*p == 0)
    SetEscape((struct acluser *)0, -1, -1);
  else {
    if ((p = ParseChar(p, (char *)buf)) == NULL ||
        (p = ParseChar(p, (char *)buf+1)) == NULL || *p)
      return -1;
    SetEscape((struct acluser *)0, buf[0], buf[1]);
  }
  return 0;
}

void SetTtyname(bool fatal, struct stat *st)
{
	int ret;
	int saved_errno = 0;

	attach_tty_is_in_new_ns = false;
	memset(&attach_tty_name_in_ns, 0, sizeof(attach_tty_name_in_ns));

	errno = 0;
	attach_tty = ttyname(0);
	if (!attach_tty) {
		if (errno == ENODEV) {
			saved_errno = errno;
			attach_tty = "/proc/self/fd/0";
			attach_tty_is_in_new_ns = true;
			ret = readlink(attach_tty, attach_tty_name_in_ns, sizeof(attach_tty_name_in_ns));
			if (ret < 0 || (size_t)ret >= sizeof(attach_tty_name_in_ns))
				Panic(0, "Bad tty '%s'", attach_tty);
		} else if (fatal) {
			Panic(0, "Must be connected to a terminal.");
		} else {
			attach_tty = "";
		}
	}

	if (attach_tty && strcmp(attach_tty, "")) {
		if (stat(attach_tty, st))
			Panic(errno, "Cannot access '%s'", attach_tty);

		if (strlen(attach_tty) >= MAXPATHLEN)
			Panic(0, "TtyName too long - sorry.");

		/* Only call CheckTtyname() if the device does not exist in
		 * another namespace.
		 */
		if (saved_errno != ENODEV && CheckTtyname(attach_tty))
			Panic(0, "Bad tty '%s'", attach_tty);
	}
}
