//==========================================================================
//
//      file.cxx
//
//      Fileio file operations
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
// Purpose:             Fileio file operations
// Description:         These are the functions that operate on files as a whole,
//                      such as open(), creat(), mkdir() etc.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/isoinfra.h>

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <string.h>                    // string functions
#include <dirent.h>
#include <stdio.h>                     // stdin, stdout, stderr

#include "fio.h"                       // Private header

//==========================================================================
// Implement filesystem locking protocol. 

#define LOCK_FS( _mte_ )  {                             \
   CYG_ASSERT(_mte_ != NULL, "Bad mount table entry");  \
   CYG_ASSERT(_mte_->fs != NULL, "Bad mount filesystem entry");  \
   cyg_fs_lock( _mte_, (_mte_)->fs->syncmode);          \
}

#define UNLOCK_FS( _mte_ ) cyg_fs_unlock( _mte_, (_mte_)->fs->syncmode)

//==========================================================================
// A local strcpy clone that returns a pointer to the end of the copied
// string, not the beginning.

static char *my_strcpy( char *s1, const char *s2 )
{
    while( (*s1++ = *s2++) != 0);
    return s1-1;
}

//==========================================================================
// Compare a pathname fragment with an element in a pathname. This
// deals with zero or separator termination and avoids substring
// matches.

static int pathcmp( const char *path, const char *name )
{
    while( *path == *name && *path != '\0' )
        path++, name++;
    if( *name != '\0' ) return false;
    if( *path == '/' || *path == '\0' ) return true;
    return false;
}

//==========================================================================
// CWD support

#ifdef CYGPKG_IO_FILEIO_TRACK_CWD

// buffer for storing CWD path
static char cwd[PATH_MAX];
static size_t cwd_size = 0;


static void update_cwd( cyg_mtab_entry *mte, cyg_dir dir, const char *path )
{
    char *p = cwd;

    if( mte != cyg_cdir_mtab_entry || dir != cyg_cdir_dir )
    {
        // Here, the path is relative to the root of the filesystem,
        // or in a totally new filesystem, initialize the cwd with the
        // mount point name of the filesystem.

        p = my_strcpy( p, mte->name );
    }
    else p = cwd+cwd_size;

    // We must now copy the path into the cwd buffer while dealing
    // with any "." and ".."  components textually.

    while( *path != '\0' )
    {
        // skip any stray directory separators.
        if( *path == '/' ) path++;

        // Look for a "." entry and just ignore it.
        if( pathcmp( path, "." ) )
        {
            path++;
            continue;
        }

        // Look for a ".." entry. If found, chew off the last cwd
        // entry.
        if( pathcmp( path, ".." ) )
        {
            while( *(--p) != '/' );     // back up to last '/'
            path += 2;                  // skip "..".
            continue;
        }

        // Otherwise just copy the name fragment over to the cwd.

        if( *(p-1) != '/' )
            *p++ = '/';        // add a directory separator
        while( *path != '/' && *path != '\0' )
            *p++ = *path++;

    }

    // Zero terminate the cwd.
    *p = '\0';
    
    // update size
    cwd_size = p-cwd;
}

#else

#ifdef CYGPKG_KERNEL
static Cyg_Mutex getcwd_lock CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);
#endif

#endif


//==========================================================================
// Open a file

__externC int open( const char *path, int oflag, ... )
{
    FILEIO_ENTRY();

    // we want to be sure we pull in stdin/out/err, so they can be
    // assigned to fds 0, 1 and 2
#ifdef CYGINT_ISO_STDIO_STREAMS
    CYG_REFERENCE_OBJECT(stdin);
    CYG_REFERENCE_OBJECT(stdout);
    CYG_REFERENCE_OBJECT(stderr);
#endif

    CYG_CANCELLATION_POINT;

    int ret = 0;
    int fd;
    cyg_file *file;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    // At least one of O_RDONLY, O_WRONLY, O_RDWR must be provided
    if( (oflag & O_RDWR) == 0 )
        FILEIO_RETURN(EINVAL);

    fd = cyg_fd_alloc(0);

    if( fd < 0 )
        FILEIO_RETURN(EMFILE);
    
    file = cyg_file_alloc();

    if( file == NULL )
    {
        cyg_fd_free(fd);
        FILEIO_RETURN(ENFILE);
    }
    
    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
    {
        cyg_fd_free(fd);
        cyg_file_free(file);
        FILEIO_RETURN(ENOENT);
    }

    LOCK_FS( mte );
    
    ret = mte->fs->open( mte, dir, name, oflag, file );
    
    UNLOCK_FS( mte );
    
    if( 0 != ret )
    {
        cyg_fd_free(fd);
        cyg_file_free(file);
        FILEIO_RETURN(ret);
    }

    file->f_mte = mte;
    file->f_syncmode = mte->fs->syncmode;
    
    cyg_fd_assign( fd, file );

    FILEIO_RETURN_VALUE(fd);
}

