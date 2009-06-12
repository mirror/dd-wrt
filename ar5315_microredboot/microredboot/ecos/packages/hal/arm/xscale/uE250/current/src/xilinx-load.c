//==========================================================================
//
//      xilinx-load.c
//
//      FPGA support for NMI uEngine uE250 PCI
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Author(s):    David Mazur <david@mind.be>
// Contributors: gthomas
// Date:         2003-02-20
// Purpose:      FPGA support
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diagnostic printing

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#include <cyg/hal/plx.h>

#define FPGA_PROG 0x00020000
#define FPGA_INIT 0x00000002
#define FPGA_DONE 0x00080000

#define _FPGA_PROG_BASE 0x0c000000
#define FPGA_PROG_BASE (*((volatile cyg_uint32 *)(_FPGA_PROG_BASE)))

#define FPGA_DONE_DRV   0x8
#define FPGA_INIT_DRV   0x10
#define FPGA_WRITE      0x20

#define VGA_PROG_CTRL  0x4008
#define VGA_PROG_DATA  0x400C

#define VGA_DONE       0x1
#define VGA_INIT       0x2
#define VGA_PROG       0x4
#define VGA_DONE_DRV   0x8
#define VGA_INIT_DRV   0x10
#define VGA_WRITE      0x20


#include <cyg/compress/zlib.h>

extern char _end;

static z_stream stream;

#define FEEDBACK_COUNT 16
#define ZCHAR_BUF_SIZE 256
struct _zchar_info {
    char  buf[ZCHAR_BUF_SIZE];
    char *ptr;
    int   avail;
    int   feedback;
    int   total;
};

// Internal allocator for decompression - just use bottom of heap
// which will be reclaimed by eCos once the system is initialized
static void *
_zcalloc(void *opaque, unsigned int items, unsigned int size)
{
    static char *ptr = (char *)&_end;
    char *res = ptr;

//    diag_printf("%s(%p,%d,%d) = %p\n", __FUNCTION__, opaque, items, size, res);
    ptr += (size*items);
    return res;
}

static void 
_zcfree(void *opaque, void *ptr)
{
//    diag_printf("%s(%p,%p)\n", __FUNCTION__, opaque, ptr);    
}

static int
_zchar(void)
{
    int err;
    struct _zchar_info *info = (struct _zchar_info *)stream.opaque;
    static char spin[] = "|/-\\|-";
    static int tick = 0;

    if (info->avail == 0) {
        stream.next_out = info->buf;
        stream.avail_out = sizeof(info->buf);
        info->ptr = info->buf;
        err = inflate(&stream, Z_SYNC_FLUSH);
        info->avail = (char *)stream.next_out - info->buf;
        if (--info->feedback == 0) {
            diag_printf("%c\b", spin[tick++]);
            if (tick >= (sizeof(spin)-1)) {
                tick = 0;
            }
            info->feedback = FEEDBACK_COUNT;
        }
    }
    if (info->avail) {
        info->avail--;
        info->total++;
        return *(info->ptr)++;
    } else {
        // End of data stream
        return -1;
    }
}

/**
 * A little bit swapping function, necessary due to the xilinx bit file format.
 */
static const cyg_uint8 _swapped[] = {
    0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E,
    0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F
};

static cyg_uint8 
bitswap(cyg_uint8 byte)
{
    cyg_uint8 _new = (_swapped[byte & 0x0F] << 4) | (_swapped[(byte >> 4) & 0x0F]);
    return _new;
}

typedef int _bitfile_fun(void);
typedef void _download_fun(_bitfile_fun *_bitfile);

/**
 * Gets the tag at given location in the bitfile.
 */
static cyg_uint8 
bitfile_get_tag(_bitfile_fun *_bitfile)
{
    return (*_bitfile)();
}

static cyg_uint16 
bitfile_get_len16(_bitfile_fun *_bitfile)
{
    cyg_uint16 length;

    length = (*_bitfile)() << 8;
    length |= (*_bitfile)();

    return length;
}

static int 
bitfile_get_len32(_bitfile_fun *_bitfile)
{
    cyg_uint32 length;

    length = (*_bitfile)() << 24;
    length |= (*_bitfile)() << 16;
    length |= (*_bitfile)() << 8;
    length |= (*_bitfile)();

    return length;
}

/**
 * Process a string tag.
 */
static void 
bitfile_process_string_tag(char *description, _bitfile_fun *_bitfile)
{
    int len,i;

    len = bitfile_get_len16(_bitfile);
    diag_printf(description);
    for (i = 0; i < len; i++) {
        diag_printf("%c", (*_bitfile)());
    }
}

/**
 * Process the 'e' tag in the bit file, which is the actual code that is to
 * be programmed on the fpga.
 */
