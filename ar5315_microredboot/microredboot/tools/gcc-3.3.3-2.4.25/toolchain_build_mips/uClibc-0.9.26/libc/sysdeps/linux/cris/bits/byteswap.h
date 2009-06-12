#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

#define ___swab16(x) \
({ \
	unsigned short __x = (x); \
	((unsigned short)( \
		(((unsigned short)(__x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(__x) & (unsigned short)0xff00U) >> 8) )); \
})

#define ___swab32(x) \
({ \
	unsigned long __x = (x); \
	((unsigned long)( \
		(((unsigned long)(__x) & (unsigned long)0x000000ffUL) << 24) | \
		(((unsigned long)(__x) & (unsigned long)0x0000ff00UL) <<  8) | \
		(((unsigned long)(__x) & (unsigned long)0x00ff0000UL) >>  8) | \
		(((unsigned long)(__x) & (unsigned long)0xff000000UL) >> 24) )); \
})

/* these are CRIS specific */

static inline unsigned short __fswab16(unsigned short x)
{
	__asm__ ("swapb %0" : "=r" (x) : "0" (x));
	
	return(x);
}

static inline unsigned long __fswab32(unsigned long x)
{
	__asm__ ("swapwb %0" : "=r" (x) : "0" (x));
	
	return(x);
}

#  define __bswap_16(x) \
(__builtin_constant_p((unsigned short)(x)) ? \
 ___swab16((x)) : \
 __fswab16((x)))

#  define __bswap_32(x) \
(__builtin_constant_p((unsigned long)(x)) ? \
 ___swab32((x)) : \
 __fswab32((x)))

#endif /* _BITS_BYTESWAP_H */
