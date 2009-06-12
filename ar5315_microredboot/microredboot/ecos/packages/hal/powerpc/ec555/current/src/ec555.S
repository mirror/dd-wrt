##=============================================================================
##
##      ec555.S
##
##      ec555 board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   Bob Koninckx
## Contributors:Bob Koninckx
## Date:        2002-01-01
## Purpose:     ec555 board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a ec555 mpc555 board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/arch.inc>

#include <pkgconf/system.h>
#ifdef CYGPKG_DEVS_ETH_POWERPC_EC555
#include <pkgconf/devs_eth_powerpc_ec555.h>
#endif

## FIXME
## The following probably belongs in the variant hal rather than the board specifics ... 
#ifdef CYGPKG_DEVICES_WATCHDOG_MPC5xx
#include <pkgconf/devices_watchdog_mpc5xx.h>
#define CYG_SYPCR 0x0000ff8e | (CYGNUM_DEVICES_WATCHDOG_POWERPC_MPC5XX_RELOAD << 16) | CYGDAT_DEVICES_WATCHDOG_POWERPC_MPC5XX_PRESCALE
#else
#define CYG_SYPCR 0x0000ff88
#endif

#define ADDRESS_MASK_1MB         0xfff00000
#define ADDRESS_MASK_2MB         0xffe00000
#define ADDRESS_MASK_4MB         0xffc00000
#define ADDRESS_MASK_8MB         0xff800000
#define ADDRESS_MASK_16MB        0xff000000

#define EC555_RAM_BASE_ADDRESS   0x01000000
#define EC555_FLASH_BASE_ADDRESS 0x02000000
#define EC555_CS2_BASE_ADDRESS   0x04000000
#define EC555_CS3_BASE_ADDRESS   0x08000000

#if defined(CYGHWR_HAL_EC555_BOARD_VARIANT_F02_S01)
#define EC555_RAM_ADDRESS_MASK   ADDRESS_MASK_1MB
#define EC555_FLASH_ADDRESS_MASK ADDRESS_MASK_2MB
#elif defined(CYGHWR_HAL_EC555_BOARD_VARIANT_F04_S02)
#define EC555_RAM_ADDRESS_MASK   ADDRESS_MASK_2MB
#define EC555_FLASH_ADDRESS_MASK ADDRESS_MASK_4MB
#elif defined(CYGHWR_HAL_EC555_BOARD_VARIANT_F08_S04)
#define EC555_RAM_ADDRESS_MASK   ADDRESS_MASK_4MB
#define EC555_FLASH_ADDRESS_MASK ADDRESS_MASK_8MB
#elif defined(CYGHWR_HAL_EC555_BOARD_VARIANT_F08_S08)
#define EC555_RAM_ADDRESS_MASK   ADDRESS_MASK_8MB
#define EC555_FLASH_ADDRESS_MASK ADDRESS_MASK_16MB
#else
#error "EC555 Board variant unspecified"
#endif

#define EC555_CS2_ADDRESS_MASK   0xffff8000
#define EC555_CS3_ADDRESS_MASK   0xffff8000

#------------------------------------------------------------------------------
                
        .globl  hal_hardware_init
hal_hardware_init:
#if defined(CYGPKG_HAL_POWERPC_EC555) && defined(CYGPKG_HAL_POWERPC_MPC5xx)
        lwi     r3, CYGARC_REG_IMM_BASE             # Base address of control registers

