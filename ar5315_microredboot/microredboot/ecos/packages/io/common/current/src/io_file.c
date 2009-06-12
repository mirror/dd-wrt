//==========================================================================
//
//      io/common/io_file.c
//
//      High-level file I/O support.
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


// File I/O support

#include <pkgconf/io.h>
#include <cyg/io/file.h>
#include <cyg/error/codes.h>

struct file fds[CYGPKG_IO_NFILE];

//
// This function allcates a file "slot".
//
int
falloc(struct file **fp, int *fd)
{
    int i;
    struct file *f;
    f = &fds[0];
    for (i = 0;  i < CYGPKG_IO_NFILE;  i++, f++) {
        if (f->f_flag == 0) {
            f->f_flag = FALLOC;
            *fp = f;
            *fd = i;
            return 0;
        }
    }
    return EMFILE;  // No more files
}

//
// This function is used to return a file slot.
//
void
ffree(struct file *fp)
{
    fp->f_flag = 0;  // Mark free
}

//
// This function provides the mapping from a file descriptor (small
// integer used by application code) to the corresponding file slot.
cyg_bool
getfp(int fd, struct file **fp)
{
    struct file *f;
    if (fd >= CYGPKG_IO_NFILE)
        return -1;
    f =  &fds[fd];
    if (f->f_flag == 0)
        return -1;
    *fp = f;
    return 0;
}
