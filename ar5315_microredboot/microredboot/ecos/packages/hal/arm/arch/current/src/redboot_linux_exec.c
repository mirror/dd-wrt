//==========================================================================
//
//      redboot_linux_boot.c
//
//      RedBoot command to boot Linux on ARM platforms
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
// Copyright (C) 2003, 2004 Gary Thomas
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
//####OTHERCOPYRIGHTBEGIN####
//
//  The structure definitions below are taken from include/asm-arm/setup.h in
//  the Linux kernel, Copyright (C) 1997-1999 Russell King. Their presence
//  here is for the express purpose of communication with the Linux kernel
//  being booted and is considered 'fair use' by the original author and
//  are included with his permission.
//
//####OTHERCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, jskov,
//               Russell King <rmk@arm.linux.org.uk>
// Date:         2001-02-20
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <redboot.h>

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/hal_io.h>

#ifndef CYGARC_PHYSICAL_ADDRESS
# error
# define CYGARC_PHYSICAL_ADDRESS(x) (x)
#endif

// FIXME: This should be a CDL variable, and CYGSEM_REDBOOT_ARM_LINUX_BOOT
//        active_if  CYGHWR_HAL_ARM_REDBOOT_MACHINE_TYPE>0
#ifdef HAL_PLATFORM_MACHINE_TYPE
#define CYGHWR_REDBOOT_ARM_MACHINE_TYPE HAL_PLATFORM_MACHINE_TYPE

// Exported CLI function(s)
static void do_exec(int argc, char *argv[]);
RedBoot_cmd("exec", 
            "Execute an image - with MMU off", 
            "[-w timeout] [-b <load addr> [-l <length>]]\n"
            "        [-r <ramdisk addr> [-s <ramdisk length>]]\n"
            "        [-c \"kernel command line\"] [<entry_point>]",
            do_exec
    );

// CYGARC_HAL_MMU_OFF inserts code to turn off MMU and jump to a physical
// address. Some ARM implementations may need special handling and define
// their own version.
#ifndef CYGARC_HAL_MMU_OFF

#define __CYGARC_GET_CTLREG \
              "   mrc p15,0,r0,c1,c0,0\n"

#define __CYGARC_CLR_MMU_BITS         \
  "   bic r0,r0,#0xd\n"               \
  "   bic r0,r0,#0x1000\n"            \

#ifdef CYGHWR_HAL_ARM_BIGENDIAN
#define __CYGARC_CLR_MMU_BITS_X       \
  "   bic r0,r0,#0x8d\n"              \
  "   bic r0,r0,#0x1000\n"
#else
#define __CYGARC_CLR_MMU_BITS_X       \
  "   bic r0,r0,#0xd\n"               \
  "   bic r0,r0,#0x1000\n"            \
  "   orr r0,r0,#0x80\n"
#endif

#define __CYGARC_SET_CTLREG(__paddr__) \
  "   mcr p15,0,r0,c1,c0,0\n"          \
  "   mov pc," #__paddr__ "\n"

#define CYGARC_HAL_MMU_OFF(__paddr__)  \
  "   mcr p15,0,r0,c7,c10,4\n"         \
  "   mcr p15,0,r0,c7,c7,0\n"          \
  __CYGARC_GET_CTLREG                  \  
  __CYGARC_CLR_MMU_BITS                \
  __CYGARC_SET_CTLREG(__paddr__)

#define CYGARC_HAL_MMU_OFF_X(__paddr__)  \
  "   mcr p15,0,r0,c7,c10,4\n"           \
  "   mcr p15,0,r0,c7,c7,0\n"            \
  __CYGARC_GET_CTLREG                    \  
  __CYGARC_CLR_MMU_BITS_X                \
  __CYGARC_SET_CTLREG(__paddr__)

#endif  // CYGARC_HAL_MMU_OFF

//
// Parameter info for Linux kernel
//   ** C A U T I O N **  This setup must match "asm-arm/setup.h"
//
// Info is passed at a fixed location, using a sequence of tagged
// data entries.
//

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

//=========================================================================
//  From Linux <asm-arm/setup.h>

#define ATAG_NONE	0x00000000
struct tag_header {
    u32 size;    // Size of tag (hdr+data) in *longwords*
    u32 tag;
};

#define ATAG_CORE	0x54410001
struct tag_core {
    u32 flags;		/* bit 0 = read-only */
    u32 pagesize;
    u32 rootdev;
};

#define ATAG_MEM		0x54410002
struct tag_mem32 {
    u32	size;
    u32	start;
};

#define ATAG_VIDEOTEXT	0x54410003
struct tag_videotext {
    u8	 x;
    u8	 y;
    u16	 video_page;
    u8	 video_mode;
    u8	 video_cols;
    u16	 video_ega_bx;
    u8	 video_lines;
    u8	 video_isvga;
    u16	 video_points;
};

#define ATAG_RAMDISK	0x54410004
struct tag_ramdisk {
    u32 flags;		/* b0 = load, b1 = prompt */
    u32 size;
    u32 start;
};

/*
 * this one accidentally used virtual addresses - as such,
 * its deprecated.
 */
#define ATAG_INITRD	0x54410005

/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2	0x54420005
struct tag_initrd {
    u32 start;
    u32 size;
};

#define ATAG_SERIAL	0x54410006
struct tag_serialnr {
    u32 low;
    u32 high;
};

#define ATAG_REVISION	0x54410007
struct tag_revision {
    u32 rev;
};

#define ATAG_VIDEOLFB	0x54410008
struct tag_videolfb {
    u16	lfb_width;
    u16	lfb_height;
    u16	lfb_depth;
    u16	lfb_linelength;
    u32	lfb_base;
    u32	lfb_size;
    u8	red_size;
    u8	red_pos;
    u8	green_size;
    u8	green_pos;
    u8	blue_size;
    u8	blue_pos;
    u8	rsvd_size;
    u8	rsvd_pos;
};

#define ATAG_CMDLINE	0x54410009
struct tag_cmdline {
    char cmdline[1];
};

#define ATAG_ACORN	0x41000101
struct tag_acorn {
    u32 memc_control_reg;
    u32 vram_pages;
    u8 sounddefault;
    u8 adfsdrives;
};

#define ATAG_MEMCLK	0x41000402
struct tag_memclk {
    u32 fmemclk;
};

struct tag {
    struct tag_header hdr;
    union {
        struct tag_core		core;
        struct tag_mem32	mem;
        struct tag_videotext	videotext;
        struct tag_ramdisk	ramdisk;
        struct tag_initrd	initrd;
        struct tag_serialnr	serialnr;
        struct tag_revision	revision;
        struct tag_videolfb	videolfb;
        struct tag_cmdline	cmdline;

        /*
         * Acorn specific
         */
        struct tag_acorn	acorn;

        /*
         * DC21285 specific
         */
        struct tag_memclk	memclk;
    } u;
};

// End of inclusion from <asm-arm/setup.h>
//=========================================================================

// Default memory layout - can be overridden by platform, typically in
// <cyg/hal/plf_io.h>
#ifndef CYGHWR_REDBOOT_LINUX_ATAG_MEM
#define CYGHWR_REDBOOT_LINUX_ATAG_MEM(_p_)                                                      \
    CYG_MACRO_START                                                                             \
    /* Next ATAG_MEM. */                                                                        \
    _p_->hdr.size = (sizeof(struct tag_mem32) + sizeof(struct tag_header))/sizeof(long);        \
    _p_->hdr.tag = ATAG_MEM;                                                                    \
    /* Round up so there's only one bit set in the memory size.                                 \
     * Don't double it if it's already a power of two, though.                                  \
     */                                                                                         \
    _p_->u.mem.size  = 1<<hal_msbindex(CYGMEM_REGION_ram_SIZE);                                 \
    if (_p_->u.mem.size < CYGMEM_REGION_ram_SIZE)                                               \
	    _p_->u.mem.size <<= 1;                                                              \
    _p_->u.mem.start = CYGARC_PHYSICAL_ADDRESS(CYGMEM_REGION_ram);                              \
    CYG_MACRO_END
#endif


// Round up a quantity to a longword (32 bit) length
#define ROUNDUP(n) (((n)+3)&~3)

