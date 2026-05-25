/* ISC license. */

#include <skalibs/sysdeps.h>

#if defined(SKALIBS_HASARC4RANDOM) && !defined(__linux__)

#include <skalibs/nonposix.h>
#include <stdlib.h>
#include <skalibs/random.h>

void random_buf_early (char *s, size_t n)
{
  arc4random_buf(s, n) ;
}

#elif defined(SKALIBS_HASGETRANDOM) && defined(SKALIBS_HASGRNDINSECURE)

#include <errno.h>
#include <sys/random.h>

#include <skalibs/random.h>

void random_buf_early (char *s, size_t n)
{
  static int broken = 0 ;
  if (broken) goto bleh ;
  int e = errno ;
  while (n)
  {
    ssize_t r = getrandom(s, n, GRND_INSECURE) ;
    if (r == -1)
    {
      if (errno != EINTR) goto breakit ;
      else continue ;
    }
    s += r ;
    n -= r ;
  }
  errno = e ;
  return ;
 breakit:
  broken = 1 ;
  errno = e ;
 bleh:
  random_devurandom(s, n) ;
}

#elif defined(SKALIBS_HASDEVURANDOM)

#include <skalibs/random.h>

void random_buf_early (char *s, size_t n)
{
  random_devurandom(s, n) ;
}

#else

#include <skalibs/surf.h>
#include <skalibs/random.h>

void random_buf_early (char *s, size_t n)
{
  autosurf(s, n) ;
}

#endif
