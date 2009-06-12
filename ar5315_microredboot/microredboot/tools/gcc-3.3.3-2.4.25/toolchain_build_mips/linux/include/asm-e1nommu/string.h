#ifndef _HYPERSTONE_NOMMU_STRING_H
#define _HYPERSTONE_NOMMU_STRING_H

#define __HAVE_ARCH_MEMCHR
extern void * memchr(const void * s, int c, size_t count);

#define __HAVE_ARCH_MEMCMP
extern int memcmp(const void * s ,const void * d ,size_t );

#define __HAVE_ARCH_MEMCPY
extern void * memcpy(const void *d, const void *s, size_t count);

#define __HAVE_ARCH_MEMSET
extern void * memset(const void * s, int c, size_t count);

#define __HAVE_ARCH_STRCPY
extern char * strcpy(char * dest,const char *src);

#define __HAVE_ARCH_STRCMP
extern int strcmp(const char * cs,const char * ct);

#endif
