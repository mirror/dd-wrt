//==========================================================================
//
//      syscall.c
//
//      Minimal generic syscall support.
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
// Purpose:      Minimal generic syscall support.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <errno.h>
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "syscall.h"

/*
 * read  -- read bytes from the serial port. Ignore fd, since
 *          we only have stdin.
 */
static int
sys_read(int fd, char *buf, int nbytes)
{
    int i = 0;

    for (i = 0; i < nbytes; i++) {
	*(buf + i) = bsp_console_getc();
	if ((*(buf + i) == '\n') || (*(buf + i) == '\r')) {
	    (*(buf + i + 1)) = 0;
	    break;
	}
    }
    return (i);
}


/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
static int
sys_write(int fd, char *buf, int nbytes)
{
#define WBUFSIZE  256
    char ch, lbuf[WBUFSIZE];
    int  i, tosend;

    tosend = nbytes;

    while (tosend > 0) {
	for (i = 0; tosend > 0 && i < (WBUFSIZE-2); tosend--) {
	    ch = *buf++;
	    if (ch == '\n')
		lbuf[i++] = '\r';
	    lbuf[i++] = ch;
	}
	bsp_console_write(lbuf, i);
    }

    return (nbytes);
}


/*
 * open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 */
static int
sys_open (const char *buf, int flags, int mode)
{
    return (-EIO);
}


/*
 * close -- We don't need to do anything, but pretend we did.
 */
static int
sys_close(int fd)
{
    return (0);
}


/*
 * lseek --  Since a serial port is non-seekable, we return an error.
 */
static int
sys_lseek(int fd,  int offset, int whence)
{
#ifdef ESPIPE
    return (-ESPIPE);
#else
    return (-EIO);
#endif
}


/*
 *  Generic syscall handler.
 *
 *  Returns 0 if syscall number is not handled by this
 *  module, 1 otherwise. This allows applications to
 *  extend the syscall handler by using exception chaining.
 */
int
_bsp_do_syscall(int func,		/* syscall function number */
		long arg1, long arg2,	/* up to four args.        */
		long arg3, long arg4,
		int *retval)		/* syscall return value    */
{
    int err = 0;

    switch (func) {

      case SYS_read:
	err = sys_read((int)arg1, (char *)arg2, (int)arg3);
	break;

      case SYS_write:
	err = sys_write((int)arg1, (char *)arg2, (int)arg3);
	break;

      case SYS_open:
	err = sys_open((const char *)arg1, (int)arg2, (int)arg3);
	break;

      case SYS_close:
	err = sys_close((int)arg1);
	break;

      case SYS_lseek:
	err = sys_lseek((int)arg1, (int)arg2, (int)arg3);
	break;

      case BSP_GET_SHARED:
	*(bsp_shared_t **)arg1 = bsp_shared_data;
	break;

      case SYS_meminfo:
        {
          // Return the top and size of memory.
          struct bsp_mem_info      mem;
          int                      i;
          unsigned long            u, totmem, topmem, numbanks;

          i = totmem = topmem = numbanks = 0;
          while (bsp_sysinfo(BSP_INFO_MEMORY, i++, &mem) == 0)
            {
              if (mem.kind == BSP_MEM_RAM)
                {
                  numbanks++;
                  totmem += mem.nbytes;
                  u = (unsigned long)mem.virt_start + mem.nbytes;
                  if (u > topmem)
                    topmem = u;
                }
            }
          *(unsigned long *)arg1 = totmem;
          *(unsigned long *)arg2 = topmem;
          *retval = numbanks;
        }
        return 1;
        break;

      default:
	return 0;
    }    

    *retval = err;
    return 1;
}


