//==========================================================================
//
//      pdk403_eth_init.c
//
//      Ethernet device driver for NS DP83902a ethernet controller
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    dwmw2
// Contributors: 
// Date:         2003-11-17
// Purpose:      
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>

#include <cyg/io/dp83902a.h>

#define _EECLK	0x80
#define _EEO	0x40
#define _EEI	0x20
#define _EECS	0x10

void cyg_ax88796_eth_init(dp83902a_priv_data_t *dp)
{
	int i;
	unsigned char x;
#if defined(CYGSEM_DEVS_ETH_PDK403_ETH0_SET_ESA) 
	cyg_bool esa_ok;
	unsigned char _esa[6];
#endif
    
	/* Reset */
	DP_IN(dp->base, 0x1f, x);

	/* Wait (but not indefinitely) for RST bit in ISR */
	for (i=0; i < 10000; i++) {
		DP_IN(dp->base, DP_ISR, x);
		if (x & DP_ISR_RESET)
			goto ready;
		CYGACC_CALL_IF_DELAY_US(1);
	}
	diag_printf("AX88796 at %p not ready after 10ms. Giving up\n",
		    dp->base);
	dp->base = 0;
	return;

 ready:
	diag_printf("AX88796 at %p, interrupt: %x\n", dp->base, dp->interrupt);

	/* Wait for 2s for link, else power cycle the PHY */
	for (i=0; i<2000; i++) {
		DP_IN(dp->base, 0x17, x);
		if (x&1) {
			diag_printf("Link OK: %dMbps %s duplex (after %d ms)\n",
				    x&4?100:10, x&2?"full":"half", i);
			goto link_ok;
		}
		CYGACC_CALL_IF_DELAY_US(1000);
	}
	diag_printf("No Ethernet link after 2s, power cycle PHY...");

	// Assert PPDSET to turn off internal PHY, wait 2s, turn it back on again.
	DP_OUT(dp->base, 0x17, 0x40);
	CYGACC_CALL_IF_DELAY_US(2000000);
	DP_OUT(dp->base, 0x17, 0x00);
	diag_printf("done.\n");

	// Maybe we're not actually connected. Don't wait.
 link_ok:

#if defined(CYGSEM_DEVS_ETH_PDK403_ETH0_SET_ESA) 
	esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
					     "lan_esa", _esa, CONFIG_ESA);
	if (esa_ok) {
		memcpy(dp->esa, _esa, sizeof(_esa));
	}
#else
	// Send 13 bits of EEPROM read command 0.0110.0000.0000.
	for (i=0; i < 13; i++) {
		x = _EECS;
		if (i == 2 || i == 3)
			x |= _EEI;

		DP_OUT(dp->base, 0x14, x);
		CYGACC_CALL_IF_DELAY_US(10);

		DP_OUT(dp->base, 0x14, x | _EECLK);
		CYGACC_CALL_IF_DELAY_US(10);
	}

	// Read MAC address
	for (i=0; i<6; i++) {
		int j;

		dp->esa[i] = 0;
		for (j=0; j<8; j++) {
			DP_OUT(dp->base, 0x14, _EECS);
			CYGACC_CALL_IF_DELAY_US(10);
			   
			DP_OUT(dp->base, 0x14, _EECS | _EECLK);
			CYGACC_CALL_IF_DELAY_US(10);
			   
			DP_IN(dp->base, 0x14, x);
			   
			dp->esa[i] <<= 1;
			dp->esa[i] |= !!(x & _EEO);
		}
	}
	DP_OUT(dp->base, 0x14, _EECS);
	CYGACC_CALL_IF_DELAY_US(20);
	DP_OUT(dp->base, 0x14, 0);
	CYGACC_CALL_IF_DELAY_US(20);
	if (dp->esa[0] != 0 || dp->esa[1] != 0x0e ||
	    dp->esa[2] != 0 || dp->esa[3] != 0x50) {
		/* Bad MAC address. PDK docs say these four bytes should always
		   be as above */
		diag_printf("Bad EEPROM MAC address %02x:02x:%02x:%02x:%02x:%02x. Disabling AX88796\n",
			    dp->esa[0], dp->esa[1], dp->esa[2], dp->esa[3], dp->esa[4], dp->esa[5]);
		// Turn off PHY and stop chip
		DP_OUT(dp->base, 0x17, 0x40);
		dp->base = 0;
		return;
	}

	// Set ESA into chip
	DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1);  // Select page 1
	for (i = 0; i < 6; i++)
		DP_OUT(dp->base, DP_P1_PAR0+i, dp->esa[i]);

	DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0);  // Select page 0
#endif
}
