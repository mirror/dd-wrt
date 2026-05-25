/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/posixplz.h>

static int f (char const *fn, mode_t mode, void *data)
{
  (void)mode ;
  (void)data ;
  return access(fn, F_OK) == 0 ? (errno = EEXIST, -1) :
    errno == ENOENT ? (errno = 0, 0) : -1 ;
}

int mkntemp (char *s)
{
  return mkfiletemp(s, &f, 0, 0) ;
}
