/* ISC license. */

#ifndef SKALIBS_ERROR_H
#define SKALIBS_ERROR_H

#include <skalibs/gccattributes.h>

extern int error_temp (int) gccattr_const ;
extern int error_isalready (int) gccattr_const ;
#define error_isagain(e) (((e) == EAGAIN) || ((e) == EWOULDBLOCK))

#endif
