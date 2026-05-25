/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <skalibs/bytestr.h>
#include <skalibs/env.h>
#include <skalibs/direntry.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>
#include "envdir-internal.h"

#define MAXVARSIZE 4095

int envdir_internal_clamp (char const *path, stralloc *modifs, unsigned int options, char nullis)
{
  char buf[MAXVARSIZE + 1] ;
  unsigned int n = 0 ;
  size_t pathlen = strlen(path) ;
  size_t modifbase = modifs->len ;
  int wasnull = !modifs->s ;
  DIR *dir = opendir(path) ;
  if (!dir) return -1 ;
  for (;;)
  {
    direntry *d ;
    size_t len ;
    ssize_t r ;
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
      r = openreadnclose(tmp, buf, MAXVARSIZE) ;
    }
    if (r < 0)
    {
      if (errno == ENOENT) errno = EIDRM ;
      goto err ;
    }
    else if (r > 0)
    {
      if (options & SKALIBS_ENVDIR_VERBATIM)
      {
        if (!(options & SKALIBS_ENVDIR_NOCHOMP) && (buf[r-1] == '\n')) r-- ;
      }
      else
      {
        r = byte_chr(buf, r, '\n') ;
        if (!(options & SKALIBS_ENVDIR_NOCHOMP))
        {
          while (r--) if ((buf[r] != ' ') && (buf[r] != '\t') && (buf[r] != '\r')) break ;
          r++ ;
        }
      }
      {
        size_t i = 0 ;
        for (; i < (size_t)r ; i++) if (!buf[i]) buf[i] = nullis ;
      }
      buf[r++] = 0 ;
      if (!env_addmodif(modifs, d->d_name, buf)) goto err ;
    }
    else if (!env_addmodif(modifs, d->d_name, 0)) goto err ;
    n++ ;
  }
  if (errno) goto err ;
  dir_close(dir) ;
  return n ;

 err:
  dir_close(dir) ;
  if (wasnull) stralloc_free(modifs) ; else modifs->len = modifbase ;
  return -1 ;
}
