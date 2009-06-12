//==========================================================================
//
//      misc.cxx
//
//      Fileio miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Purpose:             Fileio miscellaneous functions
// Description:         This file contains various miscellaneous functions
//                      for use with the fileio system. These include startup,
//                      table management, and other service routines.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>
#ifdef CYGPKG_LIBC_TIME
#include <pkgconf/libc_time.h>
#endif


#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <string.h>                    // strcmp()
#include <time.h>                      // time()

#ifdef CYGPKG_IO_WALLCLOCK
# include <cyg/io/wallclock.hxx>       // Wallclock class
#endif

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/kernel/clock.inl>         // Clock inlines
#endif

#include "fio.h"                       // Private header

//==========================================================================
// forward definitions

static void cyg_mtab_init();

__externC int chdir( const char *path );

//==========================================================================
// Filesystem tables

// -------------------------------------------------------------------------
// Filesystem table.

// This array contains entries for all filesystem that are installed in
// the system.
__externC cyg_fstab_entry cyg_fstab[];
CYG_HAL_TABLE_BEGIN( cyg_fstab, fstab );

// end of filesystem table, set in linker script.
__externC cyg_fstab_entry cyg_fstab_end;
CYG_HAL_TABLE_END( cyg_fstab_end, fstab );

#ifdef CYGPKG_KERNEL
// Array of mutexes for locking the fstab entries
static Cyg_Mutex fstab_lock[CYGNUM_FILEIO_FSTAB_MAX] CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);
#endif

// -------------------------------------------------------------------------
// Mount table.

// This array contains entries for all valid running filesystems.
__externC cyg_mtab_entry cyg_mtab[];
CYG_HAL_TABLE_BEGIN( cyg_mtab, mtab );

// Extra entries at end of mtab for dynamic mount points.
cyg_mtab_entry cyg_mtab_extra[CYGNUM_FILEIO_MTAB_EXTRA] CYG_HAL_TABLE_EXTRA(mtab) = { { NULL } };

// End of mount table, set in the linker script.
__externC cyg_mtab_entry cyg_mtab_end;
CYG_HAL_TABLE_END( cyg_mtab_end, mtab );

#ifdef CYGPKG_KERNEL
// Array of mutexes for locking the mtab entries
static Cyg_Mutex mtab_lock[CYGNUM_FILEIO_MTAB_MAX] CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);
#endif

//==========================================================================
// Current directory

cyg_mtab_entry *cyg_cdir_mtab_entry = NULL;
cyg_dir cyg_cdir_dir = CYG_DIR_NULL;

//==========================================================================
// Initialization object

class Cyg_Fileio_Init_Class
{
public:    
    Cyg_Fileio_Init_Class();
};

static Cyg_Fileio_Init_Class fileio_initializer CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);

Cyg_Fileio_Init_Class::Cyg_Fileio_Init_Class()
{
    cyg_fd_init();

    cyg_mtab_init();

    chdir("/");
}

//==========================================================================
// Mount table initializer

static void cyg_mtab_init()
{
    cyg_mtab_entry *m;
    
    for( m = &cyg_mtab[0]; m != &cyg_mtab_end; m++ )
    {
        const char *fsname = m->fsname;
        cyg_fstab_entry *f;

        // Ignore empty entries
        if( m->name == NULL )
            continue;
        
        // stop if there are more than the configured maximum
        if( m-&cyg_mtab[0] >= CYGNUM_FILEIO_MTAB_MAX )
            break;
        
        for( f = &cyg_fstab[0]; f != &cyg_fstab_end; f++ )
        {
            // stop if there are more than the configured maximum
            if( f-&cyg_fstab[0] >= CYGNUM_FILEIO_FSTAB_MAX )
                break;
            
            if( strcmp( fsname, f->name) == 0 )
            {
                // We have a match.

                if( f->mount( f, m ) == 0 )
                {
                    m->valid    = true;
                    m->fs       = f;
                    // m->root installed by fs.
                }
                else
                {
                    m->valid = false;
                }
                
                break;
            }
        }
    }
}

//==========================================================================
// Mount table matching