//==========================================================================
// create a file

__externC int creat( const char *path, mode_t mode )
{
    return open( path, O_WRONLY | O_CREAT | O_TRUNC, mode );
}


//==========================================================================
// Unlink/remove a file

__externC int unlink( const char *path )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
        
    ret = mte->fs->unlink( mte, dir, name );
    
    UNLOCK_FS( mte );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// Make a directory

__externC int mkdir( const char *path, mode_t mode )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    mode=mode;
    
    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->mkdir( mte, dir, name );
    
    UNLOCK_FS( mte );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// Remove a directory

__externC int rmdir( const char *path )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->rmdir( mte, dir, name );
    
    UNLOCK_FS( mte );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// Rename a file

__externC int rename( const char *path1, const char *path2 ) __THROW
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte1 = cyg_cdir_mtab_entry;
    cyg_mtab_entry *mte2 = cyg_cdir_mtab_entry;
    cyg_dir dir1 = cyg_cdir_dir;
    cyg_dir dir2 = cyg_cdir_dir;
    const char *name1 = path1;
    const char *name2 = path2;

    ret = cyg_mtab_lookup( &dir1, &name1, &mte1 );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    ret = cyg_mtab_lookup( &dir2, &name2, &mte2 );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    // Cannot rename between different filesystems
    if( mte1 != mte2 )
        FILEIO_RETURN(EXDEV);

    LOCK_FS( mte1 );
    
    ret = mte1->fs->rename( mte1, dir1, name1, dir2, name2 );

    UNLOCK_FS( mte1 );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// Create a link from an existing file (path1) to a new one (path2)

__externC int link( const char *path1, const char *path2 )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte1 = cyg_cdir_mtab_entry;
    cyg_mtab_entry *mte2 = cyg_cdir_mtab_entry;
    cyg_dir dir1 = cyg_cdir_dir;
    cyg_dir dir2 = cyg_cdir_dir;
    const char *name1 = path1;
    const char *name2 = path2;

    ret = cyg_mtab_lookup( &dir1, &name1, &mte1 );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    ret = cyg_mtab_lookup( &dir2, &name2, &mte2 );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    // Cannot hard-link between different filesystems
    if( mte1 != mte2 )
        FILEIO_RETURN(EXDEV);

    LOCK_FS( mte1 );
    
    ret = mte1->fs->link( mte1, dir1, name1, dir2, name2, CYG_FSLINK_HARD );

    UNLOCK_FS( mte1 );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// Change current directory

__externC int chdir( const char *path )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    cyg_dir newdir;
    const char *name = path;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS(mte);
    
    ret = mte->fs->chdir( mte, dir, name, &newdir );

    UNLOCK_FS(mte);
    
    if( 0 != ret )
        FILEIO_RETURN(ret);

#ifdef CYGPKG_IO_FILEIO_TRACK_CWD
    update_cwd( mte, dir, name );
#endif
    
    if( cyg_cdir_mtab_entry != NULL && cyg_cdir_dir != CYG_DIR_NULL )
    {
        // Now detach from current cyg_cdir. We call the current directory's
        // chdir routine with a NULL dir_out pointer.

        LOCK_FS(cyg_cdir_mtab_entry);

        ret = cyg_cdir_mtab_entry->fs->chdir( cyg_cdir_mtab_entry, cyg_cdir_dir, NULL, NULL );
    
        UNLOCK_FS(cyg_cdir_mtab_entry);

        // We really shouldn't get an error here.
        if( 0 != ret )
            FILEIO_RETURN(ret);
    }
    
    cyg_cdir_mtab_entry = mte;
    cyg_cdir_dir = newdir;
    
    FILEIO_RETURN(ENOERR);
}

//==========================================================================
// Get file statistics

__externC int stat( const char *path, struct stat *buf )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->stat( mte, dir, name, buf );
    
    UNLOCK_FS( mte );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// Get file configurable pathname variables

