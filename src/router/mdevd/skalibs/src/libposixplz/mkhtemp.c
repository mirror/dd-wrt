/* ISC license. */

#include <unistd.h>

#include <skalibs/posixplz.h>

int mkhtemp (char const *src, char *dst)
{
  return mklinktemp(src, dst, &link) ;
}
