//=============================================================================
//
//      fb_support.c
//
//      Frame buffer support for Dreamcast
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
// Author(s):   t@keshi.org
// Contributors:t@keshi.org, gthomas
// Date:        2001-07-30
// Purpose:     Red Hat/eCos banner for display during boot
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_if.h>       // Virtual vector support
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#define LOGO_AT_TOP
#include "banner.xpm"
#include "font.h"

#define RESETREG	0xa05f8008
#define BORDERRGB	0xa05f8040
#define DISPLAYMODE	0xa05f8044
#define ALPHAMODE	0xa05f8048
#define DISPLAYALIGN	0xa05f804c
#define BASEOFFSET1	0xa05f8050
#define BASEOFFSET2	0xa05f8054
#define DISPLAYSIZE	0xa05f805c
#define SYNCMODE	0xa05f80d0
#define VERTICALRANGE	0xa05f80dc
#define HORIZPOSITION	0xa05f80ec
#define VERTPOSITION	0xa05f80f0
#define PALETTEMODE	0xa05f8108
#define VIDEOOUTPUT	0xa0702c00

static unsigned long dc_parm_vga_16bpp[] = {
    DISPLAYMODE,	0x00800005,
    BASEOFFSET1,	0,
    BASEOFFSET2,	640*2,
    DISPLAYSIZE,	(1<<20)+((480-1)<<10)+(640*2/4-1),
    SYNCMODE,		0x100,
    VERTPOSITION,	0x00230023,
    VERTICALRANGE,	0x00280208,
    HORIZPOSITION,	0x00000090,
    VIDEOOUTPUT,	0,
    0, 0,
};
    
static unsigned long dc_parm_vga_32bpp[] = {
    DISPLAYMODE,	0x0080000d,
    BASEOFFSET1,	0,
    BASEOFFSET2,	640*4,
    DISPLAYSIZE,	(1<<20)+((480-1)<<10)+(640*4/4-1),
    SYNCMODE,		0x100,
    VERTPOSITION,	0x00230023,
    VERTICALRANGE,	0x00280208,
    HORIZPOSITION,	0x00000090,
    VIDEOOUTPUT,	0,
    0, 0,
};

static unsigned long *dc_parm_vga[] = {
    dc_parm_vga_16bpp,
    dc_parm_vga_32bpp,
};

static unsigned long dc_parm_composite_16bpp[] = {
    DISPLAYMODE,	0x00000005,
    BASEOFFSET1,	0,
    BASEOFFSET2,	640*2,
    DISPLAYSIZE,	((640*2/4+1)<<20)+((240-1)<<10)+(640*2/4-1),
    SYNCMODE,		0x150,
    VERTPOSITION,	0x00120012,
    VERTICALRANGE,	0x00240204,
    HORIZPOSITION,	0x000000a4,
    VIDEOOUTPUT,	0x300,
    0, 0,
};
    
static unsigned long dc_parm_composite_32bpp[] = {
    DISPLAYMODE,	0x0000000d,
    BASEOFFSET1,	0,
    BASEOFFSET2,	640*4,
    DISPLAYSIZE,	((640*4/4+1)<<20)+((240-1)<<10)+(640*4/4-1),
    SYNCMODE,		0x150,
    VERTPOSITION,	0x00120012,
    VERTICALRANGE,	0x00240204,
    HORIZPOSITION,	0x000000a4,
    VIDEOOUTPUT,	0x300,
    0, 0,
};

static unsigned long *dc_parm_composite[] = {
    dc_parm_composite_16bpp,
    dc_parm_composite_32bpp,
};

static unsigned long dc_parm_interlace_16bpp[] = {
    DISPLAYMODE,	0x00000005,
    BASEOFFSET1,	0,
    BASEOFFSET2,	640*2,
    DISPLAYSIZE,	((640*2/4+1)<<20)+((240-1)<<10)+(640*2/4-1),
    SYNCMODE,		0x150,
    VERTPOSITION,	0x00120012,
    VERTICALRANGE,	0x00240204,
    HORIZPOSITION,	0x000000a4,
    VIDEOOUTPUT,	0,
    0, 0,
};
    
