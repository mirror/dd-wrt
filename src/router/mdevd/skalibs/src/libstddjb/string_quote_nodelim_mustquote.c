/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <skalibs/fmtscan.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

int string_quote_nodelim_mustquote_options (stralloc *sa, char const *s, size_t len, char const *delim, size_t delimlen, uint32_t options)
{
  char class[256] = "dddddddaaaaaaaddddddddddddddddddcccccccccccccccceeeeeeeeeeccccccccccccccccccccccccccccccccccbcccceeeeeecccccccecccececececcccccddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd" ;
  size_t base = sa->len ;
  size_t i = 0 ;
  int wasnull = !sa->s ;

  if (options & 1) class[' '] = 'f' ;

  for (; i < delimlen ; i++)
    if (class[(uint8_t)delim[i]] == 'c' || class[(uint8_t)delim[i]] == 'b')
      class[(uint8_t)delim[i]] = 'b' ;
    else return (errno = EINVAL, 0) ;

  for (i = 0 ; i < len ; i++)
  {
    switch (class[(unsigned char)s[i]])
    {
      case 'a' :
      {
        static char const tab[7] = "abtnvfr" ;
        char fmt[2] = "\\" ;
        fmt[1] = tab[s[i] - 7] ;
        if (!stralloc_catb(sa, fmt, 2)) goto err ;
        break ;
      }
      case 'b' :
      {
        char fmt[2] = "\\" ;
        fmt[1] = s[i] ;
        if (!stralloc_catb(sa, fmt, 2)) goto err ;
        break ;
      }
      case 'c' :
      case 'e' :
        if (!stralloc_catb(sa, s+i, 1)) goto err ;
        break ;
      case 'd' :
      {
        char fmt[5] = "\\0x" ;
        ucharn_fmt(fmt+3, s+i, 1) ;
        if (!stralloc_catb(sa, fmt, 5)) goto err ;
        break ;
      }
      case 'f' :
        if (!stralloc_catb(sa, "\\s", 2)) goto err ;
        break ;
      default : errno = EFAULT ; goto err ; /* can't happen */
    }
  }
  return 1 ;

 err:
  if (wasnull) stralloc_free(sa) ; else sa->len = base ;
  return 0 ;
}
