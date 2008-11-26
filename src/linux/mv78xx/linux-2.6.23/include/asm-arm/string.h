#ifndef __ASM_ARM_STRING_H
#define __ASM_ARM_STRING_H

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */

#define __HAVE_ARCH_STRRCHR
extern char * strrchr(const char * s, int c);

#define __HAVE_ARCH_STRCHR
extern char * strchr(const char * s, int c);

#define __HAVE_ARCH_MEMCPY
extern void * memcpy(void *, const void *, __kernel_size_t);

#define __HAVE_ARCH_MEMMOVE
extern void * memmove(void *, const void *, __kernel_size_t);

#define __HAVE_ARCH_MEMCHR
extern void * memchr(const void *, int, __kernel_size_t);

#define __HAVE_ARCH_MEMZERO
#define __HAVE_ARCH_MEMSET
extern void * memset(void *, int, __kernel_size_t);

extern void __memzero(void *ptr, __kernel_size_t n);

#if defined (CONFIG_MV_XORMEMZERO)

extern void xor_memzero(void *, __kernel_size_t);
extern void asm_memzero(void *, __kernel_size_t);

static inline void mv_acc_memzero(void * dest, size_t n)
{
	if (n < CONFIG_MV_XOR_MEMZERO_THRESHOLD) 
		return asm_memzero(dest, n);
	return xor_memzero(dest, n);
}

#elif defined (CONFIG_MV_IDMA_MEMZERO)

extern void dma_memzero(void *, __kernel_size_t);
extern void asm_memzero(void *, __kernel_size_t);

static inline void mv_acc_memzero(void * dest, size_t n)
{
	if (n < CONFIG_MV_IDMA_MEMZERO_THRESHOLD) 
		return asm_memzero(dest, n);
	return dma_memzero(dest, n);
}

#else

#define mv_acc_memzero(a,b)      __memzero(a,b)

#endif


#define memset(p,v,n)							\
	({								\
	 	void *__p = (p); size_t __n = n;			\
		if ((__n) != 0) {					\
			if (__builtin_constant_p((v)) && (v) == 0)	\
				mv_acc_memzero((__p),(__n));			\
			else						\
				memset((__p),(v),(__n));		\
		}							\
		(__p);							\
	})

#define memzero(p,n) 							\
	({ 								\
	 	void *__p = (p); size_t __n = n;			\
	 	if ((__n) != 0) 					\
	 		mv_acc_memzero((__p),(__n)); 			\
	 	(__p); 							\
	 })

#if defined (CONFIG_MV_XORMEMCOPY)

extern void * xor_memcpy(void *, const void*, __kernel_size_t);
extern void * asm_memcpy(void *, const void *, __kernel_size_t);
extern void * asm_memmove(void *, const void *, __kernel_size_t);

static inline void * mv_xor_memcpy(void * dest, const void * src,
			    size_t n)
{
	if (n < CONFIG_MV_XOR_MEMCOPY_THRESHOLD) 
		return asm_memcpy(dest, src, n);
	return xor_memcpy(dest, src, n);
}
static inline void * mv_xor_memmove(void * dest, const void * src,
			    size_t n)
{
	if (n < CONFIG_MV_XOR_MEMCOPY_THRESHOLD) 
		return asm_memmove(dest, src, n);
	return xor_memcpy(dest, src, n);
}

#define memcpy(a, b, c) mv_xor_memcpy(a, b, c)
// Still need to support memmove and check overlapping regions
#define memmove(a, b, c) mv_xor_memmove(a, b, c)

#elif defined (CONFIG_MV_IDMA_MEMCOPY)

extern void * dma_memcpy(void *, const void*, __kernel_size_t);
extern void * asm_memcpy(void *, const void *, __kernel_size_t);
extern void * asm_memmove(void *, const void *, __kernel_size_t);

static inline void * mv_dma_memcpy(void * dest, const void * src,
			    size_t n)
{
	if (n < CONFIG_MV_IDMA_MEMCOPY_THRESHOLD) 
		return asm_memcpy(dest, src, n);
	return dma_memcpy(dest, src, n);
}
static inline void * mv_dma_memmove(void * dest, const void * src,
			    size_t n)
{
	if (n < CONFIG_MV_IDMA_MEMCOPY_THRESHOLD) 
		return asm_memmove(dest, src, n);
	return dma_memcpy(dest, src, n);
}

#define memcpy(a, b, c) mv_dma_memcpy(a, b, c)
// Still need to support memmove and check overlapping regions
#define memmove(a, b, c) mv_dma_memmove(a, b, c)

#endif





#endif
