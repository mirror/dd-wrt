/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/stralloc.h>
#include <skalibs/netstring.h>

int mininetstring_read (int fd, stralloc *sa, uint32_t *w)
{
  if (!*w)
  {
    unsigned char pack[2] ;
    switch (fd_read(fd, (char *)pack, 2))
    {
      case -1 : return -1 ;
      case 0 : return 0 ;
      case 1 : *w = ((uint32_t)pack[0] << 8) | (1U << 31) ; break ;
      case 2 : *w = ((uint32_t)pack[0] << 8) | (uint32_t)pack[1] | (1U << 30) ; break ;
      default : return (errno = EDOM, -1) ;
    }
  }
  if (*w & (1U << 31))
  {
    unsigned char c ;
    switch (fd_read(fd, (char *)&c, 1))
    {
      case -1 : return -1 ;
      case 0 : return (errno = EPIPE, -1) ;
      case 1 : *w |= (uint32_t)c | (1U << 30) ; *w &= ~(1U << 31) ; break ;
      default : return (errno = EDOM, -1) ;
    }
  }
  if (*w & (1U << 30))
  {
    if (!stralloc_readyplus(sa, *w & ~(1U << 30))) return -1 ;
    *w &= ~(1U << 30) ;
  }
  {
    size_t r = allread(fd, sa->s + sa->len, *w) ;
    sa->len += r ; *w -= r ;
  }
  return *w ? -1 : 1 ;
}
