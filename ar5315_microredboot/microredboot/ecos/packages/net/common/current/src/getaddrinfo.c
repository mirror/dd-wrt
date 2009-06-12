//==========================================================================
//
//      lib/getaddrinfo.c
//
//      getaddrinfo(), freeaddrinfo(), gai_strerror(), getnameinfo()
//
//==========================================================================
//####ECOSPDCOPYRIGHTBEGIN####
//
// Copyright (C) 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
// Copyright (C) 2003 andrew.lunn@ascom.ch
// All Rights Reserved.
//
// Permission is granted to use, copy, modify and redistribute this
// file.
//
//####ECOSPDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:         2002-03-05
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <sys/param.h>
#include <sys/socket.h>           // PF_xxx
#include <netinet/in.h>           // IPPROTO_xx
#include <net/netdb.h>
#include <netdb.h>                // DNS support routines
#include <errno.h>
#include <cyg/infra/cyg_ass.h>

#include <pkgconf/system.h>
#ifdef CYGPKG_NS_DNS
#include <pkgconf/ns_dns.h>
#ifdef CYGPKG_NS_DNS_BUILD
#include <cyg/ns/dns/dns.h>
#endif
#endif

extern int  sprintf(char *, const char *, ...);
extern long strtol(const char *, char **, int);
extern void *malloc(size_t);
extern void *calloc(int, size_t);
extern void free(void *);

// Allocate a new addrinfo structure and if passed an existing
// addrinfo copy all the port, protocol info into the new structure
// and then link the new onto the old.

struct addrinfo * alloc_addrinfo(struct addrinfo * ai) {
    
    struct addrinfo * nai;
    struct sockaddr * sa;

    nai = (struct addrinfo *)malloc(sizeof(struct addrinfo));
    if (!nai) {
        return NULL;
    }
    sa = (struct sockaddr *) malloc(sizeof(struct sockaddr));
    if (!sa) {
        free (nai);
        return NULL;
    }
    memset(sa,0,sizeof(*sa));

    if (ai) {
        memcpy(nai,ai,sizeof(struct addrinfo));
        ai->ai_next = nai;
    } else {
        memset(nai,0,sizeof(*nai));
    }
    nai->ai_addr = sa;
    nai->ai_addrlen = sizeof(*sa);

    return nai;
}

// getaddrinfo has not been passed a hostname. So it should use the
// loopback or the any address.
static int
no_node_addr(struct addrinfo *ai, const struct addrinfo *hints, int port) {

    switch (hints->ai_family) {
    case AF_INET: {
        struct sockaddr_in *sa = (struct sockaddr_in *) ai->ai_addr;
        if (hints->ai_flags & AI_PASSIVE) {
            sa->sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            sa->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        }
        sa->sin_len = sizeof(*sa);
        sa->sin_port = htons(port);
        sa->sin_family = AF_INET;
        ai->ai_family = AF_INET;
        break;
    }
#ifdef CYGPKG_NET_INET6
    case AF_INET6: {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *) ai->ai_addr;
        if (hints->ai_flags & AI_PASSIVE) {
            memcpy(&sa->sin6_addr, &in6addr_any, sizeof(sa->sin6_addr));
        } else {
            memcpy(&sa->sin6_addr, &in6addr_loopback, sizeof(sa->sin6_addr));
        }
        sa->sin6_len = sizeof(*sa);
        sa->sin6_port = htons(port);
        sa->sin6_family = AF_INET6;
        ai->ai_family = AF_INET6;
        break;
    }
#endif
    case PF_UNSPEC: {
        struct sockaddr_in *sa = (struct sockaddr_in *) ai->ai_addr;
        if (hints->ai_flags & AI_PASSIVE) {
            sa->sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            sa->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        }
        sa->sin_len = sizeof(*sa);
        sa->sin_port = htons(port);
        sa->sin_family = AF_INET;
        ai->ai_family = AF_INET;
#ifdef CYGPKG_NET_INET6 
        {
            struct sockaddr_in6 *sa6;
            ai=alloc_addrinfo(ai);
            if (ai == NULL) {
                return EAI_MEMORY;
            }
            sa6 = (struct sockaddr_in6 *) ai->ai_addr;
            if (hints->ai_flags & AI_PASSIVE) {
                memcpy(&sa6->sin6_addr, &in6addr_any, sizeof(sa6->sin6_addr));
            } else {
                memcpy(&sa6->sin6_addr, &in6addr_loopback, sizeof(sa6->sin6_addr));
            }
            sa6->sin6_len = sizeof(*sa);
            sa6->sin6_port = htons(port);
            sa6->sin6_family = AF_INET6;
            ai->ai_family = AF_INET6;
        }
#endif
        break;
    }
    }
   return EAI_NONE;
}


