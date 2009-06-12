//=============================================================================
//
//      scr_ecospcsvga.c
//
//      eCos support for a PC display using SVGA/VESA
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
// Author(s):    bartv
// Date:         2002-04-19
// Purpose:      Implement a screen driver for PC's with a VESA video BIOS
//
//    PC graphics cards are problematical. Many graphic cards are still more
//    or less compatible with ancient VGA hardware, but this provides access
//    only to rather low resolutions. There is no other hardware compatibility,
//    and many different graphics card all needing their own driver.
//
//    Each graphics card comes with a video BIOS @ 0x000C0000, which can be
//    invoked via int 0x10 to do useful operations like querying the
//    available video modes and setting the desired one. However the video
//    BIOS can normally only be called in x86 real mode, not protected mode,
//    Currently eCos only runs in protected mode, and has no support for
//    briefly switching back into real mode.
//
//    Current VESA VBE2 compliant graphics cards do offer some protected
//    mode entry points, but not enough to perform mode switches etc.
//    VBE3 is supposed to provide additional support for protected mode
//    applications, but does not appear to be widely implemented yet.
//
//    So for now the only solution is to perform the mode switching
//    during bootstrap, and specifically inside RedBoot. This is controlled
//    by an option in the RedBoot configuration, which has the side
//    effect of disabling RedBoot's own use of the screen and keyboard.
//    SVGA graphics modes are not completely standardized, so it is the
//    user's responsibility to pick a suitable mode.
//
//    Because RedBoot is a separate application, it is not guaranteed
//    that the appropriate mode switch has actually occurred by the
//    time this code runs. Therefore RedBoot also places the main SVGA
//    info block at location 0x000A0000, normally a window into video
//    memory, since that memory is not being used for anything else
//    right now. Similarly the mode info block for the current mode
//    is placed @ 0x000A0200, and to make it easier to find out what
//    modes are available on the current hardware and allow RedBoot to
//    be reconfigured appropriately, all the mode info blocks are
//    stored @ 0x000A0400 at 256-byte boundaries. The main info block
//    can be used to find out which modes are actually available.
//
//####DESCRIPTIONEND####
//=============================================================================


#include <pkgconf/system.h>
#include <pkgconf/microwindows.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <microwin/device.h>

#define VTSWITCH 0
#include "fb.h"
#include "genfont.h"
#include "genmem.h"

// ----------------------------------------------------------------------------
// Information about the current mode and all available video modes,
// should have been provided by RedBoot.
struct VBESVGAInfoBlock {
    unsigned char   signature[4];      /* VESA */
    unsigned short  version;
    char*           oem_string_ptr;
    unsigned char   capabilities[4];
    unsigned short* video_mode_ptr;
    unsigned short  total_memory;
    unsigned short  oem_software_rev;
    char*           oem_vendor_name_ptr;
    char*           oem_product_name_ptr;
    char*           oem_product_rev_ptr;
    /* Reserved data here */
} __attribute__((packed));

struct VBEModeInfoBlock {
    unsigned short  mode_attributes;
    unsigned char   win_a_atributes;
    unsigned char   win_b_attributes;
    unsigned short  win_granularity;
    unsigned short  win_size;
    unsigned short  win_a_segment;
    unsigned short  win_b_segment;
    unsigned int    win_func_ptr;
    unsigned short  bytes_per_scanline;
    unsigned short  x_resolution;
    unsigned short  y_resolution;
    unsigned char   x_char_size;
    unsigned char   y_char_size;
    unsigned char   number_of_planes;
    unsigned char   bits_per_pixel;
    unsigned char   number_of_banks;
    unsigned char   memory_model;
    unsigned char   bank_size;
    unsigned char   number_of_image_pages;
    unsigned char   reserved;
    unsigned char   red_mask_size;
    unsigned char   red_field_position;
    unsigned char   green_mask_size;
    unsigned char   green_field_position;
    unsigned char   blue_mask_size;
    unsigned char   blue_field_position;
    unsigned char   reserved_mask_size;
    unsigned char   reserved_field_position;
    unsigned char   direct_color_mode_info;
    void*           physical_base_ptr;
    unsigned int    offscreen_memory_offset;
    unsigned short  offscreen_memory_size;
} __attribute__((packed));

#if defined(CYGDBG_MICROWINDOWS_PCSVGA_VERBOSE)
static void*
segoff_to_phys(void* addr)
{
    int x       = (int) addr;
    int segment = (x >> 12) & 0x0FFFF0;
    int offset  = x & 0x0FFFF;
    return (void*) (segment | offset);
}
#endif

