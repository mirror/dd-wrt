//==========================================================================
//
//      dir.cxx
//
//      Fileio directory support
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-05-25
// Purpose:             Fileio directory support
// Description:         Support for directory operations.
//                      
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <stdarg.h>                     // for fcntl()

#include "fio.h"                       // Private header

#include <dirent.h>                    // struct dirent

//==========================================================================

#define DIROPEN_RETURN_ERR( err )               \
CYG_MACRO_START                                 \
    errno = err;                                \
    CYG_REPORT_RETVAL( NULL );                  \
    return NULL;                                \
CYG_MACRO_END

//==========================================================================
// Implement filesystem locking protocol. 

#define LOCK_FS( _mte_ )  {                             \
   CYG_ASSERT(_mte_ != NULL, "Bad mount table entry");  \
   cyg_fs_lock( _mte_, (_mte_)->fs->syncmode);          \
}

#define UNLOCK_FS( _mte_ ) cyg_fs_unlock( _mte_, (_mte_)->fs->syncmode)

//==========================================================================
// Open a directory for reading

extern DIR *opendir( const char *dirname )
{
    FILEIO_ENTRY();

    CYG_CANCELLATION_POINT;

    int ret = 0;
    int fd;
    cyg_file *file;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = dirname;

    fd = cyg_fd_alloc(1); // Never return fd 0

    if( fd < 0 )
        DIROPEN_RETURN_ERR(EMFILE);
    
    file = cyg_file_alloc();

    if( file == NULL )
    {
        cyg_fd_free(fd);
        DIROPEN_RETURN_ERR(ENFILE);
    }
    
    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
    {
        cyg_fd_free(fd);
        cyg_file_free(file);
        DIROPEN_RETURN_ERR(ENOENT);
    }

    LOCK_FS( mte );
    
    ret = mte->fs->opendir( mte, dir, name, file );
    
    UNLOCK_FS( mte );
    
    if( 0 != ret )
    {
        cyg_fd_free(fd);
        cyg_file_free(file);
        DIROPEN_RETURN_ERR(ret);
    }

    file->f_flag |= CYG_FDIR|CYG_FREAD;
    file->f_mte = mte;
    file->f_syncmode = mte->fs->syncmode;
    
    cyg_fd_assign( fd, file );

    DIR *dirp = (DIR *)fd;
    
    FILEIO_RETURN_VALUE(dirp);
}

//==========================================================================
// Read a directory entry.
// This is the thread-unsafe version that uses a static result buffer.
// It just calls the thread-safe version to do the work.

extern struct dirent *readdir( DIR *dirp )
{
    FILEIO_ENTRY();
    
    static struct dirent ent;
    struct dirent *result;
    int err;

    err = readdir_r( dirp, &ent, &result );

    if( err != 0 )
    {
        errno = err;
        FILEIO_RETURN_VALUE( NULL );
    }
    
    FILEIO_RETURN_VALUE( result );
}

//==========================================================================

extern int readdir_r( DIR *dirp, struct dirent *entry, struct dirent **result )
{
    FILEIO_ENTRY();

    int fd = (int)dirp;    
    ssize_t res;

    *result = NULL;

    if( NULL == dirp )
    {
        FILEIO_RETURN_VALUE( EBADF );
    }

    res = read( fd, (void *)entry, sizeof(struct dirent));

    if( res < 0 )
    {
        FILEIO_RETURN_VALUE( errno );
    }
    
    if( res > 0 )
        *result = entry;
    
    FILEIO_RETURN( ENOERR );
}

//==========================================================================

extern void rewinddir( DIR *dirp )
{
    FILEIO_ENTRY();

    int fd = (int)dirp;

    lseek( fd, 0, SEEK_SET );
    
    FILEIO_RETURN_VOID();
}

//==========================================================================

extern int closedir( DIR *dirp )
{
    FILEIO_ENTRY();

    int fd = (int)dirp;
    int err = close( fd );

    FILEIO_RETURN_VALUE( err );
}

// -------------------------------------------------------------------------
// EOF dir.cxx