static void 
bitfile_process_tag_e(_bitfile_fun *_bitfile)
{
    int len,count,i;
    cyg_uint8 byte;
    cyg_uint32 word;

    len = bitfile_get_len32(_bitfile);

    *PXA2X0_GPCR0 = FPGA_PROG;

    for (count=0; count<10000; count++)
        if ((*PXA2X0_GPLR0 & FPGA_INIT) == 0)
            break;
    if ((*PXA2X0_GPLR0 & FPGA_INIT) != 0)
        diag_printf("INIT did not go low. FPGA programming failed\n");

    *PXA2X0_GPSR0 = FPGA_PROG;

    for (count=0; count<10000; count++)
        if ((*PXA2X0_GPLR0 & FPGA_INIT) != 0)
            break;
    if ((*PXA2X0_GPLR0 & FPGA_INIT) == 0)
        diag_printf("INIT did not go high. FPGA programming failed\n");

    for( i=0; i<len; i++) {
        if ((*PXA2X0_GPLR0 & FPGA_INIT) == 0) {
            diag_printf("CRC Error. FPGA programming failed\n");
        }

        byte = (*_bitfile)();
        word = 0;

        if (byte & (0x01 << 7)) word|=(0x01);
        if (byte & (0x01 << 6)) word|=(0x01 << 18);
        if (byte & (0x01 << 5)) word|=(0x01 << 14);
        if (byte & (0x01 << 4)) word|=(0x01 << 1);
        if (byte & (0x01 << 3)) word|=(0x01 << 4);
        if (byte & (0x01 << 2)) word|=(0x01 << 6);
        if (byte & (0x01 << 1)) word|=(0x01 << 9);
        if (byte & (0x01)) word|=(0x01 << 30);

        FPGA_PROG_BASE = word;
    }

    for (count=0; count<10000; count++)
        if ((*PXA2X0_GPLR0 & FPGA_DONE) != 0)
            break;
    if ((*PXA2X0_GPLR0 & FPGA_DONE) == 0)
        diag_printf("DONE did not go high. FPGA programming failed\n");

}

/**
 * Process the 'e' tag in the bit file, which is the actual code that is to
 * be programmed on the fpga.
 */
static void 
vga_bitfile_process_tag_e(_bitfile_fun *_bitfile)
{
    int len,count,i;
    cyg_uint8 byte;

    len = bitfile_get_len32(_bitfile);

    localbus_writeb(VGA_WRITE,  VGA_PROG_CTRL);
    localbus_writeb(VGA_WRITE | VGA_PROG,  VGA_PROG_CTRL);

    for (count=0; count<10000; count++)
        if (localbus_readb(VGA_PROG_CTRL) & VGA_INIT)
            break;
    if (!(localbus_readb(VGA_PROG_CTRL) & VGA_INIT))
        diag_printf("INIT did not go high. VGA FPGA programming failed\n");

    localbus_writeb(VGA_PROG, VGA_PROG_CTRL);

    for (i=0; i<len; i++) {
        byte = (*_bitfile)();
        localbus_writeb(bitswap(byte),VGA_PROG_DATA);
    } 

    for (count=0; count<10000; count++)
        if (localbus_readb(VGA_PROG_CTRL) & VGA_DONE)
            break;
    if (!(localbus_readb(VGA_PROG_CTRL) & VGA_DONE))
        diag_printf("DONE did not go high. VGA FPGA programming failed\n");

    localbus_writeb(VGA_PROG | VGA_WRITE,  VGA_PROG_CTRL);
}

//
// Download a bitstream
//
static void
download_bitstream(char *title, _bitfile_fun *_bitfile, _download_fun *_download)
{
    int len, tag;

    diag_printf("Load %s(", title);

    len = bitfile_get_len16(_bitfile);
    while (len-- > 0) {
        (*_bitfile)();  // Skip
    }
    len = bitfile_get_len16(_bitfile);

    tag = 0;
    while (tag != 'e') {

        tag = bitfile_get_tag(_bitfile);
        switch (tag) {
        case 'a':
            bitfile_process_string_tag("Design:", _bitfile);
            break;

        case 'b':
            bitfile_process_string_tag(", Part:", _bitfile);
            break;

        case 'c':
            bitfile_process_string_tag(", Date:", _bitfile);
            break;

        case 'd':
            bitfile_process_string_tag(" ", _bitfile);
            break;

        case 'e':
            (*_download)(_bitfile);
            break;

        default:
            diag_printf("Unknown tag. aborting...\n");
            return;
        }
    }
}


/**
 * Process a bitfile located at the given address.
 */
void 
load_fpga(cyg_uint8 *compressed_bitfile, int len) 
{
    int err;
    struct _zchar_info zchar_data;

    stream.zalloc = _zcalloc;
    stream.zfree = _zcfree;
    stream.next_in = compressed_bitfile;
    stream.avail_in = len;
    stream.next_out = 0;
    stream.avail_out = 0;
    stream.opaque = (void *)&zchar_data;
    zchar_data.avail = 0;
    zchar_data.feedback = FEEDBACK_COUNT;
    zchar_data.total = 0;
    err = inflateInit(&stream);
    if (err) {
        diag_printf("%s: Can't init stream\n", __FUNCTION__);
        return;
    }
    // Set up to download FPGA bitstreap
    *PXA2X0_GPSR0 = FPGA_PROG;
    download_bitstream("PCI ctlr", _zchar, bitfile_process_tag_e);
    inflateEnd(&stream);
    diag_printf(") %x bytes\n", zchar_data.total);
}

/**
 * Process a bitfile located at the given address.
 */
void 
load_vga(cyg_uint8 *compressed_bitfile, int len)
{
    int err;
    struct _zchar_info zchar_data;

    stream.zalloc = _zcalloc;
    stream.zfree = _zcfree;
    stream.next_in = compressed_bitfile;
    stream.avail_in = len;
    stream.next_out = 0;
    stream.avail_out = 0;
    stream.opaque = (void *)&zchar_data;
    zchar_data.avail = 0;
    zchar_data.feedback = FEEDBACK_COUNT;
    zchar_data.total = 0;
    err = inflateInit(&stream);
    if (err) {
        diag_printf("%s: Can't init stream\n", __FUNCTION__);
        return;
    }
    download_bitstream("VGA ctlr", _zchar, vga_bitfile_process_tag_e);
    inflateEnd(&stream);
    diag_printf(") %x bytes\n", zchar_data.total);
}

