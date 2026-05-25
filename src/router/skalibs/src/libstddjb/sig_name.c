 /* ISC license. */

#include <skalibs/nonposix.h>

#include <signal.h>
#include <skalibs/nsig.h>

static char const *const table[SKALIBS_NSIG] =
{
  [SIGABRT] = "ABRT",
  [SIGALRM] = "ALRM",
  [SIGBUS] = "BUS",
  [SIGCHLD] = "CHLD",
  [SIGCONT] = "CONT",
  [SIGFPE] = "FPE",
  [SIGHUP] = "HUP",
  [SIGILL] = "ILL",
  [SIGINT] = "INT",
  [SIGKILL] = "KILL",
  [SIGPIPE] = "PIPE",
  [SIGQUIT] = "QUIT",
  [SIGSEGV] = "SEGV",
  [SIGSTOP] = "STOP",
  [SIGTERM] = "TERM",
  [SIGTSTP] = "TSTP",
  [SIGTTIN] = "TTIN",
  [SIGTTOU] = "TTOU",
  [SIGUSR1] = "USR1",
  [SIGUSR2] = "USR2",
#ifdef SIGPOLL
  [SIGPOLL] = "POLL",
#endif
#ifdef SIGPROF
  [SIGPROF] = "PROF",
#endif 
#ifdef SIGSYS
  [SIGSYS] = "SYS",
#endif
#ifdef SIGTRAP
  [SIGTRAP] = "TRAP",
#endif
#ifdef SIGURG
  [SIGURG] = "URG",
#endif
#ifdef SIGVTALRM
  [SIGVTALRM] = "VTALRM",
#endif
#ifdef SIGXCPU
  [SIGXCPU] = "XCPU",
#endif
#ifdef SIGXFSZ
  [SIGXFSZ] = "XFSZ",
#endif
#ifdef SIGIOT
  [SIGIOT] = "IOT",
#endif
#ifdef SIGEMT
  [SIGEMT] = "EMT",
#endif
#ifdef SIGSTKFLT
  [SIGSTKFLT] = "STKFLT",
#endif
#ifdef SIGCLD
  [SIGCLD] = "CLD",
#endif
#ifdef SIGWINCH
  [SIGWINCH] = "WINCH",
#endif
#ifdef SIGIO
  [SIGIO] = "IO",
#endif
#ifdef SIGINFO
  [SIGINFO] = "INFO",
#endif
#ifdef SIGLOST
  [SIGLOST] = "LOST",
#endif
#ifdef SIGPWR
  [SIGPWR] = "PWR",
#endif
#ifdef SIGUNUSED
  [SIGUNUSED] = "UNUSED"   
#endif
} ;

char const *sig_name (int sig)
{
  return sig <= 0 || sig > SKALIBS_NSIG || !table[sig] ? "???" : table[sig] ;
}
