#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt, jskov 
// Contributors: hmt, jskov
// Date:         1999-08-09
// Purpose:      Intel EBSA285 PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
// Note:         Based on information in 
//               "21285 Core Logic for SA-110 Microprocessor"
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal_arm_ebsa285.h>

#include <cyg/hal/hal_ebsa285.h>

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_platform_ints.h>  // Interrupt vectors

// Memory map is 1-1
#define CYGARC_PHYSICAL_ADDRESS(_x_) (_x_)

// The PCI resources required by the EBSA are hardcoded to the lowest
// addresses in the PCI address space, thus:
// PCI Memory Space
#define EBSA_SDRAM_PCI_ADDR   0
#define EBSA_SDRAM_PCI_SIZE   (CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_SIZE)
#define EBSA_CSR_MEM_PCI_ADDR (EBSA_SDRAM_PCI_SIZE)
#define EBSA_CSR_MEM_PCI_SIZE 0x80
// PCI IO Space
#define EBSA_CSR_IO_PCI_ADDR  0
#define EBSA_CSR_IO_PCI_SIZE  0x80

// Incidentally I now understand why it's necessary to force PCI reset
// (2000-08-07) - with RedBoot in ROM, the RAM app was unconditionally
// reinitializing the PCI bus when it was already initialized, without
// resetting it.  We cannot play the same game here as with
// hal_platform_setup.h - I tried - because otherwise the net cannot
// re-initialize itself; the scan for devices fails.

// Initialize the PCI bus.
#define HAL_PCI_INIT()                                                   \
    CYG_MACRO_START                                                      \
    cyg_uint32 __tmp, __tmp2;                                            \
                                                                         \
    /* Assert PCI_reset                                               */ \
    HAL_READ_UINT32(SA110_CONTROL, __tmp);                               \
    __tmp &= ~SA110_CONTROL_RST_I;                                       \
    HAL_WRITE_UINT32(SA110_CONTROL, __tmp);                              \
                                                                         \
    /* Disable PCI Outbound interrupts                                */ \
    /* (according to 7-14 SA110_OUT_INT_MASK is not accessible        */ \
    /* by SA-100)                                                     */ \
    HAL_WRITE_UINT32(SA110_OUT_INT_STATUS,                               \
                     SA110_OUT_INT_STATUS_DOORBELL_INT                   \
                     |SA110_OUT_INT_STATUS_OUTBOUND_INT);                \
                                                                         \
    /* Disable Doorbells                                              */ \
    HAL_WRITE_UINT32(SA110_DOORBELL_PCI_MASK, 0);                        \
    HAL_WRITE_UINT32(SA110_DOORBELL_SA_MASK, 0);                         \
                                                                         \
    /* Map high PCI address bits to 0                                 */ \
    HAL_WRITE_UINT32(SA110_PCI_ADDR_EXT, 0);                             \
                                                                         \
    /* Interrupt ID to 1                                              */ \
    HAL_WRITE_UINT16(SA110_PCI_CFG_INT_LINE, 0x0100);                    \
                                                                         \
    /* Remove PCI_reset                                               */ \
    HAL_READ_UINT32(SA110_CONTROL, __tmp);                               \
    __tmp |= SA110_CONTROL_RST_I;                                        \
    HAL_WRITE_UINT32(SA110_CONTROL, __tmp);                              \
                                                                         \
    /* Open a window to SDRAM from PCI address space                  */ \
    HAL_WRITE_UINT32(SA110_SDRAM_BASE_ADDRESS_MASK,                      \
           ((CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_SIZE-1) & 0xfffc0000));  \
    HAL_WRITE_UINT32(SA110_SDRAM_BASE_ADDRESS_OFFSET,                    \
             CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE);                   \
                                                                         \
    /* Only init PCI if central function is set and */                   \
    /* standalone bit is cleared                    */                   \
    HAL_READ_UINT32(SA110_CONTROL, __tmp);                               \
    HAL_READ_UINT32(SA110_XBUS_XCS2, __tmp2);                            \
    if ((__tmp & SA110_CONTROL_CFN) == SA110_CONTROL_CFN                 \
        && (__tmp2 & SA110_XBUS_XCS2_PCI_DISABLE) == 0) {                \
                                                                         \
        /* Don't respond to any commands                              */ \
        HAL_WRITE_UINT16(SA110_PCI_CFG_COMMAND, 0);                      \
                                                                         \
        /* Set up default addresses for board's resources.            */ \
        HAL_WRITE_UINT32(SA110_PCI_CFG_CSR_MEM_BAR,                      \
                         EBSA_CSR_MEM_PCI_ADDR);                         \
        HAL_WRITE_UINT32(SA110_PCI_CFG_CSR_IO_BAR, EBSA_CSR_IO_PCI_ADDR);\
        HAL_WRITE_UINT32(SA110_PCI_CFG_SDRAM_BAR, EBSA_SDRAM_PCI_ADDR);  \
                                                                         \
        /* Respond to I/O space & Memory transactions.                */ \
        HAL_WRITE_UINT32(SA110_PCI_CFG_COMMAND,                          \
                         CYG_PCI_CFG_COMMAND_INVALIDATE                  \
                         |CYG_PCI_CFG_COMMAND_IO                         \
                         |CYG_PCI_CFG_COMMAND_MEMORY                     \
                         |CYG_PCI_CFG_COMMAND_MASTER);                   \
    }                                                                    \
    /* Signal PCI_init_complete                                       */ \
    HAL_READ_UINT32(SA110_CONTROL, __tmp);                               \
    __tmp |= SA110_CONTROL_INIT_COMPLETE;                                \
    HAL_WRITE_UINT32(SA110_CONTROL, __tmp);                              \
                                                                         \
    CYG_MACRO_END


