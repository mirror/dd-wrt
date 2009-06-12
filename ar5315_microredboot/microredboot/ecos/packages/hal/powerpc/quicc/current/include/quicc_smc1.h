#ifndef CYGONCE_HAL_PPC_QUICC_SMC1_H
#define CYGONCE_HAL_PPC_QUICC_SMC1_H
//=============================================================================
//
//      quicc_smc1.h
//
//      PowerPC QUICC basic Serial IO using port SMC1/SCC1
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    Red Hat
// Contributors: hmt, gthomas
// Date:         1999-06-08
// Purpose:      Provide basic Serial IO for MPC8xx boards (like Motorola MBX)
// Description:  Serial IO for MPC8xx boards which connect their debug channel
//               to SMC1 or SCC1; or any QUICC user who wants to use SMC1/SCC1
// Usage:
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/quicc/ppc8xx.h>             // FIXME: bad, but need eppc_base


#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
// This one should only be used by old-stub compatibility code!
externC void cyg_hal_plf_serial_init_channel(void);
#endif

externC void cyg_hal_plf_serial_init(void);

#endif /* CYGONCE_HAL_PPC_QUICC_SMC1_H */
