/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    tmichals
// Contributors: nickg, dmoseley
// Date:         2003-02-13
// Purpose:      HAL diagnostic output
// Description:  Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/hal_intr.h>

#include <cyg/hal/hal_io.h>

//-----------------------------------------------------------------------------
// Select which diag channels to use

//#define CYG_KERNEL_DIAG_LCD
#define CYG_KERNEL_DIAG_SERIAL

/*---------------------------------------------------------------------------*/

void hal_diag_led(int x)
{
//  HAL_WRITE_UINT32(HAL_DISPLAY_LEDBAR, x);
#if !defined(CYG_KERNEL_DIAG_LCD)
//  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIWORD, x);
#endif
}

externC void diag_write_string (const char*);

#ifdef CYG_KERNEL_DIAG_SERIAL
extern void cyg_hal_plf_comms_init(void);
extern void cyg_hal_plf_serial_putc(void*, cyg_uint8);
extern cyg_uint8 cyg_hal_plf_serial_getc(void*);
#endif

void hal_diag_init(void)
{
#if defined(CYGSEM_HAL_ROM_MONITOR) && !defined(CYG_KERNEL_DIAG_SERIAL)
  // It's handy to have the LCD initialized at reset when using it
  // for debugging output.
  // The serial port likely doesn't work yet.  Let's wait.
  diag_write_string ("eCos ROM   " __TIME__ "\n");
  diag_write_string (__DATE__ "\n");
#endif
	
  cyg_hal_plf_comms_init();
}

#if defined(CYG_KERNEL_DIAG_LCD)
static void hal_diag_clear_lcd(void)
{
  volatile int i = 0x20000;
  while (--i) ;

  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS0, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS1, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS2, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS3, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS4, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS5, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS6, ' ');
  HAL_WRITE_UINT32(HAL_DISPLAY_ASCIIPOS7, ' ');
}
#endif /* defined(CYG_KERNEL_DIAG_LCD) */

void hal_diag_write_char(char c)
{
#if defined(CYG_KERNEL_DIAG_LCD)
  static volatile CYG_WORD* reg = HAL_DISPLAY_ASCIIPOS0;
#endif

  unsigned long __state;

  HAL_DISABLE_INTERRUPTS(__state);

  if(c == '\n')
    {
#if defined(CYG_KERNEL_DIAG_LCD)
      reg = HAL_DISPLAY_ASCIIPOS0;
      hal_diag_clear_lcd();
#endif
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
#if defined(CYG_KERNEL_DIAG_LCD)
      if (reg == HAL_DISPLAY_ASCIIPOS0)
        hal_diag_clear_lcd();

      HAL_WRITE_UINT32(reg, c);

      // Advance to next LED position.
      if (reg == HAL_DISPLAY_ASCIIPOS0)
        reg = HAL_DISPLAY_ASCIIPOS1;
      else if (reg == HAL_DISPLAY_ASCIIPOS1)
        reg = HAL_DISPLAY_ASCIIPOS2;
      else if (reg == HAL_DISPLAY_ASCIIPOS2)
        reg = HAL_DISPLAY_ASCIIPOS3;
      else if (reg == HAL_DISPLAY_ASCIIPOS3)
        reg = HAL_DISPLAY_ASCIIPOS4;
      else if (reg == HAL_DISPLAY_ASCIIPOS4)
        reg = HAL_DISPLAY_ASCIIPOS5;
      else if (reg == HAL_DISPLAY_ASCIIPOS5)
        reg = HAL_DISPLAY_ASCIIPOS6;
      else if (reg == HAL_DISPLAY_ASCIIPOS6)
        reg = HAL_DISPLAY_ASCIIPOS7;
      else // reg == HAL_DISPLAY_ASCIIPOS7 or UNKNOWN
        reg = HAL_DISPLAY_ASCIIPOS0;
#endif
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
