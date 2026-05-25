/* ISC license. */

#include <stdarg.h>
#include <skalibs/bufalloc.h>
#include <skalibs/lolstdio.h>

int baprintf (bufalloc *ba, char const *format, ...)
{
  va_list args ;
  int r ;
  va_start(args, format) ;
  r = vbaprintf(ba, format, args) ;
  va_end(args) ;
  return r ;
}
