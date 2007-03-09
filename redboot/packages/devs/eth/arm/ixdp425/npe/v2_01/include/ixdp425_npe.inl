//==========================================================================
//
//      ixdp425_npe.inl
//
//      IXDP425 ethernet I/O definitions.
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
// Author(s):   msalter
// Contributors:msalter
// Date:        2003-03-20
// Purpose:     ixdp425 NPE ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

#ifdef CYGPKG_DEVS_ETH_ARM_IXDP425_NPE_ETH0
#define CYGSEM_INTEL_NPE_USE_ETH0
#define CYGNUM_ETH0_ETH_ID    IX_ETH_PORT_1
#define CYGNUM_ETH0_PHY_NO    0
#define CYGDAT_NPE_ETH0_NAME  CYGDAT_DEVS_ETH_ARM_IXDP425_NPE_ETH0_NAME
#endif

#ifdef CYGPKG_DEVS_ETH_ARM_IXDP425_NPE_ETH1
#define CYGSEM_INTEL_NPE_USE_ETH1
#define CYGNUM_ETH1_ETH_ID    IX_ETH_PORT_2
#define CYGNUM_ETH1_PHY_NO    1
#define CYGDAT_NPE_ETH1_NAME  CYGDAT_DEVS_ETH_ARM_IXDP425_NPE_ETH1_NAME
#endif

#define CYGDAT_ETH0_DEFAULT_ESA {0x00, 0x03, 0x47, 0xdf, 0x32, 0xa8}
#define CYGDAT_ETH1_DEFAULT_ESA {0x00, 0x03, 0x47, 0xdf, 0x32, 0xaa}

#ifndef CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA
#ifdef CYGSEM_DEVS_ETH_INTEL_NPE_PLATFORM_EEPROM
extern int cyghal_get_npe_esa(int, cyg_uint8 *);

#define CYGHAL_GET_NPE_ESA(_ethid,_addr,_res) \
 (_res) = cyghal_get_npe_esa((_ethid), (_addr))

#endif // CYGSEM_DEVS_ETH_INTEL_NPE_PLATFORM_EEPROM
#endif // CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA
