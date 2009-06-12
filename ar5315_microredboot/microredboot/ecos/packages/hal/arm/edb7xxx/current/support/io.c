//==========================================================================
//
//        io.c
//
//        Linux I/O support for Cirrus Logic EDB7xxx eval board tools
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
// Date:          1999-06-16
// Description:   Linux I/O support for Cirrus Logic EDB7xxx FLASH tools
//####DESCRIPTIONEND####

//
// io.c - I/O functions
//

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

char _ReceiveChar(long port)
{
    char buf;
    int res;
    // Port is in non-blocking mode, this needs to have a loop
    do {
        res = read(port, &buf, 1);
    } while (res < 0);
//    printf("Read: %c\n", buf);
    return buf;
}

static void
uspin(int len)
{
    volatile int cnt;
    while (--len >= 0) {
        for (cnt = 1;  cnt < 2000;  cnt++) ;
    }
}

void _SendChar(long port, char ch)
{
    char buf = ch;
    write(port, &buf, 1);
//    printf("Send: %x\n", ch);
//    usleep(100);
    uspin(100);
}

void _SetBaud(long port, long reqRate)
{
    struct termios buf;
    int rate;

    // Get original configuration
    if (tcgetattr(port, &buf)) {
        fprintf(stderr, "Can't get port config\n");
        return;
    }

    // Reset to raw.
    buf.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                     |INLCR|IGNCR|ICRNL|IXON);
    buf.c_oflag &= ~OPOST;
    buf.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    buf.c_cflag &= ~(CSIZE|PARENB);
    buf.c_cflag |= CS8;

    // Set baud rate.
    switch (reqRate) {
    case 9600:
        rate = B9600;
        break;
    case 38400:
        rate = B38400;
        break;
    case 57600:
        rate = B57600;
        break;
    case 115200:
        rate = B115200;
        break;
    }
    cfsetispeed(&buf, rate);
    cfsetospeed(&buf, rate);

    // Set data bits.
    buf.c_cflag &= ~CSIZE;
    buf.c_cflag |= CS8;

    // Set stop bits.
    buf.c_cflag &= ~CSTOPB;  // One stop

    // Set parity.
    buf.c_cflag &= ~(PARENB | PARODD); // no parity.
    
    // Set the new settings
    if (tcsetattr(port, TCSADRAIN, &buf)) {
	fprintf(stderr, "Error: tcsetattr\n");
	return;
    }

    tcdrain(port);
    usleep(1000000/10*2);
    sleep(2);
}

int _CharReady(long port)
{
#ifdef __CYGWIN__
    // Windows doesn't support the below ioctl
    return 1;
#else
    int n, res;
    res = ioctl(port, FIONREAD, &n);
    if (res < 0) {
        fprintf(stderr, "I/O error: %s\n", strerror(errno));
        exit(1);
    }
    return n;  // Non-zero if characters ready to read
#endif
}

void _WaitForOutputEmpty(long port)
{
    usleep(2000000);
}

long _OpenPort(char *name)
{
    int fd;
    fd = open(name, O_RDWR|O_NONBLOCK);
    return fd;
}
