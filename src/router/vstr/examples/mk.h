#ifndef MK_H
#define MK_H 1

# define MK(sz)                                 \
    malloc_check_malloc(sz, __FILE__, __LINE__)
# define M0(num, sz)                                    \
    malloc_check_calloc(num, sz, __FILE__, __LINE__)
# define MV(ptr, tmp, sz)                                               \
    (((tmp) = malloc_check_realloc(ptr, sz, __FILE__, __LINE__)) &&     \
     ((ptr) = (tmp)))
# define F(ptr)                                 \
    malloc_check_free(ptr)

#ifdef __GNUC__
#define MALLOC_CHECK__ATTR_USED() __attribute__((used))
#endif

#include "../include/malloc-check.h"

#endif
