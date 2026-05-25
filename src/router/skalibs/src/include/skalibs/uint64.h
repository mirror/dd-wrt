/* ISC license. */

#ifndef SKALIBS_UINT64_H
#define SKALIBS_UINT64_H

#include <stddef.h>
#include <stdint.h>

extern size_t uint64_fmt_generic (char *, uint64_t, uint8_t) ;
extern size_t uint640_fmt_generic (char *, uint64_t, size_t, uint8_t) ;
extern size_t int64_fmt_generic (char *, int64_t, uint8_t) ;
extern size_t uint64_fmtlist_generic (char *, void const *, size_t, uint8_t, uint64_t (*)(void const *, size_t)) ;

extern size_t uint64_scan_base_max (char const *, uint64_t *, uint8_t, uint64_t) ;
extern size_t int64_scan_base_max (char const *, int64_t *, uint8_t, uint64_t) ;


#define UINT64_BSWAP(a) (((a) & 0x00000000000000ffull) << 56 | ((a) & 0x000000000000ff00ull) << 40 | ((a) & 0x0000000000ff0000ull) << 24 | ((a) & 0x00000000ff000000ull) << 8 | ((a) & 0x000000ff00000000ull) >> 8 | ((a) & 0x0000ff0000000000ull) >> 24 | ((a) & 0x00ff000000000000ull) >> 40 | ((a) & 0xff00000000000000ull) >> 56)

#define UINT64_LITTLE(u) (u)
#define UINT64_BIG(u) UINT64_BSWAP(u)

#define uint64_little(u) (u)
#define uint64_big(u) uint64_bswap(u)

#define uint64_littlep(u)
#define uint64_bigp(u) uint64_bswapp(u)

#define uint64_littlen(array, n)
#define uint64_bign(array, n) uint64_bswapn(array, n)

extern void uint64_pack (char *, uint64_t) ;
extern void uint64_pack_big (char *, uint64_t) ;
extern void uint64_unpack (char const *, uint64_t *) ;
extern void uint64_unpack_big (char const *, uint64_t *) ;
extern uint64_t uint64_bswap (uint64_t) ;
extern void uint64_bswapp (uint64_t *) ;
extern void uint64_bswapn (uint64_t *, size_t) ;

#define UINT64_FMT 21
#define UINT64_OFMT 25
#define UINT64_XFMT 17
#define UINT64_BFMT 65

#define uint64_fmt_base uint64_fmt_generic
#define uint640_fmt_base uint640_fmt_generic
#define uint64_fmt(s, u) uint64_fmt_base(s, (u), 10)
#define uint640_fmt(s, u, n) uint640_fmt_base(s, u, (n), 10)
#define uint64_ofmt(s, o) uint64_fmt_base(s, (o), 8)
#define uint640_ofmt(s, o, n) uint640_fmt_base(s, o, (n), 8)
#define uint64_xfmt(s, x) uint64_fmt_base(s, (x), 16)
#define uint640_xfmt(s, x, n) uint640_fmt_base(s, x, (n), 16)
#define uint64_bfmt(s, b) uint64_fmt_base(s, (b), 2)
#define uint640_bfmt(s, b, n) uint640_fmt_base(s, b, (n), 2)

extern size_t uint64_fmtlist (char *, uint64_t const *, size_t) ;

extern size_t uint64_scan_base (char const *, uint64_t *, uint8_t) ;
extern size_t uint640_scan_base (char const *, uint64_t *, uint8_t) ;

#define uint64_scan(s, u) uint64_scan_base(s, (u), 10)
#define uint640_scan(s, u) uint640_scan_base(s, (u), 10)
#define uint64_oscan(s, u) uint64_scan_base(s, (u), 8)
#define uint640_oscan(s, u) uint640_scan_base(s, (u), 8)
#define uint64_xscan(s, u) uint64_scan_base(s, (u), 16)
#define uint640_xscan(s, u) uint640_scan_base(s, (u), 16)
#define uint64_bscan(s, u) uint64_scan_base(s, (u), 2)
#define uint640_bscan(s, u) uint640_scan_base(s, (u), 2)

extern size_t uint64_scanlist (uint64_t *, size_t, char const *, size_t *) ;

#define int64_fmt_base int64_fmt_generic
#define int64_fmt(s, u) int64_fmt_base(s, (u), 10)
#define int64_ofmt(s, o) int64_fmt_base(s, (o), 8)
#define int64_xfmt(s, x) int64_fmt_base(s, (x), 16)
#define int64_bfmt(s, b) int64_fmt_base(s, (b), 2)

extern size_t int64_fmtlist (char *, int64_t const *, size_t) ;

extern size_t int64_scan_base (char const *, int64_t *, uint8_t) ;
extern size_t int640_scan_base (char const *, int64_t *, uint8_t) ;

#define int64_scan(s, u) int64_scan_base(s, (u), 10)
#define int640_scan(s, u) int640_scan_base(s, (u), 10)
#define int64_oscan(s, u) int64_scan_base(s, (u), 8)
#define int640_oscan(s, u) int640_scan_base(s, (u), 8)
#define int64_xscan(s, u) int64_scan_base(s, (u), 16)
#define int640_xscan(s, u) int640_scan_base(s, (u), 16)
#define int64_bscan(s, u) int64_scan_base(s, (u), 2)
#define int640_bscan(s, u) int640_scan_base(s, (u), 2)

extern size_t int64_scanlist (int64_t *, size_t, char const *, size_t *) ;

#endif
