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
// Author(s):    dhowells
// Contributors: dmoseley
// Date:         2001-05-17
// Purpose:      ASB2305 platform IO support
// Description:
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#ifndef __ASSEMBLER__
#include <cyg/hal/hal_intr.h>
#endif

#ifdef __ASSEMBLER__
#define HAL_REG_8(x)              x
#define HAL_REG_16(x)             x
#define HAL_REG_32(x)             x
#else
#define HAL_REG_8(x)              (volatile cyg_uint8*)(x)
#define HAL_REG_16(x)             (volatile cyg_uint16*)(x)
#define HAL_REG_32(x)             (volatile cyg_uint32*)(x)
#endif

# define CYGARC_UNCACHED_ADDRESS(x) ((x)|0x20000000)

//-----------------------------------------------------------------------------

/* ASB GPIO Registers */
#define HAL_GPIO_BASE                           0xDB000000

#define HAL_GPIO_0_MODE_OFFSET                  0x0000
#define HAL_GPIO_0_IN_OFFSET                    0x0004
#define HAL_GPIO_0_OUT_OFFSET                   0x0008
#define HAL_GPIO_1_MODE_OFFSET                  0x0100
#define HAL_GPIO_1_IN_OFFSET                    0x0104
#define HAL_GPIO_1_OUT_OFFSET                   0x0108
#define HAL_GPIO_2_MODE_OFFSET                  0x0200
#define HAL_GPIO_2_IN_OFFSET                    0x0204
#define HAL_GPIO_2_OUT_OFFSET                   0x0208
#define HAL_GPIO_3_MODE_OFFSET                  0x0300
#define HAL_GPIO_3_IN_OFFSET                    0x0304
#define HAL_GPIO_3_OUT_OFFSET                   0x0308
#define HAL_GPIO_4_MODE_OFFSET                  0x0400
#define HAL_GPIO_4_IN_OFFSET                    0x0404
#define HAL_GPIO_4_OUT_OFFSET                   0x0408
#define HAL_GPIO_5_MODE_OFFSET                  0x0500
#define HAL_GPIO_5_IN_OFFSET                    0x0504
#define HAL_GPIO_5_OUT_OFFSET                   0x0508

#define HAL_GPIO_0_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_0_MODE_OFFSET)
#define HAL_GPIO_0_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_0_IN_OFFSET)
#define HAL_GPIO_0_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_0_OUT_OFFSET)
#define HAL_GPIO_1_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_1_MODE_OFFSET)
#define HAL_GPIO_1_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_1_IN_OFFSET)
#define HAL_GPIO_1_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_1_OUT_OFFSET)
#define HAL_GPIO_2_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_2_MODE_OFFSET)
#define HAL_GPIO_2_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_2_IN_OFFSET)
#define HAL_GPIO_2_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_2_OUT_OFFSET)
#define HAL_GPIO_3_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_3_MODE_OFFSET)
#define HAL_GPIO_3_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_3_IN_OFFSET)
#define HAL_GPIO_3_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_3_OUT_OFFSET)
#define HAL_GPIO_4_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_4_MODE_OFFSET)
#define HAL_GPIO_4_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_4_IN_OFFSET)
#define HAL_GPIO_4_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_4_OUT_OFFSET)
#define HAL_GPIO_5_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_5_MODE_OFFSET)
#define HAL_GPIO_5_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_5_IN_OFFSET)
#define HAL_GPIO_5_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_5_OUT_OFFSET)

//-----------------------------------------------------------------------------
#define HAL_LED_ADDRESS                         0xA6F90000
#define HAL_GPIO_MODE_ALL_OUTPUT                0x5555


#ifdef __ASSEMBLER__

#  include <cyg/hal/platform.inc>
#  define DEBUG_DISPLAY(hexdig)   hal_diag_led hexdig
#  define DEBUG_DELAY()                                        \
     mov	0x20000, d0;                                       \
0:	 sub    1, d0;                                             \
     bne    0b;                                                \
     nop

#else

