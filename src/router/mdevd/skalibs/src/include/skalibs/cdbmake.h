/* ISC license. */

#ifndef SKALIBS_CDBMAKE_H
#define SKALIBS_CDBMAKE_H

#include <stdint.h>
#include <sys/uio.h>

#include <skalibs/genalloc.h>
#include <skalibs/buffer.h>

typedef struct cdbmaker_s cdbmaker, *cdbmaker_ref ;
struct cdbmaker_s
{
  genalloc hplist ; /* array of diuint32 */
  uint32_t pos ;
  buffer b ;
  char buf[BUFFER_OUTSIZE] ;
} ;
#define CDBMAKER_ZERO { .hplist = GENALLOC_ZERO, .pos = 2048, .b = BUFFER_ZERO, .buf = { 0 } }

extern int cdbmake_start (cdbmaker *, int) ;
extern int cdbmake_add (cdbmaker *, char const *, uint32_t, char const *, uint32_t) ;
extern int cdbmake_addv (cdbmaker *, struct iovec const *, unsigned int, struct iovec const *, unsigned int) ;
extern int cdbmake_finish (cdbmaker *) ;

#endif
