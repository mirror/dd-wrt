//==========================================================================
//
//      diag.c
//
//      Additional RedBoot commands to run board diags.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2000-10-10
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================
#include <redboot.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/hal_tables.h>
#include <cyg/hal/iq80321.h>


int pci_config_cycle = 0;	/* skip exception handling when performing pci config cycle */

static void do_hdwr_diag(int argc, char *argv[]);

RedBoot_cmd("diag", 
            "Run board diagnostics", 
            "",
            do_hdwr_diag
    );

void hdwr_diag (void);

void do_hdwr_diag(int arg, char *argv[])
{
    hal_virtual_comm_table_t* __chan;

    // Turn off interrupts on debug channel.
    // All others should already be disabled.
    __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    if (__chan)
	CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_DISABLE);

    hdwr_diag();
}




