/* ISC license. */

#include <skalibs/bufalloc.h>
#include <skalibs/genwrite.h>

int genwrite_flush_bufalloc (void *target)
{
  return bufalloc_flush((bufalloc *)target) ;
}
