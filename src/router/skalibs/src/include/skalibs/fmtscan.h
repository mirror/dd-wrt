/* ISC license. */

#ifndef SKALIBS_FMTSCAN_H
#define SKALIBS_FMTSCAN_H

#include <sys/types.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>


 /* fmt */

extern size_t str_fmt (char *, char const *) ;
extern size_t strn_fmt (char *, char const *, size_t) ;

extern unsigned char fmtscan_asc (unsigned char) gccattr_const ;

#define IP4_FMT 20
extern size_t ip4_fmt (char *, char const *) ;
extern size_t ip4_fmtu32 (char *, uint32_t) ;

extern size_t ucharn_fmt (char *, char const *, size_t) ;
extern size_t ucharn_fmt_little (char *, char const *, size_t) ;

#define IP6_FMT 40
extern size_t ip6_fmt (char *, char const *) ;


 /* scan */

extern unsigned char fmtscan_num (unsigned char, unsigned char) gccattr_const ;

extern size_t ip4_scan (char const *, char *) ;
extern size_t ip4_scanu32 (char const *, uint32_t *) ;
extern size_t ip4_scanlist_u32 (uint32_t *, size_t, char const *, size_t *) ;
extern size_t ip4_scanlist (char *, size_t, char const *, size_t *) ;
extern size_t ip6_scan (char const *, char *) ;
extern size_t ip6_scanlist (char *, size_t, char const *, size_t *) ;

extern size_t ucharn_scan (char const *, char *, size_t) ;
extern size_t ucharn_scan_little (char const *, char *, size_t) ;
extern size_t ucharn_findlen (char const *) gccattr_pure ;


 /* misc */

extern int ip4_netmask (char *, uint8_t) ;
extern int ip6_netmask (char *, uint8_t) ;

#endif
