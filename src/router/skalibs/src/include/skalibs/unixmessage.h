/* ISC license. */

#ifndef SKALIBS_UNIXMESSAGE_H
#define SKALIBS_UNIXMESSAGE_H

#include <sys/uio.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/buffer.h>
#include <skalibs/cbuffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/tai.h>


 /* Message */

typedef struct unixmessage_s unixmessage, *unixmessage_ref ;
struct unixmessage_s
{
  char *s ;
  size_t len ;
  int *fds ;
  unsigned int nfds ;
} ;
#define UNIXMESSAGE_ZERO { .s = 0, .len = 0, .fds = 0, .nfds = 0 }
extern unixmessage const unixmessage_zero ;

extern void unixmessage_drop (unixmessage const *) ;

typedef struct unixmessagev_s unixmessagev, *unixmessagev_ref ;
struct unixmessagev_s
{
  struct iovec *v ;
  unsigned int vlen ;
  int *fds ;
  unsigned int nfds ;
} ;
#define UNIXMESSAGEV_ZERO { .v = 0, .vlen = 0, .fds = 0, .nfds = 0 }
extern unixmessagev const unixmessagev_zero ;

#define UNIXMESSAGE_MAXSIZE (1U << 28)
#define UNIXMESSAGE_MAXFDS 253
#define UNIXMESSAGE_BUFSIZE 2048
#define UNIXMESSAGE_AUXBUFSIZE (sizeof(int) * UNIXMESSAGE_MAXFDS + 1)
#define UNIXMESSAGE_MAXREADS 128


 /* Sender */

typedef void unixmessage_sender_closecb_func (int, void *) ;
typedef unixmessage_sender_closecb_func *unixmessage_sender_closecb_func_ref ;

typedef struct unixmessage_sender_s unixmessage_sender, *unixmessage_sender_ref ;
struct unixmessage_sender_s
{
  int fd ;
  stralloc data ;
  genalloc fds ; /* int */
  genalloc offsets ; /* disize */
  size_t head ;
  size_t shorty ;
  unixmessage_sender_closecb_func_ref closecb ;
  void *closecbdata ;
} ;
#define UNIXMESSAGE_SENDER_ZERO UNIXMESSAGE_SENDER_INIT(-1)
#define UNIXMESSAGE_SENDER_INIT(s) UNIXMESSAGE_SENDER_INIT_WITHCLOSECB((s), &unixmessage_sender_closecb, 0)
#define UNIXMESSAGE_SENDER_INIT_WITHCLOSECB(s, f, p) { .fd = (s), .data = STRALLOC_ZERO, .fds = GENALLOC_ZERO, .offsets = GENALLOC_ZERO, .head = 0, .shorty = 0, .closecb = (f), .closecbdata = (p) }

extern unixmessage_sender const unixmessage_sender_zero ;
extern unixmessage_sender_closecb_func unixmessage_sender_closecb ;
extern void unixmessage_sender_init (unixmessage_sender *, int) ;
extern void unixmessage_sender_init_withclosecb (unixmessage_sender *, int, unixmessage_sender_closecb_func_ref, void *) ;
extern void unixmessage_sender_free (unixmessage_sender *) ;
#define unixmessage_sender_fd(b) ((b)->fd)
extern int unixmessage_sender_getfd (unixmessage_sender const *) gccattr_pure ;
#define unixmessage_sender_isempty(b) (!genalloc_len(unsigned int, &(b)->offsets))

extern int unixmessage_put_and_close (unixmessage_sender *, unixmessage const *, unsigned char const *) ;
#define unixmessage_put(b, m) unixmessage_put_and_close(b, m, unixmessage_bits_closenone)
extern int unixmessage_putv_and_close (unixmessage_sender *, unixmessagev const *, unsigned char const *) ;
#define unixmessage_putv(b, m) unixmessage_putv_and_close(b, m, unixmessage_bits_closenone)

extern int unixmessage_unput_and_maybe_drop (unixmessage_sender *, int) ;
#define unixmessage_unput(b) unixmessage_unput_and_maybe_drop((b), 0)
#define unixmessage_unput_and_drop(b) unixmessage_unput_and_maybe_drop((b), 1)

