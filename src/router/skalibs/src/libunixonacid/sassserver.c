/* ISC license. */

#include <sys/uio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/alloc.h>
#include <skalibs/error.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/iopause.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/avltree.h>
#include <skalibs/textmessage.h>
#include <skalibs/textclient.h>
#include <skalibs/sass.h>
#include <skalibs/sassserver.h>
#include "sassserver-internal.h"

static void sassserver_sync_answer (sassserver *a, int e)
{
  char pack[4] ;
  uint32_pack_big(pack, (uint32_t)e) ;
  if (!textmessage_put(textmessage_sender_1, pack, 4))
  {
    (*a->cleanupf)(a->aux) ;
    strerr_diefu1sys(111, "textmessage_put") ;
  }
}

static void sassserver_remove (sassserver *a, uint32_t handle)
{
  sassserver_query *p = SASSSERVER_QUERY(a, handle) ;
  avltree_delete(&a->by_deadline, &p->deadline) ;
  avltree_delete(&a->by_id, &p->id) ;
  gensetdyn_delete(&a->queries, handle) ;
}

static inline void sassserver_uniquify (sassserver const *a, tain *deadline)
{
  static tain const nanosec = { .sec = TAI_ZERO, .nano = 1 } ;
  uint32_t dummy ;
  while (avltree_search(&a->by_deadline, deadline, &dummy))
    tain_add(deadline, deadline, &nanosec) ;
}

static int sassserver_parse_protocol (struct iovec const *v, void *aux)
{
  sassserver *a = aux ;
  char const *s = v->iov_base ;
  size_t vlen = v->iov_len ;
  if (vlen-- < 5)
  {
    (*a->cleanupf)(a->aux) ;
    strerr_dief1x(100, "invalid client request") ;
  }
  switch (*s++)
  {
    case '-' : /* cancel */
    {
      uint32_t handle, id ;
      if (vlen != 4)
      {
        (*a->cleanupf)(a->aux) ;
        strerr_dief1x(100, "invalid client request") ;
      }
      uint32_unpack_big(s, &id) ;
      if (!avltree_search(&a->by_id, &id, &handle)) sassserver_sync_answer(a, EINVAL) ;
      (*a->cancelf)(SASSSERVER_QUERY(a, handle)->data) ;
      sassserver_remove(a, handle) ;
      sassserver_sync_answer(a, 0) ;
      break ;
    }
    case '+' : /* send */
    {
      sassserver_query *p ;
      uint32_t handle ;
      uint32_t flags ;
      uint32_t timeout ;
      uint32_t opcode ;
      uint32_t len ;
      int e ;
      if (vlen < 20)
      {
        (*a->cleanupf)(a->aux) ;
        strerr_dief1x(100, "invalid client request") ;
      }
      if (!gensetdyn_new(&a->queries, &handle))
      {
        (*a->cleanupf)(a->aux) ;
        strerr_diefu1sys(111, "gensetdyn_new") ;
      }
      p = SASSSERVER_QUERY(a, handle) ;
      uint32_unpack_big(s, &p->id) ; s += 4 ; vlen -= 4 ;
      uint32_unpack_big(s, &flags) ; s += 4 ; vlen -= 4 ;
      uint32_unpack_big(s, &timeout) ; s += 4 ; vlen -= 4 ;
      uint32_unpack_big(s, &opcode) ; s += 4 ; vlen -= 4 ;
      uint32_unpack_big(s, &len) ; s += 4 ; vlen -= 4 ;
      if (len != vlen)
      {
        (*a->cleanupf)(a->aux) ;
        strerr_dief1x(100, "invalid client request") ;
      }
      if (timeout)
      {
        if (!tain_from_millisecs(&p->deadline, timeout)) strerr_dief1x(100, "invalid client request") ;
        tain_add_g(&p->deadline, &p->deadline) ;
      }
      else tain_add_g(&p->deadline, &tain_infinite_relative) ;
      sassserver_uniquify(a, &p->deadline) ;
      if (!avltree_insert(&a->by_deadline, handle)
       || !avltree_insert(&a->by_id, handle))
      {
        (*a->cleanupf)(a->aux) ;
        strerr_diefu1sys(111, "avltree_insert") ;
      }
      if (!p->data)
      {
        p->data = alloc(a->datasize) ;
        if (!p->data)
        {
          (*a->cleanupf)(a->aux) ;
          strerr_diefu1sys(111, "alloc") ;
        }
        memset(p->data, 0, a->datasize) ;
      }
      e = (*a->sendf)(p->data, handle, flags, opcode, s, len) ;
      if (e) sassserver_remove(a, handle) ;
      sassserver_sync_answer(a, e) ;
      break ;
    }
    default :
      (*a->cleanupf)(a->aux) ;
      strerr_dief1x(100, "invalid client request") ;
  }
  return 1 ;
}