// -------------------------------------------------------------------------
// matchlen() compares two strings and returns the number of bytes by which
// they match.

static int matchlen( const char *s1, const char *s2 )
{
    int len = 0;
    while( s1[len] == s2[len] && s1[len] && s2[len] ) len++;

    // Return length only if s2 is an initial substring of s1,
    // and it terminates in s1 at end-of-string or a '/'.

    // Special case for s2 == "/"
    if( len == 1 && s2[0] == '/' && s2[1] == 0 )
        return len;
    
    if( (s2[len] == 0) && (s1[len] == 0 || s1[len] == '/'))
         return len;
    else return 0;
}

// -------------------------------------------------------------------------
// Search the mtab for the entry that matches the longest substring of
// **name. 

__externC int cyg_mtab_lookup( cyg_dir *dir, const char **name, cyg_mtab_entry **mte)
{
    cyg_mtab_entry *m, *best = NULL;
    int best_len = 0;

    // Unrooted file names go straight to current dir
    if( **name != '/' ) {
        if (*mte == (cyg_mtab_entry *)NULL) {
            // No known current directory
            return -1;
        }
        // Current directory is well known
        return 0;
    }

    // Otherwise search the mount table.
    for( m = &cyg_mtab[0]; m != &cyg_mtab_end; m++ )
    {
        if( m->name != NULL && m->valid )
        {
            int len = matchlen(*name,m->name);
            if( len > best_len )
                best = m, best_len = len;
        }
    }

    // No match found, bad path name...
    if( best_len == 0 ) return -1;
    
    *name += best_len;
    if( **name == '/' )
        (*name)++;
    *mte = best;
    *dir = best->root;

    return 0;
}

//==========================================================================
// mount filesystem

__externC int mount( const char *devname,
                     const char *dir,
                     const char *fsname)
{

    FILEIO_ENTRY();
    
    cyg_mtab_entry *m;
    cyg_fstab_entry *f;
    int result = ENOERR;

    // Search the mount table for an empty entry
    for( m = &cyg_mtab[0]; m != &cyg_mtab_end; m++ )
    {
        // stop if there are more than the configured maximum
        if( m-&cyg_mtab[0] >= CYGNUM_FILEIO_MTAB_MAX )
        {
            m = &cyg_mtab_end;
            break;
        }

         if( m->name == NULL ) break;
    }

    if( m == &cyg_mtab_end )
        FILEIO_RETURN(ENOMEM);

    // Now search the fstab for the filesystem implementation
    for( f = &cyg_fstab[0]; f != &cyg_fstab_end; f++ )
    {
        // stop if there are more than the configured maximum
        if( f-&cyg_fstab[0] >= CYGNUM_FILEIO_FSTAB_MAX )
            break;
            
        if( strcmp( fsname, f->name) == 0 )
            break;
    }

    if( f == &cyg_fstab_end )
        FILEIO_RETURN(ENODEV);
            
    // We have a match.

    m->name = dir;
    m->fsname = fsname;
    m->devname = devname;
    
    if( (result = f->mount( f, m )) == 0 )
    {
        m->valid    = true;
        m->fs       = f;
        // m->root installed by fs.
    }
    else
    {
        m->valid = false;
        m->name = NULL;
    }

    // Make sure that there is something to search (for open)

    if (cyg_cdir_mtab_entry == (cyg_mtab_entry *)NULL) {
        cyg_cdir_mtab_entry = m;
    }

    FILEIO_RETURN(result);
}

//==========================================================================
// unmount filesystem

__externC int umount( const char *name)
{
    int err = ENOERR;
    
    FILEIO_ENTRY();
    
    cyg_mtab_entry *m;

    // Search the mount table for a matching entry
    for( m = &cyg_mtab[0]; m != &cyg_mtab_end; m++ )
    {
        // stop if there are more than the configured maximum
        if( m-&cyg_mtab[0] >= CYGNUM_FILEIO_MTAB_MAX )
        {
            m = &cyg_mtab_end;
            break;
        }

        // Ignore empty or invalid entries
         if( m->name == NULL || !m->valid ) continue;

         // match names.
         if( strcmp(name,m->name) == 0 ) break;

         // Match device name too?
    }

    if( m == &cyg_mtab_end )
        FILEIO_RETURN(EINVAL);

    // We have a match, call the umount function

    err = m->fs->umount( m );

    if( err == ENOERR )
    {
        m->valid        = false;
        m->name         = NULL;
    }
    
    FILEIO_RETURN(err);
}

