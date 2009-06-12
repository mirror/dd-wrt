//==========================================================================
//
//      if_i82559.c
//
//	Intel 82559 ethernet driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Portions of this software may have been derived from OpenBSD or
// other sources, and are covered by the appropriate copyright
// disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt, gthomas
// Contributors: Ron Spence, Pacific Softworks, jskov
// Date:         2000-02-01
// Purpose:      
// Description:  hardware driver for 82559 Intel PRO/100+ ethernet
// Notes:        CU commands such as dump and config should, according
//               to the docs, set the CU active state while executing.
//               That does not seem to be the case though, and the
//               driver polls the completion bit in the packet status
//               word instead.
//
//               Platform code may provide this vector:
//               CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT if it
//               requires the interrupts to be handled via demuxers
//               attached to a distinct interrupt.
//
//               Platform code may alternatively define:
//               CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL if it is necessary
//               to demux all interrupt sources - for example if they are
//               wire-or'd together on some hardware but distinct on
//               others.  In this circumstance it is permitted for
//               cyg_pci_translate_interrupt [HAL_PCI_TRANSLATE_INTERRUPT]
//               to return invalid for 2nd and subsequent devices.
//
//               Platform code can also define these three:
//               CYGPRI_DEVS_ETH_INTEL_I82559_MASK_INTERRUPTS(p_i82559,old)
//               CYGPRI_DEVS_ETH_INTEL_I82559_UNMASK_INTERRUPTS(p_i82559,old)
//               CYGPRI_DEVS_ETH_INTEL_I82559_ACK_INTERRUPTS(p_i82559)
//               which are particularly useful when nested interrupt
//               management is needed (which is always IMHO).
//
//               Platform code can define this:
//               CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559)
//               to detect a dropped interrupt and loop again or
//               direct-call the DSR to reschedule the delivery routine.
//               Only a problem on edge-triggered interrupt systems.
//
//               Platform code can also provide this macro:
//               CYGPRI_DEVS_ETH_INTEL_I82559_INTERRUPT_ACK_LOOP(p_i82559)
//               to handle delaying for acks to register on the interrupt
//               controller as necessary on the EBSA.
//
//               Platform can define CYGHWR_DEVS_ETH_INTEL_I82559_GET_ESA()
//               as an external means to get ESAs, possibly from RedBoot
//               configuration info that's stored in flash memory.
//
//               Platform def CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
//               removes all code for dealing with the EEPROM for those
//               targets where there is none fitted.  Either an external
//               means to get ESAs should be used, or we must rely on
//               hard-wiring the ESA's into each executable by means of the
//               usual CDL configuration.
//
//               Platform def CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
//               is for hardware with multiple devices, but only one with a
//               serial EEPROM installed.  The 2nd device would get either
//               the same ESA - because they are certain to be on different
//               segment and internets - or the same ESA incremented by
//               CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_MAC_ADJUST.
//               CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM should be the
//               number (0 or 1) of the device that does have the EEPROM.
//
//               CYGHWR_DEVS_ETH_INTEL_I82559_PCIMEM_DISCONTIGUOUS enables
//               checking code for breaks in the physical address of PCI
//               window memory.  This can happen on some boards where a
//               smaller SDRAM is fitted than the hardware allows, so some
//               higher-order address bits are ignored.  We make SDRAM
//               contiguous in mapped memory, but what the i82559 sees
//               might be discontiguous.  The checking code skips any
//               allocated chunk who appears to contain such a break, and
//               tries again.
//
//               CYGHWR_DEVS_ETH_INTEL_I82559_RESET_TIMEOUT( int32 )
//               CYGHWR_DEVS_ETH_INTEL_I82559_TIMEOUT_FIRED( int32 ) if
//               both defined give the driver a means to detect that we
//               have been fixated on the same transmit operation for too
//               long - we missed an interrupt or the device crashed.  The
//               int32 argument is used to hold a eg. the value of a
//               fast-running hardware timer.
//
//               CYGHWR_DEVS_ETH_INTEL_I82559_ENDIAN_NEUTRAL_IO if PCI IO
//               access is not affected by CPU endianess.
//
//        FIXME: replace -1/-2 return values with proper E-defines
//        FIXME: For 82557/8 compatibility i82559_configure() function
//               probably needs some tweaking - config bits differ
//               slightly but crucially.
//        FIXME: EEPROM code not tested on a BE system.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_intel_i82559.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
// So we can check the validity of the PCI window against the MLTs opinion,
// and thereby what the malloc heap consumes willy-nilly:
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif

// Exported statistics and the like
#include <cyg/devs/eth/i82559_info.h>
#include <cyg/io/eth/eth_drv_stats.h>
#include CYGDAT_DEVS_ETH_INTEL_I82559_INL

#include <cyg/hal/hal_if.h>

// Use with care!  Local variable defined!
#define START_CONSOLE()                                                                 \
{   /* NEW BLOCK */                                                                     \
    int _cur_console =                                                                  \
        CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);      \
    {                                                                                   \
        int i;                                                                          \
        if ( CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,                 \
                                          "info_console_force", &i,                     \
                                          CYGNUM_FLASH_CFG_TYPE_CONFIG_BOOL ) ) {       \
            if ( i ) {                                                                  \
                if ( CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,         \
                                                  "info_console_number", &i,            \
                                                  CYGNUM_FLASH_CFG_TYPE_CONFIG_INT ) ){ \
                    /* Then i is the console to force it to: */                         \
                    CYGACC_CALL_IF_SET_CONSOLE_COMM( i );                               \
                }                                                                       \
            }                                                                           \
        }                                                                               \
    }

#define END_CONSOLE()                                   \
    CYGACC_CALL_IF_SET_CONSOLE_COMM(_cur_console);      \
}   /* END BLOCK */

void CheckRxRing(struct i82559* p_i82559, char * func, int line);

// ------------------------------------------------------------------------
// Check on the environment.
// 
// These are not CDL type config points; they are set up for your platform
// in the platform driver's include file and that's that.  These messages
// are for the eCos driver writer, not config tool users.

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
#error Both a separate demux interrupt *and* DEMUX_ALL are defined
#endif
#endif

#ifdef CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
#error This platform has no EEPROM, yet WRITE_EEPROM is enabled
#endif
#endif

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
#error This platform has no EEPROM, yet it also HAS_ONE_EEPROM
#endif
#endif

// ------------------------------------------------------------------------

#ifdef CYGDBG_DEVS_ETH_INTEL_I82559_CHATTER
#define DEBUG          // Startup printing mainly
#define DEBUG_EE       // Some EEPROM specific retries &c
#if (CYGDBG_DEVS_ETH_INTEL_I82559_CHATTER > 1)
#define DEBUG_82559 // This one prints stuff as packets come and go
#endif
#endif

#ifdef CYGDBG_USE_ASSERTS
static struct {
    int can_send;
    int deliver;
    int stats;
    int waitcmd_timeouts;
    int waitcmd_timeouts_cu;
    int lockup_timeouts;
    int bad_cu_idles;
} missed_interrupt = { 0,0,0, 0,0, 0, 0 };
#endif

int 
console_printf(const char *fmt, ...)
{
    va_list ap;
    int ret;
    
    START_CONSOLE();

    va_start(ap, fmt);
    ret = diag_vprintf(fmt, ap);
    va_end(ap);
    
    END_CONSOLE();
    return (ret);
}

#define os_printf console_printf
#define db_printf console_printf

// ------------------------------------------------------------------------
//
//                             MEMORY ADDRESSING
// 
// There is scope for confusion here; we deal with LE/BE systems and
// addressing issues in two separate ways depending on the type of access
// in question.
//
// 1) IO-style access to the device regsiters over the PCI bus
// 2) Memory access to build and read the structures in shared memory
// 
// In detail:
// 
// 1) IO-style access to the device regsiters over the PCI bus
// 
// All such access is via macros which perform byte-swapping as necessary
// for the endianness of the CPU.  These macros are called INL, INW, INB
// and OUTL, OUTW, OUTB - for Long (32) Word (16) and Byte (8).  Intel
// nomenclature seems to have crept in here for shorts.
// 
// Consequently, all the constants only have to be expressed once, in their
// true LE format - bit 15 is 0x8000, bit 0 is 1.
// 
// All the offsets are also only expressed once.  This is OK so long as GIB
// endian addressing (sic, see below) is not employed - or so long as is
// does not affect the PCI bus accesses.
//
// 
// 2) Memory access to build and read the structures in shared memory
// 
// These accesses are by means of peek and poke to an address created from
// a base + offset.  No swapping occurs within the access so all constants
// and flags need to be defined twice, once for BE and once for LE
// accesses.  Worse, for BE, constants need to be defined in flavours for
// 16-bit versus 32-bit accesses, ie. 0x8000 sets bit 7 only in BE; for a
// 32-bit access you must instead write 0x80000000 to set bit 7.
//
// Thus all constants are defined twice depending on the CPU's endianness.
//
// For most BE/LE machines, this is sufficient; the layout of memory is the
// same.  Specifically, within a 32-bit word, byte[0] will be data[0:7],
// short[0] will be data [0:15] and so on.  &byte[0] == &short[0] == &word
// regardless.  But data[0:7] *OF THE MEMORY SYSTEM* will hold either the
// LSbyte (0xFF) on a LE machine, and the MSbyte (0xFF000000) on a BE
// machine, for a 32-bit access.
// 
// It is in terms of the memory system that the i82559 defines its view of
// the world.
// 
// Therefore the structure layouts remain the same for both BE and LE
// machines.  This means that the offsets for, specifically, the status
// word in command blocks is always zero, and the offset for the command
// word is always two.
//
// But there is one furter variant: so-called GIB endian.  (BIG endian
// backwards) Some architectures support BE only for software
// compatibility; they allow code to run which relies on overlaid C
// structures behaving in a certain way; specifically
//     *(char *)&i == (i >> 24)
// ARM's BE mode is an example of this.
// 
// But while such an operation would use data[0:7] for the char access in a
// true BE or any LE system, in a GE system, data[24:31] are used here.
// The implementation is that for memory accesses, A0 and A1 are inverted
// before going outside to the memory system.  So if &i == 0x1000,
// accessing i uses address 0x1000, A0 and A1 being ignored for a 32-bit
// access.  But the 8-bit access to *(char *)&i also uses 0x1000 for the
// address as the code sees it, the memory system sees a byte request for
// address 0x1003, thus picking up the MSbyte, from data[24:31].
//
// For such addressing, offsets need to be redefined to swap bytes and
// shorts within words.  Therefore offsets are defined twice, once for
// "normal" addressing, and once for "GIB endian" addressing.
//
// FIXME: this BE/GE configuration probably won't work with an ARM in its
// BE mode - because that will define the global BE flags, yet it's not
// true BE, it's GE.
// Perhaps a solution whereby the GE flag CYG_ADDRESSING_IS_GIBENDIAN
// overrides the BYTEORDER choices would be good; we want the constants to
// be LE, but address offsets to be swapped per GE.
//
// Essay ends.
//
// ------------------------------------------------------------------------
// I/O access macros as inlines for type safety

#if (CYG_BYTEORDER == CYG_MSBFIRST)

#define HAL_CTOLE32(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))
#define HAL_LE32TOC(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

#define HAL_CTOLE16(x)  ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define HAL_LE16TOC(x)  ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))

#else
// Maintaining the same styleee as above...
#define HAL_CTOLE32(x)  ((((x))))
#define HAL_LE32TOC(x)  ((((x))))

#define HAL_CTOLE16(x)  ((((x))))
#define HAL_LE16TOC(x)  ((((x))))

#endif


#if (CYG_BYTEORDER == CYG_MSBFIRST) && !defined(CYGHWR_DEVS_ETH_INTEL_I82559_ENDIAN_NEUTRAL_IO)

static inline void OUTB(cyg_uint8 value, cyg_uint32 io_address)
{
    HAL_WRITE_UINT8( io_address, value);
}

static inline void OUTW(cyg_uint16 value, cyg_uint32 io_address)
{
    HAL_WRITE_UINT16( io_address, (((value & 0xff) << 8) | ((value & 0xff00) >> 8)) );
}

static inline void OUTL(cyg_uint32 value, cyg_uint32 io_address)
{
    HAL_WRITE_UINT32( io_address,
                      ((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) & 0xff0000) >> 8) | (((value) >> 24) & 0xff)) );
}

static inline cyg_uint8 INB(cyg_uint32 io_address)
{   
    cyg_uint8 d;
    HAL_READ_UINT8( io_address, d );
    return d;
}

static inline cyg_uint16 INW(cyg_uint32 io_address)
{
    cyg_uint16 d;
    HAL_READ_UINT16( io_address, d );
    return (((d & 0xff) << 8) | ((d & 0xff00) >> 8));
}

static inline cyg_uint32 INL(cyg_uint32 io_address)
{
    cyg_uint32 d;
    HAL_READ_UINT32( io_address, d );
    return ((((d) & 0xff) << 24) | (((d) & 0xff00) << 8) | (((d) & 0xff0000) >> 8) | (((d) >> 24) & 0xff));
}
#else

static inline void OUTB(cyg_uint8  value, cyg_uint32 io_address)
{   HAL_WRITE_UINT8( io_address, value );   }

static inline void OUTW(cyg_uint16 value, cyg_uint32 io_address)
{   HAL_WRITE_UINT16( io_address, value );   }

static inline void OUTL(cyg_uint32 value, cyg_uint32 io_address)
{   HAL_WRITE_UINT32( io_address, value );   }

static inline cyg_uint8  INB(cyg_uint32 io_address)
 {   cyg_uint8  _t_; HAL_READ_UINT8(  io_address, _t_ ); return _t_;   }

static inline cyg_uint16 INW(cyg_uint32 io_address)
 {   cyg_uint16 _t_; HAL_READ_UINT16( io_address, _t_ ); return _t_;   }

static inline cyg_uint32 INL(cyg_uint32 io_address)
 {   cyg_uint32 _t_; HAL_READ_UINT32( io_address, _t_ ); return _t_;   }

#endif // byteorder

// ------------------------------------------------------------------------
// Macros for writing shared memory structures - no need for byte flipping

#define READMEM8(   _reg_, _val_ ) ((_val_) = *((volatile CYG_BYTE *)(_reg_)))
#define WRITEMEM8(  _reg_, _val_ ) (*((volatile CYG_BYTE *)(_reg_)) = (_val_))
#define READMEM16(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define WRITEMEM16( _reg_, _val_ ) (*((volatile CYG_WORD16 *)(_reg_)) = (_val_))
#define READMEM32(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD32 *)(_reg_)))
#define WRITEMEM32( _reg_, _val_ ) (*((volatile CYG_WORD32 *)(_reg_)) = (_val_))

// ------------------------------------------------------------------------
// Map from CPU-view addresses to PCI-bus master's view - however that is:

#ifdef CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS

#define VIRT_TO_BUS( _x_ ) CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS( _x_ )

#else // use default mappings: get a physical address to give to the device

#define VIRT_TO_BUS( _x_ ) virt_to_bus((cyg_uint32)(_x_))
static inline cyg_uint32 virt_to_bus(cyg_uint32 p_memory)
{ return CYGARC_PHYSICAL_ADDRESS(p_memory);    }

#endif // not defined CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS

// ------------------------------------------------------------------------
//                                                                      
//                   82559 REGISTER OFFSETS (I/O SPACE)                 
//                                                                      
// ------------------------------------------------------------------------
#define SCBStatus       0               // Rx/Command Unit Status *Word*
#define SCBIntAckByte   1               // Rx/Command Unit STAT/ACK byte
#define SCBCmd          2               // Rx/Command Unit Command *Word*
#define SCBIntrCtlByte  3               // Rx/Command Unit Intr.Control Byte
#define SCBPointer      4               // General purpose pointer.
#define SCBPort         8               // Misc. commands and operands.
#define SCBflash        12              // Flash memory control.
#define SCBeeprom       14              // EEPROM memory control.
#define SCBCtrlMDI      16              // MDI interface control.
#define SCBEarlyRx      20              // Early receive byte count.
#define SCBGenControl   28              // 82559 General Control Register
#define SCBGenStatus    29              // 82559 General Status register


// ------------------------------------------------------------------------
//
//               82559 SCB STATUS WORD DEFNITIONS
//
// ------------------------------------------------------------------------
#define SCB_STATUS_CX   0x8000          // CU finished command (transmit)
#define SCB_STATUS_FR   0x4000          // frame received
#define SCB_STATUS_CNA  0x2000          // CU left active state
#define SCB_STATUS_RNR  0x1000          // receiver left ready state
#define SCB_STATUS_MDI  0x0800          // MDI read/write cycle done
#define SCB_STATUS_SWI  0x0400          // software generated interrupt
#define SCB_STATUS_FCP  0x0100          // flow control pause interrupt

#define SCB_INTACK_MASK 0xFD00          // all the above
#define SCB_INTACK_MASK_BYTE 0xFD       // all the above

