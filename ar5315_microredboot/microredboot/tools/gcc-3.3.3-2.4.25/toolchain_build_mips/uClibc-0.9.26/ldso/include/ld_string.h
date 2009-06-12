#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

extern void *_dl_malloc(int size);
extern char *_dl_getenv(const char *symbol, char **envp);
extern void _dl_unsetenv(const char *symbol, char **envp);
extern char *_dl_strdup(const char *string);
extern void _dl_dprintf(int, const char *, ...);


static size_t _dl_strlen(const char * str);
static char *_dl_strcat(char *dst, const char *src);
static char * _dl_strcpy(char * dst,const char *src);
static int _dl_strcmp(const char * s1,const char * s2);
static int _dl_strncmp(const char * s1,const char * s2,size_t len);
static char * _dl_strchr(const char * str,int c);
static char *_dl_strrchr(const char *str, int c);
static char *_dl_strstr(const char *s1, const char *s2);
static void * _dl_memcpy(void * dst, const void * src, size_t len);
static int _dl_memcmp(const void * s1,const void * s2,size_t len);
static void *_dl_memset(void * str,int c,size_t len);
static char *_dl_get_last_path_component(char *path);
static char *_dl_simple_ltoa(char * local, unsigned long i);
static char *_dl_simple_ltoahex(char * local, unsigned long i);

#ifndef NULL
#define NULL ((void *) 0)
#endif

static inline size_t _dl_strlen(const char * str)
{
	register char *ptr = (char *) str;

	while (*ptr)
		ptr++;
	return (ptr - str);
}

static inline char *_dl_strcat(char *dst, const char *src)
{
	register char *ptr = dst;

	while (*ptr)
		ptr++;

	while (*src)
		*ptr++ = *src++;
	*ptr = '\0';

	return dst;
}

static inline char * _dl_strcpy(char * dst,const char *src)
{
	register char *ptr = dst;

	while (*src)
		*dst++ = *src++;
	*dst = '\0';

	return ptr;
}
 
static inline int _dl_strcmp(const char * s1,const char * s2)
{
	register unsigned char c1, c2;

	do {
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0')
			return c1 - c2;
	}
	while (c1 == c2);

	return c1 - c2;
}

static inline int _dl_strncmp(const char * s1,const char * s2,size_t len)
{
	register unsigned char c1 = '\0';
	register unsigned char c2 = '\0';

	while (len > 0) {
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		len--;
	}

	return c1 - c2;
}

static inline char * _dl_strchr(const char * str,int c)
{
	register char ch;

	do {
		if ((ch = *str) == c)
			return (char *) str;
		str++;
	}
	while (ch);

	return 0;
}

static inline char *_dl_strrchr(const char *str, int c)
{
    register char *prev = 0;
    register char *ptr = (char *) str;

    while (*ptr != '\0') {
	if (*ptr == c)
	    prev = ptr;
	ptr++;  
    }   
    if (c == '\0')
	return(ptr);
    return(prev);
}


static inline char *_dl_strstr(const char *s1, const char *s2)
{
    register const char *s = s1;
    register const char *p = s2;
    
    do {
        if (!*p) {
	    return (char *) s1;;
	}
	if (*p == *s) {
	    ++p;
	    ++s;
	} else {
	    p = s2;
	    if (!*s) {
	      return NULL;
	    }
	    s = ++s1;
	}
    } while (1);
}

static inline void * _dl_memcpy(void * dst, const void * src, size_t len)
{
	register char *a = dst;
	register const char *b = src;

	while (len--)
		*a++ = *b++;

	return dst;
}


static inline int _dl_memcmp(const void * s1,const void * s2,size_t len)
{
	unsigned char *c1 = (unsigned char *)s1;
	unsigned char *c2 = (unsigned char *)s2;

	while (len--) {
		if (*c1 != *c2) 
			return *c1 - *c2;
		c1++;
		c2++;
	}
	return 0;
}

static inline void * _dl_memset(void * str,int c,size_t len)
{
	register char *a = str;

	while (len--)
		*a++ = c;

	return str;
}

static inline char *_dl_get_last_path_component(char *path)
{
	char *s;
	register char *ptr = path;
	register char *prev = 0;

	while (*ptr)
		ptr++;
	s = ptr - 1;

	/* strip trailing slashes */
	while (s != path && *s == '/') {
		*s-- = '\0';
	}

	/* find last component */
	ptr = path;
	while (*ptr != '\0') {
	    if (*ptr == '/')
		prev = ptr;
	    ptr++;  
	}   
	s = prev;

	if (s == NULL || s[1] == '\0')
		return path;
	else
		return s+1;
}

/* Early on, we can't call printf, so use this to print out
 * numbers using the SEND_STDERR() macro */
static inline char *_dl_simple_ltoa(char * local, unsigned long i)
{
	/* 21 digits plus null terminator, good for 64-bit or smaller ints */
	char *p = &local[22];
	*p-- = '\0';
	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);
	return p + 1;
}

static inline char *_dl_simple_ltoahex(char * local, unsigned long i)
{
	/* 21 digits plus null terminator, good for 64-bit or smaller ints */
	char *p = &local[22];
	*p-- = '\0';
	do {
		char temp = i % 0x10;
		if (temp <= 0x09)
		    *p-- = '0' + temp;
		else
		    *p-- = 'a' - 0x0a + temp;
		i /= 0x10;
	} while (i > 0);
	*p-- = 'x';
	*p-- = '0';
	return p + 1;
}


#if defined(mc68000) || defined(__arm__) || defined(__mips__) || defined(__sh__) ||  defined(__powerpc__)
/* On some arches constant strings are referenced through the GOT. */
/* XXX Requires load_addr to be defined. */
#define SEND_STDERR(X)				\
  { const char *__s = (X);			\
    if (__s < (const char *) load_addr) __s += load_addr;	\
    _dl_write (2, __s, _dl_strlen (__s));	\
  }
#else
#define SEND_STDERR(X) _dl_write(2, X, _dl_strlen(X));
#endif

#define SEND_ADDRESS_STDERR(X, add_a_newline) { \
    char tmp[22], *tmp1; \
    _dl_memset(tmp, 0, sizeof(tmp)); \
    tmp1=_dl_simple_ltoahex( tmp, (unsigned long)(X)); \
    _dl_write(2, tmp1, _dl_strlen(tmp1)); \
    if (add_a_newline) { \
	tmp[0]='\n'; \
	_dl_write(2, tmp, 1); \
    } \
};

#define SEND_NUMBER_STDERR(X, add_a_newline) { \
    char tmp[22], *tmp1; \
    _dl_memset(tmp, 0, sizeof(tmp)); \
    tmp1=_dl_simple_ltoa( tmp, (unsigned long)(X)); \
    _dl_write(2, tmp1, _dl_strlen(tmp1)); \
    if (add_a_newline) { \
	tmp[0]='\n'; \
	_dl_write(2, tmp, 1); \
    } \
};


#endif
