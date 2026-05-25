/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sadirname (stralloc *sa, char const *s, size_t len)
{
  if (!len) return stralloc_catb(sa, ".", 1) ;
  while (len && (s[len-1] == '/')) len-- ;
  if (!len) return stralloc_catb(sa, "/", 1) ;
  {
    size_t i = byte_rchr(s, len, '/') ;
    return (i == len) ? stralloc_catb(sa, ".", 1) :
           (i == 0)   ? stralloc_catb(sa, "/", 1) :
                        stralloc_catb(sa, s, i) ;
  }
}
