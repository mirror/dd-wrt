//==========================================================================
//
//      devs_eth_powerpc_ec555.inl
//
//      ec555 ethernet I/O definitions.
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
// Author(s):   Bob Koninckx
// Contributors:Bob Koninckx
// Date:        2002-11-20
// Purpose:     ec555 ethernet defintions
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h> 
#include <cyg/hal/hal_if.h>

#ifdef CYGPKG_REDBOOT
# include <pkgconf/redboot.h>
# ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#  include <redboot.h>
#  include <flash_config.h>
# endif
#endif

#ifdef __WANT_CONFIG
# define CS8900A_step 2
#endif

#ifdef __WANT_DEVS

#ifdef CYGPKG_DEVS_ETH_POWERPC_EC555_ETH0

#ifndef CYGSEM_DEVS_ETH_POWERPC_EC555_ETH0_SET_ESA
# if defined(CYGPKG_REDBOOT) && defined(CYGSEM_REDBOOT_FLASH_CONFIG)
RedBoot_config_option("Set " CYGDAT_DEVS_ETH_POWERPC_EC555_ETH0_NAME " network hardware address [MAC]",
                      eth0_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_BOOL, false
    );
RedBoot_config_option(CYGDAT_DEVS_ETH_POWERPC_EC555_ETH0_NAME " network hardware address [MAC]",
                      eth0_esa_data,
                      "eth0_esa", true,
                      CONFIG_ESA, 0
    );
# endif // CYGPKG_REDBOOT && CYGSEM_REDBOOT_FLASH_CONFIG

# ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
// Note that this section *is* active in an application, outside RedBoot,
// where the above section is not included.

#  include <cyg/hal/hal_if.h>

#  ifndef CONFIG_ESA
#   define CONFIG_ESA (6)
#  endif
#  ifndef CONFIG_BOOL
#   define CONFIG_BOOL (1)
#  endif

cyg_bool
_ec555_provide_eth0_esa(struct cs8900a_priv_data* cpd)
{
    cyg_bool set_esa;
    int ok;
    ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,
                                      "eth0_esa", &set_esa, CONFIG_BOOL);
    if (ok && set_esa) {
        ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,
                                          "eth0_esa_data", cpd->esa, CONFIG_ESA);
    }
    return ok && set_esa;
}

# endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
#endif // ! CYGSEM_DEVS_ETH_POWERPC_EC555_ETH0_SET_ESA

// ------------------------------------------------------------------------
// EEPROM access functions
// These are byte swapped
#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
#define PP_ECR          0x4000    // EEPROM Command Register
#define PP_EE_READ_CMD  0x0002
#define PP_EE_WRITE_CMD 0x0001
#define PP_EE_DATA		0x4200
#else
#define PP_ECR          0x0040    // EEPROM Command Register
#define PP_EE_READ_CMD  0x0200
#define PP_EE_WRITE_CMD 0x0100
#define PP_EE_DATA      0x0042
#endif

#define PP_EE_ADDR_W0   0x001C    // Notice that the EEPROM is not programmed when you got the
#define PP_EE_ADDR_W1   0x001D    // Module from Wuerz. Make sure to program the address to these
#define PP_EE_ADDR_W2   0x001E    // locations before using the adapter. This is fairly easy using GDB

// The example below programs the MAC address 0050 c4ff fc07 to the eeprom, supposed that the module
// is mapped to adresses 0x0400 0000, and supposed that the data bus is _not_ byte swapped
//
// set *(unsigned short *)0x0400030a = 0x0040        Make the eeprom writable / erasable
// set *(unsigned short *)0x0400030c = 0x0030        
//
// set *(unsigned short *)0x0400030a = 0x0040        erase the old esa
// set *(unsigned short *)0x0400030c = 0x031c       
// set *(unsigned short *)0x0400030c = 0x031d       
// set *(unsigned short *)0x0400030c = 0x031e       
// 
// set *(unsigned short *)0x0400030a = 0x0042        program the new esa
// set *(unsigned short *)0x0400030c = 0x5000
// set *(unsigned short *)0x0400030a = 0x0040
// set *(unsigned short *)0x0400030c = 0x011c
// set *(unsigned short *)0x0400030a = 0x0042
// set *(unsigned short *)0x0400030c = 0xffc4
// set *(unsigned short *)0x0400030a = 0x0040
// set *(unsigned short *)0x0400030c = 0x011d
// set *(unsigned short *)0x0400030a = 0x0042
// set *(unsigned short *)0x0400030c = 0x07fc
// set *(unsigned short *)0x0400030a = 0x0040
// set *(unsigned short *)0x0400030c = 0x011e
//
// set *(unsigned short *)0x0400030a = 0x0040        Write protect the eeprom
// set *(unsigned short *)0x0400030c = 0x0030         

