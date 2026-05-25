/* ISC license. */

#ifndef SKALIBS_KOLBAK_H
#define SKALIBS_KOLBAK_H

#include <sys/types.h>

#include <skalibs/unixmessage.h>

typedef struct kolbak_closure_s kolbak_closure, *kolbak_closure_ref ;
struct kolbak_closure_s
{
  unixmessage_handler_func_ref f ;
  void *data ;
} ;
#define KOLBAK_CLOSURE_ZERO { .f = 0, .data = 0 }

typedef struct kolbak_queue_s kolbak_queue, *kolbak_queue_ref ;
struct kolbak_queue_s
{
  kolbak_closure *x ;
  size_t n ;
  size_t head ;
  size_t tail ;
} ;
#define KOLBAK_QUEUE_ZERO { .x = 0, .n = 0, .head = 0, .tail = 0 }
#define KOLBAK_QUEUE_INIT(s, len) { .x = (s), .n = (len), .head = 0, .tail = 0 }

extern int kolbak_queue_init (kolbak_queue *, kolbak_closure *, size_t) ;
extern int kolbak_enqueue (kolbak_queue *, unixmessage_handler_func_ref, void *) ;
extern int kolbak_unenqueue (kolbak_queue *) ;
extern int kolbak_call (unixmessage const *, kolbak_queue *) ; 

#endif
