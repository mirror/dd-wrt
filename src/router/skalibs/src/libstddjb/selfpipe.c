/* ISC license. */

/* MT-unsafe */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASSIGNALFD

#include <errno.h>
#include <signal.h>
#include <sys/signalfd.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/sig.h>
#include <skalibs/djbunix.h>

struct selfpipe_s
{
  sigset_t caught ;
  int fd ;
} ;

static struct selfpipe_s sp = { .fd = -1 } ;

int selfpipe_fd ()
{
  return sp.fd ;
}

int selfpipe_read ()
{
  struct signalfd_siginfo buf ;
  ssize_t r = sanitize_read(fd_read(sp.fd, (char *)&buf, sizeof(struct signalfd_siginfo))) ;
  return (r <= 0) ? r : buf.ssi_signo ;
}

int selfpipe_trap (int sig)
{
  sigset_t set = sp.caught ;
  sigset_t old ;
  if (sp.fd == -1) return (errno = EBADF, 0) ;
  if (sigaddset(&set, sig) == -1 || sigprocmask(SIG_BLOCK, &set, &old) == -1) return 0 ;
  if (signalfd(sp.fd, &set, SFD_NONBLOCK | SFD_CLOEXEC) == -1)
  {
    int e = errno ;
    sigprocmask(SIG_SETMASK, &old, 0) ;
    errno = e ;
    return 0 ;
  }
  sp.caught = set ;
  return 1 ;
}

int selfpipe_trapset (sigset_t const *set)
{
  sigset_t old ;
  if (sp.fd == -1) return (errno = EBADF, 0) ;
  if (sigprocmask(SIG_SETMASK, set, &old) == -1) return 0 ;
  if (signalfd(sp.fd, set, SFD_NONBLOCK | SFD_CLOEXEC) == -1)
  {
    int e = errno ;
    sigprocmask(SIG_SETMASK, &old, 0) ;
    errno = e ;
    return 0 ;
  }
  sp.caught = *set ;
  return 1 ;
}

void selfpipe_finish ()
{
  int e = errno ;
  fd_close(sp.fd) ; sp.fd = -1 ;
  sigprocmask(SIG_UNBLOCK, &sp.caught, 0) ;
  sigemptyset(&sp.caught) ;
  errno = e ;
}

int selfpipe_init ()
{
  sigemptyset(&sp.caught) ;
  sig_blocknone() ;
  sp.fd = signalfd(sp.fd, &sp.caught, SFD_NONBLOCK | SFD_CLOEXEC) ;
  return sp.fd ;
}

#else

#include <skalibs/nonposix.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <skalibs/nsig.h>
#include <skalibs/sig.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>

struct selfpipe_s
{
  sigset_t caught ;
  int fd[2] ;
} ;

static struct selfpipe_s sp = { .fd = { -1, -1 } } ;

static void selfpipe_tophalf (int s)
{
  int e = errno ;
  unsigned char c = (unsigned char)s ;
  write(sp.fd[1], (char *)&c, 1) ;
  errno = e ;
}

int selfpipe_fd ()
{
  return sp.fd[0] ;
}

int selfpipe_read ()
{
  char c ;
  ssize_t r = sanitize_read((fd_read(sp.fd[0], &c, 1))) ;
  return (r <= 0) ? r : c ;
}

int selfpipe_trap (int sig)
{
  if (sp.fd[0] == -1) return (errno = EBADF, 0) ;
  if (!sig_catch(sig, &selfpipe_tophalf)) return 0 ;
  sigaddset(&sp.caught, sig) ;
  sig_unblock(sig) ;
  return 1 ;
}

int selfpipe_trapset (sigset_t const *set)
{
  unsigned int i = 1 ;
  if (sp.fd[0] == -1) return (errno = EBADF, 0) ;
  for (; i < SKALIBS_NSIG ; i++)
  {
    int h = sigismember(set, i) ;
    if (h < 0) continue ;
    if (h)
    {
      if (!sig_catch(i, &selfpipe_tophalf)) goto err ;
    }
    else if (sigismember(&sp.caught, i))
    {
      if (!sig_restore(i)) goto err ;
    }
  }
  sig_blocknone() ;
  sp.caught = *set ;
  return 1 ;

 err:
  sig_restoreto(set, i) ;
  return 0 ;
}

void selfpipe_finish ()
{
  int e = errno ;
  sigprocmask(SIG_BLOCK, &sp.caught, 0) ;
  sig_restoreto(&sp.caught, SKALIBS_NSIG) ;
  fd_close(sp.fd[1]) ;
  fd_close(sp.fd[0]) ;
  sigprocmask(SIG_UNBLOCK, &sp.caught, 0) ;
  sigemptyset(&sp.caught) ;
  sp.fd[0] = sp.fd[1] = -1 ;
  errno = e ;
}

int selfpipe_init ()
{
  if (sp.fd[0] >= 0) selfpipe_finish() ;
  else sigemptyset(&sp.caught) ;
  sig_blocknone() ;
  return pipenbcoe(sp.fd) < 0 ? -1 : sp.fd[0] ;
}

#endif
