/* ISC license. */

#ifndef SKALIBS_UINT32_H
#define SKALIBS_UINT32_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/uint64.h>

#define UINT32_BSWAP(a) (((a) & 0x000000ffu) << 24 | ((a) & 0x0000ff00u) << 8 | ((a) & 0x00ff0000u) >> 8 | ((a) & 0xff000000u) >> 24)

#define UINT32_LITTLE(u) (u)
#define UINT32_BIG(u) UINT32_BSWAP(u)

#define uint32_little(u) (u)
#define uint32_big(u) uint32_bswap(u)

#define uint32_littlep(u)
#define uint32_bigp(u) uint32_bswapp(u)

#define uint32_littlen(array, n)
#define uint32_bign(array, n) uint32_bswapn(array, n)

extern void uint32_pack (char *, uint32_t) ;
extern void uint32_pack_big (char *, uint32_t) ;
extern void uint32_unpack (char const *, uint32_t *) ;
extern void uint32_unpack_big (char const *, uint32_t *) ;
extern uint32_t uint32_bswap (uint32_t) ;
extern void uint32_bswapp (uint32_t *) ;
extern void uint32_bswapn (uint32_t *, size_t) ;

#define UINT32_FMT 11
#define UINT32_OFMT 13
#define UINT32_XFMT 9
#define UINT32_BFMT 33

#define uint32_fmt_base uint64_fmt_generic
#define uint320_fmt_base uint640_fmt_generic
#define uint32_fmt(s, u) uint32_fmt_base(s, (u), 10)
#define uint320_fmt(s, u, n) uint320_fmt_base(s, u, (n), 10)
#define uint32_ofmt(s, o) uint32_fmt_base(s, (o), 8)
#define uint320_ofmt(s, o, n) uint320_fmt_base(s, o, (n), 8)
#define uint32_xfmt(s, x) uint32_fmt_base(s, (x), 16)
#define uint320_xfmt(s, x, n) uint320_fmt_base(s, x, (n), 16)
#define uint32_bfmt(s, b) uint32_fmt_base(s, (b), 2)
#define uint320_bfmt(s, b, n) uint320_fmt_base(s, b, (n), 2)

extern size_t uint32_fmtlist (char *, uint32_t const *, size_t) ;

extern size_t uint32_scan_base (char const *, uint32_t *, uint8_t) ;
extern size_t uint320_scan_base (char const *, uint32_t *, uint8_t) ;

#define uint32_scan(s, u) uint32_scan_base(s, (u), 10)
#define uint320_scan(s, u) uint320_scan_base(s, (u), 10)
#define uint32_oscan(s, u) uint32_scan_base(s, (u), 8)
#define uint320_oscan(s, u) uint320_scan_base(s, (u), 8)
#define uint32_xscan(s, u) uint32_scan_base(s, (u), 16)
#define uint320_xscan(s, u) uint320_scan_base(s, (u), 16)
#define uint32_bscan(s, u) uint32_scan_base(s, (u), 2)
#define uint320_bscan(s, u) uint320_scan_base(s, (u), 2)

extern size_t uint32_scanlist (uint32_t *, size_t, char const *, size_t *) ;

#define int32_fmt_base int64_fmt_generic
#define int32_fmt(s, u) int32_fmt_base(s, (u), 10)
#define int32_ofmt(s, o) int32_fmt_base(s, (o), 8)
#define int32_xfmt(s, x) int32_fmt_base(s, (x), 16)
#define int32_bfmt(s, b) int32_fmt_base(s, (b), 2)

extern size_t int32_fmtlist (char *, int32_t const *, size_t) ;

extern size_t int32_scan_base (char const *, int32_t *, uint8_t) ;
extern size_t int320_scan_base (char const *, int32_t *, uint8_t) ;

#define int32_scan(s, u) int32_scan_base(s, (u), 10)
#define int320_scan(s, u) int320_scan_base(s, (u), 10)
#define int32_oscan(s, u) int32_scan_base(s, (u), 8)
#define int320_oscan(s, u) int320_scan_base(s, (u), 8)
#define int32_xscan(s, u) int32_scan_base(s, (u), 16)
#define int320_xscan(s, u) int320_scan_base(s, (u), 16)
#define int32_bscan(s, u) int32_scan_base(s, (u), 2)
#define int320_bscan(s, u) int320_scan_base(s, (u), 2)

extern size_t int32_scanlist (int32_t *, size_t, char const *, size_t *) ;

#endif
