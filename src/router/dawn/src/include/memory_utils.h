#ifndef DAWN_UTIL_MEMORY
#define DAWN_UTIL_MEMORY

#include <memory.h>

enum dawn_memop
{
    DAWN_MALLOC,
    DAWN_CALLOC,
    DAWN_REALLOC,
    DAWN_MEMREG,
    DAWN_MEMUNREG,
    DAWN_FREE
};

#define DAWN_MEMORY_AUDITING

#ifdef DAWN_MEMORY_AUDITING
#define dawn_malloc(size) dawn_memory_alloc(DAWN_MALLOC, __FILE__, __LINE__, 1, size, NULL)
#define dawn_calloc(nmemb, size) dawn_memory_alloc(DAWN_CALLOC, __FILE__, __LINE__, nmemb, size, NULL)
#define dawn_realloc(ptr, size) dawn_memory_alloc(DAWN_REALLOC, __FILE__, __LINE__, 1, size, ptr)
#define dawn_free(p) dawn_memory_free(DAWN_FREE, __FILE__, __LINE__, p)

#define dawn_regmem(p) dawn_memory_register(DAWN_MEMREG, __FILE__, __LINE__, 0, p)
#define dawn_unregmem(p) dawn_memory_unregister(DAWN_MEMUNREG, __FILE__, __LINE__, p)
#else
#define dawn_malloc(size) malloc(size)
#define dawn_calloc(nmemb, size) calloc(nmemb, size)
#define dawn_realloc(ptr, size) realloc(ptr, size)
#define dawn_free(p) free(p)

#define dawn_regmem(p)
#define dawn_unregmem(p)
#endif

void* dawn_memory_alloc(enum dawn_memop type, char* file, int line, size_t nmemb, size_t size, void *ptr);

void* dawn_memory_register(enum dawn_memop type, char* file, int line, size_t size, void *ptr);

void dawn_memory_free(enum dawn_memop type, char* file, int line, void* ptr);

void dawn_memory_unregister(enum dawn_memop type, char* file, int line, void* ptr);

void dawn_memory_audit();
#endif