// ----------------------------------------------------------------------------
static PSD  ecos_pcsvga_open(PSD);
static void ecos_pcsvga_close(PSD);
static void ecos_pcsvga_setportrait(PSD, int);
static void ecos_pcsvga_setpalette(PSD, int, int, MWPALENTRY*);
static void ecos_pcsvga_getscreeninfo(PSD , PMWSCREENINFO);

SCREENDEVICE scrdev = {
    xres:               0,
    yres:               0,
    xvirtres:           0,
    yvirtres:           0,
    planes:             1,
    linelen:            0,
    size:               0,
#if defined(CYGIMP_MICROWINDOWS_PCSVGA32)
    bpp:                32,
    ncolors:            1 << 24,
    pixtype:            MWPF_TRUECOLOR0888,
#elif defined(CYGIMP_MICROWINDOWS_PCSVGA16)
    bpp:                16,
    ncolors:            1 << 16,
    pixtype:            MWPF_TRUECOLOR565,
#else
# error Unsupported video mode.    
#endif    
    flags:              PSF_SCREEN | PSF_HAVEBLIT,
    addr:               0,

    Open:               &ecos_pcsvga_open,
    Close:              &ecos_pcsvga_close,
    GetScreenInfo:      &ecos_pcsvga_getscreeninfo,
    SetPalette:         &ecos_pcsvga_setpalette,
    DrawPixel:          NULL,
    ReadPixel:          NULL,
    DrawHorzLine:       NULL,
    DrawVertLine:       NULL,
    FillRect:           NULL,
    builtin_fonts:      gen_fonts,
    Blit:               NULL,
    PreSelect:          NULL,
    DrawArea:           NULL,
    SetIOPermissions:   NULL,
    AllocateMemGC:      &gen_allocatememgc,
    MapMemGC:           &fb_mapmemgc,
    FreeMemGC:          &gen_freememgc,
    StretchBlit:        NULL,
    SetPortrait:        &ecos_pcsvga_setportrait,
    portrait:           MWPORTRAIT_NONE,
    orgsubdriver:       NULL
};