__externC long pathconf( const char *path, int vname )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    struct cyg_pathconf_info info;

    info.name = vname;
    info.value = 0;
        
    LOCK_FS( mte );
    
    ret = mte->fs->getinfo( mte, dir, name,
                            FS_INFO_CONF, (char *)&info, sizeof(info) );
    
    UNLOCK_FS( mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ret);

    FILEIO_RETURN_VALUE(info.value);
}

//==========================================================================
// Sync filesystem without unmounting

__externC int cyg_fs_fssync( const char *path )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->setinfo( mte, dir, name, FS_INFO_SYNC, NULL, 0 );
    
    UNLOCK_FS( mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ret);

    FILEIO_RETURN_VALUE(ENOERR);
}

//==========================================================================
// Set file attributes

__externC int cyg_fs_set_attrib( const char *fname, 
                                 const cyg_fs_attrib_t new_attrib )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = fname;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->setinfo( mte, dir, name,
                            FS_INFO_ATTRIB, 
                            (char *)&new_attrib, sizeof(new_attrib) );
    
    UNLOCK_FS( mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ret);

    FILEIO_RETURN(ENOERR);
}

//==========================================================================
// Get file attributes

__externC int cyg_fs_get_attrib( const char *fname, 
                                 cyg_fs_attrib_t * const file_attrib )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = fname;

    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->getinfo( mte, dir, name,
                            FS_INFO_ATTRIB, 
                            (char *)file_attrib, sizeof(*file_attrib) );
    
    UNLOCK_FS( mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ret);

    FILEIO_RETURN(ENOERR);
}

//==========================================================================
// Access() function.
// This simply piggybacks onto stat().

extern int 	access(const char *path, int amode)
{
    FILEIO_ENTRY();

    int ret;
    struct stat buf;
    
    ret = stat( path, &buf );

    // Translate not found into EACCES if the F_OK bit is
    // set.
    if( (amode & F_OK) && (ret < 0) && (errno == ENOENT) )
        FILEIO_RETURN(EACCES);

    // All other errors go straight back to the user.
    if( ret < 0 )
        FILEIO_RETURN_VALUE(ret);

    // At present we do not have any access modes, so there is nothing
    // to test.  Just return success for all access modes.
    
    FILEIO_RETURN(ENOERR);
}

//==========================================================================
// getcwd()

