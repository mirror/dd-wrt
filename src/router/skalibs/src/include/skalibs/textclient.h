/* ISC license. */

#ifndef SKALIBS_TEXTCLIENT_H
#define SKALIBS_TEXTCLIENT_H

#include <sys/types.h>
#include <sys/uio.h>
#include <stdint.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/tai.h>
#include <skalibs/textmessage.h>


 /*
   This is a simpler, smaller version of skaclient for basic cases:
   - no fd-passing
   - no asyncout
   - no kolbak: client calls are always synchronous
   - fixed-size buffers included in the client structure
 */

#define TEXTCLIENT_BUFSIZE 4096
#define TEXTCLIENT_OPTION_WAITPID 0x00000001U


 /* Server-side functions */

extern int textclient_server_init (textmessage_receiver *, textmessage_sender *, textmessage_sender *, char const *, size_t, char const *, size_t, tain const *, tain *) ;
extern int textclient_server_init_frompipe (textmessage_receiver *, textmessage_sender *, textmessage_sender *, char const *, size_t, char const *, size_t, tain const *, tain *) ;
extern int textclient_server_init_fromsocket (textmessage_receiver *, textmessage_sender *, textmessage_sender *, char const *, size_t, char const *, size_t, tain const *, tain *) ;
#define textclient_server_init_g(in, syncout, asyncout, before, beforelen, after, afterlen, deadline) textclient_server_init(in, syncout, asyncout, before, beforelen, after, afterlen, (deadline), &STAMP)
#define textclient_server_init_frompipe_g(in, syncout, asyncout, before, beforelen, after, afterlen, deadline) textclient_server_init_frompipe(in, syncout, asyncout, before, beforelen, after, afterlen, (deadline), &STAMP)
#define textclient_server_init_fromsocket_g(in, syncout, asyncout, before, beforelen, after, afterlen, deadline) textclient_server_init_fromsocket(in, syncout, asyncout, before, beforelen, after, afterlen, (deadline), &STAMP)
#define textclient_server_01x_init(before, beforelen, after, afterlen, deadline, stamp) textclient_server_init(textmessage_receiver_0, textmessage_sender_1, textmessage_sender_x, before, beforelen, after, afterlen, deadline, stamp)
#define textclient_server_01x_init_frompipe(before, beforelen, after, afterlen, deadline, stamp) textclient_server_init_frompipe(textmessage_receiver_0, textmessage_sender_1, textmessage_sender_x, before, beforelen, after, afterlen, deadline, stamp)
#define textclient_server_01x_init_fromsocket(before, beforelen, after, afterlen, deadline, stamp) textclient_server_init_fromsocket(textmessage_receiver_0, textmessage_sender_1, textmessage_sender_x, before, beforelen, after, afterlen, deadline, stamp)
#define textclient_server_01x_init_g(before, beforelen, after, afterlen, deadline) textclient_server_01x_init(before, beforelen, after, afterlen, (deadline), &STAMP)
#define textclient_server_01x_init_frompipe_g(before, beforelen, after, afterlen, deadline) textclient_server_01x_init_frompipe(before, beforelen, after, afterlen, (deadline), &STAMP)
#define textclient_server_01x_init_fromsocket_g(before, beforelen, after, afterlen, deadline) textclient_server_01x_init_fromsocket(before, beforelen, after, afterlen, (deadline), &STAMP)


 /* User structure */

typedef struct textclient_s textclient, *textclient_ref ;
struct textclient_s
{
  textmessage_sender syncout ;
  textmessage_receiver syncin ;
  textmessage_receiver asyncin ;
  pid_t pid ;
  uint32_t options ;
  char syncbuf[TEXTCLIENT_BUFSIZE] ;
  char asyncbuf[TEXTCLIENT_BUFSIZE] ;
} ;
#define TEXTCLIENT_ZERO { .syncout = TEXTMESSAGE_SENDER_ZERO, .syncin = TEXTMESSAGE_RECEIVER_ZERO, .asyncin = TEXTMESSAGE_RECEIVER_ZERO, .pid = 0, .options = 0 }
extern textclient const textclient_zero ;


 /* Starting and ending */

