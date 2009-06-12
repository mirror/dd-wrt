//==========================================================================
//
//      include/netdb.h
//
//      eCos implementations of network "database" functions
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

//
// Support for various "network databases"
//

#ifndef _NETDB_H_
#define _NETDB_H_

#include <sys/param.h>

#ifdef __cplusplus
extern "C" {
#endif

// Internet protocols
struct protoent {
    char *p_name;
    int   p_proto;
};

struct protoent *getprotobyname(const char *);
struct protoent *getprotobynumber(const int);

// Internet services
struct servent {
    char    *s_name;        /* official service name */
    char    **s_aliases;    /* alias list */
    int     s_port;         /* port number */
    char    *s_proto;       /* protocol to use */
};

struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);

// Name/address manipulation
struct addrinfo {
        int     ai_flags;       /* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
        int     ai_family;      /* PF_xxx */
        int     ai_socktype;    /* SOCK_xxx */
        int     ai_protocol;    /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
        size_t  ai_addrlen;     /* length of ai_addr */
        char    *ai_canonname;  /* canonical name for hostname */
        struct  sockaddr *ai_addr;      /* binary address */
        struct  addrinfo *ai_next;      /* next structure in linked list */
};

/*
 * Error return codes from getaddrinfo(), getnameinfo()
 */
#define EAI_NONE         0      /* valid return - no errors */
#define EAI_AGAIN        2      /* temporary failure in name resolution */
#define EAI_BADFLAGS     3      /* invalid value for ai_flags */
#define EAI_FAIL         4      /* non-recoverable failure in name resolution */
#define EAI_FAMILY       5      /* ai_family not supported */
#define EAI_MEMORY       6      /* memory allocation failure */
#define EAI_NONAME       8      /* hostname nor servname provided, or not known */
#define EAI_SERVICE      9      /* servname not supported for ai_socktype */
#define EAI_SOCKTYPE    10      /* ai_socktype not supported */
#define EAI_SYSTEM      11      /* system error returned in errno */
#define EAI_BADHINTS    12      /* inconsistent hints */
#define EAI_PROTOCOL    13
#define EAI_MAX         14

/*
 * Flag values for getaddrinfo()
 */
#define AI_PASSIVE      0x00000001 /* get address to use bind() */
#define AI_CANONNAME    0x00000002 /* fill ai_canonname */
#define AI_NUMERICHOST  0x00000004 /* prevent name resolution */
/* valid flags for addrinfo */
#define AI_MASK \
    (AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_ADDRCONFIG)

#define AI_ALL          0x00000100 /* IPv6 and IPv4-mapped (with AI_V4MAPPED) */
#define AI_V4MAPPED_CFG 0x00000200 /* accept IPv4-mapped if kernel supports */
#define AI_ADDRCONFIG   0x00000400 /* only if any address is assigned */
#define AI_V4MAPPED     0x00000800 /* accept IPv4-mapped IPv6 address */
/* special recommended flags for getipnodebyname */
#define AI_DEFAULT      (AI_V4MAPPED_CFG | AI_ADDRCONFIG)

int   getaddrinfo(const char *, const char *,
                  const struct addrinfo *, struct addrinfo **);
void  freeaddrinfo(struct addrinfo *);
char *gai_strerror(int);

/*
 * Support for getnameinfo()
 */

#define NI_MAXHOST      1025
#define NI_MAXSERV      32

#define NI_NUMERICHOST 1       /* Don't try to look up hostname.  */
#define NI_NUMERICSERV 2       /* Don't convert port number to name.  */
#define NI_NOFQDN      4       /* Only return nodename portion.  */
#define NI_NAMEREQD    8       /* Don't return numeric addresses.  */
#define NI_DGRAM       16      /* Look up UDP service rather than TCP.  */

/* Translate a socket address to a location and service name.  */
int getnameinfo (const struct sockaddr *sa, socklen_t salen, 
                 char *host, socklen_t hostlen, 
                 char *serv, socklen_t servlen, 
                 unsigned int flags);

// Miscellaneous address manipulation functions
#include <netinet/in.h>
char     *inet_ntoa(struct in_addr);
char     *inet_ntoa_r(struct in_addr, char *);
char     *inet_ntop(int af, const char *src, char *dst, size_t len);
int      inet_pton(int af, const char *src, char *dst);
char     *_inet_ntop(struct sockaddr *sa, char *dst, size_t len);
u_int16_t _inet_port(struct sockaddr *sa);

#ifdef __cplusplus
}
#endif
#endif // _NETDB_H_
