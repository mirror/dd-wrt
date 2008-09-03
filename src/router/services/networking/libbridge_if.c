/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <shutils.h>

#include "libbridge.h"
#include "libbridge_private.h"

int br_add_bridge( const char *brname )
{
    int ret;
    eval("ifconfig",brname,"mtu",getBridgeMTU(brname));

#ifdef SIOCBRADDBR
    ret = ioctl( br_socket_fd, SIOCBRADDBR, brname );
    if( ret < 0 )
#endif
    {
	char _br[IFNAMSIZ];
	unsigned long arg[3] = { BRCTL_ADD_BRIDGE, ( unsigned long )_br };

	strncpy( _br, brname, IFNAMSIZ );
	ret = ioctl( br_socket_fd, SIOCSIFBR, arg );
    }

    return ret < 0 ? errno : 0;
}

int br_del_bridge( const char *brname )
{
    int ret;

#ifdef SIOCBRDELBR
    ret = ioctl( br_socket_fd, SIOCBRDELBR, brname );
    if( ret < 0 )
#endif
    {
	char _br[IFNAMSIZ];
	unsigned long arg[3] = { BRCTL_DEL_BRIDGE, ( unsigned long )_br };

	strncpy( _br, brname, IFNAMSIZ );
	ret = ioctl( br_socket_fd, SIOCSIFBR, arg );
    }
    return ret < 0 ? errno : 0;
}

int br_add_interface( const char *bridge, const char *dev )
{

    eval("ifconfig",dev,"mtu",getBridgeMTU(bridge));

    struct ifreq ifr;
    int err;
    int ifindex = if_nametoindex( dev );

    if( ifindex == 0 )
	return ENODEV;

    strncpy( ifr.ifr_name, bridge, IFNAMSIZ );
#ifdef SIOCBRADDIF
    ifr.ifr_ifindex = ifindex;
    err = ioctl( br_socket_fd, SIOCBRADDIF, &ifr );
    if( err < 0 )
#endif
    {
	unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };

	ifr.ifr_data = ( char * )args;
	err = ioctl( br_socket_fd, SIOCDEVPRIVATE, &ifr );
    }
    eval( "ifconfig", dev, "promisc" );
    return err < 0 ? errno : 0;
}

int br_del_interface( const char *bridge, const char *dev )
{
    struct ifreq ifr;
    int err;
    int ifindex = if_nametoindex( dev );

    if( ifindex == 0 )
	return ENODEV;

    strncpy( ifr.ifr_name, bridge, IFNAMSIZ );
#ifdef SIOCBRDELIF
    ifr.ifr_ifindex = ifindex;
    err = ioctl( br_socket_fd, SIOCBRDELIF, &ifr );
    if( err < 0 )
#endif
    {
	unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };

	ifr.ifr_data = ( char * )args;
	err = ioctl( br_socket_fd, SIOCDEVPRIVATE, &ifr );
    }

    return err < 0 ? errno : 0;
}

static int
br_set( const char *bridge, const char *name,
	unsigned long value, unsigned long oldcode )
{
    int ret = -1;

#ifdef HAVE_LIBSYSFS
    struct sysfs_class_device *dev;

    dev = sysfs_get_class_device( br_class_net, bridge );
    if( dev )
    {
	struct sysfs_attribute *attr;
	char buf[32];
	char path[SYSFS_PATH_MAX];

	snprintf( buf, sizeof( buf ), "%ld\n", value );
	snprintf( path, SYSFS_PATH_MAX, "%s/bridge/%s", dev->path, name );

	attr = sysfs_open_attribute( path );
	if( attr )
	{
	    ret = sysfs_write_attribute( attr, buf, strlen( buf ) );
	    sysfs_close_attribute( attr );
	}
	sysfs_close_class_device( dev );
    }
    else
#endif
    {
	struct ifreq ifr;
	unsigned long args[4] = { oldcode, value, 0, 0 };

	strncpy( ifr.ifr_name, bridge, IFNAMSIZ );
	ifr.ifr_data = ( char * )&args;
	ret = ioctl( br_socket_fd, SIOCDEVPRIVATE, &ifr );
    }

    return ret < 0 ? errno : 0;
}

int br_set_bridge_forward_delay( const char *br, int sec )
{
    struct timeval tv;

    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return br_set( br, "forward_delay", __tv_to_jiffies( &tv ),
		   BRCTL_SET_BRIDGE_FORWARD_DELAY );
}

int br_set_stp_state( const char *br, int stp_state )
{
    return br_set( br, "stp_state", stp_state, BRCTL_SET_BRIDGE_STP_STATE );
}

int br_set_bridge_prio( const char *br, char *prio )
{
    return br_set( br, "priority", atoi( prio ), BRCTL_SET_BRIDGE_PRIORITY );
}

int br_set_port_prio( const char *bridge, char *port, char *prio )
{
    return port_set( bridge, port, "priority", atoi( prio ),
		     BRCTL_SET_PORT_PRIORITY );
}
