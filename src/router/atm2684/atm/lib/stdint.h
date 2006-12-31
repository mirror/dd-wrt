/* stdint.h - provides uintXX_t until glibc does */

#ifndef _STDINT_H

#include <features.h>

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1

#include "/usr/include/stdint.h"

#elif __GLIBC__ >= 2

/* Works for i386 and Alpha */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#else

#ifndef _LINUX_TYPES_H
#include <linux/types.h>
#endif

#endif

#ifndef _STDINT_H
#define _STDINT_H
#endif

#endif
