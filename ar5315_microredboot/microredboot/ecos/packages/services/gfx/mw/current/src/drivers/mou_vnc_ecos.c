//==========================================================================
//
//      mou_vnc_ecos.c
//
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-08-22
// Purpose:
// Description:  Microwindows mouse driver for VNC server on eCos
//
//####DESCRIPTIONEND####
//
//========================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <pkgconf/vnc_server.h>  /* CYGDAT_VNC_SERVER_MOUSE_NAME */
#include "device.h"

static int vnc_Open(MOUSEDEVICE *pmd);
static void vnc_Close(void);
static int vnc_GetButtonInfo(void);
static void vnc_GetDefaultAccel(int *pscale,int *pthresh);
static int vnc_Read(MWCOORD *px, MWCOORD *py, MWCOORD *pz, int *pb);

static int mouse_fd = -1;

/* Hack extern to used when hiding the mouse cursor
 * There needs to be a better way to do this
*/
extern SCREENDEVICE scrdev;

MOUSEDEVICE mousedev = {
    vnc_Open,
    vnc_Close,
    vnc_GetButtonInfo,
    vnc_GetDefaultAccel,
    vnc_Read,
    NULL
};


static int vnc_Open(MOUSEDEVICE *pmd)
{
    mouse_fd = open(CYGDAT_VNC_SERVER_MOUSE_NAME, O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0)
    {
        EPRINTF("Error %d opening VNC mouse\n", errno);
        return -1;
    }

//    GdHideCursor(&scrdev);
    return mouse_fd;
}

static void vnc_Close(void)
{
    /* Close the mouse device. */
    if (mouse_fd >= 0)
    {
        close(mouse_fd);
    }

    mouse_fd = -1;
}

static int vnc_GetButtonInfo(void)
{
 	/* Get mouse buttons supported */
    return (MWBUTTON_L + MWBUTTON_R);
}

static void vnc_GetDefaultAccel(int *pscale,int *pthresh)
{
	/*
	 * Get default mouse acceleration settings
	 * Just return something inconspicuous for now.
	 */
//     diag_printf("Mouse: vnc_GetDefaultAccel()\n");
	*pscale = 3;
	*pthresh = 5;
}

static int vnc_Read(MWCOORD *px, MWCOORD *py, MWCOORD *pz, int *pb)
{
	/* Read a mouse event */
    cyg_uint8 data[8];
    int bytes_read;

    bytes_read = read(mouse_fd, data, 8);

    if (bytes_read != sizeof(data))
    {
        if (errno == EINTR || errno == EAGAIN)
        {
            return 0;
        }
        /*
         * kernel driver bug: select returns read available,
         * but read returns -1
         * we return 0 here to avoid GsError above
         */
        /*return -1;*/

        return 0;
    }

    *px = data[2]*256 + data[3];
    *py = data[4]*256 + data[5];
    *pb = 0;
    if (data[1] & 0x01)
    {
        *pb += MWBUTTON_L;
    }
    if (data[1] & 0x04)
    {
        *pb += MWBUTTON_R;
    }

    *pz = 0;

    return 2;

#if 0
    if(! *pb )
    {
        return 1;         /* Don't have button data */
    }
    else
    {
        return 2;         /* Have full set of data */
    }
#endif

}