#define SCB_INTACK_TX (SCB_STATUS_CX | SCB_STATUS_CNA)
#define SCB_INTACK_RX (SCB_STATUS_FR | SCB_STATUS_RNR)

// ------------------------------------------------------------------------
//
//               SYSTEM CONTROL BLOCK COMMANDS
//
// ------------------------------------------------------------------------
// CU COMMANDS
#define CU_NOP          0x0000
#define	CU_START        0x0010
#define	CU_RESUME       0x0020
#define	CU_STATSADDR    0x0040          // Load Dump Statistics ctrs addr
#define	CU_SHOWSTATS    0x0050          // Dump statistics counters.
#define	CU_ADDR_LOAD    0x0060          // Base address to add to CU commands
#define	CU_DUMPSTATS    0x0070          // Dump then reset stats counters.

// RUC COMMANDS
#define RUC_NOP         0x0000
#define	RUC_START       0x0001
#define	RUC_RESUME      0x0002
#define RUC_ABORT       0x0004
#define	RUC_ADDR_LOAD   0x0006          // (seems not to clear on acceptance)
#define RUC_RESUMENR    0x0007

#define CU_CMD_MASK     0x00f0
#define RU_CMD_MASK     0x0007

#define SCB_M	        0x0100          // 0 = enable interrupt, 1 = disable
#define SCB_SWI         0x0200          // 1 - cause device to interrupt
#define SCB_BYTE_M	  0x01          // 0 = enable interrupt, 1 = disable
#define SCB_BYTE_SWI      0x02          // 1 - cause device to interrupt

#define CU_STATUS_MASK  0x00C0
#define RU_STATUS_MASK  0x003C

#define RU_STATUS_IDLE  (0<<2)
#define RU_STATUS_SUS   (1<<2)
#define RU_STATUS_NORES (2<<2)
#define RU_STATUS_READY (4<<2)
#define RU_STATUS_NO_RBDS_SUS   ((1<<2)|(8<<2))
#define RU_STATUS_NO_RBDS_NORES ((2<<2)|(8<<2))
#define RU_STATUS_NO_RBDS_READY ((4<<2)|(8<<2))

#define MAX_MEM_RESERVED_IOCTL 1000

// ------------------------------------------------------------------------
//
//               82559 PORT INTERFACE COMMANDS
//
// ------------------------------------------------------------------------
#define I82559_RESET            0x00000000 // software reset
#define I82559_SELFTEST         0x00000001 // 82559 selftest command
#define I82559_SELECTIVE_RESET  0x00000002
#define I82559_DUMP             0x00000003
#define I82559_DUMP_WAKEUP      0x00000007


// ------------------------------------------------------------------------
//
//                   82559 EEPROM INTERFACE
//
// ------------------------------------------------------------------------
//  EEPROM_Ctrl bits.
#define EE_SHIFT_CLK	0x01            // EEPROM shift clock.
#define EE_CS		0x02            // EEPROM chip select.
#define EE_DATA_WRITE	0x04            // EEPROM chip data in.
#define EE_DATA_READ	0x08            // EEPROM chip data out.
#define EE_ENB		(0x4800 | EE_CS)

// Delay between EEPROM clock transitions.
#define eeprom_delay(usec)		udelay(usec);

// The EEPROM commands include the always-set leading bit.
#define EE_WRITE_CMD(a)     (5 << (a))
#define EE_READ_CMD(a)	    (6 << (a))
#define EE_ERASE_CMD(a)	    (7 << (a))
#define EE_WRITE_EN_CMD(a)  (19 << ((a)-2))
#define EE_WRITE_DIS_CMD(a) (16 << ((a)-2))
#define EE_ERASE_ALL_CMD(a) (18 << ((a)-2))

#define EE_TOP_CMD_BIT(a)      ((a)+2) // Counts down to zero
#define EE_TOP_DATA_BIT        (15)    // Counts down to zero

#define EEPROM_ENABLE_DELAY (10) // Delay at chip select

#define EEPROM_SK_DELAY  (4) // Delay between clock edges *and* data
                             // read or transition; 3 of these per bit.
#define EEPROM_DONE_DELAY (100) // Delay when all done


// ------------------------------------------------------------------------
//
//               RECEIVE FRAME DESCRIPTORS
//
// ------------------------------------------------------------------------

// The status is split into two shorts to get atomic access to the EL bit;
// the upper word is not written by the device, so we can just hit it,
// leaving the lower word (which the device updates) alone.  Otherwise
// there's a race condition between software moving the end-of-list (EL)
// bit round and the device writing into the previous slot.

#if (CYG_BYTEORDER == CYG_MSBFIRST)

// Note that status bits are endian converted - so we don't need to
// fiddle the byteorder when accessing the status word!
#define RFD_STATUS_EL   0x00000080      // 1=last RFD in RFA
#define RFD_STATUS_S    0x00000040      // 1=suspend RU after receiving frame
#define RFD_STATUS_H    0x00001000      // 1=RFD is a header RFD
#define RFD_STATUS_SF   0x00000800      // 0=simplified, 1=flexible mode
#define RFD_STATUS_C    0x00800000      // completion of received frame
#define RFD_STATUS_OK   0x00200000      // frame received with no errors

#define RFD_STATUS_HI_EL   0x0080       // 1=last RFD in RFA
#define RFD_STATUS_HI_S    0x0040       // 1=suspend RU after receiving frame
#define RFD_STATUS_HI_H    0x1000       // 1=RFD is a header RFD
#define RFD_STATUS_HI_SF   0x0800       // 0=simplified, 1=flexible mode

#define RFD_STATUS_LO_C    0x0080       // completion of received frame
#define RFD_STATUS_LO_OK   0x0020       // frame received with no errors


#define RFD_COUNT_MASK     0x3fff
#define RFD_COUNT_F        0x4000
#define RFD_COUNT_EOF      0x8000

#define RFD_RX_CRC          0x00080000  // crc error
#define RFD_RX_ALIGNMENT    0x00040000  // alignment error
#define RFD_RX_RESOURCE     0x00020000  // out of space, no resources
#define RFD_RX_DMA_OVER     0x00010000  // DMA overrun
#define RFD_RX_SHORT        0x80000000  // short frame error
#define RFD_RX_LENGTH       0x20000000  //
#define RFD_RX_ERROR        0x10000000  // receive error
#define RFD_RX_NO_ADR_MATCH 0x04000000  // no address match
#define RFD_RX_IA_MATCH     0x02000000  // individual address does not match
#define RFD_RX_TCO          0x01000000  // TCO indication

#else // Little-endian

#define RFD_STATUS_EL   0x80000000      // 1=last RFD in RFA
#define RFD_STATUS_S    0x40000000      // 1=suspend RU after receiving frame
#define RFD_STATUS_H    0x00100000      // 1=RFD is a header RFD
#define RFD_STATUS_SF   0x00080000      // 0=simplified, 1=flexible mode
#define RFD_STATUS_C    0x00008000      // completion of received frame
#define RFD_STATUS_OK   0x00002000      // frame received with no errors

#define RFD_STATUS_HI_EL   0x8000       // 1=last RFD in RFA
#define RFD_STATUS_HI_S    0x4000       // 1=suspend RU after receiving frame
#define RFD_STATUS_HI_H    0x0010       // 1=RFD is a header RFD
#define RFD_STATUS_HI_SF   0x0008       // 0=simplified, 1=flexible mode

#define RFD_STATUS_LO_C    0x8000       // completion of received frame
#define RFD_STATUS_LO_OK   0x2000       // frame received with no errors

#define RFD_COUNT_MASK     0x3fff
#define RFD_COUNT_F        0x4000
#define RFD_COUNT_EOF      0x8000

#define RFD_RX_CRC          0x00000800  // crc error
#define RFD_RX_ALIGNMENT    0x00000400  // alignment error
#define RFD_RX_RESOURCE     0x00000200  // out of space, no resources
#define RFD_RX_DMA_OVER     0x00000100  // DMA overrun
#define RFD_RX_SHORT        0x00000080  // short frame error
#define RFD_RX_LENGTH       0x00000020  //
#define RFD_RX_ERROR        0x00000010  // receive error
#define RFD_RX_NO_ADR_MATCH 0x00000004  // no address match
#define RFD_RX_IA_MATCH     0x00000002  // individual address does not match
#define RFD_RX_TCO          0x00000001  // TCO indication

#endif // CYG_BYTEORDER

#ifndef CYG_ADDRESSING_IS_GIBENDIAN

// Normal addressing
#define RFD_STATUS       0
#define RFD_STATUS_LO    0
#define RFD_STATUS_HI    2
#define RFD_LINK         4
#define RFD_RDB_ADDR     8
#define RFD_COUNT       12
#define RFD_SIZE        14
#define RFD_BUFFER      16
#define RFD_SIZEOF      16

#else // CYG_ADDRESSING_IS_GIBENDIAN

// GIBENDIAN addressing; A0 and A1 are flipped:
#define RFD_STATUS       0
#define RFD_STATUS_LO    2              // swapped
#define RFD_STATUS_HI    0              // swapped
#define RFD_LINK         4
#define RFD_RDB_ADDR     8
#define RFD_COUNT       14              // swapped
#define RFD_SIZE        12              // swapped
#define RFD_BUFFER      16
#define RFD_SIZEOF      16

#endif // CYG_ADDRESSING_IS_GIBENDIAN



// ------------------------------------------------------------------------
//
//               TRANSMIT FRAME DESCRIPTORS
//
// ------------------------------------------------------------------------

#if (CYG_BYTEORDER == CYG_MSBFIRST)

// Note that CMD/STATUS bits are endian converted - so we don't need
// to fiddle the byteorder when accessing the CMD/STATUS word!

#define TxCB_CMD_TRANSMIT   0x0400      // transmit command
#define TxCB_CMD_SF         0x0800      // 0=simplified, 1=flexible mode
#define TxCB_CMD_NC         0x0010      // 0=CRC insert by controller
#define TxCB_CMD_I          0x0020      // generate interrupt on completion
#define TxCB_CMD_S          0x0040      // suspend on completion
#define TxCB_CMD_EL         0x0080      // last command block in CBL

#define TxCB_COUNT_MASK     0x3fff
#define TxCB_COUNT_EOF      0x8000

#else // Little-endian layout

#define TxCB_COUNT_MASK     0x3fff
#define TxCB_COUNT_EOF      0x8000

#define TxCB_CMD_TRANSMIT   0x0004      // transmit command
#define TxCB_CMD_SF         0x0008      // 0=simplified, 1=flexible mode
#define TxCB_CMD_NC         0x0010      // 0=CRC insert by controller
#define TxCB_CMD_I          0x2000      // generate interrupt on completion
#define TxCB_CMD_S          0x4000      // suspend on completion
#define TxCB_CMD_EL         0x8000      // last command block in CBL

#endif // CYG_BYTEORDER

#ifndef CYG_ADDRESSING_IS_GIBENDIAN

// Normal addressing
#define TxCB_STATUS          0
#define TxCB_CMD             2
#define TxCB_LINK            4
#define TxCB_TBD_ADDR        8
#define TxCB_COUNT          12
#define TxCB_TX_THRESHOLD   14
#define TxCB_TBD_NUMBER     15
#define TxCB_BUFFER         16
#define TxCB_SIZEOF         16

#else // CYG_ADDRESSING_IS_GIBENDIAN

// GIBENDIAN addressing; A0 and A1 are flipped:
#define TxCB_STATUS          2          // swapped
#define TxCB_CMD             0          // swapped
#define TxCB_LINK            4
#define TxCB_TBD_ADDR        8
#define TxCB_COUNT          14          // swapped
#define TxCB_TX_THRESHOLD   13          // swapped
#define TxCB_TBD_NUMBER     12          // swapped
#define TxCB_BUFFER         16
#define TxCB_SIZEOF         16
#endif // CYG_ADDRESSING_IS_GIBENDIAN

// ------------------------------------------------------------------------
//
//                   STRUCTURES ADDED FOR PROMISCUOUS MODE
//
// ------------------------------------------------------------------------

#if (CYG_BYTEORDER == CYG_MSBFIRST)

// Note CFG CMD and STATUS swapped, so no need for endian conversion
// in code.
#define CFG_CMD_EL         0x0080
#define CFG_CMD_SUSPEND    0x0040
#define CFG_CMD_INT        0x0020
#define CFG_CMD_IAS        0x0100       // individual address setup
#define CFG_CMD_CONFIGURE  0x0200       // configure
#define CFG_CMD_MULTICAST  0x0300       // Multicast-Setup

#define CFG_STATUS_C       0x0080
#define CFG_STATUS_OK      0x0020

#else // Little-endian 

#define CFG_CMD_EL         0x8000
#define CFG_CMD_SUSPEND    0x4000
#define CFG_CMD_INT        0x2000
#define CFG_CMD_IAS        0x0001       // individual address setup
#define CFG_CMD_CONFIGURE  0x0002       // configure
#define CFG_CMD_MULTICAST  0x0003       // Multicast-Setup

#define CFG_STATUS_C       0x8000
#define CFG_STATUS_OK      0x2000

#endif  // CYG_BYTEORDER

#ifndef CYG_ADDRESSING_IS_GIBENDIAN

// Normal addressing
#define CFG_STATUS          0
#define CFG_CMD             2
#define CFG_CB_LINK_OFFSET  4
#define CFG_BYTES           8
#define CFG_SIZEOF          32

#else // CYG_ADDRESSING_IS_GIBENDIAN

// GIBENDIAN addressing; A0 and A1 are flipped:
#define CFG_STATUS          2
#define CFG_CMD             0
#define CFG_CB_LINK_OFFSET  4
#define CFG_BYTES           8
#define CFG_SIZEOF          32

#endif // CYG_ADDRESSING_IS_GIBENDIAN

// Normal addressing
#define CFG_MC_LIST_BYTES 8
#define CFG_MC_LIST_DATA 10

// ------------------------------------------------------------------------
//
//                       STATISTICAL COUNTER STRUCTURE
//
// ------------------------------------------------------------------------
#ifdef KEEP_STATISTICS
STATISTICS statistics[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT];
I82559_COUNTERS i82559_counters[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT];
#endif // KEEP_STATISTICS

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------

#define MAX_RX_PACKET_SIZE  1536        // maximum Rx packet size
#define MAX_TX_PACKET_SIZE  1536        // maximum Tx packet size


