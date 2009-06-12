//==========================================================================
//
//        dl.c
//
//        ARM PID7 eval board FLASH program tool
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1998-11-18
// Description:   Tool used for simple handshake downloads.
//####DESCRIPTIONEND####

#include <stdio.h>

#define SYNC_COUNT 63

int
main(int argc, char *argv[])
{
    int c, count, j;
    char cout, cin;
    FILE *in, *log;
    if ((in = fopen(argv[1], "r")) == (FILE *)NULL) {
        fprintf(stderr, "Can't open '%s'\n", argv[1]);
        exit(1);
    }
    if ((log = fopen("/tmp/dl.log", "w")) == (FILE *)NULL) {
        fprintf(stderr, "Can't open log file\n");
        exit(1);
    }
    fprintf(stderr, "Downloading '%s'\n", argv[1]);
    count = 0; j = 0;
    write(1, ">", 1);  // Magic start
    while ((c = fgetc(in)) != EOF) {
        cout = c;
        write(1, &cout, 1);
        if (++j == SYNC_COUNT) {
            read(0, &cin, 1);
            if (cin != cout) {
                fprintf(stderr, "Sync problem - in: %x, out: %x, byte: %x\n", cin, cout, count);
                fprintf(log, "Sync problem - in: %x, out: %x, byte: %x\n", cin, cout, count);
                fflush(log);
                break;
            }
            j = 0;
        }
        count++;
        if ((count % 256) == 255) fprintf(stderr, "%08X\n", count);
    }
    sleep(2);
    write(1, ":", 1);
    fclose(log);
    exit(0);
}
