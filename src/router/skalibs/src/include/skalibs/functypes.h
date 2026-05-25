/* ISC license. */

#ifndef FUNCTYPES_H
#define FUNCTYPES_H

#include <stdint.h>
#include <sys/stat.h>
#include <sys/uio.h>

typedef int uintcmp_func (unsigned int, unsigned int, void *) ;
typedef uintcmp_func *uintcmp_func_ref ;

typedef int uint32cmp_func (uint32_t, uint32_t, void *) ;
typedef uint32cmp_func *uint32cmp_func_ref ;

typedef int cmp_func (void const *, void const *, void *) ;
typedef cmp_func *cmp_func_ref ;

typedef void *dtok_func (uint32_t, void *) ;
typedef dtok_func *dtok_func_ref ;

typedef int iter_func (void *, void *) ;
typedef iter_func *iter_func_ref ;

typedef void free_func (void *) ;
typedef free_func *free_func_ref ;

typedef int init_func (void *) ;
typedef init_func *init_func_ref ;

typedef void deinit_func (int, void *) ;
typedef deinit_func *deinit_func_ref ;

typedef ssize_t get_func (void *) ;
typedef get_func *get_func_ref ;

typedef size_t uget_func (void *) ;
typedef uget_func *uget_func_ref ;

typedef ssize_t io_func (int, char *, size_t) ;
typedef io_func *io_func_ref ;

typedef ssize_t iow_func (int, char const *, size_t) ;
typedef iow_func *iow_func_ref ;

typedef ssize_t iov_func (int, struct iovec const *, unsigned int) ;
typedef iov_func *iov_func_ref ;

typedef size_t allio_func (int, char *, size_t) ;
typedef allio_func *allio_func_ref ;

typedef size_t alliow_func (int, char const *, size_t) ;
typedef alliow_func *alliow_func_ref ;

typedef size_t alliov_func (int, struct iovec const *, unsigned int) ;
typedef alliov_func *alliov_func_ref ;

typedef int create_func (char const *, mode_t, void *) ;
typedef create_func *create_func_ref ;

typedef int link_func (char const *, char const *) ;
typedef link_func *link_func_ref ;

typedef void randomgen_func (char *, size_t) ;
typedef randomgen_func *randomgen_func_ref ;

typedef int main_func (char const *const *) ;
typedef main_func *main_func_ref ;

#endif
