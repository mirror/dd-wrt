/* ISC license. */

#undef SUBGETOPT_SHORT
#include <skalibs/sgetopt.h>

int subgetopt_r (int argc, char const *const *argv, char const *opts, subgetopt *o)
{
  o->arg = 0 ;
  if ((o->ind >= argc) || !argv[o->ind]) return -1 ;
  if (o->pos && !argv[o->ind][o->pos])
  {
    o->ind++ ;
    o->pos = 0 ;
    if ((o->ind >= argc) || !argv[o->ind]) return -1 ;
  }

  if (!o->pos)
  {
    char c ;
    if (argv[o->ind][0] != '-') return -1 ;
    o->pos++ ;
    c = argv[o->ind][1] ;
    if (c == '-')
    {
      if (argv[o->ind][2]) return o->problem = '-', '?' ;
      o->ind++ ;
      o->pos = 0 ;
      return -1 ;
    }
    if (!c || c == '-') return o->pos = 0, -1 ;
  }
  {
    char c = argv[o->ind][o->pos++] ;
    char const *s = opts ;
    char retnoarg = (*s == ':') ? (s++, ':') : '?' ;
    while (*s)
    {
      if (c == *s)
      {
        if (s[1] == ':')
        {
          o->arg = argv[o->ind++] + o->pos ;
          o->pos = 0 ;
          if (!*o->arg)
          {
            o->arg = argv[o->ind] ;
            if ((o->ind >= argc) || !o->arg)
            {
              o->problem = c ;
              return retnoarg ;
            }
	    o->ind++ ;
          }
        }
        return c ;
      }
      if (*++s == ':') s++ ;
    }
    o->problem = c ;
  }
  return '?' ;
}
