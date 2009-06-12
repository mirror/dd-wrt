//==========================================================================
//
//        lcd_support.c
//
//        Cirrus EDB7xxx eval board LCD support code
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
// Date:          1999-09-01
// Description:   Functions to drive LCD display
//####DESCRIPTIONEND####

// 8x8 Font - from Helios

#define FIRST_CHAR 0x20
#define LAST_CHAR  0x7E
#define FONT_HEIGHT 8
#define FONT_WIDTH  8
#define CURSOR_ON  0x5F
#define CURSOR_OFF 0x20
static char font_table[LAST_CHAR-FIRST_CHAR+1][8] =
{
        {        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /*   */
        {        0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00 }, /* ! */
        {        0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* " */
        {        0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00 }, /* # */
        {        0x30, 0xFC, 0x16, 0x7C, 0xD0, 0x7E, 0x18, 0x00 }, /* $ */
        {        0x06, 0x66, 0x30, 0x18, 0x0C, 0x66, 0x60, 0x00 }, /* % */
        {        0x1C, 0x36, 0x36, 0x1C, 0xB6, 0x66, 0xDC, 0x00 }, /* & */
        {        0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* ' */
        {        0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00 }, /* ( */
        {        0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00 }, /* ) */
        {        0x00, 0x18, 0x7E, 0x3C, 0x7E, 0x18, 0x00, 0x00 }, /* * */
        {        0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00 }, /* + */
        {        0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x0C }, /* , */
        {        0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 }, /* - */
        {        0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 }, /* . */
        {        0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00 }, /* / */
        {        0x3C, 0x66, 0x76, 0x7E, 0x6E, 0x66, 0x3C, 0x00 }, /* 0 */
        {        0x18, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00 }, /* 1 */
        {        0x3C, 0x66, 0x60, 0x30, 0x18, 0x0C, 0x7E, 0x00 }, /* 2 */
        {        0x3C, 0x66, 0x60, 0x38, 0x60, 0x66, 0x3C, 0x00 }, /* 3 */
        {        0x30, 0x38, 0x3C, 0x36, 0x7E, 0x30, 0x30, 0x00 }, /* 4 */
        {        0x7E, 0x06, 0x3E, 0x60, 0x60, 0x66, 0x3C, 0x00 }, /* 5 */
        {        0x38, 0x0C, 0x06, 0x3E, 0x66, 0x66, 0x3C, 0x00 }, /* 6 */
        {        0x7E, 0x60, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00 }, /* 7 */
        {        0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00 }, /* 8 */
        {        0x3C, 0x66, 0x66, 0x7C, 0x60, 0x30, 0x1C, 0x00 }, /* 9 */
        {        0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00 }, /* : */
        {        0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x0C }, /* ; */
        {        0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00 }, /* < */
        {        0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00 }, /* = */
        {        0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00 }, /* > */
        {        0x3C, 0x66, 0x30, 0x18, 0x18, 0x00, 0x18, 0x00 }, /* ? */
        {        0x3C, 0x66, 0x76, 0x56, 0x76, 0x06, 0x3C, 0x00 }, /* @ */
        {        0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00 }, /* A */
        {        0x3E, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3E, 0x00 }, /* B */
        {        0x3C, 0x66, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00 }, /* C */
        {        0x1E, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1E, 0x00 }, /* D */
        {        0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x7E, 0x00 }, /* E */
        {        0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x06, 0x00 }, /* F */
        {        0x3C, 0x66, 0x06, 0x76, 0x66, 0x66, 0x3C, 0x00 }, /* G */
        {        0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00 }, /* H */
        {        0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00 }, /* I */
        {        0x7C, 0x30, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00 }, /* J */
        {        0x66, 0x36, 0x1E, 0x0E, 0x1E, 0x36, 0x66, 0x00 }, /* K */
        {        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x00 }, /* L */
        {        0xC6, 0xEE, 0xFE, 0xD6, 0xD6, 0xC6, 0xC6, 0x00 }, /* M */
        {        0x66, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x66, 0x00 }, /* N */
        {        0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 }, /* O */
        {        0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x00 }, /* P */
        {        0x3C, 0x66, 0x66, 0x66, 0x56, 0x36, 0x6C, 0x00 }, /* Q */
        {        0x3E, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x66, 0x00 }, /* R */
        {        0x3C, 0x66, 0x06, 0x3C, 0x60, 0x66, 0x3C, 0x00 }, /* S */
        {        0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 }, /* T */
        {        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 }, /* U */
        {        0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00 }, /* V */
        {        0xC6, 0xC6, 0xD6, 0xD6, 0xFE, 0xEE, 0xC6, 0x00 }, /* W */
        {        0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00 }, /* X */
        {        0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00 }, /* Y */
        {        0x7E, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x7E, 0x00 }, /* Z */
        {        0x3E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x3E, 0x00 }, /* [ */
        {        0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00 }, /* \ */
        {        0x7C, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7C, 0x00 }, /* ] */ 
        {        0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* ^ */
        {        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF }, /* _ */
        {        0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* ` */
        {        0x00, 0x00, 0x3C, 0x60, 0x7C, 0x66, 0x7C, 0x00 }, /* a */
        {        0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00 }, /* b */
        {        0x00, 0x00, 0x3C, 0x66, 0x06, 0x66, 0x3C, 0x00 }, /* c */
        {        0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00 }, /* d */
        {        0x00, 0x00, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00 }, /* e */
        {        0x38, 0x0C, 0x0C, 0x3E, 0x0C, 0x0C, 0x0C, 0x00 }, /* f */
        {        0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x3C }, /* g */
        {        0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x00 }, /* h */
        {        0x18, 0x00, 0x1C, 0x18, 0x18, 0x18, 0x3C, 0x00 }, /* i */
        {        0x18, 0x00, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x0E }, /* j */
        {        0x06, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x00 }, /* k */
        {        0x1C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00 }, /* l */
        {        0x00, 0x00, 0x6C, 0xFE, 0xD6, 0xD6, 0xC6, 0x00 }, /* m */
        {        0x00, 0x00, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x00 }, /* n */
        {        0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00 }, /* o */
        {        0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06 }, /* p */
        {        0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0xE0 }, /* q */
        {        0x00, 0x00, 0x36, 0x6E, 0x06, 0x06, 0x06, 0x00 }, /* r */
        {        0x00, 0x00, 0x7C, 0x06, 0x3C, 0x60, 0x3E, 0x00 }, /* s */
        {        0x0C, 0x0C, 0x3E, 0x0C, 0x0C, 0x0C, 0x38, 0x00 }, /* t */
        {        0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x00 }, /* u */
        {        0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00 }, /* v */
        {        0x00, 0x00, 0xC6, 0xD6, 0xD6, 0xFE, 0x6C, 0x00 }, /* w */
        {        0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00 }, /* x */
        {        0x00, 0x00, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x3C }, /* y */
        {        0x00, 0x00, 0x7E, 0x30, 0x18, 0x0C, 0x7E, 0x00 }, /* z */
        {        0x30, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x30, 0x00 }, /* { */
        {        0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 }, /* | */
        {        0x0C, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0C, 0x00 }, /* } */
        {        0x8C, 0xD6, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00 }  /* ~ */
};

#define ALPS_LH
#ifdef ALPS_LH
#define LCD_WIDTH  640
#define LCD_HEIGHT 240
#else
#define LCD_WIDTH  320
#define LCD_HEIGHT 240
#endif
// This can be 1 or 2
#define SCREEN_SCALE 2
#define NIBBLES_PER_PIXEL (1*SCREEN_SCALE)
#define PIXELS_PER_BYTE (2/NIBBLES_PER_PIXEL)
#define PIXEL_MASK      ((1<<PIXELS_PER_BYTE)-1)

// Physical screen info
static int lcd_depth;  // Should be 1, 2, or 4
static int lcd_width  = LCD_WIDTH;
static int lcd_height = LCD_HEIGHT;
static cyg_uint8 *lcd_base = (cyg_uint8 *)0xC0000000;

// Virtual screen info
static int curX = 0;  // Last used position
static int curY = 0;
static int width = LCD_WIDTH / (FONT_WIDTH*NIBBLES_PER_PIXEL);
static int height = LCD_HEIGHT / (FONT_HEIGHT*SCREEN_SCALE);
static int fg = 0x0F;
static int bg = 0x00;

// Functions
static void lcd_drawc(cyg_int8 c, int x, int y);

void
lcd_on(int depth)
{
    cyg_int32 ctl_word = 0;
    lcd_depth = depth;
    switch (depth) {
    case 1:
        *(volatile cyg_int32 *)PALLSW = 0xF0F0F0F0;
        *(volatile cyg_int32 *)PALMSW = 0xF0F0F0F0;
        ctl_word = 0;
        break;
    case 2:
        *(volatile cyg_int32 *)PALLSW = 0xFA50FA50;
        *(volatile cyg_int32 *)PALMSW = 0xFA50FA50;
        ctl_word = LCDCON_GSEN;
        break;
    case 4:
        *(volatile cyg_int32 *)PALLSW = 0x76543210;
        *(volatile cyg_int32 *)PALMSW = 0xFEDCBA98;
        ctl_word = LCDCON_GSEN | LCDCON_GSMD;
        break;
    }
    // Video buffer size
    ctl_word |= ((lcd_width * lcd_height * lcd_depth) / 128 - 1) << LCDCON_BUFSIZ_S;
    // Line length
    ctl_word |= ((lcd_width / 16) - 1) << LCDCON_LINE_LENGTH_S;
    // Pixel prescale
    ctl_word |= ((526628 / (lcd_height * lcd_width)) - 1) << LCDCON_PIX_PRESCALE_S;
    // AC frequency bias
    ctl_word |= 0 << LCDCON_AC_PRESCALE_S;
    *(volatile cyg_int32 *)LCDCON = ctl_word;
    diag_printf("Depth: %d, ctl: %x\n", depth, ctl_word);

#ifdef ALPS_LH
#define LCD_DCDC      0x02
#define LCD_ENABLE    0x04
#define LCD_BACKLIGHT 0x08
#define LCD_INIT (LCD_DCDC|LCD_ENABLE|LCD_BACKLIGHT)
    *(volatile cyg_uint8 *)PDDDR   = 0x40;      // 1's are inputs (backwards from A/B/E)
    *(volatile cyg_uint8 *)PDDR    = LCD_INIT;  // Enable video + backlight + DC-DC converter
#else
#ifdef _CL7111
    *(volatile cyg_uint8 *)PDDR    = 0x20;  // Enable video
    *(volatile cyg_uint8 *)PEDDR   = 0x0F;  // Register E data direction
    *(volatile cyg_uint8 *)PEDR   |= 0x01;  // Enable PE2

    *(volatile cyg_uint32 *)PMPCON = 0x500; // Enable DC-to-DC pump #1
#endif
#endif

    *(volatile cyg_uint8 *)FRBADDR = 0x0C;  // Highest order nibble of LCD frame address
 
    *(volatile cyg_uint32 *)SYSCON1 |= SYSCON1_LCDEN;
}

// Clear screen
void
lcd_clear(void)
{
    cyg_int8 *ptr = lcd_base;
    int i;
    for (i = 0;  i < (lcd_width*lcd_height*lcd_depth)/8;  i++) {
        *ptr++ = 0;
    }
    curX = 0;  curY = 0;
    lcd_drawc(CURSOR_ON, curX, curY);
}

// Position cursor
void
lcd_moveto(int X, int Y)
{
    lcd_drawc(CURSOR_OFF, curX, curY);
    if (X < 0) X = 0;
    if (X >= width) X = width-1;
    curX = X;
    if (Y < 0) Y = 0;
    if (Y >= height) Y = height-1;
    curY = Y;
    lcd_drawc(CURSOR_ON, curX, curY);
}

// Render a character at position (X,Y) with current background/foreground
static void
lcd_drawc(cyg_int8 c, int x, int y)
{
    // Note: this only works for fonts which are a multiple of 8 bits wide!
    cyg_uint8 *bufptr, bits;
    int l, p, w;
    switch (lcd_depth) {
    case 1:
        bufptr = lcd_base + ((y * FONT_HEIGHT * (lcd_width*lcd_depth)) + (x * FONT_WIDTH)) / 8;
        for (l = 0;  l < FONT_HEIGHT;  l++) {
            *bufptr = font_table[c][l];
            bufptr += (lcd_width*lcd_depth)/8;
        }
        break;
    case 2:
        diag_printf("Depth 2 not implemented\n");
        break;
    case 4:
        bufptr = lcd_base + ((y * (FONT_HEIGHT*SCREEN_SCALE) * (lcd_width*lcd_depth)) 
                             + (x * (FONT_WIDTH*SCREEN_SCALE) * lcd_depth)) / 8;
        for (l = 0;  l < FONT_HEIGHT;  l++) {
            for (w = 0;  w < SCREEN_SCALE;  w++) {
                bits = font_table[c-FIRST_CHAR][l]; 
                for (p = 0;  p < 8;  p += PIXELS_PER_BYTE) {
                    switch (bits & PIXEL_MASK) {
                    case 0:
                        *bufptr++ = (bg << 4) | bg;
                        break;
                    case 1:
#if SCREEN_SCALE == 1
                        *bufptr++ = (bg << 4) | fg;
#else
                        *bufptr++ = (fg << 4) | fg;
#endif
                        break;
#if SCREEN_SCALE == 1
                    case 2:
                        *bufptr++ = (fg << 4) | bg;
                        break;
                    case 3:
                        *bufptr++ = (fg << 4) | fg;
                        break;
#endif
                    }
                    bits >>= PIXELS_PER_BYTE;
                }
                bufptr += (lcd_width*lcd_depth)/8 - (8/PIXELS_PER_BYTE);
            }
        }
        break;
    }
}

// Draw one character at the current position
void
lcd_putc(cyg_int8 c)
{
    lcd_drawc(CURSOR_OFF, curX, curY);
    switch (c) {
    case '\r':
        curX = 0;
        break;
    case '\n':
        curY++;
        break;
    case '\b':
        curX--;
        if (curX < 0) {
            curY--;
            if (curY < 0) curY = 0;
            curX = width-1;
        }
        break;
    default:
        lcd_drawc(c, curX, curY);
        curX++;
        if (curX == width) {
            curY++;
            curX = 0;
        }
    } 
    lcd_drawc(CURSOR_ON, curX, curY);
}

// Basic LCD 'printf()' support

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#include <stdarg.h>

#define is_digit(c) ((c >= '0') && (c <= '9'))

static int
_cvt(unsigned long val, char *buf, long radix, char *digits)
{
    char temp[80];
    char *cp = temp;
    int length = 0;
    if (val == 0) {
        /* Special case */
        *cp++ = '0';
    } else {
        while (val) {
            *cp++ = digits[val % radix];
            val /= radix;
        }
    }
    while (cp != temp) {
        *buf++ = *--cp;
        length++;
    }
    *buf = '\0';
    return (length);
}

int
lcd_vprintf(void (*putc)(cyg_int8), const char *fmt0, va_list ap)
{
    char c, sign, *cp;
    int left_prec, right_prec, zero_fill, length, pad, pad_on_right;
    char buf[32];
    long val;
    while ((c = *fmt0++)) {
        cp = buf;
        length = 0;
        if (c == '%') {
            c = *fmt0++;
            left_prec = right_prec = pad_on_right = 0;
            if (c == '-') {
                c = *fmt0++;
                pad_on_right++;
            }
            if (c == '0') {
                zero_fill = TRUE;
                c = *fmt0++;
            } else {
                zero_fill = FALSE;
            }
            while (is_digit(c)) {
                left_prec = (left_prec * 10) + (c - '0');
                c = *fmt0++;
            }
            if (c == '.') {
                c = *fmt0++;
                zero_fill++;
                while (is_digit(c)) {
                    right_prec = (right_prec * 10) + (c - '0');
                    c = *fmt0++;
                }
            } else {
                right_prec = left_prec;
            }
            sign = '\0';
            switch (c) {
            case 'd':
            case 'x':
            case 'X':
                val = va_arg(ap, long);
                switch (c) {
                case 'd':
                    if (val < 0) {
                        sign = '-';
                        val = -val;
                    }
                    length = _cvt(val, buf, 10, "0123456789");
                    break;
                case 'x':
                    length = _cvt(val, buf, 16, "0123456789abcdef");
                    break;
                case 'X':
                    length = _cvt(val, buf, 16, "0123456789ABCDEF");
                    break;
                }
                break;
            case 's':
                cp = va_arg(ap, char *);
                length = strlen(cp);
                break;
            case 'c':
                c = va_arg(ap, long /*char*/);
                (*putc)(c);
                continue;
            default:
                (*putc)('?');
            }
            pad = left_prec - length;
            if (sign != '\0') {
                pad--;
            }
            if (zero_fill) {
                c = '0';
                if (sign != '\0') {
                    (*putc)(sign);
                    sign = '\0';
                }
            } else {
                c = ' ';
            }
            if (!pad_on_right) {
                while (pad-- > 0) {
                    (*putc)(c);
                }
            }
            if (sign != '\0') {
                (*putc)(sign);
            }
            while (length-- > 0) {
                (*putc)(c = *cp++);
                if (c == '\n') {
                    (*putc)('\r');
                }
            }
            if (pad_on_right) {
                while (pad-- > 0) {
                    (*putc)(' ');
                }
            }
        } else {
            (*putc)(c);
            if (c == '\n') {
                (*putc)('\r');
            }
        }
    }
}

int
lcd_printf(char const *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = lcd_vprintf(lcd_putc, fmt, ap);
	va_end(ap);
	return (ret);
}

