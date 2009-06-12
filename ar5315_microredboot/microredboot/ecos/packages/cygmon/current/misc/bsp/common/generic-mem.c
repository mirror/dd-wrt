//==========================================================================
//
//      generic-mem.c
//
//      Generic support for safe memory read/write.
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
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Generic support for safe memory read/write.
// Description:  Some targets may need to provide their own version.
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <stdlib.h>
#include <setjmp.h>
#include <bsp/bsp.h>

typedef void (*moveproc_t)(void *s, void *d);

static jmp_buf __errjmp;

/*
 * These are globals because we want them preserved
 * across function calls.
 */
static bsp_handler_t __oldtrap;
static int __done;


static void
move_8(void *src, void *dest)
{
    *(char *)dest = *(char *)src;
}


static void
move_16(void *src, void *dest)
{
    *(short *)dest = *(short *)src;
}


static void
move_32(void *src, void *dest)
{
    *(int *)dest = *(int *)src;
}


static int
err_trap(int exc_nr, void *regs)
{
    longjmp(__errjmp, 1);
}


int
bsp_memory_read(void *addr,    /* start addr of memory to read */
		int  asid,     /* address space id */
		int  rsize,    /* size of individual read operation */
		int  nreads,   /* number of read operations */
		void *buf)     /* result buffer */
{
    if (nreads <= 0)
	return 0;

    __oldtrap = bsp_install_dbg_handler(err_trap);
    
    if (setjmp(__errjmp) == 0) {
	moveproc_t move_mem;
	int        incr;
	char       *src, *dest;

	switch (rsize) {
	  case 8:
	    move_mem = move_8;
	    incr = 1;
	    break;
	  case 16:
	    move_mem = move_16;
	    incr = 2;
	    break;
	  case 32:
	    move_mem = move_32;
	    incr = 4;
	    break;
	  default:
	    (void)bsp_install_dbg_handler(__oldtrap);
	    return 0;
	}

	src = addr;
	dest = buf;

	for (__done = 0; __done < nreads; __done++) {
	    move_mem(src, dest);
	    src  += incr;
	    dest += incr;
	}
    }

    (void)bsp_install_dbg_handler(__oldtrap);
    return __done;
}


int bsp_memory_write(void *addr,   /* start addr of memory to write */
                     int  asid,    /* address space id */
                     int  wsize,   /* size of individual write operation */
                     int  nwrites, /* number of write operations */
                     void *buf)    /* source buffer for write data */
{
    if (nwrites <= 0)
	return 0;

    __oldtrap = bsp_install_dbg_handler(err_trap);
    
    if (setjmp(__errjmp) == 0) {
	moveproc_t move_mem;
	int        incr;
	char       *src, *dest;

	switch (wsize) {
	  case 8:
	    move_mem = move_8;
	    incr = 1;
	    break;
	  case 16:
	    move_mem = move_16;
	    incr = 2;
	    break;
	  case 32:
	    move_mem = move_32;
	    incr = 4;
	    break;
	  default:
	    (void)bsp_install_dbg_handler(__oldtrap);
	    return 0;
	}

	src = buf;
	dest = addr;

	for (__done = 0; __done < nwrites; __done++) {
	    move_mem(src, dest);
	    src  += incr;
	    dest += incr;
	}
    }

    (void)bsp_install_dbg_handler(__oldtrap);
    return __done;
}
