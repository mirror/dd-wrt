/* ISC license. */

#ifndef SKALIBS_BUFFER_H
#define SKALIBS_BUFFER_H

#include <sys/uio.h>

#include <skalibs/gccattributes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/cbuffer.h>
#include <skalibs/functypes.h>

typedef struct buffer_s buffer, *buffer_ref ;
struct buffer_s
{
  iov_func_ref op ;
  int fd ;
  cbuffer c ;
} ;
#define BUFFER_ZERO { 0, -1, CBUFFER_ZERO }


 /*
    Circular buffers need to be 1 char bigger than the storage space,
    so that the head == tail case is nonambiguous (empty, not full).
 */

#define BUFFER_INSIZE 8192
#define BUFFER_OUTSIZE 8192
#define BUFFER_ERRSIZE 1024
#define BUFFER_INSIZE_SMALL 512
#define BUFFER_OUTSIZE_SMALL 512

#define BUFFER_INIT(f, d, buf, len) { (f), (d), CBUFFER_INIT(buf, len) }
extern int buffer_init (buffer *, iov_func_ref, int, char *, size_t) ;


 /* Writing */

extern int buffer_flush (buffer *) ;

#define buffer_putnoflush(b, s, len) cbuffer_put(&(b)->c, s, len)
#define buffer_putvnoflush(b, v, n) cbuffer_putv(&(b)->c, v, n)
extern size_t buffer_putsnoflush (buffer *, char const *) ;

extern int buffer_putallnoflush (buffer *, char const *, size_t) ;
extern int buffer_putvallnoflush (buffer *, struct iovec const *, unsigned int) ;
extern int buffer_putsallnoflush (buffer *, char const *) ;

extern int buffer_putall (buffer *, char const *, size_t, size_t *) ;
extern int buffer_putvall (buffer *, struct iovec const *, unsigned int, size_t *) ;
extern int buffer_putsall (buffer *, char const *, size_t *) ;

#define buffer_putallflush(b, s, len, w) (buffer_putall(b, s, len, w) && buffer_flush(b))
#define buffer_putvallflush(b, v, n, w) (buffer_putvall(b, v, n, w) && buffer_flush(b))
extern int buffer_putsallflush (buffer *, char const *, size_t *) ;

extern ssize_t buffer_put (buffer *, char const *, size_t) ;
extern ssize_t buffer_putv (buffer *, struct iovec const *, unsigned int) ;
extern ssize_t buffer_puts (buffer *, char const *) ;

extern ssize_t buffer_putflush (buffer *, char const *, size_t) ;
extern ssize_t buffer_putvflush (buffer *, struct iovec const *, unsigned int) ;
extern ssize_t buffer_putsflush (buffer *, char const *) ;

#define buffer_unput(b, n) cbuffer_unput(&(b)->c, n)
#define buffer_wpeek(b, v) cbuffer_wpeek(&(b)->c, v)
#define buffer_wseek(b, n) cbuffer_wseek(&(b)->c, n)


 /* Reading */

extern ssize_t buffer_fill (buffer *) ;

#define buffer_getnofill(b, s, len) cbuffer_get(&(b)->c, s, len)
#define buffer_getvnofill(b, v, n) cbuffer_getv(&(b)->c, v, n)

extern int buffer_getallnofill (buffer *, char *, size_t) ;
extern int buffer_getvallnofill (buffer *, struct iovec const *, unsigned int) ;

extern int buffer_getall (buffer *, char *, size_t, size_t *) ;
extern int buffer_getvall (buffer *, struct iovec const *, unsigned int, size_t *) ;

extern ssize_t buffer_get (buffer *, char *, size_t) ;
extern ssize_t buffer_getv (buffer *, struct iovec const *, unsigned int) ;

#define buffer_unget(b, n) cbuffer_unget(&(b)->c, n)
#define buffer_rpeek(b, v) cbuffer_rpeek(&(b)->c, v)
#define buffer_rseek(b, n) cbuffer_rseek(&(b)->c, n)


 /* Utility */

#define buffer_len(b) cbuffer_len(&(b)->c)
extern size_t buffer_getlen (buffer const *) gccattr_pure ;
#define buffer_available(b) cbuffer_available(&(b)->c)
#define buffer_isempty(b) cbuffer_isempty(&(b)->c)
#define buffer_isfull(b) cbuffer_isfull(&(b)->c)
#define buffer_fd(b) ((b)->fd)
extern int buffer_getfd (buffer const *) gccattr_pure ;
#define buffer_isreadable(b) (!buffer_isfull(b))
#define buffer_iswritable(b) (!buffer_isempty(b))


 /* Globals */

#define buffer_read fd_readv
#define buffer_write fd_writev
extern iov_func buffer_flush1read ;

extern buffer buffer_0_ ;
#define buffer_0 (&buffer_0_)

extern buffer buffer_0small_ ;
#define buffer_0small (&buffer_0small_)

extern buffer buffer_0f1_ ;
#define buffer_0f1 (&buffer_0f1_)

extern buffer buffer_1_ ;
#define buffer_1 (&buffer_1_)

extern buffer buffer_1small_ ;
#define buffer_1small (&buffer_1small_)

extern buffer buffer_2_ ;
#define buffer_2 (&buffer_2_)

#endif
