//==========================================================================
//
//      devs/eth/frv/frv400/..../include/devs_eth_frv400.inl
//
//      FRV400 ethernet I/O definitions.
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov, hmt, gthomas
// Contributors: jskov
// Date:         2001-02-28
// Purpose:      FRV400 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR
#include <cyg/hal/hal_if.h>
#include <cyg/io/pci.h>

#ifdef __WANT_CONFIG

#define CYGHWR_NS_DP83902A_PLF_RESET(_dp_)      \
    CYG_MACRO_START                             \
    cyg_uint8 _t;                               \
    HAL_READ_UINT8(_dp_->reset, _t);            \
    CYGACC_CALL_IF_DELAY_US(10);                \
    HAL_WRITE_UINT8(_dp_->reset, _t);           \
    CYGACC_CALL_IF_DELAY_US(10000);                \
    CYG_MACRO_END

#define DP_IN(_b_, _o_, _d_)                                    \
    CYG_MACRO_START                                             \
    HAL_READ_UINT8(((cyg_addrword_t)(_b_)+(_o_))^0x03, (_d_));    \
    CYG_MACRO_END

#define DP_OUT(_b_, _o_, _d_)                                   \
    CYG_MACRO_START                                             \
    HAL_WRITE_UINT8(((cyg_addrword_t)(_b_)+(_o_))^0x03, (_d_));   \
    CYG_MACRO_END

#define DP_IN_DATA(_b_, _d_)                                    \
    CYG_MACRO_START                                             \
    HAL_READ_UINT16((cyg_addrword_t)(_b_)^0x02, (_d_));         \
    CYG_MACRO_END

#define DP_OUT_DATA(_b_, _d_)                                   \
    CYG_MACRO_START                                             \
    HAL_WRITE_UINT16((cyg_addrword_t)(_b_)^0x02, (_d_));        \
    CYG_MACRO_END

//#define CYGHWR_NS_DP83902A_PLF_16BIT_DATA
//#define CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA

#endif // __WANT_CONFIG

#ifdef __WANT_DEVS

#if defined(CYGSEM_DEVS_ETH_FRV400_ETH0_SET_ESA) 
#if defined(CYGPKG_REDBOOT) 
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("Network hardware address [MAC]",
                      lan_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif  // CYGSEM_REDBOOT_FLASH_CONFIG
#endif  // CYGPKG_REDBOOT
#include <cyg/hal/hal_if.h>
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif
#endif

static cyg_bool
find_rtl8029_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
    return ((v == 0x10EC) && (d == 0x8029));
}

static void
_frv400_eth_init(dp83902a_priv_data_t *dp)
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
#if defined(CYGSEM_DEVS_ETH_FRV400_ETH0_SET_ESA) 
    cyg_bool esa_ok;
    unsigned char _esa[6];
#else
    unsigned char prom[32];
    int i;
#endif

    devid = CYG_PCI_NULL_DEVID;
    if (cyg_pci_find_matching( &find_rtl8029_match_func, NULL, &devid )) {
        cyg_pci_get_device_info(devid, &dev_info);
        cyg_pci_translate_interrupt(&dev_info, &dp->interrupt);
        dp->base = (cyg_uint8 *)(dev_info.base_map[0] & ~1);
        dp->data = dp->base + 0x10;
        dp->reset = dp->base + 0x1F;
        diag_printf("RTL8029 at %p, interrupt: %x\n", dp->base, dp->interrupt);
#if defined(CYGSEM_DEVS_ETH_FRV400_ETH0_SET_ESA) 
        esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                             "lan_esa", _esa, CONFIG_ESA);
        if (esa_ok) {
            memcpy(dp->esa, _esa, sizeof(_esa));
        }