extern cyg_uint8 cyg_hal_plf_led_val(CYG_WORD hexdig);
#  define DEBUG_DISPLAY(hexdig)	HAL_WRITE_UINT8(HAL_LED_ADDRESS, cyg_hal_plf_led_val(hexdig))
#  define DEBUG_DELAY()                                        \
   {                                                           \
     volatile int i = 0x80000;                                 \
     while (--i) ;                                             \
   }

#endif

//-----------------------------------------------------------------------------
// PCI access stuff

// Compute address necessary to access PCI config space for the given
// bus and device.
#define HAL_PCI_CONFIG_ADDRESS( __bus, __devfn, __offset )		\
 (0x80000000 | ((__bus) << 16) | ((__devfn) << 8) | ((__offset) & ~3))

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )				\
do {												\
	if ((__bus)==0 && (__devfn)==0) {							\
		HAL_READ_UINT8(0xBE040000+(__offset),(__val));					\
	}											\
	else {											\
		HAL_WRITE_UINT32(0xBFFFFFF8,HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset));	\
		HAL_READ_UINT8(0xBFFFFFFC + ((__offset)&3),(__val));				\
	}											\
} while(0)

#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val )				\
do {												\
	if ((__bus)==0 && (__devfn)==0) {							\
		HAL_READ_UINT16(0xBE040000+(__offset),(__val));					\
	}											\
	else {											\
		HAL_WRITE_UINT32(0xBFFFFFF8,HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset));	\
		HAL_READ_UINT16(0xBFFFFFFC + ((__offset)&2),(__val));				\
	}											\
} while(0)

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val )				\
do {												\
	if ((__bus)==0 && (__devfn)==0) {							\
		HAL_READ_UINT32(0xBE040000+(__offset),(__val));					\
	}											\
	else {											\
		HAL_WRITE_UINT32(0xBFFFFFF8,HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset));	\
		HAL_READ_UINT32(0xBFFFFFFC,(__val));						\
	}											\
} while(0)

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )				\
do {												\
	if ((__bus)==0 && (__devfn)==0) {							\
		HAL_WRITE_UINT8(0xBE040000+(__offset),(__val));					\
	}											\
	else {											\
		HAL_WRITE_UINT32(0xBFFFFFF8,HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset));	\
		HAL_WRITE_UINT8(0xBFFFFFFC + ((__offset)&3),(__val));				\
	}											\
} while(0)

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val )				\
do {												\
	if ((__bus)==0 && (__devfn)==0) {							\
		HAL_WRITE_UINT16(0xBE040000+(__offset),(__val));				\
	}											\
	else {											\
		HAL_WRITE_UINT32(0xBFFFFFF8,HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset));	\
		HAL_WRITE_UINT16(0xBFFFFFFC + ((__offset)&2),(__val));				\
	}											\
} while(0)

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val )				\
do {												\
	if ((__bus)==0 && (__devfn)==0) {							\
		HAL_WRITE_UINT32(0xBE040000+(__offset),(__val));				\
	}											\
	else {											\
		HAL_WRITE_UINT32(0xBFFFFFF8,HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset));	\
		HAL_WRITE_UINT32(0xBFFFFFFC,(__val));						\
	}											\
} while(0)

