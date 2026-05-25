/* ISC license. */

#include <string.h>
#include "genqdyn-internal.h"

void genqdyn_clean (genqdyn *g)
{
  memmove(g->queue.s, g->queue.s + g->head, g->queue.len - g->head) ;
  g->queue.len -= g->head ;
  g->head = 0 ;
}
