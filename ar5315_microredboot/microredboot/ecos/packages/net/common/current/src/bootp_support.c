//==========================================================================
//
//      lib/bootp_support.c
//
//      Minimal BOOTP functions
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// BOOTP support (and a little DHCP support also)

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/isoinfra.h>

#include <network.h>
#include <errno.h>
#ifdef CYGPKG_NET_FREEBSD_STACK  // New layout
#include <net/if_var.h>
#include <netinet/in_var.h>
#include <netinet6/nd6.h>
#endif

#ifdef CYGINT_ISO_DNS
#include <netdb.h>
#endif
#ifdef CYGPKG_NET_SNTP
#include <cyg/sntp/sntp.h>
#endif

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

// This function sets up the interface it the simplest configuration.
// Just enough to broadcast a BOOTP request and get a response.
// It returns 'true' if a response was obtained.
cyg_bool_t
do_bootp(const char *intf, struct bootp *recv)
{
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    struct sockaddr_in cli_addr, serv_addr, bootp_server_addr;
    struct ecos_rtentry route;
    int s=-1, addrlen;
    int one = 1;
    struct bootp bootp_xmit;
    unsigned char mincookie[] = {99,130,83,99,255} ;
    struct timeval tv;
    cyg_bool_t retcode = false;

    // Ensure clean slate
    cyg_route_reinit();  // Force any existing routes to be forgotten

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
        goto out;
    }

    addrp = (struct sockaddr_in *) &ifr.ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = INADDR_ANY;

    strcpy(ifr.ifr_name, intf);
    if (ioctl(s, SIOCSIFADDR, &ifr)) {
        perror("SIOCSIFADDR");
        goto out;
    }

    if (ioctl(s, SIOCSIFNETMASK, &ifr)) {
        perror("SIOCSIFNETMASK");
        goto out;
    }

    /* the broadcast address is 255.255.255.255 */
    memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
    if (ioctl(s, SIOCSIFBRDADDR, &ifr)) {
        perror("SIOCSIFBRDADDR");
        goto out;
    }

    ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
        perror("SIOCSIFFLAGS");
        goto out;
    }

    if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        perror("SIOCGIFHWADDR");
        goto out;
    }

    // Set up routing
    /* the broadcast address is 255.255.255.255 */
    memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
    memset(&route, 0, sizeof(route));
    memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = INADDR_ANY;
    memcpy(&route.rt_dst, addrp, sizeof(*addrp));
    memcpy(&route.rt_genmask, addrp, sizeof(*addrp));

    route.rt_dev = ifr.ifr_name;
    route.rt_flags = RTF_UP|RTF_GATEWAY;
    route.rt_metric = 0;

    if (ioctl(s, SIOCADDRT, &route)) {
        if (errno != EEXIST) {
            perror("SIOCADDRT 3");
            goto out;
        }
    }

    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(IPPORT_BOOTPC);
    
    if(bind(s, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        perror("bind error");
        goto out;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        perror("setsockopt SO_REUSEADDR");
        goto out;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        perror("setsockopt SO_REUSEPORT");
        goto out;
    }
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    serv_addr.sin_port = htons(IPPORT_BOOTPS);

    // Fill in the BOOTP request
    bzero(&bootp_xmit, sizeof(bootp_xmit));
    if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        perror("SIOCGIFHWADDR");
        goto out;
    }
    bootp_xmit.bp_htype = HTYPE_ETHERNET;
    bootp_xmit.bp_hlen = IFHWADDRLEN;
    bcopy(ifr.ifr_hwaddr.sa_data, &bootp_xmit.bp_chaddr, bootp_xmit.bp_hlen);

    bootp_xmit.bp_secs = 0;
    bcopy(mincookie, bootp_xmit.bp_vend, sizeof(mincookie));

    bootp_xmit.bp_op = BOOTREQUEST;

    if(sendto(s, &bootp_xmit, sizeof(struct bootp), 0, 
              (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("sendto error");
        goto out;
    }

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    // Everything passed here is a success.
    retcode = true;
    
    addrlen = sizeof(bootp_server_addr);
    if (recvfrom(s, recv, sizeof(struct bootp), 0,
                 (struct sockaddr *)&bootp_server_addr, &addrlen) < 0) {
        // This is an "acceptable" error, it means there is no server for
        // us: do not initialize the interface.
        retcode = false;
    }

    // Shut things down regardless of success of rx, otherwise other
    // interfaces cannot be initialised!
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = INADDR_ANY;

    strcpy(ifr.ifr_name, intf);
    if (ioctl(s, SIOCDIFADDR, &ifr)) {
        perror("SIOCDIFADDR");
    }

    // Shut down interface so it can be reinitialized
    ifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
    if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
        perror("SIOCSIFFLAGS");
        retcode = false;
    }

 out:
    // All done with socket
    if (s != -1)
      close(s);
    return retcode;
}

