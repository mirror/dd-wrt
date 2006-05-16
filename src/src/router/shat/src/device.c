/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: device.c,v 1.23 2005/03/20 23:34:28 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_tun.h>
#include <net/route.h>

#include <features.h>    /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
# include <netpacket/packet.h>
# include <net/ethernet.h>     /* the L2 protocols */
#else
# include <asm/types.h>
# include <linux/if_packet.h>
# include <linux/if_ether.h>   /* The L2 protocols */
#endif

#include "util.h"

#define __DEVICE_PRIVATE
#include "device.h"

/* ----------------------------------------------------------------------- *
 * variables and definitions
 * ----------------------------------------------------------------------- */

#ifdef         _DEBUG
#define _DEVICE_DEBUG
#endif

/* ----------------------------------------------------------------------- *
 * Fancy error message
 * ----------------------------------------------------------------------- */

static
void init_error (const char   *msg, 
                const char    *arg, 
                const char *action,
                int            xit) {
    if (arg == 0 && action == 0)
        logger (LOG_ERR, "%s: %s", progname, msg);
    else if (arg == 0 && action != 0)
        logger (LOG_ERR, "%s: %s: %s (%s)",
                 progname, msg, action, strerror (errno));
    else if (arg != 0 && action == 0)
        logger (LOG_ERR, "%s: %s %s", progname, msg, arg);
    else /*  arg != 0 && action != 0 */
        logger (LOG_ERR, "%s: %s %s: %s (%s)",
                progname, msg, arg, action, strerror (errno));
    if (xit) {
        sleep (5); /* do not exit immediately - wait for some seconds */
        exit (2);
    }
}

static
void if_init_error (const char *dev, const char *action, int xit) {
    return init_error
        ("Failed to initialise interface", dev, action, xit);
}

static
void bind_init_error (const char *dev, const char *action, int xit) {
    return init_error
        ("Cannot bind socket to interface", dev, action, xit);
}

static
void nonblock_init_error (const char *dev, const char *action, int xit) {
    return init_error 
        ("Cannot set interface socket to nonblocking mode",
         dev, action, xit);
}

static
void sname_init_error (const char *dev, const char *action, int xit) {
    return init_error
        ("Cannot read socket name of interface", dev, action, xit);
}

static
void ll_init_error (const char *dev, const char *action, int xit) {
    return init_error 
        ("Unusable interface (missing link layer address)",
         dev, action, xit);
}

/* ----------------------------------------------------------------------- *
 * general interface initialisation helpers
 * ----------------------------------------------------------------------- */

static
int set_devproc (char      *name,
                 const char *fmt,
                 long      value,
                 int     xtonerr) {

    char *p, current [2];
    char *buf = (char *)xmalloc (strlen (fmt) + IFNAMSIZ + 2);
    int fd ;
    
    sprintf (buf, fmt, name);
    if ((fd = open (buf, O_RDWR)) == 0) {
        init_error ("Cannot set", buf, "open", xtonerr);
        goto return_error;
    }
    if (read (fd, current, 1) != 1) {
        init_error ("Cannot read value", buf, "read", xtonerr);
        goto close_fd_return_error;
    }
    p = ulongp_pp (&value);
    if (write (fd, p, 1) < 1) {
        init_error ("Cannot set value", buf, "read", xtonerr);
        goto close_fd_return_error;
    }
    close (fd);
    free (buf);

    current [1] = '\0' ;
    return atoi (current);

close_fd_return_error:
    close (fd);

return_error:
    free (buf);
    return -1;
}

/* ----------------------------------------------------------------------- *
 * inbound interface initialisation
 * ----------------------------------------------------------------------- */

