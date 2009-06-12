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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt, jskov 
// Contributors: hmt, jskov, gthomas
// Date:         2002-07-23
// Purpose:      TAMS MOAB (PowerPC 405GPr) PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

//-----------------------------------------------------------------------------
//
// PCI support
//
#define CYGARC_PHYSICAL_ADDRESS(x) ((unsigned long)(x) & 0x7FFFFFFF)
#define CYGARC_VIRTUAL_ADDRESS(x) ((unsigned long)(x) & 0x7FFFFFFF)

// Restrict device [slot] space
#define CYG_PCI_MAX_BUS                       1  // Only one BUS
#define CYG_PCI_MIN_DEV                       1  // Slots start at 11
#define CYG_PCI_MAX_DEV                      22  // ... and end at 31
#define _IRQ1 CYGNUM_HAL_INTERRUPT_IRQ1
#define _IRQ2 CYGNUM_HAL_INTERRUPT_IRQ2
#define _IRQ3 CYGNUM_HAL_INTERRUPT_IRQ3
#define _IRQ4 CYGNUM_HAL_INTERRUPT_IRQ4
#define CYG_PCI_IRQ_MAP                                                         \
/*                                                                              \
 * This mapping comes from this table, based on how the backplane is wired      \
 * IRQ assignments (Acrosser):                                                  \
 *                                                                              \
 *               CPU        Slot1      Slot2    Slot3    Slot4                  \
 * IRQ1     PCI INTD        INTB       INTA     INTD      na                    \
 * IRQ2     PCI INTC        INTA       INTD     INTC      na                    \
 * IRQ3     PCI INTB        INTD       INTC     INTB      na                    \
 * IRQ4     PCI INTA        INTC       INTB     INTA      na                    \
 *                                                                              \
 *      PCI IDSEL/INTPIN->INTLINE                                               \
 *      A       B       C       D                                               \
 */                                                                             \
{                                                                               \
    {_IRQ1, _IRQ4, _IRQ3, _IRQ2},   /* IDSEL 1 - 2nd LAN */                     \
    {_IRQ3, _IRQ3, _IRQ3, _IRQ3},   /* IDSEL 2 - USB */                         \
    {   -1,    -1,    -1,    -1},   /* IDSEL 3 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 4 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 5 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 6 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 7 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 8 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 9 - unavailable */                 \
    {   -1,    -1,    -1,    -1},   /* IDSEL 10 - unavailable */                \
    {   -1,    -1,    -1,    -1},   /* IDSEL 11 - unavailable */                \
    {   -1,    -1,    -1,    -1},   /* IDSEL 12 - unavailable */                \
    {   -1,    -1,    -1,    -1},   /* IDSEL 13 - unavailable */                \
    {   -1,    -1,    -1,    -1},   /* IDSEL 14 - unavailable */                \
    {   -1,    -1,    -1,    -1},   /* IDSEL 15 - unavailable */                \
    {   -1,    -1,    -1,    -1},   /* IDSEL 16 - unavailable */                \
    {_IRQ1, _IRQ4, _IRQ3, _IRQ2},   /* IDSEL 17 - PCI slot 2 */                 \
    {_IRQ4, _IRQ3, _IRQ2, _IRQ1},   /* IDSEL 18 - PCI slot 3 */                 \
    {_IRQ1, _IRQ4, _IRQ3, _IRQ2},   /* IDSEL 19 - PCI slot 2 */                 \
    {_IRQ2, _IRQ1, _IRQ4, _IRQ3},   /* IDSEL 20 - PCI slot 1 */                 \
}

//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY                 0x00000000
#define HAL_PCI_ALLOC_BASE_IO                     0x00800000

// This is where the PCI spaces are mapped in the CPU's address space.
#define HAL_PCI_PHYSICAL_MEMORY_BASE              0xA0000000
#define HAL_PCI_PHYSICAL_IO_BASE                  0xE8000000

#if 1 // This is an old-school idea about how to handle PCI devices
// These seem to be defined multiple ways?
#define CYGMEM_SECTION_pci_window                 0x03F00000
#define CYGMEM_SECTION_pci_window_SIZE            0x00100000
#endif

// IDE support
#define HAL_IDE_NUM_CONTROLLERS 2  // One card, two controllers

externC cyg_uint8 cyg_hal_plf_ide_read_uint8(int ctlr, cyg_uint32 reg);
externC void cyg_hal_plf_ide_write_uint8(int ctlr, cyg_uint32 reg, cyg_uint8 val);
externC cyg_uint16 cyg_hal_plf_ide_read_uint16(int ctlr, cyg_uint32 reg);
externC void cyg_hal_plf_ide_write_uint16(int ctlr, cyg_uint32 reg, cyg_uint16 val);
externC void cyg_hal_plf_ide_write_control(int ctlr, cyg_uint8 val);
externC int cyg_hal_plf_ide_init(void);

#define HAL_IDE_READ_UINT8( __ctlr, __reg, __val) \
    __val = cyg_hal_plf_ide_read_uint8((__ctlr),  (__reg))

#define HAL_IDE_READ_UINT16( __ctlr, __reg, __val) \
    __val = cyg_hal_plf_ide_read_uint16((__ctlr),  (__reg))

#define HAL_IDE_WRITE_UINT8( __ctlr, __reg, __val) \
    cyg_hal_plf_ide_write_uint8((__ctlr),  (__reg), (__val))

#define HAL_IDE_WRITE_UINT16( __ctlr, __reg, __val) \
    cyg_hal_plf_ide_write_uint16((__ctlr),  (__reg), (__val))

#define HAL_IDE_WRITE_CONTROL( __ctlr, __val) \
    cyg_hal_plf_ide_write_control((__ctlr),  (__val))

#define HAL_IDE_INIT() cyg_hal_plf_ide_init()


//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
