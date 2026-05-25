/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/env.h>
#include <skalibs/direntry.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/skamisc.h>
#include "envdir-internal.h"

#define N 4096

int envdir_internal_noclamp (char const *path, stralloc *modifs, unsigned int options, char nullis)
{
  unsigned int n = 0 ;
  size_t pathlen = strlen(path) ;
  size_t modifbase = modifs->len ;
  int wasnull = !modifs->s ;
  int fd ;
  DIR *dir = opendir(path) ;
  if (!dir) return -1 ;
  for (;;)
  {
    direntry *d ;
    size_t len, pos ;
    errno = 0 ;
    d = readdir(dir) ;
    if (!d) break ;
    if (d->d_name[0] == '.') continue ;
    len = strlen(d->d_name) ;
    if (str_chr(d->d_name, '=') < len) continue ;
    {
      char tmp[pathlen + len + 2] ;
      memcpy(tmp, path, pathlen) ;
      tmp[pathlen] = '/' ;
      memcpy(tmp + pathlen + 1, d->d_name, len + 1) ;
      fd = openc_readb(tmp) ;
    }
    if (fd < 0)
    {
      if (errno == ENOENT) errno = EIDRM ;
      goto err ;
    }
    if (!stralloc_catb(modifs, d->d_name, len) || !stralloc_catb(modifs, "=", 1)) goto errfd ;
    pos = modifs->len ;
    if (options & SKALIBS_ENVDIR_VERBATIM)
    {
      if (!slurp(modifs, fd)) goto errfd ;
      if (modifs->len == pos) modifs->len = pos - 1 ;
      else if (!(options & SKALIBS_ENVDIR_NOCHOMP) && modifs->s[modifs->len - 1] == '\n') modifs->len-- ;
    }
    else
    {
      int r ;
      char buf[N] ;
      buffer b = BUFFER_INIT(&buffer_read, fd, buf, N) ;
      r = skagetln(&b, modifs, '\n') ;
      if (r == -1)
      {
        if (errno != EPIPE) goto errfd ;
        if (!(options & SKALIBS_ENVDIR_NOCHOMP)) modifs->len = pos ;
        modifs->len++ ;
      }
      if (!r) modifs->len = pos - 1 ;
      else
      {
        modifs->len-- ;
        if (!(options & SKALIBS_ENVDIR_NOCHOMP))
        {
          while (modifs->len-- > pos)
          {
            char c = modifs->s[modifs->len] ;
            if ((c != ' ') && (c != '\t') && (c != '\r')) break ;
          }
          modifs->len++ ;
        }
      }
    }
    fd_close(fd) ;
    for (; pos < modifs->len ; pos++) if (!modifs->s[pos]) modifs->s[pos] = nullis ;
    if (!stralloc_0(modifs)) goto err ;
    n++ ;
  }
  if (errno) goto err ;
  dir_close(dir) ;
  return n ;

 errfd:
  fd_close(fd) ;
 err:
  dir_close(dir) ;
  if (wasnull) stralloc_free(modifs) ; else modifs->len = modifbase ;
  return -1 ;
}