static PSD
ecos_pcsvga_open(PSD psd)
{
    struct VBESVGAInfoBlock*    vesa_info_block;
    struct VBEModeInfoBlock*    vesa_current_mode;
    
    // Detect repeated invocations. The information in the video
    // memory will be blown away after the first call, so can
    // only be consulted once.
    static int  opened  = 0;
    static PSD  result  = NULL;
    if (opened) {
        return result;
    }
    opened = 1;
    
    // First make sure that there is valid video information in video
    // memory, i.e. that the system was booted by means of a suitable
    // RedBoot instance, that nothing has overwritten video memory
    // yet, and that we are running at the desired screen depth.
    vesa_info_block   = (struct VBESVGAInfoBlock*) 0x000A0000;
    vesa_current_mode = (struct VBEModeInfoBlock*) 0x000A0200;
    if (('V' != vesa_info_block->signature[0]) || ('E' != vesa_info_block->signature[1]) ||
        ('S' != vesa_info_block->signature[2]) || ('A' != vesa_info_block->signature[3])) {
        EPRINTF("No Video BIOS information at location 0x000A0000\n"
                "Please use a suitably configured RedBoot for bootstrap\n");
        return NULL;
    }

    // Optionally, provide lots of information about the graphics card,
    // the various modes available, and the current mode.
#ifdef CYGDBG_MICROWINDOWS_PCSVGA_VERBOSE
    {
        diag_printf("VESA info: %c%c%c%c\n", vesa_info_block->signature[0], vesa_info_block->signature[1],
                    vesa_info_block->signature[2], vesa_info_block->signature[3]);
        diag_printf("Version %04x\n", vesa_info_block->version);
        if (NULL != vesa_info_block->oem_string_ptr) {
            vesa_info_block->oem_string_ptr = segoff_to_phys(vesa_info_block->oem_string_ptr);
            diag_printf("OEM %s\n", vesa_info_block->oem_string_ptr);
        }
        diag_printf("Total memory %dK\n", 64 * vesa_info_block->total_memory);
        diag_printf("OEM software rev %04x\n", vesa_info_block->oem_software_rev);
        if (NULL != vesa_info_block->oem_vendor_name_ptr) {
            vesa_info_block->oem_vendor_name_ptr = segoff_to_phys(vesa_info_block->oem_vendor_name_ptr);
            diag_printf("OEM vendor %s\n", vesa_info_block->oem_vendor_name_ptr);
        }
        if (NULL != vesa_info_block->oem_product_name_ptr) {
            vesa_info_block->oem_product_name_ptr = segoff_to_phys(vesa_info_block->oem_product_name_ptr);
            diag_printf("OEM product name %s\n", vesa_info_block->oem_product_name_ptr);
        }
        if (NULL != vesa_info_block->oem_product_rev_ptr) {
            vesa_info_block->oem_product_rev_ptr = segoff_to_phys(vesa_info_block->oem_product_rev_ptr);
            diag_printf("OEM product revision %s\n", vesa_info_block->oem_product_rev_ptr);
        }
        diag_printf("Capabilities: %02x, %02x, %02x, %02x\n",
                    vesa_info_block->capabilities[0], vesa_info_block->capabilities[1],
                    vesa_info_block->capabilities[2], vesa_info_block->capabilities[3]);
        vesa_info_block->video_mode_ptr = (unsigned short*) segoff_to_phys((char*) vesa_info_block->video_mode_ptr);
        for (i = 0; 0x0FFFF != vesa_info_block->video_mode_ptr[i]; i++) {
            int mode = vesa_info_block->video_mode_ptr[i];
            struct VBEModeInfoBlock*   mode_data   = (struct VBEModeInfoBlock*) (0x0A0400 + (0x100 * i));
            diag_printf("Mode %04x: %4d*%4d @ %2dbpp, %4d bytes/line, fb %s 0x%08x\n", mode,
                        mode_data->x_resolution, mode_data->y_resolution, mode_data->bits_per_pixel,
                        mode_data->bytes_per_scanline,
                        (0 == (mode_data->mode_attributes & 0x0080)) ? "no " : "yes",
                        mode_data->physical_base_ptr);
            if (32 == mode_data->bits_per_pixel) {
                diag_printf("    red %d bits << %d, green %d bits << %d, blue %d bits << %d, reserved %d bits << %d\n",
                            mode_data->red_mask_size, mode_data->red_field_position,
                            mode_data->green_mask_size, mode_data->green_field_position,
                            mode_data->blue_mask_size, mode_data->blue_field_position,
                            mode_data->reserved_mask_size, mode_data->reserved_field_position);
            }
        }

        diag_printf("Current mode: %4d*%4d @ %2dbpp, %4d bytes/line, fb %s 0x%08x\n", 
                    vesa_current_mode->x_resolution, vesa_current_mode->y_resolution, vesa_current_mode->bits_per_pixel,
                    vesa_current_mode->bytes_per_scanline,
                    (0 == (vesa_current_mode->mode_attributes & 0x0080)) ? "no" : "yes",
                    vesa_current_mode->physical_base_ptr);
        if (32 == vesa_current_mode->bits_per_pixel) {
            diag_printf("    red %d bits << %d, green %d bits << %d, blue %d bits << %d, reserved %d bits << %d\n",
                        vesa_current_mode->red_mask_size, vesa_current_mode->red_field_position,
                        vesa_current_mode->green_mask_size, vesa_current_mode->green_field_position,
                        vesa_current_mode->blue_mask_size, vesa_current_mode->blue_field_position,
                        vesa_current_mode->reserved_mask_size, vesa_current_mode->reserved_field_position);
        }
    }
#endif

#if defined(CYGIMP_MICROWINDOWS_PCSVGA32)    
    // A 32-bit displays, 0888
    if ((32 != vesa_current_mode->bits_per_pixel) ||
        (8 != vesa_current_mode->red_mask_size)   || (16 != vesa_current_mode->red_field_position) ||
        (8 != vesa_current_mode->green_mask_size) || ( 8 != vesa_current_mode->green_field_position) ||
        (8 != vesa_current_mode->blue_mask_size)  || ( 0 != vesa_current_mode->blue_field_position)) {

        EPRINTF("RedBoot has not set up a valid initial graphics mode.\n"
                "This screen driver requires 32 bits per pixel, 0RGB.\n",
                "Configuration option CYGDBG_MICROWINDOWS_PCSVGA_VERBOSE can be used to\n"
                "get information about the available video modes.\n");
        return NULL;
    }
#elif defined(CYGIMP_MICROWINDOWS_PCSVGA16)
    // A 16-bit display, 565
    if ((16 != vesa_current_mode->bits_per_pixel) ||
        (5 != vesa_current_mode->red_mask_size)   || (11 != vesa_current_mode->red_field_position) ||
        (6 != vesa_current_mode->green_mask_size) || ( 5 != vesa_current_mode->green_field_position) ||
        (5 != vesa_current_mode->blue_mask_size)  || ( 0 != vesa_current_mode->blue_field_position)) {

        EPRINTF("RedBoot has not set up a valid initial graphics mode.\n"
                "This screen driver requires 16 bits per pixel, RGB=565.\n",
                "Configuration option CYGDBG_MICROWINDOWS_PCSVGA_VERBOSE can be used to\n"
                "get information about the available video modes.\n");
        return NULL;
    }
#endif
    
    if ((0 == (vesa_current_mode->mode_attributes & 0x0080)) || (NULL == vesa_current_mode->physical_base_ptr)) {
        EPRINTF("RedBoot has not set up a valid initial graphics mode.\n"
                "The frame buffer is not linearly accessible.\n");
        return NULL;
    }
    

    // OK, we are at the expected depth. Fill in the resolution etc. based
    // on the current video mode.
    psd->xres       = vesa_current_mode->x_resolution;
    psd->yres       = vesa_current_mode->y_resolution;
    psd->xvirtres   = vesa_current_mode->x_resolution;
    psd->yvirtres   = vesa_current_mode->y_resolution;
    psd->linelen    = vesa_current_mode->bytes_per_scanline;
    psd->addr       = vesa_current_mode->physical_base_ptr;
    // The remaining parameters in the structure are statically
    // initialized for now, e.g. 32bpp, pixtype TRUECOLOR0888

    // Use one of the standard framebuffer subdrivers.
    psd->orgsubdriver  = select_fb_subdriver(psd);
    if (NULL == psd->orgsubdriver) {
        EPRINTF("There is no standard framebuffer driver for the current video mode.\n");
        return NULL;
    }
    if (!set_subdriver(psd, psd->orgsubdriver, TRUE)) {
        EPRINTF("Framebuffer subdriver initialization failed.\n");
        return NULL;
    }

    // That should be all for now.
    result = psd;
    return result;
}

