//==========================================================================
//
//      arm_integrator_flash.c
//
//      Flash programming for Intel FlashFile devices on ARM Integrator board
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-08-07
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

//--------------------------------------------------------------------------
// Device properties

// We use the eight Intel 28F320S3 parts on the Integrator board.
// These are arranged in a 2x4 grid, giving 32 bits wide by 32MB long.

#define CYGNUM_FLASH_INTERLEAVE (2)
#define CYGNUM_FLASH_SERIES     (4)
#define CYGNUM_FLASH_WIDTH      (16)
#define CYGNUM_FLASH_BASE       (0x24000000)

//--------------------------------------------------------------------------
// Platform specific extras

#define CYGHWR_FLASH_WRITE_ENABLE()								\
	{											\
		volatile cyg_uint32 *ebi_csr1 = (volatile cyg_uint32 *)INTEGRATOR_EBI_CSR1;	\
												\
		/* allow write access to EBI_CSR1 area (Flash) */				\
     		*ebi_csr1 |= INTEGRATOR_EBI_WRITE_ENABLE;					\
												\
		if (!(*ebi_csr1 & INTEGRATOR_EBI_WRITE_ENABLE)) {				\
		    *(volatile cyg_uint32 *)INTEGRATOR_EBI_LOCK = 0xA05F;			\
		    *ebi_csr1 |= INTEGRATOR_EBI_WRITE_ENABLE;					\
		    *(volatile cyg_uint32 *)INTEGRATOR_EBI_LOCK = 0;				\
		}										\
												\
		/* Enable Vpp and allow write access to Flash in system controller */		\
		*(volatile unsigned int *)INTEGRATOR_SC_CTRLS = FL_SC_CONTROL;			\
	}

#define CYGHWR_FLASH_WRITE_DISABLE()								\
	{											\
		volatile cyg_uint32 *ebi_csr1 = (volatile cyg_uint32 *)INTEGRATOR_EBI_CSR1;	\
												\
		/* disable write access to EBI_CSR1 area (Flash) */				\
		*ebi_csr1 &= ~INTEGRATOR_EBI_WRITE_ENABLE;					\
												\
		if (*ebi_csr1 & INTEGRATOR_EBI_WRITE_ENABLE) {					\
		    *(volatile cyg_uint32 *)INTEGRATOR_EBI_LOCK = 0xA05F;			\
		    *ebi_csr1 &= ~INTEGRATOR_EBI_WRITE_ENABLE;					\
		    *(volatile cyg_uint32 *)INTEGRATOR_EBI_LOCK = 1;				\
		}										\
												\
		/* Disable Vpp and disable write access to Flash in system controller */	\
		*(volatile unsigned int *)INTEGRATOR_SC_CTRLS = 0;				\
	}

//--------------------------------------------------------------------------
// Now include the driver code.
#include "cyg/io/flash_28fxxx.inl"

// ------------------------------------------------------------------------
// EOF arm_integrator_flash.c
