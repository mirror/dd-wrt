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
// Contributors:        nickg, richard.panton@3glab.com, jlarmour
// Date:                2000-05-25
// Purpose:             Test fileio system
// Description:         This test uses the testfs to check out the initialization
//                      and basic operation of the fileio system
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/system.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include <cyg/fileio/fileio.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>            // HAL polled output

// Test ROMFS data. Two example data files are generated so that
// the test will work on both big-endian and little-endian targets.
#if (CYG_BYTEORDER == CYG_LSBFIRST)
# include <cyg/romfs/testromfs_le.h>
#else
# include <cyg/romfs/testromfs_be.h>
#endif

//==========================================================================

MTAB_ENTRY( romfs_mte1,
                   "/",
                   "romfs",
                   "",
                   (CYG_ADDRWORD) &filedata[0] );


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

#ifndef CYGINT_ISO_STRING_STRFUNCS

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

#ifdef CYGPKG_FS_RAM
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
#endif

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
    char address[16];

    CYG_TEST_INIT();

    // --------------------------------------------------------------

    diag_printf("<INFO>: ROMFS root follows\n");
    listdir( "/", true );

    diag_printf("<INFO>: cd /etc\n" );
    err = chdir( "/etc" );
    if ( err < 0 ) {
        SHOW_RESULT( chdir, err );
        CYG_TEST_FAIL_FINISH("fileio1");
    }

    diag_printf("<INFO>: ROMFS list of '' follows\n");
    listdir( "", true );

    diag_printf("<INFO>: ROMFS list of /etc follows\n");
    listdir( "/etc", true );

    diag_printf("<INFO>: ROMFS list of . follows\n");
    listdir( ".", true );
    
#ifdef CYGPKG_FS_RAM
    err = mount( "", "/var", "ramfs" );
    if( err < 0 ) SHOW_RESULT( mount, err );

    copyfile( "/etc/passwd", "/var/passwd_copy" );

    comparefiles( "/etc/passwd", "/var/passwd_copy" );
#endif
    
    diag_printf("<INFO>: ROMFS list of / follows\n");
#ifdef CYGPKG_FS_RAM
    diag_printf("<INFO>: Note that /var now gives stat() info for RAMFS\n");
#endif
    listdir( "/", true );

    diag_printf("<INFO>: Mount ROMFS again onto /mnt\n");
    sprintf( address, "%p", (void*)&filedata[0] );
    err = mount( address, "/mnt", "romfs" );
    if( err < 0 ) SHOW_RESULT( mount, err );    

    comparefiles( "/etc/passwd", "/mnt/etc/passwd" );


    err = mkdir( "/foo", 0 );
    CHKFAIL_TYPE( mkdir, err, EROFS );

    err = rename( "/var", "/tmp" );	// RAMFS is mounted here
#ifdef CYGPKG_FS_RAM
    CHKFAIL_TYPE( rename, err, EXDEV );
#else
    CHKFAIL_TYPE( rename, err, EROFS );
#endif

    err = rename( "/var/passwd_copy", "/mnt/etc/passwd_copy" );
    CHKFAIL_TYPE( rename, err, EXDEV );

    err = rename( "/etc", "/tmp" );
    CHKFAIL_TYPE( rename, err, EROFS );

    diag_printf("<INFO>: cd /etc\n");
    err = chdir( "/etc" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    err = chdir( "/mnt/etc" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    listdir( ".", true );

    diag_printf("<INFO>: unlink /tmp\n");        
    err = unlink( "/tmp" );
    CHKFAIL_TYPE( unlink, err, EROFS );

    diag_printf("<INFO>: mount random area\n");
    sprintf(address, "%p", (void*)(&filedata[0] + 0x100));
    err = mount( address, "/tmp", "romfs" );
    CHKFAIL_TYPE( mount, err, ENOENT );

    err = umount( "/mnt" );
    if( err < 0 ) SHOW_RESULT( umount, err );    

    err = umount( "/var" );
#ifdef CYGPKG_FS_RAM
    if( err < 0 ) SHOW_RESULT( umount, err );    
#else
    CHKFAIL_TYPE( umount, err, EINVAL );
#endif

    err = umount( "/" );
    if( err < 0 ) SHOW_RESULT( umount, err );    


    CYG_TEST_PASS_FINISH("fileio1");
}

// -------------------------------------------------------------------------
// EOF fileio1.c
