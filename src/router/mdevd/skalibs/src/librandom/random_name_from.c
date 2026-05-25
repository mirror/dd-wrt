/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/random.h>

void random_name_from (char *s, size_t n, randomgen_func_ref f)
{
  static char const random_oklist[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZghijklmnopqrstuvwxyz-_0123456789abcdef" ;
  (*f)(s, n) ;
  while (n--) s[n] = random_oklist[s[n] & 63] ;
}
