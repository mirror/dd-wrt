/* Changed by Lineo, Inc, May 2001 for FRIO */ 

#ifndef __FRIO_UNALIGNED_H
#define __FRIO_UNALIGNED_H

#include <linux/config.h>

/* The FRIO architecture follows Strict Data Alighment rules for accessing data,
   and an unaligned data access causes a software exception.  */

/* Use memmove here, so gcc does not insert a __builtin_memcpy. */

#define get_unaligned(ptr) \
  ({ __typeof__(*(ptr)) __tmp; memmove(&__tmp, (ptr), sizeof(*(ptr))); __tmp; })

#define put_unaligned(val, ptr)				\
  ({ __typeof__(*(ptr)) __tmp = (val);			\
     memmove((ptr), &__tmp, sizeof(*(ptr)));		\
     (void)0; })

#endif /* __FRIO_UNALIGNED_H */
