/* ISC license. */

#include <strings.h>

#include <skalibs/bytestr.h>
#include "bytestr-internal.h"

int stringkey_casecmp (void const *a, void const *b)
{
  struct stringkey_s const *aa = a ;
  struct stringkey_s const *bb = b ;
  return strcasecmp(aa->s, bb->s) ;
}
