/* ISC license. */

#include <string.h>

#include <skalibs/bytestr.h>

int str_bcmp (void const *a, void const *b)
{
  return strcmp((char const *)a, *(char const *const *)b) ;
}
