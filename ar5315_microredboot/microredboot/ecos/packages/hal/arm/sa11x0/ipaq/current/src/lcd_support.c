//==========================================================================
//
//        lcd_support.c
//
//        SA1110/iPAQ - LCD support routines
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
//                Richard Panton <richard.panton@3glab.com>
// Date:          2001-02-24
// Description:   Simple LCD support
//####DESCRIPTIONEND####

#include <pkgconf/hal.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_if.h>       // Virtual vector support
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#include <cyg/hal/hal_sa11x0.h>   // Board definitions
#include <cyg/hal/ipaq.h>
#include <cyg/hal/lcd_support.h>
#include <cyg/hal/atmel_support.h>
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
# ifdef CYGINT_ISO_STDIO_FORMATTED_IO
#  include <stdio.h>  // sscanf
# endif
#endif

// Gross hack to force use of simple internal functions.
// Using libc sscanf fails (as far as I could tell from a quick look)
// when assertions are enabled because some of the C++ contructors have
// not been run yet.   jskov
#undef CYGINT_ISO_STDIO_FORMATTED_IO

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define PORTRAIT_MODE
//#define LOGO_AT_TOP
#include "banner.xpm"
#include "font.h"

// Physical dimensions of LCD display
#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

// Logical layout
#ifdef PORTRAIT_MODE
#define LCD_WIDTH  240
#define LCD_HEIGHT 320
#else
#define LCD_WIDTH  320
#define LCD_HEIGHT 240
#endif
#define LCD_DEPTH   16

static struct lcd_frame {
    unsigned short palette[16];
    unsigned short pixels[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    unsigned char  pad[256];
} *lcd_frame_buffer = (struct lcd_frame *)0x01FC0000;  // Actually just 0x26000 bytes, but...

#define USE_RGB565
#ifdef USE_RGB565
#define RGB_RED(x)   (((x)&0x1F)<<11)
#define RGB_GREEN(x) (((x)&0x3F)<<5)
#define RGB_BLUE(x)  ((x)&0x1F)
#else
#define RGB_RED(x)   (((x)&0x0F)<<12)
#define RGB_GREEN(x) (((x)&0x0F)<<7)
#define RGB_BLUE(x)  (((x)&0x0F)<<1)
#endif

static volatile struct lcd_frame *fp;

// Physical screen info
//static int lcd_depth  = LCD_DEPTH;  // Should be 1, 2, or 4
static int lcd_bpp;
static int lcd_width  = LCD_WIDTH;
static int lcd_height = LCD_HEIGHT;

// Virtual screen info
static int curX = 0;  // Last used position
static int curY = 0;
//static int width = LCD_WIDTH / (FONT_WIDTH*NIBBLES_PER_PIXEL);
//static int height = LCD_HEIGHT / (FONT_HEIGHT*SCREEN_SCALE);

static int fg = RGB_RED(15) | RGB_GREEN(63) | RGB_BLUE(8);
static int bg = RGB_RED(0) | RGB_GREEN(0) | RGB_BLUE(15/*31*/);

#define SCREEN_PAN            20
#define SCREEN_WIDTH          80
#define SCREEN_HEIGHT         (LCD_HEIGHT/FONT_HEIGHT)
#define VISIBLE_SCREEN_WIDTH  (LCD_WIDTH/FONT_WIDTH)
#define VISIBLE_SCREEN_HEIGHT (LCD_HEIGHT/FONT_HEIGHT)
static char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
static int screen_start = 0;
static int screen_height = SCREEN_HEIGHT;
static int screen_width = SCREEN_WIDTH;
static int screen_pan = 0;

static bool cursor_enable = true;

static int kbd_pos;

// Functions
static void lcd_drawc(cyg_int8 c, int x, int y);

#ifdef PORTRAIT_MODE
// Translate coordinates, rotating clockwise 90 degrees
static void
set_pixel(int row, int col, unsigned short val)
{
    fp->pixels[col][(DISPLAY_WIDTH-1)-row] = val;
}
#else
static void
set_pixel(int row, int col, unsigned short val)
{
    fp->pixels[row][col] = val;
}
#endif

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
    return 0;
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

    while (*cp && (*cp != 'c')) cp++;
    if (cp) {
        cp += 2;
        if (*cp == '#') {
            red = _hex(cp+1);
            green = _hex(cp+3);
            blue = _hex(cp+5);
#ifdef USE_RGB565
            return RGB_RED(red>>3) | RGB_GREEN(green>>2) | RGB_BLUE(blue>>3);
#else
            return RGB_RED(red>>3) | RGB_GREEN(green>>3) | RGB_BLUE(blue>>3);
#endif
        } else {
            // Should be "None"
            return 0xFFFF;
        }
    } else {
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

int
show_xpm(char *xpm[], int screen_pos)
{
    int i, row, col, offset;
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
        return 0;
    }
    // printf("%d rows, %d cols, %d colors\n", nrows, ncols, nclrs);

    for (i = 0;  i < 256;  i++) {
        colors[i] = 0x0000;
    }
    for (i = 0;  i < nclrs;  i++) {
        cp = xpm[i+1];
        colors[(unsigned int)*cp] = parse_color(&cp[1]);
        // printf("Color[%c] = %x\n", *cp, colors[(unsigned int)*cp]);
    }

#ifdef LOGO_AT_TOP
    offset = screen_pos;
#else
    offset = screen_pos-nrows;
#endif
    for (row = 0;  row < nrows;  row++) {            
        cp = xpm[nclrs+1+row];        
        for (col = 0;  col < ncols;  col++) {
            set_pixel(row+offset, col, colors[(unsigned int)*cp++]);
        }
    }
#ifdef LOGO_AT_TOP
    screen_start = (nrows + (FONT_HEIGHT-1))/FONT_HEIGHT;
    return offset+nrows;
#else    
    screen_height = offset / FONT_HEIGHT;
    return offset;
#endif
}

void
lcd_init(int depth)
{
    // Currently only color/16bpp supported

    if (depth != 16) {
        return;
    }
    lcd_bpp = depth;
    fp = (struct lcd_frame *)hal_virt_to_phys_address((cyg_uint32)lcd_frame_buffer);
    // Enable LCD in 320x240 16bpp
    *SA1110_DBAR1 = (unsigned long)&(fp->palette);
    *SA1110_LCCR1 = 0x0b100800 + DISPLAY_WIDTH - 16;
    *SA1110_LCCR2 = 0x0a010400 + DISPLAY_HEIGHT - 1;
    *SA1110_LCCR3 = 0x00300010;
    fp->palette[0] = 0x2000;  // Tell controller true color / 16 bits
    *SA1110_LCCR0 = 0xB9;     // Color
    ipaq_EGPIO(SA1110_EIO_LCD_3V3|SA1110_EIO_LCD_CTRL|SA1110_EIO_LCD_5V|SA1110_EIO_LCD_VDD,
	    SA1110_EIO_LCD_3V3_ON|SA1110_EIO_LCD_CTRL_ON|SA1110_EIO_LCD_5V_ON|SA1110_EIO_LCD_VDD_ON);
    lcd_clear();
    lcd_brightness(31);
}

// Get information about the frame buffer
int
lcd_getinfo(struct lcd_info *info)
{
    if (lcd_bpp == 0) {
        return 0;  // LCD not initialized
    }
    info->width = DISPLAY_WIDTH;
    info->height = DISPLAY_HEIGHT;
    info->bpp = lcd_bpp;
    info->fb = (void *)lcd_frame_buffer->pixels;  // Note: this is cached
    info->rlen = DISPLAY_WIDTH * 2;
    info->type = FB_TRUE_RGB565;
    return 1; // Information valid
}

// Control screen light [brightness]

static void
lcd_brightness_ack(atmel_pkt *pkt)
{
}

static int _lcd_brightness;

void
lcd_brightness(int level)
{
    unsigned char cmd[3];

    atmel_register(ATMEL_CMD_LIGHT, lcd_brightness_ack);
    cmd[0] = 1;  // LCD magic
    cmd[1] = (level > 0) ? 1 : 0;  // Turn light on
    cmd[2] = level;
    if (level) _lcd_brightness = level;
    atmel_send(ATMEL_CMD_LIGHT, cmd, 3);
}

// Clear screen
void
lcd_clear(void)
{
    int row, col;
    int pos;

#ifndef USE_RGB565
    int val;
    for (row = 0;  row < lcd_height;  row++) {
        for (col = 0;  col < lcd_width;  col++) {
            set_pixel(row, col, RGB_RED(31));
        }
    }
    CYGACC_CALL_IF_DELAY_US(100000);

    for (row = 0;  row < lcd_height;  row++) {
        for (col = 0;  col < lcd_width;  col++) {
            set_pixel(row, col, RGB_GREEN(31));
        }
    }
    CYGACC_CALL_IF_DELAY_US(100000);
    val = 0;
    for (pos = 0;  pos < 16;  pos++) {
        val = (1<<pos);
        diag_printf("Set pixel to 0x%04x\n", val);
        for (row = 0;  row < lcd_height;  row++) {
            for (col = 0;  col < lcd_width;  col++) {
                set_pixel(row, col, val);
            }
        }
        CYGACC_CALL_IF_DELAY_US(100000);
    }
    val = 0;
    for (pos = 8;  pos < 16;  pos++) {
        val |= (1<<pos);
        diag_printf("Set pixel to 0x%04x\n", val);
        for (row = 0;  row < lcd_height;  row++) {
            for (col = 0;  col < lcd_width;  col++) {
                set_pixel(row, col, val);
            }
        }
        CYGACC_CALL_IF_DELAY_US(100000);
    }

    for (row = 0;  row < lcd_height;  row++) {
        for (col = 0;  col < lcd_width;  col++) {
            set_pixel(row, col, RGB_BLUE(31));
        }
    }
    CYGACC_CALL_IF_DELAY_US(100000);
#endif // RGB565

    for (row = 0;  row < lcd_height;  row++) {
        for (col = 0;  col < lcd_width;  col++) {
            set_pixel(row, col, bg);
        }
    }
    for (row = 0;  row < screen_height;  row++) {
        for (col = 0;  col < screen_width;  col++) {
            screen[row][col] = ' ';
        }
    }
    // Note: Row 0 seems to wrap incorrectly
#ifdef LOGO_AT_TOP
    pos = 0;
#else
    pos = (LCD_HEIGHT-1);
#endif
    kbd_pos = show_xpm(banner_xpm, pos);
    curX = 0;  curY = screen_start;
    if (cursor_enable) {
        lcd_drawc(CURSOR_ON, curX-screen_pan, curY);
    }
}

// Position cursor
void
lcd_moveto(int X, int Y)
{
    if (cursor_enable) {
        lcd_drawc(screen[curY][curX], curX-screen_pan, curY);
    }
    if (X < 0) X = 0;
    if (X >= screen_width) X = screen_width-1;
    curX = X;
    if (Y < screen_start) Y = screen_start;
    if (Y >= screen_height) Y = screen_height-1;
    curY = Y;
    if (cursor_enable) {
        lcd_drawc(CURSOR_ON, curX-screen_pan, curY);
    }
}

// Render a character at position (X,Y) with current background/foreground
static void
lcd_drawc(cyg_int8 c, int x, int y)
{
    cyg_uint8 bits;
    int l, p;

    if ((x < 0) || (x >= VISIBLE_SCREEN_WIDTH) || 
        (y < 0) || (y >= screen_height)) return;  
    for (l = 0;  l < FONT_HEIGHT;  l++) {
        bits = font_table[c-FIRST_CHAR][l]; 
        for (p = 0;  p < FONT_WIDTH;  p++) {
            if (bits & 0x01) {
                set_pixel(y*FONT_HEIGHT+l, x*FONT_WIDTH + p, fg);
            } else {
                set_pixel(y*FONT_HEIGHT+l, x*FONT_WIDTH + p, bg);
            }
            bits >>= 1;
        }
    }
}

static void
lcd_refresh(void)
{
    int row, col;
#ifdef PORTRAIT_MODE
    for (row = screen_start;  row < screen_height;  row++) {
        for (col = 0;  col < VISIBLE_SCREEN_WIDTH;  col++) {
            if ((col+screen_pan) < screen_width) {
                lcd_drawc(screen[row][col+screen_pan], col, row);
            } else {
                lcd_drawc(' ', col, row);
            }
        }
    }
#else
    cyg_uint16 *p1, *p2;
    // Now the physical screen
    for (row = FONT_HEIGHT*(screen_start+1);  row < LCD_HEIGHT;  row++) {        
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
#endif
    if (cursor_enable) {
        lcd_drawc(CURSOR_ON, curX-screen_pan, curY);
    }
}

static void
lcd_scroll(void)
{
    int row, col;
    cyg_uint8 *c1, *c2;

    // First scroll up the virtual screen
    for (row = (screen_start+1);  row < screen_height;  row++) {
        c1 = &screen[row-1][0];
        c2 = &screen[row][0];
        for (col = 0;  col < screen_width;  col++) {
            *c1++ = *c2++;
        }
    } 
    c1 = &screen[screen_height-1][0];
    for (col = 0;  col < screen_width;  col++) {
        *c1++ = 0x20;
    }
    lcd_refresh();
}

// Draw one character at the current position
void
lcd_putc(cyg_int8 c)
{
    if (cursor_enable) {
        lcd_drawc(screen[curY][curX], curX-screen_pan, curY);
    }
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
            curX = screen_width-1;
        }
        break;
    default:
        if (((cyg_uint8)c < FIRST_CHAR) || ((cyg_uint8)c > LAST_CHAR)) c = '.';
        screen[curY][curX] = c;
        lcd_drawc(c, curX-screen_pan, curY);
        curX++;
        if (curX == screen_width) {
            curY++;
            curX = 0;
        }
    } 
    if (curY >= screen_height) {
        lcd_scroll();
        curY = (screen_height-1);
    }
    if (cursor_enable) {
        lcd_drawc(CURSOR_ON, curX-screen_pan, curY);
    }
}

