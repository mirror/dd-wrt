/* ISC license. */

#ifndef SKALIBS_GENWRITE_H
#define SKALIBS_GENWRITE_H

#include <sys/types.h>

typedef ssize_t genwrite_put_func (void *, char const *, size_t) ;
typedef genwrite_put_func *genwrite_put_func_ref ;

typedef int genwrite_flush_func (void *) ;
typedef genwrite_flush_func *genwrite_flush_func_ref ;

typedef struct genwrite_s genwrite, *genwrite_ref ;
struct genwrite_s
{
  genwrite_put_func_ref put ;
  genwrite_flush_func_ref flush ;
  void *target ;
} ;
#define GENWRITE_ZERO { .put = 0, .flush = 0, .target = 0 }

extern genwrite_put_func genwrite_put_stralloc ;
extern genwrite_flush_func genwrite_flush_stralloc ;
extern genwrite_put_func genwrite_put_buffer ;
extern genwrite_flush_func genwrite_flush_buffer ;
extern genwrite_put_func genwrite_put_bufalloc ;
extern genwrite_flush_func genwrite_flush_bufalloc ;

#define GENWRITE_STRALLOC_INIT(sa) { .put = &genwrite_put_stralloc, .flush = &genwrite_flush_stralloc, .target = (sa) }
#define GENWRITE_BUFFER_INIT(b) { .put = &genwrite_put_buffer, .flush = &genwrite_flush_buffer, .target = (b) }
#define GENWRITE_BUFALLOC_INIT(ba) { .put = &genwrite_put_bufalloc, .flush = &genwrite_flush_bufalloc, .target = (ba) }

extern genwrite genwrite_stdout ;
extern genwrite genwrite_stderr ;

#endif
