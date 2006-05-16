/*********************************************************************
 *                
 * Filename:      irda.c
 * Version:       0.1
 * Description:   A first attempt to make ifconfig understand IrDA
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Wed Apr 21 09:03:09 1999
 * Modified at:   Wed Apr 21 09:17:05 1999
 * Modified by:   Dag Brattli <dagb@cs.uit.no>
 * 
 *     This program is free software; you can redistribute it and/or 
 *     modify it under the terms of the GNU General Public License as 
 *     published by the Free Software Foundation; either version 2 of 
 *     the License, or (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License 
 *     along with this program; if not, write to the Free Software 
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *     MA 02111-1307 USA
 *     
 ********************************************************************/

#include "config.h"

#if HAVE_AFIRDA || HAVE_HWIRDA
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

/* Probably not a good idea to include <linux/if_arp.h> */
#ifndef ARPHRD_IRDA
#define ARPHRD_IRDA 783
#endif

/*
 * Function irda_print (ptr)
 *
 *    Print hardware address of interface
 *
 */
static char *irda_print(unsigned char *ptr)
{
    static char buff[8];

    sprintf(&buff[strlen(buff)], "%02x:%02x:%02x:%02x", ptr[3], ptr[2], 
	    ptr[1], ptr[0]);

    return (buff);
}

struct hwtype irda_hwtype =
{
     "irda", NULL, ARPHRD_IRDA, 2,
     irda_print, NULL, NULL, 0
};

#endif				/* HAVE_xxIRDA */
