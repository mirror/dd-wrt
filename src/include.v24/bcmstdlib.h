/*
 * prototypes for functions defined in bcmstdlib.c
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: bcmstdlib.h,v 1.1.1.1 2006/02/27 03:43:16 honor Exp $:
 */

/*
 * bcmstdlib.h file should be used only to construct an OSL or alone without any OSL
 * It should not be used with any orbitarary OSL's as there could be a conflict
 * with some of the routines defined here.
*/

#ifndef	_BCMSTDLIB_H
#define	_BCMSTDLIB_H
#include <stdarg.h>


#ifndef INT_MAX
#define INT_MAX 2147483647 /* from limits.h */
#endif

#define _BCM_U	0x01	/* upper */
#define _BCM_L	0x02	/* lower */
#define _BCM_D	0x04	/* digit */
#define _BCM_C	0x08	/* cntrl */
#define _BCM_P	0x10	/* punct */
#define _BCM_S	0x20	/* white space (space/lf/tab) */
#define _BCM_X	0x40	/* hex digit */
#define _BCM_SP	0x80	/* hard space (0x20) */

extern unsigned char ctype[];
#define ismask(x) ((int)ctype[(int)(unsigned char)(x)])

#define isalnum(c)	((ismask(c)&(_BCM_U|_BCM_L|_BCM_D)) != 0)
#define isalpha(c)	((ismask(c)&(_BCM_U|_BCM_L)) != 0)
#define iscntrl(c)	((ismask(c)&(_BCM_C)) != 0)
#define isdigit(c)	((ismask(c)&(_BCM_D)) != 0)
#define isgraph(c)	((ismask(c)&(_BCM_P|_BCM_U|_BCM_L|_BCM_D)) != 0)
#define islower(c)	((ismask(c)&(_BCM_L)) != 0)
#define isprint(c)	((ismask(c)&(_BCM_P|_BCM_U|_BCM_L|_BCM_D|_BCM_SP)) != 0)
#define ispunct(c)	((ismask(c)&(_BCM_P)) != 0)
#define isspace(c)	((ismask(c)&(_BCM_S)) != 0)
#define isupper(c)	((ismask(c)&(_BCM_U)) != 0)
#define isxdigit(c)	((ismask(c)&(_BCM_D|_BCM_X)) != 0)

#if !defined(NDIS) && !defined(_CFE_)

typedef int FILE;
#define stdout (FILE*)(1)
#define stderr (FILE*)(2)

/* i/o functions */
extern int fputc(int c, FILE *stream);
extern void putc(int c);
/* extern int putc(int c, FILE *stream); */
#define putchar(c) putc(c)
extern int fputs(const char *s, FILE *stream);
extern int puts(const char *s);
extern int getc(void);

/* string functions */
extern int printf(const char *fmt, ...);
extern int sprintf(char *buf, const char *fmt, ...);

extern char *index(const char *s, int c);

extern int toupper(int c);
extern int tolower(int c);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern size_t strlen(const char *s);
extern char *strchr(const char *str, int c);
extern char *strrchr(const char *str, int c);
extern char *strcat(char *d, const char *s);
extern char *strstr(const char *s, const char *find);
extern size_t strspn(const char *s1, const char *s2);
extern size_t strcspn(const char *s1, const char *s2);
extern ulong strtoul(char *cp, char **endp, uint base);
#define strtol(nptr, endptr, base) ((long)strtoul((nptr), (endptr), (base)))
extern int atoi(char *s);

/* mem functions */
extern void *memset(void *dest, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memchr(const void *s, int c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);

extern int vsprintf(char *buf, const char *fmt, va_list ap);

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))
extern unsigned long rand(void);

#endif /* !defined(vxworks) && !defined(NDIS) && !defined(_CFE_) */

extern int snprintf(char *str, size_t n, char const *fmt, ...);
extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

#endif 	/* _BCMSTDLIB_H */
