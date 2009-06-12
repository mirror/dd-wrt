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
// Author(s):    vsubbiah
// Contributors: based on existing files for other platforms.
// Date:         2005-11-01
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

//#include <cyg/hal/ar531xreg.h>

#include "uart16550.h"


static void hal_idt32438_flash_setup(void);

typedef cyg_uint32 u32;
typedef cyg_uint16 u16;
typedef cyg_uint8  u8;

/*
 * This is board-specific data that is stored in a "fixed" location in flash.
 * It is shared across operating systems, so it should not be changed lightly.
 * The main reason we need it is in order to extract the ethernet MAC
 * address(es).
 */
struct idt32438_boarddata {
    u32 magic;                       /* board data is valid */
#define IDT32438_BD_MAGIC 0x50423332   /* "PB32" */
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

void uart_test();

void
hal_platform_init(void)
{
    uart_test();

    hal_idt32438_flash_setup();

    hal_if_init();
    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
}


unsigned int
hal_idt32438_cpu_frequency(void)
{
   unsigned int Mhz=10000000;
   
   return 100*Mhz;

}

unsigned int
hal_idt32438_sys_frequency(void)
{

   unsigned int Mhz=10000000;

   //This needs to be validated. Not used for know.
   return 100*Mhz;


}

char *idt32438_board_configuration;
char *idt32438_radio_configuration;

static void
hal_idt32438_flash_setup(void)
{
    void *err_addr;
    cyg_uint32 flash_data;
    int idt32438_flash_data_found;
#ifdef CYGNUM_USE_CONSOLIDATED_BOARD_DATA
    char* board_data_scan_start = 0xbfff0000; 
    int radio_config_offset   = 0xf8;
#else 
    char *board_data_scan_start = 0xbffff000;
    int radio_config_offset   = 0x1000;
#endif
    /*
     * Find start of Board Configuration data, using heuristics:
     * Search back from the (aliased) end of flash by 0x1000 bytes
     * at a time until we find the string "PB32", which marks the
     * start of Board Configuration.  Give up if we've searched
     * more than 500KB.
     */
    idt32438_flash_data_found = 0;
    
    for (idt32438_board_configuration = (char *)board_data_scan_start;
         idt32438_board_configuration > (char *)0xbff80000;
         idt32438_board_configuration -= 0x1000)
    {
        if(*(int *)idt32438_board_configuration == IDT32438_BD_MAGIC) {
            idt32438_flash_data_found = 1;
            break;
        }
    }

    if (!idt32438_flash_data_found) {
        idt32438_board_configuration = (char *)NULL;
        // diag_printf("hal_ar5312_flash_setup:Flash data not found.\n");
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
    struct idt32438_boarddata *board_config_data =
        (struct idt32438_boarddata *)idt32438_board_configuration;
    unsigned char *mac_addr = NULL;

    if (!board_config_data) {
       diag_printf("enet_mac_address_get:Board config not found.\n");
       return NULL;
    }

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
    diag_printf("enet_mac_address_get:Mac addr failed sanity check.\n");
    return NULL;
}

/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void
hal_idt32438_reset(void)
{

}

int DEBUG_LED;
#include "uart16550.h"


void uart_test()
{
    int diff = 'A' - 'a';

    DEBUG_LED  = 0x11;
#if 1
    Uart16550Init(9600, 
                  UART16550_DATA_8BIT,
                  UART16550_PARITY_NONE,
                  UART16550_STOP_1BIT);
#endif

#if 0
    for(;;) {
        int c = Uart16550GetPoll();
        Uart16550Put(c);
        c += diff;
        Uart16550Put(c);
        Uart16550Put('\r');
        Uart16550Put('\n');
    }
#endif 
}


/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
