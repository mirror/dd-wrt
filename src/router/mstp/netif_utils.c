/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "log.h"

#ifndef SYSFS_CLASS_NET
#define SYSFS_CLASS_NET "/sys/class/net"
#endif

static int netsock = -1;

int netsock_init(void)
{
    netsock = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 > netsock)
    {
        ERROR("Couldn't open inet socket for ioctls: %m\n");
        return -1;
    }
    return 0;
}

int get_hwaddr(char *ifname, __u8 *hwaddr)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if(0 > ioctl(netsock, SIOCGIFHWADDR, &ifr))
    {
        ERROR("%s: get hw address failed: %m", ifname);
        return -1;
    }
    memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    return 0;
}

int get_flags(char *ifname)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if(0 > ioctl(netsock, SIOCGIFFLAGS, &ifr))
    {
        ERROR("%s: get interface flags failed: %m", ifname);
        return -1;
    }
    return ifr.ifr_flags;
}

int if_shutdown(char *ifname)
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    /* TODO: Let's hope -1 is not a valid flag combination */
    if(-1 == (ifr.ifr_flags = get_flags(ifname)))
    {
        return -1;
    }
    ifr.ifr_flags &= ~IFF_UP;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if(0 > ioctl(netsock, SIOCSIFFLAGS, &ifr))
    {
        ERROR("%s: set if_down flag failed: %m", ifname);
        return -1;
    }
    return 0;
}

int ethtool_get_speed_duplex(char *ifname, int *speed, int *duplex)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    struct ethtool_cmd ecmd;

    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    if(0 > ioctl(netsock, SIOCETHTOOL, &ifr))
    {
        ERROR("Cannot get speed/duplex for %s: %m\n", ifname);
        return -1;
    }
    *speed = ecmd.speed;   /* Ethtool speed is in Mbps */
    *duplex = ecmd.duplex; /* We have same convention as ethtool.
                               0 = half, 1 = full */
    return 0;
}

char *index_to_name(int index, char *name)
{
    return if_indextoname(index, name);
}

char *index_to_port_name(int index, char *name)
{
    return if_indextoname(index, name);
}

/********* Sysfs based utility functions *************/

/* This sysfs stuff might break with interface renames */
bool is_bridge(char *if_name)
{
    char path[32 + IFNAMSIZ];
    sprintf(path, SYSFS_CLASS_NET "/%s/bridge", if_name);
    return (0 == access(path, R_OK));
}

int get_bridge_portno(char *if_name)
{
    char path[32 + IFNAMSIZ];
    sprintf(path, SYSFS_CLASS_NET "/%s/brport/port_no", if_name);
    char buf[128];
    int fd;
    long res = -1;
    TSTM((fd = open(path, O_RDONLY)) >= 0, -1, "%m");
    int l;
    TSTM((l = read(fd, buf, sizeof(buf) - 1)) >= 0, -1, "%m");
    if(0 == l)
    {
        ERROR("Empty port index file");
        goto out;
    }
    else if((sizeof(buf) - 1) == l)
    {
        ERROR("port_index file too long");
        goto out;
    }
    buf[l] = 0;
    if('\n' == buf[l - 1])
        buf[l - 1] = 0;
    char *end;
    res = strtoul(buf, &end, 0);
    if(0 != *end || INT_MAX < res)
    {
        ERROR("Invalid port index %s", buf);
        res = -1;
    }
out:
    close(fd);
    return res;
}
