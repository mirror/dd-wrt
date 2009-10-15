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
hal_ar7100_board_init()
{
    /* XXX - should be set based board configuration */
    *(volatile unsigned int *)0xb8050004 = 0x50C0;
    hal_delay_us(10);
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    hal_delay_us(10);
    *(volatile unsigned int *)0xb805001c = 0xee;
    hal_delay_us(10);
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
