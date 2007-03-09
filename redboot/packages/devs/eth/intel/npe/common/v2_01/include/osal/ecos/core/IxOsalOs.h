#ifndef IxOsalOs_H
#define IxOsalOs_H

#include <pkgconf/system.h>
#include CYGBLD_HAL_VARIANT_H

#ifndef IX_OSAL_CACHED
#error "Uncached memory not supported in linux environment"
#endif

#include <cyg/hal/hal_cache.h>

static inline unsigned long __v2p(unsigned long v)
{
    if (v < 0x40000000)
	return (v & 0xfffffff);
}

#define IX_OSAL_OS_MMU_VIRT_TO_PHYS(addr)        __v2p(addr)
#define IX_OSAL_OS_MMU_PHYS_TO_VIRT(addr)        (addr)
#define IX_OSAL_OS_CACHE_INVALIDATE(addr, size)  HAL_DCACHE_INVALIDATE((addr), (size))
#define IX_OSAL_OS_CACHE_FLUSH(addr, size)       HAL_DCACHE_FLUSH((addr), (size))

#define printf	diag_printf

#if defined(CYGHWR_HAL_ARM_XSCALE_CPU_IXP42x)
#define __ixp42X
#elif defined(CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x)
#define __ixp46X
#endif

#endif // IxOsalOs_H

