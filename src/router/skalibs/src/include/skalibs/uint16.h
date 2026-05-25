/* ISC license. */

#ifndef SKALIBS_UINT16_H
#define SKALIBS_UINT16_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/uint64.h>

#define UINT16_BSWAP(a) (((a) & 0x00ffu) << 8 | ((a) & 0xff00u) >> 8)

#define UINT16_LITTLE(u) (u)
#define UINT16_BIG(u) UINT16_BSWAP(u)

#define uint16_little(u) (u)
#define uint16_big(u) uint16_bswap(u)

#define uint16_littlep(u)
#define uint16_bigp(u) uint16_bswapp(u)

#define uint16_littlen(array, n)
#define uint16_bign(array, n) uint16_bswapn(array, n)

extern void uint16_pack (char *, uint16_t) ;
extern void uint16_pack_big (char *, uint16_t) ;
extern void uint16_unpack (char const *, uint16_t *) ;
extern void uint16_unpack_big (char const *, uint16_t *) ;
extern uint16_t uint16_bswap (uint16_t) ;
extern void uint16_bswapp (uint16_t *) ;
extern void uint16_bswapn (uint16_t *, size_t) ;

#define UINT16_FMT 6
#define UINT16_OFMT 7
#define UINT16_XFMT 5
#define UINT16_BFMT 17

#define uint16_fmt_base uint64_fmt_generic
#define uint160_fmt_base uint640_fmt_generic
#define uint16_fmt(s, u) uint16_fmt_base(s, (u), 10)
#define uint160_fmt(s, u, n) uint160_fmt_base(s, u, (n), 10)
#define uint16_ofmt(s, o) uint16_fmt_base(s, (o), 8)
#define uint160_ofmt(s, o, n) uint160_fmt_base(s, o, (n), 8)
#define uint16_xfmt(s, x) uint16_fmt_base(s, (x), 16)
#define uint160_xfmt(s, x, n) uint160_fmt_base(s, x, (n), 16)
#define uint16_bfmt(s, b) uint16_fmt_base(s, (b), 2)
#define uint160_bfmt(s, b, n) uint160_fmt_base(s, b, (n), 2)

extern size_t uint16_fmtlist (char *, uint16_t const *, size_t) ;

extern size_t uint16_scan_base (char const *, uint16_t *, uint8_t) ;
extern size_t uint160_scan_base (char const *, uint16_t *, uint8_t) ;

#define uint16_scan(s, u) uint16_scan_base(s, (u), 10)
#define uint160_scan(s, u) uint160_scan_base(s, (u), 10)
#define uint16_oscan(s, u) uint16_scan_base(s, (u), 8)
#define uint160_oscan(s, u) uint160_scan_base(s, (u), 8)
#define uint16_xscan(s, u) uint16_scan_base(s, (u), 16)
#define uint160_xscan(s, u) uint160_scan_base(s, (u), 16)
#define uint16_bscan(s, u) uint16_scan_base(s, (u), 2)
#define uint160_bscan(s, u) uint160_scan_base(s, (u), 2)

extern size_t uint16_scanlist (uint16_t *, size_t, char const *, size_t *) ;

#define int16_fmt_base int64_fmt_generic
#define int16_fmt(s, u) int16_fmt_base(s, (u), 10)
#define int16_ofmt(s, o) int16_fmt_base(s, (o), 8)
#define int16_xfmt(s, x) int16_fmt_base(s, (x), 16)
#define int16_bfmt(s, b) int16_fmt_base(s, (b), 2)

extern size_t int16_fmtlist (char *, int16_t const *, size_t) ;

extern size_t int16_scan_base (char const *, int16_t *, uint8_t) ;
extern size_t int160_scan_base (char const *, int16_t *, uint8_t) ;

#define int16_scan(s, u) int16_scan_base(s, (u), 10)
#define int160_scan(s, u) int160_scan_base(s, (u), 10)
#define int16_oscan(s, u) int16_scan_base(s, (u), 8)
#define int160_oscan(s, u) int160_scan_base(s, (u), 8)
#define int16_xscan(s, u) int16_scan_base(s, (u), 16)
#define int160_xscan(s, u) int160_scan_base(s, (u), 16)
#define int16_bscan(s, u) int16_scan_base(s, (u), 2)
#define int160_bscan(s, u) int160_scan_base(s, (u), 2)

extern size_t int16_scanlist (int16_t *, size_t, char const *, size_t *) ;

#endif
