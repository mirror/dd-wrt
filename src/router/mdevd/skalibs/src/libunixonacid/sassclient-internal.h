/* ISC license. */

#ifndef SKALIBS_SASSCLIENT_INTERNAL_H
#define SKALIBS_SASSCLIENT_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/tai.h>
#include <skalibs/sassclient.h>

typedef struct sassclient_data_s sassclient_data, *sassclient_data_ref ;
struct sassclient_data_s
{
  void *data ;
  sassclient_cb_func_ref cb ;
} ;

extern int sassclient_cancel_internal (sassclient *, uint32_t, tain const *, tain *) ;
#define asyncnss_cancel_internal_g(a, id, deadline) sassclient_cancel_internal(a, id, (deadline), &STAMP)

#endif
