//==========================================================================
//
//      ixdp425_misc.c
//
//      HAL misc board support code for Intel XScale IXDP425
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-12-11
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ixp425.h>         // Hardware definitions
#include <cyg/hal/ixdp425.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

//
// Platform specific initialization
//
void
plf_hardware_init(void)
{
    // GPIO(15) used for ENET clock
    HAL_GPIO_OUTPUT_ENABLE(15);
    *IXP425_GPCLKR |= GPCLKR_CLK1_ENABLE;
    *IXP425_GPCLKR |= GPCLKR_CLK1_PCLK2;

    HAL_GPIO_OUTPUT_SET(GPIO_EEPROM_SCL);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SCL);

    HAL_GPIO_OUTPUT_SET(GPIO_EEPROM_SDA);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);

    // ENET-0 IRQ line
    HAL_GPIO_OUTPUT_DISABLE(GPIO_ENET0_INT_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_ETH0, 1, 0);

    // ENET-1 IRQ line
    HAL_GPIO_OUTPUT_DISABLE(GPIO_ENET1_INT_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_ETH1, 1, 0);

    // HSS IRQ lines
    HAL_GPIO_OUTPUT_DISABLE(GPIO_HSS0_INT_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_HSS0, 1, 0);
    HAL_GPIO_OUTPUT_DISABLE(GPIO_HSS1_INT_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_HSS1, 1, 0);

    // DSL IRQ line
    HAL_GPIO_OUTPUT_DISABLE(GPIO_DSL_INT_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_DSL, 1, 0);

    *IXP425_EXP_CS4 = (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
		       EXP_RECOVERY_T(15) | EXP_SZ_512 | EXP_WR_EN | EXP_CS_EN);
    *IXP425_EXP_CS5 = (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
		       EXP_RECOVERY_T(15) | EXP_SZ_512 | EXP_WR_EN | EXP_CS_EN);

#ifdef CYGPKG_IO_PCI
    extern void hal_plf_pci_init(void);
    hal_plf_pci_init();
#endif
}

// --------------------------------------------------------------------------------------
// EEPROM Support 
//
#ifdef CYGPKG_DEVS_ETH_INTEL_NPE

#define CLK_LO()      HAL_GPIO_OUTPUT_CLEAR(GPIO_EEPROM_SCL)
#define CLK_HI()      HAL_GPIO_OUTPUT_SET(GPIO_EEPROM_SCL)

#define DATA_LO()     HAL_GPIO_OUTPUT_CLEAR(GPIO_EEPROM_SDA)
#define DATA_HI()     HAL_GPIO_OUTPUT_SET(GPIO_EEPROM_SDA)


// returns non-zero if ACK bit seen
static int
eeprom_start(cyg_uint8 b)
{
    int i;

    CLK_HI();
    hal_delay_us(5);
    DATA_LO();
    hal_delay_us(5);
    CLK_LO();

    for (i = 7; i >= 0; i--) {
	if (b & (1 << i))
	    DATA_HI();
	else
	    DATA_LO();
	hal_delay_us(5);
	CLK_HI();
	hal_delay_us(5);
	CLK_LO();
    }
    hal_delay_us(5);
    HAL_GPIO_OUTPUT_DISABLE(GPIO_EEPROM_SDA);
    CLK_HI();
    hal_delay_us(5);
    i = (*IXP425_GPINR & (1 << GPIO_EEPROM_SDA)) ? 0 : 1;
    CLK_LO();
    hal_delay_us(5);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);

    return i;
}


static void
eeprom_stop(void)
{
    int i;
    
    hal_delay_us(5);
    DATA_LO();
    hal_delay_us(5);
    CLK_HI();
    hal_delay_us(5);
    DATA_HI();
    hal_delay_us(5);
    CLK_LO();
    hal_delay_us(5);
    CLK_HI();
    hal_delay_us(5);
}


static int
eeprom_putb(cyg_uint8 b)
{
    int i;

    for (i = 7; i >= 0; i--) {
	if (b & (1 << i))
	    DATA_HI();
	else
	    DATA_LO();
	CLK_HI();
	hal_delay_us(5);
	CLK_LO();
	hal_delay_us(5);
    }
    HAL_GPIO_OUTPUT_DISABLE(GPIO_EEPROM_SDA);
    CLK_HI();
    hal_delay_us(5);
    i = (*IXP425_GPINR & (1 << GPIO_EEPROM_SDA)) ? 0 : 1;
    CLK_LO();
    hal_delay_us(5);

    DATA_HI();
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);

    return i;
}


static cyg_uint8
eeprom_getb(int more)
{
    int i;
    cyg_uint8 b = 0;

    HAL_GPIO_OUTPUT_DISABLE(GPIO_EEPROM_SDA);
    hal_delay_us(5);

    for (i = 7; i >= 0; i--) {
	b <<= 1;
	if (*IXP425_GPINR & (1 << GPIO_EEPROM_SDA))
	    b |= 1;
	CLK_HI();
	hal_delay_us(5);
	CLK_LO();
	hal_delay_us(5);
    }
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);
    if (more)
	DATA_LO();
    else
	DATA_HI();
    hal_delay_us(5);
    CLK_HI();
    hal_delay_us(5);
    CLK_LO();
    hal_delay_us(5);

    return b;
}


