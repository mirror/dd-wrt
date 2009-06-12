//==========================================================================
//
//      scr_vnc_ecos.c
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
// Description:  Microwindows screen driver for VNC server on eCos
//
//####DESCRIPTIONEND####
//
//========================================================================*/



#define _GNU_SOURCE 1

#include <pkgconf/system.h>

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <cyg/hal/drv_api.h>
#include <cyg/infra/diag.h>
#include <cyg/io/io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "device.h"
#include "genfont.h"
#include "genmem.h"
#include <vnc-server.h>

/* In genmem.c*/
MWBOOL  set_subdriver(PSD psd, PSUBDRIVER subdriver, MWBOOL init);

/* Prototypes for driver functions */
static int vnc_init(PSD psd);
static PSD  vnc_open(PSD psd);
static void vnc_close(PSD psd);
static void vnc_getscreeninfo(PSD psd,PMWSCREENINFO psi);
static void vnc_drawpixel(PSD psd,MWCOORD x, MWCOORD y, MWPIXELVAL c);
static MWPIXELVAL vnc_readpixel(PSD psd,MWCOORD x, MWCOORD y);
static void vnc_drawhorizline(PSD psd,MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c);
static void vnc_drawvertline(PSD psd,MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c);
static void vnc_fillrect(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c);
static void vnc_blit(PSD , MWCOORD, MWCOORD, MWCOORD, MWCOORD, PSD, MWCOORD, MWCOORD, long);
static void vnc_stretchblit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD dstw,
                            MWCOORD dsth, PSD srcpsd, MWCOORD srcx, MWCOORD srcy,
                            MWCOORD srcw, MWCOORD srch, long op);
static void vnc_drawarea(PSD psd, driver_gc_t *gc, int op);
MWBOOL vnc_mapmemgc(PSD, MWCOORD, MWCOORD, int, int, int, int, void *);



SCREENDEVICE	scrdev = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
    vnc_open,
    vnc_close,
    vnc_getscreeninfo,
	NULL,
    vnc_drawpixel,     /* DrawPixel subdriver*/
    vnc_readpixel,     /* ReadPixel subdriver*/
    vnc_drawhorizline, /* DrawHorzLine subdriver*/
    vnc_drawvertline,  /* DrawVertLine subdriver*/
    vnc_fillrect,      /* FillRect subdriver*/
	gen_fonts,
    vnc_blit,          /* Blit subdriver*/
	NULL,			   /* PreSelect*/
    NULL,              /* DrawArea subdriver*/
	NULL,			   /* SetIOPermissions*/
	gen_allocatememgc,
    vnc_mapmemgc,
    gen_freememgc,
    vnc_stretchblit,   /* StretchBlit subdriver*/
	NULL	           /* SetPortrait*/
};

SUBDRIVER vnc_subdriver = {
    vnc_init,
    vnc_drawpixel,
    vnc_readpixel,
    vnc_drawhorizline,
    vnc_drawvertline,
    vnc_fillrect,
    vnc_blit,
    vnc_drawarea,
    vnc_stretchblit
};

/* Static variables*/
static int status;		/* 0=never inited, 1=once inited, 2=inited. */
static vnc_frame_format_t *frame_format;

/* Calc linelen and mmap size, return 0 on fail*/
static int vnc_init(PSD psd)
{
    if (!psd->size)
    {
        psd->size = psd->yres * psd->xres * psd->bpp / 8;
        /* convert linelen from byte to pixel len for bpp 16, 24, 32*/
        psd->linelen = psd->xres;
    }

    return 1;
}

/* Initialise the VNC framebuffer */
static PSD vnc_open(PSD psd)
{
    /* Get frame format details */
    frame_format = VncGetInfo();

    psd->xres = psd->xvirtres = frame_format->frame_width;
    psd->yres = psd->yvirtres = frame_format->frame_height;
    psd->portrait = MWPORTRAIT_NONE;
    psd->planes = 1;  /* Should probably find out what this means */

    if (frame_format->rgb332)
    {
        psd->bpp = 8;
        psd->ncolors = 0xFF + 1;
        psd->pixtype = MWPF_TRUECOLOR332;
    }
    else if (frame_format->rgb555)
    {
        psd->bpp = 16;
        psd->ncolors = 0x7FFF + 1;
        psd->pixtype = MWPF_TRUECOLOR555;
    }
    else if (frame_format->rgb565)
    {
        psd->bpp = 16;
        psd->ncolors = 0xFFFF + 1;
        psd->pixtype = MWPF_TRUECOLOR565;
    }
    else if (frame_format->bgr233)
    {
        psd->bpp = 8;
        psd->ncolors = 0xFF + 1;
        psd->pixtype = MWPF_TRUECOLOR233;
    }
    else if (frame_format->truecolor0888)
    {
        psd->bpp = 32;
        psd->ncolors = 0xFFFFFF + 1;
        psd->pixtype = MWPF_TRUECOLOR0888;
    }
    else
    {
        EPRINTF("Unsupported display type\n");
        goto fail;
    }

    psd->linelen = frame_format->frame_width * psd->bpp / 8;;  /* What is linelen?  - linelen in bytes for now...*/
    psd->size = psd->xres * psd->yres * psd->bpp / 8;
    psd->flags = PSF_SCREEN | PSF_HAVEBLIT;
    psd->addr = frame_format->frame_buffer;  /* Test */
//    psd->addr = NULL;  /* We do not want MW to access the frame buffer directly */

    /* Initialise the frame buffer (white) */
    VncInit(VNC_WHITE);

    /* We always use our own subdriver */
    psd->orgsubdriver = &vnc_subdriver;


    status = 2;
    return psd;	/* success*/

 fail:
    return NULL;
}


