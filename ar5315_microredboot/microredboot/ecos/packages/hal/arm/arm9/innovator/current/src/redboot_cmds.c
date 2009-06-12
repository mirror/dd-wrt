#if 0
//==========================================================================
//
//      redboot_cmds.c
//
//      OMAP1510DC EVM [platform] specific RedBoot commands
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
// Author(s):    Patrick Doyle <wpd@delcomsys.com>
// Contributors: Patrick Doyle <wpd@delcomsys.com>
// Date:         2002-11-27
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).  It was modified from "redboot_cmds"
// for the iPaq by wpd in order to add some platform specific commands to
// the OMAP1510DC EVM platform.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/innovator.h>

static void do_mem(int argc, char *argv[]);
static void do_try_reset(int argc, char *argv[]);
static void do_testsdram(int argc, char *argv[]);
static void do_delay(int argc, char *argv[]);

RedBoot_cmd("mem",
	    "Set a memory location",
	    "[-h|-b] [-n] [-a <address>] <data>",
	    do_mem
    );

RedBoot_cmd("try_reset",
	    "try to reset the platform",
	    "[-1|-2|-3|-4]",
	    do_try_reset
    );


RedBoot_cmd("testsdram",
	    "test SDRAM (very simply)",
	    "[-l length]",
	    do_testsdram
    );

RedBoot_cmd("delay",
	    "delay specified number of usecs",
	    "[-c count] amount",
	    do_delay
    );


static void
do_mem(int argc,
       char *argv[])
{
  struct option_info opts[4];
  bool mem_half_word, mem_byte;
  bool no_verify = false;
  static int address = 0x00000000;
  int value;
  init_opts(&opts[0], 'b', false, OPTION_ARG_TYPE_FLG,
            (void**)&mem_byte, 0, "write a byte");
  init_opts(&opts[1], 'h', false, OPTION_ARG_TYPE_FLG,
            (void**)&mem_half_word, 0, "write a half-word");
  init_opts(&opts[2], 'a', true, OPTION_ARG_TYPE_NUM,
            (void**)&address, NULL, "address to write at");
  init_opts(&opts[3], 'n', false, OPTION_ARG_TYPE_FLG,
            (void**)&no_verify, 0, "noverify");

  if (!scan_opts(argc, argv,
                 1,
                 opts, sizeof(opts) / sizeof(opts[0]),
                 (void*)&value, OPTION_ARG_TYPE_NUM, "address to set")) {
    return;
  }

  if ( mem_byte && mem_half_word ) {
    diag_printf("*ERR: Should not specify both byte and half-word access\n");
  } else if ( mem_byte ) {
    *(cyg_uint8*)address = (cyg_uint8)(value & 255);
    if (no_verify) {
      diag_printf("  Set 0x%08X to 0x%02X\n",
                  address, value & 255);
    } else {
            diag_printf("  Set 0x%08X to 0x%02X (result 0x%02X)\n",
                  address, value & 255, (int)*(cyg_uint8*)address );
    }
  } else if ( mem_half_word ) {
    if ( address & 1 ) {
      diag_printf( "*ERR: Badly aligned address 0x%08X for half-word store\n",
                   address );
    } else {
      *(cyg_uint16*)address = (cyg_uint16)(value & 0xffff);
      if (no_verify) {
        diag_printf("  Set 0x%08X to 0x%04X\n",
                    address, value & 0xffff);
      } else {
        diag_printf("  Set 0x%08X to 0x%04X (result 0x%04X)\n",
                    address, value & 0xffff, (int)*(cyg_uint16*)address );
      }
    }
  } else {
    if ( address & 3 ) {
      diag_printf( "*ERR: Badly aligned address 0x%08X for word store\n",
                   address );
    } else {
      *(cyg_uint32*)address = (cyg_uint32)value;
      if (no_verify) {
        diag_printf("  Set 0x%08X to 0x%08X\n",
                    address, value);
      } else {
        diag_printf("  Set 0x%08X to 0x%08X (result 0x%08X)\n",
                    address, value, (int)*(cyg_uint32*)address );
      }
    }
  }
}

static void
do_try_reset(int argc,
             char *argv[])
{
  struct option_info opts[4];
  bool flag_1 = false;
  bool flag_2 = false;
  bool flag_3 = false;
  bool flag_4 = false;

  cyg_uint16 tmp = 0;

  init_opts(&opts[0], '1', false, OPTION_ARG_TYPE_FLG,
            (void**)&flag_1, 0, "only try phase 1");
  init_opts(&opts[1], '2', false, OPTION_ARG_TYPE_FLG,
            (void**)&flag_2, 0, "try phase 1 & phase 2");
  init_opts(&opts[2], '3', false, OPTION_ARG_TYPE_FLG,
            (void**)&flag_3, 0, "try phase 1 through phase 3");
  init_opts(&opts[3], '4', false, OPTION_ARG_TYPE_FLG,
            (void**)&flag_4, 0, "try phase 1 through phase 4");

  if (!scan_opts(argc, argv,
                 1,
                 opts, sizeof(opts) / sizeof(opts[0]),
                 0, 0, 0)) {
    return;
  }
  diag_printf("flag_4 = %d, flag_3 = %d, flag_2 = %d, flag_1 = %d\n",
              flag_4, flag_3, flag_2, flag_1);
  if (flag_4) {
    flag_3 = flag_2 = flag_1 = true;
  } else if (flag_3) {
    flag_2 = flag_1 = true;
  } else if (flag_2) {
    flag_1 = true;
  }
  diag_printf("flag_4 = %d, flag_3 = %d, flag_2 = %d, flag_1 = %d\n",
              flag_4, flag_3, flag_2, flag_1);

  if (flag_1) {
    HAL_READ_UINT16(CLKM_ARM_IDLECT2, tmp);
    diag_printf("tmp = %04X\n", tmp);
  }

  if (flag_2) {
    tmp |= 1;
    diag_printf("Writing %04X to %08X\n", tmp, CLKM_ARM_IDLECT2);
    HAL_WRITE_UINT16(CLKM_ARM_IDLECT2, tmp | 1);
  }

  if (flag_3) {
    diag_printf("Writing %04X to %08X\n", 0x80F5, WATCHDOG_TIMER_MODE);
    HAL_WRITE_UINT16(WATCHDOG_TIMER_MODE, 0x80F5); \
  }

  if (flag_4) {
    diag_printf("Writing %04X to %08X\n", 0x80F5, WATCHDOG_TIMER_MODE);
    HAL_WRITE_UINT16(WATCHDOG_TIMER_MODE, 0x80F5); \
  }

  diag_printf("try_reset: done\n");
}

#define SDRAM_BASE 0x10000000
#define SDRAM_LEN  (32*1024*1024)	/* 32 Mbytes */
static void
do_testsdram(int argc,
	     char *argv[])
{
  cyg_uint32 *mem_addr = (cyg_uint32 *)SDRAM_BASE;
  unsigned    len;
  bool        len_set;

  register int i;
  struct option_info opts[1];

  init_opts(&opts[0], 'l', true, OPTION_ARG_TYPE_NUM,
            (void**)&len, &len_set, "length");

  if (!scan_opts(argc, argv,
                 1,
                 opts, 1,
                 0, 0, 0)) {
    return;
  }
  if (!len_set) {
    len = SDRAM_LEN / 4;
  }

  diag_printf("Length = 0x%08X\n", len);

  diag_printf("Writing data-equals-address pattern to SDRAM\n");
  for (i = 0; i < len; i++) {
    mem_addr[i] = ~(cyg_uint32)(mem_addr + i);
  }
  diag_printf("Reading back pattern\n");
  for (i = 0; i < len; i++) {
    if (mem_addr[i] != ~(cyg_uint32)(mem_addr + i)) {
      diag_printf("Error: mismatch at address %p, read back 0x%08x\n",
		  mem_addr + i,
		  mem_addr[i]);
      break;
    }
  }
  diag_printf("Done\n");
}

static void
do_delay(int argc,
         char *argv[])
{
  struct option_info opts[1];
  int count = 0x00000000;
  bool count_set;
  int value;
  register int i;

  init_opts(&opts[0], 'c', true, OPTION_ARG_TYPE_NUM,
            (void**)&count, &count_set, "Number of times to delay");

  if (!scan_opts(argc, argv,
                 1,
                 opts, sizeof(opts) / sizeof(opts[0]),
                 (void*)&value, OPTION_ARG_TYPE_NUM, "amount of time to delay (usec)")) {
    return;
  }

  if (!count_set) {
    count = 1;
  }
  for (i = 0; i < count; i++) {
    diag_printf("Delaying %d useconds...", value);
    HAL_DELAY_US(value);
    diag_printf("Done\n");
  }
}

#endif
