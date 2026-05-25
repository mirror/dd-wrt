 /* ISC license. */

#ifndef SKALIBS_UNIXCONNECTION_H
#define SKALIBS_UNIXCONNECTION_H

#include <skalibs/unixmessage.h>

typedef struct unixconnection_s unixconnection, *unixconnection_ref ;
struct unixconnection_s
{
  unixmessage_sender out ;
  unixmessage_receiver in ;
  char mainbuf[UNIXMESSAGE_BUFSIZE] ;
  char auxbuf[UNIXMESSAGE_AUXBUFSIZE] ;
} ;
#define UNIXCONNECTION_ZERO { .out = UNIXMESSAGE_SENDER_ZERO, .in = UNIXMESSAGE_RECEIVER_ZERO }
extern unixconnection const unixconnection_zero ;

extern void unixconnection_init (unixconnection *, int, int) ;
extern void unixconnection_init_withclosecb (unixconnection *, int, int, unixmessage_sender_closecb_func_ref, void *) ;
extern void unixconnection_free (unixconnection *) ;

#define unixconnection_flush(io) unixmessage_sender_flush(&(io)->out)
#define unixconnection_receive(io, m) unixmessage_receive(&(io)->in, m)

#endif
