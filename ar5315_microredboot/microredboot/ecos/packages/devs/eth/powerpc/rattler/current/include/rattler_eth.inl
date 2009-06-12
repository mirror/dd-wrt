#ifndef CYGONCE_DEVS_RATTLER_ETH_INL
#define CYGONCE_DEVS_RATTLER_ETH_INL
//==========================================================================
//
//      rattler_eth.inl
//
//      Hardware specifics for A&M Rattler ethernet support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    gthomas
// Contributors: gthomas,F.Robbins
// Date:         2003-08-19
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

// 
// Pin layout for PHY connections
//
#define FCC1_PHY_RESET 0x01000000
#define FCC1_PHY_DATA  0x10000000
#define FCC1_PHY_CLOCK 0x20000000
#define FCC2_PHY_RESET 0x02000000
#define FCC2_PHY_DATA  0x04000000
#define FCC2_PHY_CLOCK 0x08000000

#ifdef CYGHWR_DEVS_ETH_POWERPC_RATTLER_FCC1
//
// Initialize the PHY associated with FCC1/eth0
//
static void 
fcc1_phy_init(void)
{
    // Set up PHY reset line
    IMM->io_regs[PORT_B].pdat |= FCC1_PHY_RESET;
    IMM->io_regs[PORT_C].pdir |= FCC1_PHY_CLOCK;
}

//
// Reset the PHY associated with FCC1/eth0
//
static void 
fcc1_phy_reset(void)
{
    // Toggle PHY reset line
    IMM->io_regs[PORT_B].pdat &= ~FCC1_PHY_RESET;
    IMM->io_regs[PORT_B].pdat |= FCC1_PHY_RESET;
}

//
// Set up a particular data bit for FCC1/eth0
//
static void 
fcc1_phy_set_data(int val)
{
    if (val) {
        // Output
        IMM->io_regs[PORT_C].pdat |= FCC1_PHY_DATA;
    } else {
        // Input
        IMM->io_regs[PORT_C].pdat &= ~FCC1_PHY_DATA;
    }
}

//
// Read the current data bit for FCC1/eth0
//
static int  
fcc1_phy_get_data(void)
{
    if ((IMM->io_regs[PORT_C].pdat & FCC1_PHY_DATA) != 0) {
        return 1;
    } else {
        return 0;
    }
}

//
// Set the clock bit for FCC1/eth0
//
static void 
fcc1_phy_set_clock(int val)
{
    if (val) {
        // Output
        IMM->io_regs[PORT_C].pdat |= FCC1_PHY_CLOCK;
    } else {
        // Input
        IMM->io_regs[PORT_C].pdat &= ~FCC1_PHY_CLOCK;
    }
}

//
// Set the clock/data direction for FCC1/eth0
// Note: always forces clock to be an output
//
static void 
fcc1_phy_set_dir(int data_dir)
{
    if (data_dir) {
        // Output
        IMM->io_regs[PORT_C].pdir |= FCC1_PHY_DATA;
    } else {
        // Input
        IMM->io_regs[PORT_C].pdir &= ~FCC1_PHY_DATA;
    }
}

ETH_PHY_BIT_LEVEL_ACCESS_FUNS(fcc1_phy,
                    fcc1_phy_init,
                    fcc1_phy_reset,
                    fcc1_phy_set_data,
                    fcc1_phy_get_data,
                    fcc1_phy_set_clock,
                    fcc1_phy_set_dir);

static unsigned char fcc_eth0_rxbufs[CYGNUM_DEVS_ETH_POWERPC_FCC_RxNUM *
                                    (CYGNUM_DEVS_ETH_POWERPC_FCC_BUFSIZE + 32)];