// ------------------------------------------------------------------------
// Use arrays provided by platform header to verify pointers.
#ifdef CYGDBG_USE_ASSERTS
#define CHECK_NDP_SC_LINK()                                             \
    CYG_MACRO_START                                                     \
    int i, valid_netdev = 0, valid_sc = 0;                              \
    for(i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {       \
        if (i82559_netdev_array[i] == ndp) valid_netdev = 1;            \
        if (i82559_sc_array[i] == sc) valid_sc = 1;                     \
        if (valid_sc || valid_netdev) break;                            \
    }                                                                   \
    CYG_ASSERT( valid_netdev, "Bad ndp" );                              \
    CYG_ASSERT( valid_sc, "Bad sc" );                                   \
    CYG_ASSERT( (void *)p_i82559 == i82559_sc_array[i]->driver_private, \
                "sc pointer bad" );                                     \
    CYG_MACRO_END
#else
#define CHECK_NDP_SC_LINK()
#endif

#define IF_BAD_82559( _p_ )                                             \
if (({                                                                  \
    int i, valid_p = 0;                                                 \
    for(i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {       \
        if (i82559_priv_array[i] == (_p_)) {                            \
            valid_p = 1;                                                \
            break;                                                      \
        }                                                               \
    }                                                                   \
    CYG_ASSERT(valid_p, "Bad pointer-to-i82559");                       \
    (!valid_p);                                                         \
}))

// ------------------------------------------------------------------------
//
// Managing the memory that is windowed onto the PCI bus
//
// ------------------------------------------------------------------------

static cyg_uint32 i82559_heap_size;
static cyg_uint8 *i82559_heap_base;
static cyg_uint8 *i82559_heap_free;

static void *mem_reserved_ioctl = (void*)0;
// uncacheable memory reserved for ioctl calls

// ------------------------------------------------------------------------
//
//                       FUNCTION PROTOTYPES
//
// ------------------------------------------------------------------------

static int pci_init_find_82559s(void);

static void i82559_reset(struct i82559* p_i82559);
static void i82559_restart(struct i82559 *p_i82559);
static int eth_set_mac_address(struct i82559* p_i82559, char *addr, int eeprom );

static void InitRxRing(struct i82559* p_i82559);
static void ResetRxRing(struct i82559* p_i82559);
static void InitTxRing(struct i82559* p_i82559);
static void ResetTxRing(struct i82559* p_i82559);

static void
eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32
eth_isr(cyg_vector_t vector, cyg_addrword_t data);

static int i82559_configure(struct i82559* p_i82559, int promisc,
                            int oversized, int multicast_all);
#ifdef ETH_DRV_SET_MC_LIST
static int i82559_set_multicast(struct i82559* p_i82559,
                                int num_addrs,
                                cyg_uint8 *address_list );
#endif // ETH_DRV_SET_MC_ALL
#ifdef CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM
static void
program_eeprom(cyg_uint32 ioaddr, cyg_uint32 eeprom_size, cyg_uint8 *data);
static void
write_eeprom(long ioaddr, int location, int addr_len, unsigned short value);
static int
write_enable_eeprom(long ioaddr,  int addr_len);
#endif

// debugging/logging only:
void dump_txcb(TxCB* p_txcb);
void DisplayStatistics(void);
void update_statistics(struct i82559* p_i82559);
void dump_rfd(RFD* p_rfd, int anyway );
void dump_all_rfds( int intf );
void dump_packet(cyg_uint8 *p_buffer, int length);

static void i82559_stop( struct eth_drv_sc *sc );

// ------------------------------------------------------------------------
// utilities
// ------------------------------------------------------------------------

typedef enum {
    WAIT_RU,                            // wait before RU cmd
    WAIT_CU                             // wait before CU cmd
} cmd_wait_t;

static inline
void 
wait_for_cmd_done(long scb_ioaddr, cmd_wait_t type)
{
    register int status;
    register int wait;

    wait = 0x100000;
    do status = INW(scb_ioaddr + SCBCmd) /* nothing */ ;
    while( (0 != ((CU_CMD_MASK | RU_CMD_MASK) & status)) && --wait >= 0);

    if ( wait <= 0 ) {
        // Then we timed out; reset the device and try again...
        OUTL(I82559_SELECTIVE_RESET, scb_ioaddr + SCBPort);
#ifdef CYGDBG_USE_ASSERTS
        missed_interrupt.waitcmd_timeouts++;
#endif
        wait = 0x100000;
        do status = INW(scb_ioaddr + SCBCmd) /* nothing */;
        while( (0 != ((CU_CMD_MASK | RU_CMD_MASK) & status)) && --wait >= 0);
    }

    // special case - don't complain about RUC_ADDR_LOAD as it doesn't clear
    // on some silicon
    if ( RUC_ADDR_LOAD != (status & RU_CMD_MASK) )
        CYG_ASSERT( wait > 0, "wait_for_cmd_done: cmd busy" );

    if (WAIT_CU == type) {
        // Also check CU is idle
        wait = 0x100000;
        do status = INW(scb_ioaddr + SCBStatus) /* nothing */;
        while( (status & CU_STATUS_MASK) && --wait >= 0);
        if ( wait <= 0 ) {
            // Then we timed out; reset the device and try again...
            OUTL(I82559_SELECTIVE_RESET, scb_ioaddr + SCBPort);
#ifdef CYGDBG_USE_ASSERTS
            missed_interrupt.waitcmd_timeouts_cu++;
#endif
            wait = 0x100000;
            do status = INW(scb_ioaddr + SCBCmd) /* nothing */;
            while( (0 != ((CU_CMD_MASK | RU_CMD_MASK) & status)) && --wait >= 0);
        }

        CYG_ASSERT( wait > 0, "wait_for_cmd_done: CU busy" );

    } else if (WAIT_RU == type) {
        // Can't see any active state in the doc to check for 
    }
}

// ------------------------------------------------------------------------

static void
udelay(int delay)
{
    CYGACC_CALL_IF_DELAY_US(delay);
}

// ------------------------------------------------------------------------
// If we are demuxing for all interrupt sources, we must mask and unmask
// *all* interrupt sources together.

static inline int 
Mask82559Interrupt(struct i82559* p_i82559)
{
    int old = 0;

#ifdef CYGPRI_DEVS_ETH_INTEL_I82559_MASK_INTERRUPTS
    CYGPRI_DEVS_ETH_INTEL_I82559_MASK_INTERRUPTS(p_i82559,old);
#else
//    if (query_enabled)
    old |= 1;
    cyg_drv_interrupt_mask(p_i82559->vector);
#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
//    if (query_enabled)
    old |= 2;
    cyg_drv_interrupt_mask(CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT);
#endif
#endif
    /* it may prove necessary to do something like this
    if ( mybits != (mybits & old) )
        /# then at least one of them was disabled, so
         # omit both from any restoration: #/
        old = 0;
    */

    return old;
}

static inline void
UnMask82559Interrupt(struct i82559* p_i82559, int old)
{
#ifdef CYGPRI_DEVS_ETH_INTEL_I82559_UNMASK_INTERRUPTS
    CYGPRI_DEVS_ETH_INTEL_I82559_UNMASK_INTERRUPTS(p_i82559,old);
#else
    // We must only unmask (enable) those which were unmasked before,
    // according to the bits in old.
    if (old & 1)
        cyg_drv_interrupt_unmask(p_i82559->vector);
#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
    if (old & 2)
        cyg_drv_interrupt_mask(CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT);
#endif
#endif
}


static inline void
Acknowledge82559Interrupt(struct i82559* p_i82559)
{
#ifdef CYGPRI_DEVS_ETH_INTEL_I82559_ACK_INTERRUPTS
    CYGPRI_DEVS_ETH_INTEL_I82559_ACK_INTERRUPTS(p_i82559);
#else
    cyg_drv_interrupt_acknowledge(p_i82559->vector);
#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
    cyg_drv_interrupt_acknowledge(CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT);
#endif
#endif

#ifdef CYGPRI_DEVS_ETH_INTEL_I82559_INTERRUPT_ACK_LOOP
    CYGPRI_DEVS_ETH_INTEL_I82559_INTERRUPT_ACK_LOOP(p_i82559);
#endif
}


static inline void
Check82559TxLockupTimeout(struct i82559* p_i82559)
{
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_RESET_TIMEOUT
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_TIMEOUT_FIRED
    int ints = Mask82559Interrupt(p_i82559);
    if ( p_i82559->tx_in_progress &&
        (p_i82559->tx_descriptor_timeout == p_i82559->tx_descriptor_active) ) {
        if ( CYGHWR_DEVS_ETH_INTEL_I82559_TIMEOUT_FIRED( p_i82559->platform_timeout ) ) {
            // Then reset the device; the TX unit is locked up.
            cyg_uint32 ioaddr = p_i82559->io_address;
            // Wait some time afterwards for chip to settle.
            OUTL(I82559_SELECTIVE_RESET, ioaddr + SCBPort);
#ifdef CYGDBG_USE_ASSERTS
            missed_interrupt.lockup_timeouts++;
#endif
            udelay(20);
        }
    }
    else {
        // All is well:
        CYGHWR_DEVS_ETH_INTEL_I82559_RESET_TIMEOUT( p_i82559->platform_timeout );
        p_i82559->tx_descriptor_timeout = p_i82559->tx_descriptor_active;
    }
    UnMask82559Interrupt(p_i82559,ints);
#endif
#endif
}

// ------------------------------------------------------------------------
// Memory management
//
// Simply carve off from the front of the PCI mapped window into real memory

static void*
pciwindow_mem_alloc(int size)
{
    void *p_memory;
    int _size = size;

    CYG_ASSERT(
        (CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE <= (int)i82559_heap_free)
        &&
        ((CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE + 
          CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE) > (int)i82559_heap_free)
        &&
        (0 < i82559_heap_size)
        &&
        (CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE >= i82559_heap_size)
        &&
        (CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE == (int)i82559_heap_base),
        "Heap variables corrupted" );

    p_memory = (void *)0;
    size = (size + 3) & ~3;
    if ( (i82559_heap_free+size) < (i82559_heap_base+i82559_heap_size) ) {
        cyg_uint32 *p;
        p_memory = (void *)i82559_heap_free;
        i82559_heap_free += size;
        for ( p = (cyg_uint32 *)p_memory; _size > 0; _size -= 4 )
            *p++ = 0;
    }

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_PCIMEM_DISCONTIGUOUS
    // Then the underlying physical address can have breaks in it.
    // We cannot use such a block, clearly, so we just discard and re-do.
    if ( p_memory ) {
        char *bpm = (char *)VIRT_TO_BUS( p_memory );
        char *bmf = (char *)VIRT_TO_BUS( i82559_heap_free );
        
        if ( bpm + size != bmf ) {
            // then we found a break; retry:
            if ( (i82559_heap_free+size) < (i82559_heap_base+i82559_heap_size) ) {
                cyg_uint32 *p;
                p_memory = (void *)i82559_heap_free;
                i82559_heap_free += size;
                for ( p = (cyg_uint32 *)p_memory; _size > 0; _size -= 4 )
                    *p++ = 0;
            }
        }
    }
#endif
    CYG_ASSERT(
        NULL == p_memory ||
        VIRT_TO_BUS( p_memory ) + size == VIRT_TO_BUS( i82559_heap_free ),
        "Discontiguous PCI memory in real addresses" );

    return p_memory;
}


// ------------------------------------------------------------------------
//
//                       GET EEPROM SIZE
//
// ------------------------------------------------------------------------
#ifndef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
static int
get_eeprom_size(long ioaddr)
{
    unsigned short retval = 0;
    int ee_addr = ioaddr + SCBeeprom;
    int i, addrbits;

    // Should already be not-selected, but anyway:
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay(EEPROM_ENABLE_DELAY);
    OUTW(EE_ENB, ee_addr);
    eeprom_delay(EEPROM_ENABLE_DELAY);
    
    // Shift the read command bits out.
    for (i = 2; i >= 0; i--) {
        short dataval = (6 & (1 << i)) ? EE_DATA_WRITE : 0;
        OUTW(EE_ENB | dataval               , ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB | dataval               , ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
    }
    // Now clock out address zero, looking for the dummy 0 data bit
    for ( i = 1; i <= 12; i++ ) {
        OUTW(EE_ENB               , ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB               , ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        retval = INW(ee_addr) & EE_DATA_READ;
        if ( 0 == retval )
            break; // The dummy zero est arrive'
    }

#ifdef DEBUG_EE
    os_printf( "eeprom data bits %d (ioaddr %x)\n", i, ee_addr );
#endif
    
    if ( 6 != i && 8 != i && 1 != i) {
#ifdef DEBUG_EE
        os_printf( "*****EEPROM data bits not 6, 8 or 1*****\n" );
#endif
        i = 6; // guess, to complete this routine
        addrbits = 1; // Flag no eeprom here.
    }
    else
        addrbits = i;

    // clear the dataval, leave the clock low to read in the data regardless
    OUTW(EE_ENB, ee_addr);
    eeprom_delay(1);
    
    retval = INW(ee_addr);
    if ( (EE_DATA_READ & retval) != 0 ) {
#ifdef DEBUG_EE
        os_printf( "Size EEPROM: Dummy data bit not 0, reg %x\n" , retval );
#endif
    }
    eeprom_delay(1);
    
    for (i = EE_TOP_DATA_BIT; i >= 0; i--) {
        OUTW(EE_ENB | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        retval = INW(ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
    }
    
    // Terminate the EEPROM access.
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay(EEPROM_DONE_DELAY);
    
    return addrbits;
}


// ------------------------------------------------------------------------
//
//                       READ EEPROM
//
// ------------------------------------------------------------------------
static int
read_eeprom(long ioaddr, int location, int addr_len)
{
    unsigned short retval = 0;
    int ee_addr = ioaddr + SCBeeprom;
    int read_cmd = location | EE_READ_CMD(addr_len);
    int i, tries = 10;

 try_again:
    // Should already be not-selected, but anyway:
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay(EEPROM_ENABLE_DELAY);
    OUTW(EE_ENB, ee_addr);
    eeprom_delay(EEPROM_ENABLE_DELAY);
    
    // Shift the read command bits out, changing only one bit per time.
    for (i = EE_TOP_CMD_BIT(addr_len); i >= 0; i--) {
        short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
        OUTW(EE_ENB | dataval               , ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB | dataval               , ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
    }

    // clear the dataval, leave the clock low
    OUTW(EE_ENB, ee_addr);
    eeprom_delay(1);
    
    retval = INW(ee_addr);
    // This should show a zero in the data read bit to confirm that the
    // address transfer is compelete.  If not, go to the start and try
    // again!
    if ( (0 != (retval & EE_DATA_READ)) && (tries-- > 0) ) {
    // Terminate the EEPROM access.
        OUTW(EE_ENB & ~EE_CS, ee_addr);
        eeprom_delay(EEPROM_DONE_DELAY);
#ifdef DEBUG_EE
        os_printf( "Warning: Retrying EEPROM read word %d, address %x, try %d\n",
                   location,  ee_addr, tries+1 );
#endif
        goto try_again;
    }

    // This fires with one device on one of the customer boards!
    // (but is OK on all other h/w.  Worrying huh.)
    if ( (EE_DATA_READ & retval) != 0 ) {
#ifdef DEBUG_EE
        os_printf( "Read EEPROM: Dummy data bit not 0, reg %x\n" , retval );
#endif
    }
    eeprom_delay(1);
    retval = 0;

    for (i = EE_TOP_DATA_BIT; i >= 0; i--) {
        OUTW(EE_ENB | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
        retval = (retval << 1) | ((INW(ee_addr) & EE_DATA_READ) ? 1 : 0);
        eeprom_delay(EEPROM_SK_DELAY);
        OUTW(EE_ENB, ee_addr);
        eeprom_delay(EEPROM_SK_DELAY);
    }
    
    // Terminate the EEPROM access.
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay(EEPROM_DONE_DELAY);
    
    return retval;
}

static int
read_eeprom_esa(struct i82559 *p_i82559, char *addr)
{
    int addr_length, i, count;
    cyg_uint16 checksum;
    cyg_uint32 ioaddr = p_i82559->io_address;

    // read eeprom and get 82559's mac address
    addr_length = get_eeprom_size(ioaddr);
    // (this is the length of the *EEPROM*s address, not MAC address)

    // If length is 1, it _probably_ means there's no EEPROM
    // present.  Couldn't find an explicit mention of this in the
    // docs, but length=1 appears to be the behaviour in that case.
    if (1 == addr_length) {
#ifdef DEBUG_EE
	os_printf("Error: No EEPROM present for device %d\n", 
		  p_i82559->index);
#endif
    } else {
	for (checksum = 0, i = 0, count = 0; count < (1 << addr_length); count++) {
	    cyg_uint16 value;
	    // read word from eeprom
	    value = read_eeprom(ioaddr, count, addr_length);
#ifdef DEBUG_EE
	    os_printf( "%2d: %04x\n", count, value );
#endif
	    checksum += value;
	    if (count < 3) {
		addr[i++] = value & 0xFF;
		addr[i++] = (value >> 8) & 0xFF;
	    }
	}

#ifndef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_WITHOUT_CRC
	// If the EEPROM checksum is wrong, the MAC address read
	// from the EEPROM is probably wrong as well. In that
	// case, we don't set mac_addr_ok, but continue the
	// initialization. If then somebody calls i82559_start
	// without calling eth_set_mac_address() first, we refuse
	// to bring up the interface, because running with an
	// invalid MAC address is not a very brilliant idea.
        
	if ((checksum & 0xFFFF) != 0xBABA)  {
	    // selftest verified checksum, verify again
#ifdef DEBUG_EE
	    os_printf("Warning: Invalid EEPROM checksum %04X for device %d\n",
		      checksum, p_i82559->index);
#endif
	} else // trailing block
#endif // ! CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_WITHOUT_CRC
        {
#ifdef DEBUG_EE
	    os_printf("Valid EEPROM checksum\n");
#endif
	    return 1;
	}
    }
    return 0;
}
#endif // ! CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM

// ------------------------------------------------------------------------
//
//                NETWORK INTERFACE INITIALIZATION
//
//  Function : Init82559
//
//  Description :
//       This routine resets, configures, and initializes the chip.
//       It also clears the ethernet statistics structure, and selects
//       which statistics are supported by this driver.
//
// ------------------------------------------------------------------------
static bool
i82559_init(struct cyg_netdevtab_entry * ndp)
{
    static int initialized = 0; // only probe PCI et al *once*

    struct eth_drv_sc *sc;
    cyg_uint32 selftest;
    volatile cyg_uint32 *p_selftest;
    cyg_uint32 ioaddr;
    int count;
    int ints;
    struct i82559 *p_i82559;
    cyg_uint8 mac_address[ETHER_ADDR_LEN];

#ifdef DEBUG
    //    db_printf("intel_i82559_init\n");
#endif

    sc = (struct eth_drv_sc *)(ndp->device_instance);
    p_i82559 = (struct i82559 *)(sc->driver_private);

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "Bad device private pointer %x\n", sc->driver_private );
#endif
        return 0;
    }

    CHECK_NDP_SC_LINK();

    if ( 0 == initialized++ ) {
        // then this is the first time ever:
        if ( ! pci_init_find_82559s() ) {
#ifdef DEBUG
            os_printf( "pci_init_find_82559s failed\n" );
#endif
            return 0;
        }
    }

    // If this device is not present, exit
    if (0 == p_i82559->found)
        return 0;

    p_i82559->mac_addr_ok = 0;

    ioaddr = p_i82559->io_address; // get I/O address for 82559

#ifdef DEBUG
    os_printf("Init82559 %d @ %x\n82559 Self Test\n",
              p_i82559->index, (int)ndp);
#endif

    ints = Mask82559Interrupt(p_i82559);

    // Reset device
    i82559_reset(p_i82559);

    // Perform a system self-test. (get enough mem to round address)
    if ( (selftest = (cyg_uint32)pciwindow_mem_alloc(32) ) == 0)
        return (0);
    p_selftest = (cyg_uint32 *) ((selftest + 15) & ~0xf);
    p_selftest[0] = p_selftest[1] = -1;

    OUTL( (VIRT_TO_BUS(p_selftest)) | I82559_SELFTEST, ioaddr + SCBPort);
    count = 0x7FFFF;                // Timeout for self-test.
    do {
        udelay(10);
    } while ( (p_selftest[1] == -1)  &&  (--count >= 0) );

    // Reset device again after selftest
    i82559_reset(p_i82559);

    Acknowledge82559Interrupt(p_i82559);
    UnMask82559Interrupt(p_i82559, ints );
    
    if (count < 0) {
        // Test timed out.
#ifdef DEBUG
        os_printf("Self test failed\n");
#endif
        return (0);
    }
#ifdef DEBUG
    os_printf("  General self-test: %s.\n"
              "  Serial sub-system self-test: %s.\n"
              "  Internal registers self-test: %s.\n"
              "  ROM checksum self-test: %s (%08X).\n",
              HAL_LE32TOC(p_selftest[1]) & 0x1000 ? "failed" : "passed",
              HAL_LE32TOC(p_selftest[1]) & 0x0020 ? "failed" : "passed",
              HAL_LE32TOC(p_selftest[1]) & 0x0008 ? "failed" : "passed",
              HAL_LE32TOC(p_selftest[1]) & 0x0004 ? "failed" : "passed",
              HAL_LE32TOC(p_selftest[0]));
#endif

    // free self-test memory?
    // No, there's no point: this "heap" does not support free.

    if (p_i82559->hardwired_esa) {
        // Hardwire the address without consulting the EEPROM.
        // When this flag is set, the p_i82559 will already contain
        // the ESA. Copy it to a mac_address for call to set_mac_addr
        mac_address[0] = p_i82559->mac_address[0];
        mac_address[1] = p_i82559->mac_address[1];
        mac_address[2] = p_i82559->mac_address[2];
        mac_address[3] = p_i82559->mac_address[3];
        mac_address[4] = p_i82559->mac_address[4];
        mac_address[5] = p_i82559->mac_address[5];

        eth_set_mac_address(p_i82559, mac_address, 0);

    } else {

        // Acquire the ESA either from extenal means (probably RedBoot
        // variables) or from the attached EEPROM - if there is one.

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_GET_ESA
        int ok = false;
        int wflag = 0;
        CYGHWR_DEVS_ETH_INTEL_I82559_GET_ESA( p_i82559, mac_address, ok );
        if ( ok ) {
#ifndef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
	    if ( CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM == p_i82559->index )
#endif // CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
	    {
		cyg_uint8 tmp_addr[ETHER_ADDR_LEN];

		// write eeprom address unless it is already there
		wflag = 1;
		if (read_eeprom_esa(p_i82559, tmp_addr)) {
		    int i;
		    for (i = 0; i < ETHER_ADDR_LEN; i++)
			if (tmp_addr[i] != mac_address[i])
			    break;
		    if (i >= ETHER_ADDR_LEN)
			wflag = 0;
		}
	    }
#endif // ! CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
	    eth_set_mac_address(p_i82559, mac_address, wflag);
	}
#else // ! CYGHWR_DEVS_ETH_INTEL_I82559_GET_ESA

#ifndef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
        if ( CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM == p_i82559->index ) {
#endif // CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM

	    if (read_eeprom_esa(p_i82559, mac_address)) {
		// record the MAC address in the device structure
		p_i82559->mac_address[0] = mac_address[0];
		p_i82559->mac_address[1] = mac_address[1];
		p_i82559->mac_address[2] = mac_address[2];
		p_i82559->mac_address[3] = mac_address[3];
		p_i82559->mac_address[4] = mac_address[4];
		p_i82559->mac_address[5] = mac_address[5];

                p_i82559->mac_addr_ok = 1;

                eth_set_mac_address(p_i82559, mac_address, 0);
	    }

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
        }
        else { // We are now "in" another device
#if 1 < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT
            struct i82559 *other; // The one that *is* set up from EEPROM
            other = i82559_priv_array[CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM];
            if ( other->mac_addr_ok ) {
                mac_address[0] = other->mac_address[0];
                mac_address[1] = other->mac_address[1];
                mac_address[2] = other->mac_address[2];
                mac_address[3] = other->mac_address[3];
                mac_address[4] = other->mac_address[4];
                mac_address[5] = other->mac_address[5];

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_MAC_ADJUST
                mac_address[5] += CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_MAC_ADJUST;
#endif
                eth_set_mac_address(p_i82559, mac_address, 0);
            }
#endif // 1 < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT
        }
#endif // CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM

#endif // ! CYGHWR_DEVS_ETH_INTEL_I82559_HAS_NO_EEPROM
#endif // ! CYGHWR_DEVS_ETH_INTEL_I82559_GET_ESA
    }

#ifdef DEBUG
    os_printf("i82559_init: MAC Address = %02X %02X %02X %02X %02X %02X\n",
              p_i82559->mac_address[0], p_i82559->mac_address[1],
              p_i82559->mac_address[2], p_i82559->mac_address[3],
              p_i82559->mac_address[4], p_i82559->mac_address[5]);
#endif
    
    // and record the net dev pointer
    p_i82559->ndp = (void *)ndp;
    
    p_i82559->within_send = 0; // init recursion level

    p_i82559->promisc = 0;  // None of these initially
    p_i82559->multicast_all = 0;
    p_i82559->oversized = 1;    // Enable this for VLAN mode by default

    InitRxRing(p_i82559);
    InitTxRing(p_i82559);

    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);

    // Initialize upper level driver
    if ( p_i82559->mac_addr_ok )
        (sc->funs->eth_drv->init)(sc, &(p_i82559->mac_address[0]) );
    else
        (sc->funs->eth_drv->init)(sc, NULL );

    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);

    return (1);
}

// ------------------------------------------------------------------------
//
//  Function : i82559_start
//
// ------------------------------------------------------------------------
static void 
i82559_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags )
{
    struct i82559 *p_i82559;
    cyg_uint32 ioaddr;
#ifdef KEEP_STATISTICS
    void *p_statistics;
#endif
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif

    p_i82559 = (struct i82559 *)sc->driver_private;
    ioaddr = p_i82559->io_address; // get 82559's I/O address
    
    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_start: Bad device pointer %x\n", p_i82559 );
#endif
        return;
    }

    if ( ! p_i82559->mac_addr_ok ) {
#ifdef DEBUG
        os_printf("i82559_start %d: invalid MAC address, "
                  "can't bring up interface\n",
                  p_i82559->index );
#endif
        return;
    }

    if ( p_i82559->active )
        i82559_stop( sc );

#ifdef KEEP_STATISTICS
#ifdef CYGDBG_DEVS_ETH_INTEL_I82559_KEEP_STATISTICS
    p_statistics = p_i82559->p_statistics;
    memset(p_statistics, 0xFFFFFFFF, sizeof(I82559_COUNTERS));
    // set statistics dump address
    wait_for_cmd_done(ioaddr, WAIT_CU);
    OUTL(VIRT_TO_BUS(p_statistics), ioaddr + SCBPointer);
    OUTW(SCB_M | CU_STATSADDR, ioaddr + SCBCmd);
    // Start dump command
    wait_for_cmd_done(ioaddr, WAIT_CU);
    OUTW(SCB_M | CU_DUMPSTATS, ioaddr + SCBCmd); // start register dump
    // ...and wait for it to complete operation

    // The code to wait was bogus; it was looking at the structure in the
    // wrong way.  In any case, there is no need to wait, the
    // wait_for_cmd_done() in any following activity is good enough.

#endif
#endif

    // Enable device
    p_i82559->active = 1;

    /* Enable promiscuous mode if requested, reception of oversized frames always.
     * The latter is needed for VLAN support and shouldn't hurt even if we're not
     * using VLANs.  Reset multicastALL reception choice.
     */

    p_i82559->promisc = 0
#ifdef CYGPKG_NET
                     || !!(ifp->if_flags & IFF_PROMISC)
#endif
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
                     || !!(flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif
            ;

    p_i82559->multicast_all = 0;

    i82559_configure(p_i82559,
                     p_i82559->promisc,
                     p_i82559->oversized,
                     p_i82559->multicast_all );

#ifdef DEBUG
    {
        int status = i82559_status( sc );
        os_printf("i82559_start %d flg %x Link = %s, %s Mbps, %s Duplex\n",
                  p_i82559->index,
                  *(int *)p_i82559,
                  status & GEN_STATUS_LINK ? "Up" : "Down",
                  status & GEN_STATUS_100MBPS ?  "100" : "10",
                  status & GEN_STATUS_FDX ? "Full" : "Half");
    }
#endif

    i82559_restart(p_i82559);
    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
}

static void i82559_restart(struct i82559 *p_i82559)
{
    cyg_uint32 ioaddr;
    ioaddr = p_i82559->io_address; // get 82559's I/O address

    // Load pointer to Rx Ring and enable receiver
    wait_for_cmd_done(ioaddr, WAIT_RU);
    OUTL(VIRT_TO_BUS(p_i82559->rx_ring[0]), ioaddr + SCBPointer);
    OUTW(RUC_START, ioaddr + SCBCmd);
}

// ------------------------------------------------------------------------
//
//  Function : i82559_status
//
// ------------------------------------------------------------------------
int
i82559_status( struct eth_drv_sc *sc )
{
    int status;
    struct i82559 *p_i82559;
    cyg_uint32 ioaddr;
    p_i82559 = (struct i82559 *)sc->driver_private;
    
    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_status: Bad device pointer %x\n", p_i82559 );
#endif
        return 0;
    }

    ioaddr = p_i82559->io_address; // get 82559's I/O address

    status = INB(ioaddr + SCBGenStatus);

    return status;
}

// ------------------------------------------------------------------------
//
//  Function : BringDown82559
//
// ------------------------------------------------------------------------

static void
i82559_stop( struct eth_drv_sc *sc )
{
    struct i82559 *p_i82559;

    p_i82559 = (struct i82559 *)sc->driver_private;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_stop: Bad device pointer %x\n", p_i82559 );
#endif
        return;
    }
   
#ifdef DEBUG
    os_printf("i82559_stop %d flg %x\n", p_i82559->index, *(int *)p_i82559 );
#endif

    p_i82559->active = 0;               // stop people tormenting it
    i82559_reset(p_i82559);             // that should stop it

    // Now that it's inactive, return all pending tx status to the higher
    // layers:
    // "Done" txen are from here to active, OR 
    // the remove one if the queue is full AND its status is nonzero:
    while ( 1 ) {
	int tx_descriptor_remove = p_i82559->tx_descriptor_remove;
	unsigned long key = p_i82559->tx_keys[ tx_descriptor_remove ];

	// Break out if "remove" is the active slot
	// (AND the Q is not full, or the Tx is not complete yet)
	if ( (tx_descriptor_remove == p_i82559->tx_descriptor_active) &&
	     ( ! p_i82559->tx_queue_full) )
	    break;
	    
	// Zero the key in global state before the callback:
	p_i82559->tx_keys[ tx_descriptor_remove ] = 0;

#ifdef DEBUG_82559
	os_printf("Stop: TxDone %d %x: KEY %x TxCB %x\n",
		  p_i82559->index, (int)p_i82559, key, 
                  p_i82559->tx_ring[ tx_descriptor_remove ]);
#endif
	// tx_done() can now cope with a NULL key, no guard needed here
	(sc->funs->eth_drv->tx_done)( sc, key, 1 /* status */ );

	if ( ++tx_descriptor_remove >= MAX_TX_DESCRIPTORS )
	    tx_descriptor_remove = 0;
	p_i82559->tx_descriptor_remove = tx_descriptor_remove;
	p_i82559->tx_queue_full = 0;
    }

    ResetRxRing( p_i82559 );
    ResetTxRing( p_i82559 );
    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
}


// ------------------------------------------------------------------------
//
//  Function : InitRxRing
//
// ------------------------------------------------------------------------
static void
InitRxRing(struct i82559* p_i82559)
{
    int i;
    RFD *rfd;
    RFD *p_rfd = 0;
#ifdef DEBUG_82559
    os_printf("InitRxRing %d\n", p_i82559->index);
#endif
    for ( i = 0; i < MAX_RX_DESCRIPTORS; i++ ) {
        rfd = (RFD *)pciwindow_mem_alloc(RFD_SIZEOF + MAX_RX_PACKET_SIZE);
        p_i82559->rx_ring[i] = rfd;
        if ( i )
            WRITEMEM32(p_rfd+RFD_LINK, HAL_CTOLE32(VIRT_TO_BUS(rfd)));
        p_rfd = (RFD *)rfd;
    }
    // link last RFD to first:
    WRITEMEM32(p_rfd+RFD_LINK,
                     HAL_CTOLE32(VIRT_TO_BUS(p_i82559->rx_ring[0])));

    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
    ResetRxRing( p_i82559 );
}

// ------------------------------------------------------------------------
//
//  Function : ResetRxRing
//
// ------------------------------------------------------------------------
static void
ResetRxRing(struct i82559* p_i82559)
{
    RFD *p_rfd;
    int i;
#ifdef DEBUG_82559
    os_printf("ResetRxRing %d\n", p_i82559->index);
#endif
    for ( i = 0; i < MAX_RX_DESCRIPTORS; i++ ) {
        p_rfd = p_i82559->rx_ring[i];
        CYG_ASSERT( (cyg_uint8 *)p_rfd >= i82559_heap_base, "rfd under" );
        CYG_ASSERT( (cyg_uint8 *)p_rfd <  i82559_heap_free, "rfd over" );
#ifdef CYGDBG_USE_ASSERTS
        {
          RFD *p_rfd2;
          cyg_uint32 link;
          p_rfd2 = p_i82559->rx_ring[ ( i ? (i-1) : (MAX_RX_DESCRIPTORS-1) ) ];
          READMEM32(p_rfd2 + RFD_LINK, link);
	  if (!(HAL_LE32TOC(link) == VIRT_TO_BUS(p_rfd))) {
            START_CONSOLE();
	    diag_printf("Bad link eth%d %p %p %d %p\n",
			p_i82559->index,
			HAL_LE32TOC(link), VIRT_TO_BUS(p_rfd),
			i, __builtin_return_address(0));
            END_CONSOLE();
	  }
          CYG_ASSERT( HAL_LE32TOC(link) == VIRT_TO_BUS(p_rfd), 
                      "rfd linked list broken" );
        }
#endif
        WRITEMEM32(p_rfd + RFD_STATUS, 0 ); // do NOT suspend after just one rx
        WRITEMEM16(p_rfd + RFD_COUNT, 0);
        WRITEMEM32(p_rfd + RFD_RDB_ADDR, HAL_CTOLE32(0xFFFFFFFF));
        WRITEMEM16(p_rfd + RFD_SIZE, HAL_CTOLE16(MAX_RX_PACKET_SIZE));
    }
    p_i82559->next_rx_descriptor = 0;
    // And set an end-of-list marker in the previous one.
    WRITEMEM32(p_rfd + RFD_STATUS, RFD_STATUS_EL);
    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
}

// ------------------------------------------------------------------------
//
//  Function : CheckRxRing
//
// ------------------------------------------------------------------------
void
CheckRxRing(struct i82559* p_i82559, char * func, int line)
{
    RFD *p_rfd;
    int i;
    RFD *p_rfd2;
    cyg_uint32 link;
    
    //    console_printf("%s:%d(eth%d)\n",func,line,p_i82559->index);

    for ( i = 0; i < MAX_RX_DESCRIPTORS; i++ ) {
      p_rfd = p_i82559->rx_ring[i];
      if (! ((cyg_uint8 *)p_rfd >= i82559_heap_base))
	console_printf("rfd under: %s:%d\n", func, line);
      if (! ((cyg_uint8 *)p_rfd <  i82559_heap_free)) 
	console_printf("rfd over: %s:%d\n", func, line );
      
      p_rfd2 = p_i82559->rx_ring[ ( i ? (i-1) : (MAX_RX_DESCRIPTORS-1) ) ];
      READMEM32(p_rfd2 + RFD_LINK, link);
      if (!(HAL_LE32TOC(link) == VIRT_TO_BUS(p_rfd))) {
	console_printf("Bad link eth%d %p %p %d %p: %s:%d\n",
		       p_i82559->index,
		       HAL_LE32TOC(link), VIRT_TO_BUS(p_rfd),
		       i, __builtin_return_address(0),
		       func,line);
	CYG_ASSERT(HAL_LE32TOC(link) == VIRT_TO_BUS(p_rfd),"Bad Link");
      }
    }
}

// ------------------------------------------------------------------------
//
//  Function : PacketRxReady     (Called from delivery thread & foreground)
//
// ------------------------------------------------------------------------
static void
PacketRxReady(struct i82559* p_i82559)
{
    RFD *p_rfd;
    int next_descriptor;
    int length, ints;
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;
    cyg_uint32 ioaddr;
    cyg_uint16 status;

    ndp = (struct cyg_netdevtab_entry *)(p_i82559->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);

    CHECK_NDP_SC_LINK();

    ioaddr = p_i82559->io_address;

    next_descriptor = p_i82559->next_rx_descriptor;
    p_rfd = p_i82559->rx_ring[next_descriptor];

    CYG_ASSERT( (cyg_uint8 *)p_rfd >= i82559_heap_base, "rfd under" );
    CYG_ASSERT( (cyg_uint8 *)p_rfd <  i82559_heap_free, "rfd over" );

    while ( 1 ) {
        cyg_uint32 rxstatus;
        cyg_uint16 rxstatus_hi;
        READMEM32(p_rfd + RFD_STATUS, rxstatus);
        if (0 == (rxstatus & RFD_STATUS_C))
            break;

        READMEM16(p_rfd + RFD_STATUS_HI, rxstatus_hi);
        rxstatus_hi |= RFD_STATUS_HI_EL;
        WRITEMEM16(p_rfd + RFD_STATUS_HI, rxstatus_hi);

        READMEM16(p_rfd + RFD_COUNT, length);
        length = HAL_LE16TOC(length);
        length &= RFD_COUNT_MASK;

#ifdef DEBUG_82559
        os_printf( "Device %d (eth%d), rx descriptor %d:\n", 
                   p_i82559->index, p_i82559->index, next_descriptor );
//        dump_rfd( p_rfd, 1 );
#endif

        p_i82559->next_rx_descriptor = next_descriptor;
        // Check for bogusly short packets; can happen in promisc mode:
        // Asserted against and checked by upper layer driver.
#ifdef CYGPKG_NET
        if ( length > sizeof( struct ether_header ) )
            // then it is acceptable; offer the data to the network stack
#endif
        (sc->funs->eth_drv->recv)( sc, length );

        WRITEMEM16(p_rfd + RFD_COUNT, 0);
        WRITEMEM16(p_rfd + RFD_STATUS_LO, 0);

        // The just-emptied slot is now ready for re-use and already marked EL;
        // we can now remove the EL marker from the previous one.
        if ( 0 == next_descriptor )
            p_rfd = p_i82559->rx_ring[ MAX_RX_DESCRIPTORS-1 ];
        else
            p_rfd = p_i82559->rx_ring[ next_descriptor-1 ];
        // The previous one: check it *was* marked before clearing.
        READMEM16(p_rfd + RFD_STATUS_HI, rxstatus_hi);
        CYG_ASSERT( rxstatus_hi & RFD_STATUS_HI_EL, "No prev EL" );
        // that word is not written by the device.
        WRITEMEM16(p_rfd + RFD_STATUS_HI, 0);

#ifdef KEEP_STATISTICS
        statistics[p_i82559->index].rx_deliver++;
#endif
        if (++next_descriptor >= MAX_RX_DESCRIPTORS)
            next_descriptor = 0;
        p_rfd = p_i82559->rx_ring[next_descriptor];

        CYG_ASSERT( (cyg_uint8 *)p_rfd >= i82559_heap_base, "rfd under" );
        CYG_ASSERT( (cyg_uint8 *)p_rfd <  i82559_heap_free, "rfd over" );

	CheckRxRing(p_i82559,__FUNCTION__,__LINE__);

#ifdef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
	// Can't deliver more than one packet in polled standalone mode
	break;
#endif
    }

    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
    
    // See if the RU has gone idle (usually because of out of resource
    // condition) and restart it if needs be.
    ints = Mask82559Interrupt(p_i82559);
    status = INW(ioaddr + SCBStatus);
    if ( RU_STATUS_READY != (status & RU_STATUS_MASK) ) {
        // Acknowledge the RX INT sources
        OUTW( SCB_INTACK_RX, ioaddr + SCBStatus);
        // (see pages 6-10 & 6-90)

#ifdef KEEP_STATISTICS
        statistics[p_i82559->index].rx_restart++;
#endif
        // There's an end-of-list marker out there somewhere...
        // So mop it up; it takes a little time but this is infrequent.
        ResetRxRing( p_i82559 );  
        next_descriptor = 0;        // re-initialize next desc.
        // load pointer to Rx Ring
        wait_for_cmd_done(ioaddr, WAIT_RU);
        OUTL(VIRT_TO_BUS(p_i82559->rx_ring[0]),
             ioaddr + SCBPointer);
        OUTW(RUC_START, ioaddr + SCBCmd);
        Acknowledge82559Interrupt(p_i82559);
    }
    UnMask82559Interrupt(p_i82559, ints);

    p_i82559->next_rx_descriptor = next_descriptor;
}

// and the callback function

static void 
i82559_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len )
{
    struct i82559 *p_i82559;
    RFD *p_rfd;
    int next_descriptor;
    int total_len;
    struct eth_drv_sg *last_sg;
    volatile cyg_uint8 *from_p;
    cyg_uint32 rxstatus;
    cyg_uint16 rxstatus16;

    p_i82559 = (struct i82559 *)sc->driver_private;
    
    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_recv: Bad device pointer %x\n", p_i82559 );
#endif
        return;
    }

    next_descriptor = p_i82559->next_rx_descriptor;
    p_rfd = p_i82559->rx_ring[next_descriptor];
    
    CYG_ASSERT( (cyg_uint8 *)p_rfd >= i82559_heap_base, "rfd under" );
    CYG_ASSERT( (cyg_uint8 *)p_rfd <  i82559_heap_free, "rfd over" );

    READMEM32(p_rfd + RFD_STATUS, rxstatus);
    CYG_ASSERT( rxstatus & RFD_STATUS_C, "No complete frame" );
    CYG_ASSERT( rxstatus & RFD_STATUS_EL, "No marked frame" );
    READMEM16(p_rfd + RFD_STATUS_LO, rxstatus16 );
    CYG_ASSERT( rxstatus16 & RFD_STATUS_LO_C, "No complete frame 2" );
    READMEM16(p_rfd + RFD_STATUS_HI, rxstatus16 );
    CYG_ASSERT( rxstatus16 & RFD_STATUS_HI_EL, "No marked frame 2" );

    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
    
    if ( 0 == (rxstatus & RFD_STATUS_C) )
        return;

    READMEM16(p_rfd + RFD_COUNT, total_len);
    total_len = HAL_LE16TOC(total_len);
    total_len &= RFD_COUNT_MASK;
    
