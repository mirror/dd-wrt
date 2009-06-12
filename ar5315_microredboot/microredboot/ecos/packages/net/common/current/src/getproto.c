//==========================================================================
//
//      lib/getproto.c
//
//      getprotobyname(), getprotobynumber()
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <sys/param.h>
#include <netdb.h>
#include <errno.h>

static struct protoent protocols[] = {
    { "ip",         0}, // internet protocol, pseudo protocol number
    { "icmp",       1}, // internet control message protocol
    { "igmp",       2}, // Internet Group Management
    { "ggp",        3}, // gateway-gateway protocol
    { "ipencap",    4}, // IP encapsulated in IP (officially ``IP'')
    { "st",         5}, // ST datagram mode
    { "tcp",        6}, // transmission control protocol
    { "egp",        8}, // exterior gateway protocol
    { "pup",       12}, // PARC universal packet protocol
    { "udp",       17}, // user datagram protocol
    { "hmp",       20}, // host monitoring protocol
    { "xns-idp",   22}, // Xerox NS IDP
    { "rdp",       27}, // "reliable datagram" protocol
    { "iso-tp4",   29}, // ISO Transport Protocol class 4
    { "xtp",       36}, // Xpress Tranfer Protocol
    { "ddp",       37}, // Datagram Delivery Protocol
    { "idpr-cmtp", 39}, // IDPR Control Message Transport
    { "ipv6-icmp", 58}, // ICMP for IPv6
    { "rspf",      73}, // Radio Shortest Path First.
    { "vmtp",      81}, // Versatile Message Transport
    { "ospf",      89}, // Open Shortest Path First IGP
    { "ipip",      94}, // Yet Another IP encapsulation
    { "encap",     98}, // Yet Another IP encapsulation
    { 0, 0}
};

struct protoent *
getprotobyname(const char *name)
{
    struct protoent *p = protocols;
    while (p->p_name) {
        if (strcmp(name, p->p_name) == 0) {
            return p;
        }
        p++;
    }
    errno = ENOENT;
    return (struct protoent *)0;
}

struct protoent *
getprotobynumber(const int num)
{
    struct protoent *p = protocols;
    while (p->p_name) {
        if (p->p_proto == num) {
            return p;
        }
        p++;
    }
    errno = ENOENT;
    return (struct protoent *)0;
}
