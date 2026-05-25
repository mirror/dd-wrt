/* ISC license. */

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <skalibs/posixplz.h>
#include <skalibs/unix-transactional.h>

int atomic_symlink4 (char const *target, char const *name, char *oldbuf, size_t oldlen)
{
  {
    int e = errno ;
    if (symlink(target, name) == 0)
    {
      if (oldbuf && oldlen) *oldbuf = 0 ;
      return 1 ;
    }
    if (errno != EEXIST) return 0 ;
    errno = e ;
  }
  {
    size_t namelen = strlen(name) ;
    char tmp[namelen + 32] ;
    if (oldbuf && oldlen)
    {
      ssize_t r = readlink(name, oldbuf, oldlen) ;
      if (r == -1) return 0 ;
      if (r >= oldlen) return (errno = ENAMETOOLONG, 0) ;
      oldbuf[r] = 0 ;
    }
    memcpy(tmp, name, namelen) ;
    memcpy(tmp + namelen, ":skalibs-atomic_symlink4:XXXXXX", 32) ;
    if (mkltemp(target, tmp) == -1) return 0 ;
    if (rename(tmp, name) == -1)
    {
      unlink_void(tmp) ;
      return 0 ;
    }
  }
  return 1 ;
}
