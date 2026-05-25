/* ISC license. */

#include <skalibs/posixplz.h>
#include <skalibs/cdb.h>

extern void cdb_free (cdb *c)
{
  if (c->map)
  {
    munmap_void((void *)c->map, c->size) ;
    c->map = 0 ;
  }
}