extern void textclient_end (textclient *) ;

extern int textclient_start (textclient *, char const *, uint32_t, char const *, size_t, char const *, size_t, tain const *, tain *) ;
extern int textclient_startf (textclient *, char const *const *, char const *const *, uint32_t, char const *, size_t, char const *, size_t, tain const *, tain *) ;

#define textclient_start_g(a, path, options, before, beforelen, after, afterlen, deadline) textclient_start(a, path, options, before, beforelen, after, afterlen, (deadline), &STAMP)
#define textclient_startf_g(a, argv, envp, options, before, beforelen, after, afterlen, deadline) textclient_startf_b(a, argv, envp, options, before, beforelen, after, afterlen, (deadline), &STAMP)


 /* Writing */

#define textclient_put(a, s, len) textmessage_sender_put(&(a)->syncout, s, len)
#define textclient_putv(a, v, n) textmessage_sender_putv(&(a)->syncout, v, n)
#define textclient_flush(a) textmessage_sender_flush(&(a)->syncout)
#define textclient_timed_flush(a, deadline, stamp) textmessage_sender_timed_flush(&(a)->syncout, deadline, stamp)
#define textclient_timed_flush_g(a, deadline) textclient_timed_flush(a, (deadline), &STAMP)
#define textclient_send(a, s, len) textmessage_send(&(a)->syncout, s, len)
#define textclient_sendv(a, v, n) textmessage_sendv(&(a)->syncout, v, n)
#define textclient_timed_send(a, s, len, deadline, stamp) textmessage_timed_send(&(a)->syncout, s, len, deadline, stamp)
#define textclient_timed_sendv(a, v, n, deadline, stamp) textmessage_timed_sendv(&(a)->syncout, v, n, deadline, stamp)
#define textclient_timed_send_g(a, s, len, deadline) textclient_timed_send(a, s, len, (deadline), &STAMP)
#define textclient_timed_sendv_g(a, v, n, deadline) textclient_timed_sendv(a, v, n, (deadline), &STAMP)


 /* Sync reading */

#define textclient_get(a, v) textmessage_receive(&(a)->syncin, v)
#define textclient_timed_get(a, v, deadline, stamp) (sanitize_read(textmessage_timed_receive(&(a)->syncin, v, deadline, stamp)) > 0)
#define textclient_timed_get_g(a, v, deadline) textclient_timed_get(a, v, (deadline), &STAMP)


 /* Sync writing+reading */

#define textclient_exchange(a, s, len, ans, deadline, stamp) (textclient_timed_send(a, s, len, deadline, stamp) && textclient_timed_get(a, ans, deadline, stamp))
#define textclient_exchangev(a, v, n, ans, deadline, stamp) (textclient_timed_sendv(a, v, n, deadline, stamp) && textclient_timed_get(a, ans, deadline, stamp))
#define textclient_exchange_g(a, s, len, ans, deadline) textclient_exchange(a, s, len, ans, (deadline), &STAMP)
#define textclient_exchangev_g(a, v, n, ans, deadline) textclient_exchangev(a, v, n, ans, (deadline), &STAMP)

extern int textclient_command (textclient *, char const *, size_t, tain const *, tain *) ;
extern int textclient_commandv (textclient *, struct iovec const *, unsigned int, tain const *, tain *) ;
#define textclient_command_g(a, s, len, deadline) textclient_command(a, s, len, (deadline), &STAMP)
#define textclient_commandv_g(a, v, n, deadline) textclient_commandv(a, v, n, (deadline), &STAMP)


 /* Async reading */

#define textclient_fd(a) textmessage_receiver_fd(&(a)->asyncin)
#define textclient_update(a, f, p) textmessage_handle(&(a)->asyncin, f, p)
#define textclient_timed_update(a, f, p, deadline, stamp) textmessage_timed_handle(&(a)->asyncin, f, p, deadline, stamp)
#define textclient_timed_update_g(a, f, p, deadline) textclient_timed_update(a, f, p, (deadline), &STAMP)

#endif
