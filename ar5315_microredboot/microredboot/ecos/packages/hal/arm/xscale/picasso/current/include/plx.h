#ifndef CYGONCE_PLX_H
#define CYGONCE_PLX_H

//=============================================================================
//
//      plx.h
//
//      Description of PCI Local Bus (PLX) resources
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================

#define LOCALBUS_CONFIG_OFFSET  0x0
#define LOCALBUS_OFFSET         0x100

#ifndef _DEFINE_VARS
#define __global externC
#else
#define __global
#endif
__global cyg_uint32 _plx_config_addr;
__global cyg_uint32 _plx_localbus_addr;
#undef __global

#define plx_config_readl(a) (*(volatile cyg_uint32 *)(_plx_config_addr + (a)))
#define plx_config_writel(v,a)  (*(volatile cyg_uint32 *)(_plx_config_addr + (a)))=(v)

#define localbus_readb(a) (*(volatile cyg_uint8 *)(_plx_localbus_addr + (a)))
#define localbus_readw(a) (*(volatile cyg_uint16 *)(_plx_localbus_addr + (a)))
#define localbus_readl(a) (*(volatile cyg_uint32 *)(_plx_localbus_addr + (a)))
#define localbus_writeb(v,a) (*(volatile cyg_uint8 *)(_plx_localbus_addr + (a)))=(v)
#define localbus_writew(v,a) (*(volatile cyg_uint16 *)(_plx_localbus_addr + (a)))=(v)
#define localbus_writel(v,a) (*(volatile cyg_uint32 *)(_plx_localbus_addr + (a)))=(v)

#define display_readb(a) (*(volatile cyg_uint8 *)(PCI_MEM_BASE2 + DISPLAY_MEM_BASE + (a)))
#define display_readw(a) (*(volatile cyg_uint16 *)(PCI_MEM_BASE2 + DISPLAY_MEM_BASE + (a)))
#define display_readl(a) (*(volatile cyg_uint32 *)(PCI_MEM_BASE2 + DISPLAY_MEM_BASE + (a)))
#define display_writeb(v,a) (*(volatile cyg_uint8 *)(PCI_MEM_BASE2 + DISPLAY_MEM_BASE + (a)))=(v)
#define display_writew(v,a) (*(volatile cyg_uint16 *)(PCI_MEM_BASE2 + DISPLAY_MEM_BASE + (a)))=(v)
#define display_writel(v,a) (*(volatile cyg_uint32 *)(PCI_MEM_BASE2 + DISPLAY_MEM_BASE + (a)))=(v)

  
#define LOCALBUS_IRQ_MASK 0x4010
#define LOCALBUS_IRQ_STATUS 0x4014

#define FDC37C672_CONFIG 0xfc0
#define FDC37C672_INDEX  0xfc0
#define FDC37C672_DATA   0xfc4

#define ASCII_DISPLAY_BASE 0x4020
  
#define ADV471_PIXEL_READ_MASK    0x14

#endif // CYGONCE_PLX_H
