#ifndef _VGA_SUPPORT_H_
#define _VGA_SUPPORT_H_
//==========================================================================
//
//        vga_support.h
//
//        VGA support routines
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          2001-09-29
// Description:   Simple VGA support
//####DESCRIPTIONEND####

struct vga_info {
    short height, width;  // Pixels
    short bpp;            // Depth (bits/pixel)
    short type;
    short rlen;           // Length of one raster line in bytes
    short access_size;    // Data path width to frame buffer
    short stride;         // Offset (in bytes) between elements
    void  *fb;            // Frame buffer
    void  *ctlr;          // Controller regs
    void  (*off)(void);   // Turn screen off
    void  (*on)(void);    // Turn screen on
};

// Frame buffer types - used by MicroWindows
#define FB_TRUE_RGB565 0x01
#define FB_TRUE_RGB555 0x02  

// Exported functions
void vga_init(cyg_uint32 *ctlr);
void vga_clear(void);
int  vga_getinfo(struct vga_info *info);
void vga_on(bool enable);
#ifdef CYGSEM_UE250_VGA_COMM 
void vga_screen_clear(void);
void vga_moveto(int X, int Y);
void vga_putc(cyg_uint8 c);
int  _vga_printf(char const *fmt, ...);
void vga_comm_init(cyg_uint32 *addr);
#endif

#endif //  _VGA_SUPPORT_H_
