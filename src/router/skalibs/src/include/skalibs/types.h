/* ISC license. */

#ifndef SKALIBS_TYPES_H
#define SKALIBS_TYPES_H

#include <sys/types.h>
#include <stdint.h>

#include <skalibs/uint16.h>
#include <skalibs/uint32.h>
#include <skalibs/uint64.h>


#define USHORT_PACK 2
#define ushort_pack uint16_pack
#define ushort_pack_big uint16_pack_big
#define ushort_unpack uint16_unpack
#define ushort_unpack_big uint16_unpack_big

#define ushort_little uint16_little
#define ushort_big uint16_big
#define ushort_littlep uint16_littlep
#define ushort_bigp uint16_bigp
#define ushort_littlen uint16_littlen
#define ushort_bign uint16_bign

#define USHORT_FMT UINT16_FMT
#define USHORT_OFMT UINT16_OFMT
#define USHORT_XFMT UINT16_XFMT
#define USHORT_BFMT UINT16_BFMT

#define ushort_fmt_base uint16_fmt_base
#define ushort0_fmt_base uint160_fmt_base
#define ushort_fmt uint16_fmt
#define ushort0_fmt uint160_fmt
#define ushort_ofmt uint16_ofmt
#define ushort0_ofmt uint160_ofmt
#define ushort_xfmt uint16_xfmt
#define ushort0_xfmt uint160_xfmt
#define ushort_bfmt uint16_bfmt
#define ushort0_bfmt uint160_bfmt

#define ushort_fmtlist uint16_fmtlist

#define ushort_scan_base(s, u, b) uint16_scan_base(s, (uint16_t *)u, b)
#define ushort0_scan_base(s, u, b) uint160_scan_base(s, (uint16_t *)u, b)
#define ushort_scanlist(tab, max, s, num) uint16_scanlist((uint16_t *)tab, max, s, num)

#define ushort_scan(s, u) ushort_scan_base(s, (u), 10)
#define ushort0_scan(s, u) ushort0_scan_base(s, (u), 10)
#define ushort_oscan(s, u) ushort_scan_base(s, (u), 8)
#define ushort0_oscan(s, u) ushort0_scan_base(s, (u), 8)
#define ushort_xscan(s, u) ushort_scan_base(s, (u), 16)
#define ushort0_xscan(s, u) ushort0_scan_base(s, (u), 16)
#define ushort_bscan(s, u) ushort_scan_base(s, (u), 2)
#define ushort0_bscan(s, u) ushort0_scan_base(s, (u), 2)

#define SHORT_PACK 2
#define short_pack uint16_pack
#define short_pack_big uint16_pack_big
#define short_unpack(pack, p) uint16_unpack(pack, (uint16_t *)(p))
#define short_unpack_big(pack, p) uint16_unpack_big(pack, (uint16_t *)(p))

#define SHORT_FMT (1+UINT16_FMT)
#define short_fmt int16_fmt
#define short_fmtlist int16_fmtlist
#define short_scan(s, d) int16_scan(s, (int16_t *)d)
#define short0_scan(s, d) int160_scan(s, (int16_t *)d)
#define short_scanlist(tab, max, s, num) int16_scanlist((int16_t *)tab, max, s, num)


#define UINT_PACK 4
#define uint_pack uint32_pack
#define uint_pack_big uint32_pack_big
#define uint_unpack uint32_unpack
#define uint_unpack_big uint32_unpack_big

#define uint_little uint32_little
#define uint_big uint32_big
#define uint_littlep uint32_littlep
#define uint_bigp uint32_bigp
#define uint_littlen uint32_littlen
#define uint_bign uint32_bign

#define UINT_FMT UINT32_FMT
#define UINT_OFMT UINT32_OFMT
#define UINT_XFMT UINT32_XFMT
#define UINT_BFMT UINT32_BFMT

#define uint_fmt_base uint32_fmt_base
#define uint0_fmt_base uint320_fmt_base
#define uint_fmt uint32_fmt
#define uint0_fmt uint320_fmt
#define uint_ofmt uint32_ofmt
#define uint0_ofmt uint320_ofmt
#define uint_xfmt uint32_xfmt
#define uint0_xfmt uint320_xfmt
#define uint_bfmt uint32_bfmt
#define uint0_bfmt uint320_bfmt

