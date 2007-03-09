//==========================================================================
//
//      splash.c
//
//      RedBoot framebuffer snapshot/restore to flash config space
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
// Author(s):    dwmw2
// Contributors: dwmw2
// Date:         2002-01-23
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <redboot.h>
#include <flash_config.h>
#include <pkgconf/io_gfx_drivers.h>
#include <cyg/gfx/gfx_dev.h>


RedBoot_cmd("splash",
	    "Show splash screen",
	    "",
	    do_splash_withargs
    );

RedBoot_cmd("setsplash",
	    "Set splash screen from current framebuffer contents",
	    "",
	    do_setsplash
    );

#ifdef CYGPKG_IO_GFX_BMP_SUPPORT
RedBoot_cmd("showbmp",
	    "Display loaded Windows .BMP image",
	    "",
	    do_showbmp
    );
#endif

void do_splash(void)
{
	unsigned char *image;
	unsigned long len = CYGNUM_REDBOOT_FLASH_SPLASH_SIZE;

	if (!flash_get_config("splash", &image, CONFIG_SPLASH))
		return;

	if (!cyg_io_gfx_fb) {
		diag_printf( "Framebuffer not initialised. No splash screen\n");
		return;
	}

	if (cyg_io_gfx_fb_size < len)
		len = cyg_io_gfx_fb_size;

	memcpy(cyg_io_gfx_fb, image, len);
}

RedBoot_init(do_splash, RedBoot_INIT_PRIO(1));

void do_splash_withargs(int argc, char *argv[])
{
	/* Bah. */
	do_splash();
}


void do_setsplash(int argc, char *argv[])
{
	struct config_option opt;

	if (!cyg_io_gfx_fb) {
		diag_printf("Framebuffer not initialised. Cannot snapshot\n");
		return;
	}
	if (cyg_io_gfx_fb_size < CYGNUM_REDBOOT_FLASH_SPLASH_SIZE) {
		diag_printf("Framebuffer too small. Cannot safely snapshot\n");
		return;
	}

	opt.type = CONFIG_SPLASH;
	opt.enable = (char *)0;
	opt.enable_sense = 1;
	opt.key = "splash";
	opt.dflt = (CYG_ADDRESS)cyg_io_gfx_fb;

	flash_add_config(&opt, true);
}


#ifdef CYGPKG_IO_GFX_BMP_SUPPORT
void do_showbmp(int argc, char *argv[])
{
	if (!load_address) {
		diag_printf("No image has been loaded\n");
		return;
	}
	if (!cyg_io_gfx_fb) {
		diag_printf("Graphics device not present/initialised\n");
		return;
	}

	cyg_io_gfx_fill(0, 0, cyg_io_gfx_xres(), cyg_io_gfx_yres(), 0);
	cyg_io_gfx_show_bmp(0, 0, load_address);
}
#endif