__externC char *getcwd( char *buf, size_t size )
{
    FILEIO_ENTRY();

    int err = ENOERR;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    cyg_getcwd_info info;

    if( size == 0 )
    {
        errno = EINVAL;
        FILEIO_RETURN_VALUE(NULL);        
    }
        
    info.buf = buf;
    info.size = size;

    LOCK_FS( mte );
    
    err = mte->fs->getinfo( mte, dir, "",
                            FS_INFO_GETCWD, (char *)&info, sizeof(info) );
    
    UNLOCK_FS( mte );

    if( err == ENOERR )
        FILEIO_RETURN_VALUE(buf);

    // Attempting to use filesystem support for getcwd() has
    // failed.

#ifdef CYGPKG_IO_FILEIO_TRACK_CWD

    // If this option is set, the current directory path has been
    // tracked in chdir(). Just report that value here.
    
    if( size < cwd_size+1 )
    {
        errno = ERANGE;
        FILEIO_RETURN_VALUE(NULL);        
    }

    char *p = my_strcpy( buf, cwd );
    *p = '\0';

#else

    // As a fallback we try to use ".." entries in the directory tree
    // to climb back up to the root.  Because we cannot assume that
    // any filesystem can handle more than one directory pointer we
    // have to do the climbing textually, by manufacturing a path name
    // consisting of ".."s. At each level we then scan the parent
    // directory looking for the entry for the lower level directory
    // by matching st_ino values. This is not guaranteed to work at
    // all since there is no requirement on filesystems to support "."
    // and "..", or for them to report distinct inode values in
    // stat().

    static char ddbuf[PATH_MAX];
    char *p = buf+size-1;
    int ddbufpos;

    // Claim lock to serialize use of ddbuf.
    FILEIO_MUTEX_LOCK(getcwd_lock);

    // Initialize ddbuf with ".".
    ddbuf[0] = '.';
    ddbuf[1] = '\0';
    ddbufpos = 1;

    // Start result buffer with a zero terminator. We accumulate the
    // path name in the top end of the result buffer.
    *p = '\0';
    
    for(;;)
    {
        struct stat sdbuf;
        struct stat sddbuf;

        // Get status for "." and "..". If the filesystem does not
        // support these, then this whole function will fail here.
        
        err = stat( ddbuf, &sdbuf );
        if( err < 0 ) break;

        ddbuf[ddbufpos++] = '/';
        ddbuf[ddbufpos++] = '.';
        ddbuf[ddbufpos++] = '.';
        ddbuf[ddbufpos] = '\0';
        
        err = stat( ddbuf, &sddbuf );
        if( err < 0 ) break;

        // See whether we are at the root. This will be true when
        // the inode numbers of "." and ".." are the same.
        if( sdbuf.st_ino == sddbuf.st_ino )
            break;

        // We now need to find an entry in the ".." directory that
        // matches the inode number of ".".

        struct dirent de;
        DIR *d = opendir( ddbuf );
        if( d == NULL )
        {
            err = -1;
            break;
        }
        
        for(;;)
        {
            struct dirent *res;
            struct stat objstat;
            int i;
            
            err = readdir_r( d, &de, &res );
            if( err < 0 || res == NULL ) break;

            // Skip "." and ".." entries.
            if( pathcmp( de.d_name, "." ) || pathcmp( de.d_name, ".." ) )
                continue;
            
            // Tack the name of the directory entry on to the ddbuf
            // and stat the object.
            
            ddbuf[ddbufpos] = '/';
            for( i = 0; de.d_name[i] != '\0'; i++ )
                ddbuf[ddbufpos+i+1] = de.d_name[i];
            ddbuf[ddbufpos+i+1] = '\0';

            // Take a look at it
            err = stat( ddbuf, &objstat );
            if( err < 0 ) break;

            // Cast out directories
            if( !S_ISDIR(objstat.st_mode) )
                continue;

            // We have a directory. Compare its inode with that of "."
            // and if they are the same, we have found our entry.

            if( sdbuf.st_ino == objstat.st_ino )
                break;
        }

        ddbuf[ddbufpos] = '\0'; // reterminate ddbuf
        
        closedir( d );

        // Halt on any errors.
        if( err < 0 )
            break;

        // Here de contains the name of the directory entry in ".."
        // that has the same inode as ".". Add the name to the path we
        // are accumulating in the buffer.

        char *q = de.d_name;
        while( *q != '\0' ) q++;        // skip to end of name

        do
        {
            *--p = *--q;
        } while( q != de.d_name );

        *--p = '/';                     // add a separator
    }

    // We have finished using ddbuf now.
    FILEIO_MUTEX_UNLOCK(getcwd_lock);
    
    if( err < 0 )
        FILEIO_RETURN_VALUE(NULL);

    // We have the directory path in the top end of the buffer.  Add
    // the mount point name at the beginning and copy the rest of the
    // name down.

    char *bp = buf;
    
    bp = my_strcpy( bp, mte->name );

    // Sort out the separators between the mount name and the
    // pathname.  This is a bit messy since we have to deal with mount
    // names of both "/" and "/foo" and pathnames that start with '/'
    // or are empty.
    if( *(bp-1) != '/' && *p != '\0' ) *bp++ = '/';
    if( *p == '/' ) p++;

    // Now copy the path over.
    while( *p )
        *bp++ = *p++;

    *bp = '\0';                         // Terminate the string

    // All done!
    
#endif
    
    FILEIO_RETURN_VALUE(buf);
}

//==========================================================================
// FS get info.

__externC int cyg_fs_getinfo( const char *path, int key, void *buf, int len )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;
    
    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->getinfo( mte, dir, name, key, buf, len );
    
    UNLOCK_FS( mte );
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// FS set info.

__externC int cyg_fs_setinfo( const char *path, int key, void *buf, int len )
{
    FILEIO_ENTRY();
    
    int ret = 0;
    cyg_mtab_entry *mte = cyg_cdir_mtab_entry;
    cyg_dir dir = cyg_cdir_dir;
    const char *name = path;
    
    ret = cyg_mtab_lookup( &dir, &name, &mte );
    
    if( 0 != ret )
        FILEIO_RETURN(ENOENT);

    LOCK_FS( mte );
    
    ret = mte->fs->setinfo( mte, dir, name, key, buf, len );
    
    UNLOCK_FS( mte );
    
    FILEIO_RETURN(ret);
}

// -------------------------------------------------------------------------
// EOF file.cxx
