//=================================================================
//
//        usb_chmod.c
//
//        A utility to manipulate /proc/bus/usb access rights
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
// The Linux kernel allows raw access to USB devices via /proc/bus/usb.
// However, such access requires root privileges: this makes perfect
// sense for typical USB devices, but gets in the way of eCos USB
// testing. This utility runs suid and can be used to change the access
// rights to a specific and validated USB device.
//
// Author(s):     bartv
// Date:          2001-07-18
//####DESCRIPTIONEND####
//==========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// Note: this code is duplicated in usbhost.c. Any changes here
// should be propagated. For now the routine is too small to warrant
// a separate source file.
#define USB_ROOT        "/proc/bus/usb/"
#define PRODUCT_STRING  "Red Hat eCos USB test"

static int
usb_scan_devices(int* bus, int* dev)
{
    FILE*       devs_file       = fopen(USB_ROOT "devices", "r");
    int         current_bus     = -1;
    int         current_dev     = -1;
    int         ch;
    
    if (NULL == devs_file) {
        fprintf(stderr, "Error: unable to access " USB_ROOT "devices\n");
        return 0;
    }
    ch = getc(devs_file);
    while (EOF != ch) {
        if ('T' == ch) {
            if (2 !=fscanf(devs_file, ":  Bus=%d %*[^D\n]Dev#=%d", &current_bus, &current_dev)) { 
                current_bus = -1;
                current_dev = -1;
            }
        } else if ('S' == ch) {
            int start = 0, end = 0;
            if (EOF != fscanf(devs_file, ":  Product=%n" PRODUCT_STRING "%n", &start, &end)) {
                if (start < end) {
                    *bus = current_bus;
                    *dev = current_dev;
                    break;
                }
            } 
        }
        // Move to the end of the current line.
        do {
            ch = getc(devs_file);
        } while ((EOF != ch) && ('\n' != ch));
        if (EOF != ch) {
            ch = getc(devs_file);
        }
    }
    
    fclose(devs_file);
    if ((-1 != *bus) && (-1 != *dev)) {
        return 1;
    }
    fprintf(stderr, "Error: failed to find a USB device \"" PRODUCT_STRING "\"\n");
    return 0;
}

int
main(int argc, char** argv)
{
    int         bus, dev;
    int         actual_bus, actual_dev;
    char        devname[_POSIX_PATH_MAX];
    long        strtol_tmp1;
    char*       strtol_tmp2;
    
    if (3 != argc) {
        fprintf(stderr, "usb_chmod: wrong number of arguments\n");
        fprintf(stderr, "         : usage, usb_chmod <bus> <dev>\n");
        exit(EXIT_FAILURE);
    }
    if (('\0' == argv[1][0]) || ('\0' == argv[2][0])) {
        fprintf(stderr, "usb_chmod: invalid arguments\n");
        exit(EXIT_FAILURE);
    }
                                 
    strtol_tmp1 = strtol(argv[1], &strtol_tmp2, 10);
    if ('\0' != *strtol_tmp2) {
        fprintf(stderr, "usbchmod: invalid first argument, not a number\n");
        exit(EXIT_FAILURE);
    }
    if (strtol_tmp1 > INT_MAX) {
        fprintf(stderr, "usbchmod: invalid first argument, number too large\n");
        exit(EXIT_FAILURE);
    }
    bus = (int) strtol_tmp1;

    strtol_tmp1 = strtol(argv[2], &strtol_tmp2, 10);
    if ('\0' != *strtol_tmp2) {
        fprintf(stderr, "usbchmod: invalid second argument, not a number\n");
        exit(EXIT_FAILURE);
    }
    if (strtol_tmp1 > INT_MAX) {
        fprintf(stderr, "usbchmod: invalid second argument, number too large\n");
        exit(EXIT_FAILURE);
    }
    dev = (int) strtol_tmp1;
        
    if (!usb_scan_devices(&actual_bus, &actual_dev)) {
        fprintf(stderr, "usb_chmod: failed to find eCos USB test application\n");
        exit(EXIT_FAILURE);
    }
    if ((bus != actual_bus) || (dev != actual_dev)) {
        fprintf(stderr, "usbchmod: mismatch between specified and actual USB identifiers.\n");
        fprintf(stderr, "         : eCos test application is at %03d/%03d, not %03d/%03d\n",
                actual_bus, actual_dev, bus, dev);
        exit(EXIT_FAILURE);
    }
    
    if (_POSIX_PATH_MAX == snprintf(devname, _POSIX_PATH_MAX, "/proc/bus/usb/" "%03d/%03d", actual_bus, actual_dev)) {
        fprintf(stderr, "usbchmod: internal error, buffer overflow\n");
        exit(EXIT_FAILURE);
    }

    if (0 != chmod(devname, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) {
        int old_errno = errno;
        fprintf(stderr, "usbchmod: failed to modify access rights on %s\n", devname);
        if ((old_errno >= 0) && (old_errno < sys_nerr)) {
            fprintf(stderr, "         : %s\n", sys_errlist[old_errno]);
        }
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
