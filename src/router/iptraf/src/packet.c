/***

packet.c - routines to open the raw socket, read socket data and
           adjust the initial packet pointer
           
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997-2002

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <asm/types.h>
#include <curses.h>
#include <panel.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_fddi.h>
#include <linux/if_tr.h>
#include <linux/isdn.h>
#include <linux/sockios.h>
#include <msgboxes.h>
#include "deskman.h"
#include "error.h"
#include "options.h"
#include "links.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "isdntab.h"
#include "ifaces.h"
#include "packet.h"
#include "ipcsum.h"
#include "ipfrag.h"
#include "tr.h"

extern int daemonized;
extern int accept_unsupported_interfaces;

int isdnfd;
struct isdntab isdntable;

void open_socket(int *fd)
{
    *fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (*fd < 0) {
        write_error("Unable to open raw socket", daemonized);
        return;
    }
}

unsigned short getlinktype(unsigned short family, char *ifname,
                           int isdn_fd, struct isdntab *isdnlist)
{
    unsigned short result = 0;
    struct isdntabent *isdnent;

    switch (family) {
    case ARPHRD_ETHER:
        if (strncmp(ifname, "eth", 3) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "plip", 4) == 0)
            result = LINK_PLIP;
        else if (strncmp(ifname, "fddi", 4) == 0)       /* For some Ethernet- */
            result = LINK_ETHERNET;     /* emulated FDDI ifaces */
        else if (strncmp(ifname, "dvb", 3) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "sbni", 4) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "ipsec", 5) == 0)
            result = LINK_ETHERNET;
        else if ((strncmp(ifname, "wvlan", 5) == 0)
                 || (strncmp(ifname, "wlan", 4) == 0))
            result = LINK_ETHERNET;
        else if ((strncmp(ifname, "sm2", 3) == 0)
                 || (strncmp(ifname, "sm3", 3) == 0))
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "pent", 4) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "lec", 3) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "tun", 3) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "vlan", 3) == 0)
            result = LINK_VLAN;
        else if (strncmp(ifname, "brg", 3) == 0)
            result = LINK_ETHERNET;
        else if (strncmp(ifname, "tap", 3) == 0)
            result = LINK_ETHERNET;
        else if ((strncmp(ifname, "isdn", 4) == 0) && (isdn_fd != -1)) {
            isdnent = isdn_table_lookup(isdnlist, ifname, isdn_fd);

            switch (isdnent->encap) {
            case ISDN_NET_ENCAP_RAWIP:
                result = LINK_ISDN_RAWIP;
                break;
            case ISDN_NET_ENCAP_CISCOHDLC:
                result = LINK_ISDN_CISCOHDLC;
                break;
            default:
                result = LINK_INVALID;
                break;
            }
        } else if (accept_unsupported_interfaces)
            result = LINK_ETHERNET;
        break;
    case ARPHRD_LOOPBACK:
        result = LINK_LOOPBACK;
        break;
    case ARPHRD_SLIP:
    case ARPHRD_CSLIP:
    case ARPHRD_SLIP6:
    case ARPHRD_CSLIP6:
        result = LINK_SLIP;
        break;
    case ARPHRD_PPP:
        result = LINK_PPP;
        break;
    case ARPHRD_FDDI:
        result = LINK_FDDI;
        break;
    case ARPHRD_IEEE802:
    case ARPHRD_IEEE802_TR:
        result = LINK_TR;
        break;
    case ARPHRD_FRAD:
        result = LINK_FRAD;
        break;
    case ARPHRD_DLCI:
        result = LINK_DLCI;
        break;
    case ARPHRD_HDLC:
        result = LINK_CISCOHDLC;
        break;
    case ARPHRD_TUNNEL:
        result = LINK_IPIP;
        break;
    default:
        result = LINK_INVALID;
        break;
    }
    return result;
}

