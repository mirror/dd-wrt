/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASKEVENT

#include <skalibs/nonposix.h>

#include <errno.h>
#include <pthread.h>
#include <limits.h>

#include <skalibs/fcntl.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>
#include <skalibs/keventbridge.h>

#ifdef SKALIBS_HASKQUEUE1

# define kqueuec() kqueue1(O_CLOEXEC)

#else

static int kqueuec (void)
{
  int kq = kqueue() ;
  if (kq == -1) return -1 ;
  if (coe(kq) == -1)
  {
    fd_close(kq) ;
    return -1 ;
  }
  return kq ;
}

#endif

static void *keventbridge_reader (void *arg)
{
  keventbridge *kbp = arg ;
  struct kevent ke ;
  for (;;)
  {
    int r = kevent(kbp->kq, 0, 0, &ke, 1, 0) ;
    if (r == -1 && errno != EINTR) break ;
    if (r > 0 && allwrite(kbp->p[1], (char *)&ke, sizeof(ke)) < sizeof(ke)) break ;
  }
  fd_close(kbp->p[1]) ;
  kbp->p[1] = -1 ;
  return 0 ;
}

int keventbridge_start (keventbridge *kbp)
{
  pthread_attr_t attr ;
  int e ;

  kbp->kq = kqueuec() ;
  if (pipecoe(kbp->p) == -1) goto errkq ;
  if (ndelay_on(kbp->p[0]) == -1) goto errp ;
  e = pthread_attr_init(&attr) ;
  if (e) { errno = e ; goto errp ; }
#ifdef PTHREAD_STACK_MIN
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN) ;
#endif
  e = pthread_create(&kbp->th, &attr, &keventbridge_reader, kbp) ;
  if (e) { errno = e ; goto errattr ; }
  pthread_attr_destroy(&attr) ;
  return kbp->p[0] ;

 errattr:
  pthread_attr_destroy(&attr) ;
 errp:
  fd_close(kbp->p[1]) ;
  fd_close(kbp->p[0]) ;
 errkq:
  fd_close(kbp->kq) ;
  return -1 ;
}

#endif
