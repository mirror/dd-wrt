/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __XFS_ARCH_H__
#define __XFS_ARCH_H__

#if __BYTE_ORDER == __BIG_ENDIAN
#define	XFS_NATIVE_HOST	1
#else
#undef XFS_NATIVE_HOST
#endif

#ifdef __CHECKER__
#define __bitwise		__attribute__((bitwise))
#define __force			__attribute__((force))
#else
#define __bitwise
#define __force
#endif

typedef __u16	__bitwise	__le16;
typedef __u32	__bitwise	__le32;
typedef __u64	__bitwise	__le64;

typedef __u16	__bitwise	__be16;
typedef __u32	__bitwise	__be32;
typedef __u64	__bitwise	__be64;

/*
 * Casts are necessary for constants, because we never know how for sure
 * how U/UL/ULL map to __u16, __u32, __u64. At least not in a portable way.
 */
#define ___swab16(x) \
({ \
	__u16 __x = (x); \
	((__u16)( \
		(((__u16)(__x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(__x) & (__u16)0xff00U) >> 8) )); \
})

#define ___swab32(x) \
({ \
	__u32 __x = (x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})

#define ___swab64(x) \
({ \
	__u64 __x = (x); \
	((__u64)( \
		(__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
		(__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
		(__u64)(((__u64)(__x) & (__u64)0x000000ff00000000ULL) >>  8) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
		(__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) )); \
})

#define ___constant_swab16(x) \
	((__u16)( \
		(((__u16)(x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(x) & (__u16)0xff00U) >> 8) ))
#define ___constant_swab32(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x) & (__u32)0xff000000UL) >> 24) ))
#define ___constant_swab64(x) \
	((__u64)( \
		(__u64)(((__u64)(x) & (__u64)0x00000000000000ffULL) << 56) | \
		(__u64)(((__u64)(x) & (__u64)0x000000000000ff00ULL) << 40) | \
		(__u64)(((__u64)(x) & (__u64)0x0000000000ff0000ULL) << 24) | \
		(__u64)(((__u64)(x) & (__u64)0x00000000ff000000ULL) <<  8) | \
		(__u64)(((__u64)(x) & (__u64)0x000000ff00000000ULL) >>  8) | \
		(__u64)(((__u64)(x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
		(__u64)(((__u64)(x) & (__u64)0x00ff000000000000ULL) >> 40) | \
		(__u64)(((__u64)(x) & (__u64)0xff00000000000000ULL) >> 56) ))

/*
 * provide defaults when no architecture-specific optimization is detected
 */
#ifndef __arch__swab16
#  define __arch__swab16(x) ({ __u16 __tmp = (x) ; ___swab16(__tmp); })
#endif
#ifndef __arch__swab32
#  define __arch__swab32(x) ({ __u32 __tmp = (x) ; ___swab32(__tmp); })
#endif
#ifndef __arch__swab64
#  define __arch__swab64(x) ({ __u64 __tmp = (x) ; ___swab64(__tmp); })
#endif

#ifndef __arch__swab16p
#  define __arch__swab16p(x) __arch__swab16(*(x))
#endif
#ifndef __arch__swab32p
#  define __arch__swab32p(x) __arch__swab32(*(x))
#endif
#ifndef __arch__swab64p
#  define __arch__swab64p(x) __arch__swab64(*(x))
#endif

#ifndef __arch__swab16s
#  define __arch__swab16s(x) do { *(x) = __arch__swab16p((x)); } while (0)
#endif
#ifndef __arch__swab32s
#  define __arch__swab32s(x) do { *(x) = __arch__swab32p((x)); } while (0)
#endif
#ifndef __arch__swab64s
#  define __arch__swab64s(x) do { *(x) = __arch__swab64p((x)); } while (0)
#endif


/*
 * Allow constant folding
 */
#  define __swab16(x) \
(__builtin_constant_p((__u16)(x)) ? \
 ___constant_swab16((x)) : \
 __fswab16((x)))
#  define __swab32(x) \
(__builtin_constant_p((__u32)(x)) ? \
 ___constant_swab32((x)) : \
 __fswab32((x)))
#  define __swab64(x) \
(__builtin_constant_p((__u64)(x)) ? \
 ___constant_swab64((x)) : \
 __fswab64((x)))


static __inline__ __u16 __fswab16(__u16 x)
{
	return (__extension__ __arch__swab16(x));
}
static __inline__ __u16 __swab16p(__u16 *x)
{
	return (__extension__ __arch__swab16p(x));
}
static __inline__ void __swab16s(__u16 *addr)
{
	(__extension__ ({__arch__swab16s(addr);}));
}

static __inline__ __u32 __fswab32(__u32 x)
{
	return (__extension__ __arch__swab32(x));
}
static __inline__ __u32 __swab32p(__u32 *x)
{
	return (__extension__ __arch__swab32p(x));
}
static __inline__ void __swab32s(__u32 *addr)
{
	(__extension__ ({__arch__swab32s(addr);}));
}

static __inline__ __u64 __fswab64(__u64 x)
{
#  ifdef __SWAB_64_THRU_32__
	__u32 h = x >> 32;
	__u32 l = x & ((1ULL<<32)-1);
	return (((__u64)__swab32(l)) << 32) | ((__u64)(__swab32(h)));
#  else
	return (__extension__ __arch__swab64(x));
#  endif
}
static __inline__ __u64 __swab64p(__u64 *x)
{
	return (__extension__ __arch__swab64p(x));
}
static __inline__ void __swab64s(__u64 *addr)
{
	(__extension__ ({__arch__swab64s(addr);}));
}

#ifdef XFS_NATIVE_HOST
#define cpu_to_be16(val)	((__force __be16)(__u16)(val))
#define cpu_to_be32(val)	((__force __be32)(__u32)(val))
#define cpu_to_be64(val)	((__force __be64)(__u64)(val))
#define be16_to_cpu(val)	((__force __u16)(__be16)(val))
#define be32_to_cpu(val)	((__force __u32)(__be32)(val))
#define be64_to_cpu(val)	((__force __u64)(__be64)(val))

#define cpu_to_le32(val)	((__force __be32)__swab32((__u32)(val)))
#define le32_to_cpu(val)	(__swab32((__force __u32)(__le32)(val)))

#define __constant_cpu_to_le32(val)	\
	((__force __le32)___constant_swab32((__u32)(val)))
#define __constant_cpu_to_be32(val)	\
	((__force __be32)(__u32)(val))
#else
#define cpu_to_be16(val)	((__force __be16)__swab16((__u16)(val)))
#define cpu_to_be32(val)	((__force __be32)__swab32((__u32)(val)))
#define cpu_to_be64(val)	((__force __be64)__swab64((__u64)(val)))
#define be16_to_cpu(val)	(__swab16((__force __u16)(__be16)(val)))
#define be32_to_cpu(val)	(__swab32((__force __u32)(__be32)(val)))
#define be64_to_cpu(val)	(__swab64((__force __u64)(__be64)(val)))

#define cpu_to_le32(val)	((__force __le32)(__u32)(val))
#define le32_to_cpu(val)	((__force __u32)(__le32)(val))

#define __constant_cpu_to_le32(val)	\
	((__force __le32)(__u32)(val))
#define __constant_cpu_to_be32(val)	\
	((__force __be32)___constant_swab32((__u32)(val)))
#endif

static inline void be16_add_cpu(__be16 *a, __s16 b)
{
	*a = cpu_to_be16(be16_to_cpu(*a) + b);
}

static inline void be32_add_cpu(__be32 *a, __s32 b)
{
	*a = cpu_to_be32(be32_to_cpu(*a) + b);
}

static inline void be64_add_cpu(__be64 *a, __s64 b)
{
	*a = cpu_to_be64(be64_to_cpu(*a) + b);
}

static inline __uint16_t get_unaligned_be16(void *p)
{
	__uint8_t *__p = p;
	return __p[0] << 8 | __p[1];
}

static inline __uint32_t get_unaligned_be32(void *p)
{
	__uint8_t *__p = p;
        return __p[0] << 24 | __p[1] << 16 | __p[2] << 8 | __p[3];
}

static inline __uint64_t get_unaligned_be64(void *p)
{
	return (__uint64_t)get_unaligned_be32(p) << 32 |
			   get_unaligned_be32(p + 4);
}

static inline void put_unaligned_be16(__uint16_t val, void *p)
{
	__uint8_t *__p = p;
	*__p++ = val >> 8;
	*__p++ = val;
}

static inline void put_unaligned_be32(__uint32_t val, void *p)
{
	__uint8_t *__p = p;
	put_unaligned_be16(val >> 16, __p);
	put_unaligned_be16(val, __p + 2);
}

static inline void put_unaligned_be64(__uint64_t val, void *p)
{
	put_unaligned_be32(val >> 32, p);
	put_unaligned_be32(val, p + 4);
}

#endif	/* __XFS_ARCH_H__ */
