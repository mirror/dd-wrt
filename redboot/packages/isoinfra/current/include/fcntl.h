#ifndef CYGONCE_ISO_FCNTL_H
#define CYGONCE_ISO_FCNTL_H
/*========================================================================
//
//      fcntl.h
//
//      POSIX file control functions
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-05-05
// Purpose:       This file provides the macros, types and functions
//                for file control required by POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <fcntl.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

#ifdef CYGBLD_ISO_OFLAG_HEADER
# include CYGBLD_ISO_OFLAG_HEADER
#else

/* File access modes used for open() and fnctl() */
#define O_RDONLY     (1<<0)   /* Open for reading only */
#define O_WRONLY     (1<<1)   /* Open for writing only */
#define O_RDWR       (O_RDONLY|O_WRONLY) /* Open for reading and writing */

/* File access mode mask */
#define O_ACCMODE    (O_RDONLY|O_RDWR|O_WRONLY)

/* open() mode flags */

#define O_CREAT      (1<<3)    /* Create file it it does not exist */
#define O_EXCL       (1<<4)    /* Exclusive use */
#define O_NOCTTY     (1<<5)    /* Do not assign a controlling terminal */
#define O_TRUNC      (1<<6)    /* Truncate */

/* File status flags used for open() and fcntl() */
#define O_APPEND     (1<<7)    /* Set append mode */
#define O_DSYNC      (1<<8)    /* Synchronized I/O data integrity writes */
#define O_NONBLOCK   (1<<9)    /* No delay */
#define O_RSYNC      (1<<10)   /* Synchronized read I/O */
#define O_SYNC       (1<<11)   /* Synchronized I/O file integrity writes */

#endif /* ifndef CYGBLD_ISO_OFLAG_HEADER */


#if CYGINT_ISO_FCNTL
# ifdef CYGBLD_ISO_FCNTL_HEADER
#  include CYGBLD_ISO_FCNTL_HEADER
# else

/* fcntl() command values */

#define F_DUPFD      (1<<0)    /* Duplicate file descriptor */
#define F_GETFD      (1<<1)    /* Get file descriptor flags */
#define F_SETFD      (1<<2)    /* Set file descriptor flags */
#define F_GETFL      (1<<3)    /* Get file status flags */
#define F_SETFL      (1<<4)    /* Set file status flags */
#define F_GETLK      (1<<5)    /* Get record locking information */
#define F_SETLK      (1<<6)    /* Set record locking information */
#define F_SETLKW     (1<<7)    /* Set record locking info; wait if blocked */

/* fd flags */
#define FD_CLOEXEC   (1<<0)    /* Close fd on exec */

/* Lock types */

#define F_RDLCK      (1<<0)    /* Shared or read lock */
#define F_UNLCK      (1<<1)    /* Unlock */
#define F_WRLCK      (1<<2)    /* Exclusive or write lock */

#include <sys/types.h>   /* off_t, pid_t */

struct flock {
    short l_type;     /* F_RDLCK, F_WRLCK, F_UNLCK */
    short l_whence;   /* Flag for starting offset */
    off_t l_start;    /* Relative offset in bytes */
    off_t l_len;      /* Size; if 0, then until EOF */
    pid_t l_pid;      /* Process ID of the process holding the lock,
                       * returned with F_GETLK. */
};

#ifdef __cplusplus
extern "C"
#else
extern
#endif

int
fcntl( int /* fildes */, int /* cmd */, ... );

# endif /* ifndef CYGBLD_ISO_FCNTL_HEADER */
#endif  /* if CYGINT_ISO_FCNTL */


#if CYGINT_ISO_OPEN
# ifdef CYGBLD_ISO_OPEN_HEADER
#  include CYGBLD_ISO_OPEN_HEADER
# else

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>    /* mode_t */

extern int
open( const char * /* path */, int /* oflag */, ... );

extern int
creat( const char * /* path */, mode_t /* mode */ );

#ifdef __cplusplus
}   /* extern "C" */
#endif

# endif /* ifndef CYGBLD_ISO_OPEN_HEADER */
#endif  /* if CYGINT_ISO_OPEN */

#endif /* CYGONCE_ISO_FCNTL_H multiple inclusion protection */

/* EOF fcntl.h */
