/* ISC license. */

#ifndef SKALIBS_DJBTIME_H
#define SKALIBS_DJBTIME_H

#include <stdint.h>
#include <time.h>

#include <skalibs/uint64.h>
#include <skalibs/tai.h>


extern int timespec_cmp (struct timespec const *, struct timespec const *) ;


/* UTC <--> TAI conversions */
/* sysclock can be either TAI-10 or UTC */

extern int utc_from_tai (uint64_t *, tai const *) ;
extern int tai_from_utc (tai *, uint64_t) ;
extern int utc_from_sysclock (uint64_t *) ;
extern int sysclock_from_utc (uint64_t *) ;


/* NTP internal format */

#define NTP_OFFSET 2208988800UL
extern int ntp_from_tain (uint64_t *, tain const *) ;
#define ntp_from_tain_g(u) ntp_from_tain((u), &STAMP)
extern int tain_from_ntp (tain *, uint64_t) ;


/* localtime handling - replaces caltimedate functions */
/* ltm64 can be either TAI-10 or UTC depending on the current timezone */
/* normally ltm64 is the same as sysclock, but we allow it to be different */
/* for instance for musl TAI-10 systems */

typedef struct localtmn_s localtmn, *localtmn_ref ;
struct localtmn_s
{
  struct tm tm ;
  uint32_t nano ;
} ;

extern int ltm64_from_tai (uint64_t *, tai const *) ;
extern int tai_from_ltm64 (tai *, uint64_t) ;
extern int ltm64_from_utc (uint64_t *) ;
extern int utc_from_ltm64 (uint64_t *) ;
extern int ltm64_from_sysclock (uint64_t *) ;
extern int sysclock_from_ltm64 (uint64_t *) ;

extern int localtm_from_ltm64 (struct tm *, uint64_t, int) ;
extern int ltm64_from_localtm (uint64_t *, struct tm const *) ;
extern int localtm_from_sysclock (struct tm *, uint64_t, int) ;
extern int sysclock_from_localtm (uint64_t *, struct tm const *) ;
extern int localtm_from_utc (struct tm *, uint64_t, int) ;
extern int utc_from_localtm (uint64_t *, struct tm const *) ;
extern int localtm_from_tai (struct tm *, tai const *, int) ;
extern int tai_from_localtm (tai *, struct tm const *) ;

extern int localtmn_from_tain (localtmn *, tain const *, int) ;
#define localtmn_from_tain_g(l, h) localtmn_from_tain(l, &STAMP, h)
extern int tain_from_localtmn (tain *, localtmn const *) ;
extern int localtmn_from_sysclock (localtmn *, tain const *, int) ;
extern int sysclock_from_localtmn (tain *, localtmn const *) ;

#define LOCALTM_FMT 21
extern size_t localtm_fmt (char *, struct tm const *) ;
extern size_t localtm_scan (char const *, struct tm *) ;

#define LOCALTMN_FMT 31
extern size_t localtmn_fmt (char *, localtmn const *) ;
extern size_t localtmn_scan (char const *, localtmn *) ;

#endif