// We have been asked to convert only numeric addresses so as to not
// need a DNS server query.
static int
numeric_node_addr(struct addrinfo *ai, const char *node, 
               const struct addrinfo *hints, int port) {
    
    switch (hints->ai_family) {
    case AF_INET: {
        struct sockaddr_in *sa = (struct sockaddr_in *) ai->ai_addr;
        if (!inet_pton(AF_INET, node, (void *)&sa->sin_addr.s_addr)) {
            return EAI_FAIL;
        }
        sa->sin_port = htons(port);
        sa->sin_family = AF_INET;
        sa->sin_len = sizeof(*sa);
        ai->ai_family = AF_INET;
        break;
    }
#ifdef CYGPKG_NET_INET6
    case AF_INET6: {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *) ai->ai_addr;
        if (!inet_pton(AF_INET6, node, (void *)&sa->sin6_addr.s6_addr)) {
            return EAI_FAIL;
        }
        sa->sin6_port = htons(port);
        sa->sin6_family = AF_INET6;
        sa->sin6_len = sizeof(*sa);
        ai->ai_family = AF_INET6;
        break;
    }
#endif
    case PF_UNSPEC: {
        struct sockaddr_in *sa = (struct sockaddr_in *) ai->ai_addr;
        sa->sin_len = sizeof(*sa);
        sa->sin_port = htons(port);
        sa->sin_family = AF_INET;
        ai->ai_family = AF_INET;
        if (inet_pton(AF_INET, node, (void *)&sa->sin_addr.s_addr)) {
          return EAI_NONE;
        }
#ifdef CYGPKG_NET_INET6
        {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *) ai->ai_addr;
            sa->sin6_len = sizeof(*sa);
            sa->sin6_port = htons(port);
            sa->sin6_family = AF_INET6;
            ai->ai_family = AF_INET6;
            if (inet_pton(AF_INET6, node, (void *)&sa->sin6_addr.s6_addr)) {
                return EAI_NONE;
            }
        }
#endif
        return EAI_FAIL;
        break;
    }
    }
    return EAI_NONE;
}

// We have a host name. Use the DNS client to perform a lookup. If the
// DNS client is not part of the configuration try using the numeric
// convertion.
static int
with_node_addr(struct addrinfo *ai, const char *node, 
               const struct addrinfo *hints, int port) {
    
#ifdef CYGPKG_NS_DNS_BUILD
    struct sockaddr addrs[CYGNUM_NS_DNS_GETADDRINFO_ADDRESSES];
    int nresults;
    int i;
    char ** canon = NULL;
    
    if (hints->ai_flags & AI_CANONNAME) {
        canon = &ai->ai_canonname;
    }
    nresults = cyg_dns_getaddrinfo(node, 
                                   addrs, CYGNUM_NS_DNS_GETADDRINFO_ADDRESSES, 
                                   hints->ai_family, canon);
    if (nresults < 0) {
        return -nresults;
    }
    
    for (i=0; i < nresults; i++) {
        if (i != 0) {
            ai = alloc_addrinfo(ai);
            if (ai == NULL) {
                return EAI_MEMORY;
            }
        }
        memcpy(ai->ai_addr, &addrs[i], sizeof(addrs[i]));        
        ai->ai_family = addrs[i].sa_family;
        ai->ai_addrlen = addrs[i].sa_len;
        switch (ai->ai_family) {
        case AF_INET: {
            struct sockaddr_in *sa = (struct sockaddr_in *) ai->ai_addr;
            sa->sin_port = htons(port);
            break;
        }
#ifdef CYGPKG_NET_INET6
        case AF_INET6: {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *) ai->ai_addr;
            sa->sin6_port = htons(port);
            break;
        }
#endif
        }
    }
    return EAI_NONE;
