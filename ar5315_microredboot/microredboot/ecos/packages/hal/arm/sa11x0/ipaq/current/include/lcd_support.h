#ifndef _LCD_SUPPORT_H_
#define _LCD_SUPPORT_H_
//==========================================================================
//
//        lcd_support.h
//
//        SA1110/iPAQ - LCD support routines
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          2001-02-24
// Description:   Simple LCD support
//####DESCRIPTIONEND####

struct lcd_info {
    short height, width;  // Pixels
    short bpp;            // Depth (bits/pixel)
    short type;
    short rlen;           // Length of one raster line in bytes
    void  *fb;            // Frame buffer
};

// Frame buffer types
#define FB_TRUE_RGB565 0x01

// Exported functions
void lcd_init(int depth);
void lcd_clear(void);
void lcd_brightness(int level);
void lcd_moveto(int X, int Y);
void lcd_putc(cyg_int8 c);
int  lcd_printf(char const *fmt, ...);
void lcd_setbg(int red, int green, int blue);
void lcd_setfg(int red, int green, int blue);
void lcd_comm_init(void);
int  lcd_getinfo(struct lcd_info *info);
void lcd_on(bool enable);

#endif //  _LCD_SUPPORT_H_
