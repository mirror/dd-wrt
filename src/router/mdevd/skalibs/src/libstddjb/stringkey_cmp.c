/* ISC license. */

#include <string.h>

#include <skalibs/bytestr.h>
#include "bytestr-internal.h"

int stringkey_cmp (void const *a, void const *b)
{
  struct stringkey_s const *aa = a ;
  struct stringkey_s const *bb = b ;
  return strcmp(aa->s, bb->s) ;
}
