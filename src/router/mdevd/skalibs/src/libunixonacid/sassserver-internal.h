/* ISC license. */

#ifndef SKALIBS_SASSSERVER_INTERNAL_H
#define SKALIBS_SASSSERVER_INTERNAL_H

#include <stdint.h>

#include <skalibs/functypes.h>
#include <skalibs/tai.h>
#include <skalibs/sassserver.h>

typedef struct sassserver_query_s sassserver_query, sassserver_query_ref ;
struct sassserver_query_s
{
  uint32_t id ;
  tain deadline ;
  void *data ;
} ;

#define SASSSERVER_QUERY(a, i) GENSETDYN_P(sassserver_query, &(a)->queries, i)

extern void sassserver_init_structures (sassserver *a, sassserver_send_func_ref, sassserver_cancel_func_ref, size_t, free_func_ref, void *) ;

#endif
