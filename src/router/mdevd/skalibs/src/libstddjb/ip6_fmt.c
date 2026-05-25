/* ISC license. */

#include <skalibs/diuint.h>
#include <skalibs/fmtscan.h>

#define px(c) ((j || (c)) ? (*s++ = fmtscan_asc(c), 1) : 0)

static inline size_t xfmt16 (char *s, char const *key)
{
  size_t j = 0 ;
  j += px((unsigned char)key[0] >> 4) ;
  j += px((unsigned char)key[0] & 15) ;
  j += px((unsigned char)key[1] >> 4) ;
  j += px((unsigned char)key[1] & 15) ;
  return j ? j : (*s = '0', 1) ;
}

static inline unsigned int find_colcol (char const *key, unsigned int *pos)
{
  diuint z[4] = { DIUINT_ZERO, DIUINT_ZERO, DIUINT_ZERO, DIUINT_ZERO } ;
  unsigned int j = 0 ;
  unsigned int max = 0 ;
  int iszero = 0 ;
  unsigned int i = 0 ;
  for ( ; i < 8 ; i++)
  {
    if (key[i<<1] || key[(i<<1)+1])
    {
      if (iszero)
      {
        iszero = 0 ;
        z[j].right = i - z[j].left ;
        if (z[j].right > max) max = z[j].right ;
        j++ ;
      }
    }
    else
    {
      if (!iszero)
      {
        iszero = 1 ;
        z[j].left = i ;
      }
    }
  }
  if (iszero)
  {
    z[j].right = 8 - z[j].left ;
    if (z[j].right > max) max = z[j].right ;
    j++ ;
  }

  if (max >= 2)
    for (i = 0 ; i < j ; i++) if (z[i].right == max) return (*pos = z[i].left, max) ;
  return 0 ; 
}

size_t ip6_fmt (char *s, char const *ip6)
{
  size_t w = 0 ;
  unsigned int pos = 8 ;
  unsigned int len = find_colcol(ip6, &pos) ;
  unsigned int i = 0 ;
  for (; i < 8 ; i++)
  {
    if (i == pos)
    {
      if (!i) s[w++] = ':' ;
      s[w++] = ':' ;
      i += len-1 ;
    }
    else
    {
      w += xfmt16(s + w, ip6 + (i<<1)) ;
      if (i < 7) s[w++] = ':' ;
    }
  }
  return w ;
}