#ifdef DEBUG_82559
    os_printf("Rx %d %x (status %x): %d sg's, %d bytes\n",
              p_i82559->index, (int)p_i82559, 
              HAL_LE32TOC(rxstatus), sg_len, total_len);
#endif

    // Copy the data to the network stack
    from_p = p_rfd + RFD_BUFFER;

    // check we have memory to copy into; we would be called even if
    // caller was out of memory in order to maintain our state.
    if ( 0 == sg_len || 0 == sg_list )
        return; // caller was out of mbufs

    CYG_ASSERT( 0 < sg_len, "sg_len underflow" );
    CYG_ASSERT( MAX_ETH_DRV_SG >= sg_len, "sg_len overflow" );

    for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ ) {
        cyg_uint8 *to_p;
        int l;
            
        to_p = (cyg_uint8 *)(sg_list->buf);
        l = sg_list->len;

        CYG_ASSERT( 0 <= l, "sg length -ve" );

        if ( 0 >= l || 0 == to_p )
            return; // caller was out of mbufs

        if ( l > total_len )
            l = total_len;

        memcpy( to_p, (unsigned char *)from_p, l );
        from_p += l;
        total_len -= l;
    }

    CYG_ASSERT( 0 == total_len, "total_len mismatch in rx" );
    CYG_ASSERT( last_sg == sg_list, "sg count mismatch in rx" );
    CYG_ASSERT( p_rfd + RFD_BUFFER < from_p, "from_p wild in rx" );
    CYG_ASSERT( p_rfd + RFD_BUFFER + MAX_RX_PACKET_SIZE >= from_p,
                "from_p overflow in rx" );
}


