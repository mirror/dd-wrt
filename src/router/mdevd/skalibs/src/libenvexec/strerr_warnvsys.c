/* ISC license. */

/* MT-unsafe */

#include <string.h>
#include <errno.h>

#include <skalibs/buffer.h>
#include <skalibs/strerr.h>

void strerr_warnvsys (char const *const *v, unsigned int n)
{
  int e = errno ;
  for (unsigned int i = 0 ; i < n ; i++)
    if (v[i]) buffer_puts(buffer_2, v[i]) ;
  buffer_put(buffer_2, ": ", 2) ;
  buffer_puts(buffer_2, strerror(e)) ;
  buffer_putflush(buffer_2, "\n", 1) ;
  errno = e ;
}
