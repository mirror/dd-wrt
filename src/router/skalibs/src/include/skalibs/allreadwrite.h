/* ISC license. */

#ifndef SKALIBS_ALLREADWRITE_H
#define SKALIBS_ALLREADWRITE_H

#include <sys/uio.h>

#include <skalibs/functypes.h>

extern ssize_t sanitize_read (ssize_t) ;
extern ssize_t unsanitize_read (ssize_t) ;

extern size_t allreadwrite (io_func_ref, int, char *, size_t) ;
extern size_t allreadwritev (iov_func_ref, int, struct iovec const *, unsigned int) ;

extern ssize_t fd_read (int, char *, size_t) ;
extern ssize_t fd_write (int, char const *, size_t) ;
extern ssize_t fd_readv (int, struct iovec const *, unsigned int) ;
extern ssize_t fd_writev (int, struct iovec const *, unsigned int) ;

extern ssize_t fd_recv (int, char *, size_t, unsigned int) ;
extern ssize_t fd_send (int, char const *, size_t, unsigned int) ;

extern size_t allread (int, char *, size_t) ;
extern size_t allwrite (int, char const *, size_t) ;
extern size_t allreadv (int, struct iovec const *, unsigned int) ;
extern size_t allwritev (int, struct iovec const *, unsigned int) ;

#endif
