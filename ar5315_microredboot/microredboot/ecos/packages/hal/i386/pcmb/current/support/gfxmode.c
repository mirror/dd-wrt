//==========================================================================
//
//      gfxmode.c
//
//      Display information about the available graphics modes.
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
// Author(s):    bartv
// Date:         2002-04-26
// Purpose:      Display information about the various graphics modes
// Description:
//
//    RedBoot on a PC has an option to switch to a suitable graphics mode
//    during bootstrap, by calling the video BIOS before switching from
//    real to protected mode. This is controlled by the option
//    CYGNUM_HAL_I386_PC_STARTUP_VIDEO_MODE in the RedBoot configuration.
//    It is then possible to download an eCos application which uses
//    the current graphics mode.
//
//    However different graphics cards will use different numbers for
//    the various resolutions, so the user must somehow find out which
//    graphics mode corresponds to which resolution and depth. That
//    can be achieved by creating another eCos configuration suitable
//    for applications, and then building this utility with that
//    configuration. RedBoot can then be reconfigured and rebuilt
//    to use the right video mode.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <stdio.h>
#include <stdlib.h>

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
    unsigned int    physical_base_ptr;
    unsigned int    offscreen_memory_offset;
    unsigned short  offscreen_memory_size;
} __attribute__((packed));

static char*
segoff_to_phys(char* addr)
{
    int x       = (int) addr;
    int segment = (x >> 12) & 0x0FFFF0;
    int offset  = x & 0x0FFFF;
    return (char*) (segment | offset);
}

int
main(int argc, char** argv)
{
    struct VBESVGAInfoBlock*    info_block = (struct VBESVGAInfoBlock*) 0x000A0000;
    struct VBEModeInfoBlock*    current_mode = (struct VBEModeInfoBlock*) 0x000A0200;
    int i;
    
    printf("VESA info: %c%c%c%c\n", info_block->signature[0], info_block->signature[1],
           info_block->signature[2], info_block->signature[3]);
    printf("version %04x\n", info_block->version);
    if (NULL != info_block->oem_string_ptr) {
        info_block->oem_string_ptr = segoff_to_phys(info_block->oem_string_ptr);
        printf("OEM %s\n", info_block->oem_string_ptr);
    }
    printf("total memory %dK\n", 64 * info_block->total_memory);
    printf("oem software rev %04x\n", info_block->oem_software_rev);
    if (NULL != info_block->oem_vendor_name_ptr) {
        info_block->oem_vendor_name_ptr = segoff_to_phys(info_block->oem_vendor_name_ptr);
        printf("OEM vendor %s\n", info_block->oem_vendor_name_ptr);
    }
    if (NULL != info_block->oem_product_name_ptr) {
        info_block->oem_product_name_ptr = segoff_to_phys(info_block->oem_product_name_ptr);
        printf("OEM product name %s\n", info_block->oem_product_name_ptr);
    }
    if (NULL != info_block->oem_product_rev_ptr) {
        info_block->oem_product_rev_ptr = segoff_to_phys(info_block->oem_product_rev_ptr);
        printf("OEM product revision %s\n", info_block->oem_product_rev_ptr);
    }
    printf("Capabilities: %02x, %02x, %02x, %02x\n",
           info_block->capabilities[0], info_block->capabilities[1],
           info_block->capabilities[2], info_block->capabilities[3]);
    info_block->video_mode_ptr = (unsigned short*) segoff_to_phys((char*) info_block->video_mode_ptr);
    for (i = 0; 0x0FFFF != info_block->video_mode_ptr[i]; i++) {
        int mode = info_block->video_mode_ptr[i];
        struct VBEModeInfoBlock*   mode_data   = (struct VBEModeInfoBlock*) (0x0A0400 + (0x100 * i));
        printf("Mode %04x: %4d*%4d @ %2dbpp, %4d bytes/line, fb %s 0x%08x\n", mode,
               mode_data->x_resolution, mode_data->y_resolution, mode_data->bits_per_pixel,
               mode_data->bytes_per_scanline,
               (0 == (mode_data->mode_attributes & 0x0080)) ? "no " : "yes",
               mode_data->physical_base_ptr);
        if (8 < mode_data->bits_per_pixel) {
            printf("    red %d bits << %d, green %d bits << %d, blue %d bits << %d, reserved %d bits << %d\n",
                   mode_data->red_mask_size, mode_data->red_field_position,
                   mode_data->green_mask_size, mode_data->green_field_position,
                   mode_data->blue_mask_size, mode_data->blue_field_position,
                   mode_data->reserved_mask_size, mode_data->reserved_field_position);
        }
    }

    printf("Current mode: %4d*%4d @ %2dbpp, %4d bytes/line, fb %s 0x%08x\n", 
           current_mode->x_resolution, current_mode->y_resolution, current_mode->bits_per_pixel,
           current_mode->bytes_per_scanline,
           (0 == (current_mode->mode_attributes & 0x0080)) ? "no" : "yes",
           current_mode->physical_base_ptr);
    if (8 < current_mode->bits_per_pixel) {
        printf("    red %d bits << %d, green %d bits << %d, blue %d bits << %d, reserved %d bits << %d\n",
               current_mode->red_mask_size, current_mode->red_field_position,
               current_mode->green_mask_size, current_mode->green_field_position,
               current_mode->blue_mask_size, current_mode->blue_field_position,
               current_mode->reserved_mask_size, current_mode->reserved_field_position);
    }

    exit(0);
}
