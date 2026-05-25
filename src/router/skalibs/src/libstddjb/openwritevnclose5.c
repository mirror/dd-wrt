/* ISC license. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <skalibs/posixplz.h>
#include <skalibs/djbunix.h>

#define SUFFIX ":skalibs-openwritevnclose:XXXXXX"

int openwritevnclose5 (char const *fn, struct iovec const *v, unsigned int vlen, devino *devino, unsigned int options)
{
  int fd ;
  size_t fnlen = strlen(fn) ;
  char tmp[fnlen + sizeof(SUFFIX)] ;
  memcpy(tmp, fn, fnlen) ;
  memcpy(tmp + fnlen, SUFFIX, sizeof(SUFFIX)) ;
  fd = mkstemp(tmp) ;
  if (fd < 0) return 0 ;
  if (!writevnclose_unsafe5(fd, v, vlen, devino, options)) goto failclose ;
  if (rename(tmp, fn) < 0) goto fail ;
  return 1 ;

 failclose:
  fd_close(fd) ;
 fail:
  unlink_void(tmp) ;
  return 0 ;
}
