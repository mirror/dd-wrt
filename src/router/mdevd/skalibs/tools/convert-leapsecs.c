/* ISC license. */

#include <sys/types.h>
#include <time.h>

#include <skalibs/uint64.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr2.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/skamisc.h>

static genalloc table = GENALLOC_ZERO ; /* uint64_t */

static void add_leapsecs (uint64_t *t)
{
  uint64_t *tab = genalloc_s(uint64_t, &table) ;
  size_t n = genalloc_len(uint64_t, &table) ;
  size_t i = 0 ;
  for (; i < n ; i++) if (*t >= tab[i]) (*t)++ ;
}

int main (int argc, char const *const *argv)
{
  stralloc sa = STRALLOC_ZERO ;
  for (;;)
  {
    struct tm tm ;
    uint64_t tt ;
    time_t t ;
    int r ;
    char fmt[UINT64_FMT] ;
    sa.len = 0 ;
    r = skagetln(buffer_0, &sa, '\n') ;
    if (r < 0) strerr_diefu1sys(111, "read from stdin") ;
    if (!r) break ;
    sa.s[sa.len-1] = 0 ;
    if (!strptime(sa.s, "+%Y-%m-%d", &tm)) continue ;
    tm.tm_sec = 59 ;
    tm.tm_min = 59 ;
    tm.tm_hour = 23 ;
    t = mktime(&tm) ;
    if (t < 0) strerr_diefu1sys(111, "mktime") ;
    tt = t + 1 ;
    add_leapsecs(&tt) ;
    if (!genalloc_append(uint64_t, &table, &tt))
      strerr_diefu1sys(111, "genalloc_append") ;
    fmt[uint64_fmt(fmt, tt)] = 0 ;
    buffer_puts(buffer_1, "  TAI_MAGIC + ") ;
    buffer_puts(buffer_1, fmt) ;
    buffer_puts(buffer_1, ",\n") ;
  }
  buffer_unput(buffer_1, 2) ;
  buffer_putsflush(buffer_1, "\n") ;
  return 0 ;
}
