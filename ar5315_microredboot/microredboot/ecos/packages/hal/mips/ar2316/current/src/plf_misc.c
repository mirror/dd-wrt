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

#include <cyg/hal/ar2316reg.h>

static void hal_ar2316_flash_setup(void);

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

char *ar531x_board_configuration;
char *ar531x_radio_configuration;

//--------------------------------------------------------------------------


static void
sysGpioSet(int gpio, int val)
{
    u32 reg;
	
    HAL_READ_UINT32(AR2316_GPIO_DO, reg);
    reg &= ~(1 << gpio);
    reg |= (val&1) << gpio;
    HAL_WRITE_UINT32(AR2316_GPIO_DO, reg);
    HAL_READ_UINT32(AR2316_GPIO_DO, reg); /* flush write to hardware */
} 

void
hal_platform_init(void)
{
    volatile u32 tmp;

    HAL_WRITE_UINT32(AR2316_WDC, 0);

    /* Setup the GPIO so we make sure the reset gpio is ok */
    sysGpioSet(AR2316_RESET_GPIO, 1);
    HAL_READ_UINT32(AR2316_GPIO_CR, tmp);
    HAL_WRITE_UINT32(AR2316_GPIO_CR, (tmp | GPIO_CR_O(AR2316_RESET_GPIO)));
    HAL_READ_UINT32(AR2316_GPIO_DO, tmp); /* flush write to hardware */

    sysGpioSet(0, 1);
    HAL_READ_UINT32(AR2316_GPIO_CR, tmp);
    HAL_WRITE_UINT32(AR2316_GPIO_CR, (tmp | GPIO_CR_O(0)));
    HAL_READ_UINT32(AR2316_GPIO_DO, tmp); /* flush write to hardware */

    /* CPU owns AHB bus */
    HAL_WRITE_UINT32(AR2316_AHB_ARB_CTL, ARB_CPU);

    hal_if_init();
    hal_ar2316_flash_setup();

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
}

#define AR5315_DSLBASE          0xB1000000      /* RESET CONTROL MMR */
#define AR5315_PLLC_CTL         (AR5315_DSLBASE + 0x0064)
#define AR5315_CPUCLK           (AR5315_DSLBASE + 0x006c)
#define AR5315_AMBACLK          (AR5315_DSLBASE + 0x0070)
#define AR5315_RESET            (AR5315_DSLBASE + 0x0004)
#define AR5315_RESET_UART0                 0x00000100      /* warm reset UART0 */

/* PLLc Control fields */
#define PLLC_REF_DIV_M              0x00000003
#define PLLC_REF_DIV_S              0
#define PLLC_FDBACK_DIV_M           0x0000007C
#define PLLC_FDBACK_DIV_S           2
#define PLLC_ADD_FDBACK_DIV_M       0x00000080
#define PLLC_ADD_FDBACK_DIV_S       7
#define PLLC_CLKC_DIV_M             0x0001c000
#define PLLC_CLKC_DIV_S             14
#define PLLC_CLKM_DIV_M             0x00700000
#define PLLC_CLKM_DIV_S             20

/* CPU CLK Control fields */
#define CPUCLK_CLK_SEL_M            0x00000003
#define CPUCLK_CLK_SEL_S            0
#define CPUCLK_CLK_DIV_M            0x0000000c
#define CPUCLK_CLK_DIV_S            2




static const int CLOCKCTL1_PREDIVIDE_TABLE[4] = {
    1,
    2,
    4,
    5
};

static const int PLLC_DIVIDE_TABLE[5] = {
    2,
    3,
    4,
    6,
    3
};