// ------------------------------------------------------------------------
//
//  Function : InitTxRing
//
// ------------------------------------------------------------------------
static void
InitTxRing(struct i82559* p_i82559)
{
    int i;
    cyg_uint32 ioaddr;

#ifdef DEBUG_82559
    os_printf("InitTxRing %d\n", p_i82559->index);
#endif
    ioaddr = p_i82559->io_address;
    for ( i = 0; i < MAX_TX_DESCRIPTORS; i++) {
        p_i82559->tx_ring[i] = (TxCB *)pciwindow_mem_alloc(
            TxCB_SIZEOF + MAX_TX_PACKET_SIZE);
    }

    ResetTxRing(p_i82559);
}

// ------------------------------------------------------------------------
//
//  Function : ResetTxRing
//
// ------------------------------------------------------------------------
static void
ResetTxRing(struct i82559* p_i82559)
{
    int i;
    cyg_uint32 ioaddr;

#ifdef DEBUG_82559
    os_printf("ResetTxRing %d\n", p_i82559->index);
#endif
    ioaddr = p_i82559->io_address;
    p_i82559->tx_descriptor_add =
        p_i82559->tx_descriptor_active = 
        p_i82559->tx_descriptor_remove = 0;
    p_i82559->tx_in_progress =
        p_i82559->tx_queue_full = 0;

    for ( i = 0; i < MAX_TX_DESCRIPTORS; i++) {
        TxCB *p_txcb = p_i82559->tx_ring[i];
        CYG_ASSERT( (cyg_uint8 *)p_txcb >= i82559_heap_base, "txcb under" );
        CYG_ASSERT( (cyg_uint8 *)p_txcb <  i82559_heap_free, "txcb over" );

        WRITEMEM16(p_txcb + TxCB_STATUS, 0);
        WRITEMEM16(p_txcb + TxCB_CMD, 0);
        WRITEMEM32(p_txcb + TxCB_LINK,
                         HAL_CTOLE32(VIRT_TO_BUS((cyg_uint32)p_txcb)));
        WRITEMEM32(p_txcb + TxCB_TBD_ADDR, HAL_CTOLE32(0xFFFFFFFF));
        WRITEMEM8(p_txcb + TxCB_TBD_NUMBER, 0);
        WRITEMEM8(p_txcb + TxCB_TX_THRESHOLD, 16);
        WRITEMEM16(p_txcb + TxCB_COUNT,
                         HAL_CTOLE16(TxCB_COUNT_EOF | 0));
        p_i82559->tx_keys[i] = 0;
    }
    
    wait_for_cmd_done(ioaddr,WAIT_CU);
    OUTL(0, ioaddr + SCBPointer);
    OUTW(SCB_M | CU_ADDR_LOAD, ioaddr + SCBCmd);
    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
}

// ------------------------------------------------------------------------
//
//  Function : TxMachine          (Called from FG & ISR)
//
// This steps the Tx Machine onto the next record if necessary - allowing
// for missed interrupts, and so on.
// ------------------------------------------------------------------------

static void
TxMachine(struct i82559* p_i82559)
{
    int tx_descriptor_active;
    cyg_uint32 ioaddr;

    tx_descriptor_active = p_i82559->tx_descriptor_active;
    ioaddr = p_i82559->io_address;  
    
    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
    // See if the CU is idle when we think it isn't; this is the only place
    // tx_descriptor_active is advanced. (Also recovers from a dropped intr)
    if ( p_i82559->tx_in_progress ) {
        cyg_uint16 status;
        status = INW(ioaddr + SCBStatus);
        if ( 0 == (status & CU_STATUS_MASK) ) {
            // It is idle.  So ack the TX interrupts
            OUTW( SCB_INTACK_TX, ioaddr + SCBStatus);
            // (see pages 6-10 & 6-90)

            // and step on to the next queued tx.
            p_i82559->tx_in_progress = 0;
            if ( ++tx_descriptor_active >= MAX_TX_DESCRIPTORS )
                tx_descriptor_active = 0;
            p_i82559->tx_descriptor_active = tx_descriptor_active;
        }
    }

    // is the CU idle, and there a next tx to set going?
    if ( ( ! p_i82559->tx_in_progress )
         && (( p_i82559->tx_descriptor_add != tx_descriptor_active )
            || p_i82559->tx_queue_full ) )  {
        TxCB *p_txcb = p_i82559->tx_ring[tx_descriptor_active];
        cyg_uint16 status;

        // start Tx operation
        wait_for_cmd_done(ioaddr, WAIT_CU);
        status = INW(ioaddr + SCBStatus);
        // The following assert sometimes fires here, apparantly harmless!
        // So check a second time to let the cursed thing settle and not
        // tell us lies.
        if ( 0 != (status & CU_STATUS_MASK) ) {
#ifdef CYGDBG_USE_ASSERTS
            missed_interrupt.bad_cu_idles++;
#endif
            wait_for_cmd_done(ioaddr, WAIT_CU);
            status = INW(ioaddr + SCBStatus);
        }
        CYG_ASSERT( ( 0 == (status & CU_STATUS_MASK)), "CU not idle");
        CYG_ASSERT( (cyg_uint8 *)p_txcb >= i82559_heap_base, "txcb under" );
        CYG_ASSERT( (cyg_uint8 *)p_txcb <  i82559_heap_free, "txcb over" );
#ifdef DEBUG_82559
        {
            unsigned long key = p_i82559->tx_keys[ tx_descriptor_active ];
            os_printf("Tx %d %x: Starting Engines: KEY %x TxCB %x\n",
                      p_i82559->index, (int)p_i82559, key, p_txcb);
        }
#endif

        OUTL(VIRT_TO_BUS(p_txcb), ioaddr + SCBPointer);
        OUTW(CU_START, ioaddr + SCBCmd);
        p_i82559->tx_in_progress = 1;
    }
    CheckRxRing(p_i82559,__FUNCTION__,__LINE__);
}

// ------------------------------------------------------------------------
//
//  Function : TxDone          (Called from delivery thread)
//
// This returns Tx's from the Tx Machine to the stack (ie. reports
// completion) - allowing for missed interrupts, and so on.
// ------------------------------------------------------------------------

static void
TxDone(struct i82559* p_i82559)
{
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;
    int tx_descriptor_remove = p_i82559->tx_descriptor_remove;

    ndp = (struct cyg_netdevtab_entry *)(p_i82559->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

    CHECK_NDP_SC_LINK();
    
    // "Done" txen are from here to active, OR 
    // the remove one if the queue is full AND its status is nonzero:
    while ( 1 ) {
        cyg_uint16 txstatus;
        TxCB *p_txcb = p_i82559->tx_ring[ tx_descriptor_remove ];
        unsigned long key = p_i82559->tx_keys[ tx_descriptor_remove ];

        READMEM16(p_txcb + TxCB_STATUS, txstatus);

        // Break out if "remove" is the active slot
        // (AND the Q is not full, or the Tx is not complete yet)
        if ( (tx_descriptor_remove == p_i82559->tx_descriptor_active) &&
             ( ( ! p_i82559->tx_queue_full) || (0 == txstatus) ) )
            break;

        // Zero the key in global state before the callback:
        p_i82559->tx_keys[ tx_descriptor_remove ] = 0;

#ifdef DEBUG_82559
        os_printf("TxDone %d %x: KEY %x TxCB %x\n",
                  p_i82559->index, (int)p_i82559, key, p_txcb );
#endif
        // tx_done() can now cope with a NULL key, no guard needed here
        (sc->funs->eth_drv->tx_done)( sc, key, 1 /* status */ );
        
        if ( ++tx_descriptor_remove >= MAX_TX_DESCRIPTORS )
            tx_descriptor_remove = 0;
        p_i82559->tx_descriptor_remove = tx_descriptor_remove;
        p_i82559->tx_queue_full = 0;
    }
}


// ------------------------------------------------------------------------
//
//  Function : i82559_can_send
//
// ------------------------------------------------------------------------

static int 
i82559_can_send(struct eth_drv_sc *sc)
{
    struct i82559 *p_i82559;
    int ints;

    p_i82559 = (struct i82559 *)sc->driver_private;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_send: Bad device pointer %x\n", p_i82559 );
#endif
        return 0;
    }
    
    // Advance TxMachine atomically
    ints = Mask82559Interrupt(p_i82559);

    // This helps unstick deadly embraces.
    CYG_ASSERT( p_i82559->within_send < 10, "send: Excess send recursions" );
    p_i82559->within_send++;

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
    {
        int i;
        // The problem is, if DEMUX_ALL, either device can eat the other's
        // interrupts; so we must poll them *both*:
        for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {
            p_i82559 = i82559_priv_array[i];
            if ( p_i82559->active ) {
                // See if the Tx machine is wedged - reset if so:
                Check82559TxLockupTimeout(p_i82559);
                TxMachine(p_i82559);
                Acknowledge82559Interrupt(p_i82559);
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
		// We are not prepared to receive a packet now if we are in a polled
		// standalone configuration.
                PacketRxReady(p_i82559);
#endif
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
                if ( CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559) ) {
#ifdef CYGDBG_USE_ASSERTS
                    missed_interrupt.can_send++;
#endif
                    eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );
                    eth_dsr( p_i82559->vector, 1, (cyg_addrword_t)p_i82559 );
                }
#endif
            }
        }
    }
    // ensure we look at the correct device at the end
    p_i82559 = (struct i82559 *)sc->driver_private;