#define uint_fmtlist uint32_fmtlist

#define uint_scan_base(s, u, b) uint32_scan_base(s, (uint32_t *)u, b)
#define uint0_scan_base(s, u, b) uint320_scan_base(s, (uint32_t *)u, b)
#define uint_scanlist(tab, max, s, num) uint32_scanlist((uint32_t *)tab, max, s, num)

#define uint_scan(s, u) uint_scan_base(s, (u), 10)
#define uint0_scan(s, u) uint0_scan_base(s, (u), 10)
#define uint_oscan(s, u) uint_scan_base(s, (u), 8)
#define uint0_oscan(s, u) uint0_scan_base(s, (u), 8)
#define uint_xscan(s, u) uint_scan_base(s, (u), 16)
#define uint0_xscan(s, u) uint0_scan_base(s, (u), 16)
#define uint_bscan(s, u) uint_scan_base(s, (u), 2)
#define uint0_bscan(s, u) uint0_scan_base(s, (u), 2)

#define INT_PACK 4
#define int_pack uint32_pack
#define int_pack_big uint32_pack_big
#define int_unpack(pack, p) uint32_unpack(pack, (uint32_t *)(p))
#define int_unpack_big(pack, p) uint32_unpack_big(pack, (uint32_t *)(p))

#define INT_FMT (1+UINT32_FMT)
#define int_fmt int32_fmt
#define int_fmtlist int32_fmtlist
#define int_scan(s, d) int32_scan(s, (int32_t *)d)
#define int0_scan(s, d) int320_scan(s, (int32_t *)d)
#define int_scanlist(tab, max, s, num) int32_scanlist((int32_t *)tab, max, s, num)


#define ULONG_PACK 8
#define ulong_pack uint64_pack
#define ulong_pack_big uint64_pack_big
#define ulong_unpack uint64_unpack
#define ulong_unpack_big uint64_unpack_big

#define ulong_little uint64_little
#define ulong_big uint64_big
#define ulong_littlep uint64_littlep
#define ulong_bigp uint64_bigp
#define ulong_littlen uint64_littlen
#define ulong_bign uint64_bign

#define ULONG_FMT UINT64_FMT
#define ULONG_OFMT UINT64_OFMT
#define ULONG_XFMT UINT64_XFMT
#define ULONG_BFMT UINT64_BFMT

#define ulong_fmt_base uint64_fmt_base
#define ulong0_fmt_base uint640_fmt_base
#define ulong_fmt uint64_fmt
#define ulong0_fmt uint640_fmt
#define ulong_ofmt uint64_ofmt
#define ulong0_ofmt uint640_ofmt
#define ulong_xfmt uint64_xfmt
#define ulong0_xfmt uint640_xfmt
#define ulong_bfmt uint64_bfmt
#define ulong0_bfmt uint640_bfmt

#define ulong_fmtlist uint64_fmtlist

#define ulong_scan_base(s, u, b) uint64_scan_base(s, (uint64_t *)u, b)
#define ulong0_scan_base(s, u, b) uint640_scan_base(s, (uint64_t *)u, b)
#define ulong_scanlist(tab, max, s, num) uint64_scanlist((uint64_t *)tab, max, s, num)

#define ulong_scan(s, u) ulong_scan_base(s, (u), 10)
#define ulong0_scan(s, u) ulong0_scan_base(s, (u), 10)
#define ulong_oscan(s, u) ulong_scan_base(s, (u), 8)
#define ulong0_oscan(s, u) ulong0_scan_base(s, (u), 8)
#define ulong_xscan(s, u) ulong_scan_base(s, (u), 16)
#define ulong0_xscan(s, u) ulong0_scan_base(s, (u), 16)
#define ulong_bscan(s, u) ulong_scan_base(s, (u), 2)
#define ulong0_bscan(s, u) ulong0_scan_base(s, (u), 2)

