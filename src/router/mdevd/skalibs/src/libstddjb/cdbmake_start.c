/* ISC license. */

#include <unistd.h>

#include <skalibs/buffer.h>
#include <skalibs/genalloc.h>
#include <skalibs/cdbmake.h>

int cdbmake_start (cdbmaker *c, int fd)
{
  c->hplist = genalloc_zero ;
  c->pos = 2048 ;
  buffer_init(&c->b, &buffer_write, fd, c->buf, BUFFER_OUTSIZE) ;
  return lseek(fd, c->pos, SEEK_SET) >= 0 ;
}
