#ifndef _HYPERSTONE_BYTEORDER_H_
#define _HYPERSTONE_BYTEORDER_H_

#include <asm/types.h>

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#include <linux/byteorder/big_endian.h>

#endif /* _HYPERSTONE_BYTEORDER_H_ */
