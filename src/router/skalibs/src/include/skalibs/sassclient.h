/* ISC license. */

#ifndef SKALIBS_SASSCLIENT_H
#define SKALIBS_SASSCLIENT_H

#include <pthread.h>

#include <skalibs/tai.h>
#include <skalibs/textclient.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/genqdyn.h>

typedef struct sassclient_s sassclient, *sassclient_ref ;
struct sassclient_s
{
  textclient connection ;
  gensetdyn store ;  /* sassclient_data */
  genqdyn results ;  /* char id[4] + char status[4] */
  pthread_mutex_t connection_mutex ;
  pthread_mutex_t results_mutex ;
} ;
#define SASSCLIENT_ZERO { .connection = TEXTCLIENT_ZERO, .store = GENSETDYN_ZERO, .results = GENQDYN_ZERO }

typedef int sassclient_cb_func (char const *, size_t, uint32_t, void *) ;
typedef sassclient_cb_func *sassclient_cb_func_ref ;

extern int sassclient_start (sassclient *, char const *const *, char const *, char const *, tain const *, tain *) ;
#define sassclient_start_g(a, argv, banner1, banner2, deadline) sassclient_start(a, argv, banner1, banner2, (deadline), &STAMP)

extern int sassclient_send (sassclient *, uint32_t *, uint32_t, uint32_t, uint32_t, char const *, size_t, sassclient_cb_func_ref, void *, tain const *, tain *) ;
#define sassclient_send_g(a, id, flags, timeout, opcode, s, len, cb, data, deadline) sassclient_send(a, id, flags, timeout, opcode, s, len, cb, data, (deadline), &STAMP)

extern int sassclient_sendv (sassclient *, uint32_t *, uint32_t, uint32_t, uint32_t, struct iovec const *, unsigned int, sassclient_cb_func_ref, void *, tain const *, tain *) ;
#define sassclient_sendv_g(a, id, flags, timeout, opcode, v, n, cb, data, deadline) sassclient_sendv(a, id, flags, timeout, opcode, v, n, cb, data, (deadline), &STAMP)

extern int sassclient_cancel (sassclient *, uint32_t, tain const *, tain *) ;
#define sassclient_cancel_g(a, id, deadline) sassclient_cancel(a, id, (deadline), &STAMP)

#define sassclient_fd(a) textclient_fd(&(a)->connection)
extern int sassclient_update (sassclient *) ;
extern int sassclient_ack (sassclient *, uint32_t *, int *) ;
extern void sassclient_release (sassclient *, uint32_t) ;

extern void sassclient_end (sassclient *) ;

#endif
