//==========================================================================
//
//        lcd_support.c
//
//        SA1110/Assabet - LCD support routines
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
// Date:          2000-06-05
// Description:   Tool used to test LCD stuff
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#include <cyg/hal/hal_sa11x0.h>   // Board definitions
#include <cyg/hal/assabet.h>
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
# ifdef CYGINT_ISO_STDIO_FORMATTED_IO
#  include <stdio.h>  // sscanf
# endif
#endif

#include "banner.xpm"

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

static struct lcd_frame {
    unsigned short palette[16];
    unsigned short pixels[240][320];
    unsigned char  pad[256];
} lcd_frame_buffer;

#define RGB_RED(x)   (((x)&0x1F)<<11)
#define RGB_GREEN(x) (((x)&0x3F)<<5)
#define RGB_BLUE(x)  ((x)&0x1F)

static volatile struct lcd_frame *fp;

static int
_hexdigit(char c)
{
    if ((c >= '0') && (c <= '9')) {
        return c - '0';
    } else
    if ((c >= 'A') && (c <= 'F')) {
        return (c - 'A') + 0x0A;
    } else
    if ((c >= 'a') && (c <= 'f')) {
        return (c - 'a') + 0x0a;
    }
}

static int
_hex(char *cp)
{
    return (_hexdigit(*cp)<<4) | _hexdigit(*(cp+1));
}

static unsigned short
parse_color(char *cp)
{
    int red, green, blue;

    if (*cp == '#') {
        red = _hex(cp+1);
        green = _hex(cp+3);
        blue = _hex(cp+5);
        return RGB_RED(red>>3) | RGB_GREEN(green>>2) | RGB_BLUE(blue>>3);
    } else {
        // Should be "None"
        return 0xFFFF;
    }
}

#ifndef CYGINT_ISO_STDIO_FORMATTED_IO
static int
get_int(char **_cp)
{
    char *cp = *_cp;
    char c;
    int val = 0;
    
    while ((c = *cp++) && (c != ' ')) {
        if ((c >= '0') && (c <= '9')) {
            val = val * 10 + (c - '0');
        } else {
            return -1;
        }
    }
    *_cp = cp;
    return val;
}
#endif