static char *_bootp_op[] = {"", "REQUEST", "REPLY"};
static char *_bootp_hw_type[] = {"", "Ethernet", "Exp Ethernet", "AX25",
                                     "Pronet", "Chaos", "IEEE802", "Arcnet"};

static char *_dhcpmsgs[] = {"","DISCOVER", "OFFER", "REQUEST", "DECLINE",
                           "ACK", "NAK", "RELEASE" };

void
show_bootp(const char *intf, struct bootp *bp)
{
    int i, len;
    unsigned char *op, *ap = 0, optover;
    unsigned char name[128];
    struct in_addr addr[32];
    unsigned int length;
    
    diag_printf("BOOTP[%s] op: %s\n", intf, _bootp_op[bp->bp_op]);
    diag_printf("       htype: %s\n", _bootp_hw_type[bp->bp_htype]);
    diag_printf("        hlen: %d\n", bp->bp_hlen );
    diag_printf("        hops: %d\n", bp->bp_hops );
    diag_printf("         xid: 0x%x\n", bp->bp_xid );
    diag_printf("        secs: %d\n", bp->bp_secs );
    diag_printf("       flags: 0x%x\n", bp->bp_flags );
    diag_printf("       hw_addr: ");
    for (i = 0;  i < bp->bp_hlen;  i++) {
        diag_printf("%02x", bp->bp_chaddr[i]);
        if (i != (bp->bp_hlen-1)) diag_printf(":");
    }
    diag_printf("\n");
    diag_printf("     client IP: %s\n", inet_ntoa(bp->bp_ciaddr));
    diag_printf("         my IP: %s\n", inet_ntoa(bp->bp_yiaddr));
    diag_printf("     server IP: %s\n", inet_ntoa(bp->bp_siaddr));
    diag_printf("    gateway IP: %s\n", inet_ntoa(bp->bp_giaddr));

    optover = 0; // See whether sname and file are overridden for options
    length = sizeof(optover);
    (void)get_bootp_option( bp, TAG_DHCP_OPTOVER, &optover, &length );
    if ( !(1 & optover) && bp->bp_sname[0] )
        diag_printf("        server: %s\n", bp->bp_sname);
    if ( ! (2 & optover) && bp->bp_file[0] )
        diag_printf("          file: %s\n", bp->bp_file);
    if (bp->bp_vend[0]) {
        diag_printf("  options:\n");
        op = &bp->bp_vend[4];
        while (*op != TAG_END) {
            switch (*op) {
            case TAG_PAD:
                op++;
                continue;
            case TAG_SUBNET_MASK:
            case TAG_GATEWAY:
            case TAG_IP_BROADCAST:
            case TAG_DOMAIN_SERVER:
                ap = (unsigned char *)&addr[0];
                len = *(op+1);
                for (i = 0;  i < len;  i++) {
                    *ap++ = *(op+i+2);
                }
                if (*op == TAG_SUBNET_MASK)   ap =  "  subnet mask";
                if (*op == TAG_GATEWAY)       ap =  "      gateway";
                if (*op == TAG_IP_BROADCAST)  ap =  " IP broadcast";
                if (*op == TAG_DOMAIN_SERVER) ap =  "domain server";
                diag_printf("      %s: ", ap);
                ap = (unsigned char *)&addr[0];
                while (len > 0) {
                    diag_printf("%s", inet_ntoa(*(struct in_addr *)ap));
                    len -= sizeof(struct in_addr);
                    ap += sizeof(struct in_addr);
                    if (len) diag_printf(", ");
                }
                diag_printf("\n");
                break;
            case TAG_DOMAIN_NAME:
            case TAG_HOST_NAME:
                for (i = 0;  i < *(op+1);  i++) {
                    name[i] = *(op+i+2);
                }
                name[*(op+1)] = '\0';
                if (*op == TAG_DOMAIN_NAME) ap =  " domain name";
                if (*op == TAG_HOST_NAME)   ap =  "   host name";
                diag_printf("       %s: %s\n", ap, name);
                break;
            case TAG_DHCP_MESS_TYPE:
                diag_printf("        DHCP message: %d %s\n",
                            op[2], _dhcpmsgs[op[2]] );
                break;
            case TAG_DHCP_REQ_IP:
                diag_printf("        DHCP requested ip: %d.%d.%d.%d\n",
                            op[2], op[3], op[4], op[5] );  
                break;
            case TAG_DHCP_LEASE_TIME   :
            case TAG_DHCP_RENEWAL_TIME :
            case TAG_DHCP_REBIND_TIME  :
                diag_printf("        DHCP time %d: %d\n",
                            *op, ((((((op[2]<<8)+op[3])<<8)+op[4])<<8)+op[5]) );

                break;
            case TAG_DHCP_SERVER_ID    :
                diag_printf("        DHCP server id: %d.%d.%d.%d\n",
                            op[2], op[3], op[4], op[5] );  
                break;

            case TAG_DHCP_OPTOVER      :
            case TAG_DHCP_PARM_REQ_LIST:
            case TAG_DHCP_TEXT_MESSAGE :
            case TAG_DHCP_MAX_MSGSZ    :
            case TAG_DHCP_CLASSID      :
            case TAG_DHCP_CLIENTID     :
                diag_printf("        DHCP option: %x/%d.%d:", *op, *op, *(op+1));
                if ( 1 == op[1] )
                    diag_printf( " %d", op[2] );
                else if ( 2 == op[1] )
                    diag_printf( " %d", (op[2]<<8)+op[3] );
                else if ( 4 == op[1] )
                    diag_printf( " %d", ((((((op[2]<<8)+op[3])<<8)+op[4])<<8)+op[5]) );
                else
                    for ( i = 2; i < 2 + op[1]; i++ )
                        diag_printf(" %d",op[i]);
                diag_printf("\n");
                break;
            case TAG_NTP_SERVER:
              diag_printf("        NTP servers: ");
              for ( i = 0 ; i < op[1]/4 ; i++) {
                diag_printf("%d.%d.%d.%d ",
                            op[1+i*4+1], op[1+i*4+2], 
                            op[1+i*4+3], op[1+i*4+4]);
              }
              diag_printf("\n");
              break;
            default:
                diag_printf("Unknown option: %x/%d.%d:", *op, *op, *(op+1));
                for ( i = 2; i < 2 + op[1]; i++ )
                    diag_printf(" %d",op[i]);
                diag_printf("\n");
                break;
            }                
            op += *(op+1)+2;
        }
    }
}