static unsigned long dc_parm_interlace_32bpp[] = {
    DISPLAYMODE,	0x0000000d,
    BASEOFFSET1,	0,
    BASEOFFSET2,	640*4,
    DISPLAYSIZE,	((640*4/4+1)<<20)+((240-1)<<10)+(640*4/4-1),
    SYNCMODE,		0x150,
    VERTPOSITION,	0x00120012,
    VERTICALRANGE,	0x00240204,
    HORIZPOSITION,	0x000000a4,
    VIDEOOUTPUT,	0,
    0, 0,
};

static unsigned long *dc_parm_interlace[] = {
    dc_parm_interlace_16bpp,
    dc_parm_interlace_32bpp,
};

/*
 *	Check cable type.
 *	0: VGA, 2: RGB, 3: Composite
 */

#define	PCTRA	0xff80002c
#define PDTRA	0xff800030

static int dcfb_cable_check(void)
{
    unsigned long temp;
    HAL_READ_UINT32(PCTRA, temp);
    temp &= 0xfff0ffff;
    temp |= 0x000a0000;
    HAL_WRITE_UINT32(PCTRA, temp);
    HAL_READ_UINT16(PDTRA, temp);
    return (temp>>8)&3;
}


// Physical dimensions of LCD display
#define DISPLAY_WIDTH  640
#define DISPLAY_HEIGHT 480

#define LCD_WIDTH  640
#define LCD_HEIGHT 480
#define LCD_DEPTH   16

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

// Physical screen info
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

static unsigned short *framebuffer = (void*)0xa5000000;

// Functions
static void lcd_drawc(cyg_int8 c, int x, int y);

static inline void
set_pixel(int row, int col, unsigned short val)
{
    framebuffer[row*640+col] = val;
}

// Clear screen
void
lcd_clear(void)
{
    int row, col;
    int pos;

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
    cyg_uint16 *p1, *p2;
    // Now the physical screen
    for (row = FONT_HEIGHT*(screen_start+1);  row < LCD_HEIGHT;  row++) {
        p1 = &framebuffer[(row-FONT_HEIGHT)*LCD_WIDTH+0];
        p2 = &framebuffer[row*LCD_WIDTH+0];
        for (col = 0;  col < LCD_WIDTH;  col++) {
            *p1++ = *p2++;
        }
    }
    for (row = LCD_HEIGHT-FONT_HEIGHT;  row < LCD_HEIGHT;  row++) {
        p1 = &framebuffer[row*LCD_WIDTH+0];
        for (col = 0;  col < LCD_WIDTH;  col++) {
            *p1++ = bg;
        }
    }
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


static int  _timeout = 500;


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


void fb_init(int depth)
{
    int cable = dcfb_cable_check();
    unsigned long **parm_list[4] = {
        dc_parm_vga, dc_parm_vga, dc_parm_interlace, dc_parm_composite,
    };
    unsigned long **dc_parms = parm_list[cable];

    unsigned long a, d, *p;
  
    switch(depth) {
    default:
    case 16:
        p = dc_parms[0];
        break;
    case 32:
        p = dc_parms[1];
        break;
    }
  
    HAL_WRITE_UINT32(RESETREG, 0);
    HAL_WRITE_UINT32(BORDERRGB, 0);

    while(1) {
        a = *p++; d = *p++;
        if (!a) break;
        HAL_WRITE_UINT32(a, d);
    }

    lcd_clear();
}


static cyg_bool
lcd_comm_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
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
}

static void
lcd_comm_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
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
    int ret = 0;

    switch (__func) {
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
	break;
    default:
        break;
    }

    return ret;
}

static int
lcd_comm_isr(void *__ch_data, int* __ctrlc, 
           CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    return 0;
}

void fb_comm_init(void)
{
    static int init = 0;
  
    if (!init) {
        hal_virtual_comm_table_t* comm;
        int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    
        // Setup procs in the vector table
        CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
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
    }
}

//-----------------------------------------------------------------------------
// End of fb_support.c