// Compute address necessary to access PCI config space for the given
// bus and device.
#define HAL_PCI_CONFIG_ADDRESS( __bus, __devfn, __offset )               \
    ({                                                                   \
    cyg_uint32 __addr;                                                   \
    cyg_uint32 __dev = CYG_PCI_DEV_GET_DEV(__devfn);                     \
    if (0 == __bus) {                                                    \
        if (0 == __dev) {                                                \
            /* Access self via CSR base */                               \
            __addr = SA110_CONTROL_STATUS_BASE;                          \
        } else {                                                         \
            /* Page 3-17, table 3-5: bits 15-11 generate PCI address */  \
            __addr = SA110_PCI_CONFIG0_BASE | 0x00c00000 | (__dev << 11);\
        }                                                                \
    } else {                                                             \
        __addr = SA110_PCI_CONFIG1_BASE | (__bus << 16) | (__dev << 11); \
    }                                                                    \
    __addr |= CYG_PCI_DEV_GET_FN(__devfn) << 8;                          \
    __addr |= __offset;                                                  \
    __addr;                                                              \
    })

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
    HAL_READ_UINT8(HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset), __val)
    
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val ) \
    HAL_READ_UINT16(HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset), __val)

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val ) \
    HAL_READ_UINT32(HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset), __val)

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )  \
    HAL_WRITE_UINT8(HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset), __val)

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
    HAL_WRITE_UINT16(HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset), __val)

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset), __val)

//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY (EBSA_SDRAM_PCI_SIZE+EBSA_CSR_MEM_PCI_SIZE)
#define HAL_PCI_ALLOC_BASE_IO     (EBSA_CSR_IO_PCI_SIZE)

// This is where the PCI spaces are mapped in the CPU's address space.
#define HAL_PCI_PHYSICAL_MEMORY_BASE    0x80000000
#define HAL_PCI_PHYSICAL_IO_BASE        0x7c000000

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint8 __req;                                                          \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);       \
    if (0 != __req) {                                                         \
        /* Interrupt assignment as 21285 sees them. (From                 */  \
        /* EBSA285 Eval Board Reference Manual, 3.4 Interrupt Assignment) */  \
        CYG_ADDRWORD __translation[4] = {                                     \
            CYGNUM_HAL_INTERRUPT_IRQ_IN_1,  /* INTC# */                       \
            CYGNUM_HAL_INTERRUPT_IRQ_IN_0,  /* INTB# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_IRQ,   /* INTA# */                       \
            CYGNUM_HAL_INTERRUPT_IRQ_IN_3}; /* INTD# */                       \
                                                                              \
        /* The PCI lines from the different slots are wired like this  */     \
        /* on the PCI backplane:                                       */     \
        /*                pin6A     pin7B    pin7A   pin8B             */     \
        /* System Slot    INTA#     INTB#    INTC#   INTD#             */     \
        /* I/O Slot 1     INTB#     INTC#    INTD#   INTA#             */     \
        /* I/O Slot 2     INTC#     INTD#    INTA#   INTB#             */     \
        /* I/O Slot 3     INTD#     INTA#    INTB#   INTC#             */     \
        /* I/O Slot 4     INTA#     INTB#    INTC#   INTD#             */     \
        /*                                                             */     \
        /* (From PCI Development Backplane, 3.2.2 Interrupts)          */     \
        /*                                                             */     \
        /* Devsel signals are wired to, resulting in device IDs:       */     \
        /* I/O Slot 1     AD19 / dev 8       [(8+1)&3 = 1]             */     \
        /* I/O Slot 2     AD18 / dev 7       [(7+1)&3 = 0]             */     \
        /* I/O Slot 3     AD17 / dev 6       [(6+1)&3 = 3]             */     \
        /* I/O Slot 4     AD16 / dev 5       [(5+1)&3 = 2]             */     \
        /*                                                             */     \
        /* (From PCI Development Backplane, 3.2.1 General)             */     \
        /*                                                             */     \
        /* The observant reader will notice that the array does not    */     \
        /* match the table of how interrupts are wired. The array      */     \
        /* does however match observed behavior of the hardware:       */     \
        /*                                                             */     \
        /* Observed interrupts with an Intel ethernet card             */     \
        /* put in the slots in turn and set to generate interrupts:    */     \
        /*  slot 1/intA# (dev 8): caused host INTB#                    */     \
        /*  slot 2/intA# (dev 7): caused host INTC#                    */     \
        /*  slot 3/intA# (dev 6): caused host INTD#                    */     \
        /*  slot 4/intA# (dev 5): caused host INTA#                    */     \
                                                                              \
        __vec = __translation[((__req+CYG_PCI_DEV_GET_DEV(__devfn))&3)];      \
        __valid = true;                                                       \
    } else {                                                                  \
        /* Device will not generate interrupt requests. */                    \
        __valid = false;                                                      \
    }                                                                         \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
