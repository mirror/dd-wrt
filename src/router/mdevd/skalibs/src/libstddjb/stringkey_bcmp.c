/* ISC license. */

#include <string.h>

#include <skalibs/bytestr.h>
#include "bytestr-internal.h"

int stringkey_bcmp (void const *a, void const *b)
{
  struct stringkey_s const *bb = b ;
  return strcmp((char const *)a, bb->s) ;
}
