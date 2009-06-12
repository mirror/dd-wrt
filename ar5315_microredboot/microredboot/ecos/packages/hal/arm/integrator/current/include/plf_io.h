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
// Author(s):    Philippe Robin
// Contributors: David A Rusling
// Date:         November 7, 2000
// Purpose:      Integrator PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal_arm_integrator.h>

#include <cyg/hal/hal_integrator.h>

#include <cyg/hal/hal_platform_ints.h>  // Interrupt vectors

__externC void cyg_plf_pci_init(void);

// Initialization of the PCI bus.
#define HAL_PCI_INIT() cyg_plf_pci_init()


// V3 access routines
#define _V3Write16(o,v) (*(volatile cyg_uint16 *)(PCI_V3_BASE + (cyg_uint32)(o)) \
					 = (cyg_uint16)(v))
#define _V3Read16(o)    (*(volatile cyg_uint16 *)(PCI_V3_BASE + (cyg_uint32)(o)))

#define _V3Write32(o,v) (*(volatile cyg_uint32 *)(PCI_V3_BASE + (cyg_uint32)(o)) \
					= (cyg_uint32)(v))
#define _V3Read32(o)    (*(volatile cyg_uint32 *)(PCI_V3_BASE + (cyg_uint32)(o)))

// _V3OpenConfigWindow - open V3 configuration window
#define _V3OpenConfigWindow() 							\
    {										\
    /* Set up base0 to see all 512Mbytes of memory space (not	     */		\
    /* prefetchable), this frees up base1 for re-use by configuration*/		\
    /* memory */								\
										\
    _V3Write32 (V3_LB_BASE0, ((INTEGRATOR_PCI_BASE & 0xFFF00000) |		\
			     0x90 | V3_LB_BASE_M_ENABLE));			\
    /* Set up base1 to point into configuration space, note that MAP1 */	\
    /* register is set up by pciMakeConfigAddress(). */				\
										\
    _V3Write32 (V3_LB_BASE1, ((CPU_PCI_CNFG_ADRS & 0xFFF00000) |		\
			     0x40 | V3_LB_BASE_M_ENABLE));			\
    }

// _V3CloseConfigWindow - close V3 configuration window
#define _V3CloseConfigWindow()							\
    {										\
    /* Reassign base1 for use by prefetchable PCI memory */			\
    _V3Write32 (V3_LB_BASE1, (((INTEGRATOR_PCI_BASE + SZ_256M) & 0xFFF00000)	\
					| 0x84 | V3_LB_BASE_M_ENABLE));		\
    _V3Write16 (V3_LB_MAP1,							\
	    (((INTEGRATOR_PCI_BASE + SZ_256M) & 0xFFF00000) >> 16) | 0x0006);	\
										\
    /* And shrink base0 back to a 256M window (NOTE: MAP0 already correct) */	\
										\
    _V3Write32 (V3_LB_BASE0, ((INTEGRATOR_PCI_BASE & 0xFFF00000) |		\
			     0x80 | V3_LB_BASE_M_ENABLE));			\
    }

// Compute address necessary to access PCI config space for the given
// bus and device.
#define HAL_PCI_CONFIG_ADDRESS( __bus, __devfn, __offset ) \
    ({                                                                   		\
    cyg_uint32 __address, __devicebit;							\
    cyg_uint16 __mapaddress;								\
    cyg_uint32 __dev = CYG_PCI_DEV_GET_DEV(__devfn);	/* FIXME to check!! (slot?) */	\
											\
    if (__bus == 0) {									\
	/* local bus segment so need a type 0 config cycle */				\
        /* build the PCI configuration "address" with one-hot in A31-A11 */		\
        __address = PCI_CONFIG_BASE;							\
        __address |= ((__devfn & 0x07) << 8);						\
        __address |= __offset & 0xFF;							\
        __mapaddress = 0x000A;    /* 101=>config cycle, 0=>A1=A0=0 */			\
        __devicebit = (1 << (__dev + 11));						\
											\
        if ((__devicebit & 0xFF000000) != 0) {						\
            /* high order bits are handled by the MAP register */			\
            __mapaddress |= (__devicebit >> 16);					\
        } else {									\
            /* low order bits handled directly in the address */			\
            __address |= __devicebit;							\
        }										\
    } else {	/* bus !=0 */								\
        /* not the local bus segment so need a type 1 config cycle */			\
        /* A31-A24 are don't care (so clear to 0) */					\
        __mapaddress = 0x000B;    /* 101=>config cycle, 1=>A1&A0 from PCI_CFG */	\
        __address = PCI_CONFIG_BASE;							\
        __address |= ((__bus & 0xFF) << 16);  	/* bits 23..16 = bus number 	*/	\
        __address |= ((__dev & 0x1F) << 11);  	/* bits 15..11 = device number  */	\
        __address |= ((__devfn & 0x07) << 8);  	/* bits 10..8  = function number*/	\
        __address |= __offset & 0xFF;  		/* bits  7..0  = register number*/	\
    }											\
    _V3Write16(V3_LB_MAP1, __mapaddress);						\
											\
    __address;										\
    })

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  		\
    {										\
    _V3OpenConfigWindow();							\
    __val = *(volatile cyg_uint8 *)HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset); 	\
    _V3CloseConfigWindow();							\
    }

