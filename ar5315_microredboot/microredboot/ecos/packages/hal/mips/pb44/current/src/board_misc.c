//==========================================================================
//
//      board_misc.c
//
//      Board specific miscellaneous functions
//
//==========================================================================

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>

#include <cyg/hal/ar7100_soc.h>

typedef cyg_uint32 u32;
typedef cyg_uint16 u16;
typedef cyg_uint8  u8;

void
hal_ar7100_board_init(void)
{
#ifdef CYGNUM_CONFIG_SEC_PLL
  ar7100_reg_wr(AR7100_CONFIG_SEC_PLL, CYGNUM_CONFIG_SEC_PLL);
#else
#error CYGNUM_CONFIG_SEC_PLL is not defined
#endif
  hal_delay_us(10);
  
#ifdef CYGNUM_CONFIG_ETH_EXT_CLOCK
  ar7100_reg_wr(AR7100_CONFIG_ETH_EXT_CLOCK, CYGNUM_CONFIG_ETH_EXT_CLOCK);
#else
#error CYGNUM_ETH_EXT_CLOCK is not defined
#endif
    hal_delay_us(10);
    
#ifdef CYGNUM_CONFIG_PCI_CLOCK
    ar7100_reg_wr(AR7100_CONFIG_PCI_CLOCK, CYGNUM_CONFIG_PCI_CLOCK);
#else
#error CYGNUM_CONFIG_PCI_CLOCK
#endif
    hal_delay_us(10);
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