cyg_bool_t
get_bootp_option(struct bootp *bp, unsigned char tag, void *opt, 
                 unsigned int *length)
{
    unsigned char *val = (unsigned char *)opt;
    int i;
    cyg_uint8 optover;
    
#define SCANTAG( ptr ) CYG_MACRO_START          \
    unsigned int max;                           \
    unsigned char *op = (ptr);                  \
    while (*op != TAG_END) {                    \
        if (*op == tag) {                       \
            max=(*(op+1)>*length ? *length : *(op+1)); \
            for (i = 0;  i < max;  i++) {       \
                *val++ = *(op+i+2);             \
            }                                   \
            *length=max;                        \
            return true;                        \
        }                                       \
        if (*op == TAG_PAD) {                   \
            op++;                               \
        } else {                                \
            op += *(op+1)+2;                    \
        }                                       \
    }                                           \
CYG_MACRO_END

    SCANTAG( &bp->bp_vend[4] );

    if ( TAG_DHCP_OPTOVER == tag ) // prevent recursion > once
        return false;
    // else, look for that tag to see if there's more...
    optover = 0;
    if ( ! get_bootp_option( bp, TAG_DHCP_OPTOVER, &optover, length) )
        return false;

    if ( 1 & optover ) // then the file field also holds options
        SCANTAG( &bp->bp_file[0] );

    if ( 2 & optover ) // then the sname field also holds options
        SCANTAG( &bp->bp_sname[0] );

    return false;
}

