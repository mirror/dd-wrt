/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/hal_intr.h>

#include <cyg/hal/hal_io.h>

//-----------------------------------------------------------------------------
// Select which diag channels to use

#define CYG_KERNEL_DIAG_SERIAL

/*---------------------------------------------------------------------------*/

#ifdef CYG_KERNEL_DIAG_SERIAL
extern void cyg_hal_plf_comms_init(void);
extern void cyg_hal_plf_serial_putc(void*, cyg_uint8);
extern cyg_uint8 cyg_hal_plf_serial_getc(void*);
#endif

void hal_diag_init(void)
{
#if defined(CYG_KERNEL_DIAG_SERIAL)
  cyg_hal_plf_comms_init();
#endif
}

void hal_diag_write_char(char c)
{
  unsigned long __state;

  HAL_DISABLE_INTERRUPTS(__state);

  if(c == '\n')
    {
#if defined (CYG_KERNEL_DIAG_SERIAL)
      cyg_hal_plf_serial_putc(NULL, '\r');
      cyg_hal_plf_serial_putc(NULL, '\n');
#endif
    }
  else if (c == '\r')
    {
      // Ignore '\r'
    }
  else
    {
#if defined(CYG_KERNEL_DIAG_SERIAL)
      cyg_hal_plf_serial_putc(NULL, c);
#endif
    }

  HAL_RESTORE_INTERRUPTS(__state);
}

void hal_diag_read_char(char* c)
{
  *c = cyg_hal_plf_serial_getc(NULL);
}

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
