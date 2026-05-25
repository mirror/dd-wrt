/* ISC license. */

#include <string.h>
#include <stdio.h>

#include <skalibs/posixplz.h>
#include <skalibs/djbunix.h>

int openwritevnclose_suffix6 (char const *fn, struct iovec const *v, unsigned int n, devino *devino, unsigned int options, char const *suffix)
{
  size_t len = strlen(fn) ;
  size_t suffixlen = strlen(suffix) ;
  char tmp[len + suffixlen + 1] ;
  memcpy(tmp, fn, len) ;
  memcpy(tmp + len, suffix, suffixlen + 1) ;
  if (!openwritevnclose_unsafe5(tmp, v, n, devino, options)) return 0 ;	
  if (rename(tmp, fn) < 0)
  {
    unlink_void(tmp) ;
    return 0 ;
  }
  return 1 ;
}