static int
eeprom_read(int addr, cyg_uint8 *buf, int nbytes)
{
    cyg_uint8 start_byte;
    int i;

    start_byte = 0xA0;  // write

    if (addr & (1 << 8))
	start_byte |= 2;

    
    for (i = 0; i < 10; i++)
	if (eeprom_start(start_byte))
	    break;

    if (i == 10) {
	diag_printf("eeprom_read: Can't get write start ACK\n");
	return 0;
    }

    if (!eeprom_putb(addr & 0xff)) {
	diag_printf("eeprom_read: Can't get address ACK\n");
	return 0;
    }

    start_byte |= 1; // READ command
    if (!eeprom_start(start_byte)) {
	diag_printf("eeprom_read: Can't get read start ACK\n");
	return 0;
    }

    for (i = 0; i < (nbytes - 1); i++)
	*buf++ = eeprom_getb(1);

    *buf++ = eeprom_getb(0);
    hal_delay_us(5);
    eeprom_stop();

    return nbytes;
}

static void
eeprom_write(int addr, cyg_uint8 val)
{
    cyg_uint8 start_byte;
    int i;

    start_byte = 0xA0;  // write

    if (addr & (1 << 8))
	start_byte |= 2;

    for (i = 0; i < 10; i++)
	if (eeprom_start(start_byte))
	    break;

    if (i == 10) {
	diag_printf("eeprom_write: Can't get start ACK\n");
	return;
    }

    if (!eeprom_putb(addr & 0xff)) {
	diag_printf("eeprom_write: Can't get address ACK\n");
	return;
    }

    if (!eeprom_putb(val)) {
	diag_printf("eeprom_write: no data ACK\n");
	return;
    }
    eeprom_stop();
}


#define MAC_EEPROM_OFFSET(p)  (0x100 + ((p) * 6))

int
cyghal_get_npe_esa(int port, cyg_uint8 *buf)
{
    if (port != 0 && port != 1)
	return 0;

    if (eeprom_read(MAC_EEPROM_OFFSET(port), buf, 6) != 6)
	return 0;

    // don't use broadcast address
    if (buf[0] == 0xff && buf[1] == 0xff && buf[2] == 0xff &&
        buf[3] == 0xff && buf[4] == 0xff && buf[5] == 0xff)
	return 0;

    return 1;
}


#ifdef CYGPKG_REDBOOT
#include <redboot.h>

static void
do_set_npe_mac(int argc, char *argv[])
{
    bool portnum_set;
    int  portnum, i;
    char *addr = 0;
    struct option_info opts[1];
    cyg_uint8  mac[6];
    
    init_opts(&opts[0], 'p', true, OPTION_ARG_TYPE_NUM, 
              (void **)&portnum, (bool *)&portnum_set, "port number");
    if (!scan_opts(argc, argv, 1, opts, 1, (void *)&addr,
		   OPTION_ARG_TYPE_STR, "MAC address")) {
        return;
    }

    if ((!portnum_set && addr) ||
	(portnum_set && portnum != 0 && portnum != 1)) {
	diag_printf("Must specify port with \"-p <0|1>\"\n");
	return;
    }

    if (!portnum_set) {
	for (i = 0; i < 2; i++) {
	    cyghal_get_npe_esa(i, mac);
	    diag_printf("NPE eth%d mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
			i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	return;
    }

    if (!addr) {
	cyghal_get_npe_esa(portnum, mac);
	diag_printf("NPE eth%d mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		    portnum, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return;
    }

    // parse MAC address from user.
    // acceptable formats are "nn:nn:nn:nn:nn:nn" and "nnnnnnnnnnnn"
    for (i = 0; i < 6; i++) {
	if (!_is_hex(addr[0]) || !_is_hex(addr[1]))
	    break;
	mac[i] = (_from_hex(addr[0]) * 16) + _from_hex(addr[1]);
	addr += 2;
	if (*addr == ':')
	    addr++;
    }
    
    if (i != 6 || *addr != '\0') {
	diag_printf("Malformed MAC address.\n");
	return;
    }

    for (i = 0; i < 6; i++) {
	eeprom_write(MAC_EEPROM_OFFSET(portnum) + i, mac[i]);
	hal_delay_us(100000);
    }
}

RedBoot_cmd("set_npe_mac", 
            "Set/Read MAC address for NPE ethernet ports", 
            "[-p <portnum>] [xx:xx:xx:xx:xx:xx]",
            do_set_npe_mac);

#endif // CYGPKG_REDBOOT

#endif // CYGPKG_DEVS_ETH_INTEL_NPE

// ------------------------------------------------------------------------
// EOF ixdp425_misc.c
