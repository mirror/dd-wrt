//==========================================================================
//
//      io/common/include/file.h
//
//      Defines for high level file I/O
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


#ifndef _CYG_IO_FILE_H_
#define _CYG_IO_FILE_H_

#include <pkgconf/system.h>

//==========================================================================
// If the fileio package is loaded, we need to go through that to do all
// basic IO operations. This code redefines the tags on the structures so
// that they have the names expected by BSD based code.

#ifdef CYGPKG_IO_FILEIO

#include <pkgconf/io_fileio.h>

#define CYG_IOVEC_TAG iovec
#define CYG_UIO_TAG uio
#define CYG_FILEOPS_TAG fileops
#define CYG_FILE_TAG file
#define CYG_SELINFO_TAG selinfo

#include <cyg/fileio/fileio.h>

// File states
#define FREAD      CYG_FREAD
#define FWRITE     CYG_FWRITE
#define FNONBLOCK  CYG_FNONBLOCK
#define FASYNC     CYG_FASYNC

// Type of "file"
#define	DTYPE_VNODE	CYG_FILE_TYPE_FILE	/* file */
#define	DTYPE_SOCKET	CYG_FILE_TYPE_SOCKET	/* communications endpoint */

//==========================================================================
// Otherwise define all the structs here...

#else // CYGPKG_IO_FILEIO

// High-level file I/O interfaces
// Derived [in part] from OpenBSD <sys/file.h>, <sys/uio.h>, <sys/fcntl.h>

#include <pkgconf/io.h>
#include <cyg/infra/cyg_type.h>

#define NFILE CYGPKG_IO_NFILE

struct iovec {
    void           *iov_base;   /* Base address. */
    CYG_ADDRWORD   iov_len;     /* Length. */
};

enum	uio_rw { UIO_READ, UIO_WRITE };

/* Segment flag values. */
enum uio_seg {
    UIO_USERSPACE,		/* from user data space */
    UIO_SYSSPACE		/* from system space */
};

struct uio {
    struct	iovec *uio_iov;	/* pointer to array of iovecs */
    int	uio_iovcnt;	/* number of iovecs in array */
    CYG_ADDRWORD	uio_offset;	/* offset into file this uio corresponds to */
    CYG_ADDRWORD	uio_resid;	/* residual i/o count */
    enum	uio_seg uio_segflg; /* see above */
    enum	uio_rw uio_rw;	/* see above */
};

/*
 * Limits
 */
#define UIO_SMALLIOV	8		/* 8 on stack, else malloc */

// Description of open file
struct file {
    short	f_flag;		/* file state */
    short	f_type;		/* descriptor type */
    struct	fileops {
        int	(*fo_read)(struct file *fp, struct uio *uio);
        int	(*fo_write)(struct file *fp, struct uio *uio);
        int	(*fo_ioctl)(struct file *fp, CYG_ADDRWORD com,
                            CYG_ADDRWORD data);
        int	(*fo_select)(struct file *fp, int which);
        int	(*fo_close)(struct file *fp);
    } *f_ops;
    CYG_ADDRWORD	f_offset;
    CYG_ADDRWORD	f_data;		/* vnode or socket */
};

// File states
#define FREAD      0x01
#define FWRITE     0x02
#define FNONBLOCK  0x10
#define FASYNC     0x20
#define FALLOC     0x80         // File is "busy", i.e. allocated

// Type of "file"
#define	DTYPE_VNODE	1	/* file */
#define	DTYPE_SOCKET	2	/* communications endpoint */
#define	DTYPE_PIPE	3	/* pipe */

externC cyg_bool getfp(int fdes, struct file **fp);
externC int falloc(struct file **fp, int *fd);
externC void ffree(struct file *fp);

//==========================================================================

#endif // CYGPKG_IO_FILEIO

//==========================================================================
#endif // _CYG_IO_FILE_H_
