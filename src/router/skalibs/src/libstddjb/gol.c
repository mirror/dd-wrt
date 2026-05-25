/* ISC license. */

#include <string.h>

#include <skalibs/gol.h>

int gol (char const *const *argv, gol_bool const *b, unsigned int bn, gol_arg const *a, unsigned int an, uint64_t *br, char const **ar, int *problem)
{
  unsigned int i = 0 ;
  for (; argv[i] ; i++)
  {
    if (argv[i][0] != '-' || !argv[i][1]) return i ;
    if (argv[i][1] == '-')
    {
      unsigned int j = 0 ;
      char const *x ;
      if (!argv[i][2]) return i+1 ;
      x = strchr(argv[i]+2, '=') ;
      if (x)
      {
        size_t len = x - argv[i] - 2 ;
        if (!len) return (*problem = 0, -1-i) ;
        for (; j < an ; j++) if (a[j].lo && !strncmp(argv[i] + 2, a[j].lo, len) && !a[j].lo[len]) break ;
        if (j >= an) return (*problem = -len-2, -1-i) ;
        ar[a[j].i] = x + 1 ;
      }
      else
      {
        for (; j < bn ; j++) if (b[j].lo && !strcmp(argv[i] + 2, b[j].lo)) break ;
        if (j >= bn) return (*problem = -1, -1-i) ;
        *br &= ~b[j].clear ;
        *br |= b[j].set ;
      }
    }
    else
    {
      char const *p = argv[i] + 1 ;
      for (; *p ; p++)
      {
        unsigned int j = 0 ;
        for (; j < an ; j++) if (*p == a[j].so) break ;
        if (j < an)
        {
          if (p[1]) ar[a[j].i] = p + 1 ;
          else if (argv[i+1] && strcmp(argv[i+1], "--")) ar[a[j].i] = argv[++i] ;
          break ;
        }
        for (j = 0 ; j < bn ; j++) if (*p == b[j].so) break ;
        if (j >= bn) return (*problem = p - argv[i], -1-i) ;
        *br &= ~b[j].clear ;
        *br |= b[j].set ;
      }
    }
  }
  return i ;
}