static void 
do_exec(int argc, char *argv[])
{
    unsigned long entry;
    unsigned long oldints;
    bool wait_time_set;
    int  wait_time, res, num_opts;
    bool base_addr_set, length_set, cmd_line_set;
    bool ramdisk_addr_set, ramdisk_size_set;
    unsigned long base_addr, length;
    unsigned long ramdisk_addr, ramdisk_size;
    struct option_info opts[7];
    char line[8];
    char *cmd_line;
    struct tag *params = (struct tag *)CYGHWR_REDBOOT_ARM_LINUX_TAGS_ADDRESS;
#ifdef CYGHWR_REDBOOT_LINUX_EXEC_X_SWITCH
    bool swap_endian;
    extern char __xtramp_start__[], __xtramp_end__[];
#endif
    extern char __tramp_start__[], __tramp_end__[];

    // Check to see if a valid image has been loaded
    if (entry_address == (unsigned long)NO_MEMORY) {
        diag_printf("Can't execute Linux - invalid entry address\n");
        return;
    }
    // Default physical entry point for Linux is kernel base.
    entry = (unsigned long)CYGHWR_REDBOOT_ARM_LINUX_EXEC_ADDRESS;

    base_addr = load_address;
    length = load_address_end - load_address;
    // Round length up to the next quad word
    length = (length + 3) & ~0x3;

    ramdisk_size = 4096*1024;
    init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void **)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base_addr, (bool *)&base_addr_set, "base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[3], 'c', true, OPTION_ARG_TYPE_STR, 
              (void **)&cmd_line, (bool *)&cmd_line_set, "kernel command line");
    init_opts(&opts[4], 'r', true, OPTION_ARG_TYPE_NUM, 
              (void **)&ramdisk_addr, (bool *)&ramdisk_addr_set, "ramdisk_addr");
    init_opts(&opts[5], 's', true, OPTION_ARG_TYPE_NUM, 
              (void **)&ramdisk_size, (bool *)&ramdisk_size_set, "ramdisk_size");
    num_opts = 6;
#ifdef CYGHWR_REDBOOT_LINUX_EXEC_X_SWITCH
    init_opts(&opts[6], 'x', false, OPTION_ARG_TYPE_FLG, 
              (void **)&swap_endian, 0, "swap endianess");
    ++num_opts;
#endif
    if (!scan_opts(argc, argv, 1, opts, num_opts, (void *)&entry, OPTION_ARG_TYPE_NUM, "[physical] starting address"))
    {
        return;
    }

    // Set up parameters to pass to kernel

    // CORE tag must be present & first
    params->hdr.size = (sizeof(struct tag_core) + sizeof(struct tag_header))/sizeof(long);
    params->hdr.tag = ATAG_CORE;
    params->u.core.flags = 0;
    params->u.core.pagesize = 0;
    params->u.core.rootdev = 0;
    params = (struct tag *)((long *)params + params->hdr.size);

    // Fill in the details of the memory layout
    CYGHWR_REDBOOT_LINUX_ATAG_MEM(params);

    params = (struct tag *)((long *)params + params->hdr.size);
    if (ramdisk_addr_set) {
        params->hdr.size = (sizeof(struct tag_initrd) + sizeof(struct tag_header))/sizeof(long);
        params->hdr.tag = ATAG_INITRD2;
        params->u.initrd.start = CYGARC_PHYSICAL_ADDRESS(ramdisk_addr);
        params->u.initrd.size = ramdisk_size;
        params = (struct tag *)((long *)params + params->hdr.size);
    }
    if (cmd_line_set) {
        params->hdr.size = (ROUNDUP(strlen(cmd_line)) + sizeof(struct tag_header))/sizeof(long);
        params->hdr.tag = ATAG_CMDLINE;
        strcpy(params->u.cmdline.cmdline, cmd_line);
        params = (struct tag *)((long *)params + params->hdr.size);
    }
    // Mark end of parameter list
    params->hdr.size = 0;
    params->hdr.tag = ATAG_NONE;

    if (wait_time_set) {
        int script_timeout_ms = wait_time * 1000;
#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
        unsigned char *hold_script = script;
        script = (unsigned char *)0;
#endif
        diag_printf("About to start execution at %p - abort with ^C within %d seconds\n",
                    (void *)entry, wait_time);
        while (script_timeout_ms >= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT) {
            res = _rb_gets(line, sizeof(line), CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT);
            if (res == _GETS_CTRLC) {
#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
                script = hold_script;  // Re-enable script
#endif
                return;
            }
            script_timeout_ms -= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT;
        }
    }
    if (!base_addr_set) {
        if ((base_addr == 0) || (length == 0)) {
            // Probably not valid - don't try it
            diag_printf("Base address unknown - use \"-b\" option\n");
            return;
        }
        diag_printf("Using base address %p and length %p\n",
                    (void*)base_addr, (void*)length);
    } else if (base_addr_set && !length_set) {
        diag_printf("Length required for non-standard base address\n");
        return;
    }

#ifdef CYGPKG_IO_ETH_DRIVERS
    eth_drv_stop();
