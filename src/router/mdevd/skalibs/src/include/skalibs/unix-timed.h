/* ISC license. */

#ifndef SKALIBS_UNIX_TIMED_H
#define SKALIBS_UNIX_TIMED_H

#include <sys/types.h>
#include <sys/uio.h>

#include <skalibs/bufalloc.h>
#include <skalibs/buffer.h>
#include <skalibs/functypes.h>
#include <skalibs/stralloc.h>
#include <skalibs/tai.h>


 /* Timed "blocking" operations (the fd must still be non-blocking) */

extern int timed_flush (void *, init_func_ref, init_func_ref, init_func_ref, tain const *, tain *) ;
#define timed_flush_g(b, getfd, isnonempty, flush, deadline) timed_flush(b, getfd, isnonempty, flush, (deadline), &STAMP)
extern ssize_t timed_get (void *, init_func_ref, get_func_ref, tain const *, tain *) ;
#define timed_get_g(b, getfd, get, deadline) timed_get(b, getfd, get, (deadline), &STAMP)

extern size_t timed_read (int, char *, size_t, tain const *, tain *) ;
#define timed_read_g(fd, s, len, deadline) timed_read(fd, s, len, (deadline), &STAMP)
extern size_t timed_readv (int, struct iovec *, unsigned int, tain const *, tain *) ;
#define timed_readv_g(fd, v, vlen, deadline) timed_readv(fd, v, vlen, (deadline), &STAMP)

extern size_t timed_write (int, char const *, size_t, tain const *, tain *) ;
#define timed_write_g(fd, s, len, deadline) timed_write(fd, s, len, (deadline), &STAMP)
extern size_t timed_writev (int, struct iovec const *, unsigned int, tain const *, tain *) ;
#define timed_writev_g(fd, v, vlen, deadline) timed_writev(fd, v, vlen, (deadline), &STAMP)

extern ssize_t buffer_timed_fill (buffer *, tain const *, tain *) ;
#define buffer_timed_fill_g(b, deadline) buffer_timed_fill(b, (deadline), &STAMP)
extern int bufalloc_timed_flush (bufalloc *, tain const *, tain *) ;
#define bufalloc_timed_flush_g(ba, deadline) bufalloc_timed_flush(ba, (deadline), &STAMP)
extern int buffer_timed_flush (buffer *, tain const *, tain *) ;
#define buffer_timed_flush_g(b, deadline) buffer_timed_flush(b, (deadline), &STAMP)

extern size_t buffer_timed_get (buffer *, char *, size_t, tain const *, tain *) ;
#define buffer_timed_get_g(b, s, len, deadline) buffer_timed_get(b, s, len, (deadline), &STAMP)
extern size_t buffer_timed_getv (buffer *, struct iovec *, unsigned int, tain const *, tain *) ;
#define buffer_timed_getv_g(b, v, vlen, deadline) buffer_timed_getv(b, v, vlen, (deadline), &STAMP)
extern size_t buffer_timed_put (buffer *, char const *, size_t, tain const *, tain *) ;
#define buffer_timed_put_g(b, s, len, deadline) buffer_timed_put(b, s, len, (deadline), &STAMP)
extern size_t buffer_timed_putv (buffer *, struct iovec const *, unsigned int, tain const *, tain *) ;
#define buffer_timed_putv_g(b, v, vlen, deadline) buffer_timed_putv(b, v, vlen, (deadline), &STAMP)
extern size_t buffer_timed_puts (buffer *, char const *, tain const *, tain *) ;
#define buffer_timed_puts_g(b, s, deadline) buffer_timed_puts(b, s, (deadline), &STAMP)

extern int timed_getln (buffer *, stralloc *, char, tain const *, tain *) ;
#define timed_getln_g(b, sa, sep, deadline) timed_getln(b, sa, sep, (deadline), &STAMP)
extern ssize_t timed_getlnmax (buffer *, char *, size_t, size_t *, char, tain const *, tain *) ;
#define timed_getlnmax_g(b, max, maxlen, len, sep, deadline) timed_getlnmax(b, max, maxlen, len, sep, (deadline), &STAMP)
extern int netstring_timed_get (buffer *, stralloc *, tain const *, tain *) ;
#define netstring_timed_get_g(b, sa, deadline) netstring_timed_get(b, sa, (deadline), &STAMP)

extern int ipc_timed_send (int, char const *, size_t, tain const *, tain *) ;
#define ipc_timed_send_g(fd, s, len, deadline) ipc_timed_send(fd, s, len, (deadline), &STAMP)
extern int ipc_timed_sendv (int, struct iovec const *, unsigned int, tain const *, tain *) ;
#define ipc_timed_sendv_g(fd, v, n, deadline) ipc_timed_sendv(fd, v, n, (deadline), &STAMP)
extern ssize_t ipc_timed_recv (int, char *, size_t, char *, tain const *, tain *) ;
#define ipc_timed_recv_g(fd, s, len, path, deadline) ipc_timed_recv(fd, s, len, path, (deadline), &STAMP)

#endif