#else
        // Read ESA from EEPROM
        DP_OUT(dp->base, DP_DCR, 0x48);  // Bytewide access
        DP_OUT(dp->base, DP_RBCH, 0);    // Remote byte count
        DP_OUT(dp->base, DP_RBCL, 0);
        DP_OUT(dp->base, DP_ISR, 0xFF);  // Clear any pending interrupts
        DP_OUT(dp->base, DP_IMR, 0x00);  // Mask all interrupts 
        DP_OUT(dp->base, DP_RCR, 0x20);  // Monitor
        DP_OUT(dp->base, DP_TCR, 0x02);  // loopback
        DP_OUT(dp->base, DP_RBCH, 32);   // Remote byte count
        DP_OUT(dp->base, DP_RBCL, 0);
        DP_OUT(dp->base, DP_RSAL, 0);    // Remote address
        DP_OUT(dp->base, DP_RSAH, 0);
        DP_OUT(dp->base, DP_CR, DP_CR_START|DP_CR_RDMA);  // Read data
        for (i = 0;  i < 32;  i++) {
            cyg_uint16 _val;
            HAL_READ_UINT16(dp->data, _val);
            prom[i] = _val;
        }
        // Set ESA into chip
        DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1);  // Select page 1
        for (i = 0; i < 6; i++) {
            DP_OUT(dp->base, DP_P1_PAR0+i, prom[i]);
        }
        DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0);  // Select page 0
#endif
    }
}

#define CYGHWR_NS_DP83902A_PLF_INIT(dp) _frv400_eth_init(dp)

#ifndef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static void
_frv400_eth_int_clear(dp83902a_priv_data_t *dp)
{
    cyg_drv_interrupt_acknowledge(dp->interrupt);
}

#define CYGHWR_NS_DP83902A_PLF_INT_CLEAR(dp) _frv400_eth_int_clear(dp)
#endif

#ifdef CYGPKG_DEVS_ETH_FRV400_ETH0

static dp83902a_priv_data_t dp83902a_eth0_priv_data = { 
    base : (cyg_uint8*) 0,  //
    data : (cyg_uint8*) 0,  // Filled in at runtime
    reset: (cyg_uint8*) 0,  //
    interrupt: 0,           //
    tx_buf1: 0x40,          // 
    tx_buf2: 0x48,          // Buffer layout - change with care
    rx_buf_start: 0x50,     //
    rx_buf_end: 0x80,       //
#ifdef CYGSEM_DEVS_ETH_FRV400_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_FRV400_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
#endif
};

ETH_DRV_SC(dp83902a_sc,
           &dp83902a_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_FRV400_ETH0_NAME,
           dp83902a_start,
           dp83902a_stop,
           dp83902a_control,
           dp83902a_can_send,
           dp83902a_send,
           dp83902a_recv,
           dp83902a_deliver,     // "pseudoDSR" called from fast net thread
           dp83902a_poll,        // poll function, encapsulates ISR and DSR
           dp83902a_int_vector);

NETDEVTAB_ENTRY(dp83902a_netdev, 
                "dp83902a_" CYGDAT_DEVS_ETH_FRV400_ETH0_NAME,
                dp83902a_init, 
                &dp83902a_sc);


#ifdef CYGPKG_REDBOOT


#define EECS (0x80|(1<<3))
#define EESK (1<<2)
#define EEDI (1<<1)
#define EEDO (1<<0)
#define dprintf(x...) do { } while(0)


static int eeprom_cmd(dp83902a_priv_data_t *dp, int cmd, int cmd_len)
{
	unsigned retval = 0;

        DP_OUT(dp->base, 1, EECS);
	CYGACC_CALL_IF_DELAY_US(2000);

	dprintf("Enabled for %08x (%d bits)\n", cmd, cmd_len);

        DP_OUT(dp->base, 1, EECS | EESK);
	CYGACC_CALL_IF_DELAY_US(2000);

	dprintf("Clock\n");

	/* Shift the command bits out. */
        do {
                short databit = (cmd & (1 << cmd_len)) ? EEDI : 0;
		unsigned char tmp, tmp2;
                DP_OUT(dp->base, 1, EECS | databit); 
		CYGACC_CALL_IF_DELAY_US(2000);
		dprintf("Bit %d ... ", !!(cmd&(1<<cmd_len)));
                DP_OUT(dp->base, 1, EECS | databit | EESK);
		CYGACC_CALL_IF_DELAY_US(2000);
		dprintf(" \b");
		DP_IN(dp->base, 0, tmp2);
		DP_IN(dp->base, 1, tmp);
		CYGACC_CALL_IF_DELAY_US(2000);
                retval = (retval << 1) | !!(tmp & EEDO);
		dprintf(" \b");
		dprintf("Got bit %d (%02x %02x)\n", retval & 1, tmp2, tmp);
        } while (--cmd_len >= 0);
        DP_OUT(dp->base, 1, EECS);
	CYGACC_CALL_IF_DELAY_US(2000);
	dprintf("Last enb...\n");

        /* Terminate the EEPROM access. */
        DP_OUT(dp->base, 1, 0x80);
	CYGACC_CALL_IF_DELAY_US(2000);
	dprintf("Bye. retval %x\n", retval);
        return retval;
}

