//==========================================================================
//
//      vnc-server.h
//
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-08-22
// Purpose:
// Description:  Header file for VNC Server
//
//####DESCRIPTIONEND####
//
//========================================================================*/


#include <cyg/kernel/kapi.h>  /* Kernel API */
#include <pkgconf/vnc_server.h>

/* Type to hold the frame format details */
typedef struct
{
    cyg_uint16 frame_width;
    cyg_uint16 frame_height;
    void *     frame_buffer;
    bool       rgb332;
    bool       rgb555;
    bool       rgb565;
    bool       bgr233;
    bool       truecolor0888;
} vnc_frame_format_t;


#ifdef CYGNUM_VNC_SERVER_INCLUDE_VNC_PRINTF
/* The typedefs for MWIMAGEBITS and MWCFONT and required to use  */
/* the font definition files in the src/fonts/ directory.  These */
/* typedefs are repeated in the device.h file also in the        */
/* src/fonts/ directory                                          */
typedef unsigned short  MWIMAGEBITS;    /* bitmap image unit size*/
typedef struct {
    char *      name;       /* font name*/
    int     maxwidth;   /* max width in pixels*/
    int     height;     /* height in pixels*/
    int     ascent;     /* ascent (baseline) height*/
    int     firstchar;  /* first character in bitmap*/
    int     size;       /* font size in characters*/
    MWIMAGEBITS *   bits;       /* 16-bit right-padded bitmap data*/
    unsigned short *offset;     /* 256 offsets into bitmap data*/
    unsigned char * width;      /* 256 character widths or 0 if fixed*/
} MWCFONT;

/* Type to hold rectangle size details */
typedef struct
{
    int chars;
    cyg_uint16 width;
    cyg_uint16 height;
} vnc_printf_return_t;
#endif

/* Type for colour values */
#if defined(CYGNUM_VNC_SERVER_PIXEL_RGB332) || defined(CYGNUM_VNC_SERVER_PIXEL_BGR233)
typedef cyg_uint8 vnc_color_t;
typedef cyg_uint8 vnc_colour_t;
#elif defined(CYGNUM_VNC_SERVER_PIXEL_RGB555) || defined(CYGNUM_VNC_SERVER_PIXEL_RGB565)
typedef cyg_uint16 vnc_color_t;
typedef cyg_uint16 vnc_colour_t;
#elif defined(CYGNUM_VNC_SERVER_PIXEL_TRUECOLOR0888)
typedef cyg_uint32 vnc_color_t;
typedef cyg_uint32 vnc_colour_t;
#else
#error "Unsupported color model"
#endif


/* Driver function prototypes */
vnc_frame_format_t* VncGetInfo(void);
void  VncInit(vnc_colour_t colour);
void VncDrawPixel(cyg_uint16 x, cyg_uint16 y, vnc_colour_t colour);
vnc_colour_t VncReadPixel(cyg_uint16 x, cyg_uint16 y);
void VncDrawHorzLine(cyg_uint16 x1, cyg_uint16 x2, cyg_uint16 y, vnc_colour_t colour);
void VncDrawVertLine(cyg_uint16 x, cyg_uint16 y1, cyg_uint16 y2, vnc_colour_t colour);
void VncFillRect(cyg_uint16 x1, cyg_uint16 y1, cyg_uint16 x2, cyg_uint16 y2, vnc_colour_t colour);
void VncCopyRect(cyg_uint16 x1, cyg_uint16 y1, cyg_uint16 width, cyg_uint16 height, cyg_uint16 x2, cyg_uint16 y2);
void VncCopyRect2Buffer(cyg_uint16 x, cyg_uint16 y, cyg_uint16 width, cyg_uint16 height,
                        void *buffer, cyg_uint16 buff_w, cyg_uint16 buff_h, cyg_uint16 x_off, cyg_uint16 y_off);
void VncCopyBuffer2Rect(void *buffer, cyg_uint16 buff_w, cyg_uint16 buff_h, cyg_uint16 x_off, cyg_uint16 y_off,
                         cyg_uint16 x, cyg_uint16 y, cyg_uint16 width, cyg_uint16 height);
