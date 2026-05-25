/* ISC license. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <skalibs/posixplz.h>
#include <skalibs/devino.h>
#include <skalibs/djbunix.h>

#define SUFFIX ":skalibs-openwritenclose:XXXXXX"

int openwritenclose5 (char const *fn, char const *s, size_t n, devino *devino, unsigned int options)
{
  int fd ;
  size_t fnlen = strlen(fn) ;
  char tmp[fnlen + sizeof(SUFFIX)] ;
  memcpy(tmp, fn, fnlen) ;
  memcpy(tmp + fnlen, SUFFIX, sizeof(SUFFIX)) ;
  fd = mkstemp(tmp) ;
  if (fd < 0) return 0 ;
  if (!writenclose_unsafe5(fd, s, n, devino, options)) goto failclose ;
  if (rename(tmp, fn) < 0) goto fail ;
  return 1 ;

 failclose:
  fd_close(fd) ;
 fail:
  unlink_void(tmp) ;
  return 0 ;
}