#else // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL

    // See if the Tx machine is wedged - reset if so:
    Check82559TxLockupTimeout(p_i82559);
    TxMachine(p_i82559);
    Acknowledge82559Interrupt(p_i82559); // This can eat an Rx interrupt, so
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    // We are not prepared to receive a packet now if we are in a polled
    // standalone configuration.
    PacketRxReady(p_i82559);
#endif
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
    if ( CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559) ) {
#ifdef CYGDBG_USE_ASSERTS
        missed_interrupt.can_send++;
#endif
        eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );
        eth_dsr( p_i82559->vector, 1, (cyg_addrword_t)p_i82559 );
    }
#endif
#endif // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL

    p_i82559->within_send--;
    UnMask82559Interrupt(p_i82559,ints);

    return (0 == p_i82559->within_send) && ! p_i82559->tx_queue_full;
}

// ------------------------------------------------------------------------
//
//  Function : i82559_send
//
// ------------------------------------------------------------------------

static void 
i82559_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list, int sg_len, int total_len,
            unsigned long key)
{
    struct i82559 *p_i82559;
    int tx_descriptor_add, ints;
    TxCB *p_txcb;
    cyg_uint32 ioaddr;

    p_i82559 = (struct i82559 *)sc->driver_private;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_send: Bad device pointer %x\n", p_i82559 );
#endif
        return;
    }

#ifdef DEBUG_82559
    os_printf("Tx %d %x: %d sg's, %d bytes, KEY %x\n",
              p_i82559->index, (int)p_i82559, sg_len, total_len, key );
#endif

    if ( ! p_i82559->active )
        return;                         // device inactive, no return
#ifdef KEEP_STATISTICS
    statistics[p_i82559->index].tx_count++;
#endif
    ioaddr = p_i82559->io_address;      // get device I/O address

    // See if the Tx machine is wedged - reset if so:
    Check82559TxLockupTimeout(p_i82559);

    if ( p_i82559->tx_queue_full ) {
#ifdef KEEP_STATISTICS
        statistics[p_i82559->index].tx_dropped++;
#endif
#ifdef DEBUG_82559
        os_printf( "i82559_send: Queue full, device %x, key %x\n",
                   p_i82559, key );
#endif
    }
    else {
        struct eth_drv_sg *last_sg;
        volatile cyg_uint8 *to_p;

        tx_descriptor_add = p_i82559->tx_descriptor_add;

        p_i82559->tx_keys[tx_descriptor_add] = key;

        p_txcb = p_i82559->tx_ring[tx_descriptor_add];

        CYG_ASSERT( (cyg_uint8 *)p_txcb >= i82559_heap_base, "txcb under" );
        CYG_ASSERT( (cyg_uint8 *)p_txcb <  i82559_heap_free, "txcb over" );

        WRITEMEM16(p_txcb + TxCB_STATUS, 0);
        WRITEMEM16(p_txcb + TxCB_CMD,
                         (TxCB_CMD_TRANSMIT | TxCB_CMD_S
                          | TxCB_CMD_I | TxCB_CMD_EL));
        WRITEMEM32(p_txcb + TxCB_LINK, 
                         HAL_CTOLE32(VIRT_TO_BUS((cyg_uint32)p_txcb)));
        WRITEMEM32(p_txcb + TxCB_TBD_ADDR,
                         HAL_CTOLE32(0xFFFFFFFF));
        WRITEMEM8(p_txcb + TxCB_TBD_NUMBER, 0);
        WRITEMEM8(p_txcb + TxCB_TX_THRESHOLD, 16);
        WRITEMEM16(p_txcb + TxCB_COUNT,
                         HAL_CTOLE16(TxCB_COUNT_EOF | total_len));

        // Copy from the sglist into the txcb
        to_p = p_txcb + TxCB_BUFFER;

        CYG_ASSERT( 0 < sg_len, "sg_len underflow" );
        CYG_ASSERT( MAX_ETH_DRV_SG >= sg_len, "sg_len overflow" );

        for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ ) {
            cyg_uint8 *from_p;
            int l;
            
            from_p = (cyg_uint8 *)(sg_list->buf);
            l = sg_list->len;

            if ( l > total_len )
                l = total_len;

            memcpy( (unsigned char *)to_p, from_p, l );
            to_p += l;
            total_len -= l;

            if ( 0 > total_len ) 
                break; // Should exit via sg_last normally
        }

        CYG_ASSERT( 0 == total_len, "length mismatch in tx" );
        CYG_ASSERT( last_sg == sg_list, "sg count mismatch in tx" );
        CYG_ASSERT( p_txcb + TxCB_BUFFER < to_p, "to_p wild in tx" );
        CYG_ASSERT( p_txcb + TxCB_BUFFER + MAX_TX_PACKET_SIZE >= to_p,
                    "to_p overflow in tx" );
  
        // Next descriptor
        if ( ++tx_descriptor_add >= MAX_TX_DESCRIPTORS)
            tx_descriptor_add = 0;
        p_i82559->tx_descriptor_add = tx_descriptor_add;

        // From this instant, interrupts can advance the world and start,
        // even complete, this tx request...

        if ( p_i82559->tx_descriptor_remove == tx_descriptor_add )
            p_i82559->tx_queue_full = 1;
    }

    // Try advancing the Tx Machine regardless

    // no more interrupts until started
    ints = Mask82559Interrupt(p_i82559);

    // This helps unstick deadly embraces.
    CYG_ASSERT( p_i82559->within_send < 10, "send: Excess send recursions" );
    p_i82559->within_send++;

    // Check that either:
    //     tx is already active, there is other stuff queued,
    // OR  this tx just added is the current active one
    // OR  this tx just added is already complete
    CYG_ASSERT(
        // The machine is busy:
        (p_i82559->tx_in_progress == 1) ||
        // or: The machine is idle and this just added is the next one
        (((p_i82559->tx_descriptor_add-1) == p_i82559->tx_descriptor_active)
         || ((0 == p_i82559->tx_descriptor_add) &&
             ((MAX_TX_DESCRIPTORS-1) == p_i82559->tx_descriptor_active))) ||
        // or: This tx is already complete
        (p_i82559->tx_descriptor_add == p_i82559->tx_descriptor_active),
                "Active/add mismatch" );

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
    {
        int i;
        // The problem is, if DEMUX_ALL, either device can eat the other's
        // interrupts; so we must poll them *both*:
        for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {
            p_i82559 = i82559_priv_array[i];
            if ( p_i82559->active ) {
                TxMachine(p_i82559);
                Acknowledge82559Interrupt(p_i82559);
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
		// We are not prepared to receive a packet now if we are in a polled
		// standalone configuration.
                PacketRxReady(p_i82559);
#endif
            }
        }
    }
    // ensure we look at the correct device at the end
    p_i82559 = (struct i82559 *)sc->driver_private;
#else // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
    // Advance TxMachine atomically
    TxMachine(p_i82559);
    Acknowledge82559Interrupt(p_i82559); // This can eat an Rx interrupt, so
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    // We are not prepared to receive a packet now if we are in a polled
    // standalone configuration.
    PacketRxReady(p_i82559);
#endif
#endif // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL

    p_i82559->within_send--;
    UnMask82559Interrupt(p_i82559, ints);
}

// ------------------------------------------------------------------------
//
//  Function : i82559_reset
//
// ------------------------------------------------------------------------
static void
i82559_reset(struct i82559* p_i82559)
{
    cyg_uint32 ioaddr = p_i82559->io_address;

    // First do soft reset. This must be done before the hard reset,
    // otherwise we may create havoc on the PCI bus.
    // Wait 20 uSecs afterwards for chip to settle.
    OUTL(I82559_SELECTIVE_RESET, ioaddr + SCBPort);
    udelay(20);
  
    // Do hard reset now that the controller is off the PCI bus.
    // Wait 20 uSecs afterwards for chip to settle.
    OUTL(I82559_RESET, ioaddr + SCBPort);
    udelay(20);

    // Set the base addresses
    wait_for_cmd_done(ioaddr, WAIT_RU);
    OUTL(0, ioaddr + SCBPointer);
    OUTW(SCB_M | RUC_ADDR_LOAD, ioaddr + SCBCmd);

    wait_for_cmd_done(ioaddr, WAIT_CU);
    OUTL(0, ioaddr + SCBPointer);
    OUTW(SCB_M | CU_ADDR_LOAD, ioaddr + SCBCmd);
}

// ------------------------------------------------------------------------
//
//                       INTERRUPT HANDLERS
//
// ------------------------------------------------------------------------

static cyg_uint32
eth_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct i82559* p_i82559 = (struct i82559 *)data;
    cyg_uint16 status;
    cyg_uint32 ioaddr;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_isr: Bad device pointer %x\n", p_i82559 );
#endif
        return 0;
    }

    ioaddr = p_i82559->io_address;
    status = INW(ioaddr + SCBStatus);

    // Acknowledge all INT sources that were active
    OUTW( status & SCB_INTACK_MASK, ioaddr + SCBStatus);
    // (see pages 6-10 & 6-90)

#ifdef KEEP_STATISTICS
    statistics[p_i82559->index].interrupts++;

    // receiver left ready state ?
    if ( status & SCB_STATUS_RNR )
        statistics[p_i82559->index].rx_resource++;

    // frame receive interrupt ?
    if ( status & SCB_STATUS_FR )
        statistics[p_i82559->index].rx_count++;

    // transmit interrupt ?
    if ( status & SCB_STATUS_CX )
        statistics[p_i82559->index].tx_complete++;
#endif

    // Advance the Tx Machine regardless
    TxMachine(p_i82559);

    // it should have settled down now...
    Acknowledge82559Interrupt(p_i82559);

    return CYG_ISR_CALL_DSR;        // schedule DSR
}


// ------------------------------------------------------------------------
#if defined( CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT ) || \
    defined( CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL )
static cyg_uint32
eth_mux_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    int i;
    struct i82559* p_i82559;

    for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {
        p_i82559 = i82559_priv_array[i];
        if ( p_i82559->active )
            (void)eth_isr( vector, (cyg_addrword_t)p_i82559 );
    }

    return CYG_ISR_CALL_DSR;
}
#endif

// ------------------------------------------------------------------------

static void
eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // This conditioning out is necessary because of explicit calls to this
    // DSR - which would not ever be called in the case of a polled mode
    // usage ie. in RedBoot.
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    struct i82559* p_i82559 = (struct i82559 *)data;
    struct cyg_netdevtab_entry *ndp =
        (struct cyg_netdevtab_entry *)(p_i82559->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    // but here, it must be a *sc:
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
#else
# ifndef CYGPKG_REDBOOT
#  error Empty i82559 ethernet DSR is compiled.  Is this what you want?
# endif
#endif
}

// ------------------------------------------------------------------------
// Deliver routine:
#if defined( CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT ) || \
    defined( CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL )

void
i82559_deliver(struct eth_drv_sc *sc)
{
    int i;
    struct i82559* p_i82559;

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
    again:
#endif

    // Since this must mux all devices, the incoming arg is ignored.
    for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {
        p_i82559 = i82559_priv_array[i];
        if ( p_i82559->active ) {

            // See if the Tx machine is wedged - reset if so:
            Check82559TxLockupTimeout(p_i82559);

            // First pass any rx data up the stack
            PacketRxReady(p_i82559);

            // Then scan for completed Txen and inform the stack
            TxDone(p_i82559);
        }
    }

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
    {
        int retry = 0;
        for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {
            p_i82559 = i82559_priv_array[i];
            if ( p_i82559->active ) {
                if ( CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559) ) {
                    int ints = Mask82559Interrupt(p_i82559);
                    retry++;
#ifdef CYGDBG_USE_ASSERTS
                    missed_interrupt.deliver++;
#endif
                    eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );
                    UnMask82559Interrupt(p_i82559,ints);
                }
            }
            
        }
        if ( retry )
            goto again;
    }
#endif
}

#else // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL : Simplex version:

void
i82559_deliver(struct eth_drv_sc *sc)
{
    struct i82559* p_i82559 = (struct i82559 *)(sc->driver_private);

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
 again:
#endif
    // See if the Tx machine is wedged - reset if so:
    Check82559TxLockupTimeout(p_i82559);

    // First pass any rx data up the stack
    PacketRxReady(p_i82559);

    // Then scan for completed Txen and inform the stack
    TxDone(p_i82559);

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
    if ( CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559) ) {
        int ints = Mask82559Interrupt(p_i82559);
#ifdef CYGDBG_USE_ASSERTS
        missed_interrupt.deliver++;
#endif
        eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );
        UnMask82559Interrupt(p_i82559,ints);
        goto again;
    }
#endif
}

#endif // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL

// ------------------------------------------------------------------------
// Device table entry to operate the chip in a polled mode.
// Only diddle the interface we were asked to!

void
i82559_poll(struct eth_drv_sc *sc)
{
    struct i82559 *p_i82559;
    int ints;
    p_i82559 = (struct i82559 *)sc->driver_private;
    
    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_poll: Bad device pointer %x\n", p_i82559 );
#endif
        return;
    }

    // Do these atomically
    ints = Mask82559Interrupt(p_i82559);

    // As it happens, this driver always requests the DSR to be called:
    (void)eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );

    // (no harm in calling this ints-off also, when polled)
    i82559_deliver( sc );

    Acknowledge82559Interrupt(p_i82559);
    UnMask82559Interrupt(p_i82559, ints);
}

// ------------------------------------------------------------------------
// Determine interrupt vector used by a device - for attaching GDB stubs
// packet handler.
int
i82559_int_vector(struct eth_drv_sc *sc)
{
    struct i82559 *p_i82559;
    p_i82559 = (struct i82559 *)sc->driver_private;
    return (p_i82559->vector);
}

#if 0
int
i82559_int_op( struct eth_drv_sc *sc, int mask)
{
    struct i82559 *p_i82559;
    p_i82559 = (struct i82559 *)sc->driver_private;

    if ( 1 == mask )
        return Mask82559Interrupt( p_i82559 );

    if ( 0 == mask )
        UnMask82559Interrupt( p_i82559, 0x0fffffff ); // enable all

    return 0;
}
#endif
    

// ------------------------------------------------------------------------
//
//  Function : pci_init_find_82559s
//
// This is called exactly once at the start of time to:
//  o scan the PCI bus for objects
//  o record them in the device table
//  o acquire all the info needed for the driver to access them
//  o instantiate interrupts for them
//  o attach those interrupts appropriately
// ------------------------------------------------------------------------
static cyg_pci_match_func find_82559s_match_func;

// Intel 82559 and 82557 are virtually identical, with different
// dev codes; also 82559ER (cutdown) = 0x1209.
static cyg_bool
find_82559s_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
    return
        (0x8086 == v) &&
        ((0x1030 == d) ||
         (0x1229 == d) ||
         (0x1209 == d) ||
         (0x1029 == d) ||
         (0x2449 == d));
}

static int
pci_init_find_82559s( void )
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
    cyg_uint16 cmd;
    int device_index;
    int found_devices = 0;

#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
    static cyg_handle_t mux_interrupt_handle = 0;
 #endif

#ifdef DEBUG
    db_printf("pci_init_find_82559s()\n");
#endif

    // allocate memory to be used in ioctls later
    if (mem_reserved_ioctl != (void*)0) {
#ifdef DEBUG
        db_printf("pci_init_find_82559s() called > once\n");
#endif
        return 0;
    }

    // First initialize the heap in PCI window'd memory
    i82559_heap_size = CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE;
    i82559_heap_base = (cyg_uint8 *)CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE;
    i82559_heap_free = i82559_heap_base;

    mem_reserved_ioctl = pciwindow_mem_alloc(MAX_MEM_RESERVED_IOCTL);     

    cyg_pci_init();
#ifdef DEBUG
    db_printf("Finished cyg_pci_init();\n");
