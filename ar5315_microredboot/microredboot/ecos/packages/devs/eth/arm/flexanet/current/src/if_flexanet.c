//==========================================================================
//
//      devs/eth/arm/flexanet/if_flexanet.c
//
//      Ethernet device driver for Flexanet using SMSC LAN91C96
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Jordi Colomer <jco@ict.es>
// Contributors: jco, 
// Date:         2001-07-01
// Purpose:      
// Description:  hardware driver for Flexanet/SMSC LAN91CXX ethernet
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_arm_flexanet.h>
#include <pkgconf/io_eth_drivers.h>

#if defined(CYGPKG_REDBOOT)
#include <pkgconf/redboot.h>
#endif

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>


// ESA (Ethernet Station Address), when constant
#ifndef CYGSEM_DEVS_ETH_ARM_FLEXANET_REDBOOT_ESA
static unsigned char static_esa[] = CYGDAT_DEVS_ETH_ARM_FLEXANET_ESA;
#endif



