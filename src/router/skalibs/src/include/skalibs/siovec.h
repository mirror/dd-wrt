/* ISC license. */

#ifndef SKALIBS_SIOVEC_H
#define SKALIBS_SIOVEC_H

#include <sys/uio.h>

#include <skalibs/gccattributes.h>

extern size_t siovec_len (struct iovec const *, unsigned int) gccattr_pure ;
extern size_t siovec_gather (struct iovec const *, unsigned int, char *, size_t) ;
extern size_t siovec_scatter (struct iovec const *, unsigned int, char const *, size_t) ;
extern size_t siovec_deal (struct iovec const *, unsigned int, struct iovec const *, unsigned int) ;
extern size_t siovec_seek (struct iovec *, unsigned int, size_t) ;
extern unsigned int siovec_trunc (struct iovec *, unsigned int, size_t) ;

extern size_t siovec_bytechr (struct iovec const *, unsigned int, char) ;
extern size_t siovec_bytein (struct iovec const *, unsigned int, char const *, size_t) ;
extern size_t siovec_search (struct iovec const *, unsigned int, char const *, size_t) ;

#endif