#else
    return (numeric_node_addr(ai, node, hints, port));
#endif
}

int   
getaddrinfo(const char *nodename, const char *servname,
            const struct addrinfo *hints, struct addrinfo **res)
{
    struct addrinfo dflt_hints;
    struct protoent *proto = (struct protoent *)NULL;
    struct addrinfo *ai;
    char *protoname;
    char *endptr;
    int port = 0;
    int err;

    if (hints == (struct addrinfo *)NULL) {
        dflt_hints.ai_flags = 0;  // No special flags
        dflt_hints.ai_family = PF_UNSPEC;
        dflt_hints.ai_socktype = 0;
        dflt_hints.ai_protocol = 0;
        hints = &dflt_hints;
    }
    // Prevalidate parameters
    if ((nodename == (char *)NULL) && (servname == (char *)NULL)) {
        return EAI_NONAME;
    }
    switch (hints->ai_family) {
    case PF_UNSPEC:
    case PF_INET:
        break;
#ifdef CYGPKG_NET_INET6
    case PF_INET6:
        break;
#endif
    default:
        return EAI_FAMILY;
    }
    // Allocate the first/primary result
    *res = ai = alloc_addrinfo(NULL);
    if (ai == (struct addrinfo *)NULL) {
        return EAI_MEMORY;
    }
    // Handle request
    if (hints->ai_protocol != 0) {
        proto = getprotobynumber(hints->ai_protocol);
    }
    
    // Note: this does not handle the case where a given service can be
    // handled via multiple protocols, e.g. http/tcp & http/udp
    if (servname != (char *)NULL) {
        switch (hints->ai_socktype) {
        case 0:
        case SOCK_STREAM:
            protoname = "tcp";
            ai->ai_socktype = SOCK_STREAM;
            break;
        case SOCK_DGRAM:
            protoname = "udp";
            ai->ai_socktype = SOCK_DGRAM;
            break;
        default:
            freeaddrinfo(ai);
            return EAI_SOCKTYPE;
        }
        // See if this is just a port #
        if (((port = strtol(servname, &endptr, 0)) >= 0) &&
            (endptr > servname)) {
            ai->ai_socktype = hints->ai_socktype;
            if (hints->ai_socktype == 0) {
                // Need to have complete binding type/port
                freeaddrinfo(ai);
                return EAI_SERVICE;
            }
        } else {
            struct servent *serv = (struct servent *)NULL;
            
            serv = getservbyname(servname, protoname);
            if (serv == (struct servent *)NULL) {
                if (hints->ai_socktype == 0) {
                    protoname = "udp";
                    ai->ai_socktype = SOCK_DGRAM;    
                    serv = getservbyname(servname, protoname);
                }
            }
            if (serv == (struct servent *)NULL) {
                freeaddrinfo(ai);
                return EAI_SERVICE;
            }
            port = ntohs(serv->s_port);  
        }
        proto = getprotobyname(protoname);
        if (hints->ai_protocol && (hints->ai_protocol != proto->p_proto)) {
            freeaddrinfo(ai);
            return EAI_SOCKTYPE;
        }
        ai->ai_protocol = proto->p_proto;
    }
    
    if (nodename) {
        if (hints->ai_flags & AI_NUMERICHOST) {
            err = numeric_node_addr(ai, nodename, hints, port);
        } else {
            err = with_node_addr(ai, nodename, hints, port );
        }
    } else {
        err = no_node_addr(ai, hints, port);
    }
    
    if (err != EAI_NONE) {
        freeaddrinfo(ai);
        return err;
    }

    if ((hints->ai_flags & AI_CANONNAME) && !nodename) {
        ai->ai_canonname = malloc(strlen("localhost")+1);
        if (ai->ai_canonname) {
            strcpy(ai->ai_canonname, "localhost");
        } else {
            freeaddrinfo(ai);
            return EAI_MEMORY;
        }
    }
    
    /* The DNS code may have filled in the official address. If not
       and we have been asked for it, return an error */
    if ((hints->ai_flags & AI_CANONNAME) & !ai->ai_canonname) {
        freeaddrinfo(ai);
        return EAI_FAIL;
    }
    return EAI_NONE;  // No errors
}

