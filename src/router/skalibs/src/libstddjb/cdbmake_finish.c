/* ISC license. */

#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/diuint32.h>
#include <skalibs/buffer.h>
#include <skalibs/genalloc.h>
#include <skalibs/cdbmake.h>
#include "cdbmake-internal.h"

int cdbmake_finish (cdbmaker *c)
{
  uint32_t count[256] ;
  uint32_t start[256] ;
  char final[2048] ;
  unsigned int size = 1 ;
  unsigned int n = genalloc_len(diuint32, &c->hplist) ;
  unsigned int i = 0 ;
  diuint32 *hp = genalloc_s(diuint32, &c->hplist) ;

  for (; i < 256 ; i++) count[i] = 0 ;
  for (i = 0 ; i < n ; i++) ++count[hp[i].left & 255] ;

  {
    uint32_t u = 0 ;
    for (i = 0 ; i < 256 ; i++) start[i] = u += count[i] ; /* bounded by n */
    for (i = 0 ; i < 256 ; i++)
    {
      u = count[i] << 1 ;
      if (u > size) size = u ;
    }
    size += n ; /* no overflow possible up to now */
    u = 0xffffffffUL ; u /= sizeof(diuint32) ;
    if (size > u) return (errno = ENOMEM, 0) ;
  }
  i = n ;
  {
    diuint32 split[size] ;
    while (i--) split[--start[hp[i].left & 255]] = hp[i] ;
    genalloc_free(diuint32, &c->hplist) ;
    hp = split + n ;

    for (i = 0 ; i < 256 ; ++i)
    {
      char buf[8] ;
      uint32_t k = count[i] ;
      uint32_t len = k << 1 ;  /* no overflow possible */
      diuint32 *p = split + start[i] ;

      uint32_pack(final + (i << 3), c->pos) ;
      uint32_pack(final + (i << 3) + 4, len) ;

      for (uint32_t j = 0 ; j < len ; j++) hp[j].left = hp[j].right = 0 ;
      for (uint32_t j = 0 ; j < k ; j++)
      {
        uint32_t where = (p->left >> 8) % len ;
        while (hp[where].right) if (++where == len) where = 0 ;
        hp[where] = *p++ ;
      }

      for (uint32_t j = 0 ; j < len ; j++)
      {
        uint32_pack(buf, hp[j].left) ;
        uint32_pack(buf + 4, hp[j].right) ;
        if (buffer_put(&c->b, buf, 8) < 0) return 0 ;
        if (!cdbmake_posplus(c, 8)) return 0 ;
      }
    }
  }

  if (!buffer_flush(&c->b)
   || lseek(buffer_fd(&c->b), 0, SEEK_SET) == -1
   || buffer_putflush(&c->b, final, 2048) < 2048) return 0 ;

  genalloc_free(diuint32, &c->hplist) ;
  return 1 ;
}