#endif

    devid = CYG_PCI_NULL_DEVID;

    for (device_index = 0; 
         device_index < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT;
         device_index++) {
        struct i82559 *p_i82559 = i82559_priv_array[device_index];

        p_i82559->index = device_index;

        // See above for find_82559s_match_func - it selects any of several
        // variants.  This is necessary in case we have multiple mixed-type
        // devices on one board in arbitrary orders.
        if (cyg_pci_find_matching( &find_82559s_match_func, NULL, &devid )) {
#ifdef DEBUG
            db_printf("eth%d = 82559\n", device_index);
#endif
	    // Allocate it a stats window:
	    p_i82559->p_statistics = pciwindow_mem_alloc(sizeof(I82559_COUNTERS));

            cyg_pci_get_device_info(devid, &dev_info);

            p_i82559->interrupt_handle = 0; // Flag not attached.
            if (cyg_pci_translate_interrupt(&dev_info, &p_i82559->vector)) {
#ifdef DEBUG
                db_printf(" Wired to HAL vector %d\n", p_i82559->vector);
#endif

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                cyg_drv_interrupt_create(
                    p_i82559->vector,
                    0,                  // Priority - unused
                    (CYG_ADDRWORD)p_i82559, // Data item passed to ISR & DSR
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
                    eth_mux_isr,        // ISR
#else
                    eth_isr,            // ISR
#endif
                    eth_dsr,            // DSR
                    &p_i82559->interrupt_handle, // handle to intr obj
                    &p_i82559->interrupt_object ); // space for int obj

                cyg_drv_interrupt_attach(p_i82559->interrupt_handle);

                // Don't unmask the interrupt yet, that could get us into a
                // race.
#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
                // ALSO attach it to MUX interrupt for multiplexed
                // interrupts.  This is for certain boards where the
                // PCI backplane is wired "straight through" instead of
                // with a rotation of interrupt lines in the different
                // slots.
                {
                    static cyg_interrupt mux_interrupt_object;

                    if ( ! mux_interrupt_handle ) {
#ifdef DEBUG
                        db_printf(" Also attaching to HAL vector %d\n", 
                                  CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT);
#endif
                        cyg_drv_interrupt_create(
                            CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT,
                            0,              // Priority - unused
                            (CYG_ADDRWORD)p_i82559,// Data item passed to ISR and DSR
                            eth_mux_isr,    // ISR
                            eth_dsr,        // DSR
                            &mux_interrupt_handle,
                            &mux_interrupt_object );
                        
                        cyg_drv_interrupt_attach(mux_interrupt_handle);
                    }
                }
#endif // CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
            }
            else {
                p_i82559->vector=0;
#ifdef DEBUG
                db_printf(" Does not generate interrupts.\n");
#endif
            }

            if (cyg_pci_configure_device(&dev_info)) {
#ifdef DEBUG
                int i;
                db_printf("Found device on bus %d, devfn 0x%02x:\n",
                          CYG_PCI_DEV_GET_BUS(devid),
                          CYG_PCI_DEV_GET_DEVFN(devid));

                if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                    db_printf(" Note that board is active. Probed"
                              " sizes and CPU addresses invalid!\n");
                }
                db_printf(" Vendor    0x%04x", dev_info.vendor);
                db_printf("\n Device    0x%04x", dev_info.device);
                db_printf("\n Command   0x%04x, Status 0x%04x\n",
                          dev_info.command, dev_info.status);
                
                db_printf(" Class/Rev 0x%08x", dev_info.class_rev);
                db_printf("\n Header 0x%02x\n", dev_info.header_type);

                db_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                          dev_info.header.normal.sub_vendor, 
                          dev_info.header.normal.sub_id);

                for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                    db_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                    db_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                              dev_info.base_size[i], dev_info.base_map[i]);
                }
                db_printf(" eth%d configured\n", device_index);
#endif
                found_devices++;
                p_i82559->found = 1;
                p_i82559->active = 0;
                p_i82559->devid = devid;
                p_i82559->memory_address = dev_info.base_map[0];
                p_i82559->io_address = dev_info.base_map[1];
#ifdef DEBUG
                db_printf(" memory address = 0x%08x\n", dev_info.base_map[0]);
                db_printf(" I/O address = 0x%08x\n", dev_info.base_map[1]);
#endif
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_USE_MEMORY
                // Use the memory address instead of I/O.  Some devices just
                // don't want to talk using the I/O registers :-(
                p_i82559->io_address = dev_info.base_map[0];
#endif

                // Don't use cyg_pci_set_device_info since it clears
                // some of the fields we want to print out below.
                cyg_pci_read_config_uint16(dev_info.devid,
                                           CYG_PCI_CFG_COMMAND, &cmd);
                cmd |= (CYG_PCI_CFG_COMMAND_IO         // enable I/O space
                        | CYG_PCI_CFG_COMMAND_MEMORY   // enable memory space
                        | CYG_PCI_CFG_COMMAND_MASTER); // enable bus master
                cyg_pci_write_config_uint16(dev_info.devid,
                                            CYG_PCI_CFG_COMMAND, cmd);

                // Now the PCI part of the device is configured, reset
                // it. This should make it safe to enable the
                // interrupt
                i82559_reset(p_i82559);

                // This is the indicator for "uses an interrupt"
                if (p_i82559->interrupt_handle != 0) {
                    cyg_drv_interrupt_acknowledge(p_i82559->vector);
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                    cyg_drv_interrupt_unmask(p_i82559->vector);
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                }
#ifdef DEBUG
                db_printf(" **** Device enabled for I/O and Memory "
                          "and Bus Master\n");
#endif
            }
            else {
                p_i82559->found = 0;
                p_i82559->active = 0;
#ifdef DEBUG
                db_printf("Failed to configure device %d\n",device_index);
#endif
            }
        }
        else {
            p_i82559->found = 0;
            p_i82559->active = 0;
#ifdef DEBUG
            db_printf("eth%d not found\n", device_index);
#endif
        }
    }

    if (0 == found_devices)
        return 0;

#ifdef CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT
    // Now enable the mux shared interrupt if it is in use
    if (mux_interrupt_handle) {
        cyg_drv_interrupt_acknowledge(CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT);
        cyg_drv_interrupt_unmask(CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT);
    }
#endif

    // Now a delay to ensure the hardware has "come up" before you try to
    // use it.  Yes, really, the full 2 seconds.  It's only really
    // necessary if DEBUG is off - otherwise all that printout wastes
    // enough time.  No kidding.
    udelay( 2000000 );
    return 1;
}

// ------------------------------------------------------------------------
//
//  Function : i82559_set_multicast
//
// ------------------------------------------------------------------------
#ifdef ETH_DRV_SET_MC_LIST
static int i82559_set_multicast(struct i82559* p_i82559,
                                int num_addrs,
                                cyg_uint8 *address_list )
{
    cyg_uint32  ioaddr;
    volatile CFG *ccs;
    volatile cyg_uint8* config_bytes;
    cyg_uint16 status;
    int count;
    int i;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "eth_set_promiscuos_mode: Bad device pointer %x\n",
                   p_i82559 );
#endif
        return -1;
    }

    ioaddr = p_i82559->io_address;  
    wait_for_cmd_done(ioaddr, WAIT_CU); 
    // load cu base address = 0 */ 
    OUTL(0, ioaddr + SCBPointer);         
    // 32 bit linear addressing used
    OUTW(SCB_M | CU_ADDR_LOAD, ioaddr + SCBCmd);
    // wait for SCB command complete
    wait_for_cmd_done(ioaddr, WAIT_CU);   
  
    // Check the malloc we did earlier worked
    ccs = (CFG *)mem_reserved_ioctl;
    if (ccs == (void*)0) 
        return 2; // Failed

    // Prepare header
    WRITEMEM16(ccs + CFG_CMD, 
                     (CFG_CMD_EL | CFG_CMD_SUSPEND | CFG_CMD_MULTICAST));
    WRITEMEM16(ccs + CFG_STATUS, 0);
    WRITEMEM32(ccs + CFG_CB_LINK_OFFSET, 
                     HAL_CTOLE32(VIRT_TO_BUS((cyg_uint32)ccs)));

    count = 6 * num_addrs; // byte count

    WRITEMEM16(ccs + CFG_MC_LIST_BYTES,
                     HAL_CTOLE16( count ) );
               
    config_bytes = ccs + CFG_MC_LIST_DATA;
    
    for ( i = 0; i < count; i++ )
        config_bytes[i] = address_list[i];
    
    // Let chip read configuration
    wait_for_cmd_done(ioaddr, WAIT_CU);

    OUTL(VIRT_TO_BUS(ccs), ioaddr + SCBPointer);
    OUTW(SCB_M | CU_START, ioaddr + SCBCmd);

    // ...and wait for it to complete operation
    count = 10000;
    do {
      udelay(1);
      READMEM16(ccs + CFG_STATUS, status);
    } while (0 == (status & CFG_STATUS_C) && (count-- > 0));

    // Check status
    if ((status & (CFG_STATUS_C | CFG_STATUS_OK)) 
        != (CFG_STATUS_C | CFG_STATUS_OK)) {
        // Failed!
#ifdef DEBUG
        os_printf("%s:%d Multicast setup failed\n", __FUNCTION__, __LINE__);
#endif
        return 1;
    }

    wait_for_cmd_done(ioaddr, WAIT_CU);
    /* load pointer to Rx Ring */
    
    OUTL(VIRT_TO_BUS(p_i82559->rx_ring[0]), ioaddr + SCBPointer);
    OUTW(RUC_START, ioaddr + SCBCmd);

    return 0;
}
#endif // ETH_DRV_SET_MC_ALL

// ------------------------------------------------------------------------
//
//  Function : i82559_configure
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------

static int i82559_configure(struct i82559* p_i82559, int promisc,
                            int oversized, int multicast_all)
{
    cyg_uint32  ioaddr;
    volatile CFG *ccs;
    volatile cyg_uint8* config_bytes;
    cyg_uint16 status;
    int count;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "eth_set_promiscuos_mode: Bad device pointer %x\n",
                   p_i82559 );
#endif
        return -1;
    }

    ioaddr = p_i82559->io_address;  
    wait_for_cmd_done(ioaddr, WAIT_CU); 
    // load cu base address = 0 */ 
    OUTL(0, ioaddr + SCBPointer);         
    // 32 bit linear addressing used
    OUTW(SCB_M | CU_ADDR_LOAD, ioaddr + SCBCmd);
    // wait for SCB command complete
    wait_for_cmd_done(ioaddr, WAIT_CU);   
  
    // Check the malloc we did earlier worked
    ccs = (CFG *)mem_reserved_ioctl;
    if (ccs == (void*)0) 
        return 2; // Failed

    // Prepare header
    WRITEMEM16(ccs + CFG_CMD, 
                     (CFG_CMD_EL | CFG_CMD_SUSPEND | CFG_CMD_CONFIGURE));
    WRITEMEM16(ccs + CFG_STATUS, 0);
    WRITEMEM32(ccs + CFG_CB_LINK_OFFSET, 
                     HAL_CTOLE32(VIRT_TO_BUS((cyg_uint32)ccs)));

    // Default values from the Intel Manual
    config_bytes = ccs + CFG_BYTES;

    config_bytes[0]= 22; // All 22 bytes
    config_bytes[1]=0xc;
    config_bytes[2]=0x0;
    config_bytes[3]=0x0;
    config_bytes[4]=0x0;
    config_bytes[5]=0x0;
    config_bytes[6]=0x32 | (promisc ? 0x80 : 0x00); //   | 0x32 for small stats,
    config_bytes[7]=0x00 | (promisc ? 0x00 : 0x01); //\  | 0x12 for stats with PAUSE stats
    config_bytes[8]=0x01; // [7]:discard short frames  \ | 0x16 for PAUSE + TCO stats
    config_bytes[9]=0x0;
    config_bytes[10]=0x28;
    config_bytes[11]=0x0;
    config_bytes[12]=0x60;
    config_bytes[13]=0x0;          // arp
    config_bytes[14]=0x0;          // arp
    
    config_bytes[15]=0x80 | (promisc ? 1 : 0); // 0x81: promiscuous mode set
                                               // 0x80: normal mode
    config_bytes[16]=0x0;
    config_bytes[17]=0x40;
    config_bytes[18]=0x72 | (oversized ? 8 : 0); // Keep the Padding Enable bit

    config_bytes[19]=0x80; // FDX pin enable is the default
    config_bytes[20]=0x3f; // the default
    config_bytes[21]=0x05 | (multicast_all ? 8 : 0); // Bit 3 is MultiCastALL enable
    
    // Let chip read configuration
    wait_for_cmd_done(ioaddr, WAIT_CU);

    OUTL(VIRT_TO_BUS(ccs), ioaddr + SCBPointer);
    OUTW(SCB_M | CU_START, ioaddr + SCBCmd);

    // ...and wait for it to complete operation
    count = 10000;
    do {
      udelay(1);
      READMEM16(ccs + CFG_STATUS, status);
    } while (0 == (status & CFG_STATUS_C) && (count-- > 0));

    // Check status
    if ((status & (CFG_STATUS_C | CFG_STATUS_OK)) 
        != (CFG_STATUS_C | CFG_STATUS_OK)) {
        // Failed!
#ifdef DEBUG
        os_printf("%s:%d Config update failed\n", __FUNCTION__, __LINE__);
#endif
        return 1;
    }

    wait_for_cmd_done(ioaddr, WAIT_CU);
    /* load pointer to Rx Ring */
    
    OUTL(VIRT_TO_BUS(p_i82559->rx_ring[0]), ioaddr + SCBPointer);
    OUTW(RUC_START, ioaddr + SCBCmd);

    return 0;
}

// ------------------------------------------------------------------------
#if 0
// The following table below doesn't really work with all cards.
// The safest way to proceed is to do a read-modify-write of the
// EEPROM contents rather than relying on a static table.

#ifdef CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM
// We use this as a templete when writing a new MAC address into the
// eeproms. The MAC address in the first few bytes is over written
// with the correct MAC address and then the whole lot is programmed
// into the serial EEPROM. The checksum is calculated on the fly and
// sent instead of the last two bytes.
// The values are copied from the Intel EtherPro10/100+ &c devices
// in the EBSA boards.
static cyg_uint16 eeprom_burn[64] = { 
/* halfword addresses! */
/*  0: */  0x9000, 0x8c27, 0x8257, 0x0203,
/*  4: */  0x0000, 0x0201, 0x4701, 0x0000,
/*  8: */  0x7213, 0x8306, 0x40A2, 0x000c,
/*  C: */  0x8086, 0x0000, 0x0000, 0x0000,
/* 10: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 14: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 18: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 1C: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 20: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 24: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 28: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 2C: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 30: */  0x0128, 0x0000, 0x0000, 0x0000,
/* 34: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 38: */  0x0000, 0x0000, 0x0000, 0x0000,
/* 3C: */  0x0000, 0x0000, 0x0000, 0x0000,
};
#endif
#endif

// ------------------------------------------------------------------------
//
//  Function : eth_set_mac_address
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------
static int
eth_set_mac_address(struct i82559* p_i82559, char *addr, int eeprom)
{
    cyg_uint32  ioaddr;
    cyg_uint16 status;
    volatile CFG *ccs;
    volatile cyg_uint8* config_bytes;
    int count;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "eth_set_mac_address : Bad device pointer %x\n",
                   p_i82559 );
#endif
        return -1;
    }

    ioaddr = p_i82559->io_address;      
    
    ccs = (CFG *)mem_reserved_ioctl;
    if (ccs == (void*)0)
        return 2;

    WRITEMEM16(ccs + CFG_CMD, 
                     (CFG_CMD_EL | CFG_CMD_SUSPEND | CFG_CMD_IAS));
    WRITEMEM16(ccs + CFG_STATUS, 0);
    WRITEMEM32(ccs + CFG_CB_LINK_OFFSET, 
                     HAL_CTOLE32(VIRT_TO_BUS((cyg_uint32)ccs)));

    config_bytes = ccs + CFG_BYTES;
    memcpy((char *)(config_bytes),addr,6);
    config_bytes[6]=0x0;
    config_bytes[7]=0x0;

    // Let chip read new ESA
    wait_for_cmd_done(ioaddr, WAIT_CU);
    OUTL(VIRT_TO_BUS(ccs), ioaddr + SCBPointer); 
    OUTW(SCB_M | CU_START, ioaddr + SCBCmd);    

    // ...and wait for it to complete operation
    count = 1000;
    do {
      READMEM16(ccs + CFG_STATUS, status);
      udelay(1);
    } while (0 == (status & CFG_STATUS_C) && (count-- > 0));

    // Check status
    READMEM16(ccs + CFG_STATUS, status);
    if ((status & (CFG_STATUS_C | CFG_STATUS_OK)) 
        != (CFG_STATUS_C | CFG_STATUS_OK)) {
#ifdef DEBUG
        os_printf("%s:%d ESA update failed\n", __FUNCTION__, __LINE__);
#endif
        return 3;
    }