// Initialize the PCI bus.
#define HAL_PCI_INIT()										  \
do {												  \
	cyg_uint32 devfn;									  \
	cyg_uint16 word;									  \
												  \
	/* we need to set up the bridge _now_ or we won't be able to access the */		  \
	/* PCI config registers */								  \
	HAL_PCI_CFG_READ_UINT32(0,0,CYG_PCI_CFG_COMMAND,word);					  \
	word |= CYG_PCI_CFG_COMMAND_SERR | CYG_PCI_CFG_COMMAND_PARITY;				  \
	word |= CYG_PCI_CFG_COMMAND_MEMORY | CYG_PCI_CFG_COMMAND_IO | CYG_PCI_CFG_COMMAND_MASTER; \
	HAL_PCI_CFG_WRITE_UINT32(0,0,CYG_PCI_CFG_COMMAND,word);					  \
												  \
	HAL_PCI_CFG_WRITE_UINT16(0,0,CYG_PCI_CFG_STATUS,	0xF800);			  \
	HAL_PCI_CFG_WRITE_UINT8 (0,0,CYG_PCI_CFG_LATENCY_TIMER,	0x10);				  \
	HAL_PCI_CFG_WRITE_UINT32(0,0,CYG_PCI_CFG_BAR_0,		0x80000000);			  \
	HAL_PCI_CFG_WRITE_UINT8 (0,0,CYG_PCI_CFG_INT_LINE,	1);				  \
	HAL_PCI_CFG_WRITE_UINT32(0,0,0x48,			0x98000000);			  \
	HAL_PCI_CFG_WRITE_UINT8 (0,0,0x41,			0x00);				  \
	HAL_PCI_CFG_WRITE_UINT8 (0,0,0x42,			0x01);				  \
	HAL_PCI_CFG_WRITE_UINT8 (0,0,0x44,			0x01);				  \
	HAL_PCI_CFG_WRITE_UINT32(0,0,0x50,			0x00000001);			  \
	HAL_PCI_CFG_WRITE_UINT32(0,0,0x58,			0x00000002);			  \
	HAL_PCI_CFG_WRITE_UINT32(0,0,0x5C,			0x00000001);			  \
												  \
	/* we also need to set up the PCI-PCI bridge (no BIOS, you see) */			  \
	devfn = 3<<3 | 0;									  \
												  \
	/* IO: 0x00010000-0x0001ffff */								  \
	HAL_PCI_CFG_WRITE_UINT8 (0,devfn,CYG_PCI_CFG_IO_BASE,		0x01);			  \
	HAL_PCI_CFG_WRITE_UINT16(0,devfn,CYG_PCI_CFG_IO_BASE_UPPER16,	0x0001);		  \
	HAL_PCI_CFG_WRITE_UINT8 (0,devfn,CYG_PCI_CFG_IO_LIMIT,		0xF1);			  \
	HAL_PCI_CFG_WRITE_UINT16(0,devfn,CYG_PCI_CFG_IO_LIMIT_UPPER16,	0x0001);		  \
												  \
	HAL_PCI_CFG_READ_UINT32(0,0,CYG_PCI_CFG_COMMAND,word);					  \
	word |= CYG_PCI_CFG_COMMAND_SERR | CYG_PCI_CFG_COMMAND_PARITY;				  \
	word |= CYG_PCI_CFG_COMMAND_MEMORY | CYG_PCI_CFG_COMMAND_IO | CYG_PCI_CFG_COMMAND_MASTER; \
	HAL_PCI_CFG_WRITE_UINT32(0,0,CYG_PCI_CFG_COMMAND,word);					  \
	HAL_PCI_CFG_WRITE_UINT16(0,devfn,CYG_PCI_CFG_MEM_BASE,		0x1000);		  \
	HAL_PCI_CFG_WRITE_UINT16(0,devfn,CYG_PCI_CFG_MEM_LIMIT,		0x1000);		  \
} while(0)


//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY 0x10000000
#define HAL_PCI_ALLOC_BASE_IO     0x1000

// This is where the PCI spaces are mapped in the CPU's address space.
#define HAL_PCI_PHYSICAL_MEMORY_BASE    0x80000000
#define HAL_PCI_PHYSICAL_IO_BASE        0xBE000000

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
            CYGNUM_HAL_INTERRUPT_RESERVED_170,  /* INTC# */                   \
            CYGNUM_HAL_INTERRUPT_RESERVED_169,  /* INTB# */                   \
            CYGNUM_HAL_INTERRUPT_EXTERNAL_1,    /* INTA# */                   \
            CYGNUM_HAL_INTERRUPT_RESERVED_171}; /* INTD# */                   \
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
// Bus address translation macros
#define HAL_PCI_CPU_TO_BUS(__cpu_addr, __bus_addr)            \
    CYG_MACRO_START                                           \
    (__bus_addr) = (CYG_ADDRESS)((cyg_uint32)(__cpu_addr)&~0x20000000);   \
    CYG_MACRO_END

#define HAL_PCI_BUS_TO_CPU(__bus_addr, __cpu_addr)        \
    CYG_MACRO_START                                       \
    (__cpu_addr) = CYGARC_UNCACHED_ADDRESS(__bus_addr);   \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
