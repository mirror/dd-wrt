/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASEXPLICIT_BZERO

#include <skalibs/nonposix.h>
#include <string.h>
#include <strings.h>
#include <skalibs/bytestr.h>

void byte_zzero (char *s, size_t n)
{
  explicit_bzero(s, n) ;
}

#else

#include <string.h>
#include <skalibs/gccattributes.h>
#include <skalibs/bytestr.h>

void _byte_zzero_hook (char *, size_t) gccattr_weak ;

gccattr_weak void _byte_zzero_hook (char *s, size_t n)
{
}

void byte_zzero (char *s, size_t n)
{
  memset(s, 0, n) ;
  _byte_zzero_hook(s, n) ;
}

#endif
