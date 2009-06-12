//==========================================================================
//
//      fseek1.c
//
//      Test fseek on a filesystem
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Andrew Lunn
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
// Author(s):           asl
// Contributors:        asl
// Date:                2004-03-29
// Purpose:             Test fseek on a filesystem
// Description:         This test uses the ramfs to check out the fseek
//                      operation on a filesystem.
//                      
//                      
//                      
//                      
//                      
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <cyg/fileio/fileio.h>
#include <cyg/io/flash.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>            // HAL polled output
//==========================================================================

#define SHOW_RESULT( _fn, _res ) \
diag_printf("FAIL: " #_fn "() returned %d %s\n", _res, _res<0?strerror(errno):"");

//==========================================================================

char buf[1024];
char buf1[1024];

//==========================================================================
// main

int main( int argc, char **argv )
{
    int err;
    FILE *stream;
    long pos;
    int i;
    
    CYG_TEST_INIT();

    // --------------------------------------------------------------

    CYG_TEST_INFO("mount /");    
    err = mount( CYGDAT_IO_FLASH_BLOCK_DEVICE_NAME_1, "/", "jffs2" );

    if( err < 0 ) SHOW_RESULT( mount, err );    
    
    CYG_TEST_INFO("creating /fseek");    
    stream = fopen("/fseek","w+");
    if (!stream) {
      diag_printf("FAIL: fopen() returned NULL, %s\n", strerror(errno));
      CYG_TEST_FINISH("done");          \
    }

    /* Write a buffer full of cyclic numbers */
    for (i = 0; i < sizeof(buf); i++) {
      buf[i] = i % 256;
    }
    
    CYG_TEST_INFO("writing test pattern");    
    err=fwrite(buf,sizeof(buf), 1, stream);
    if ( err < 0 ) SHOW_RESULT( fwrite, err );
    
    /* The current position should be the same size as the buffer */
    pos = ftell(stream);
    
    if (pos < 0) SHOW_RESULT( ftell, pos );
    if (pos != sizeof(buf))
      diag_printf("<FAIL>: ftell is not telling the truth.");
    
    CYG_TEST_INFO("fseek()ing to beginning and writing");    

    /* Seek back to the beginning of the file */
    err = fseek(stream, 0, SEEK_SET);
    if ( err < 0 ) SHOW_RESULT( fseek, err );

    pos = ftell(stream);
    
    if (pos < 0) SHOW_RESULT( ftell, pos );
    if (pos != 0) CYG_TEST_FAIL("ftell is not telling the truth");

    /* Write 4 zeros to the beginning of the file */
    for (i = 0; i < 4; i++) {
      buf[i] = 0;
    }

    err = fwrite(buf, 4, 1, stream);
    if ( err < 0 ) SHOW_RESULT( fwrite, err );

    /* Check the pointer is at 4 */
    pos = ftell(stream);
    
    if (pos < 0) SHOW_RESULT( ftell, pos );
    if (pos != 4)  CYG_TEST_FAIL("ftell is not telling the truth");

    CYG_TEST_INFO("closing file");
    
    /* Close the file, open it up again and read it back */
    err = fclose(stream);
    if (err != 0) SHOW_RESULT( fclose, err );

    CYG_TEST_INFO("open file /fseek");
    stream = fopen("/fseek", "r+");
    if (!stream) {
      diag_printf("<FAIL>: fopen() returned NULL, %s\n", strerror(errno));
    }

    err = fread(buf1,sizeof(buf1),1, stream);
    if (err != 1) SHOW_RESULT( fread, err );
    
    CYG_TEST_INFO("Comparing contents");
    if (memcmp(buf, buf1, sizeof(buf1))) {
      CYG_TEST_FAIL("File contents inconsistent");
    }
    
    CYG_TEST_INFO("closing file");

    err = fclose(stream);
    if (err != 0) SHOW_RESULT( fclose, err );

    CYG_TEST_INFO("open file /fseek");
    stream = fopen("/fseek", "r+");
    if (!stream) {
      diag_printf("<FAIL>: fopen() returned NULL, %s\n", strerror(errno));
    }

    CYG_TEST_INFO("fseek()ing past the end to create a hole");
    /* Seek 1K after the end of the file */
    err = fseek(stream, sizeof(buf), SEEK_END);
    if ( err < 0 ) SHOW_RESULT( fseek, err );

    pos = ftell(stream);
    
    if (pos < 0) SHOW_RESULT( ftell, pos );
    if (pos != (2*sizeof(buf))) CYG_TEST_FAIL("ftell is not telling the truth");
    
    CYG_TEST_INFO("writing test pattern");    
    err=fwrite(buf,sizeof(buf), 1, stream);
    if ( err < 0 ) SHOW_RESULT( fwrite, err );
    
    pos = ftell(stream);
    
    if (pos < 0) SHOW_RESULT( ftell, pos );
    if (pos != (3*sizeof(buf))) CYG_TEST_FAIL("ftell is not telling the truth");

    CYG_TEST_INFO("closing file");
    err = fclose(stream);
    if (err != 0) SHOW_RESULT( fclose, err );

    CYG_TEST_INFO("open file /fseek");
    stream = fopen("/fseek", "r+");
    if (!stream) {
      diag_printf("<FAIL>: fopen() returned NULL, %s\n", strerror(errno));
    }
    
    err = fread(buf1,sizeof(buf1),1, stream);
    if (err != 1) SHOW_RESULT( fread, err );
    
    CYG_TEST_INFO("Comparing contents");
    if (memcmp(buf, buf1, sizeof(buf1))) {
      CYG_TEST_FAIL("File contents inconsistent");
    }

    err = fread(buf1,sizeof(buf1),1, stream);
    if (err != 1) SHOW_RESULT( fread, err );
    
    for (i = 0; i< sizeof(buf); i++) {
      if (buf1[i] != 0)
        CYG_TEST_FAIL("Hole does not contain zeros");
    }
    
    err = fread(buf1,sizeof(buf1),1, stream);
    if (err != 1) SHOW_RESULT( fread, err );
    
    if (memcmp(buf, buf1, sizeof(buf1))) {
      CYG_TEST_FAIL("File contents inconsistent");
    }

    CYG_TEST_INFO("closing file");

    /* Close the file */
    err = fclose(stream);
    if (err != 0) SHOW_RESULT( fclose, err );
    
    CYG_TEST_INFO("umount /");    
    err = umount( "/" );
    if( err < 0 ) SHOW_RESULT( umount, err );    
    
    CYG_TEST_PASS_FINISH("fseek1");
}
