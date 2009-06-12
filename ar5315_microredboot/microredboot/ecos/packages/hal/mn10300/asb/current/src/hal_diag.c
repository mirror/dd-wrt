//=============================================================================
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
// Author(s):   dmoseley (based on the original by jskov)
// Contributors:nickg, jskov, dmoseley
// Date:        2000-08-11
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)

#warning HAL_DIAG has only been verified using CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>

#warning GET HAL_DIAG STUFF WORKING

/*---------------------------------------------------------------------------*/
/* Select default diag channel to use                                        */

//#define CYG_KERNEL_DIAG_SERIAL
//#define CYG_KERNEL_DIAG_BUFFER
//#define CYG_KERNEL_DIAG_GDB

#if !defined(CYG_KERNEL_DIAG_SERIAL) && defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)
#  define CYG_KERNEL_DIAG_SERIAL
#  define CYG_KERNEL_DIAG_GDB
#else
#  define CYG_KERNEL_DIAG_SERIAL
#endif

/*---------------------------------------------------------------------------*/

externC void diag_write_string (const char*);

#if defined(CYG_KERNEL_DIAG_SERIAL)
extern void cyg_hal_plf_comms_init(void);
extern void cyg_hal_plf_serial_putc(void*, cyg_uint8);
extern cyg_uint8 cyg_hal_plf_serial_getc(void*);
#endif

#if defined(CYG_KERNEL_DIAG_BUFFER)
char hal_diag_buffer[10000];
int hal_diag_buffer_pos;
#endif

void hal_diag_init(void)
{
#if defined(CYG_KERNEL_DIAG_SERIAL)
  cyg_hal_plf_comms_init();
#endif

#if defined(CYG_KERNEL_DIAG_BUFFER)
    hal_diag_buffer_pos = 0;
#endif
}

#ifdef CYG_KERNEL_DIAG_GDB
static void gdb_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        while (1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

            {
                char c1;

                hal_diag_read_char_serial( &c1 );

                if( c1 == '+' ) break;

                
                if( cyg_hal_is_break( &c1, 1 ) )
                    cyg_hal_user_break( NULL );
            }
        }
        
        pos = 0;
    }
}
#endif // CYG_KERNEL_DIAG_GDB

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
#if defined(CYG_KERNEL_DIAG_BUFFER)
      hal_diag_buffer[hal_diag_buffer_pos++] = c;
      if (hal_diag_buffer_pos >= sizeof(hal_diag_buffer) )
          hal_diag_buffer_pos = 0;
#endif
#if defined(CYG_KERNEL_DIAG_GDB)
      gdb_diag_write_char(c);
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
#if defined(CYG_KERNEL_DIAG_BUFFER)
      hal_diag_buffer[hal_diag_buffer_pos++] = c;
      if (hal_diag_buffer_pos >= sizeof(hal_diag_buffer) )
          hal_diag_buffer_pos = 0;
#endif
#if defined(CYG_KERNEL_DIAG_GDB)
      gdb_diag_write_char(c);
#endif
    }

  HAL_RESTORE_INTERRUPTS(__state);
}

void hal_diag_read_char(char *c)
{
#if defined(CYG_KERNEL_DIAG_SERIAL)
    cyg_hal_plf_serial_getc(c);
#endif
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