static __inline__ cyg_uint16 
read_eeprom(cyg_addrword_t base, cyg_uint16 offset)
{
    while (get_reg(base, PP_SelfStat) & PP_SelfStat_SIBSY)
        ;

#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
    put_reg(base, PP_ECR, (CYG_SWAP16(offset) | PP_EE_READ_CMD));
#else
    put_reg(base, PP_ECR, (offset | PP_EE_READ_CMD));
#endif

    while (get_reg(base, PP_SelfStat) & PP_SelfStat_SIBSY)
        ;

    return get_reg(base, PP_EE_DATA);
}

static __inline__ void 
copy_eeprom(cyg_addrword_t base)
{
    volatile cyg_uint16 esa_word;
    cyg_uint16 i;
    for (i = 0;  i < 6;  i += 2)
    {    // Offset in the eeprom is WORD oriented, in the packetpage BYTE oriented
         esa_word = read_eeprom(base, PP_EE_ADDR_W0 + (i/2));
#ifdef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
         put_reg(base, (PP_IA + CYG_SWAP16(i)), esa_word);
#else
         put_reg(base, (PP_IA + i), esa_word);
#endif
    }
}

// Not so nice, but reading these will never conflict on the ec555
// They certainly differ in A18
#define FIRSTRAM 0x01000000
#define LASTRAM  0x010ffffe

static __inline__ void 
post_reset(cyg_addrword_t base)
{
   cyg_uint16 tmp;

	// The following must toggle Address line 18, connected to SBHE on the CS8900a
	HAL_READ_UINT16( LASTRAM,  tmp);
	HAL_READ_UINT16( FIRSTRAM, tmp);
	HAL_READ_UINT16( LASTRAM,  tmp);
	HAL_READ_UINT16( FIRSTRAM, tmp);
}

#undef  CYGHWR_CL_CS8900A_PLF_POST_RESET
#define CYGHWR_CL_CS8900A_PLF_POST_RESET(base) post_reset(base)

#undef  CYGHWR_CL_CS8900A_PLF_RESET
#define CYGHWR_CL_CS8900A_PLF_RESET(base) copy_eeprom(base)
 
static cs8900a_priv_data_t cs8900a_eth0_priv_data = { 
    base : (cyg_addrword_t) (CYGNUM_DEVS_ETH_POWERPC_EC555_ETH0_BASE + 0x300),
    interrupt:((CYGNUM_DEVS_ETH_POWERPC_EC555_ETH0_IRQ*2)+1), 
#ifdef CYGSEM_DEVS_ETH_POWERPC_EC555_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_POWERPC_EC555_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
# ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    provide_esa : &_ec555_provide_eth0_esa,
# else
    provide_esa : NULL,
# endif
#endif

};


ETH_DRV_SC(cs8900a_sc,
           &cs8900a_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_POWERPC_EC555_ETH0_NAME,
           cs8900a_start,
           cs8900a_stop,
           cs8900a_control,
           cs8900a_can_send,
           cs8900a_send,
           cs8900a_recv,
           cs8900a_deliver,     // "pseudoDSR" called from fast net thread
           cs8900a_poll,        // poll function, encapsulates ISR and DSR
           cs8900a_int_vector);

NETDEVTAB_ENTRY(cs8900a_netdev, 
                "cs8900a_" CYGDAT_DEVS_ETH_POWERPC_EC555_ETH0_NAME,
                cs8900a_init, 
                &cs8900a_sc);

#endif // CYGPKG_DEVS_ETH_POWERPC_EC555_ETH0

#endif // __WANT_DEVS

// EOF devs_eth_powerpc_ec555.inl