void
show_xpm(char *xpm[])
{
    int i, row, col;
    char *cp;
    int nrows, ncols, nclrs;
    unsigned short colors[256];  // Mapped by character index

    cp = xpm[0];
#ifdef CYGINT_ISO_STDIO_FORMATTED_IO
    if (sscanf(cp, "%d %d %d", &ncols, &nrows, &nclrs) != 3) {
#else
    if (((ncols = get_int(&cp)) < 0) ||
        ((nrows = get_int(&cp)) < 0) ||
        ((nclrs = get_int(&cp)) < 0)) {
#endif
        diag_printf("Can't parse XPM data, sorry\n");
        return;
    }
    //    diag_printf("%d rows, %d cols, %d colors\n", nrows, ncols, nclrs);

    for (i = 0;  i < 256;  i++) {
        colors[i] = 0x0000;
    }
    for (i = 0;  i < nclrs;  i++) {
        cp = xpm[i+1];
        colors[(unsigned int)*cp] = parse_color(&cp[4]);
//        printf("Color[%c] = %x\n", *cp, colors[(unsigned int)*cp]);
    }

    for (row = 0;  row < nrows;  row++) {            
        cp = xpm[nclrs+1+row];        
        for (col = 0;  col < ncols;  col++) {
            fp->pixels[row][col] = colors[(unsigned int)*cp++];
        }
    }
    //    cyg_thread_delay(100);
}

// 8x8 Font - from Helios

#define FIRST_CHAR 0x20
#define LAST_CHAR  0x7E
#define FONT_HEIGHT 8
#define FONT_WIDTH  8
#define CURSOR_ON  0x5F
#define CURSOR_OFF 0x20
static const char font_table[LAST_CHAR-FIRST_CHAR+1][8] =
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

#define LCD_WIDTH  320
#define LCD_HEIGHT 240
#define LCD_DEPTH   16

// This can be 1 or 2
#define SCREEN_SCALE 2
#define NIBBLES_PER_PIXEL (1*SCREEN_SCALE)
#define PIXELS_PER_BYTE (2/NIBBLES_PER_PIXEL)
#define PIXEL_MASK      ((1<<PIXELS_PER_BYTE)-1)

// Physical screen info
//static int lcd_depth  = LCD_DEPTH;  // Should be 1, 2, or 4
static int lcd_width  = LCD_WIDTH;
static int lcd_height = LCD_HEIGHT;

// Virtual screen info
static int curX = 0;  // Last used position
static int curY = 0;
//static int width = LCD_WIDTH / (FONT_WIDTH*NIBBLES_PER_PIXEL);
//static int height = LCD_HEIGHT / (FONT_HEIGHT*SCREEN_SCALE);
static int fg = RGB_RED(0) | RGB_GREEN(0) | RGB_BLUE(0);
static int bg = RGB_RED(31) | RGB_GREEN(63) | RGB_BLUE(31);
#define SCREEN_WIDTH  (LCD_WIDTH/FONT_WIDTH)
#define SCREEN_HEIGHT (LCD_HEIGHT/FONT_HEIGHT)
static char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
#define SCREEN_START 7

// Functions
static void lcd_drawc(cyg_int8 c, int x, int y);

void
lcd_init(int depth)
{
    // Currently only color/16bpp supported

    unsigned long _fp;

    // Frame buffer must be aligned on a 16 byte boundary
    _fp = (((unsigned long)&lcd_frame_buffer) + 15) & ~15;
    fp = (struct lcd_frame *)(_fp + SA11X0_RAM_BANK0_BASE);
    // Enable LCD in 320x240 16bpp
    *SA1110_DBAR1 = (unsigned long)&(fp->palette);
    *SA1110_LCCR1 = 0x1D1D0930;
    *SA1110_LCCR2 = 0xEF;
    *SA1110_LCCR3 = 0x0c;
    fp->palette[0] = 0x2000;  // Tell controller 16 bits
//    *SA1110_LCCR0 = 0xBB;  // B&W
    *SA1110_LCCR0 = 0xB9;  // Color
//    assabet_BCR(SA1110_BCR_LCD_BPP, SA1110_BCR_LCD_12BPP);
    assabet_BCR(SA1110_BCR_LCD_BPP, SA1110_BCR_LCD_16BPP);
    assabet_BCR(SA1110_BCR_LCD, SA1110_BCR_LCD_ON);
    assabet_BCR(SA1110_BCR_BACKLIGHT, SA1110_BCR_BACKLIGHT);
}

// Clear screen
void
lcd_clear(void)
{
    int row, col;
    for (row = 0;  row < lcd_height;  row++) {
        for (col = 0;  col < lcd_width;  col++) {
            fp->pixels[row][col] = bg;
        }
    }
    for (row = 0;  row < SCREEN_HEIGHT;  row++) {
        for (col = 0;  col < SCREEN_WIDTH;  col++) {
            screen[row][col] = ' ';
        }
    }
    // Note: Row 0 seems to wrap incorrectly
    curX = 0;  curY = SCREEN_START;
    lcd_drawc(CURSOR_ON, curX, curY);
    show_xpm(banner_xpm);
}

// Position cursor
void
lcd_moveto(int X, int Y)
{
    lcd_drawc(CURSOR_OFF, curX, curY);
    if (X < 0) X = 0;
    if (X >= SCREEN_WIDTH) X = SCREEN_WIDTH-1;
    curX = X;
    if (Y < SCREEN_START) Y = SCREEN_START;
    if (Y >= SCREEN_HEIGHT) Y = SCREEN_HEIGHT-1;
    curY = Y;
    lcd_drawc(CURSOR_ON, curX, curY);
}

// Render a character at position (X,Y) with current background/foreground
static void
lcd_drawc(cyg_int8 c, int x, int y)
{
    // Currently hard-coded for 16bpp
    cyg_uint8 bits;
    cyg_uint16 *pixels;
    int l, p;

    screen[curY][curX] = c;
    for (l = 0;  l < FONT_HEIGHT;  l++) {
        bits = font_table[c-FIRST_CHAR][l]; 
        pixels = &fp->pixels[curY*FONT_HEIGHT+l][curX*FONT_WIDTH];
        for (p = 0;  p < 8;  p++) {
            if (bits & 0x01) {
                *pixels++ = fg;
            } else {
                *pixels++ = bg;
            }
            bits >>= 1;
        }
    }
}

static void
lcd_scroll(void)
{
    int row, col;
    cyg_uint8 *c1, *c2;
    cyg_uint16 *p1, *p2;

    // First scroll up the virtual screen
    for (row = SCREEN_START;  row < SCREEN_HEIGHT;  row++) {
        c1 = &screen[row-1][0];
        c2 = &screen[row][0];
        for (col = 0;  col < SCREEN_WIDTH;  col++) {
            *c1++ = *c2++;
        }
    } 
    c1 = &screen[SCREEN_HEIGHT-1][0];
    for (col = 0;  col < SCREEN_WIDTH;  col++) {
        *c1++ = 0x20;
    }
    // Now the physical screen
    for (row = FONT_HEIGHT*(SCREEN_START+1);  row < LCD_HEIGHT;  row++) {        
        p1 = &fp->pixels[row-FONT_HEIGHT][0];
        p2 = &fp->pixels[row][0];
        for (col = 0;  col < LCD_WIDTH;  col++) {
            *p1++ = *p2++;
        }
    }
    for (row = LCD_HEIGHT-FONT_HEIGHT;  row < LCD_HEIGHT;  row++) {
        p1 = &fp->pixels[row][0];
        for (col = 0;  col < LCD_WIDTH;  col++) {
            *p1++ = bg;
        }
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
        if (curY == SCREEN_HEIGHT) {
            lcd_scroll();
            curY--;
        }
        break;
    case '\b':
        curX--;
        if (curX < 0) {
            curY--;
            if (curY < 0) curY = 0;
            curX = SCREEN_WIDTH-1;
        }
        break;
    default:
        lcd_drawc(c, curX, curY);
        curX++;
        if (curX == SCREEN_WIDTH) {
            curY++;
            curX = 0;
        }
    } 
    lcd_drawc(CURSOR_ON, curX, curY);
}

// Basic LCD 'printf()' support

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

void
set_bg(int red, int green, int blue)
{
    bg = RGB_RED(red) | RGB_GREEN(green) | RGB_BLUE(blue);
}

void
set_fg(int red, int green, int blue)
{
    fg = RGB_RED(red) | RGB_GREEN(green) | RGB_BLUE(blue);
}
