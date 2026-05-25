/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASSETGROUPS

#include <skalibs/nonposix.h>

#include <string.h>
#include <unistd.h>
#include <grp.h>

#include <skalibs/setgroups.h>
#include <skalibs/posixishard.h>

int setgroups_and_gid (gid_t g, size_t n, gid_t const *tab)
{
  size_t i = 1 ;
  if (!n) return setgroups(1, &g) ;
  if (tab[0] == g) return setgroups(n, tab) ;
  for (; i < n ; i++) if (tab[i] == g) break ;
  if (i < n)
  {
    gid_t newtab[n] ;
    newtab[0] = g ;
    memcpy(newtab + 1, tab, i * sizeof(gid_t)) ;
    memcpy(newtab + i + 1, tab + i + 1, (n - i - 1) * sizeof(gid_t)) ;
    return setgroups(n, newtab) ;
  }
  else
  {
    gid_t newtab[n+1] ;
    newtab[0] = g ;
    memcpy(newtab + 1, tab, n * sizeof(gid_t)) ;
    return setgroups(n+1, newtab) ;
  }
}

int setgroups_with_egid (size_t n, gid_t const *tab)
{
  return setgroups_and_gid(getegid(), n, tab) ;
}

int skalibs_setgroups (size_t n, gid_t const *tab)
{
#ifdef SKALIBS_BSD_SUCKS
  return setgroups_with_egid(n, tab) ;
#else
  return setgroups(n, tab) ;
#endif
}

#endif