#ifdef CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM
    if ( 0 == eeprom 
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM
    || CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM != p_i82559->index
#endif
        ) {
        // record the MAC address in the device structure
        p_i82559->mac_address[0] = addr[0];
        p_i82559->mac_address[1] = addr[1];
        p_i82559->mac_address[2] = addr[2];
        p_i82559->mac_address[3] = addr[3];
        p_i82559->mac_address[4] = addr[4];
        p_i82559->mac_address[5] = addr[5];
        p_i82559->mac_addr_ok = 1;

#ifdef DEBUG
        os_printf( "No EEPROM write: MAC Address = %02X %02X %02X %02X %02X %02X (ok %d)\n",
                   p_i82559->mac_address[0],
                   p_i82559->mac_address[1],
                   p_i82559->mac_address[2],
                   p_i82559->mac_address[3],
                   p_i82559->mac_address[4],
                   p_i82559->mac_address[5],
                   p_i82559->mac_addr_ok       );
#endif
    } else {
        int checksum, i, count;
        // (this is the length of the *EEPROM*s address, not MAC address)
        int addr_length;
        cyg_uint16 eeprom_burn[64];

        addr_length = get_eeprom_size( ioaddr );

        for (i = 0; i < (1 << addr_length); i++)
            eeprom_burn[i] = read_eeprom( ioaddr, i, addr_length );

        // now set this address in the device eeprom ....
        eeprom_burn[0] = addr[0] | (addr[1] << 8);
        eeprom_burn[1] = addr[2] | (addr[3] << 8);
        eeprom_burn[2] = addr[4] | (addr[5] << 8);

        // No idea what these were for...
        // eeprom_burn[20] &= 0xfe;   
        // eeprom_burn[20] |= p_i82559->index;   
        
        program_eeprom( ioaddr, addr_length, eeprom_burn );
   
        // update 82559 driver data structure ...
        udelay( 100000 );

        // by reading EEPROM to get the mac address back
        for (checksum = 0, i = 0, count = 0; count < (1 << addr_length); count++) {
            cyg_uint16 value;
            // read word from eeprom
            value = read_eeprom(ioaddr, count, addr_length);
            checksum += value;
            if (count < 3) {
                p_i82559->mac_address[i++] = value & 0xFF;
                p_i82559->mac_address[i++] = (value >> 8) & 0xFF;
            }
        }
    
#ifdef DEBUG
        os_printf("eth_set_mac_address[WRITE_EEPROM]: MAC Address = %02X %02X %02X %02X %02X %02X\n",
                  p_i82559->mac_address[0], p_i82559->mac_address[1],
                  p_i82559->mac_address[2], p_i82559->mac_address[3],
                  p_i82559->mac_address[4], p_i82559->mac_address[5]);
#endif

        p_i82559->mac_addr_ok = 1;

        for ( i = 0, count = 0; i < 6; i++ )
            if ( p_i82559->mac_address[i] != addr[i] )
                count++;

        if ( count ) {
#ifdef DEBUG
            os_printf( "Warning: MAC Address read back wrong!  %d bytes differ.\n",
                       count );
#endif
            p_i82559->mac_addr_ok = 0;
        }

        // If the EEPROM checksum is wrong, the MAC address read from
        // the EEPROM is probably wrong as well. In that case, we
        // don't set mac_addr_ok.
        if ((checksum & 0xFFFF) != 0xBABA)  {
#ifdef DEBUG
            os_printf( "Warning: Invalid EEPROM checksum %04X for device %d\n",
                       checksum, p_i82559->index);
#endif
            p_i82559->mac_addr_ok = 0;
        }
    }
#else // CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM

    // record the MAC address in the device structure
    p_i82559->mac_address[0] = addr[0];
    p_i82559->mac_address[1] = addr[1];
    p_i82559->mac_address[2] = addr[2];
    p_i82559->mac_address[3] = addr[3];
    p_i82559->mac_address[4] = addr[4];
    p_i82559->mac_address[5] = addr[5];
    p_i82559->mac_addr_ok = 1;

#ifdef DEBUG
    os_printf( "Set MAC Address = %02X %02X %02X %02X %02X %02X (ok %d)\n",
               p_i82559->mac_address[0],
               p_i82559->mac_address[1],
               p_i82559->mac_address[2],
               p_i82559->mac_address[3],
               p_i82559->mac_address[4],
               p_i82559->mac_address[5],
               p_i82559->mac_addr_ok       );
#endif

#endif // ! CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM

    return p_i82559->mac_addr_ok ? 0 : 1;
}

#ifdef CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM
// ------------------------------------------------------------------------
static void
write_eeprom(long ioaddr, int location, int addr_len, unsigned short value)
{
    int ee_addr = ioaddr + SCBeeprom;
    int write_cmd = location | EE_WRITE_CMD(addr_len); 
    int i;
    
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay( 100 );
    OUTW(EE_ENB, ee_addr);
    eeprom_delay( 100 );

//    os_printf("\n write_eeprom : write_cmd : %x",write_cmd);  
//    os_printf("\n addr_len : %x  value : %x ",addr_len,value);  

    /* Shift the write command bits out. */
    for (i = (addr_len+2); i >= 0; i--) {
        short dataval = (write_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
        OUTW(EE_ENB | dataval, ee_addr);
        eeprom_delay(100);
        OUTW(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(150);
    }
    OUTW(EE_ENB, ee_addr);
        
    for (i = 15; i >= 0; i--) {
        short dataval = (value & (1 << i)) ? EE_DATA_WRITE : 0;
        OUTW(EE_ENB | dataval, ee_addr);
        eeprom_delay(100);
        OUTW(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
        eeprom_delay(150);
    }

    /* Terminate the EEPROM access. */
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay(150000); // let the write take effect
}

// ------------------------------------------------------------------------
static int
write_enable_eeprom(long ioaddr,  int addr_len)
{
    int ee_addr = ioaddr + SCBeeprom;
    int write_en_cmd = EE_WRITE_EN_CMD(addr_len); 
    int i;

    OUTW(EE_ENB & ~EE_CS, ee_addr);
    OUTW(EE_ENB, ee_addr);

#ifdef DEBUG_82559
    os_printf("write_en_cmd : %x",write_en_cmd);
#endif

    // Shift the wr/er enable command bits out.
    for (i = (addr_len+2); i >= 0; i--) {
	short dataval = (write_en_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
	OUTW(EE_ENB | dataval, ee_addr);
	eeprom_delay(100);
	OUTW(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
	eeprom_delay(150);
    }

    // Terminate the EEPROM access.
    OUTW(EE_ENB & ~EE_CS, ee_addr);
    eeprom_delay(EEPROM_DONE_DELAY);
}


// ------------------------------------------------------------------------
static void
program_eeprom(cyg_uint32 ioaddr, cyg_uint32 eeprom_size, cyg_uint8 *data)
{
  cyg_uint32 i;
  cyg_uint16 checksum = 0;
  cyg_uint16 value;

  // First enable erase/write operations on the eeprom.
  // This is done through the EWEN instruction.
  write_enable_eeprom( ioaddr, eeprom_size );

  for (i=0 ; i< (1 << eeprom_size) ; i++) {
    value = ((unsigned short *)data)[i];
    checksum += value;
#ifdef DEBUG_82559
    os_printf("\n i : %x ... value to be written : %x",i,value);
#endif
    write_eeprom( ioaddr, i, eeprom_size, value);
#ifdef DEBUG_82559
    os_printf("\n val read : %x ",read_eeprom(ioaddr,i,eeprom_size));
#endif
  }
  value = 0xBABA - checksum;
#ifdef DEBUG_82559
  os_printf("\n i : %x ... checksum adjustment val to be written : %x",i,value);
#endif
  write_eeprom( ioaddr, i, eeprom_size, value );
}

// ------------------------------------------------------------------------
#endif // ! CYGPKG_DEVS_ETH_INTEL_I82559_WRITE_EEPROM


// ------------------------------------------------------------------------
//
//  Function : eth_get_mac_address
//
// ------------------------------------------------------------------------
#ifdef ETH_DRV_GET_MAC_ADDRESS
static int
eth_get_mac_address(struct i82559* p_i82559, char *addr)
{
    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "eth_get_mac_address : Bad device pointer %x\n",
                   p_i82559 );
#endif
        return -1;
    }

    memcpy( addr, (char *)(&p_i82559->mac_address[0]), 6 );
    return 0;
}
#endif
// ------------------------------------------------------------------------
//
//  Function : i82559_ioctl
//
// ------------------------------------------------------------------------
static int
i82559_ioctl(struct eth_drv_sc *sc, unsigned long key,
             void *data, int data_length)
{
    struct i82559 *p_i82559;

    p_i82559 = (struct i82559 *)sc->driver_private;

    IF_BAD_82559( p_i82559 ) {
#ifdef DEBUG
        os_printf( "i82559_ioctl/control: Bad device pointer %x\n", p_i82559 );
#endif
        return -1;
    }

#ifdef ioctlDEBUG
    db_printf( "i82559_ioctl: device eth%d at %x; key is 0x%x, data at %x[%d]\n",
               p_i82559->index, p_i82559, key, data, data_length );
#endif

    switch ( key ) {

#ifdef ETH_DRV_SET_MAC_ADDRESS
    case ETH_DRV_SET_MAC_ADDRESS:
        if ( 6 != data_length )
            return -2;
        return eth_set_mac_address( p_i82559, data, 1 /* do write eeprom */ );
#endif

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        return eth_get_mac_address( p_i82559, data );
#endif

#ifdef ETH_DRV_GET_IF_STATS_UD
    case ETH_DRV_GET_IF_STATS_UD: // UD == UPDATE
        ETH_STATS_INIT( sc );    // so UPDATE the statistics structure
#endif
        // drop through
#ifdef ETH_DRV_GET_IF_STATS
    case ETH_DRV_GET_IF_STATS:
#endif
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
    {
        struct ether_drv_stats *p = (struct ether_drv_stats *)data;
        int i;
        static unsigned char my_chipset[]
            = { ETH_DEV_DOT3STATSETHERCHIPSET };

        strcpy( p->description, CYGDAT_DEVS_ETH_DESCRIPTION );
        CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        for ( i = 0; i < SNMP_CHIPSET_LEN; i++ )
            if ( 0 == (p->snmp_chipset[i] = my_chipset[i]) )
                break;

        i = i82559_status( sc );

        if ( !( i & GEN_STATUS_LINK) ) {
            p->operational = 2;         // LINK DOWN
            p->duplex = 1;              // UNKNOWN
            p->speed = 0;
        }
        else {
            p->operational = 3;            // LINK UP
            p->duplex = (i & GEN_STATUS_FDX) ? 3 : 2; // 2 = SIMPLEX, 3 = DUPLEX
            p->speed = ((i & GEN_STATUS_100MBPS) ? 100 : 10) * 1000000;
        }

#ifdef KEEP_STATISTICS
        {
            I82559_COUNTERS *pc = &i82559_counters[ p_i82559->index ];
            STATISTICS      *ps = &statistics[      p_i82559->index ];

            // Admit to it...
            p->supports_dot3        = true;

            // Those commented out are not available on this chip.

            p->tx_good              = pc->tx_good             ;
            p->tx_max_collisions    = pc->tx_max_collisions   ;
            p->tx_late_collisions   = pc->tx_late_collisions  ;
            p->tx_underrun          = pc->tx_underrun         ;
            p->tx_carrier_loss      = pc->tx_carrier_loss     ;
            p->tx_deferred          = pc->tx_deferred         ;
            //p->tx_sqetesterrors   = pc->tx_sqetesterrors    ;
            p->tx_single_collisions = pc->tx_single_collisions;
            p->tx_mult_collisions   = pc->tx_mult_collisions  ;
            p->tx_total_collisions  = pc->tx_total_collisions ;
            p->rx_good              = pc->rx_good             ;
            p->rx_crc_errors        = pc->rx_crc_errors       ;
            p->rx_align_errors      = pc->rx_align_errors     ;
            p->rx_resource_errors   = pc->rx_resource_errors  ;
            p->rx_overrun_errors    = pc->rx_overrun_errors   ;
            p->rx_collisions        = pc->rx_collisions       ;
            p->rx_short_frames      = pc->rx_short_frames     ;
            //p->rx_too_long_frames = pc->rx_too_long_frames  ;
            //p->rx_symbol_errors   = pc->rx_symbol_errors    ;
        
            p->interrupts           = ps->interrupts          ;
            p->rx_count             = ps->rx_count            ;
            p->rx_deliver           = ps->rx_deliver          ;
            p->rx_resource          = ps->rx_resource         ;
            p->rx_restart           = ps->rx_restart          ;
            p->tx_count             = ps->tx_count            ;
            p->tx_complete          = ps->tx_complete         ;
            p->tx_dropped           = ps->tx_dropped          ;
        }
#endif // KEEP_STATISTICS

        p->tx_queue_len = MAX_TX_DESCRIPTORS;

        return 0; // OK
    }
#endif

#ifdef ETH_DRV_SET_MC_LIST
    case ETH_DRV_SET_MC_LIST:    {
            struct eth_drv_mc_list *mcl = (struct eth_drv_mc_list *)data;

            i82559_reset(p_i82559);
            ResetRxRing( p_i82559 );
            ResetTxRing( p_i82559 );

            p_i82559->multicast_all = 0;

            i82559_configure(p_i82559,
                             p_i82559->promisc,
                             p_i82559->oversized,
                             p_i82559->multicast_all );
            
            i82559_set_multicast( p_i82559,
                                  mcl->len,
                                  &(mcl->addrs[0][0]) );

            i82559_restart(p_i82559);
            return 0;
    }
#endif // ETH_DRV_SET_MC_LIST

#ifdef ETH_DRV_SET_MC_ALL
    case ETH_DRV_SET_MC_ALL:
            i82559_reset(p_i82559);
            ResetRxRing( p_i82559 );
            ResetTxRing( p_i82559 );

            p_i82559->multicast_all = 1;

            i82559_configure(p_i82559,
                             p_i82559->promisc,
                             p_i82559->oversized,
                             p_i82559->multicast_all );
            
            i82559_restart(p_i82559);
            return 0;
#endif // ETH_DRV_SET_MC_ALL

    default:
        break;
    }
    return -1;
}

// ------------------------------------------------------------------------
//
// Statistics update...
//
// ------------------------------------------------------------------------

#ifdef KEEP_STATISTICS
#ifdef CYGDBG_DEVS_ETH_INTEL_I82559_KEEP_82559_STATISTICS
void
update_statistics(struct i82559* p_i82559)
{
    I82559_COUNTERS *p_statistics;
    cyg_uint32 *p_counter;
    cyg_uint32 *p_register;
    int reg_count, ints;
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
    struct i82559* op_i82559 = p_i82559;
#endif

    ints = Mask82559Interrupt(p_i82559);

    // This points to the shared memory stats area/command block
    p_statistics = (I82559_COUNTERS *)(p_i82559->p_statistics);

    if ( (p_statistics->done & 0xFFFF) == 0xA007 ) {
        p_counter = (cyg_uint32 *)&i82559_counters[ p_i82559->index ];
        p_register = (cyg_uint32 *)p_statistics;
        for ( reg_count = 0;
              reg_count < sizeof( I82559_COUNTERS ) / sizeof( cyg_uint32 ) - 1;
              reg_count++ ) {
            *p_counter += *p_register;
            p_counter++;
            p_register++;
        }
        p_statistics->done = 0;
        // start register dump
        wait_for_cmd_done(p_i82559->io_address, WAIT_CU);
        OUTW(CU_DUMPSTATS, p_i82559->io_address + SCBCmd);
    }

#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
    {
        int i;
        // The problem is, if DEMUX_ALL, either device can eat the other's
        // interrupts; so we must poll them *both*:
        for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT; i++) {
            p_i82559 = i82559_priv_array[i];
            if ( p_i82559->active ) {
                // See if the Tx machine is wedged - reset if so:
                Check82559TxLockupTimeout(p_i82559);
                TxMachine(p_i82559);
                Acknowledge82559Interrupt(p_i82559);
                PacketRxReady(p_i82559);
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
                if ( CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559) ) {
#ifdef CYGDBG_USE_ASSERTS
                    missed_interrupt.stats++;
#endif
                    eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );
                    eth_dsr( p_i82559->vector, 1, (cyg_addrword_t)p_i82559 );
                }
#endif
            }
        }
    }
    // ensure we look at the correct device at the end
    p_i82559 = op_i82559;
#else // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
    // See if the Tx machine is wedged - reset if so:
    Check82559TxLockupTimeout(p_i82559);
    TxMachine(p_i82559);
    Acknowledge82559Interrupt(p_i82559); // This can eat an Rx interrupt, so
    PacketRxReady(p_i82559);
#ifdef CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT
    if ( CYGHWR_DEVS_ETH_INTEL_I82559_MISSED_INTERRUPT(p_i82559) ) {
#ifdef CYGDBG_USE_ASSERTS
        missed_interrupt.stats++;
#endif
        eth_isr( p_i82559->vector, (cyg_addrword_t)p_i82559 );
        eth_dsr( p_i82559->vector, 1, (cyg_addrword_t)p_i82559 );
    }
#endif
#endif // no CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL

    UnMask82559Interrupt(p_i82559, ints);
}
#endif
#endif // KEEP_STATISTICS

// ------------------------------------------------------------------------

// EOF if_i82559.c
