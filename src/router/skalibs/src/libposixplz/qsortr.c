/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>

#include <stdlib.h>

#include <skalibs/functypes.h>
#include <skalibs/posixplz.h>

#ifdef SKALIBS_HASQSORTR_POSIX

void qsortr (void *base, size_t n, size_t width, cmp_func_ref f, void *data)
{
  qsort_r(base, n, width, f, data) ;
}

#else
#ifdef SKALIBS_HASQSORTR_FREEBSD

void qsortr (void *base, size_t n, size_t width, cmp_func_ref f, void *data)
{
  qsort_r(base, n, width, data, f) ;
}

#else

struct aux_s
{
  cmp_func_ref f ;
  void *data ;
} ;

static struct aux_s *qsortr_aux_here ;

static int cmp (void const *a, void const *b)
{
  return (*qsortr_aux_here->f)(a, b, qsortr_aux_here->data) ;
}

void qsortr (void *base, size_t n, size_t width, cmp_func_ref f, void *data)
{
  struct aux_s aux = { .f = f, .data = data } ;
  qsortr_aux_here = &aux ;
  qsort(base, n, width, &cmp) ;
}

#endif
#endif
