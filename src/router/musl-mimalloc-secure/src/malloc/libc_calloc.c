#ifndef LIBC_CALLOC_EXTERNAL

#define calloc __libc_calloc
#define malloc __libc_malloc

#include "calloc.c"

#endif
