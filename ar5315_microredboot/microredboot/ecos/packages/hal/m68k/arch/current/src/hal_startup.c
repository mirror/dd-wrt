//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_startup.h>
#include <cyg/hal/hal_memmap.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>           // hal_diag_init
#include <cyg/infra/diag.h>             // diag_printf
#include <string.h>                     // memcpy, memset

externC void cyg_start(void);
externC void hw_vsr_reset(void);

#define CYG_HAL_RESET_DEBUG_ENABLE
#ifdef CYG_HAL_RESET_DEBUG_ENABLE
#define CYG_HAL_RESET_DEBUG diag_printf
#else
#define CYG_HAL_RESET_DEBUG()
#endif // CYG_HAL_RESET_DEBUG_ENABLE

/*****************************************************************************
hal_vsr_init -- Initialize the vector table

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
static void hal_vsr_init(void)
{

    /*   Initialize the HAL's vector table with the ROM vector table.       */

    memcpy((void*)cyg_hal_vsr_table, __romvec_start,
                (size_t)sizeof(cyg_hal_vsr_table));

}

/*****************************************************************************
hal_vsr_init -- Initialize the ISRs

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
static void hal_isr_init(void)
{
    int_t i;

    //   Initialize all ISR entries to default.

    for (i = 0; i < CYGNUM_HAL_ISR_COUNT; i++)
    {
        cyg_hal_interrupt_handlers[i] = (CYG_ADDRESS) &hal_default_isr;
        cyg_hal_interrupt_data[i] = (CYG_ADDRWORD)0;
        cyg_hal_interrupt_objects[i] = (CYG_ADDRESS)0;
    }
}

/*****************************************************************************
hal_init_ram_sections -- Initialize the RAM sections

     Initialize all RAM sections that  the C  code relies  on.  data,  bss,
sbss.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
static void hal_init_ram_sections(void)
{

    //   Initialize the RAM data section  from  the  ROM  image  of  the  data
    // section.

    memcpy(__ram_data_start, __rom_data_start, (size_t)__ram_data_size);

    //   Initialize the bss and sbss sections to zero.

    memset(__bss_start, 0, (size_t)__bss_size);
    memset(__sbss_start, 0, (size_t)__sbss_size);
}

/*****************************************************************************
cyg_hal_invoke_constructors -- Call static constructors

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

static void cyg_hal_invoke_constructors(void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    static pfunc *p = &__CTOR_END__[-1];

    cyg_hal_stop_constructors = 0;
    for (; p >= __CTOR_LIST__; p--) {
        (*p) ();
        if (cyg_hal_stop_constructors) {
            p--;
            break;
        }
    }
#else
    pfunc *p;

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
        (*p) ();
#endif
}

/*****************************************************************************
hal_reset -- Reset vector routine

     This routine must be  called with interrupts  disabled and will  never
return.

     Only the assembly reset vector routine should call this routine.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void hal_reset(void) CYGBLD_ATTRIB_NORET;
void hal_reset(void)
{
    const char * fname;

    fname = __FUNCTION__;

    //   Initialize the RAM sections that the rest of the C code requires.

    hal_init_ram_sections();

    //   It is now safe  to call  C functions  which may  rely on  initialized
    // data.

    //   Initialize the ISR and VSR tables.

    hal_isr_init();
    hal_vsr_init();

    //   Do any variant-specific reset initialization.

    var_reset();

    //   Initialize the diagnostics IO.

    HAL_DIAG_INIT();
    CYG_HAL_RESET_DEBUG("%s: RESET\r\n", fname);

    //   Call C++ constructors.

    CYG_HAL_RESET_DEBUG("%s: calling cyg_hal_invoke_constructors\r\n", fname);
    cyg_hal_invoke_constructors();

    //   It should be safe to enable interrupts now.

    CYG_HAL_RESET_DEBUG("%s: lowering interrupt mask\r\n", fname);
    HAL_ENABLE_INTERRUPTS();

    //   Call cyg_start.  This routine should not return.

    CYG_HAL_RESET_DEBUG("%s: calling cyg_start\r\n", fname);
    cyg_start();

    //   If we return, loop and print out a message.

    HAL_DIAG_INIT();
    for (;;)
    {
        CYG_HAL_RESET_DEBUG("%s: cyg_start returned!\r\n", fname);
    }
}

/*****************************************************************************
hal_hw_reset -- Simulate a hardware reset

     This routine will never return.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void hal_hw_reset(void)
{

    //   Give control to the reset vector handler.

    hw_vsr_reset();
}