IN_DEV *inbound_if_init (const char *name,
                         IP4_POOL *ifaddr,
                         IP4_POOL *ifnetw,
                         int  capture_all,
                         int          xit) {
    int n, ioctl_fd;
    struct sockaddr_ll sll;
    struct ifreq ifr ;
    static struct ifconf ifc ;
    IN_DEV dev ;

    /* set up interface info structure */
    if (name == 0 || *name == 0)
        name = DEFAULT_INBOUND;

    XZERO (dev);

    /* create ioctl command socket */
    if ((ioctl_fd = socket (PF_PACKET, SOCK_DGRAM, 0)) < 0) {
        init_error ("Cannot create a packet socket", "SOCK_DGRAM",
                    "PF_PACKET", xit);
        return 0;
    }

    /* GET INTERFACE INFO */

    /* get interface index for the socket address */
    XZERO (ifr); 
    strcpy (ifr.ifr_name, strncpy (dev.name, name, IFNAMSIZ-1));
    if ((ioctl (ioctl_fd, SIOCGIFINDEX, &ifr)) < 0) {
        if_init_error (ifr.ifr_name, "SIOCGIFINDEX", xit);
        close (ioctl_fd) ;
        return 0 ;
    }
    dev.ifindex = ifr.ifr_ifindex;

    /* check interface flags */
    if (ioctl (ioctl_fd, SIOCGIFFLAGS, &ifr) < 0) {
        init_error ("Can't get interface flags of",
                    dev.name, "SIOCGIFFLAGS", xit);
        goto close_ioctl_exit ;
    }
    if ((ifr.ifr_flags & IFF_UP) == 0) {
        init_error ("Interface is down", ifr.ifr_name, 0, xit);
        goto close_ioctl_exit ;
    }
    if ((ifr.ifr_flags & IFF_LOOPBACK) != 0) {
        init_error
            ("The loopback interface will not work: ", dev.name,0,xit);
        goto close_ioctl_exit ;
    }
    /* save flags */
    dev.flags = ifr.ifr_flags;

    /* get hardware address */
    XZERO (ifr.ifr_ifru);
    if (ioctl (ioctl_fd, SIOCGIFHWADDR, &ifr) < 0) {
        if_init_error (dev.name, "SIOCGIFHWADDR", xit);
        goto close_ioctl_exit ;
    }
    memcpy (&dev.mac, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    /* get (some) ip address */
    XZERO (ifr.ifr_ifru);
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl (ioctl_fd, SIOCGIFADDR, &ifr) < 0) {
        if_init_error (dev.name, "SIOCGIFADDR", xit);
        goto close_ioctl_exit ;
    }
    memcpy (&dev.ipa,
            &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr,
            sizeof (dev.ipa));

    if (ifaddr)
        /* collect all the device addresses */
        for (n = 4; /* empty */ ; n += 3) {
            int len = n * sizeof (struct ifreq);
            ifc.ifc_len = len ;
            ifc.ifc_req = (struct ifreq *)xmalloc (ifc.ifc_len) ;

            /* Get a list of all interface addresses */
            if (ioctl (ioctl_fd, SIOCGIFCONF, &ifc) < 0) {
                if_init_error (ifr.ifr_name, "SIOCGIFCONF", xit);
                free (ifc.ifc_req);
                goto close_ioctl_exit ;
            }

            /* Make sure that we got all. Thus we want the system to have
               less space filled than offered. */
            if (len <= ifc.ifc_len) {
                free (ifc.ifc_req);
                continue ;
            }

            /* filter out the LAN interface addresses */
            n = ifc.ifc_len / sizeof (struct ifreq);
            while (-- n >= 0)
                if (strcmp (dev.name, ifc.ifc_req [n].ifr_name) == 0)
                    /* add to the address list */
                    ip4_fetch
                        (ifaddr, &((struct sockaddr_in*)
                                   &ifc.ifc_req [n].ifr_addr)->sin_addr);
            free (ifc.ifc_req);
            break ;
        }

    /* get netmask */
    XZERO (ifr.ifr_ifru);
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl (ioctl_fd, SIOCGIFNETMASK, &ifr) < 0) {
        if_init_error (ifr.ifr_name, "SIOCGIFNETMASK", xit);
        goto close_ioctl_exit ;
    }
    memcpy (&dev.ipm,
            &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr,
            sizeof (dev.ipm));

    /* compile network address */
    dev.ipn.s_addr = dev.ipa.s_addr & dev.ipm.s_addr ;

    if (ifnetw) {
        /* add network address range to the list */
        ip4addr_t first = ntohl (dev.ipn.s_addr) + 1;
        ip4addr_t limit = first | ~ntohl (dev.ipm.s_addr);
        struct in_addr ip ;
        do {
            ip.s_addr = htonl (first) ;
            ip4_fetch (ifnetw, &ip);
        } while (++ first < limit) ;
    }
          
    /* get mtu */
    if (ioctl (ioctl_fd, SIOCGIFMTU, &ifr) < 0) {
        if_init_error (ifr.ifr_name, "SIOCGIFMTU", xit);
        goto close_ioctl_exit ;
    }
    dev.mtu = ifr.ifr_mtu ;

    /* not needed anymore */
    close (ioctl_fd);

    /* open up an ethernet/ip socket on that interface - we need to use
       ETH_P_ALL in order to see any outgoing DHCP packets */

    /* initalise socket address structure */
    sll.sll_family   = PF_PACKET;
    sll.sll_protocol = capture_all ? htons (ETH_P_ALL) : htons (ETH_P_IP);
    sll.sll_ifindex  = dev.ifindex;
    
    if ((dev.eth_fd = socket (PF_PACKET, SOCK_RAW, sll.sll_protocol)) < 0) {
        init_error ("Cannot create an IP packet socket",
                    "SOCK_RAW", "PF_PACKET", xit);
        return 0;
    }
 
    /* bind to the interface */
    if (bind (dev.eth_fd, (struct sockaddr*)&sll, sizeof (sll)) < 0) {
        bind_init_error (dev.name, "bind/ether-ip", xit);
        goto close_eth_exit ;
    }

    n = sizeof (sll);
    if (getsockname (dev.eth_fd, (struct sockaddr*)&sll, &n) < 0) {
        sname_init_error (dev.name, "getsockname/Ethernet", xit);
        goto close_eth_exit ;
    }

    if (sll.sll_halen == 0) {
        ll_init_error (dev.name, "Ethernet/IP", xit);
        goto close_eth_exit ;
    }
    
    /* set to nonblocking mode */
    if (fcntl (dev.eth_fd, F_SETFL, O_NONBLOCK) < 0) {
        nonblock_init_error (dev.name, "O_NONBLOCK/Ethernet", xit);
        goto close_eth_exit ;
    }

    /* INIT ARP SOCKET BOUND TO INTERFACE */

    /* check interface flags */
    if ((dev.flags & IFF_NOARP) != 0) {
        init_error ("NOARP configured on interface %s", dev.name, 0, xit);
        goto close_eth_exit ;
    }

    /* open up and initialise a ARP socket */
    if ((dev.arp_fd =
         socket (PF_PACKET, SOCK_DGRAM, htons (ETH_P_ARP))) < 0) {
        init_error ("Cannot create an ARP packet socket",
                    "SOCK_DGRAM", "PF_PACKET", xit);
        goto close_eth_exit ;
    }

    /* initalise socket address structure */
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = htons (ETH_P_ARP);
    sll.sll_ifindex  = dev.ifindex;

    /* bind to the interface */
    if (bind (dev.arp_fd, (struct sockaddr*)&sll, sizeof (sll)) < 0) {
        bind_init_error (dev.name, "bind/arp", xit);
        goto close_arp_and_eth_exit ;
    }
 
    n = sizeof (sll);
    if (getsockname (dev.arp_fd, (struct sockaddr*)&sll, &n) < 0) {
        sname_init_error (dev.name, "getsockname/ARP", xit);
        goto close_arp_and_eth_exit ;
    }
    if (sll.sll_halen == 0) {
        ll_init_error (dev.name, "ARP", xit);
        goto close_arp_and_eth_exit ;
    }

    /* set to nonblocking mode */
    if (fcntl (dev.arp_fd, F_SETFL, O_NONBLOCK) < 0) {
        nonblock_init_error (dev.name, "O_NONBLOCK/ARP", xit);
        goto close_arp_and_eth_exit ;
    }

    /* We need to disable the rp_fiter on the current interface, 
       otherwise spoofed packets will be dropped. */
    if ((dev.rpfilter = set_devproc
         (dev.name, PROC_RPFITLER_FMT, 0, xit))<0||
        (dev.proxyarp = set_devproc
         (dev.name, PROC_PROXYARP_FMT, 0, xit))<0)
        /* these functions did complain, already */
        goto close_arp_and_eth_exit ;

    /* minimum default mtu */
    dev.size_iobuf = 1500 ;
    
    /* allocate io buffer big enough to hold ethernet frames */
    if (dev.size_iobuf < dev.mtu)
        dev.size_iobuf = dev.mtu ;
 
    /* leave room for alignment shifting */
    dev.size_iobuf += 2 * sizeof (struct ethhdr);

    return memcpy (xmalloc (sizeof (dev) + dev.size_iobuf),
                   &dev,
                   sizeof (dev));

    /*NOTREACHED*/

