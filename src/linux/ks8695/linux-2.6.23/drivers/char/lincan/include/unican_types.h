#ifndef _UNICAN_TYPES_H
#define _UNICAN_TYPES_H

#include <linux/types.h>

#ifndef U8
#define U8  __u8
#endif
#ifndef U16
#define U16 __u16
#endif
#ifndef U32
#define U32 __u32
#endif

#define BOOLEAN1 __u8

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

#ifdef CONFIG_PPC
#define unican_readw(addr) in_be16((volatile u16 *)(addr))
#define unican_writew(b,addr) out_be16((volatile u16 *)(addr),(b))
/* #define unican_readw(addr) (*(volatile __u16 *)(addr)) */
/* #define unican_writew(v, addr) (*(volatile __u16 *)(addr) = (v)) */
#else  /* CONFIG_PPC */
#define unican_readw(addr) readw(addr)
#define unican_writew(v, addr) writew(v, addr)
#endif /* CONFIG_PPC */

#endif /*_UNICAN_TYPES_H*/
