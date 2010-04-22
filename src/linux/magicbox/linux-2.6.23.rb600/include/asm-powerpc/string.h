#ifndef _ASM_POWERPC_STRING_H
#define _ASM_POWERPC_STRING_H

#ifdef __KERNEL__

#define __HAVE_ARCH_STRCPY
#define __HAVE_ARCH_STRNCPY
#define __HAVE_ARCH_STRLEN
#define __HAVE_ARCH_STRCMP
#define __HAVE_ARCH_STRCAT
#define __HAVE_ARCH_MEMCHR

extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, __kernel_size_t);
extern __kernel_size_t strlen(const char *);
extern int strcmp(const char *,const char *);
extern char * strcat(char *, const char *);
extern void * memchr(const void *,int,__kernel_size_t);

#define __HAVE_ARCH_MEMSET
extern void *memset(void *__s, int __c, size_t __count);
#define memset(__s, __c, len)					\
({								\
	size_t __len = (len);					\
	void *__ret;						\
	if (__builtin_constant_p(len) && __len >= 64)		\
		__ret = memset((__s), (__c), __len);		\
	else							\
		__ret = __builtin_memset((__s), (__c), __len);	\
	__ret;							\
})

#define __HAVE_ARCH_MEMCPY
extern void *memcpy(void *__to, __const__ void *__from, size_t __n);
#define memcpy(dst, src, len)					\
({								\
	size_t __len = (len);					\
	void *__ret;						\
	if (__builtin_constant_p(len) && __len >= 64)		\
		__ret = memcpy((dst), (src), __len);		\
	else							\
		__ret = __builtin_memcpy((dst), (src), __len);	\
	__ret;							\
})

#define __HAVE_ARCH_MEMMOVE
extern void *memmove(void *__dest, __const__ void *__src, size_t __n);
#define memmove(dst, src, len)					\
({								\
	size_t __len = (len);					\
	void *__ret;						\
	if (__builtin_constant_p(len) && __len >= 64)		\
		__ret = memmove((dst), (src), __len);		\
	else							\
		__ret = __builtin_memmove((dst), (src), __len);	\
	__ret;							\
})

#define __HAVE_ARCH_MEMCMP
#define memcmp(src1, src2, len) __builtin_memcmp((src1), (src2), (len))

#endif /* __KERNEL__ */

#endif	/* _ASM_POWERPC_STRING_H */
