/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/tai.h>

#if defined(SKALIBS_HASCLOCKRT) && (defined(SKALIBS_HASCLOCKMON) || defined(SKALIBS_HASCLOCKBOOT))

#include <time.h>

int tain_stopwatch_init (tain *now, clock_t cl, tain *offset)
{
  tain a, b ;
  struct timespec ts ;
  if (clock_gettime(cl, &ts) < 0) return 0 ;
  if (!tain_from_timespec(&b, &ts)) return 0 ;
  if (!tain_wallclock_read(&a)) return 0 ;
  tain_sub(offset, &a, &b) ;
  *now = a ;
  return 1 ;
}

int tain_stopwatch_read (tain *now, clock_t cl, tain const *offset)
{
  struct timespec ts ;
  if (clock_gettime(cl, &ts) < 0) return 0 ;
  if (!tain_from_timespec(now, &ts)) return 0 ;
  tain_add(now, now, offset) ;
  return 1 ;
}

#else

#include <errno.h>

int tain_stopwatch_init (tain *now, clock_t cl, tain *offset)
{
  (void)now ;
  (void)cl ;
  (void)offset ;
  return (errno = ENOSYS, 0) ;
}

int tain_stopwatch_read (tain *now, clock_t cl, tain const *offset)
{
  (void)now ;
  (void)cl ;
  (void)offset ;
  return (errno = ENOSYS, 0) ;
}

#endif