void *sassserver_data (sassserver const *a, uint32_t handle)
{
  return SASSSERVER_QUERY(a, handle)->data ;
}

void sassserver_async_failure (sassserver *a, uint32_t handle, int e)
{
  sassserver_query *p = SASSSERVER_QUERY(a, handle) ;
  char pack[8] ;
  uint32_pack_big(pack, p->id) ;
  uint32_pack_big(pack + 4, (uint32_t)e) ;
  if (!textmessage_put(textmessage_sender_x, pack, 8))
  {
    (*a->cleanupf)(a->aux) ;
    strerr_diefu1sys(111, "textmessage_put") ;
  }
  sassserver_remove(a, handle) ;
}

void sassserver_async_successv (sassserver *a, uint32_t handle, uint32_t flags, struct iovec const *v, unsigned int n)
{
  sassserver_query *p = SASSSERVER_QUERY(a, handle) ;
  char pack[8] = "\0\0\0\0\0\0\0" ;
  struct iovec vv[n+1] ;
  vv[0].iov_base = pack ; vv[0].iov_len = 8 ;
  for (unsigned int i = 0 ; i < n ; i++) vv[i+1] = v[i] ;
  uint32_pack_big(pack, p->id) ;
  if (!textmessage_putv(textmessage_sender_x, vv, n+1))
  {
    (*a->cleanupf)(a->aux) ;
    strerr_diefu1sys(111, "textmessage_putv") ;
  }
  if (!(flags & SASS_FLAG_KEEP)) sassserver_remove(a, handle) ;
}

void sassserver_async_success (sassserver *a, uint32_t handle, uint32_t flags, char const *s, size_t len)
{
  struct iovec v = { .iov_base = (char *)s, .iov_len = len } ;
  sassserver_async_successv(a, handle, flags, &v, 1) ;
}

unsigned int sassserver_prepare_iopause (sassserver const *a, iopause_fd *x, tain *deadline)
{
  uint32_t i ;
  if (avltree_min(&a->by_deadline, &i)) tain_earliest1(deadline, &SASSSERVER_QUERY(a, i)->deadline) ;
  x[0].fd = 0 ;
  x[0].events = IOPAUSE_READ ;
  x[1].fd = 1 ;
  x[1].events = textmessage_sender_isempty(textmessage_sender_1) ? 0 : IOPAUSE_WRITE ;
  x[2].fd = textmessage_sender_fd(textmessage_sender_x) ;
  x[2].events = textmessage_sender_isempty(textmessage_sender_x) ? 0 : IOPAUSE_WRITE ;
  return 3 ;
}

void sassserver_timeout (sassserver *a)
{
  uint32_t i ;
  while (avltree_min(&a->by_deadline, &i))
  {
    sassserver_query *p = SASSSERVER_QUERY(a, i) ;
    if (tain_future(&p->deadline)) break ;
    avltree_delete(&a->by_deadline, &p->deadline) ;
    avltree_delete(&a->by_id, &p->id) ;
    (*a->cancelf)(p->data) ;
    sassserver_async_failure(a, p->id, ETIMEDOUT) ;
    gensetdyn_delete(&a->queries, i) ;
  }
}

void sassserver_write_event (sassserver *a, iopause_fd const *x)
{
  if (x[1].revents & IOPAUSE_WRITE)
    if (!textmessage_sender_flush(textmessage_sender_1) && !error_isagain(errno))
    {
      (*a->cleanupf)(a->aux) ;
      strerr_diefu1sys(111, "flush stdout") ;
    }
  if (x[2].revents & IOPAUSE_WRITE)
    if (!textmessage_sender_flush(textmessage_sender_x) && !error_isagain(errno))
    {
      (*a->cleanupf)(a->aux) ;
      strerr_diefu1sys(111, "flush asyncout") ;
    }
}

int sassserver_read_event (sassserver *a, iopause_fd const *x)
{
  if (!textmessage_receiver_isempty(textmessage_receiver_0) || x[0].revents & IOPAUSE_READ)
  {
    if (textmessage_handle(textmessage_receiver_0, &sassserver_parse_protocol, a) == -1)
    {
      if (errno == EPIPE) return 1 ;
      (*a->cleanupf)(a->aux) ;
      strerr_diefu1sys(111, "read messages from client") ;
    }
  }
  return 0 ;
}
