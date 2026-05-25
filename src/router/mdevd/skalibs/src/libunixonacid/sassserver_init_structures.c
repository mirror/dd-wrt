/* ISC license. */

#include <stdint.h>

#include <skalibs/tai.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/avltree.h>
#include <skalibs/sassserver.h>
#include "sassserver-internal.h"

static void *sassserver_deadline_dtok (uint32_t d, void *aux)
{
  return &GENSETDYN_P(sassserver_query, (gensetdyn *)aux, d)->deadline ;
}

static int sassserver_deadline_cmp (void const *a, void const *b, void *aux)
{
  tain const *aa = a ;
  tain const *bb = b ;
  (void)aux ;
  return tain_less(aa, bb) ? -1 : tain_less(bb, aa) ;
}

static void *sassserver_id_dtok (uint32_t d, void *aux)
{
  return &GENSETDYN_P(sassserver_query, (gensetdyn *)aux, d)->id ;
}

static int sassserver_id_cmp (void const *a, void const *b, void *aux)
{
  uint32_t const *aa = a ;
  uint32_t const *bb = b ;
  (void)aux ;
  return *aa < *bb ? -1 : *aa > *bb ;
}

void sassserver_init_structures (sassserver *a, sassserver_send_func_ref sendf, sassserver_cancel_func_ref cancelf, size_t datasize, free_func_ref cleanupf, void *aux)
{
  a->sendf = sendf ;
  a->cancelf = cancelf ;
  a->datasize = datasize ;
  a->cleanupf = cleanupf ;
  a->aux = aux ;
  gensetdyn_init(&a->queries, sizeof(sassserver_query), 8, 3, 8) ;
  avltree_init(&a->by_deadline, 8, 3, 8, &sassserver_deadline_dtok, &sassserver_deadline_cmp, &a->queries) ;
  avltree_init(&a->by_id, 8, 3, 8, &sassserver_id_dtok, &sassserver_id_cmp, &a->queries) ;
}
