//==========================================================================
//
//      ixdp465_npe.inl
//
//      IXDP465 ethernet I/O definitions.
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
// Purpose:     ixdp465 NPE ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

#ifdef CYGPKG_DEVS_ETH_ARM_IXDP465_NPE_ETH0
#define CYGSEM_INTEL_NPE_USE_ETH0
#define CYGNUM_ETH0_ETH_ID    IX_ETH_PORT_1
#define CYGNUM_ETH0_PHY_NO    NPE_PHY_UNKNOWN
#define CYGDAT_NPE_ETH0_NAME  CYGDAT_DEVS_ETH_ARM_IXDP465_NPE_ETH0_NAME
#endif

#ifdef CYGPKG_DEVS_ETH_ARM_IXDP465_NPE_ETH1
#define CYGSEM_INTEL_NPE_USE_ETH1
#define CYGNUM_ETH1_ETH_ID    IX_ETH_PORT_2
#define CYGNUM_ETH1_PHY_NO    NPE_PHY_UNKNOWN
#define CYGDAT_NPE_ETH1_NAME  CYGDAT_DEVS_ETH_ARM_IXDP465_NPE_ETH1_NAME
#endif

#ifdef CYGPKG_DEVS_ETH_ARM_IXDP465_NPE_ETH2
#define CYGSEM_INTEL_NPE_USE_ETH2
#define CYGNUM_ETH2_ETH_ID    IX_ETH_PORT_3
#define CYGNUM_ETH2_PHY_NO    NPE_PHY_UNKNOWN
#define CYGDAT_NPE_ETH2_NAME  CYGDAT_DEVS_ETH_ARM_IXDP465_NPE_ETH2_NAME
#endif

#ifdef CYGSEM_NPE_SMII

#define CYGDAT_NPE_SMII_DLL_SETTING 0x18e

static int
ixdp465_smii_init(void)
{
#if CYGINT_DEVS_ETH_INTEL_NPEB_SMII
    int smii_mask = 0;
    int smii_cnt = 0;

    diag_printf("\nChecking for SMII configuration...");

    // We need this to before looking for PHYs
    if (!npe_csr_load()) {
        diag_printf("ixdp465_smii_init: npe_csr_load failed\n");
        return 0;
    }

    // IXDP465 jumpering supports:
    //
    //    Normal MII mode:  NPEB-->PHY0, NPEC-->PHY1, NPEA -->PHY1||PHY2
    //    SMII mode: NPEA-->PHY8, NPEB-->PHY9, NPEC-->PHY13
    //
    //    NPEB must be in SMII mode for NPEA or NPEC to use SMII
    //    
    if (!check_phy_association(IX_ETH_PORT_1, 0)) {
        smii_mask |= (1 << IX_ETH_PORT_1);
        ++smii_cnt;
#if CYGINT_DEVS_ETH_INTEL_NPEC_SMII
        if (!check_phy_association(IX_ETH_PORT_2, 1)) {
            smii_mask |= (1 << IX_ETH_PORT_2);
            ++smii_cnt;
        } else
            set_phy_association(IX_ETH_PORT_2, 1);
#endif
#if CYGINT_DEVS_ETH_INTEL_NPEA_SMII
        if (npe_exists[IX_ETH_PORT_3]) {
            if (!check_phy_association(IX_ETH_PORT_3, 2)) {
                if (smii_mask & (1 << IX_ETH_PORT_2)) {
                    // NPEC not connected to PHY1, maybe NPEA is
                    if (!check_phy_association(IX_ETH_PORT_3, 1)) {
                        smii_mask |= (1 << IX_ETH_PORT_3);
                        ++smii_cnt;
                    } else
                        set_phy_association(IX_ETH_PORT_3, 1);
                } else {
                    smii_mask |= (1 << IX_ETH_PORT_3);
                    ++smii_cnt;
                }
            } else
                set_phy_association(IX_ETH_PORT_3, 2);
        }
#endif
    } else {
        set_phy_association(IX_ETH_PORT_1, 0);
        set_phy_association(IX_ETH_PORT_2, 1);
    }

    // We need this to put CSR back the way it was so that
    // the "real" npe_csr_load call will work.
    npe_csr_unload();

    if (smii_cnt) {
        // One or more ports using SMII. (or has PHY missing)
        diag_printf("yes.\nInitializing SMII for [NPE-B");
        if (--smii_cnt > 0)
            diag_printf("][");
        if (smii_mask & (1 << IX_ETH_PORT_2)) {
            diag_printf("NPE-C");
            if (--smii_cnt > 0)
                diag_printf("][");
        }
        if (smii_mask & (1 << IX_ETH_PORT_3))
            diag_printf("NPE-A");
        printf("]...");
        
        if (!npe_enable_smii_mode(smii_mask)) {
            diag_printf("failed!\n");
            return 0;
        }

        set_phy_association(IX_ETH_PORT_1, 9);

#if CYGINT_DEVS_ETH_INTEL_NPEC_SMII
        if (smii_mask & (1 << IX_ETH_PORT_2))
            set_phy_association(IX_ETH_PORT_2, 13);
#endif
#if CYGINT_DEVS_ETH_INTEL_NPEA_SMII
        if (smii_mask & (1 << IX_ETH_PORT_3))
            set_phy_association(IX_ETH_PORT_3, 8);
#endif
        diag_printf("done.");
    } else
        diag_printf("no, using MII.");

#endif  // CYGINT_DEVS_ETH_INTEL_NPEB_SMII
    return 1;
}

#define CYGHAL_NPE_SMII_INIT() ixdp465_smii_init()
#endif


#define CYGDAT_ETH0_DEFAULT_ESA {0x00, 0x03, 0x47, 0xdf, 0x32, 0xa8}
#define CYGDAT_ETH1_DEFAULT_ESA {0x00, 0x03, 0x47, 0xdf, 0x32, 0xaa}
#define CYGDAT_ETH2_DEFAULT_ESA {0x00, 0x03, 0x47, 0xdf, 0x32, 0xac}

#ifndef CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA
#ifdef CYGSEM_DEVS_ETH_INTEL_NPE_PLATFORM_EEPROM
extern int cyghal_get_npe_esa(int, cyg_uint8 *);

#define CYGHAL_GET_NPE_ESA(_ethid,_addr,_res) \
 (_res) = cyghal_get_npe_esa((_ethid), (_addr))

#endif // CYGSEM_DEVS_ETH_INTEL_NPE_PLATFORM_EEPROM
#endif // CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA
