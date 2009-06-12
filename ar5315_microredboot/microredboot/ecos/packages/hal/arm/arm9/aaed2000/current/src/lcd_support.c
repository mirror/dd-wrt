//==========================================================================
//
//        Lcd_support.c
//
//        Agilent AAED2000 - LCD support routines
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
// Date:          2001-11-03
// Description:   Simple LCD support
//####DESCRIPTIONEND####

#include <pkgconf/hal.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_if.h>       // Virtual vector support
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#include <cyg/hal/aaed2000.h>  // Board definitions
#include <cyg/hal/lcd_support.h>
#include <cyg/hal/hal_cache.h>

#include <string.h>

#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
# ifdef CYGINT_ISO_STDIO_FORMATTED_IO
#  include <stdio.h>  // sscanf
# endif
#endif

#include CYGHWR_MEMORY_LAYOUT_H
#define LCD_FRAMEBUFFER CYGMEM_REGION_lcd
static cyg_uint32 lcd_framebuffer;

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

// Physical dimensions of LCD display
#define DISPLAY_WIDTH  640
#define DISPLAY_HEIGHT 480

// Logical layout
#ifdef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
#define LCD_WIDTH  DISPLAY_HEIGHT
#define LCD_HEIGHT DISPLAY_WIDTH
#else
#define LCD_WIDTH  DISPLAY_WIDTH
#define LCD_HEIGHT DISPLAY_HEIGHT
#endif
#define LCD_DEPTH   16

#define RGB_RED(x)   (((x)&0x1F)<<0)
#define RGB_GREEN(x) (((x)&0x1F)<<5)
#define RGB_BLUE(x)  (((x)&0x1F)<<10)

// Physical screen info
//static int lcd_depth  = LCD_DEPTH;  // Should be 1, 2, or 4
static int lcd_bpp;
#ifdef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
static int lcd_width  = LCD_WIDTH;
#endif
static int lcd_height = LCD_HEIGHT;

// Black on light blue
static int bg = RGB_RED(0x17) | RGB_GREEN(0x17) | RGB_BLUE(0x1F);
#ifdef CYGSEM_AAED2000_LCD_COMM
static int fg = RGB_RED(0) | RGB_GREEN(0) | RGB_BLUE(0);
#endif

// Compute the location for a pixel within the framebuffer
static cyg_uint32 *
lcd_fb(int row, int col)
{
    return (cyg_uint32 *)(lcd_framebuffer+(((row*DISPLAY_WIDTH)+col)*2));
}

void
lcd_on(bool enable)
{
    cyg_uint32 ctl;

    if (!enable) return;  // FIXME - need a way to [safely] blank screen

    // Turn on PWM [pulse wave modulated] voltage pump
    HAL_WRITE_UINT32(AAEC_PUMP_CONTROL,    0x800);  // Magic FIXME
    HAL_WRITE_UINT32(AAEC_PUMP_FREQUENCY, 0x3733);  // Magic FIXME
    // Turn on Power
    HAL_READ_UINT32(AAED_EXT_GPIO, ctl);  
    ctl |= AAED_EXT_GPIO_LCD_PWR_EN;
    HAL_WRITE_UINT32(AAED_EXT_GPIO, ctl);  
    // Power must be on for 0..20ms before enabling signals
    CYGACC_CALL_IF_DELAY_US(500);  // 500us should be long enough
    HAL_READ_UINT32(AAEC_LCD_CONTROL, ctl);
    ctl |= AAEC_LCD_CONTROL_ENAB | AAEC_LCD_CONTROL_PWR_ENAB;
    HAL_WRITE_UINT32(AAEC_LCD_CONTROL, ctl);
}

// Initialize LCD hardware

void
lcd_init(int depth)
{
    cyg_uint32 ctl;

    HAL_VIRT_TO_PHYS_ADDRESS(LCD_FRAMEBUFFER, lcd_framebuffer);
    lcd_clear();
    HAL_READ_UINT32(AAEC_LCD_CONTROL, ctl);
    if (ctl & (AAEC_LCD_CONTROL_ENAB | AAEC_LCD_CONTROL_PWR_ENAB)) {
        // LCD currently on - turn it off carefully
        ctl &= ~(AAEC_LCD_CONTROL_ENAB | AAEC_LCD_CONTROL_PWR_ENAB);
        HAL_WRITE_UINT32(AAEC_LCD_CONTROL, ctl);
        // Now wait a little
        CYGACC_CALL_IF_DELAY_US(500);
        // Turn off Power
        HAL_READ_UINT32(AAED_EXT_GPIO, ctl);  
        ctl &= ~AAED_EXT_GPIO_LCD_PWR_EN;
        HAL_WRITE_UINT32(AAED_EXT_GPIO, ctl);  
        // Now wait for 1.0 second
        CYGACC_CALL_IF_DELAY_US(1000*1000);
    }
    HAL_WRITE_UINT32(AAEC_LCD_CONTROL, 0x00010028);   // Magic FIXME
    HAL_WRITE_UINT32(AAEC_LCD_TIMING0, 0x2B135F9C);   // Magic FIXME
    HAL_WRITE_UINT32(AAEC_LCD_TIMING1, 0x211401DF);   // Magic FIXME
    HAL_WRITE_UINT32(AAEC_LCD_TIMING2, 0x067F1820);   // Magic FIXME
    HAL_WRITE_UINT32(AAEC_LCD_TIMING3, 0x00000000);   // Magic FIXME
    HAL_WRITE_UINT32(AAEC_LCD_UPBASE,  lcd_framebuffer);
    HAL_WRITE_UINT32(AAEC_LCD_LPBASE,  lcd_framebuffer);
#if 0 // For some reason, this crashes miserably
    lcd_framebuffer = LCD_FRAMEBUFFER;  // Logical operations use cached space
#endif
    lcd_on(true);
    lcd_bpp = 16;
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
    info->fb = (void*)LCD_FRAMEBUFFER;
    info->rlen = DISPLAY_WIDTH * 2;
    info->type = FB_TRUE_RGB555;
    return 1; // Information valid
}

// Clear screen
void
lcd_clear(void)
{
#ifndef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
    cyg_uint32 *fb_row0, *fb_rown;
    cyg_uint32 _bg = (bg<<16)|bg;

    fb_row0 = lcd_fb(0, 0);
    fb_rown = lcd_fb(lcd_height, 0);
    while (fb_row0 != fb_rown) {
        *fb_row0++ = _bg;
    }
#else
    int row, col;
    for (row = 0;  row < lcd_height;  row++) {
        for (col = 0;  col < lcd_width;  col++) {
            set_pixel(row, col, bg);
        }
    }
#endif
}

#ifdef CYGSEM_AAED2000_LCD_COMM

//
// Additional support for LCD/Keyboard as 'console' device
//

#ifdef CYGOPT_AAED2000_LCD_COMM_LOGO
#include "banner.xpm"
#endif
#include "font.h"

// Virtual screen info
static int curX = 0;  // Last used position
static int curY = 0;
//static int width = LCD_WIDTH / (FONT_WIDTH*NIBBLES_PER_PIXEL);
//static int height = LCD_HEIGHT / (FONT_HEIGHT*SCREEN_SCALE);

#define SCREEN_PAN            20
#define SCREEN_WIDTH          80
#define SCREEN_HEIGHT         (LCD_HEIGHT/FONT_HEIGHT)
#define VISIBLE_SCREEN_WIDTH  (LCD_WIDTH/FONT_WIDTH)
#define VISIBLE_SCREEN_HEIGHT (LCD_HEIGHT/FONT_HEIGHT)
static char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
static int screen_height = SCREEN_HEIGHT;
static int screen_width = SCREEN_WIDTH;
static int screen_pan = 0;

// Usable area on screen [logical pixel rows]
static int screen_start = 0;                       
static int screen_end = LCD_HEIGHT/FONT_HEIGHT;

static bool cursor_enable = true;

// Functions
static void lcd_drawc(cyg_int8 c, int x, int y);

// Note: val is a 16 bit, RGB555 value which must be mapped
// onto a 12 bit value.
#define RED(v)   ((v>>12) & 0x0F)
#define GREEN(v) ((v>>7) & 0x0F)
#define BLUE(v)  ((v>>1) & 0x0F)
#ifdef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
#error PORTRAIT MODE NOT IMPLEMENTED
// Translate coordinates, rotating clockwise 90 degrees
static void
set_pixel(int row, int col, unsigned short val)
{
//    fp->pixels[col][(DISPLAY_WIDTH-1)-row] = val;
    int _row = (240-1) - col;
    int _col = row;
    unsigned char *pxptr = (unsigned char *)(0xC0000000 + (_row * 480) + ((_col * 3) / 2));

    diag_printf("%s\n", __FUNCTION__);  return;
    if ((row >= LCD_HEIGHT) || (col >= LCD_WIDTH)) return;
    if (0)
    {
        int old = start_console();
        diag_printf("row=%d/%d, col=%d/%d, pxptr = %p\n", row, _row, col, _col, pxptr);
        end_console(old);
    }
    if ((row % 2) == 0) {
        // Even row
        *pxptr++ = RED(val) | (GREEN(val) << 4);
        *pxptr = (*pxptr & 0xF0) | BLUE(val);   
    } else {
        // Odd row
        *pxptr = (*pxptr & 0x0F) | (RED(val) << 4);
        *++pxptr = GREEN(val) | (BLUE(val) << 4);
    }
}
#else

static void
set_pixel(int row, int col, unsigned short val)
{
    unsigned short *pix = (unsigned short *)lcd_fb(row, col);
    *pix = val;
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

#ifdef CYGOPT_AAED2000_LCD_COMM_LOGO
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

#ifdef CYGOPT_AAED2000_LCD_COMM_LOGO_TOP
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
#ifdef CYGOPT_AAED2000_LCD_COMM_LOGO_TOP
    screen_start = (nrows + (FONT_HEIGHT-1))/FONT_HEIGHT;
    screen_end = LCD_HEIGHT/FONT_HEIGHT;
    return offset+nrows;
#else    
    screen_start = 0;
    screen_height = offset / FONT_HEIGHT;
    screen_end = screen_height;
    return offset;
#endif
}
#endif

void
lcd_screen_clear(void)
{
    int row, col, pos;
    for (row = 0;  row < screen_height;  row++) {
        for (col = 0;  col < screen_width;  col++) {
            screen[row][col] = ' ';
        }
    }
#ifdef CYGOPT_AAED2000_LCD_COMM_LOGO
    // Note: Row 0 seems to wrap incorrectly
#ifdef CYGOPT_AAED2000_LCD_COMM_LOGO_TOP
    pos = 0;
#else
    pos = (LCD_HEIGHT-1);
#endif
    show_xpm(banner_xpm, pos);
#endif // CYGOPT_AAED2000_LCD_COMM_LOGO
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
    int xoff, yoff;
#ifndef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
    cyg_uint32 *fb;
#endif


    if ((x < 0) || (x >= VISIBLE_SCREEN_WIDTH) || 
        (y < 0) || (y >= screen_height)) return;  
    for (l = 0;  l < FONT_HEIGHT;  l++) {
        bits = font_table[c-FIRST_CHAR][l]; 
        yoff = y*FONT_HEIGHT + l;
        xoff = x*FONT_HEIGHT;
#ifndef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
        // Caution - only works for little-endian & font sizes multiple of 2
        fb = lcd_fb(yoff, xoff);
        for (p = 0;  p < FONT_WIDTH;  p += 2) {
            switch (bits & 0x03) {
            case 0:
                *fb++ = (bg << 16) | bg;
                break;
            case 1:
                *fb++ = (bg << 16) | fg;
                break;
            case 2:
                *fb++ = (fg << 16) | bg;
                break;
            case 3:
                *fb++ = (fg << 16) | fg;
                break;
            }
            bits >>= 2;
        }
#else
        for (p = 0;  p < FONT_WIDTH;  p++) {
            set_pixel(yoff, xoff + p, (bits & 0x01) ? fg : bg);
            bits >>= 1;
        }
#endif
    }
}

static void
lcd_refresh(void)
{
    int row, col;

    for (row = screen_start;  row < screen_height;  row++) {
        for (col = 0;  col < VISIBLE_SCREEN_WIDTH;  col++) {
            if ((col+screen_pan) < screen_width) {
                lcd_drawc(screen[row][col+screen_pan], col, row);
            } else {
                lcd_drawc(' ', col, row);
            }
        }
    }
    if (cursor_enable) {
        lcd_drawc(CURSOR_ON, curX-screen_pan, curY);
    }
}

static void
lcd_scroll(void)
{
    int col;
    cyg_uint8 *c1;
    cyg_uint32 *lc0, *lc1, *lcn;
#ifndef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
    cyg_uint32 *fb_row0, *fb_row1, *fb_rown;
#endif

    // First scroll up the virtual screen
#if ((SCREEN_WIDTH%4) != 0)
#error Scroll code optimized for screen with multiple of 4 columns
#endif
    lc0 = (cyg_uint32 *)&screen[0][0];
    lc1 = (cyg_uint32 *)&screen[1][0];
    lcn = (cyg_uint32 *)&screen[screen_height][0];
    while (lc1 != lcn) {
        *lc0++ = *lc1++;
    }
    c1 = &screen[screen_height-1][0];
    for (col = 0;  col < screen_width;  col++) {
        *c1++ = 0x20;
    }
#ifdef CYGSEM_AAED2000_LCD_PORTRAIT_MODE
    // Redrawing the screen in this mode is hard :-)
    lcd_refresh();
#else
    fb_row0 = lcd_fb(screen_start*FONT_HEIGHT, 0);
    fb_row1 = lcd_fb((screen_start+1)*FONT_HEIGHT, 0);
    fb_rown = lcd_fb(screen_end*FONT_HEIGHT, 0);
#if 1
    while (fb_row1 != fb_rown) {
        *fb_row0++ = *fb_row1++;
    }
#else
    // Optimized ARM assembly "move" code
    asm __volatile(
        "mov r0,%0;"
        "mov r1,%1;"
        "mov r2,%2;"
        "10: ldmia r1!,{r3-r10};"
        "stmia r0!,{r3-r10};"
        "cmp r1,r2;"
        "bne 10b"
        :
        : "r"(fb_row0), "r"(fb_row1), "r"(fb_rown)
        : "r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10"
        );
#endif
    // Erase bottom line
    for (col = 0;  col < screen_width;  col++) {
        lcd_drawc(' ', col, screen_end-1);
    }
#endif
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

    // FIXME
    return 0;
}

int
_lcd_printf(char const *fmt, ...)
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

//
// Support LCD/keyboard (PS2) as a virtual I/O channel
//   Adapted from i386/pcmb_screen.c
//

static int  _timeout = 500;

static cyg_bool
lcd_comm_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    if( !aaed2000_KeyboardTest() )
        return false;
    *ch = aaed2000_KeyboardGetc();
    return true;
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

    delay_count = _timeout * 2; // delay in .5 ms steps
    for(;;) {
        res = lcd_comm_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        CYGACC_CALL_IF_DELAY_US(500);
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
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
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
#else
    return 0;
#endif
}

void
lcd_comm_init(void)
{
    static int init = 0;

    if (!init) {
        hal_virtual_comm_table_t* comm;
        int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

        init = 1;
        lcd_on(false);
        if (!aaed2000_KeyboardInit()) {
            // No keyboard - no LCD display
            return;
        }
        // Initialize screen
        cursor_enable = true;
        lcd_init(16);
        lcd_screen_clear();

        // Setup procs in the vector table
        CYGACC_CALL_IF_SET_CONSOLE_COMM(1);  // FIXME - should be controlled by CDL
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
    }
}

#ifdef CYGPKG_REDBOOT
#include <redboot.h>

// Get here when RedBoot is idle.  If it's been long enough, then
// dim the LCD.  The problem is - how to determine other activities
// so at this doesn't get in the way.  In the default case, this will
// be called from RedBoot every 10ms (CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT)

#define MAX_IDLE_TIME (30*100)

static void
idle(bool is_idle)
{
    static int idle_time = 0;
    static bool was_idled = false;

    if (is_idle) {
        if (!was_idled) {
            if (++idle_time == MAX_IDLE_TIME) {
                was_idled = true;
                lcd_on(false);
            }
        }
    } else {        
        idle_time = 0;
        if (was_idled) {
            was_idled = false;
                lcd_on(true);
        }
    }
}

RedBoot_idle(idle, RedBoot_AFTER_NETIO);
#endif
#endif