#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val )  		\
    {										\
    _V3OpenConfigWindow();							\
    __val = *(volatile cyg_uint16 *)HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset); 	\
    _V3CloseConfigWindow();							\
    }

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val )			\
    {											\
    _V3OpenConfigWindow();								\
    __val = *(volatile cyg_uint16 *)HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset);		\
    __val |= (*(volatile cyg_uint16 *)HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, (__offset+2)))<<16;	\
    _V3CloseConfigWindow();								\
    }

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )		\
    {										\
    _V3OpenConfigWindow();							\
    *(volatile cyg_uint8 *)HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset) = __val; 	\
    _V3CloseConfigWindow();							\
    }

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val )		\
    {										\
    _V3OpenConfigWindow();							\
    *(volatile cyg_uint16 *)HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset) = __val; 	\
    _V3CloseConfigWindow();							\
    }

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val )					  \
    {													  \
    _V3OpenConfigWindow();										  \
    *(volatile cyg_uint16 *)HAL_PCI_CONFIG_ADDRESS((__bus), (__devfn), (__offset)) = ((__val) & 0xFFFF);		  \
    *(volatile cyg_uint16 *)HAL_PCI_CONFIG_ADDRESS((__bus), (__devfn), ((__offset)+2)) = (((__val)>>16) & 0xFFFF); \
    _V3CloseConfigWindow();										  \
    }

//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY 0
#define HAL_PCI_ALLOC_BASE_IO     0x4000

// This is where the PCI spaces are mapped in the CPU's address space.
#define HAL_PCI_PHYSICAL_MEMORY_BASE    (PCI_MEM_BASE)
#define HAL_PCI_PHYSICAL_IO_BASE        (PCI_IO_BASE)

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define INTA CYGNUM_HAL_INTERRUPT_PCIINT0
#define INTB CYGNUM_HAL_INTERRUPT_PCIINT1
#define INTC CYGNUM_HAL_INTERRUPT_PCIINT2
#define INTD CYGNUM_HAL_INTERRUPT_PCIINT3

#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint8 __req;                                                          \
    cyg_uint32 __dev;							      \
    /* DANGER! For now this is the SDM interrupt table... */	      	      \
    static const cyg_uint8 irq_tab[12][4] = {	      			      \
		/* INTA  INTB  INTC  INTD */				      \
		{INTA, INTB, INTC, INTD},  /* idsel 20, slot  9 */	      \
		{INTB, INTC, INTD, INTA},  /* idsel 21, slot 10 */	      \
		{INTC, INTD, INTA, INTB},  /* idsel 22, slot 11 */	      \
		{INTD, INTA, INTB, INTC},  /* idsel 23, slot 12 */	      \
		{INTA, INTB, INTC, INTD},  /* idsel 24, slot 13 */	      \
		{INTB, INTC, INTD, INTA},  /* idsel 25, slot 14 */	      \
		{INTC, INTD, INTA, INTB},  /* idsel 26, slot 15 */	      \
		{INTD, INTA, INTB, INTC},  /* idsel 27, slot 16 */	      \
		{INTA, INTB, INTC, INTD},  /* idsel 28, slot 17 */	      \
		{INTB, INTC, INTD, INTA},  /* idsel 29, slot 18 */	      \
		{INTC, INTD, INTA, INTB},  /* idsel 30, slot 19 */	      \
		{INTD, INTA, INTB, INTC}   /* idsel 31, slot 20 */	      \
    };								      	      \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);       \
    __dev  = CYG_PCI_DEV_GET_DEV(__devfn);	/* FIXME to check!! (slot?)*/ \
									      \
    /* if PIN = 0, default to A */					      \
    if (__req == 0)							      \
	__req = 1;							      \
									      \
    __vec = irq_tab[__dev - 9][__req - 1];				      \
    __valid = true;                                                           \
    CYG_MACRO_END

//-----------------------------------------------------------------------------

#define CYGARC_PHYSICAL_ADDRESS(x) (x)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
