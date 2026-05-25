/* ISC license. */

#include <sys/stat.h>

#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

static int f (char const *fn, mode_t mode, void *data)
{
  dev_t *devp = data ;
  mode_t m = umask(0) ;
  int r = mknod(fn, (mode & 00777) | S_IFBLK, *devp) ;
  umask(m) ;
  if (r == -1) return -1 ;
  r = open_readb(fn) ;
  if (r == -1) unlink_void(fn) ;
  return r ;
}

int mkbtemp (char *s, mode_t mode, dev_t dev)
{
  return mkfiletemp(s, &f, mode, &dev) ;
}