static unsigned char fcc_eth0_txbufs[CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM *
                                    (CYGNUM_DEVS_ETH_POWERPC_FCC_BUFSIZE + 32)];

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
RedBoot_config_option("FCC1/eth0 Network hardware address [MAC]",
                      fcc1_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif

static struct fcc_eth_info fcc_eth0_info = {
    CYGNUM_HAL_INTERRUPT_FCC1,               // Interrupt
    "fcc1_esa",                              // ESA 'key'
    { 0x00, 0x08, 0xe5, 0x11, 0x22, 0x33 },  // Fallback ESA
    CYGNUM_DEVS_ETH_POWERPC_FCC_RxNUM,       // Number of Rx buffers
    fcc_eth0_rxbufs,                         // Pointer to buffers
    CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM,       // Number of Tx buffers
    fcc_eth0_txbufs,                         // Pointer to buffers    
    &fcc1_phy,
};

ETH_DRV_SC(fcc_eth0_sc,
           &fcc_eth0_info,     // Driver specific data
           "eth0",             // Name for this interface
           fcc_eth_start,
           fcc_eth_stop,
           fcc_eth_control,
           fcc_eth_can_send,
           fcc_eth_send,
           fcc_eth_recv,
           fcc_eth_deliver,
           fcc_eth_int,
           fcc_eth_int_vector);

NETDEVTAB_ENTRY(fcc_eth0_netdev, 
                "fcc_eth0", 
                fcc_eth_init, 
                &fcc_eth0_sc);
#endif // CYGHWR_DEVS_ETH_POWERPC_RATTLER_FCC1

#ifdef CYGHWR_DEVS_ETH_POWERPC_RATTLER_FCC2
//
// Initialize the PHY associated with FCC2/eth1
//
static void 
fcc2_phy_init(void)
{
    // Set up PHY reset line
    IMM->io_regs[PORT_B].pdat |= FCC2_PHY_RESET;
    IMM->io_regs[PORT_C].pdir |= FCC2_PHY_CLOCK;
}

//
// Reset the PHY associated with FCC2/eth1
//
static void 
fcc2_phy_reset(void)
{
    // Toggle the PHY reset line
    IMM->io_regs[PORT_B].pdat &= ~FCC2_PHY_RESET;
    IMM->io_regs[PORT_B].pdat |= FCC2_PHY_RESET;
}

//
// Set up a particular data bit for FCC2/eth1
//
static void 
fcc2_phy_set_data(int val)
{
    if (val) {
        // Output
        IMM->io_regs[PORT_C].pdat |= FCC2_PHY_DATA;
    } else {
        // Input
        IMM->io_regs[PORT_C].pdat &= ~FCC2_PHY_DATA;
    }
}

//
// Read the current data bit for FCC2/eth1
//
static int  
fcc2_phy_get_data(void)
{
    if ((IMM->io_regs[PORT_C].pdat & FCC2_PHY_DATA) != 0) {
        return 1;
    } else {
        return 0;
    }
}

//
// Set the clock bit for FCC2/eth1
//
static void 
fcc2_phy_set_clock(int val)
{
    if (val) {
        // Output
        IMM->io_regs[PORT_C].pdat |= FCC2_PHY_CLOCK;
    } else {
        // Input
        IMM->io_regs[PORT_C].pdat &= ~FCC2_PHY_CLOCK;
    }
}

//
// Set the clock/data direction for FCC2/eth1
// Note: always forces clock to be an output
//
static void 
fcc2_phy_set_dir(int data_dir)
{
    if (data_dir) {
        // Output
        IMM->io_regs[PORT_C].pdir |= FCC2_PHY_DATA;
    } else {
        // Input
        IMM->io_regs[PORT_C].pdir &= ~FCC2_PHY_DATA;
    }
}

ETH_PHY_BIT_LEVEL_ACCESS_FUNS(fcc2_phy,
                    fcc2_phy_init,
                    fcc2_phy_reset,
                    fcc2_phy_set_data,
                    fcc2_phy_get_data,
                    fcc2_phy_set_clock,
                    fcc2_phy_set_dir);

static unsigned char fcc_eth1_rxbufs[CYGNUM_DEVS_ETH_POWERPC_FCC_RxNUM *
                                    (CYGNUM_DEVS_ETH_POWERPC_FCC_BUFSIZE + 32)];
static unsigned char fcc_eth1_txbufs[CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM *
                                    (CYGNUM_DEVS_ETH_POWERPC_FCC_BUFSIZE + 32)];

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
RedBoot_config_option("FCC2/eth1 Network hardware address [MAC]",
                      fcc2_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif

static struct fcc_eth_info fcc_eth1_info = {
    CYGNUM_HAL_INTERRUPT_FCC2,               // Interrupt
    "fcc2_esa",                              // ESA 'key'
    { 0x00, 0x08, 0xe5, 0x11, 0x22, 0x33 },  // Fallback ESA
    CYGNUM_DEVS_ETH_POWERPC_FCC_RxNUM,       // Number of Rx buffers
    fcc_eth1_rxbufs,                         // Pointer to buffers
    CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM,       // Number of Tx buffers
    fcc_eth1_txbufs,                         // Pointer to buffers    
    &fcc2_phy,
};

ETH_DRV_SC(fcc_eth1_sc,
           &fcc_eth1_info,     // Driver specific data
           "eth1",             // Name for this interface
           fcc_eth_start,
           fcc_eth_stop,
           fcc_eth_control,
           fcc_eth_can_send,
           fcc_eth_send,
           fcc_eth_recv,
           fcc_eth_deliver,
           fcc_eth_int,
           fcc_eth_int_vector);

NETDEVTAB_ENTRY(fcc_eth1_netdev, 
                "fcc_eth1", 
                fcc_eth_init, 
                &fcc_eth1_sc);
#endif // CYGHWR_DEVS_ETH_POWERPC_RATTLER_FCC2

#endif  // CYGONCE_DEVS_RATTLER_ETH_INL
// ------------------------------------------------------------------------