// [Re]initialize the network interface with the info passed from BOOTP
cyg_bool_t
init_net(const char *intf, struct bootp *bp)
{
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    int s=-1;
    int one = 1;
    struct ecos_rtentry route;
    struct in_addr netmask, gateway;
    unsigned int length;
    int retcode = false;
    
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
        goto out;
    }

    addrp = (struct sockaddr_in *) &ifr.ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr = bp->bp_yiaddr;  // The address BOOTP gave us

    // Must do this temporarily with default route and netmask so that
    // [sub]netmask can be set.
    strcpy(ifr.ifr_name, intf);
    if (ioctl(s, SIOCSIFADDR, &ifr)) {
        perror("SIOCIFADDR");
        goto out;
    }

    length = sizeof(addrp->sin_addr);
    if (get_bootp_option(bp, TAG_SUBNET_MASK, &addrp->sin_addr,&length)) {
        netmask = addrp->sin_addr;
        if (ioctl(s, SIOCSIFNETMASK, &ifr)) {
            perror("SIOCSIFNETMASK");
            goto out;
        }
        // Must do this again so that [sub]netmask (and so default route)
        // is taken notice of.
        addrp->sin_addr = bp->bp_yiaddr;  // The address BOOTP gave us
        if (ioctl(s, SIOCSIFADDR, &ifr)) {
            perror("SIOCIFADDR 2");
            goto out;
        }
    }

    length = sizeof(addrp->sin_addr);    
    if (get_bootp_option(bp, TAG_IP_BROADCAST, &addrp->sin_addr,&length)) {
        if (ioctl(s, SIOCSIFBRDADDR, &ifr)) {
            perror("SIOCSIFBRDADDR");
            goto out;
        }
        // Do not re-set the IFADDR after this; doing *that* resets the
        // BRDADDR to the default!
    }

    ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
        perror("SIOCSIFFLAGS");
        goto out;
    }

    // Set up routing
    length = sizeof(addrp->sin_addr);
    if (get_bootp_option(bp, TAG_GATEWAY, &gateway,&length)) {
        // ...and it's a nonzero address...
        if ( 0 != gateway.s_addr ) {
            memset(&route, 0, sizeof(route));
            addrp->sin_family = AF_INET;
            addrp->sin_port = 0;
            addrp->sin_len = sizeof(*addrp);
            addrp->sin_addr.s_addr = 0; // Use 0,0,GATEWAY for the default route
            memcpy(&route.rt_dst, addrp, sizeof(*addrp));
            addrp->sin_addr.s_addr = 0;
            memcpy(&route.rt_genmask, addrp, sizeof(*addrp));
            addrp->sin_addr = gateway;
            memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

            route.rt_dev = ifr.ifr_name;
            route.rt_flags = RTF_UP|RTF_GATEWAY;
            route.rt_metric = 0;

            if (ioctl(s, SIOCADDRT, &route)) {
                diag_printf("Route - dst: %s",
                  inet_ntoa(((struct sockaddr_in *)&route.rt_dst)->sin_addr));
                diag_printf(", mask: %s",
                  inet_ntoa(((struct sockaddr_in *)&route.rt_genmask)->sin_addr));
                diag_printf(", gateway: %s\n",
                  inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
                if (errno != EEXIST) {
                    perror("SIOCADDRT 3");
                    goto out;
                }
            }
        }
    }
    retcode = true;
    
#ifdef CYGINT_ISO_DNS
    {
#define MAX_IP_ADDR_LEN 16
        char buf[BP_MAX_OPTION_LEN+1];  
        memset(buf,0,sizeof(buf));
        length = sizeof(buf);
        if (get_bootp_option(bp, TAG_DOMAIN_NAME, buf, &length)) {
            setdomainname(buf, length);
        }
        length = sizeof(buf);
        if (get_bootp_option(bp, TAG_DOMAIN_SERVER, buf, &length)) {
            cyg_dns_res_init((struct in_addr *)buf);
        }
    }
#endif

#ifdef CYGNUM_NET_SNTP_UNICAST_MAXDHCP
    {
        struct in_addr dhcp_addrs[CYGNUM_NET_SNTP_UNICAST_MAXDHCP];

        /* Removed any previously registered addresses */
        cyg_sntp_set_servers(NULL, 0);

        /* See if we received any NTP servers from DHCP */
        length = sizeof(dhcp_addrs);
        if (get_bootp_option(bp, TAG_NTP_SERVER, &dhcp_addrs[0], &length))
        {
        	static struct sockaddr ntp_servers[CYGNUM_NET_SNTP_UNICAST_MAXDHCP];
            struct servent *service;
        	cyg_uint32 num;

            /* See how many addresses we got.  The length should always
             * be a multiple of 4, but cut off any extra bytes and
             * use what we got.
             */
            length /= sizeof(struct in_addr);

            /* Fill out a sockaddr array for the NTP client */
            service = getservbyname("ntp", "udp");
            CYG_CHECK_DATA_PTR(service, "NTP service not found.");
            memset(&ntp_servers[0], 0, sizeof(ntp_servers));
            for (num = 0; num < length; num++)
            {
				struct sockaddr_in *saddr = (struct sockaddr_in *)&ntp_servers[num];

                saddr->sin_len = sizeof(*saddr);
                saddr->sin_family = AF_INET;
                saddr->sin_port = service->s_port;  /* Already network-endian */
                saddr->sin_addr = dhcp_addrs[num];  /* Already network-endian */
            }

            /* Configure the client with the array */
            cyg_sntp_set_servers(&ntp_servers[0], num);
        }
    }
#endif /* CYGNUM_NET_SNTP_UNICAST_MAXDHCP */
 out:
    if (s != -1) 
      close(s);
    return retcode;
}

