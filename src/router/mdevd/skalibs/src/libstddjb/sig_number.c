/* ISC license. */

#include <stdlib.h>

#include <skalibs/bytestr.h>
#include <skalibs/sig.h>

struct sigtable_s
{
  char const *name ;
  int number ;
} ;

static struct sigtable_s const table[] =
{
  { "ABRT", SIGABRT },
  { "ALRM", SIGALRM },
  { "BUS", SIGBUS },
  { "CHLD", SIGCHLD },
#ifdef SIGCLD
  { "CLD", SIGCLD },
#endif
  { "CONT", SIGCONT },
#ifdef SIGEMT
  { "EMT", SIGEMT },
#endif
  { "FPE", SIGFPE },
  { "HUP", SIGHUP },
  { "ILL", SIGILL },
#ifdef SIGINFO
  { "INFO", SIGINFO },
#endif
  { "INT", SIGINT },
#ifdef SIGIO
  { "IO", SIGIO },
#endif
#ifdef SIGIOT
  { "IOT", SIGIOT },
#endif
  { "KILL", SIGKILL },
#ifdef SIGLOST
  { "LOST", SIGLOST },
#endif
  { "PIPE", SIGPIPE },
#ifdef SIGPOLL
  { "POLL", SIGPOLL },
#endif
#ifdef SIGPROF
  { "PROF", SIGPROF },
#endif
#ifdef SIGPWR
  { "PWR", SIGPWR },
#endif
  { "QUIT", SIGQUIT },
  { "SEGV", SIGSEGV },
#ifdef SIGSTKFLT
  { "STKFLT", SIGSTKFLT },
#endif
  { "STOP", SIGSTOP },
#ifdef SIGSYS
  { "SYS", SIGSYS },
#endif
  { "TERM", SIGTERM },
#ifdef SIGTRAP
  { "TRAP", SIGTRAP },
#endif
  { "TSTP", SIGTSTP },
  { "TTIN", SIGTTIN },
  { "TTOU", SIGTTOU },
#ifdef SIGUNUSED
  { "UNUSED", SIGUNUSED },
#endif
#ifdef SIGURG
  { "URG", SIGURG },
#endif
  { "USR1", SIGUSR1 },
  { "USR2", SIGUSR2 },
#ifdef SIGVTALRM
  { "VTALRM", SIGVTALRM },
#endif
#ifdef SIGWINCH
  { "WINCH", SIGWINCH },
#endif
#ifdef SIGXCPU
  { "XCPU", SIGXCPU },
#endif
#ifdef SIGXFSZ
  { "XFSZ", SIGXFSZ },
#endif
} ;

int sig_number (char const *name)
{
  struct sigtable_s *p = bsearch(name, table, sizeof(table)/sizeof(struct sigtable_s), sizeof(struct sigtable_s), &stringkey_bcasecmp) ;
  return p ? p->number : 0 ;
}
