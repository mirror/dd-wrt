/* ISC license. */

#include <stdio.h>
#include <stdarg.h>
#include <skalibs/stralloc.h>
#include <skalibs/bufalloc.h>
#include <skalibs/lolstdio.h>

int vbaprintf (bufalloc *ba, char const *format, va_list args)
{
  int r ;
  {
    va_list ugly ;
    va_copy(ugly, args) ;
    r = vsnprintf(0, 0, format, ugly) ;
    va_end(ugly) ;
  }
  if (r < 0) return r ;
  if (!stralloc_readyplus(&ba->x, r + 1)) return -1 ;
  r = vsnprintf(ba->x.s + ba->x.len, r + 1, format, args) ;
  if (r > 0) ba->x.len += r ;
  return r ;
}