#endif

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();

    // Tricky code. We are currently running with the MMU on and the
    // memory map possibly convoluted from 1-1.  The trampoline code
    // between labels __tramp_start__ and __tramp_end__ must be copied
    // to RAM and then executed at the non-mapped address.
    // 
    // This magic was created in order to be able to execute standard
    // Linux kernels with as little change/perturberance as possible.

#ifdef CYGHWR_REDBOOT_LINUX_EXEC_X_SWITCH
    if (swap_endian) {
	// copy the trampline code
	memcpy((char *)CYGHWR_REDBOOT_ARM_TRAMPOLINE_ADDRESS,
	       __xtramp_start__,
	       __xtramp_end__ - __xtramp_start__);

	asm volatile (
	    CYGARC_HAL_MMU_OFF_X(%5)
	    "__xtramp_start__:\n"
	    " cmp %1,%4;\n"       // Default kernel load address. Relocate
	    " beq 2f;\n"          // kernel image there if necessary, and
	    " cmp %2,#0;\n"       // if size is non-zero
	    " beq 2f;\n"
	    "1:\n"
	    " ldr r0,[%1],#4;\n"
	    " eor %5, r0, r0, ror #16;\n"
	    " bic %5, %5, #0x00ff0000;\n"
	    " mov r0, r0, ror #8;\n"
	    " eor r0, r0, %5, lsr #8;\n"
	    " str r0,[%4],#4;\n"
	    " subs %2,%2,#4;\n"
	    " bne 1b;\n"
	    "2:\n"
	    " mov r0,#0;\n"       // Set board type
	    " mov r1,%3;\n"       // Machine type
	    " mov r2,%6;\n"       // Kernel parameters
	    " mov pc,%0;\n"       // Jump to kernel
	    "__xtramp_end__:\n"
	    : : 
	    "r"(entry),
	    "r"(CYGARC_PHYSICAL_ADDRESS(base_addr)),
	    "r"(length),
	    "r"(CYGHWR_REDBOOT_ARM_MACHINE_TYPE),
	    "r"(CYGHWR_REDBOOT_ARM_LINUX_EXEC_ADDRESS),
	    "r"(CYGARC_PHYSICAL_ADDRESS(CYGHWR_REDBOOT_ARM_TRAMPOLINE_ADDRESS)),
	    "r"(CYGARC_PHYSICAL_ADDRESS(CYGHWR_REDBOOT_ARM_LINUX_TAGS_ADDRESS))
	    : "r0", "r1"
	    );
    }
#endif // CYGHWR_REDBOOT_LINUX_EXEC_X_SWITCH

    // copy the trampline code
    memcpy((char *)CYGHWR_REDBOOT_ARM_TRAMPOLINE_ADDRESS,
	   __tramp_start__,
	   __tramp_end__ - __tramp_start__);

    asm volatile (
        CYGARC_HAL_MMU_OFF(%5)
        "__tramp_start__:\n"
        " cmp %1,%4;\n"       // Default kernel load address. Relocate
        " beq 2f;\n"          // kernel image there if necessary, and
        " cmp %2,#0;\n"       // if size is non-zero
        " beq 2f;\n"
        "1:\n"
        " ldr r0,[%1],#4;\n"
        " str r0,[%4],#4;\n"
        " subs %2,%2,#4;\n"
        " bne 1b;\n"
        "2:\n"
        " mov r0,#0;\n"       // Set board type
        " mov r1,%3;\n"       // Machine type
        " mov r2,%6;\n"       // Kernel parameters
        " mov pc,%0;\n"       // Jump to kernel
        "__tramp_end__:\n"
        : : 
        "r"(entry),
        "r"(CYGARC_PHYSICAL_ADDRESS(base_addr)),
        "r"(length),
        "r"(CYGHWR_REDBOOT_ARM_MACHINE_TYPE),
        "r"(CYGHWR_REDBOOT_ARM_LINUX_EXEC_ADDRESS),
        "r"(CYGARC_PHYSICAL_ADDRESS(CYGHWR_REDBOOT_ARM_TRAMPOLINE_ADDRESS)),
        "r"(CYGARC_PHYSICAL_ADDRESS(CYGHWR_REDBOOT_ARM_LINUX_TAGS_ADDRESS))
        : "r0", "r1"
        );
}
      
#endif // HAL_PLATFORM_MACHINE_TYPE - otherwise we do not support this stuff...

// EOF redboot_linux_exec.c
