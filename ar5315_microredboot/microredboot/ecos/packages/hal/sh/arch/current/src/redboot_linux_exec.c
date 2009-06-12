//==========================================================================
//
//      redboot_linux_exec.c
//
//      RedBoot exec command for Linux booting
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2004 Gary Thomas
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
// Author(s):    t@keshi.org
// Contributors: t@keshi.org, jskov
// Date:         2001-06-19
// Purpose:      RedBoot exec command for Linux booting
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <redboot.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

#define xstr(s) str(s)
#define str(s...) #s

struct parmblock {
    cyg_uint32 mount_rdonly;
    cyg_uint32 ramdisk_flags;
    cyg_uint32 orig_root_dev;
    cyg_uint32 loader_type;
    cyg_uint32 initrd_start;
    cyg_uint32 initrd_size;
};

static void do_exec(int argc, char *argv[]);
RedBoot_cmd("exec", 
            "Execute an image", 
            "[-b <parameter block addr>] [-m <mount rdonly flags>]\n"
            "        [-f <ramdisk flags>] [-r <root device>] [-l <loader type>]\n"
            "        [-i <initrd start addr>] [-j <initrd size>] [-c \"kernel command line\"]\n"
	    "        [<entry point>]",
            do_exec
    );

static void 
do_exec(int argc, char *argv[])
{
    cyg_uint32 entry = CYGDAT_REDBOOT_SH_LINUX_BOOT_ENTRY;
    cyg_uint32 base_addr = CYGDAT_REDBOOT_SH_LINUX_BOOT_BASE_ADDR;
    cyg_uint32 mount_rdonly = CYGDAT_REDBOOT_SH_LINUX_BOOT_MOUNT_RDONLY;
    cyg_uint32 ramdisk_flags = CYGDAT_REDBOOT_SH_LINUX_BOOT_RAMDISK_FLAGS;
    cyg_uint32 orig_root_dev = CYGDAT_REDBOOT_SH_LINUX_BOOT_ORIG_ROOT_DEV;
    cyg_uint32 loader_type = CYGDAT_REDBOOT_SH_LINUX_BOOT_LOADER_TYPE;
    cyg_uint32 initrd_start = CYGDAT_REDBOOT_SH_LINUX_BOOT_INITRD_START;
    cyg_uint32 initrd_size = CYGDAT_REDBOOT_SH_LINUX_BOOT_INITRD_SIZE;
    char *cmd_line = xstr(CYGDAT_REDBOOT_SH_LINUX_BOOT_COMMAND_LINE);

    bool base_addr_set, mount_rdonly_set, ramdisk_flags_set;
    bool orig_root_dev_set, loader_type_set;
    bool initrd_start_set, initrd_size_set, cmd_line_set;

    struct option_info opts[8];
    struct parmblock *pb;
    char *pcmd;
    int oldints;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base_addr, &base_addr_set, "base address");
    init_opts(&opts[1], 'm', true, OPTION_ARG_TYPE_NUM, 
              (void **)&mount_rdonly, &mount_rdonly_set, "mount_rdonly");
    init_opts(&opts[2], 'f', true, OPTION_ARG_TYPE_NUM, 
              (void **)&ramdisk_flags, &ramdisk_flags_set, "ramdisk_flags");
    init_opts(&opts[3], 'r', true, OPTION_ARG_TYPE_NUM, 
              (void **)&orig_root_dev, &orig_root_dev_set, "orig_root_dev");
    init_opts(&opts[4], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&loader_type, &loader_type_set, "loader_type");
    init_opts(&opts[5], 'i', true, OPTION_ARG_TYPE_NUM, 
              (void **)&initrd_start, &initrd_start_set, "initrd_start");
    init_opts(&opts[6], 'j', true, OPTION_ARG_TYPE_NUM, 
              (void **)&initrd_size, &initrd_size_set, "initrd_size");
    init_opts(&opts[7], 'c', true, OPTION_ARG_TYPE_STR, 
              (void **)&cmd_line, &cmd_line_set, "kernel command line");
    
    if (!scan_opts(argc, argv, 1, opts, 8, (void *)&entry, 
                   OPTION_ARG_TYPE_NUM, "entry address"))
        return;
    if (entry == (unsigned long)NO_MEMORY) {
        diag_printf("Can't execute Linux - invalid entry address\n");
        return;
    }
  
    diag_printf("Now booting linux kernel:\n");
    diag_printf(" Base address 0x%08x Entry 0x%08x\n", base_addr, entry);
    diag_printf(" Cmdline : %s\n", cmd_line);

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

    pb = (struct parmblock *)base_addr;
    if (mount_rdonly_set) {
        pb->mount_rdonly = mount_rdonly;
        diag_printf(" MOUNT_RDONLY  : 0x%08x\n", mount_rdonly);
    }
    if (ramdisk_flags_set) {
        pb->ramdisk_flags = ramdisk_flags;
        diag_printf(" RAMDISK_FLAGS : 0x%08x\n", ramdisk_flags);
    }
    if (orig_root_dev_set) {
        pb->orig_root_dev = orig_root_dev;
        diag_printf(" ORIG_ROOT_DEV : 0x%08x\n", orig_root_dev);
    }
    if (loader_type_set) {
        pb->loader_type = loader_type;
        diag_printf(" LOADER_TYPE   : 0x%08x\n", loader_type);
    }
    if (initrd_start_set) {
        pb->initrd_start = initrd_start;
        pb->initrd_size = initrd_size;
        diag_printf(" INITRD_START  : 0x%08x\n", initrd_start);
        diag_printf(" INITRD_SIZE   : 0x%08x\n", initrd_size);
    }

    pcmd = (char *)(base_addr + 0x100);
    while ((*pcmd++ = *cmd_line++));

    asm ("jmp @%0\n\tnop" : : "r" (entry));
}