// The canonname will probably point to the same memory in each
// addrinfo in the linked list. Don't free it multiple times.
void  
freeaddrinfo(struct addrinfo *ai)
{
    struct addrinfo *next = ai;
    char * last_canonname = NULL;

    while ((ai = next) != (struct addrinfo *)NULL) {
        if ((ai->ai_canonname) && 
            (ai->ai_canonname != last_canonname)) { 
            free(ai->ai_canonname);
            last_canonname = ai->ai_canonname;
        }
        if (ai->ai_addr) free(ai->ai_addr);
        next = ai->ai_next;
        free(ai);
    }
}

char 
*gai_strerror(int err)
{
    switch (err) {
    case EAI_NONE:
        return "EAI - no error";
    case EAI_AGAIN:
        return "EAI - temporary failure in name resolution";
    case EAI_BADFLAGS:
        return "EAI - invalid flags";
    case EAI_FAIL:
        return "EAI - non-recoverable failure in name resolution";
    case EAI_FAMILY:
        return "EAI - family not supported";
    case EAI_MEMORY:
        return "EAI - memory allocation failure";
    case EAI_NONAME:
        return "EAI - hostname nor servname provided, or not known";
    case EAI_SERVICE:
        return "EAI - servname not supported for socket type";
    case EAI_SOCKTYPE:
        return "EAI - socket type not supported";
    case EAI_SYSTEM:
        return "EAI - system error";
    case EAI_BADHINTS:
        return "EAI - inconsistent hints";
    case EAI_PROTOCOL:
        return "EAI - bad protocol";
    default:
        return "EAI - unknown error";
    }
}

// Set of flags implemented
#define NI_MASK (NI_NUMERICHOST|NI_NUMERICSERV|NI_NOFQDN|NI_NAMEREQD|NI_DGRAM)

int 
getnameinfo (const struct sockaddr *sa, socklen_t salen, 
             char *host, socklen_t hostlen, 
             char *serv, socklen_t servlen, 
             unsigned int flags)
{
    int port;
    char *s;
    struct servent *se;
    int error;
    int numeric = (flags & NI_NUMERICHOST);

    if ((flags & ~NI_MASK) != 0) {
        return EAI_BADFLAGS;
    }
    switch (sa->sa_family) {
    case PF_INET:
#ifdef CYGPKG_NET_INET6
    case PF_INET6:
#endif
        if (host != (char *)NULL) {
            if ( !numeric) {
                error = EAI_NONAME;
#ifdef CYGPKG_NS_DNS_BUILD
                error = -cyg_dns_getnameinfo(sa, host,hostlen);
#endif
                if ((error == EAI_NONAME) && (flags & NI_NAMEREQD)) {
                    return EAI_NONAME;
                }
                // If lookup failed, try it as numeric address
                numeric = !(error == EAI_NONE);
            }
            if (numeric) {
                s = _inet_ntop((struct sockaddr *)sa, host, hostlen);
                if (!s) {
                    return EAI_FAIL;
                }
            }
            if (!numeric && flags & NI_NOFQDN) {
                s = index(host, '.');
                if (s) {
                    *s = '\0';
                }
            }
        }
    
        if (serv != (char *)NULL) {
            port = _inet_port((struct sockaddr *)sa);
            if (!port) {
                return EAI_FAIL;
            }
            se = (struct servent *)NULL;
            if ((flags & NI_NUMERICSERV) == 0) {
                if ((flags & NI_DGRAM) == 0) {
                    se = getservbyport(htons(port), "tcp");
                }
                if (se == (struct servent *)NULL) {
                    se = getservbyport(htons(port), "udp");
                }
            }
            if (se != (struct servent *)NULL) {
                diag_snprintf(serv,servlen, "%s/%s", se->s_name, se->s_proto);
            } else {
                diag_snprintf(serv,servlen, "%d", port);
            }
        }
        break;
    default:
        return EAI_FAMILY;
    }
    return EAI_NONE;
}
