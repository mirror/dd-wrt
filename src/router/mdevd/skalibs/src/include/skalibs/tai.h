/* ISC license. */

#ifndef SKALIBS_TAI_H
#define SKALIBS_TAI_H

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <skalibs/gccattributes.h>
#include <skalibs/uint64.h>

typedef struct tai_s tai, *tai_ref ;
struct tai_s
{
  uint64_t x ;
} ;

#define TAI_ZERO { 0 }
#define TAI_MAGIC ((uint64_t)4611686018427387904ULL)
#define TAI_EPOCH { TAI_MAGIC + 10UL }
#define TAI_INFINITE_RELATIVE { (uint64_t)1 << 61 }
#define TAI_INFINITE { TAI_MAGIC + ((uint64_t)1 << 61) }

#define tai_sec(t) ((t)->x)
extern int tai_u64 (tai *, uint64_t) ;
#define tai_unix(t, u) tai_u64(t, (TAI_MAGIC + (u)))
#define tai_uint(t, u) tai_u64(t, (uint64_t)(u))

extern int tai_now (tai *) ;

#define tai_relative_from_time(t, u) ((t)->x = (uint64_t)(u), 1)
#define tai_from_time(t, u) tai_unix(t, u)
#define tai_from_time_sysclock(t, u) tai_from_sysclock(t, (uint64_t)(u) + TAI_MAGIC)
extern int time_from_tai_relative (time_t *, tai const *) ;
extern int time_from_tai (time_t *, tai const *) ;
extern int time_sysclock_from_tai(time_t *, tai const *) ;

extern int tai_from_sysclock (tai *, uint64_t) ;
extern int sysclock_from_tai (uint64_t *, tai const *) ;

#define tai_approx(t) ((double)(tai_sec(t)))

extern int tai_add (tai *, tai const *, tai const *) ;
extern int tai_sub (tai *, tai const *, tai const *) ;
#define tai_less(t,u) (tai_sec(t) < tai_sec(u))

#define TAI_PACK 8
extern void tai_pack (char *, tai const *) ;
extern void tai_unpack (char const *, tai *) ;
extern void tai_pack_little (char *, tai const *) ;
extern void tai_unpack_little (char const *, tai *) ;

typedef struct tain_s tain, *tain_ref ;
struct tain_s
{
  tai sec ;
  uint32_t nano ; /* 0..999999999U */
} ;

#define TAIN_ZERO { TAI_ZERO, 0 }
#define TAIN_EPOCH { TAI_EPOCH, 0 }
#define TAIN_INFINITE { TAI_INFINITE, 0 }
#define TAIN_INFINITE_RELATIVE { TAI_INFINITE_RELATIVE, 0 }
#define TAIN_NANO500 { TAI_ZERO, 500 }

extern tain STAMP ; /* the global process wallclock */
extern tain const tain_zero ;
extern tain const tain_infinite_relative ;
extern tain const tain_infinite ;
extern tain const tain_nano500 ;

#define tain_sec(a) ((a)->sec)
#define tain_secp(a) (&(a)->sec)
#define tain_nano(a) ((a)->nano)

extern int tain_relative_from_timeval (tain *, struct timeval const *) ;
extern int tain_from_timeval (tain *, struct timeval const *) ;
extern int tain_from_timeval_sysclock (tain *, struct timeval const *) ;
extern int timeval_from_tain_relative (struct timeval *, tain const *) ;
extern int timeval_from_tain (struct timeval *, tain const *) ;
extern int timeval_sysclock_from_tain (struct timeval *, tain const *) ;

extern int tain_relative_from_timespec (tain *, struct timespec const *) ;
extern int tain_from_timespec (tain *, struct timespec const *) ;
extern int tain_from_timespec_sysclock (tain *, struct timespec const *) ;
extern int timespec_from_tain_relative (struct timespec *, tain const *) ;
extern int timespec_from_tain (struct timespec *, tain const *) ;
extern int timespec_sysclock_from_tain (struct timespec *, tain const *) ;

typedef int tain_clockread_func (tain *) ;
typedef tain_clockread_func *tain_clockread_func_ref ;

extern int tain_from_sysclock (tain *, tain const *) ;
extern int sysclock_from_tain (tain *, tain const *) ;
extern tain_clockread_func sysclock_get ;
extern tain_clockread_func tain_wallclock_read ;
#define tain_wallclock_read_g() tain_wallclock_read(&STAMP)
extern int tain_stopwatch_init (tain *, clock_t, tain *) ;
extern int tain_stopwatch_read (tain *, clock_t, tain const *) ;
#define tain_stopwatch_read_g(cl, offset) tain_stopwatch_read(&STAMP, (cl), offset)
extern tain_clockread_func_ref tain_now ;
#define tain_now_g() (*tain_now)(&STAMP)
#define tain_copynow(t) (*(t) = STAMP)

extern tain_clockread_func tain_now_set_wallclock ;
#define tain_now_set_wallclock_g() tain_now_set_wallclock(&STAMP)
extern tain_clockread_func tain_now_set_stopwatch ;
#define tain_now_set_stopwatch_g() tain_now_set_stopwatch(&STAMP)

extern int sysclock_set (tain const *) ;
extern int tain_setnow (tain const *) ;

extern double tain_approx (tain const *) gccattr_pure ;
extern double tain_frac (tain const *) gccattr_pure ;

extern int tain_from_millisecs (tain *, int) ;
extern int tain_to_millisecs (tain const *) gccattr_pure ;

extern int tain_add (tain *, tain const *, tain const *) ;
#define tain_add_g(deadline, tto) tain_add(deadline, &STAMP, tto)
extern int tain_addsec (tain *, tain const *, int) ;
#define tain_addsec_g(deadline, n) tain_addsec(deadline, &STAMP, n)
extern int tain_addmsec (tain *, tain const *, unsigned int) ;
#define tain_addmsec_g(deadline, n) tain_addmillisec(deadline, &STAMP, n)
extern int tain_sub (tain *, tain const *, tain const *) ;
extern int tain_less (tain const *, tain const *) gccattr_pure ;
#define tain_future(deadline) tain_less(&STAMP, (deadline))

extern void tain_earliest1 (tain *, tain const *) ;
extern void tain_earliestv (tain *, tain const *const *, unsigned int) ;
#define tain_array(...) ((tain const *const[]){__VA_ARGS__})
#define tain_earliestn(t, n, ...) tain_earliestv(t, tain_array(__VA_ARGS__), (n))
#define tain_earliest(t, ...) tain_earliestn(t, sizeof(tain_array(__VA_ARGS__))/sizeof(tain const *), __VA_ARGS__)

#define TAIN_PACK 12
extern void tain_pack (char *, tain const *) ;
extern void tain_unpack (char const *, tain *) ;
extern void tain_pack_little (char *, tain const *) ;
extern void tain_unpack_little (char const *, tain *) ;

#define TAIN_FMT 25
extern size_t tain_fmt (char *, tain const *) ;
extern size_t tain_scan (char const *, tain *) ;

#define TAIN_FMTFRAC 19
extern size_t tain_fmtfrac (char *, tain const *) ;

#define tain_uint(a, u) tain_ulong(a, u)
extern int tain_ulong (tain *, unsigned long) ;
extern void tain_half (tain *, tain const *) ;

#define TIMESTAMP (1 + (TAIN_PACK << 1))
extern size_t timestamp_fmt (char *, tain const *) ;
extern size_t timestamp_scan (char const *, tain *) ;
extern int timestamp_r (char *, tain *) ;
extern int timestamp (char *) ;
#define timestamp_g(s) timestamp_fmt((s), &STAMP)

#endif
