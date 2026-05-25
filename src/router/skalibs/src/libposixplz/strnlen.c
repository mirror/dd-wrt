/* ISC license. */

#include <skalibs/sysdeps.h>

#ifndef SKALIBS_HASSTRNLEN

#include <string.h>
#include <skalibs/bytestr.h>
#include <skalibs/posixishard.h>

size_t strnlen (char const *s, size_t max)
{
  return byte_chr(s, max, 0) ;
}

#endif