// Basic LCD 'printf()' support

#include <stdarg.h>
#include <string.h>

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

static int
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
    return 0;  // Should be length of string written
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
lcd_setbg(int red, int green, int blue)
{
    bg = RGB_RED(red) | RGB_GREEN(green) | RGB_BLUE(blue);
}

void
lcd_setfg(int red, int green, int blue)
{
    fg = RGB_RED(red) | RGB_GREEN(green) | RGB_BLUE(blue);
}

#ifdef CYGSEM_IPAQ_LCD_COMM

//
// Support LCD/touchscreen as a virtual I/O channel
//

#include "kbd.xpm"     // Contains 4 keyboard images

static int  _timeout = 500;

struct coord {
    short x,y;
};

#ifdef PORTRAIT_MODE
#define CS_UL 1
#define CS_UR 0
#define CS_LL 3
#define CS_LR 2
#else
#define CS_UL 0
#define CS_UR 1
#define CS_LL 2
#define CS_LR 3
#endif
#define KBD_FUZZ 50
static struct coord kbd_limits[4];
static short minX, maxX, minY, maxY;  // Coordinates for the keyboard matrix

#define CODE_NONE   0x00
#define CODE_CTRL   0x81
#define CODE_SHIFT  0x82
#define CODE_NUM    0x83
#define CODE_BS     0x08
#define CODE_CR     0x0D
#define CODE_ESC    0x1B
#define CODE_DEL    0x7F
#define CTRL(x)     (x&0x1F)

