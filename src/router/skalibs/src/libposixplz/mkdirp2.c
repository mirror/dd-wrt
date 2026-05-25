/* ISC license. */

#include <skalibs/bsdsnowflake.h>

#include <string.h>
#include <errno.h>

#include <skalibs/stat.h>
#include <skalibs/posixplz.h>

static int domkdir (char const *s, mode_t mode)
{
  return !mkdir(s, mode) || errno == EEXIST ;
}

int mkdirp2 (char *s, mode_t mode)
{
  mode_t m = umask(0) ;
  size_t len = strlen(s) ;
  size_t i = 2 ;
  for (; i < len ; i++) if (s[i] == '/')
  {
    s[i] = 0 ;
    if (!domkdir(s, 02755)) goto err ;
    s[i] = '/' ;
  }
  if (!domkdir(s, mode)) goto err ;
  umask(m) ;
  return 0 ;

 err:
  umask(m) ;
  return -1 ;
}