#if defined(CYG_HAL_STARTUP_ROM)
        // Burst enable
        lwi     r0, 0x00002000
        mtspr   560, r0
       
        // FIXME
        // The following probably belongs in the variant hal rather than the board specifics ... 
        // Disable / enable the Watchdog
        lwi     r4, CYG_SYPCR
        stw     r4, (CYGARC_REG_IMM_SYPCR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0x00000000
        stw     r4, (CYGARC_REG_IMM_SIUMCR-CYGARC_REG_IMM_BASE)(r3)

        // Unlock locked registers
        lwi     r4, 0x55ccaa33
        stw     r4, (CYGARC_REG_IMM_TBSCRK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_TBREF0K-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_TBREF1K-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_TBK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_RTCSCK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_RTCK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_RTSECK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_RTCALK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_PISCRK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_PITCK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_SCCRK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_PLPRCRK-CYGARC_REG_IMM_BASE)(r3)
        stw     r4, (CYGARC_REG_IMM_RSRK-CYGARC_REG_IMM_BASE)(r3)

        // Either Redboot or BDM will have already done it otherwise
        // Boost the clock to 40MHz
        lwi     r4, 0x03000000
        stw     r4, (CYGARC_REG_IMM_SCCR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0x009150c0
        stw     r4, (CYGARC_REG_IMM_PLPRCR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0x0080
        sth     r4, (CYGARC_REG_IMM_COLIR-CYGARC_REG_IMM_BASE)(r3)

        // Set up the memory map
        // Do NOT write protect the flash memory, flash drivers won't work 
        // if we do

        // flash banks
        lwi     r4, (EC555_FLASH_BASE_ADDRESS | 0x00000003)
        stw     r4, (CYGARC_REG_IMM_BR0-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, (EC555_FLASH_ADDRESS_MASK | 0x00000530)
        stw     r4, (CYGARC_REG_IMM_OR0-CYGARC_REG_IMM_BASE)(r3)

        // ram banks
        lwi     r4, (EC555_RAM_BASE_ADDRESS | 0x00000011)
        stw     r4, (CYGARC_REG_IMM_BR1-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, (EC555_RAM_ADDRESS_MASK | 0x00000000)
        stw     r4, (CYGARC_REG_IMM_OR1-CYGARC_REG_IMM_BASE)(r3)
#endif

#ifdef CYGPKG_DEVS_ETH_POWERPC_EC555
#if   (CYGNUM_DEVS_ETH_POWERPC_EC555_ETH0_CS == 2)
        lwi     r4, (EC555_CS2_BASE_ADDRESS | 0x00000803)
        stw     r4, (CYGARC_REG_IMM_BR2-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xffff8ca0
        stw     r4, (CYGARC_REG_IMM_OR2-CYGARC_REG_IMM_BASE)(r3)

        lwi     r4, EC555_CS3_BASE_ADDRESS
        stw     r4, (CYGARC_REG_IMM_BR3-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, EC555_CS3_ADDRESS_MASK
        stw     r4, (CYGARC_REG_IMM_OR3-CYGARC_REG_IMM_BASE)(r3)
#elif (CYGNUM_DEVS_ETH_POWERPC_EC555_ETH0_CS == 3)
        lwi     r4, EC555_CS2_BASE_ADDRESS
        stw     r4, (CYGARC_REG_IMM_BR2-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, EC555_CS2_ADDRESS_MASK
        stw     r4, (CYGARC_REG_IMM_OR2-CYGARC_REG_IMM_BASE)(r3)

        lwi     r4, (EC555_CS3_BASE_ADDRESS | 0x00000803)
        stw     r4, (CYGARC_REG_IMM_BR3-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xffff8ca0
        stw     r4, (CYGARC_REG_IMM_OR3-CYGARC_REG_IMM_BASE)(r3)
#else
#error "Invalid chip select for ethernet card specified"
#endif
#else
        lwi     r4, EC555_CS2_BASE_ADDRESS
        stw     r4, (CYGARC_REG_IMM_BR2-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, EC555_CS2_ADDRESS_MASK
        stw     r4, (CYGARC_REG_IMM_OR2-CYGARC_REG_IMM_BASE)(r3)

        lwi     r4, EC555_CS3_BASE_ADDRESS
        stw     r4, (CYGARC_REG_IMM_BR3-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, EC555_CS3_ADDRESS_MASK
        stw     r4, (CYGARC_REG_IMM_OR3-CYGARC_REG_IMM_BASE)(r3)
#endif

#if defined(CYGSEM_HAL_POWERPC_MPC5XX_IFLASH_DUAL_MAP)
        lwi     r4, 1
#else
        lwi     r4, 0
#endif
        stw     r4, (CYGARC_REG_IMM_DMBR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0
        stw     r4, (CYGARC_REG_IMM_DMOR-CYGARC_REG_IMM_BASE)(r3)

        // Enable the time base, but do _not_ set the freeze flag
        lwi     r4, 0xc1
        sth     r4, (CYGARC_REG_IMM_TBSCR-CYGARC_REG_IMM_BASE)(r3)

        // RTC is clocked by 4MHz crystal, set the freeze flag
        lwi     r4, 0xd2
        sth     r4, (CYGARC_REG_IMM_RTCSC-CYGARC_REG_IMM_BASE)(r3)

        // Set the freeze flag for the Periodic interrupt timer
        lwi     r4, 0x82
        sth     r4, (CYGARC_REG_IMM_PISCR-CYGARC_REG_IMM_BASE)(r3)

        // USIU rest.
        lwi     r4, 0x00000000
        lwi     r5, (CYGARC_REG_IMM_SGPIODT1-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_SGPIODT2-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_SGPIOCR-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
        lwi     r4, 0x00000ac6
        lwi     r5, (CYGARC_REG_IMM_EMCR-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5

        // Dual ported TPU RAM
        lwi     r4, 0x0000
        lwi     r5, (CYGARC_REG_IMM_DPTMCR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0xffa0
        lwi     r5, (CYGARC_REG_IMM_RAMBAR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0x00
        lwi     r5, (CYGARC_REG_IMM_PORTQS-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0x00
        lwi     r5, (CYGARC_REG_IMM_PQSPAR_DDRQST-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_MPIOSMDR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_MPIOSMDDR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_SRAMMCR_A-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        
        // They are assigned OCD functionality on this board
        // Change the following to anything else  and BDM will not work anymore
        // on the ec555. This is not true for all MPC555 based boards, eg. cme555
        // does not have this requirement
        lwi     r4, 0x3
        lwi     r5, (CYGARC_REG_IMM_MIOS1TPCR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        // Enable 32 interrupt priorities on the IMB3 unit
        lwi     r4, 0x60000000
        lwi     r5, (CYGARC_REG_IMM_UMCR-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
#endif                
        sync
        blr

#------------------------------------------------------------------------------
# end of ec555.S
