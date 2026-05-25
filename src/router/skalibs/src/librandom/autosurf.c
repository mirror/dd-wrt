/* ISC license. */

#include <unistd.h>
#include <stdint.h>

#include <skalibs/uint32.h>
#include <skalibs/tai.h>
#include <skalibs/sha1.h>
#include <skalibs/surf.h>

/*
   Writes 160 bytes of crap into s.
   Certainly not cryptographically secure or 100% unpredictable,
   but we're only using this to seed a fallback internal PRNG.
*/

static void makeseed (char *s)
{
  SHA1Schedule bak = SHA1_INIT() ;
  {
    tain now ;
    char tmp[256] ;
    uint32_t x = getpid() ;
    uint32_pack(tmp, x) ;  /* if valgrind sends you here, tell it to stfu */
    x = getppid() ;
    uint32_pack(tmp + 4, x) ;
    tain_now(&now) ;
    tain_pack(tmp + 8, &now) ;
    sha1_update(&bak, tmp, 8 + TAIN_PACK) ;
    gethostname(tmp, 256) ;
    sha1_update(&bak, tmp, 256) ;
    sha1_final(&bak, tmp) ;
    sha1_init(&bak) ;
    sha1_update(&bak, tmp, 20) ;
  }
  {
    char i = 0 ;
    for (; i < 8 ; i++)
    {
      SHA1Schedule ctx = bak ;
      sha1_update(&ctx, &i, 1) ;
      sha1_final(&ctx, s + 20*i) ;
    }
  }
}

void autosurf (char *s, size_t n)
{
  static SURFSchedule ctx = SURFSCHEDULE_ZERO ;
  static int need4seed = 1 ;
  if (need4seed)
  {
    char seed[160] ;
    makeseed(seed) ;
    surf_init(&ctx, seed) ;
    need4seed = 0 ;
  }
  surf(&ctx, s, n) ;
}
