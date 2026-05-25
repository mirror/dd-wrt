/* ISC license. */

#include <string.h>

#include <skalibs/gol.h>
#include <skalibs/strerr.h>

unsigned int gol_main (int argc, char const *const *argv, gol_bool const *b, unsigned int bn, gol_arg const *a, unsigned int an, uint64_t *br, char const **ar)
{
  if (argc < 1 || argv[argc]) strerr_diefu1x(103, "gol: invalid argc/argv") ;
  if (argc == 1) return 1 ;
  return 1 + gol_argv(argv + 1, b, bn, a, an, br, ar) ;
}