#ifdef INET6
extern const struct in6_addr in6mask128;
cyg_bool_t
init_net_IPv6(const char *intf, struct bootp *bp, char *prefix)
{
    int s =-1 ;
    struct in6_aliasreq in6_addr;
    char in6_ip[128];
    int retcode = false;

    // Set up non link-layer address
    s = socket(AF_INET6, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket IPv6");
        goto out;
    }
    bzero(&in6_addr, sizeof(in6_addr));
    diag_sprintf(in6_ip, "%s::%s", prefix, inet_ntoa(bp->bp_yiaddr));
    in6_addr.ifra_addr.sin6_len = sizeof(struct sockaddr_in6);
    in6_addr.ifra_addr.sin6_family = AF_INET6;
    if (!inet_pton(AF_INET6, in6_ip, (char *)&in6_addr.ifra_addr.sin6_addr)) {
        diag_printf("Can't set IPv6 address: %s\n", in6_ip);
        goto out;
    }
    in6_addr.ifra_prefixmask.sin6_len = sizeof(struct sockaddr_in6);
    in6_addr.ifra_prefixmask.sin6_family = AF_INET6;
    in6_addr.ifra_prefixmask.sin6_addr = in6mask128;
    strcpy(in6_addr.ifra_name, intf);
    in6_addr.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
    in6_addr.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;
    if (ioctl(s, SIOCAIFADDR_IN6, &in6_addr)) {
        perror("SIOCAIFADDR_IN6");
        goto out;
    }
    retcode = true;
 out:
    if (s != -1) 
      close(s);
    return retcode;
}
#endif

// EOF bootp_support.c
