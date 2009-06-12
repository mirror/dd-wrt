//==========================================================================
//
//        console.c
//
//        Initial device I/O tests
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
// Date:          1999-02-05
// Description:   Minimal testing of "console" I/O
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>
#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#include <cyg/io/io.h>
#include <cyg/io/ttyio.h>
#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>
#ifdef CYGFUN_KERNEL_API_C
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>
#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
unsigned char stack[STACK_SIZE];
cyg_thread thread_data;
cyg_handle_t thread_handle;
#endif

static void
dump_buf_with_offset(unsigned char *p, 
                     int s, 
                     unsigned char *base)
{
    int i, c;
    if ((unsigned int)s > (unsigned int)p) {
        s = (unsigned int)s - (unsigned int)p;
    }
    while (s > 0) {
        if (base) {
            diag_printf("%08X: ", (int)p - (int)base);
        } else {
            diag_printf("%08X: ", p);
        }
        for (i = 0;  i < 16;  i++) {
            if (i < s) {
                diag_printf("%02X", p[i] & 0xFF);
            } else {
                diag_printf("  ");
            }
            if ((i % 2) == 1) diag_printf(" ");
            if ((i % 8) == 7) diag_printf(" ");
        }
        diag_printf(" |");
        for (i = 0;  i < 16;  i++) {
            if (i < s) {
                c = p[i] & 0xFF;
                if ((c < 0x20) || (c >= 0x7F)) c = '.';
            } else {
                c = ' ';
            }
            diag_printf("%c", c);
        }
        diag_printf("|\n");
        s -= 16;
        p += 16;
    }
}

static void
dump_buf(unsigned char *p, int s)
{
   dump_buf_with_offset(p, s, 0);
}

void
hang(void)
{
    while (true) ;
}

static int
strlen(char *c)
{
    int l = 0;
    while (*c++) l++;
    return l;
}

void
console_test( CYG_ADDRWORD x )
{
    Cyg_ErrNo res;
    cyg_io_handle_t handle;
    char msg[] = "This is a test\n";
    int msglen = sizeof(msg)-1;
    char in_msg[80];
    int in_msglen = sizeof(in_msg)-1;
    cyg_serial_info_t serial_info;
    cyg_tty_info_t tty_info;
    char short_msg[] = "This is a short message\n";
    char long_msg[] = "This is a longer message 0123456789abcdefghijklmnopqrstuvwxyz\n";
    char filler[] = "          ";
    char prompt[] = "\nPlease enter some data: ";
    int i, len;

    res = cyg_io_lookup(CYGDAT_IO_SERIAL_TTY_CONSOLE, &handle);
    if (res != ENOERR) {
        diag_printf("Can't lookup - DEVIO error: %d\n", res);
        return;
    }
    len = sizeof(serial_info);
    res = cyg_io_get_config(handle, CYG_IO_GET_CONFIG_SERIAL_INFO, &serial_info, &len);
    if (res != ENOERR) {
        diag_printf("Can't get serial config - DEVIO error: %d\n", res);
hang();
        return;
    }
    len = sizeof(tty_info);
    res = cyg_io_get_config(handle, CYG_IO_GET_CONFIG_TTY_INFO, &tty_info, &len);
    if (res != ENOERR) {
        diag_printf("Can't get tty config - DEVIO error: %d\n", res);
hang();
        return;
    }
    diag_printf("Config - baud: %d, stop: %d, parity: %d, out flags: %x, in flags: %x\n", 
                serial_info.baud, serial_info.stop, serial_info.parity,
                tty_info.tty_out_flags, tty_info.tty_in_flags);
    len = sizeof(serial_info);
    res = cyg_io_set_config(handle, CYG_IO_SET_CONFIG_SERIAL_INFO, &serial_info, &len);
    if (res != ENOERR) {
        diag_printf("Can't set serial config - DEVIO error: %d\n", res);
hang();
        return;
    }
    len = sizeof(tty_info);
    res = cyg_io_set_config(handle, CYG_IO_SET_CONFIG_TTY_INFO, &tty_info, &len);
    if (res != ENOERR) {
        diag_printf("Can't set tty config - DEVIO error: %d\n", res);
hang();
        return;
    }
    msglen = strlen(msg);
    res = cyg_io_write(handle, msg, &msglen);
    if (res != ENOERR) {
        diag_printf("Can't write data - DEVIO error: %d\n", res);
hang();
        return;
    }
    for (i = 0;  i < 10;  i++) {
        len = strlen(short_msg);
        res = cyg_io_write(handle, short_msg, &len);
        if (res != ENOERR) {
            diag_printf("Can't write [short] data - DEVIO error: %d\n", res);
            hang();
            return;
        }
    }
    for (i = 0;  i < 100;  i++) {
        len = (i % 10) + 1;
        cyg_io_write(handle, filler, &len);
        len = strlen(long_msg);
        res = cyg_io_write(handle, long_msg, &len);
        if (res != ENOERR) {
            diag_printf("Can't write [long] data - DEVIO error: %d\n", res);
            hang();
            return;
        }
    }
    len = strlen(prompt);
    cyg_io_write(handle, prompt, &len);
    res = cyg_io_read(handle, in_msg, &in_msglen);
    if (res != ENOERR) {
        diag_printf("Can't read data - DEVIO error: %d\n", res);
hang();
        return;
    }
    diag_printf("Read %d bytes\n", in_msglen);
    dump_buf(in_msg, in_msglen);
    CYG_TEST_PASS_FINISH("Console I/O test OK");
}

void
cyg_start(void)
{
    CYG_TEST_INIT();
#ifdef CYGFUN_KERNEL_API_C
    cyg_thread_create(10,                   // Priority - just a number
                      (cyg_thread_entry_t*)console_test,         // entry
                      0,                    // 
                      "console_thread",     // Name
                      &stack[0],            // Stack
                      STACK_SIZE,           // Size
                      &thread_handle,       // Handle
                      &thread_data          // Thread data structure
        );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
#else
    console_test();
#endif
}
// EOF console.c
