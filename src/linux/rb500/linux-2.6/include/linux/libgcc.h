#ifndef __LINUX_LIBGCC_H
#define __LINUX_LIBGCC_H

#include <asm/byteorder.h>
#include <asm/libgcc.h>

typedef long long DWtype;
typedef int Wtype;
typedef unsigned int UWtype;
typedef int word_type __attribute__ ((mode (__word__)));

#define BITS_PER_UNIT 8

#ifdef __BIG_ENDIAN
struct DWstruct {
       Wtype high, low;
};
#elif defined(__LITTLE_ENDIAN)
struct DWstruct {
       Wtype low, high;
};
#else
#error I feel sick.
#endif

typedef union
{
       struct DWstruct s;
       DWtype ll;
} DWunion;

#endif /* __LINUX_LIBGCC_H */
