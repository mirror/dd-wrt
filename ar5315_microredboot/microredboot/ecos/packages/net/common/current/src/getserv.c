//==========================================================================
//
//      lib/getserv.c
//
//      getservbyname(), getservbyport()
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
// Contributors: gthomas
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

// These must return the port in network byte order.
//
// This means treated as a short, because it's a port, despite the types in
// the API and the struct being ints.
// 
// The argument to getservbyport() is also network byte order, so that code
// must change to flip before comparing.

static struct servent services[] = {
    { "ftp",      0,   21 , "tcp" },
    { "ftp-data", 0,   20 , "tcp" },
    { "domain",   0,   53 , "udp" },
    { "tftp",     0,   69 , "udp" },
    { "ntp",      0,  123 , "udp" },
    { "snmp",     0,  161 , "udp" },

    { NULL,       0,     0       , NULL  }
};

// Note that this contains no interlocking between clients of the
// interface; but this is completely typical of such APIs.

static struct servent *
setreturned( struct servent *p )
{
    static struct servent returned;

    returned.s_name     = p->s_name;
    returned.s_aliases  = p->s_aliases;
    returned.s_port     = htons(p->s_port); // return in net order
    returned.s_proto    = p->s_proto;
    return &returned;
}

struct servent *
getservbyname(const char *name, const char *proto)
{
    struct servent *p = services;
    while (p->s_name) {
        if ((strcmp(name, p->s_name) == 0) &&
            (strcmp(proto, p->s_proto) == 0)) {
            return setreturned(p);
        }
        p++;
    }
    errno = ENOENT;
    return (struct servent *)0;
}

struct servent *
getservbyport(const int num, const char *proto)
{
    struct servent *p = services;
    int port = ntohs(num);
    while (p->s_name) {
        if ((p->s_port == port) &&
            (strcmp(proto, p->s_proto) == 0)) {
            return setreturned(p);
        }
        p++;
    }
    errno = ENOENT;
    return (struct servent *)0;
}