extern unsigned char const *const unixmessage_bits_closenone ;
extern unsigned char const *const unixmessage_bits_closeall ;

extern int unixmessage_sender_flush (unixmessage_sender *) ;
extern int unixmessage_sender_timed_flush (unixmessage_sender *, tain const *, tain *) ;
#define unixmessage_sender_timed_flush_g(sender, deadline) unixmessage_sender_timed_flush(sender, (deadline), &STAMP)


 /* Receiver */

typedef struct unixmessage_receiver_s unixmessage_receiver, *unixmessage_receiver_ref ;
struct unixmessage_receiver_s
{
  int fd ;
  cbuffer mainb ;
  cbuffer auxb ;
  stralloc maindata ;
  stralloc auxdata ;
  uint32_t mainlen ;
  uint16_t auxlen ;
  unsigned int fds_ok : 2 ;
} ;
#define UNIXMESSAGE_RECEIVER_ZERO { .fd = -1, .mainb = CBUFFER_ZERO, .auxb = CBUFFER_ZERO, .maindata = STRALLOC_ZERO, .auxdata = STRALLOC_ZERO, .mainlen = 0, .auxlen = 0, .fds_ok = 3 }
#define UNIXMESSAGE_RECEIVER_INIT(d, mains, mainn, auxs, auxn) \
{ \
  .fd = d, \
  .mainb = CBUFFER_INIT(mains, mainn), \
  .auxb = CBUFFER_INIT(auxs, auxn), \
  .maindata = STRALLOC_ZERO, \
  .auxdata = STRALLOC_ZERO, \
  .mainlen = 0, \
  .auxlen = 0, \
  .fds_ok = 3 \
}
extern int unixmessage_receiver_init (unixmessage_receiver *, int, char *, size_t, char *, size_t) ;
extern void unixmessage_receiver_free (unixmessage_receiver *) ;
#define unixmessage_receiver_fd(b) ((b)->fd)
#define unixmessage_receiver_isempty(b) (cbuffer_isempty(&(b)->mainb) && cbuffer_isempty(&(b)->auxb))
#define unixmessage_receiver_isfull(b) (cbuffer_isfull(&(b)->mainb) || cbuffer_isfull(&(b)->auxb))

extern int unixmessage_receiver_hasmsginbuf (unixmessage_receiver const *) ;

extern int unixmessage_receive (unixmessage_receiver *, unixmessage *) ;
extern int unixmessage_timed_receive (unixmessage_receiver *, unixmessage *, tain const *, tain *) ;
#define unixmessage_timed_receive_g(receiver, msg, deadline) unixmessage_timed_receive(receiver, msg, (deadline), &STAMP)

#define unixmessage_receiver_accept_fds(b) ((b)->fds_ok = 3)
#define unixmessage_receiver_refuse_fds(b) ((b)->fds_ok = 1)
#define unixmessage_receiver_ignore_fds(b) ((b)->fds_ok = 0)

typedef int unixmessage_handler_func (unixmessage const *, void *) ;
typedef unixmessage_handler_func *unixmessage_handler_func_ref ;

extern int unixmessage_handle (unixmessage_receiver *, unixmessage_handler_func_ref, void *) ;
extern int unixmessage_timed_handle (unixmessage_receiver *, unixmessage_handler_func_ref, void *, tain const *, tain *) ;
#define unixmessage_timed_handle_g(b, f, p, deadline) unixmessage_timed_handle(b, f, p, (deadline), &STAMP)


 /* Globals */

extern unixmessage_receiver unixmessage_receiver_0_ ;
#define unixmessage_receiver_0 (&unixmessage_receiver_0_)

extern unixmessage_sender unixmessage_sender_1_ ;
#define unixmessage_sender_1 (&unixmessage_sender_1_)

extern unixmessage_sender unixmessage_sender_x_ ;
#define unixmessage_sender_x (&unixmessage_sender_x_)

#endif