// Close is a no-op, no resources have been allocated.
// The open() code detects multiple invocations.
static void
ecos_pcsvga_close(PSD psd)
{
    CYG_UNUSED_PARAM(PSD, psd);
}

// Setting the palette is a no-op for now since only true color
// modes are supported.
static void
ecos_pcsvga_setpalette(PSD psd, int first, int count, MWPALENTRY* pal)
{
    CYG_UNUSED_PARAM(PSD, psd);
    CYG_UNUSED_PARAM(int, first);
    CYG_UNUSED_PARAM(int, count);
    CYG_UNUSED_PARAM(MWPALENTRY*, pal);
}

// Setting the portrait mode. There are standard subdrivers to
// cope with this, manipulating the arguments and then calling
// the original subdriver.

extern SUBDRIVER fbportrait_left, fbportrait_right, fbportrait_down;

static void
ecos_pcsvga_setportrait(PSD psd, int portrait_mode)
{
    psd->portrait   = portrait_mode;
    if (portrait_mode & (MWPORTRAIT_LEFT | MWPORTRAIT_RIGHT)) {
        psd->xvirtres   = psd->yres;
        psd->yvirtres   = psd->xres;
    } else {
        psd->xvirtres   = psd->xres;
        psd->yvirtres   = psd->yres;
    }
    
    if (portrait_mode == MWPORTRAIT_LEFT) {
        set_subdriver(psd, &fbportrait_left, FALSE);
    } else if (portrait_mode == MWPORTRAIT_RIGHT) {
        set_subdriver(psd, &fbportrait_right, FALSE);
    } else if (portrait_mode == MWPORTRAIT_DOWN) {
        set_subdriver(psd, &fbportrait_down, FALSE);
    } else {
        set_subdriver(psd, psd->orgsubdriver, FALSE);
    }
}

// Getting screen info. It is not clear why this is part of the driver
// since nearly all the information is already available in psd.
static void
ecos_pcsvga_getscreeninfo(PSD psd, PMWSCREENINFO psi)
{
    psi->rows       = psd->yvirtres;
    psi->cols       = psd->xvirtres;
    psi->planes     = psd->planes;
    psi->bpp        = psd->bpp;
    psi->ncolors    = psd->ncolors;
    psi->portrait   = psd->portrait;
    psi->fbdriver   = TRUE;
    psi->pixtype    = psd->pixtype;
#if defined(CYGIMP_MICROWINDOWS_PCSVGA32)
    psi->rmask      = 0x00FF0000;
    psi->gmask      = 0x0000FF00;
    psi->bmask      = 0x000000FF;
#elif defined(CYGIMP_MICROWINDOWS_PCSVGA16)
    psi->rmask      = 0x0000F800;
    psi->gmask      = 0x000007E0;
    psi->bmask      = 0x0000001F;
#endif    

    // The screen dimensions are not readily available.
    // Assume an 18' monitor (actual visible diameter),
    // which corresponds ~ to 36cm by 28cm
    psi->xdpcm      = psd->xres / 36;
    psi->ydpcm      = psd->yres / 28;

    psi->fonts      = NUMBER_FONTS;

    // Remaining information such as keyboard modifiers etc. cannot be
    // known here.
}
