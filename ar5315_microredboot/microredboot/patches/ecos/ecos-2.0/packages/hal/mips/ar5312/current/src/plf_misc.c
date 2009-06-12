//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    adrian
// Contributors: based on existing files for other OS
// Date:         2003-10-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>

#include <cyg/hal/ar531xreg.h>

static void hal_ar5312_flash_setup(void);

typedef cyg_uint32 u32;
typedef cyg_uint16 u16;
typedef cyg_uint8  u8;

/*
 * This is board-specific data that is stored in a "fixed" location in flash.
 * It is shared across operating systems, so it should not be changed lightly.
 * The main reason we need it is in order to extract the ethernet MAC
 * address(es).
 */
struct ar531x_boarddata {
    u32 magic;                       /* board data is valid */
#define AR531X_BD_MAGIC 0x35333131   /* "5311", for all 531x platforms */
    u16 cksum;                       /* checksum (starting with BD_REV 2) */
    u16 rev;                         /* revision of this struct */
#define BD_REV  4
    char   boardName[64];            /* Name of board */
    u16 major;                       /* Board major number */
    u16 minor;                       /* Board minor number */
    u32 config;                      /* Board configuration */
#define BD_ENET0        0x00000001   /* ENET0 is stuffed */
#define BD_ENET1        0x00000002   /* ENET1 is stuffed */
#define BD_UART1        0x00000004   /* UART1 is stuffed */
#define BD_UART0        0x00000008   /* UART0 is stuffed (dma) */
#define BD_RSTFACTORY   0x00000010   /* Reset factory defaults stuffed */
#define BD_SYSLED       0x00000020   /* System LED stuffed */
#define BD_EXTUARTCLK   0x00000040   /* External UART clock */
#define BD_CPUFREQ      0x00000080   /* cpu freq is valid in nvram */
#define BD_SYSFREQ      0x00000100   /* sys freq is set in nvram */
#define BD_WLAN0        0x00000200   /* use WLAN0 */
#define BD_MEMCAP       0x00000400   /* CAP SDRAM @ memCap for testing */
#define BD_DISWATCHDOG  0x00000800   /* disable system watchdog */
#define BD_WLAN1        0x00001000   /* Enable WLAN1 (ar5212) */
#define BD_ISCASPER     0x00002000   /* FLAG for AR2312 */
#define BD_WLAN0_2G_EN  0x00004000   /* FLAG for radio0_2G */
#define BD_WLAN0_5G_EN  0x00008000   /* FLAG for radio0_2G */
#define BD_WLAN1_2G_EN  0x00020000   /* FLAG for radio0_2G */
#define BD_WLAN1_5G_EN  0x00040000   /* FLAG for radio0_2G */
    u16 resetConfigGpio;             /* Reset factory GPIO pin */
    u16 sysLedGpio;                  /* System LED GPIO pin */

    u32 cpuFreq;                     /* CPU core frequency in Hz */
    u32 sysFreq;                     /* System frequency in Hz */
    u32 cntFreq;                     /* Calculated C0_COUNT frequency */

    u8  wlan0Mac[6];
    u8  enet0Mac[6];
    u8  enet1Mac[6];

    u16 pciId;                       /* Pseudo PCIID for common code */
    u16 memCap;                      /* cap bank1 in MB */

    /* version 3 */
    u8  wlan1Mac[6];                 /* (ar5212) */
};

//--------------------------------------------------------------------------


void
hal_platform_init(void)
{
    HAL_WRITE_UINT32(AR531X_WDC, 0);

    hal_ar5312_flash_setup();

    // Set up eCos/ROM interfaces
    hal_if_init();

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
}


/*
 * This table is indexed by bits 5..4 of the CLOCKCTL1 register
 * to determine the predevisor value.
 */
static int AR531X_CLOCKCTL1_PREDIVIDE_TABLE[4] = {
    1,
    2,
    4,
    5
};

unsigned int
hal_ar531x_cpu_frequency(void)
{
static unsigned int ar531x_calculated_cpu_freq = 0;

   /*
    * Clocking is derived from a fixed 40MHz input clock.
    *  cpuFreq = InputClock * MULT (where MULT is PLL multiplier)
    *
    *  sysFreq = cpuFreq / 4       (used for APB clock, serial,
    *                               flash, Timer, Watchdog Timer)
    *
    *  cntFreq = cpuFreq / 2       (use for CPU count/compare)
    *
    * So, for example, with a PLL multiplier of 5, we have
    *  cpuFrez = 200MHz
    *  sysFreq = 50MHz
    *  cntFreq = 100MHz
    *
    * We compute the CPU frequency, based on PLL settings.
    */
    if (ar531x_calculated_cpu_freq == 0) {
        unsigned int clockCtl1;
        int preDivideSelect;
        int preDivisor;
        int multiplier;

        HAL_READ_UINT32(AR531X_CLOCKCTL1, clockCtl1);

        preDivideSelect = (clockCtl1 & AR531X_CLOCKCTL1_PREDIVIDE_MASK) >>
                                   AR531X_CLOCKCTL1_PREDIVIDE_SHIFT;

        preDivisor = AR531X_CLOCKCTL1_PREDIVIDE_TABLE[preDivideSelect];

        multiplier = (clockCtl1 & AR531X_CLOCKCTL1_MULTIPLIER_MASK) >>
                              AR531X_CLOCKCTL1_MULTIPLIER_SHIFT;

        if (clockCtl1 & AR531X_CLOCKCTL1_DOUBLER_MASK) {
                multiplier = multiplier << 1;
        }

        ar531x_calculated_cpu_freq = (40000000 / preDivisor) * multiplier;
    }

    return ar531x_calculated_cpu_freq;
}

