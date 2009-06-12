//==========================================================================
//
//      debug-io.c
//
//      BSP Debug Channel Interfaces.
//
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      BSP Debug Channel Interfaces.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <stdlib.h>
#include <bsp/bsp.h>
#include "bsp_if.h"

static unsigned char __debug_ungetc;

extern int stub_is_active;
extern int __output_gdb_string (const char *str, int string_len);
void
bsp_debug_write(const char *p, int len)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;
    if (stub_is_active) {
        // We are running in 'GDB' mode
        __output_gdb_string(p, len);
    } else {
        com->__write(com->ch_data, p, len);
    }
}


int
bsp_debug_read(char *p, int len)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    if (len <= 0)
	return 0;

    if (__debug_ungetc) {
	*p = __debug_ungetc;
	__debug_ungetc = 0;
	return 1;
    }

    return com->__read(com->ch_data, p, len);
}


void
bsp_debug_putc(char ch)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    com->__putc(com->ch_data, ch);
}

int
bsp_debug_getc(void)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;
    int  ch;

    if (__debug_ungetc) {
	ch = __debug_ungetc;
	__debug_ungetc = 0;
    } else
	ch = com->__getc(com->ch_data);

    return ch;
}


void
bsp_debug_ungetc(char ch)
{
    __debug_ungetc = (unsigned char)ch;
}


int
bsp_debug_irq_disable(void)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    return com->__control(com->ch_data, COMMCTL_IRQ_DISABLE);
}


void
bsp_debug_irq_enable(void)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    com->__control(com->ch_data, COMMCTL_IRQ_ENABLE);
}