static unsigned int 
ar5315_sys_clk(unsigned int clockCtl)
{
    unsigned int pllcCtrl,cpuDiv;
    unsigned int pllcOut,refdiv,fdiv,divby2;
	unsigned int clkDiv;
    HAL_READ_UINT32(AR5315_PLLC_CTL,pllcCtrl);
    refdiv = (pllcCtrl & PLLC_REF_DIV_M) >> PLLC_REF_DIV_S;
    refdiv = CLOCKCTL1_PREDIVIDE_TABLE[refdiv];
    fdiv = (pllcCtrl & PLLC_FDBACK_DIV_M) >> PLLC_FDBACK_DIV_S;
    divby2 = (pllcCtrl & PLLC_ADD_FDBACK_DIV_M) >> PLLC_ADD_FDBACK_DIV_S;
    divby2 += 1;
    pllcOut = (40000000/refdiv)*(2*divby2)*fdiv;


    /* clkm input selected */
	switch(clockCtl & CPUCLK_CLK_SEL_M) {
		case 0:
		case 1:
			clkDiv = PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKM_DIV_M) >> PLLC_CLKM_DIV_S];
			break;
		case 2:
			clkDiv = PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKC_DIV_M) >> PLLC_CLKC_DIV_S];
			break;
		default:
			pllcOut = 40000000;
			clkDiv = 1;
			break;
	}
	cpuDiv = (clockCtl & CPUCLK_CLK_DIV_M) >> CPUCLK_CLK_DIV_S;  
	cpuDiv = cpuDiv * 2 ?: 1;
	return pllcOut/(clkDiv * cpuDiv);
}
		
unsigned int hal_ar2316_cpu_frequency(void)
{
    unsigned int param;
    HAL_READ_UINT32(AR5315_CPUCLK,param);
    return ar5315_sys_clk(sysRegRead(param));
}

unsigned int hal_ar2316_sys_frequency(void)
{
    unsigned int param;	
    HAL_READ_UINT32(AR5315_AMBACLK,param);
    return ar5315_sys_clk(param);
}


static void
hal_ar2316_flash_setup(void)
{
    int ar531x_flash_data_found;

    /*
     * Find start of Radio Configuration data, using heuristics:
     * Search back from the (aliased) end of flash by 0x1000 bytes
     * at a time until we find the string "5311", which marks the
     * start of Board Configuration.  Give up if we've searched
     * more than 500KB.
     */
    ar531x_flash_data_found = 0;
    // eileen , for 8MB flash , 20080626
#if CYGNUM_FLASH_SIZE == 0x400000
	for (ar531x_board_configuration = (char *)0xbfff0000;
	ar531x_board_configuration > (char *)0xbff80000;
#elif CYGNUM_FLASH_SIZE == 0x800000
    for (ar531x_board_configuration = (char *)0xa87f0000;
         ar531x_board_configuration > (char *)0xa8180000;
#else
#Warning! Currently works for only 4MB and 8MB
#endif // CYGNUM_FLASH_SIZE
      	 ar531x_board_configuration -= 0x1000) {
        if ( *(int *)ar531x_board_configuration == AR531X_BD_MAGIC) {
            ar531x_flash_data_found = 1;
            break;
        }
    }

    if (!ar531x_flash_data_found) {
	diag_printf("No board config data found!\n");
        return;
    }

    /* 
     * Now find the start of Board Configuration data, using heuristics:
     * Search forward from Board Configuration data by 0x1000 bytes
     * at a time until we find non-0xffffffff.
     */
    ar531x_flash_data_found = 0;
    for (ar531x_radio_configuration = ar531x_board_configuration + 0xf8;
#if CYGNUM_FLASH_SIZE == 0x400000
        ar531x_radio_configuration < (char *)0xbffff000;
#elif CYGNUM_FLASH_SIZE == 0x800000
	ar531x_radio_configuration < (char *)0xa87ff000;
#else
#Warning! Currently works for only 4MB and 8MB
#endif // CYGNUM_FLASH_SIZE
	ar531x_radio_configuration += 0x1000) {
        if (*(int *)ar531x_radio_configuration != 0xffffffff) {
            ar531x_flash_data_found = 1;
            break;
        }
    }

    if (!ar531x_flash_data_found) {
	diag_printf("No radio config data found!\n");
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
hal_ar2316_reset(void)
{
	void (*mips_reset_vec)(void) = (void *) 0xbfc00000;
    for(;;) {
	HAL_WRITE_UINT32(AR2316_COLD_RESET,AR2317_RESET_SYSTEM);
        udelay(100*1000);
	sysGpioSet(AR2316_RESET_GPIO, 0);
        udelay(100*1000);
	sysGpioSet(0, 0);
        udelay(100*1000);
	mips_reset_vec();
    }
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
