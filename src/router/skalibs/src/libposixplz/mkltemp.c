/* ISC license. */

#include <unistd.h>

#include <skalibs/posixplz.h>

int mkltemp (char const *src, char *dst)
{
  return mklinktemp(src, dst, &symlink) ;
}
