#include <stddef.h>

void *xmalloc(size_t sz);
void *xrealloc(void *p, size_t sz);

#define new(p) ((p) = xmalloc(sizeof(*(p))))


int kernel_version(void);
#define KRELEASE(maj,min,patch) ((maj) * 10000 + (min)*1000 + (patch))


int nstrcmp(const char *, const char *);

char *safe_strncpy(char *dst, const char *src, size_t size); 

