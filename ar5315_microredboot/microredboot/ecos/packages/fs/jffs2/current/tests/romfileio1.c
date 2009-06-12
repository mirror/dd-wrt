//==========================================================================
//
//      fileio1.c
//
//      Test fileio system
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
// Contributors:        nickg, richard.panton@3glab.com
// Date:                2000-05-25
// Purpose:             Test fileio system
// Description:         This test uses the testfs to check out the initialization
//                      and basic operation of the fileio system
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/io/flash.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <cyg/fileio/fileio.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>            // HAL polled output

#include <pkgconf/fs_jffs2.h>	// Address of JFFS2

//==========================================================================

/* MTAB_ENTRY( jffs2_mte1,
                   "/",
                   "jffs2",
                   "",
                   (CYG_ADDRWORD) CYGNUM_FS_JFFS2_BASE_ADDRESS ); */


//==========================================================================

#define SHOW_RESULT( _fn, _res ) \
diag_printf("<FAIL>: " #_fn "() returned %d %s\n", _res, _res<0?strerror(errno):"");

#define CHKFAIL_TYPE( _fn, _res, _type ) { \
if ( _res != -1 ) \
    diag_printf("<FAIL>: " #_fn "() returned %d (expected -1)\n", _res); \
else if ( errno != _type ) \
    diag_printf("<FAIL>: " #_fn "() failed with errno %d (%s),\n    expected %d (%s)\n", errno, strerror(errno), _type, strerror(_type) ); \
}

//==========================================================================

#define IOSIZE  100

#define LONGNAME1       "long_file_name_that_should_take_up_more_than_one_directory_entry_1"
#define LONGNAME2       "long_file_name_that_should_take_up_more_than_one_directory_entry_2"


//==========================================================================

#ifndef CYGPKG_LIBC_STRING

char *strcat( char *s1, const char *s2 )
{
    char *s = s1;
    while( *s1 ) s1++;
    while( (*s1++ = *s2++) != 0);
    return s;
}

#endif

//==========================================================================

static void listdir( char *name, int statp )
{
    int err;
    DIR *dirp;
    
    diag_printf("<INFO>: reading directory %s\n",name);
    
    dirp = opendir( name );
    if( dirp == NULL ) SHOW_RESULT( opendir, -1 );

    for(;;)
    {
        struct dirent *entry = readdir( dirp );
        
        if( entry == NULL )
            break;

        diag_printf("<INFO>: entry %14s",entry->d_name);
        if( statp )
        {
            char fullname[PATH_MAX];
            struct stat sbuf;

            if( name[0] )
            {
                strcpy(fullname, name );
                if( !(name[0] == '/' && name[1] == 0 ) )
                    strcat(fullname, "/" );
            }
            else fullname[0] = 0;
            
            strcat(fullname, entry->d_name );
            
            err = stat( fullname, &sbuf );
            if( err < 0 )
            {
                if( errno == ENOSYS )
                    diag_printf(" <no status available>");
                else SHOW_RESULT( stat, err );
            }
            else
            {
                diag_printf(" [mode %08x ino %08x nlink %d size %d]",
                            sbuf.st_mode,sbuf.st_ino,sbuf.st_nlink,sbuf.st_size);
            }
        }

        diag_printf("\n");
    }

    err = closedir( dirp );
    if( err < 0 ) SHOW_RESULT( stat, err );
}

//==========================================================================

static void copyfile( char *name2, char *name1 )
{

    int err;
    char buf[IOSIZE];
    int fd1, fd2;
    ssize_t done, wrote;

    diag_printf("<INFO>: copy file %s -> %s\n",name2,name1);

    err = access( name1, F_OK );
    if( err < 0 && errno != EACCES ) SHOW_RESULT( access, err );

    err = access( name2, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );
    
    fd1 = open( name1, O_WRONLY|O_CREAT );
    if( fd1 < 0 ) SHOW_RESULT( open, fd1 );

    fd2 = open( name2, O_RDONLY );
    if( fd2 < 0 ) SHOW_RESULT( open, fd2 );
    
    for(;;)
    {
        done = read( fd2, buf, IOSIZE );
        if( done < 0 ) SHOW_RESULT( read, done );

        if( done == 0 ) break;

        wrote = write( fd1, buf, done );
        if( wrote != done ) SHOW_RESULT( write, wrote );

        if( wrote != done ) break;
    }

    err = close( fd1 );
    if( err < 0 ) SHOW_RESULT( close, err );

    err = close( fd2 );
    if( err < 0 ) SHOW_RESULT( close, err );
    
}

//==========================================================================

static void comparefiles( char *name2, char *name1 )
{
    int err;
    char buf1[IOSIZE];
    char buf2[IOSIZE];
    int fd1, fd2;
    ssize_t done1, done2;
    int i;

    diag_printf("<INFO>: compare files %s == %s\n",name2,name1);

    err = access( name1, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );

    err = access( name1, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );
    
    fd1 = open( name1, O_RDONLY );
    if( fd1 < 0 ) SHOW_RESULT( open, fd1 );

    fd2 = open( name2, O_RDONLY );
    if( fd2 < 0 ) SHOW_RESULT( open, fd2 );
    
    for(;;)
    {
        done1 = read( fd1, buf1, IOSIZE );
        if( done1 < 0 ) SHOW_RESULT( read, done1 );

        done2 = read( fd2, buf2, IOSIZE );
        if( done2 < 0 ) SHOW_RESULT( read, done2 );

        if( done1 != done2 )
            diag_printf("Files different sizes\n");
        
        if( done1 == 0 ) break;

        for( i = 0; i < done1; i++ )
            if( buf1[i] != buf2[i] )
            {
                diag_printf("buf1[%d](%02x) != buf1[%d](%02x)\n",i,buf1[i],i,buf2[i]);
                CYG_TEST_FAIL("Data in files not equal\n");
            }
    }

    err = close( fd1 );
    if( err < 0 ) SHOW_RESULT( close, err );

    err = close( fd2 );
    if( err < 0 ) SHOW_RESULT( close, err );
    
}

//==========================================================================
// main

int main( int argc, char **argv )
{
    int err;
    //    char address[16];

    CYG_TEST_INIT();

    // --------------------------------------------------------------

    err = mount( CYGDAT_IO_FLASH_BLOCK_DEVICE_NAME_1, "/", "jffs2" );
    if ( err != ENOERR ) SHOW_RESULT( chdir, err );
    
    diag_printf("<INFO>: JFFS2 root follows\n");
    diag_printf("<INFO>: Note that dev cannot be stat()ed\n");
    listdir( "/", true );

    diag_printf("<INFO>: cd /etc\n" );
    err = chdir( "/etc" );
    if ( err < 0 ) SHOW_RESULT( chdir, err );

    diag_printf("<INFO>: JFFS2 list of '' follows\n");
    listdir( "", true );

    diag_printf("<INFO>: JFFS2 list of /etc follows\n");
    listdir( "/etc", true );

    //diag_printf("<INFO>: JFFS2 list of . follows\n");
    //listdir( ".", true );
    
    err = mount( "", "/var", "ramfs" );
    if( err < 0 ) SHOW_RESULT( mount, err );    

    copyfile( "/etc/passwd", "/var/passwd_copy" );

    comparefiles( "/etc/passwd", "/var/passwd_copy" );
    
    diag_printf("<INFO>: JFFS2 list of / follows\n");
    diag_printf("<INFO>: Note that /var now gives stat() info for RAMFS\n");
    listdir( "/", true );

    diag_printf("<INFO>: Mount JFFS2 again onto /mnt\n");
    //sprintf( address, "%p", (void*)CYGNUM_FS_JFFS2_BASE_ADDRESS );
    err = mount( CYGDAT_IO_FLASH_BLOCK_DEVICE_NAME_1, "/mnt", "jffs2" );
    if( err < 0 ) SHOW_RESULT( mount, err );    

    comparefiles( "/etc/passwd", "/mnt/etc/passwd" );


    err = mkdir( "/foo", 0 );
    CHKFAIL_TYPE( mkdir, err, EROFS );

    err = rename( "/var", "/tmp" );	// RAMFS is mounted here
    CHKFAIL_TYPE( rename, err, EXDEV );

    err = rename( "/var/passwd_copy", "/mnt/etc/passwd_copy" );
    CHKFAIL_TYPE( rename, err, EXDEV );

    err = rename( "/etc", "/tmp" );
    CHKFAIL_TYPE( rename, err, EROFS );

    diag_printf("<INFO>: cd /etc\n");
    err = chdir( "/etc" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    err = chdir( "/mnt/etc" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    //listdir( ".", true );

    diag_printf("<INFO>: unlink /tmp\n");        
    err = unlink( "/tmp" );
    CHKFAIL_TYPE( unlink, err, EROFS );

    diag_printf("<INFO>: mount random area\n");
    //sprintf(address, "%p", (void*)(CYGNUM_FS_JFFS2_BASE_ADDRESS + 0x20000));
    err = mount( "", "/tmp", "jffs2" );
    SHOW_RESULT( mount, err );

    err = umount( "/mnt" );
    if( err < 0 ) SHOW_RESULT( umount, err );    

    err = umount( "/var" );
    if( err < 0 ) SHOW_RESULT( umount, err );    

    err = umount( "/" );
    if( err < 0 ) SHOW_RESULT( umount, err );    


    CYG_TEST_PASS_FINISH("fileio1");
}

// -------------------------------------------------------------------------
// EOF fileio1.c
