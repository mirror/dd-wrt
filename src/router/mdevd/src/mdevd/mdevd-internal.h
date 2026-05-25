/* ISC license. */

#ifndef MDEVD_INTERNAL_H
#define MDEVD_INTERNAL_H

#define UEVENT_MAX_VARS 63
#define UEVENT_MAX_SIZE 8192

#include <stdint.h>
#include <limits.h>

struct uevent_s
{
  unsigned short len ;
  unsigned short varn ;
  unsigned short vars[UEVENT_MAX_VARS + 1] ;
  char buf[UEVENT_MAX_SIZE + PATH_MAX + 5] ;
} ;
#define UEVENT_ZERO { .len = 0, .varn = 0 }

extern int mdevd_netlink_init (unsigned int, unsigned int) ;
extern int mdevd_uevent_read (int, struct uevent_s *, uint32_t, unsigned int) ;
extern char const *mdevd_uevent_getvar (struct uevent_s const *, char const *) ;

#endif