static void setmac(dp83902a_priv_data_t *dp, unsigned short *newmac)
{
	unsigned char old_cr, old_ee;
	int i;

	DP_IN(dp->base, DP_CR, old_cr);
	DP_OUT(dp->base, DP_CR, old_cr | 0xc0); // Select page 3.
	CYGACC_CALL_IF_DELAY_US(2000);

	DP_IN(dp->base, 1, old_ee);
	DP_OUT(dp->base, 1, 0x80);
	CYGACC_CALL_IF_DELAY_US(2000);

	/* Write enable */
	eeprom_cmd(dp, (4<<6) + (0x30), 8);

	for (i=0; i<3; i++) {
		unsigned adr = i+1;
		/* Erase word */
//		eeprom_cmd(dp, (7<<6) + (adr), 8);
		/* Write word */
		eeprom_cmd(dp, (5<<22) + (adr<<16) + newmac[i], 24);
	}

	/* Write disable */
	eeprom_cmd(dp, (4<<6), 8);

#if 0
	unsigned long words[16];
	for (i=0; i<15; i++) {
		unsigned long cmd = (6<<22) + (i<<16);
		words[i] = eeprom_cmd(dp, cmd /*6<<23 + (i<<16)*/, 24);
	}

	for (i=0; i<15; i++) {
		diag_printf("Words[%d] %08x\n", i, words[i]);
	}
#endif
	DP_OUT(dp->base, 1, old_ee);
	CYGACC_CALL_IF_DELAY_US(2000);
	DP_OUT(dp->base, DP_CR, old_cr);
	CYGACC_CALL_IF_DELAY_US(2000);
}


#include <redboot.h>
static void do_setmac(int argc, char *argv[]);
RedBoot_cmd("setmac", 
            "Set Ethernet MAC address", 
            "<xx:xx:xx:xx:xx:xx>",
            do_setmac
    );

static void 
do_setmac(int argc, char *argv[])
{
	unsigned char *mac = NULL;
	unsigned char mac_nybble[12];
	unsigned short mac_word[3];
	int c, n;

	if (!scan_opts(argc, argv, 1, NULL, 0, (void *)&mac,
		       OPTION_ARG_TYPE_STR, "<MAC address>"))
		return;

	if (!mac) {
		diag_printf("Must supply MAC address\n");
		return;
	}
	for (c=n=0; n<12; c++) {
		if (mac[c] == ':' && !((c+1) % 3))
			/* Colon in an allowed place */
			continue;

		if (mac[c] >= '0' && mac[c] <= '9')
			mac_nybble[n] = mac[c] - '0';
 		else if (mac[c] >= 'A' && mac[c] <= 'F')
			mac_nybble[n] = mac[c] - 'A' + 10;
 		else if (mac[c] >= 'a' && mac[c] <= 'f')
			mac_nybble[n] = mac[c] - 'a' + 10;
		else {
			diag_printf("Invalid MAC address bad char %x\n", mac[c]);
			return;
		}

		n++;
	}
	if (mac[c] || n!=12 || (mac_nybble[1]&1)) {
		diag_printf("Invalid MAC address\n");
		return;
	}
	mac_word[0] = (mac_nybble[2] << 12) |
		      (mac_nybble[3] << 8) |
		      (mac_nybble[0] << 4) |
		      (mac_nybble[1]);
	mac_word[1] = (mac_nybble[6] << 12) |
		      (mac_nybble[7] << 8) |
		      (mac_nybble[4] << 4) |
		      (mac_nybble[5]);
	mac_word[2] = (mac_nybble[10] << 12) |
		      (mac_nybble[11] << 8) |
		      (mac_nybble[8] << 4) |
		      (mac_nybble[9]);

	diag_printf("Setting MAC address...");

	setmac(&dp83902a_eth0_priv_data, mac_word);
	diag_printf("... done. Reset to take effect.\n");
	return;
}
#endif /* CYGPKG_REDBOOT */
#endif // CYGPKG_DEVS_ETH_FRV400_ETH0

#endif // __WANT_DEVS

// --------------------------------------------------------------

// EOF devs_eth_frv400.inl