#define LONG_PACK 8
#define long_pack uint64_pack
#define long_pack_big uint64_pack_big
#define long_unpack(pack, p) uint64_unpack(pack, (uint64_t *)(p))
#define long_unpack_big(pack, p) uint64_unpack_big(pack, (uint64_t *)(p))

#define LONG_FMT (1+UINT64_FMT)
#define long_fmt int64_fmt
#define long_fmtlist int64_fmtlist
#define long_scan(s, d) int64_scan(s, (int64_t *)d)
#define long0_scan(s, d) int640_scan(s, (int64_t *)d)
#define long_scanlist(tab, max, s, num) int64_scanlist((int64_t *)tab, max, s, num)


#define SIZE_PACK 8
#define size_pack uint64_pack
#define size_pack_big uint64_pack_big
#define size_unpack uint64_unpack
#define size_unpack_big uint64_unpack_big

#define size_little uint64_little
#define size_big uint64_big
#define size_littlep uint64_littlep
#define size_bigp uint64_bigp
#define size_littlen uint64_littlen
#define size_bign uint64_bign

#define SIZE_FMT UINT64_FMT
#define SIZE_OFMT UINT64_OFMT
#define SIZE_XFMT UINT64_XFMT
#define SIZE_BFMT UINT64_BFMT

#define size_fmt_base uint64_fmt_base
#define size0_fmt_base uint640_fmt_base
#define size_fmt uint64_fmt
#define size0_fmt uint640_fmt
#define size_ofmt uint64_ofmt
#define size0_ofmt uint640_ofmt
#define size_xfmt uint64_xfmt
#define size0_xfmt uint640_xfmt
#define size_bfmt uint64_bfmt
#define size0_bfmt uint640_bfmt

#define size_fmtlist uint64_fmtlist

#define size_scan_base(s, u, b) uint64_scan_base(s, (uint64_t *)u, b)
#define size0_scan_base(s, u, b) uint640_scan_base(s, (uint64_t *)u, b)
#define size_scanlist(tab, max, s, num) uint64_scanlist((uint64_t *)tab, max, s, num)

#define size_scan(s, u) size_scan_base(s, (u), 10)
#define size0_scan(s, u) size0_scan_base(s, (u), 10)
#define size_oscan(s, u) size_scan_base(s, (u), 8)
#define size0_oscan(s, u) size0_scan_base(s, (u), 8)
#define size_xscan(s, u) size_scan_base(s, (u), 16)
#define size0_xscan(s, u) size0_scan_base(s, (u), 16)
#define size_bscan(s, u) size_scan_base(s, (u), 2)
#define size0_bscan(s, u) size0_scan_base(s, (u), 2)

#define UID_PACK 4
#define uid_pack uint32_pack
#define uid_pack_big uint32_pack_big
#define uid_unpack uint32_unpack
#define uid_unpack_big uint32_unpack_big

#define uid_little uint32_little
#define uid_big uint32_big
#define uid_littlep uint32_littlep
#define uid_bigp uint32_bigp
#define uid_littlen uint32_littlen
#define uid_bign uint32_bign

#define UID_FMT UINT32_FMT
#define UID_OFMT UINT32_OFMT
#define UID_XFMT UINT32_XFMT
#define UID_BFMT UINT32_BFMT

#define uid_fmt_base uint32_fmt_base
#define uid0_fmt_base uint320_fmt_base
#define uid_fmt uint32_fmt
#define uid0_fmt uint320_fmt
#define uid_ofmt uint32_ofmt
#define uid0_ofmt uint320_ofmt
#define uid_xfmt uint32_xfmt
#define uid0_xfmt uint320_xfmt
#define uid_bfmt uint32_bfmt
#define uid0_bfmt uint320_bfmt

#define uid_fmtlist uint32_fmtlist

#define uid_scan_base(s, u, b) uint32_scan_base(s, (uint32_t *)u, b)
#define uid0_scan_base(s, u, b) uint320_scan_base(s, (uint32_t *)u, b)
#define uid_scanlist(tab, max, s, num) uint32_scanlist((uint32_t *)tab, max, s, num)

