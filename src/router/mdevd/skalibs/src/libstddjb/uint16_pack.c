/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/uint16.h>

void uint16_pack (char *s, uint16_t u)
{
  uint16_littlep(&u) ;
  memcpy(s, &u, sizeof(uint16_t)) ;
}