typedef unsigned char kbd_map[4][11];
static kbd_map kbd_norm_map = {
    { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', CODE_BS },
    { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '-', CODE_CR },
    { CODE_CTRL, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', ';' },
    { CODE_SHIFT, CODE_SHIFT, ' ', ' ', ' ', ' ', CODE_NUM, '\'', '=', '\\', '/'}
};

static kbd_map kbd_num_map = {
    { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', CODE_BS },
    { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', CODE_CR },
    { CODE_CTRL, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '`', '~' },
    { CODE_SHIFT, CODE_SHIFT, ' ', ' ', ' ', ' ', CODE_NUM, '[', ']', '{', '}'}
};

static kbd_map kbd_ctrl_map = {
    { CTRL('q'), CTRL('w'), CTRL('e'), CTRL('r'), CTRL('t'), CTRL('y'), 
      CTRL('u'), CTRL('i'), CTRL('o'), CTRL('p'), CODE_ESC  },
    { CTRL('a'), CTRL('s'), CTRL('d'), CTRL('f'), CTRL('g'), CTRL('h'), 
      CTRL('j'), CTRL('k'), CTRL('l'), CTRL('_'), CODE_CR },
    { CODE_CTRL, CTRL('z'), CTRL('x'), CTRL('c'), CTRL('v'), CTRL('b'), 
      CTRL('n'), CTRL('m'), '\\', CTRL(']'), CTRL('^') },
    { CODE_SHIFT, CODE_SHIFT, ' ', ' ', ' ', ' ', CODE_NUM, ' ', ' ', ' ', CODE_DEL}
};

static kbd_map kbd_shift_map = {
    { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', CODE_BS },
    { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '_', CODE_CR },
    { CODE_CTRL, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', ':' },
    { CODE_SHIFT, CODE_SHIFT, ' ', ' ', ' ', ' ', CODE_NUM, '"', '+', '|', '?'}
};

static kbd_map *cur_kbd_map = &kbd_norm_map;
static bool kbd_active = true;

// Pseudo-keyboard indicator
#define LCD_KBD_NORM  0
#define LCD_KBD_SHIFT 1
#define LCD_KBD_NUM   2
#define LCD_KBD_CTRL  3

// Display pseudo keyboard
static void
lcd_kbd(int which)
{
    char **kbd_xpm;

    switch (which) {
    case LCD_KBD_NORM:
        kbd_xpm = keynorm_xpm;
        break;
    case LCD_KBD_SHIFT:
        kbd_xpm = keyshft_xpm;
        break;
    case LCD_KBD_CTRL:
        kbd_xpm = keyctrl_xpm;
        break;
    case LCD_KBD_NUM:
        kbd_xpm = keynum_xpm;
        break;
    default:
        return;
    }
    show_xpm(kbd_xpm, kbd_pos);
}

static bool
inside(int pos, int lim1, int lim2)
{
    if (lim1 <= lim2) {
        return ((pos >= lim1) && (pos <= lim2));
    } else {
        return ((pos >= lim2) && (pos <= lim1));
    }
}

static int
abs(int x)
{
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

static int
min(int x, int y)
{
    if (x < y) {
        return x;
    } else {
        return y;
    }
}

static int
max(int x, int y)
{
    if (x < y) {
        return y;
    } else {
        return x;
    }
}

static cyg_bool
lcd_comm_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    static bool pen_down = false;
    static bool waiting_for_pen_down = true;
    static int  total_events = 0;
    static int  pen_idle;
    static unsigned long totalX, totalY;
    struct ts_event tse;
    struct key_event ke;
//#define KBD_DEBUG
#ifdef KBD_DEBUG
    static bool dump_info = false;
#endif
#define PEN_IDLE_TIMEOUT 50000
#define MIN_KBD_EVENTS   10

    // See if any buttons have been pushed
    if (key_get_event(&ke)) {
        if ((ke.button_info & ATMEL_BUTTON_STATE) == ATMEL_BUTTON_STATE_UP) {
//            diag_printf("Key = %x\n", ke.button_info);
            lcd_on(true);
            switch (ke.button_info & ATMEL_BUTTON_VALUE) {
            case ATMEL_BUTTON_RETURN:
                *ch = CTRL('C');
                return true;
            case ATMEL_BUTTON_JOY_DOWN:
            case ATMEL_BUTTON_JOY_UP:
                screen_pan = 0;
                lcd_refresh();
                break;
            case ATMEL_BUTTON_JOY_LEFT:
                screen_pan -= SCREEN_PAN;
                if (screen_pan < 0) screen_pan = 0;
                lcd_refresh();
                break;
            case ATMEL_BUTTON_JOY_RIGHT:
                screen_pan += SCREEN_PAN;
                if (screen_pan > (SCREEN_WIDTH-SCREEN_PAN)) screen_pan = SCREEN_WIDTH-SCREEN_PAN;
                lcd_refresh();
                break;
            default:
#ifdef KBD_DEBUG
                {
                    int cur = start_console(0);
                    diag_printf("pen: %d, waiting: %d, total: %d\n", pen_down, waiting_for_pen_down, total_events);
                    end_console(cur);
                    dump_info = !dump_info;
                }
#endif
                return false;
            }
        }
        return false;  // Ignore down presses
    }
    // If keyboard not active, always returns false
    if (!kbd_active) {
        return false;
    }
    // Wait for pen down
    if (waiting_for_pen_down) {
        if (ts_get_event(&tse)) {
            lcd_on(true);
            if (!tse.up) {
                pen_down = true;
                waiting_for_pen_down = false;
                totalX = totalY = 0;
                total_events = 0;
                pen_idle = PEN_IDLE_TIMEOUT;
#ifdef KBD_DEBUG
                if (dump_info) {
                    int cur = start_console(0);
                    diag_printf("start pen: %d, waiting: %d, total: %d\n", pen_down, waiting_for_pen_down, total_events);
                    end_console(cur);
                }
#endif
            }
        }
        return false;
    }
    // While the pen is down, accumulate some data
    if (ts_get_event(&tse)) {
        pen_idle = PEN_IDLE_TIMEOUT;
        if (tse.up) {
            pen_down = false;
            waiting_for_pen_down = true;
        } else {
            total_events++;
            pen_down = true;
#ifdef PORTRAIT_MODE
            totalX += tse.y;
            totalY += tse.x;
#else
            totalX += tse.x;
            totalY += tse.y;
#endif
        }
    } else {
        if (--pen_idle == 0) {
            pen_down = false;
            waiting_for_pen_down = true;
#ifdef KBD_DEBUG
            if (dump_info) {
                int cur = start_console(0);
                diag_printf("going idle\n");
                end_console(cur);
            }
#endif
        }
        return false;
    }
#ifdef KBD_DEBUG
    if (dump_info) {
        int cur = start_console(0);
        diag_printf("pen: %d, waiting: %d, total: %d\n", pen_down, waiting_for_pen_down, total_events);
        end_console(cur);
    }
#endif
    if (total_events == MIN_KBD_EVENTS) {
        // If pen just went up then see if this was a valid
        // character (inside the keyboard picture, etc)
        int x = totalX/total_events;
        int y = totalY/total_events;
        int row, col;
        int char_width, char_height;
        unsigned char kbd_ch;
#ifdef KBD_DEBUG
        if (dump_info) {
            int cur = start_console(0);
            diag_printf("Pen[%d] at %d/%d\n", total_events, x, y);
            end_console(cur);
        }
#endif
        // Try and determine row/col in our keyboard matrix
        if (inside(x, minX, maxX) && inside(y, minY, maxY)) {
            // Point seems to be with the matrix
            char_width = abs(minX - maxX) / 11;
            char_height = abs(minY - maxY) / 4;
            col = abs(x-maxX) / char_width;
            row = abs(y-minY) / char_height;
            kbd_ch = (*cur_kbd_map)[row][col];
#ifdef KBD_DEBUG
            if (dump_info) {
                int cur = start_console(0);
                diag_printf("Row/Col = %d/%d = %x\n", row, col, kbd_ch);
                end_console(cur);
            }
#endif
            switch (kbd_ch) {
            case CODE_SHIFT:
                if (cur_kbd_map == &kbd_shift_map) {
                    cur_kbd_map = &kbd_norm_map;
                    lcd_kbd(LCD_KBD_NORM);
                } else {
                    cur_kbd_map = &kbd_shift_map;
                    lcd_kbd(LCD_KBD_SHIFT);
                }
                break;
            case CODE_CTRL:
                if (cur_kbd_map == &kbd_ctrl_map) {
                    cur_kbd_map = &kbd_norm_map;
                    lcd_kbd(LCD_KBD_NORM);
                } else {
                    cur_kbd_map = &kbd_ctrl_map;
                    lcd_kbd(LCD_KBD_CTRL);
                }
                break;
            case CODE_NUM:
                if (cur_kbd_map == &kbd_num_map) {
                    cur_kbd_map = &kbd_norm_map;
                    lcd_kbd(LCD_KBD_NORM);
                } else {
                    cur_kbd_map = &kbd_num_map;
                    lcd_kbd(LCD_KBD_NUM);
                }
                break;
            case CODE_NONE:
                break;
            default:
                *ch = kbd_ch;
                return true;
            }
        }
    }
    return false;
}

static cyg_uint8
lcd_comm_getc(void* __ch_data)
{
    cyg_uint8 ch;

    while (!lcd_comm_getc_nonblock(__ch_data, &ch)) ;
    return ch;
}

static void
lcd_comm_putc(void* __ch_data, cyg_uint8 c)
{
    lcd_putc(c);
}

static void
lcd_comm_write(void* __ch_data, const cyg_uint8* __buf, cyg_uint32 __len)
{
#if 0
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        lcd_comm_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
#endif
}

static void
lcd_comm_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
#if 0
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = lcd_comm_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
#endif
}

static cyg_bool
lcd_comm_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    cyg_bool res;

    delay_count = _timeout * 10; // delay in .1 ms steps
    for(;;) {
        res = lcd_comm_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        CYGACC_CALL_IF_DELAY_US(100);
    }
    return res;
}

static int
lcd_comm_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int vector = 0;
    int ret = -1;
    static int irq_state = 0;

    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        ret = irq_state;
        irq_state = 1;
#if 0
        if (vector == 0) {
            vector = eth_drv_int_vector();
        }
        HAL_INTERRUPT_UNMASK(vector); 
#endif
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
#if 0
        HAL_INTERRUPT_MASK(vector);
#endif
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = _timeout;
        _timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
	break;
    }
    case __COMMCTL_FLUSH_OUTPUT:
    case __COMMCTL_GETBAUD:
    case __COMMCTL_SETBAUD:
        ret = 0;
	break;
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
lcd_comm_isr(void *__ch_data, int* __ctrlc, 
           CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
#if 0
    char ch;

    cyg_drv_interrupt_acknowledge(__vector);
    *__ctrlc = 0;
    if (lcd_comm_getc_nonblock(__ch_data, &ch)) {
        if (ch == 0x03) {
            *__ctrlc = 1;
        }
    }
    return CYG_ISR_HANDLED;
#endif
    return 0;
}

static bool
init_kbd_coord(int indx, char *prompt_char)
{
    char prompt[] = "Press %s on kbd graphic";
    int off = ((VISIBLE_SCREEN_WIDTH-sizeof(prompt))/2)-1;
    int off2 = ((VISIBLE_SCREEN_WIDTH-20)/2)-1;
    struct ts_event tse;
    struct key_event ke;
    bool pen_down;
    int i, down_timer, total_events;
    int timeout = 100000;
    unsigned long totalX, totalY;

    lcd_moveto(off, screen_height/2);
    lcd_printf(prompt, prompt_char);
    lcd_moveto(off2, (screen_height/2)+2);
    lcd_printf("Keep pen down until");
    lcd_moveto(off2, (screen_height/2)+3);
    lcd_printf("message disappears");
    pen_down = false;
    down_timer = 0;
    total_events = 0;
    totalX = totalY = 0;
    // Wait for a pen-down event
    while (!pen_down) {
        if (ts_get_event(&tse)) {
            if (!tse.up) {
                pen_down = true;
            }
        }
        if (key_get_event(&ke)) {
            if (ke.button_info == (ATMEL_BUTTON_STATE_UP|ATMEL_BUTTON_RETURN)) {
                return true;
            }
        }
        if (--timeout == 0) {
            // Give up if the guy hasn't pressed anything
            return true;
        }
    }
    // Now accumulate data 
    // Assumption: the Atmel can send at most 3 position reports
    // per millisecond.  We should wait for 50ms before moving on
    while (total_events < 100) {
        if (ts_get_event(&tse)) {
            if (tse.up) {
                pen_down = false;
                continue;
            } else {
#ifdef PORTRAIT_MODE
                totalX += tse.y;
                totalY += tse.x;
#else
                totalX += tse.x;
                totalY += tse.y;
#endif
                total_events++;
                pen_down = true;
            }
        }
        if (key_get_event(&ke)) {
            if (ke.button_info == (ATMEL_BUTTON_STATE_UP|ATMEL_BUTTON_RETURN)) {
                return true;
            }
        }
    }
    // Tell the guy we have enough data
    lcd_moveto(off, screen_height/2);
    for (i = 0;  i < screen_width;  i++) {
        lcd_putc(' ');
    }
    // Now wait for the pen to go back up
    while (pen_down) {
        if (ts_get_event(&tse)) {
            if (tse.up) {
                pen_down = false;
            }
        }
        if (key_get_event(&ke)) {
            if (ke.button_info == (ATMEL_BUTTON_STATE_UP|ATMEL_BUTTON_RETURN)) {
                return true;
            }
        }
    }
    kbd_limits[indx].x = totalX / total_events;
    kbd_limits[indx].y = totalY / total_events;
    return false;
}

static bool
close(int c1, int c2)
{
    int diff = c1 - c2;
    if (diff < 0) diff = -diff;
    return (diff < 50);
}

#define LCD_COMM_CHANNEL 1  // Logical I/O channel used for LCD/TS console

void
lcd_comm_init(void)
{
    static int init = 0;
    bool need_params = true;
    unsigned short cksum, param;
    int i;

    if (!init) {
        hal_virtual_comm_table_t* comm;
        int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

        // Setup procs in the vector table
        CYGACC_CALL_IF_SET_CONSOLE_COMM(LCD_COMM_CHANNEL);
        comm = CYGACC_CALL_IF_CONSOLE_PROCS();
        //CYGACC_COMM_IF_CH_DATA_SET(*comm, chan);
        CYGACC_COMM_IF_WRITE_SET(*comm, lcd_comm_write);
        CYGACC_COMM_IF_READ_SET(*comm, lcd_comm_read);
        CYGACC_COMM_IF_PUTC_SET(*comm, lcd_comm_putc);
        CYGACC_COMM_IF_GETC_SET(*comm, lcd_comm_getc);
        CYGACC_COMM_IF_CONTROL_SET(*comm, lcd_comm_control);
        CYGACC_COMM_IF_DBG_ISR_SET(*comm, lcd_comm_isr);
        CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, lcd_comm_getc_timeout);

        // Restore original console
        CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);

        init = 1;

        // Pick up parameters for virtual keyboard from RAM
        cksum = (unsigned short)&_ipaq_LCD_params[0];
        for (i = 0;  i < 4;  i++) {
            param = _ipaq_LCD_params[i*2];
            kbd_limits[i].x = param;
            cksum ^= param;
            param = _ipaq_LCD_params[(i*2)+1];
            kbd_limits[i].y = param;
            cksum ^= param;
        }
        need_params = cksum != _ipaq_LCD_params[(4*2)+1];

        // If the data are currently bad, set up some defaults
        if (need_params) {
            kbd_limits[CS_UL].x = 994;
            kbd_limits[CS_UL].y = 710;
            kbd_limits[CS_UR].x = 413;
            kbd_limits[CS_UR].y = 710;
            kbd_limits[CS_LL].x = 989;
            kbd_limits[CS_LL].y = 839;
            kbd_limits[CS_LR].x = 411;
            kbd_limits[CS_LR].y = 836;
        }

        if (!need_params) {
            // See if the guy wants to force new parameters
            lcd_clear();
            lcd_moveto(5, screen_height/2);
            lcd_printf("Calibrate touch screen?\n");
            lcd_moveto(5, (screen_height/2)+1);
            for (i = 0;  i < 10;  i++) {
                struct key_event ke;
                if (key_get_event(&ke) && ((ke.button_info & ATMEL_BUTTON_STATE) == ATMEL_BUTTON_STATE_UP)) {
                    need_params = (ke.button_info & ATMEL_BUTTON_VALUE) != ATMEL_BUTTON_RETURN;
                    break;
                }
                CYGACC_CALL_IF_DELAY_US(50000);
                lcd_putc('.');
            }
        }

        while (need_params) {
            cursor_enable = false;
            lcd_clear();
            lcd_kbd(LCD_KBD_NORM);
            if (init_kbd_coord(CS_UL, "'q'")) {
                goto no_kbd;
            }
            if (init_kbd_coord(CS_UR, "BS ")) {
                goto no_kbd;
            }
            if (init_kbd_coord(CS_LL, "SHIFT")) {
                goto no_kbd;
            }
            if (init_kbd_coord(CS_LR, "'/'  ")) {
                goto no_kbd;
            }
            cursor_enable = true;
            if (close(kbd_limits[CS_UL].x, kbd_limits[CS_LL].x) &&
                close(kbd_limits[CS_UR].x, kbd_limits[CS_LR].x) &&
                close(kbd_limits[CS_UL].y, kbd_limits[CS_UR].y) &&
                close(kbd_limits[CS_LL].y, kbd_limits[CS_LR].y)) {
                // Save values so we don't need to repeat this
                cksum = (unsigned short)&_ipaq_LCD_params[0];
                for (i = 0;  i < 4;  i++) {
                    param = kbd_limits[i].x;
                    cksum ^= param;
                    _ipaq_LCD_params[i*2] = param;
                    param = kbd_limits[i].y;
                    cksum ^= param;
                    _ipaq_LCD_params[(i*2)+1] = param;
                }
                _ipaq_LCD_params[(4*2)+1] = cksum;
                break;
            }
        }

    no_kbd:
        // Munge the limits to allow for some slop
        if (kbd_limits[CS_UL].x < kbd_limits[CS_UR].x) {
            minX = min(kbd_limits[CS_UL].x, kbd_limits[CS_LL].x) - KBD_FUZZ;
            maxX = max(kbd_limits[CS_UR].x, kbd_limits[CS_LR].x) + KBD_FUZZ;
        } else {
            minX = min(kbd_limits[CS_UR].x, kbd_limits[CS_LR].x) - KBD_FUZZ;
            maxX = max(kbd_limits[CS_UL].x, kbd_limits[CS_LL].x) + KBD_FUZZ;
        }
        if (kbd_limits[CS_UL].y < kbd_limits[CS_LL].y) {
            minY = min(kbd_limits[CS_UL].y, kbd_limits[CS_UR].y) - KBD_FUZZ;
            maxY = max(kbd_limits[CS_LL].y, kbd_limits[CS_LR].y) + KBD_FUZZ;
        } else {
            minY = min(kbd_limits[CS_LR].y, kbd_limits[CS_LL].y) - KBD_FUZZ;
            maxY = max(kbd_limits[CS_UL].y, kbd_limits[CS_UR].y) + KBD_FUZZ;
        }
        cursor_enable = true;
        lcd_clear();
        if (kbd_active) {
            lcd_kbd(LCD_KBD_NORM);
        }
#if 0
        diag_printf("KBD Limits[] = %d/%d, %d/%d, %d/%d, %d/%d\n",
                    kbd_limits[CS_UL].x, kbd_limits[CS_UL].y,
                    kbd_limits[CS_UR].x, kbd_limits[CS_UR].y,
                    kbd_limits[CS_LL].x, kbd_limits[CS_LL].y,
                    kbd_limits[CS_LR].x, kbd_limits[CS_LR].y);
        diag_printf("KBD in %d/%d .. %d/%d\n", minX, minY, maxX, maxY);
        diag_printf("screen %d x %d\n", screen_height, screen_width);
#endif
    }
}

// Control state of LCD display - only called by logical I/O layers

void
lcd_on(bool enable)
{
    static bool enabled = true;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    if (cur != LCD_COMM_CHANNEL) 
        enable = false;  // Only enable display if LCD is the active "console"
    if (enable) {
        if (!enabled) {
            ipaq_EGPIO(SA1110_EIO_LCD_3V3|SA1110_EIO_LCD_CTRL|SA1110_EIO_LCD_5V|SA1110_EIO_LCD_VDD,
                       SA1110_EIO_LCD_3V3_ON|SA1110_EIO_LCD_CTRL_ON|SA1110_EIO_LCD_5V_ON|SA1110_EIO_LCD_VDD_ON);
            lcd_brightness(_lcd_brightness);
        }
        enabled = true;
    } else {
        if (enabled) {
            lcd_brightness(0);
            ipaq_EGPIO(SA1110_EIO_LCD_3V3|SA1110_EIO_LCD_CTRL|SA1110_EIO_LCD_5V|SA1110_EIO_LCD_VDD,
                       SA1110_EIO_LCD_3V3_OFF|SA1110_EIO_LCD_CTRL_OFF|SA1110_EIO_LCD_5V_OFF|SA1110_EIO_LCD_VDD_OFF);
        }
        enabled = false;
    }
}
#endif
