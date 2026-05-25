/* ISC license. */

#ifndef SKALIBS_CBUFFER_H
#define SKALIBS_CBUFFER_H

#include <sys/uio.h>
#include <string.h>

#include <skalibs/gccattributes.h>

typedef struct cbuffer_s cbuffer, *cbuffer_ref ;
struct cbuffer_s
{
  char *x ;
  size_t a ; /* total length */
  size_t p ; /* head */
  size_t n ; /* tail */
} ;
#define CBUFFER_ZERO { 0, 0, 0, 0 }

 /*
    Circular buffers need to be 1 char bigger than the storage space,
    so that the head == tail case is nonambiguous (empty).
 */

#define CBUFFER_INIT(buf, len) { (buf), (len), 0, 0 }
extern int cbuffer_init (cbuffer *, char *, size_t) ;


 /* Writing */

extern size_t cbuffer_put (cbuffer *, char const *, size_t) ;
extern size_t cbuffer_putv (cbuffer *, struct iovec const *, unsigned int) ;
#define cbuffer_puts(b, s) cbuffer_put(b, (s), strlen(s))

#define cbuffer_UNPUT(b, w) ((b)->n = ((b)->a + (b)->n - w) % (b)->a, w) ;
extern size_t cbuffer_unput (cbuffer *, size_t) ;
extern void cbuffer_wpeek (cbuffer const *, struct iovec *) ;
#define cbuffer_WSEEK(b, w) ((b)->n = ((b)->n + (w)) % (b)->a, w)
extern size_t cbuffer_wseek (cbuffer *, size_t) ;


 /* Reading */

extern size_t cbuffer_get (cbuffer *, char *, size_t) ;
extern size_t cbuffer_getv (cbuffer *, struct iovec const *, unsigned int) ;

#define cbuffer_UNGET(b, n) ((b)->p = ((b)->a + (b)->p - n) % (b)->a, n) ;
extern size_t cbuffer_unget (cbuffer *, size_t) ;
extern void cbuffer_rpeek (cbuffer const *, struct iovec *) ;
#define cbuffer_RSEEK(b, n) ((b)->p = ((b)->p + (n)) % (b)->a, n)
extern size_t cbuffer_rseek (cbuffer *, size_t) ;


 /* Utility */

#define cbuffer_len(b) ((size_t)(((b)->a - (b)->p + (b)->n) % (b)->a))
#define cbuffer_available(b) ((size_t)(((b)->a - (b)->n + (b)->p - 1) % (b)->a))
#define cbuffer_isempty(b) (!cbuffer_len(b))
#define cbuffer_isfull(b) (!cbuffer_available(b))

#endif