#define uid_scan(s, u) uid_scan_base(s, (u), 10)
#define uid0_scan(s, u) uid0_scan_base(s, (u), 10)
#define uid_oscan(s, u) uid_scan_base(s, (u), 8)
#define uid0_oscan(s, u) uid0_scan_base(s, (u), 8)
#define uid_xscan(s, u) uid_scan_base(s, (u), 16)
#define uid0_xscan(s, u) uid0_scan_base(s, (u), 16)
#define uid_bscan(s, u) uid_scan_base(s, (u), 2)
#define uid0_bscan(s, u) uid0_scan_base(s, (u), 2)

#define GID_PACK 4
#define gid_pack uint32_pack
#define gid_pack_big uint32_pack_big
#define gid_unpack uint32_unpack
#define gid_unpack_big uint32_unpack_big

#define gid_little uint32_little
#define gid_big uint32_big
#define gid_littlep uint32_littlep
#define gid_bigp uint32_bigp
#define gid_littlen uint32_littlen
#define gid_bign uint32_bign

#define GID_FMT UINT32_FMT
#define GID_OFMT UINT32_OFMT
#define GID_XFMT UINT32_XFMT
#define GID_BFMT UINT32_BFMT

#define gid_fmt_base uint32_fmt_base
#define gid0_fmt_base uint320_fmt_base
#define gid_fmt uint32_fmt
#define gid0_fmt uint320_fmt
#define gid_ofmt uint32_ofmt
#define gid0_ofmt uint320_ofmt
#define gid_xfmt uint32_xfmt
#define gid0_xfmt uint320_xfmt
#define gid_bfmt uint32_bfmt
#define gid0_bfmt uint320_bfmt

#define gid_fmtlist uint32_fmtlist

#define gid_scan_base(s, u, b) uint32_scan_base(s, (uint32_t *)u, b)
#define gid0_scan_base(s, u, b) uint320_scan_base(s, (uint32_t *)u, b)
#define gid_scanlist(tab, max, s, num) uint32_scanlist((uint32_t *)tab, max, s, num)

#define gid_scan(s, u) gid_scan_base(s, (u), 10)
#define gid0_scan(s, u) gid0_scan_base(s, (u), 10)
#define gid_oscan(s, u) gid_scan_base(s, (u), 8)
#define gid0_oscan(s, u) gid0_scan_base(s, (u), 8)
#define gid_xscan(s, u) gid_scan_base(s, (u), 16)
#define gid0_xscan(s, u) gid0_scan_base(s, (u), 16)
#define gid_bscan(s, u) gid_scan_base(s, (u), 2)
#define gid0_bscan(s, u) gid0_scan_base(s, (u), 2)

#define PID_PACK 4
#define pid_pack uint32_pack
#define pid_pack_big uint32_pack_big
#define pid_unpack(pack, p) uint32_unpack(pack, (uint32_t *)(p))
#define pid_unpack_big(pack, p) uint32_unpack_big(pack, (uint32_t *)(p))

#define PID_FMT (1+UINT32_FMT)
#define pid_fmt int32_fmt
#define pid_fmtlist int32_fmtlist
#define pid_scan(s, d) int32_scan(s, (int32_t *)d)
#define pid0_scan(s, d) int320_scan(s, (int32_t *)d)
#define pid_scanlist(tab, max, s, num) int32_scanlist((int32_t *)tab, max, s, num)


#define TIME_PACK 8
#define time_pack uint64_pack
#define time_pack_big uint64_pack_big
#define time_unpack(pack, p) uint64_unpack(pack, (uint64_t *)(p))
#define time_unpack_big(pack, p) uint64_unpack_big(pack, (uint64_t *)(p))

#define TIME_FMT (1+UINT64_FMT)
#define time_fmt int64_fmt
#define time_fmtlist int64_fmtlist
#define time_scan(s, d) int64_scan(s, (int64_t *)d)
#define time0_scan(s, d) int640_scan(s, (int64_t *)d)
#define time_scanlist(tab, max, s, num) int64_scanlist((int64_t *)tab, max, s, num)