close_ioctl_exit:
    close (ioctl_fd);
    return 0 ;

close_arp_and_eth_exit:
    close (dev.arp_fd);

close_eth_exit:
    close (dev.eth_fd);
    return 0 ;
}


void inbound_if_close (IN_DEV *id) {
    set_devproc (id->name, PROC_RPFITLER_FMT, id->rpfilter, 0);
    set_devproc (id->name, PROC_PROXYARP_FMT, id->proxyarp, 0);
    close (id->arp_fd);
    close (id->eth_fd);
    free (id);
}


/* ----------------------------------------------------------------------- *
 * tunnel interface initialisation
 * ----------------------------------------------------------------------- */

TUN_DEV *tunnel_if_init (const char *name,
                         in_addr_t   bind,    /* address in host order */
                         int          xit) {
    TUN_DEV dev ;
    struct ifreq ifr;
    struct sockaddr_in sin ;
    int flags;
    time_t now = time (0);

    if (name == 0 || *name == 0)
        name = DEFAULT_TUNBOUND;

    XZERO (dev);

    /* on reboot, the tun devive might not be up yet */
    if ((dev.tun_fd = open (DEV_NET_TUN, O_RDWR)) < 0) {
        logger (LOG_ERR, "TUN device not ready yet, retrying ...");
        do {
            sleep (5);
            dev.tun_fd = open (DEV_NET_TUN, O_RDWR) ;
        } while (dev.tun_fd < 0 && time (0) <= now + DEV_NET_TIMEOUT) ;
        if (dev.tun_fd < 0) {
            init_error ("Cannot open TUN device", DEV_NET_TUN, "open", xit);
            return 0;
        }
    }
    flags = fcntl (dev.tun_fd, F_GETFL, 0);
    fcntl (dev.tun_fd, F_SETFL, flags | O_NONBLOCK);

    XZERO (ifr);
    /* IFF_TAP is for Ethernet frames. */
    /* IFF_TUN is for IP. */
    /* IFF_NO_PI is for not receiving extra meta packet information. */
    ifr.ifr_flags = IFF_TUN ;

    /* set an interface name if present */
    if (name) strncpy (ifr.ifr_name, name, IFNAMSIZ-1);

    /* initialise tun device */
    if (ioctl (dev.tun_fd, TUNSETIFF, (void *) &ifr) < 0) {
        init_error ("Cannot set up TUN device", name, "TUNSETIFF", xit);
        goto close_tun_exit ;
    }

    /* get name from whatever the interface is called */
    name = strncpy (dev.name, ifr.ifr_name, IFNAMSIZ-1);

    /* open up and initialise an ip socket */
    if ((dev.ioctl_fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
        init_error ("Cannot create an IP packet socket",
                    "SOCK_DGRAM", "IPPROTO_IP", xit);
        goto close_tun_exit ;
    }
 
    /* set an ip address */
    XZERO (sin);
    sin.sin_family = AF_INET;
    if ((dev.ipa.s_addr = bind) == 0)
        dev.ipa.s_addr = ntohl (inet_addr (TUN_DEFAULT_IP));
    sin.sin_addr.s_addr = dev.ipa.s_addr ;
    memcpy (&ifr.ifr_addr, &sin, sizeof (struct sockaddr));

    /* commit (using that ip socket) ... */
    if (ioctl (dev.ioctl_fd, SIOCSIFADDR, (void *) &ifr) < 0) {
        init_error
            ("Cannot initialise TUN interface", name, "SIOCSIFADDR", xit);
        goto close_ioctl_tun_exit ;
    }
    
    /* bump up the qlen to deal with bursts from the network */
    ifr.ifr_qlen = 1000;
    if (ioctl (dev.ioctl_fd, SIOCSIFTXQLEN, (void *) &ifr) < 0) {
        init_error ("Cannot set TUN interface queue length",
                    name, "SIOCSIFTXQLEN", xit);
        goto close_ioctl_tun_exit ;
    }

    ifr.ifr_flags = IFF_UP;
    if (ioctl (dev.ioctl_fd, SIOCSIFFLAGS, (void *) &ifr) < 0) {
        init_error
            ("Cannot set TUN interface flags", name, "SIOCSIFFLAGS", xit);
        goto close_ioctl_tun_exit ;
    }

    /* GET INTERFACE INFO */

    /* get interface index for the socket address */
    XZERO (ifr); 
    strcpy (ifr.ifr_name, strncpy (dev.name, name, IFNAMSIZ-1));
    if ((ioctl (dev.ioctl_fd, SIOCGIFINDEX, &ifr)) < 0) {
        if_init_error (name, "SIOCGIFINDEX", xit);
        goto close_ioctl_tun_exit ;
    }
    dev.ifindex = ifr.ifr_ifindex;

    /* check interface flags */
    if (ioctl (dev.ioctl_fd, SIOCGIFFLAGS, &ifr) < 0) {
        init_error
            ("Can't get interface flags of", name, "SIOCGIFFLAGS", xit);
        goto close_ioctl_tun_exit ;
    }
    if ((ifr.ifr_flags & IFF_UP) == 0) {
        init_error ("Interface is down", ifr.ifr_name, 0, xit);
        goto close_ioctl_tun_exit ;
    }
    if ((ifr.ifr_flags & IFF_LOOPBACK) != 0) {
        init_error ("The loopback interface will not work: ", name, 0, xit);
        goto close_ioctl_tun_exit ;
    }
    /* save flags */
    dev.flags = ifr.ifr_flags;

    /* get mtu */
    if (ioctl (dev.ioctl_fd, SIOCGIFMTU, &ifr) < 0) {
        if_init_error (ifr.ifr_name, "SIOCGIFMTU", xit);
        goto close_ioctl_tun_exit ;
    }
    dev.mtu = ifr.ifr_mtu ;

    /* minimum default mtu */
    dev.size_iobuf = 1500 ;
    
    /* allocate io buffer big enough to hold ethernet frames */
    if (dev.size_iobuf < dev.mtu)
        dev.size_iobuf = dev.mtu ;
 
    /* leave room for alignment shifting */
    dev.size_iobuf += 2 * sizeof (struct ethhdr) ;

    return memcpy (xmalloc (sizeof (dev) + dev.size_iobuf),
                   &dev,
                   sizeof (dev));

    /*NOTREACHED*/

close_ioctl_tun_exit:
    close (dev.ioctl_fd);

close_tun_exit:
    close (dev.tun_fd);
    return 0 ;
}


void tunnel_if_close (TUN_DEV *id) {
    close (id->tun_fd);
    close (id->ioctl_fd);
    free (id);
}

/* ----------------------------------------------------------------------- *
 * tunnel routing
 * ----------------------------------------------------------------------- */

unsigned tunnel_route (TUN_DEV      *dev,
                       in_addr_t from_ip,    /* address in host order */
                       in_addr_t   to_ip,
                       int           add,
                       int           xit) {

    struct rtentry   r;
    struct in_addr  ip;
    struct in_addr msk;

    XZERO (r);
    r.rt_dev           = dev->name ;
    r.rt_dst.sa_family = AF_INET;

    /* find the smallest network that contains both, from_ip and to_ip */
    if (from_ip == to_ip) {
        ip.s_addr   =  htonl (from_ip) ;
        msk.s_addr  = 0xffffffff ;
        r.rt_flags |= RTF_HOST;
    }
    else {
        in_addr_t iq = from_ip ^ to_ip ; /* note: iq <> 0 */
        int w = 0 ;/* (32 - w) counts the number of leading bits */
        while (iq) { iq >>= 1; w ++ ; }
        if (w < 32) {
            msk.s_addr = htonl (~((1 << w) - 1)) ;
            ip.s_addr  = htonl (from_ip) & msk.s_addr ;
        }
        else {
            ip.s_addr  = 0 ; /* whew */
            msk.s_addr = 0 ;
        }
    }

    r.rt_dst.sa_family                             = AF_INET;
    ((struct sockaddr_in*)&r.rt_dst)->sin_addr     = ip;

    r.rt_genmask.sa_family                         = AF_INET;
    ((struct sockaddr_in*)&r.rt_genmask)->sin_addr = msk;
    r.rt_flags                                     = (RTF_UP | RTF_STATIC);

    if (ioctl (dev->ioctl_fd, add ? SIOCADDRT : SIOCDELRT, &r) < 0) {
        char * mode = add ? "SIOCADDRT" : "SIOCDELRT" ;
        init_error ("Cannot set up route", dev->name, mode, xit);
        return 0;
    }

    logger (LOG_INFO, "Interface route %s/%s to %s %s.",
            ipaddr_pp (&ip), ipaddr_pp (&msk), dev->name,
            add ? "added" : "deleted");

    return 1;
}


/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
