#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

#include <string.h>
#include <cyg/error/codes.h>
//while EFAULT should have no meaning in eCos since there are no address spaces
//it is defined here because set/getsockopt in lwIP use it.
#define EFAULT 14

//found no better place for this prototype
int lwip_init(void);

//#define LWIP_PROVIDE_ERRNO
#include <cyg/infra/cyg_type.h>
#if (CYG_BYTEORDER == CYG_LSBFIRST)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif


typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   long    u32_t;
typedef signed     long    s32_t;
typedef unsigned long	mem_ptr_t;


#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_ass.h>
#define LWIP_PLATFORM_DIAG(x)	{diag_printf x;}
#define LWIP_PLATFORM_ASSERT(x) {CYG_FAIL(x);}

#define SYS_ARCH_DECL_PROTECT(x)
#define SYS_ARCH_PROTECT(x)
#define SYS_ARCH_UNPROTECT(x)

#endif /* __ARCH_CC_H__ */