#define DEV_PACK 8
#define dev_pack uint64_pack
#define dev_pack_big uint64_pack_big
#define dev_unpack uint64_unpack
#define dev_unpack_big uint64_unpack_big

#define dev_little uint64_little
#define dev_big uint64_big
#define dev_littlep uint64_littlep
#define dev_bigp uint64_bigp
#define dev_littlen uint64_littlen
#define dev_bign uint64_bign

#define DEV_FMT UINT64_FMT
#define DEV_OFMT UINT64_OFMT
#define DEV_XFMT UINT64_XFMT
#define DEV_BFMT UINT64_BFMT

#define dev_fmt_base uint64_fmt_base
#define dev0_fmt_base uint640_fmt_base
#define dev_fmt uint64_fmt
#define dev0_fmt uint640_fmt
#define dev_ofmt uint64_ofmt
#define dev0_ofmt uint640_ofmt
#define dev_xfmt uint64_xfmt
#define dev0_xfmt uint640_xfmt
#define dev_bfmt uint64_bfmt
#define dev0_bfmt uint640_bfmt

#define dev_fmtlist uint64_fmtlist

#define dev_scan_base(s, u, b) uint64_scan_base(s, (uint64_t *)u, b)
#define dev0_scan_base(s, u, b) uint640_scan_base(s, (uint64_t *)u, b)
#define dev_scanlist(tab, max, s, num) uint64_scanlist((uint64_t *)tab, max, s, num)

#define dev_scan(s, u) dev_scan_base(s, (u), 10)
#define dev0_scan(s, u) dev0_scan_base(s, (u), 10)
#define dev_oscan(s, u) dev_scan_base(s, (u), 8)
#define dev0_oscan(s, u) dev0_scan_base(s, (u), 8)
#define dev_xscan(s, u) dev_scan_base(s, (u), 16)
#define dev0_xscan(s, u) dev0_scan_base(s, (u), 16)
#define dev_bscan(s, u) dev_scan_base(s, (u), 2)
#define dev0_bscan(s, u) dev0_scan_base(s, (u), 2)

#define INO_PACK 8
#define ino_pack uint64_pack
#define ino_pack_big uint64_pack_big
#define ino_unpack uint64_unpack
#define ino_unpack_big uint64_unpack_big

#define ino_little uint64_little
#define ino_big uint64_big
#define ino_littlep uint64_littlep
#define ino_bigp uint64_bigp
#define ino_littlen uint64_littlen
#define ino_bign uint64_bign

#define INO_FMT UINT64_FMT
#define INO_OFMT UINT64_OFMT
#define INO_XFMT UINT64_XFMT
#define INO_BFMT UINT64_BFMT

#define ino_fmt_base uint64_fmt_base
#define ino0_fmt_base uint640_fmt_base
#define ino_fmt uint64_fmt
#define ino0_fmt uint640_fmt
#define ino_ofmt uint64_ofmt
#define ino0_ofmt uint640_ofmt
#define ino_xfmt uint64_xfmt
#define ino0_xfmt uint640_xfmt
#define ino_bfmt uint64_bfmt
#define ino0_bfmt uint640_bfmt

#define ino_fmtlist uint64_fmtlist

#define ino_scan_base(s, u, b) uint64_scan_base(s, (uint64_t *)u, b)
#define ino0_scan_base(s, u, b) uint640_scan_base(s, (uint64_t *)u, b)
#define ino_scanlist(tab, max, s, num) uint64_scanlist((uint64_t *)tab, max, s, num)

#define ino_scan(s, u) ino_scan_base(s, (u), 10)
#define ino0_scan(s, u) ino0_scan_base(s, (u), 10)
#define ino_oscan(s, u) ino_scan_base(s, (u), 8)
#define ino0_oscan(s, u) ino0_scan_base(s, (u), 8)
#define ino_xscan(s, u) ino_scan_base(s, (u), 16)
#define ino0_xscan(s, u) ino0_scan_base(s, (u), 16)
#define ino_bscan(s, u) ino_scan_base(s, (u), 2)
#define ino0_bscan(s, u) ino0_scan_base(s, (u), 2)


#endif
