//==========================================================================
//
//      board_misc.c
//
//      Board specific miscellaneous functions
//
//==========================================================================

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>
#include <cyg/hal/ar7240_soc.h>



typedef cyg_uint32 u32;
typedef cyg_uint32 uint32_t;
typedef cyg_uint16 u16;
typedef cyg_uint16 uint16_t;
typedef cyg_uint8  u8;
typedef cyg_uint8  uint8_t;


#if defined(CYGPKG_IO_PCI)
void cyg_hal_plf_pci_init(void);
#endif /* defined(CYGPKG_IO_PCI) */

void
hal_board_init(void)
{

}


/* End of plf_misc.c                                                      */
