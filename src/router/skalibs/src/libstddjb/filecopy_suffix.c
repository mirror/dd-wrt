/* ISC license. */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <skalibs/djbunix.h>

int filecopy_suffix (char const *src, char const *dst, unsigned int mode, char const *suffix)
{
  size_t dstlen = strlen(dst) ;
  size_t suffixlen = strlen(suffix) ;
  char tmp[dstlen + suffixlen + 1] ;
  memcpy(tmp, dst, dstlen) ;
  memcpy(tmp + dstlen, suffix, suffixlen + 1) ;
  if (!filecopy_unsafe(src, tmp, mode)) return 0 ;	
  if (rename(tmp, dst) < 0)
  {
    int e = errno ;
    unlink(tmp) ;
    errno = e ;
    return 0 ;
  }
  return 1 ;
}