//==========================================================================
// Implement filesystem locking protocol. 

void cyg_fs_lock( cyg_mtab_entry *mte, cyg_uint32 syncmode )
{
    CYG_ASSERT(mte != NULL, "Bad mount table entry");

    if( syncmode & CYG_SYNCMODE_FILE_FILESYSTEM ) {
        CYG_ASSERT(mte->fs-&cyg_fstab[0] < CYGNUM_FILEIO_FSTAB_MAX, "Bad file system");
        FILEIO_MUTEX_LOCK( fstab_lock[mte->fs-&cyg_fstab[0]] );
    }

    if( syncmode & CYG_SYNCMODE_FILE_MOUNTPOINT ) {
        CYG_ASSERT(mte-&cyg_mtab[0] < CYGNUM_FILEIO_MTAB_MAX, "Bad mount point");
        FILEIO_MUTEX_LOCK( mtab_lock[mte-&cyg_mtab[0]] );
    }
}

void cyg_fs_unlock( cyg_mtab_entry *mte, cyg_uint32 syncmode )
{
    CYG_ASSERT(mte != NULL, "Bad mount table entry");

    if( syncmode & CYG_SYNCMODE_FILE_FILESYSTEM ) {
        CYG_ASSERT(mte->fs-&cyg_fstab[0] < CYGNUM_FILEIO_FSTAB_MAX, "Bad file system");
        FILEIO_MUTEX_UNLOCK( fstab_lock[mte->fs-&cyg_fstab[0]] );
    }

    if( syncmode & CYG_SYNCMODE_FILE_MOUNTPOINT ) {
        CYG_ASSERT(mte-&cyg_mtab[0] < CYGNUM_FILEIO_MTAB_MAX, "Bad mount point");
        FILEIO_MUTEX_UNLOCK( mtab_lock[mte-&cyg_mtab[0]] );
    }
}

//==========================================================================
// Timestamp support
// This provides access to the current time/date, expressed as a
// time_t.  It uses a number of mechanisms to do this, selecting
// whichever is available in the current configuration.

__externC time_t cyg_timestamp()
{
#if defined(CYGPKG_IO_WALLCLOCK)

    // First, try to get the time from the wallclock device.
    
    return (time_t) Cyg_WallClock::wallclock->get_current_time();

#elif defined(CYGINT_ISO_POSIX_TIMERS)

    // If POSIX is present, use the current value of the realtime
    // clock.
    
    struct timespec tp;

    clock_gettime( CLOCK_REALTIME, &tp );

    return (time_t) tp.tv_sec;
    
#elif defined(CYGPKG_KERNEL) 

    // If all else fails, get the current realtime clock value and
    // convert it to seconds ourself.
    
    static struct Cyg_Clock::converter sec_converter;
    static cyg_bool initialized = false;
    cyg_tick_count ticks;
    
    if( !initialized )
    {
        Cyg_Clock::real_time_clock->get_clock_to_other_converter( 1000000000, &sec_converter );
        initialized = true;
    }

    ticks = Cyg_Clock::real_time_clock->current_value();
    
    return (time_t) Cyg_Clock::convert( ticks, &sec_converter );
#else    
    /* No clock support at all. */
    return (time_t) 0;
#endif    
    
}

//==========================================================================
// Default functions

__externC int cyg_fileio_enosys() { return ENOSYS; }
__externC int cyg_fileio_erofs() { return EROFS; }
__externC int cyg_fileio_enoerr() { return ENOERR; }
__externC int cyg_fileio_enotdir() { return ENOTDIR; }

__externC cyg_bool cyg_fileio_seltrue (struct CYG_FILE_TAG *fp, int which, CYG_ADDRWORD info)
{ return 1; }

// -------------------------------------------------------------------------
// EOF misc.cxx
