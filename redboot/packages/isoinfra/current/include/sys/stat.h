#ifndef CYGONCE_ISO_STAT_H
#define CYGONCE_ISO_STAT_H
/*========================================================================
//
//      stat.h
//
//      POSIX file characteristics
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
// Date:          2000-05-08
// Purpose:       This file provides the macros, types and functions
//                for file characteristics required by POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <sys/stat.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

#ifdef CYGBLD_ISO_STAT_DEFS_HEADER
# include CYGBLD_ISO_STAT_DEFS_HEADER
#else

#include <sys/types.h>   /* ino_t, dev_t, etc. */
#include <time.h>        /* time_t */

#define __stat_mode_DIR    (1<<0)
#define __stat_mode_CHR    (1<<1)
#define __stat_mode_BLK    (1<<2)
#define __stat_mode_REG    (1<<3)
#define __stat_mode_FIFO   (1<<4)
#define __stat_mode_MQ     (1<<5)
#define __stat_mode_SEM    (1<<6)
#define __stat_mode_SHM    (1<<7)
#define __stat_mode_LNK    (1<<8)
#define __stat_mode_SOCK   (1<<9)

#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE >= 200112L)
#define S_IFDIR          (__stat_mode_DIR)
#define S_IFCHR          (__stat_mode_CHR)
#define S_IFBLK          (__stat_mode_BLK)
#define S_IFREG          (__stat_mode_REG)
#define S_IFIFO          (__stat_mode_FIFO)
#define S_IFLNK          (__stat_mode_LNK)
#define S_IFSOCK         (__stat_mode_SOCK)
#define S_IFMT           (S_IFDIR|S_IFCHR|S_IFBLK|S_IFREG| \
                          S_IFIFO|S_IFLNK|S_IFSOCK)
#endif

#define S_ISDIR(__mode)  ((__mode) & __stat_mode_DIR )
#define S_ISCHR(__mode)  ((__mode) & __stat_mode_CHR )
#define S_ISBLK(__mode)  ((__mode) & __stat_mode_BLK )
#define S_ISREG(__mode)  ((__mode) & __stat_mode_REG )
#define S_ISFIFO(__mode) ((__mode) & __stat_mode_FIFO )
#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE >= 200112L)
#define S_ISLNK(__mode)  ((__mode) & __stat_mode_LNK )
#define S_ISSOCK(__mode)  ((__mode) & __stat_mode_SOCK )
#endif

#define S_TYPEISMQ(__buf)   ((__buf)->st_mode & __stat_mode_MQ )
#define S_TYPEISSEM(__buf)  ((__buf)->st_mode & __stat_mode_SEM )
#define S_TYPEISSHM(__buf)  ((__buf)->st_mode & __stat_mode_SHM )


#define S_IRUSR  (1<<16)
#define S_IWUSR  (1<<17)
#define S_IXUSR  (1<<18)
#define S_IRWXU  (S_IRUSR|S_IWUSR|S_IXUSR)

#define S_IRGRP  (1<<19)
#define S_IWGRP  (1<<20)
#define S_IXGRP  (1<<21)
#define S_IRWXG  (S_IRGRP|S_IWGRP|S_IXGRP)

#define S_IROTH  (1<<22)
#define S_IWOTH  (1<<23)
#define S_IXOTH  (1<<24)
#define S_IRWXO  (S_IROTH|S_IWOTH|S_IXOTH)

#define S_ISUID  (1<<25)
#define S_ISGID  (1<<26)


struct stat {
    mode_t  st_mode;     /* File mode */
    ino_t   st_ino;      /* File serial number */
    dev_t   st_dev;      /* ID of device containing file */
    nlink_t st_nlink;    /* Number of hard links */
    uid_t   st_uid;      /* User ID of the file owner */
    gid_t   st_gid;      /* Group ID of the file's group */
    off_t   st_size;     /* File size (regular files only) */
    time_t  st_atime;    /* Last access time */
    time_t  st_mtime;    /* Last data modification time */
    time_t  st_ctime;    /* Last file status change time */
}; 

#endif /* ifndef CYGBLD_ISO_STAT_DEFS_HEADER */

/* PROTOTYPES */

__externC int stat( const char *path, struct stat *buf );

__externC int fstat( int fd, struct stat *buf );

__externC int mkdir(const char *path, mode_t mode);

#endif /* CYGONCE_ISO_STAT_H multiple inclusion protection */

/* EOF stat.h */
