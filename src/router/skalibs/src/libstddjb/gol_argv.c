/* ISC license. */

#include <skalibs/gol.h>
#include <skalibs/strerr.h>

unsigned int gol_argv (char const *const *argv, gol_bool const *b, unsigned int bn, gol_arg const *a, unsigned int an, uint64_t *br, char const **ar)
{
  int problem = 0 ;
  int r = gol(argv, b, bn, a, an, br, ar, &problem) ;

  if (r < 0)
  {
    if (problem > 0)
    {
      char s[2] = { argv[-r-1][problem], 0 } ;
      strerr_dief4x(100, "unrecognized ", "short ", "option: ", s) ;
    }
    else if (!problem)
      strerr_dief3x(100, "invalid ", "option: ", argv[-r-1]) ;
    else if (problem == -1)
      strerr_dief4x(100, "unrecognized ", "boolean ", "option: ", argv[-r-1]) ;
    else
      strerr_dief3x(100, "unrecognized ", "option with argument: ", argv[-r-1]) ;
  }
  else return r ;
}