void adjustpacket(char *tpacket, unsigned short family,
                  char **packet, char *aligned_buf, unsigned int *readlen)
{
    unsigned int dataoffset;

    switch (family) {
    case LINK_ETHERNET:
    case LINK_LOOPBACK:
    case LINK_PLIP:
        *packet = tpacket + ETH_HLEN;
        *readlen -= ETH_HLEN;

        /*
         * Move IP data into an aligned buffer.  96 bytes should be sufficient
         * for IP and TCP headers with reasonable numbers of options and some
         * data.
         */

        memmove(aligned_buf, *packet, min(SNAPSHOT_LEN, *readlen));
        *packet = aligned_buf;
        break;
    case LINK_PPP:
    case LINK_SLIP:
    case LINK_ISDN_RAWIP:
        *packet = tpacket;
        break;
    case LINK_ISDN_CISCOHDLC:
    case LINK_FRAD:
    case LINK_DLCI:
        *packet = tpacket + 4;
        *readlen -= 4;
        break;
    case LINK_FDDI:
        *packet = tpacket + sizeof(struct fddihdr);
        *readlen -= sizeof(struct fddihdr);

        /*
         * Move IP data into an aligned buffer.  96 bytes should be sufficient
         * for IP and TCP headers with reasonable numbers of options and some
         * data.
         */

        memmove(aligned_buf, *packet, min(SNAPSHOT_LEN, *readlen));
        *packet = aligned_buf;
        break;
    case LINK_TR:
        /*
         * Token Ring patch supplied by Tomas Dvorak 
         */

        /*
         * Get the start of the IP packet from the Token Ring frame.
         */
        dataoffset = get_tr_ip_offset(tpacket);
        *packet = tpacket + dataoffset;
        *readlen -= dataoffset;
        /*
         * Move IP datagram into an aligned buffer.
         */
        memmove(aligned_buf, *packet, min(SNAPSHOT_LEN, *readlen));
        *packet = aligned_buf;
        break;
    case LINK_IPIP:
        *packet = tpacket;
        break;
    case LINK_VLAN:
        *packet = tpacket + VLAN_ETH_HLEN;
        readlen -= VLAN_ETH_HLEN;
        /*
         * Move IP datagram into an aligned buffer.
         */
        memmove(aligned_buf, *packet, min(SNAPSHOT_LEN, *readlen));
        *packet = aligned_buf;
    default:
        *packet = (char *) NULL;        /* return a NULL packet to signal */
        break;                  /* an unrecognized link protocol */
    }                           /* to the caller.  Hopefully, this */
}                               /* switch statement will grow. */

/*
 * IPTraf input function; reads both keystrokes and network packets.
 */

void getpacket(int fd, char *buf, struct sockaddr_ll *fromaddr,
               int *ch, int *br, char *ifname, WINDOW * win)
{
    int fromlen;
    fd_set set;
    struct timeval tv;
    int ss;
    int ir;
    struct ifreq ifr;

    FD_ZERO(&set);

    /*
     * Monitor stdin only if in interactive, not daemon mode.
     */

    if (!daemonized)
        FD_SET(0, &set);

    /*
     * Monitor raw socket
     */

    FD_SET(fd, &set);

    tv.tv_sec = 0;
    tv.tv_usec = DEFAULT_UPDATE_DELAY;

    do {
        ss = select(fd + 1, &set, 0, 0, &tv);
    } while ((ss < 0) && (errno == EINTR));

    *br = 0;
    *ch = ERR;

    if (FD_ISSET(fd, &set)) {
        fromlen = sizeof(struct sockaddr_pkt);
        *br = recvfrom(fd, buf, MAX_PACKET_SIZE, 0,
                       (struct sockaddr *) fromaddr, &fromlen);
        ifr.ifr_ifindex = fromaddr->sll_ifindex;
        ir = ioctl(fd, SIOCGIFNAME, &ifr);
        strcpy(ifname, ifr.ifr_name);
    }
    if (!daemonized) {
        if (FD_ISSET(0, &set))
            *ch = wgetch(win);
    } else
        *ch = ERR;
}

