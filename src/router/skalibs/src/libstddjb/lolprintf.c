/* ISC license. */

#include <stdarg.h>
#include <skalibs/buffer.h>
#include <skalibs/lolstdio.h>

int lolprintf (char const *format, ...)
{
  va_list args ;
  int r ;
  va_start(args, format) ;
  r = vbprintf(buffer_1, format, args) ;
  va_end(args) ;
  return r ;
}