unsigned int
hal_ar531x_sys_frequency(void)
{
    static unsigned int ar531x_calculated_sys_freq = 0;

    if (ar531x_calculated_sys_freq == 0) {
        ar531x_calculated_sys_freq = hal_ar531x_cpu_frequency() / 4;
    }

    return ar531x_calculated_sys_freq;
}

char *ar531x_board_configuration;
char *ar531x_radio_configuration;

static void
hal_ar5312_flash_setup(void)
{
    cyg_uint32 flash_ctl;
    int ar531x_flash_data_found;

    HAL_READ_UINT32(AR531X_FLASHCTL0, flash_ctl);

    flash_ctl &= FLASHCTL_MW;

    /* 
     * Configure flash bank 0.
     * We lie about the prom size here in order to allow it to be multiply
     * mapped.  The heuristics, below, count on that in order to easily 
     * find configuration data.  The flash drivers know how big the flash
     * really is.
     */
    flash_ctl = FLASHCTL_E |
                FLASHCTL_AC_8M |
                FLASHCTL_RBLE |
                (0x01 << FLASHCTL_IDCY_S) |
                (0x07 << FLASHCTL_WST1_S) |
                (0x07 << FLASHCTL_WST2_S) |
                flash_ctl;

    HAL_WRITE_UINT32(AR531X_FLASHCTL0, flash_ctl);

    /* Disable other flash banks */
    HAL_READ_UINT32(AR531X_FLASHCTL1, flash_ctl);
    flash_ctl &= ~(FLASHCTL_E | FLASHCTL_AC);
    HAL_WRITE_UINT32(AR531X_FLASHCTL1, flash_ctl);

    HAL_READ_UINT32(AR531X_FLASHCTL2, flash_ctl);
    flash_ctl &= ~(FLASHCTL_E | FLASHCTL_AC);
    HAL_WRITE_UINT32(AR531X_FLASHCTL2, flash_ctl);

    /*
     * Find start of Radio Configuration data, using heuristics:
     * Search back from the (aliased) end of flash by 0x1000 bytes
     * at a time until we find the string "5311", which marks the
     * start of Board Configuration.  Give up if we've searched
     * more than 500KB.
     */
    ar531x_flash_data_found = 0;
    for (ar531x_board_configuration = (char *)0xbffff000;
         ar531x_board_configuration > (char *)0xbff80000;
         ar531x_board_configuration -= 0x1000)
    {
        if ( *(int *)ar531x_board_configuration == AR531X_BD_MAGIC) {
            ar531x_flash_data_found = 1;
            break;
        }
    }

    if (!ar531x_flash_data_found) {
        return;
    }

    /* 
     * Now find the start of Board Configuration data, using heuristics:
     * Search forward from Board Configuration data by 0x1000 bytes
     * at a time until we find non-0xffffffff.
     */
    ar531x_flash_data_found = 0;
    for (ar531x_radio_configuration = ar531x_board_configuration + 0x1000;
         ar531x_radio_configuration < (char *)0xbffff000;
         ar531x_radio_configuration += 0x1000)
    {
        if (*(int *)ar531x_radio_configuration != 0xffffffff) {
            ar531x_flash_data_found = 1;
            break;
        }
    }

    if (!ar531x_flash_data_found) {
        return;
    }
}

/*
 * Fetch a pointer to an ethernet's MAC address
 * in the Board Configuration data (in flash).
 */
unsigned char *
enet_mac_address_get(int unit)
{
    struct ar531x_boarddata *board_config_data =
        (struct ar531x_boarddata *)ar531x_board_configuration;
    unsigned char *mac_addr = NULL;

    if (!board_config_data)
        return NULL;

    if (unit == 0) {
        mac_addr = board_config_data->enet0Mac;
    }

    if (unit == 1) {
        mac_addr = board_config_data->enet1Mac;
    }

    /* Small sanity check, mainly to deal with uninitialized board config */
    if (mac_addr[0] != 0xff) {
        return mac_addr;
    }

    return NULL;
}

/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void
hal_ar5312_reset(void)
{
    for(;;) {
        HAL_WRITE_UINT32(AR531X_RESET, RESET_SYSTEM);
    }
}


/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