int processpacket(char *tpacket, char **packet, unsigned int *br,
                  unsigned int *total_br, unsigned int *sport,
                  unsigned int *dport, struct sockaddr_ll *fromaddr,
                  unsigned short *linktype, struct filterstate *filter,
                  int match_opposite, char *ifname, char *ifptr)
{
    static char aligned_buf[ALIGNED_BUF_LEN];
    struct iphdr *ip;
    int hdr_check;
    register int ip_checksum;
    register int iphlen;

    unsigned int sport_tmp, dport_tmp;
    unsigned int f_sport, f_dport;

    int firstin;

    union {
        struct tcphdr *tcp;
        struct udphdr *udp;
    } in_ip;

    /*
     * Is interface supported?
     */
    if (!iface_supported(ifname))
        return INVALID_PACKET;

    /*
     * Does returned interface (ifname) match the specified interface name
     * (ifptr)?
     */
    if (ifptr != NULL) {
        if (strcmp(ifptr, ifname) != 0) {
            return INVALID_PACKET;
        }
    }

    /*
     * Prepare ISDN reference descriptor and table.
     */

    bzero(&isdntable, sizeof(struct isdntab));
    isdn_iface_check(&isdnfd, ifname);

    /*
     * Get IPTraf link type based on returned information and move past
     * data link header.
     */
    *linktype =
        getlinktype(fromaddr->sll_hatype, ifname, isdnfd, &isdntable);
    fromaddr->sll_protocol = ntohs(fromaddr->sll_protocol);
    adjustpacket(tpacket, *linktype, packet, aligned_buf, br);

    if (*packet == NULL)
        return INVALID_PACKET;

    /*
     * Apply non-IP packet filter
     */

    if (fromaddr->sll_protocol != ETH_P_IP) {
        if ((fromaddr->sll_protocol == ETH_P_ARP) ||
            (fromaddr->sll_protocol == ETH_P_RARP)) {
            if (!nonipfilter(filter, fromaddr->sll_protocol)) {
                return PACKET_FILTERED;
            }
        } else {
            if (!nonipfilter(filter, 0)) {
                return PACKET_FILTERED;
            }
        }
        return PACKET_OK;
    }

    /*
     * TODO: Insert IPv6 processing code here
     */

    /*
     * At this point, we're now processing IP packets.  Start by getting
     * IP header and length.
     */
    ip = (struct iphdr *) (*packet);
    iphlen = ip->ihl * 4;

    /* 
     * Compute and verify IP header checksum.
     */

    ip_checksum = ip->check;
    ip->check = 0;
    hdr_check = in_cksum((u_short *) ip, iphlen);

    if (hdr_check != ip_checksum)
        return CHECKSUM_ERROR;

    if ((ip->protocol == IPPROTO_TCP || ip->protocol == IPPROTO_UDP) &&
        (sport != NULL && dport != NULL)) {
        /*
         * Process TCP/UDP fragments
         */
        if ((ntohs(ip->frag_off) & 0x3fff) != 0) {
            /*
             * total_br contains total byte count of all fragments
             * not yet retrieved.  Will differ only if fragments
             * arrived before the first fragment, in which case
             * the total accumulated fragment sizes will be returned
             * once the first fragment arrives.
             */

            if (total_br != NULL)
                *total_br = processfragment(ip, &sport_tmp, &dport_tmp,
                                            &firstin);

            if (!firstin)
                return MORE_FRAGMENTS;
        } else {
            if (ip->protocol == IPPROTO_TCP) {
                in_ip.tcp = (struct tcphdr *) ((char *) ip + iphlen);
                sport_tmp = in_ip.tcp->source;
                dport_tmp = in_ip.tcp->dest;
            } else if (ip->protocol == IPPROTO_UDP) {
                in_ip.udp = (struct udphdr *) ((char *) ip + iphlen);
                sport_tmp = in_ip.udp->source;
                dport_tmp = in_ip.udp->dest;
            } else {
                sport_tmp = 0;
                dport_tmp = 0;
            }

            if (total_br != NULL)
                *total_br = *br;
        }

        if (sport != NULL)
            *sport = sport_tmp;

        if (dport != NULL)
            *dport = dport_tmp;

        /*
         * Process IP filter
         */
        f_sport = ntohs(sport_tmp);
        f_dport = ntohs(dport_tmp);

        if ((filter->filtercode != 0) && (!ipfilter
                                          (ip->saddr, ip->daddr, f_sport,
                                           f_dport, ip->protocol,
                                           match_opposite, &(filter->fl))))
            return PACKET_FILTERED;
    } else {
        if ((filter->filtercode != 0) && (!ipfilter
                                          (ip->saddr, ip->daddr, 0, 0,
                                           ip->protocol, match_opposite,
                                           &(filter->fl))))
            return PACKET_FILTERED;
    }

    return PACKET_OK;
}

void pkt_cleanup(void)
{
    close(isdnfd);
    isdnfd = -1;
    destroyfraglist();
    destroy_isdn_table(&isdntable);
}
