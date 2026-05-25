/* ISC license. */

#ifndef SKALIBS_TEXTMESSAGE_H
#define SKALIBS_TEXTMESSAGE_H

#include <sys/uio.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/bufalloc.h>
#include <skalibs/buffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/tai.h>

#define TEXTMESSAGE_MAXREADS 128
#define TEXTMESSAGE_MAXLEN 0x01000000U


 /* Sender */

typedef struct textmessage_sender_s textmessage_sender, *textmessage_sender_ref ;
struct textmessage_sender_s
{
  bufalloc out ;
} ;
#define TEXTMESSAGE_SENDER_ZERO { BUFALLOC_ZERO }
extern textmessage_sender const textmessage_sender_zero ;
#define TEXTMESSAGE_SENDER_INIT(fd) { BUFALLOC_INIT(&fd_write, (fd)) }

#define textmessage_sender_init(ts, fd) bufalloc_init(&(ts)->out, &fd_write, fd)
#define textmessage_sender_free(ts) bufalloc_free(&(ts)->out)
#define textmessage_sender_fd(ts) bufalloc_fd(&(ts)->out)
extern int textmessage_sender_getfd (textmessage_sender const *) gccattr_pure ;
#define textmessage_sender_isempty(ts) bufalloc_isempty(&(ts)->out)

extern int textmessage_put (textmessage_sender *, char const *, size_t) ;
extern int textmessage_putv (textmessage_sender *, struct iovec const *, unsigned int) ;

extern int textmessage_sender_flush (textmessage_sender *) ;
extern int textmessage_sender_timed_flush (textmessage_sender *, tain const *, tain *) ;
#define textmessage_sender_timed_flush_g(ts, deadline) textmessage_sender_timed_flush(ts, (deadline), &STAMP)

#define textmessage_send(ts, s, len) (textmessage_put(ts, s, len) && textmessage_sender_flush(ts))
#define textmessage_sendv(ts, v, n) (textmessage_putv(ts, v, n) && textmessage_sender_flush(ts))
#define textmessage_timed_send(ts, s, len, deadline, stamp) (textmessage_put(ts, s, len) && textmessage_sender_timed_flush(ts, deadline, stamp))
#define textmessage_timed_sendv(ts, v, n, deadline, stamp) (textmessage_putv(ts, v, n) && textmessage_sender_timed_flush(ts, deadline, stamp))
#define textmessage_timed_send_g(ts, s, len, deadline) textmessage_timed_send(ts, s, len, (deadline), &STAMP)
#define textmessage_timed_sendv_g(ts, v, n, deadline) textmessage_timed_sendv(ts, v, n, (deadline), &STAMP)


 /* Receiver */

typedef struct textmessage_receiver_s textmessage_receiver, *textmessage_receiver_ref ;
struct textmessage_receiver_s
{
  buffer in ;
  stralloc indata ;
  uint32_t wanted ;
  uint32_t max ;
} ;
#define TEXTMESSAGE_RECEIVER_ZERO { BUFFER_ZERO, STRALLOC_ZERO, 0, 0 }
extern textmessage_receiver const textmessage_receiver_zero ;
#define TEXTMESSAGE_RECEIVER_INIT(fd, buf, len, n) { BUFFER_INIT(&buffer_read, (fd), buf, len), STRALLOC_ZERO, 0, n }

extern int textmessage_receiver_init (textmessage_receiver *, int, char *, size_t, uint32_t) ;
extern void textmessage_receiver_free (textmessage_receiver *) ;
#define textmessage_receiver_fd(tr) buffer_fd(&(tr)->in)
#define textmessage_receiver_isempty(tr) buffer_isempty(&(tr)->in)
#define textmessage_receiver_isfull(tr) buffer_isfull(&(tr)->in)

extern int textmessage_receiver_hasmsginbuf (textmessage_receiver const *) gccattr_pure ;

extern int textmessage_receive (textmessage_receiver *, struct iovec *) ;
extern int textmessage_timed_receive (textmessage_receiver *, struct iovec *, tain const *, tain *) ;
#define textmessage_timed_receive_g(tr, v, deadline) textmessage_timed_receive(tr, v, (deadline), &STAMP)

typedef int textmessage_handler_func (struct iovec const *, void *) ;
typedef textmessage_handler_func *textmessage_handler_func_ref ;

extern int textmessage_handle (textmessage_receiver *, textmessage_handler_func_ref, void *) ;
extern int textmessage_timed_handle (textmessage_receiver *, textmessage_handler_func_ref, void *, tain const *, tain *) ;
#define textmessage_timed_handle_g(tr, f, p, deadline) textmessage_timed_handle(tr, f, p, (deadline), &STAMP)


 /* Creating new textmessage channels via fd-passing over a socket */

extern int textmessage_create_send_channel (int, textmessage_sender *, char const *, size_t, tain const *, tain *) ;
extern int textmessage_recv_channel (int, textmessage_receiver *, char *, size_t, char const *, size_t, tain const *, tain *) ;


 /* Globals */

extern textmessage_receiver textmessage_receiver_0_ ;
#define textmessage_receiver_0 (&textmessage_receiver_0_)

extern textmessage_sender textmessage_sender_1_ ;
#define textmessage_sender_1 (&textmessage_sender_1_)

extern textmessage_sender textmessage_sender_x_ ;
#define textmessage_sender_x (&textmessage_sender_x_)

#endif