/* Close framebuffer*/
static void vnc_close(PSD psd)
{
    printf("%s - NOT IMPLEMENTED\n", __FUNCTION__);
}


static void vnc_getscreeninfo(PSD psd,PMWSCREENINFO psi)
{
    psi->rows = psd->yvirtres;
    psi->cols = psd->xvirtres;
    psi->planes = psd->planes;
    psi->bpp = psd->bpp;
    psi->ncolors = psd->ncolors;
    psi->pixtype = psd->pixtype;
    psi->fonts = NUMBER_FONTS;
    psi->portrait = psd->portrait;
    psi->fbdriver = true;

    switch (psd->pixtype) {
    case MWPF_TRUECOLOR332:
        psi->rmask = 0xE0;
        psi->gmask = 0x1C;
        psi->bmask = 0x03;
        break;
    case MWPF_TRUECOLOR233:
        psi->rmask = 0x07;
        psi->gmask = 0x38;
        psi->bmask = 0xC0;
        break;
    case MWPF_TRUECOLOR555:
        psi->rmask = 0x7c00;
        psi->gmask = 0x03e0;
        psi->bmask = 0x001f;
        break;
    case MWPF_TRUECOLOR565:
        psi->rmask = 0xf800;
        psi->gmask = 0x07e0;
        psi->bmask = 0x001f;
        break;
    case MWPF_TRUECOLOR0888:
        psi->rmask = 0xFF0000;
        psi->gmask = 0x00FF00;
        psi->bmask = 0x0000FF;
        break;
    default:
        printf("%s - unsupported pixtype\n", __FUNCTION__);
        psi->rmask = 0xff;
        psi->gmask = 0xff;
        psi->bmask = 0xff;
        break;
    }

    /* Need to figure out better values possibly */
    psi->xdpcm = 27;    /* assumes screen width of 24 cm */
    psi->ydpcm = 27;    /* assumes screen height of 18 cm */
}


static void vnc_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
    VncDrawPixel(x, y, (vnc_colour_t)c);
}


static MWPIXELVAL vnc_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
    return VncReadPixel(x, y);
}


static void vnc_drawhorizline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
    VncDrawHorzLine(x1, x2, y, (vnc_colour_t)c);
}


static void vnc_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
    VncDrawVertLine(x, y1, y2, (vnc_colour_t)c);
}


static void vnc_fillrect(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2, MWPIXELVAL c)
{
    VncFillRect(x1, y1, x2, y2, (vnc_colour_t)c);
}


static void vnc_blit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD w, MWCOORD h,
                     PSD srcpsd, MWCOORD srcx, MWCOORD srcy, long op)
{
    if (op != 0)
    {
        diag_printf("vnc_blit(): op = 0x%x not supported\n", op);
    }

    if (srcpsd->addr == frame_format->frame_buffer && dstpsd->addr != frame_format->frame_buffer)
    {
        /* Copying rectangle from VNC frame buffer to supplied buffer */
        VncCopyRect2Buffer(srcx, srcy, w, h, dstpsd->addr, dstpsd->xres, dstpsd->yres, dstx, dsty);
    }
    else if (srcpsd->addr != frame_format->frame_buffer && dstpsd->addr == frame_format->frame_buffer)
    {
        /* Copying rectangle from a supplied buffer to the VNC frame buffer */
        VncCopyBuffer2Rect(srcpsd->addr, srcpsd->xres, srcpsd->yres, srcx, srcy, dstx, dsty, w, h);
    }
    else if (srcpsd->addr == frame_format->frame_buffer && dstpsd->addr == frame_format->frame_buffer)
    {
        /* Copying rectangle from VNC frame buffer to VNC frame buffer */
        VncCopyRect(srcx, srcy, w, h, dstx, dsty);
    }
    else
    {
        diag_printf("vnc_blit(): Error unsupported operation\n");
    }
}

static void vnc_stretchblit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD dstw,
                            MWCOORD dsth, PSD srcpsd, MWCOORD srcx, MWCOORD srcy,
                            MWCOORD srcw, MWCOORD srch, long op)
{
    diag_printf("vnc_stretch_blit() not implemented\n");
}

static void vnc_drawarea(PSD psd, driver_gc_t *gc, int op)
{
    diag_printf("vnc_drawarea() not implemented\n");
}


/*
 * Initialize memory device with passed parms,
 * select suitable framebuffer subdriver,
 * and set subdriver in memory device.
 */
MWBOOL vnc_mapmemgc(PSD mempsd,MWCOORD w,MWCOORD h,int planes,int bpp,int linelen,
    int size,void *addr)
{
    PSUBDRIVER subdriver;

    /* initialize mem screen driver*/
    initmemgc(mempsd, w, h, planes, bpp, linelen, size, addr);

    subdriver = &vnc_subdriver;

    /* set and initialize subdriver into mem screen driver*/
    if(!set_subdriver(mempsd, subdriver, TRUE))
    {
        diag_printf("set_subdriver() failed\n");
        return 0;
    }

    return 1;
}
