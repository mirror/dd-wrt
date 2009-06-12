//==========================================================================
//
//      sysinfo.c
//
//      Interface for getting system information.
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
// Purpose:      Interface for getting system information.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <stdlib.h>
#include <bsp/bsp.h>
#include "bsp_if.h"


/*
 * In order to construct the _bsp_memory_list, some board specific code
 * may have to size RAM regions. To do this easily and reliably, the code
 * needs to run from ROM before .bss and .data sections are initialized.
 * This leads to the problem of where to store the results of the memory
 * sizing tests. In this case, the _bsp_init_stack routine which sizes
 * memory and sets up the stack will place the board-specific information
 * on the stack and return with the stack pointer pointing to a pointer to
 * the information. That is, addr_of_info = *(void **)sp. The architecture
 * specific code will then copy that pointer to the _bsp_ram_info_ptr variable
 * after initializing the .data and .bss sections.
 */
void *_bsp_ram_info_ptr;

/*
 *  Name of CPU and board. Should be overridden by arch/board specific
 *  code.
 */
struct bsp_platform_info _bsp_platform_info = {
    "Unknown",  /* cpu name */
    "Unknown",  /* board name */
    ""          /* extra info */
};


/*
 *  Information about possible data cache. Should be overridden by
 *  by arch/board specific code.
 */
struct bsp_cachesize_info _bsp_dcache_info = {
    0, 0, 0
};


/*
 *  Information about possible instruction cache. Should be overridden by
 *  by arch/board specific code.
 */
struct bsp_cachesize_info _bsp_icache_info = {
    0, 0, 0
};


/*
 *  Information about possible secondary cache. Should be overridden by
 *  by arch/board specific code.
 */
struct bsp_cachesize_info _bsp_scache_info = {
    0, 0, 0
};



int
_bsp_sysinfo(enum bsp_info_id id, va_list ap)
{
    int  index, rval = 0;
    void *p;

    switch (id) {
      case BSP_INFO_PLATFORM:
	p = va_arg(ap, void *);
	*(struct bsp_platform_info *)p = _bsp_platform_info;
	break;

      case BSP_INFO_DCACHE:
	p = va_arg(ap, void *);
	*(struct bsp_cachesize_info *)p = _bsp_dcache_info;
	break;

      case BSP_INFO_ICACHE:
	p = va_arg(ap, void *);
	*(struct bsp_cachesize_info *)p = _bsp_icache_info;
	break;

      case BSP_INFO_SCACHE:
	p = va_arg(ap, void *);
	*(struct bsp_cachesize_info *)p = _bsp_scache_info;
	break;

      case BSP_INFO_MEMORY:
	index = va_arg(ap, int);
	p = va_arg(ap, void *);

	if (index >= 0 && index < _bsp_num_mem_regions)
	    *(struct bsp_mem_info *)p = _bsp_memory_list[index];
	else
	    rval = -1;
	break;

      case BSP_INFO_COMM:
	index = va_arg(ap, int);
	p = va_arg(ap, void *);

	if (index >= 0 && index < _bsp_num_comms)
	    *(struct bsp_comm_info *)p = _bsp_comm_list[index].info;
	else if (index == _bsp_num_comms && _bsp_net_channel != NULL)
	    *(struct bsp_comm_info *)p = _bsp_net_channel->info;
	else
	    rval = -1;
	break;

      default:
	rval =  -1;
    }

    return rval;
}