void VncCopyBuffer2RectMask( void *buffer, cyg_uint16 buff_w, cyg_uint16 buff_h ,cyg_uint16 x_off, cyg_uint16 y_off,
                             cyg_uint16 x, cyg_uint16 y, cyg_uint16 width, cyg_uint16 height, vnc_colour_t col);
void VncSoundBell(void);
#ifdef CYGNUM_VNC_SERVER_INCLUDE_VNC_PRINTF
vnc_printf_return_t VncPrintf(MWCFONT* font, int do_print, vnc_colour_t colour, int x, int y, const char *fmt, ... );
#endif


/* Macros to convert from RGB to colour values */
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB332
#define VNC_RGB2COL(r,g,b) (vnc_colour_t)(((((cyg_uint8)r)&0xE0) >> 0)  \
                                      |((((cyg_uint8)g)&0xE0) >> 3)  \
                                      |((((cyg_uint8)b)&0xC0) >> 6))
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_BGR233
#define VNC_RGB2COL(r,g,b) (vnc_colour_t)(((((cyg_uint8)r)&0xE0) >> 5)  \
                                      |((((cyg_uint8)g)&0xE0) >> 2)  \
                                      |((((cyg_uint8)b)&0xC0) >> 0))
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB555
#define VNC_RGB2COL(r,g,b) (vnc_colour_t)(((((cyg_uint8)r)&0xF8) << 7)  \
                                       |((((cyg_uint8)g)&0xF8) << 2)  \
                                       |((((cyg_uint8)b)&0xF8) >> 3))
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB565
#define VNC_RGB2COL(r,g,b) (vnc_colour_t)(((((cyg_uint8)r)&0xF8) << 8)  \
                                       |((((cyg_uint8)g)&0xFC) << 3)  \
                                       |((((cyg_uint8)b)&0xF8) >> 3))
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_TRUECOLOR0888
#define VNC_RGB2COL(r,g,b) (vnc_colour_t)(((((cyg_uint8)r)&0xFF) << 16)  \
                                       |((((cyg_uint8)g)&0xFF) << 8)  \
                                       |((((cyg_uint8)b)&0xFF) << 0))
#endif


/* 16 defined colours for application use */
#define VNC_BLACK       VNC_RGB2COL( 0  , 0  , 0   )
#define VNC_BLUE        VNC_RGB2COL( 0  , 0  , 128 )
#define VNC_GREEN       VNC_RGB2COL( 0  , 128, 0   )
#define VNC_CYAN        VNC_RGB2COL( 0  , 128, 128 )
#define VNC_RED         VNC_RGB2COL( 128, 0  , 0   )
#define VNC_MAGENTA     VNC_RGB2COL( 128, 0  , 128 )
#define VNC_BROWN       VNC_RGB2COL( 128, 64 , 0   )
#define VNC_LTGRAY      VNC_RGB2COL( 192, 192, 192 )
#define VNC_LTGREY      VNC_RGB2COL( 192, 192, 192 )
#define VNC_GRAY        VNC_RGB2COL( 128, 128, 128 )
#define VNC_GREY        VNC_RGB2COL( 128, 128, 128 )
#define VNC_LTBLUE      VNC_RGB2COL( 0  , 0  , 255 )
#define VNC_LTGREEN     VNC_RGB2COL( 0  , 255, 0   )
#define VNC_LTCYAN      VNC_RGB2COL( 0  , 255, 255 )
#define VNC_LTRED       VNC_RGB2COL( 255, 0  , 0   )
#define VNC_LTMAGENTA   VNC_RGB2COL( 255, 0  , 255 )
#define VNC_YELLOW      VNC_RGB2COL( 255, 255, 0   )
#define VNC_WHITE       VNC_RGB2COL( 255, 255, 255 )

#ifdef CYGNUM_VNC_SERVER_INCLUDE_VNC_PRINTF
/* Compiled in fonts*/
extern MWCFONT font_rom8x16, font_rom8x8;
extern MWCFONT font_winFreeSansSerif11x13;
extern MWCFONT font_winFreeSystem14x16;  /* Default font */
extern MWCFONT font_helvB10, font_helvB12, font_helvR10;
extern MWCFONT font_X5x7, font_X6x13;
#endif
