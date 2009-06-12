//=============================================================================
//
//      dns.c
//
//      DNS client code
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   andrew.lunn
// Contributors:andrew.lunn, jskov
// Date:        2001-09-18
// Description: Provides DNS lookup as per RFC 1034/1035.
// 
// Note:        Does only support A and PTR lookups. Maybe add for other
//              types as well?
//
//              parse_answer() only returns the first found record.
//
//              Add tracing and assertions.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/kapi.h>
#endif
#include <cyg/hal/drv_api.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         /* Tracing support */

#include <netdb.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cyg/ns/dns/dns_priv.h>

static short id = 0;              /* ID of the last query */
static int s = -1;                /* Socket to the DNS server */
static cyg_drv_mutex_t dns_mutex; /* Mutex to stop multiple queries as once */
static cyg_ucount32 ptdindex;     /* Index for the per thread data */
static char * domainname=NULL;    /* Domain name used for queries */

/* Allocate space for string of length len. Return NULL on failure. */
static inline char*
alloc_string(int len)
{
    return malloc(len);
}

static inline void
free_string(char* s)
{
    free(s);
}

/* Deallocate the memory taken to hold a hent structure */
static void
free_hent(struct hostent * hent)
{
    if (hent->h_name) {
        free_string(hent->h_name);
    }

    if (hent->h_addr_list) {
        int i = 0;
        while (hent->h_addr_list[i]) {
            free(hent->h_addr_list[i]);
            i++;
        }
        free(hent->h_addr_list);
    }
    free(hent);
}

/* Allocate hent structure with room for one in_addr. Returns NULL on
   failure. */
static struct hostent*
alloc_hent(void)
{
    struct hostent *hent;

    hent = malloc(sizeof(struct hostent));
    if (hent) {
        memset(hent, 0, sizeof(struct hostent));
        hent->h_addr_list = malloc(sizeof(char *)*2);
        if (!hent->h_addr_list) {
            free_hent(hent);
            return NULL;
        }
        hent->h_addr_list[0] = malloc(sizeof(struct in_addr));
        if (!hent->h_addr_list[0]) {
            free_hent(hent);
            return NULL;
        }
        hent->h_addr_list[1] = NULL;
    }

    return hent;
}

/* Thread destructor used to free stuff stored in per-thread data slot. */
static void
thread_destructor(CYG_ADDRWORD data)
{
    struct hostent *hent;
    hent = (struct hostent *)cyg_thread_get_data(ptdindex);
    if (hent)
        free_hent(hent);
    return;
    data=data;
}

/* Store the hent away in the per-thread data. */
static void
store_hent(struct hostent *hent)
{
    // Prevent memory leaks by setting a destructor to be
    // called on thread exit to free per-thread data.
    cyg_thread_add_destructor( &thread_destructor, 0 );
    cyg_thread_set_data(ptdindex, (CYG_ADDRWORD)hent);
}

/* If there is an answer to an old query, free the memory it uses. */
static void
free_stored_hent(void)
{
    struct hostent *hent;
    hent = (struct hostent *)cyg_thread_get_data(ptdindex);
    if (hent) {
        free_hent(hent);
        cyg_thread_set_data(ptdindex, (CYG_ADDRWORD)NULL);
        cyg_thread_rem_destructor( &thread_destructor, 0 );
    }
}

/* Send the query to the server and read the response back. Return -1
   if it fails, otherwise put the response back in msg and return the
   length of the response. */
static int 
send_recv(char * msg, int len, int msglen)
{
    struct dns_header *dns_hdr;
    struct timeval timeout;
    int finished = false;
    int backoff = 1;
    fd_set readfds;
    int written;
    int ret;

    CYG_REPORT_FUNCNAMETYPE( "send_recv", "returning %d" );
    CYG_REPORT_FUNCARG3( "msg=%08x, len=%d, msglen", msg, len, msglen );

    CYG_CHECK_DATA_PTR( msg, "msg is not a valid pointer!" );

    dns_hdr = (struct dns_header *) msg;

    do { 
        written = write(s, msg, len);
        if (written < 0) {
            ret = -1;
            break;
        }

        FD_ZERO(&readfds);
        FD_SET(s, &readfds);
    
        timeout.tv_sec = backoff;
        timeout.tv_usec = 0;
        backoff = backoff << 1;

        ret = select(s+1, &readfds, NULL, NULL, &timeout);
        if (ret < 0) {
            ret = -1;
            break;
        }
        /* Timeout */
        if (ret == 0) {
            if (backoff > 16) {
                h_errno = TRY_AGAIN;
                ret = -1;
                break;
            }
        }
        if (ret == 1) {
            ret = read(s, msg, msglen);
            if (ret < 0) {
                ret = -1;
                break;
            }
      
            /* Reply to an old query. Ignore it */
            if (ntohs(dns_hdr->id) != (id-1)) {
                continue;
            }
            finished = true;
        }
    } while (!finished);

    CYG_REPORT_RETVAL( ret );

    return ret;
}

/* Include the DNS client implementation code */
#include <cyg/ns/dns/dns_impl.inl>

/* (re)Start the DNS client. This opens a socket to the DNS server
   who's address is passed in. This address can be either an IPv4 or
   IPv6 address. This function can be called multiple times.  If we
   are being called a second time we have to be careful to allow any
   ongoing lookups to finish before we close the socket and connect to
   a different DNS server. The danger here is that we may have to wait
   for up to 32 seconds if the DNS server is down.
   Each invocation will close any previous connection and make a new
   connection to the new server. Note the address of the server must
   be in numeric form since its not possible to do a DNS lookup! */

int cyg_dns_res_start(char * dns_server) {

    static int init =0;
    struct addrinfo * res;
    int err;

    CYG_REPORT_FUNCNAMETYPE( "cyg_dns_res_start", "returning %d" );
    CYG_REPORT_FUNCARG1( "dns_server=%s", dns_server );

    CYG_CHECK_DATA_PTR( dns_server, "dns_server is not a valid pointer!" );

    if (init) {
      cyg_drv_mutex_lock(&dns_mutex);
      cyg_thread_free_data_index(ptdindex);
      if (s >= 0) {
        close(s);
      }
    } else {
      init = 1;
      cyg_drv_mutex_init(&dns_mutex);
      cyg_drv_mutex_lock(&dns_mutex);
    }
    
    err = getaddrinfo(dns_server,"domain", NULL, &res);
    if (err != 0) {
        cyg_drv_mutex_unlock(&dns_mutex);
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
  
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == -1) {
        cyg_drv_mutex_unlock(&dns_mutex);
        freeaddrinfo(res);
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
        s = -1;
        cyg_drv_mutex_unlock(&dns_mutex);
        freeaddrinfo(res);
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    ptdindex = cyg_thread_new_data_index();  
  
    cyg_drv_mutex_unlock(&dns_mutex);
    freeaddrinfo(res);
    CYG_REPORT_RETVAL( 0 );
    return 0;
}

/* This is the old interface to start the resolver. It is only IPv4
   capable and so is now deprecated in favor of cyg_dns_res_start().
   Initialise the resolver. Open a socket and bind it to the
   address of the server.  return -1 if something goes wrong,
   otherwise 0. If we are being called a second time we have to be
   careful to allow any ongoing lookups to finish before we close the
   socket and connect to a different DNS server. The danger here is
   that we may have to wait for up to 32 seconds if the DNS server is
   down.  */
int  
cyg_dns_res_init(struct in_addr *dns_server)
{
  char name[20];
  unsigned char *bytes = (unsigned char *)dns_server;
  int ret;

  CYG_REPORT_FUNCNAMETYPE( "cyg_dns_res_init", "returning %d" );
  CYG_REPORT_FUNCARG1( "dns_server=%08x", dns_server );

  diag_sprintf(name, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
  
  ret = cyg_dns_res_start(name);
  
  CYG_REPORT_RETVAL( ret );
  return ret;
}

/* add_answer checks to see if we already have this answer and if not,
   adds it to the answers. */
static int 
add_answer(char *rdata, short rr_type, int family, 
           struct sockaddr addrs[], int num, int used) {
    int i;
    int found = 0;
    
    for (i = 0; i < used ; i++) {
        if ((addrs[i].sa_family == family) &&
            !memcmp(addrs[i].sa_data, rdata, addrs[i].sa_len)) {
            found = 1;
            break;
        }
    }
    if (!found) {
        memset(&addrs[used],0,sizeof(addrs[used]));
        addrs[used].sa_family = family;
        
        switch(family) {
        case AF_INET: {
            struct sockaddr_in * addr = (struct sockaddr_in *) &addrs[used];
            addr->sin_len = sizeof(*addr);
            memcpy(&addr->sin_addr, rdata, sizeof(struct in_addr));
            used++;
            break;
        }
#ifdef CYGPKG_NET_INET6
        case AF_INET6: {
            struct sockaddr_in6 * addr = (struct sockaddr_in6 *) &addrs[used];
            addr->sin6_len = sizeof(*addr);
            memcpy(&addr->sin6_addr, rdata, sizeof(struct in6_addr));
            used++;
            break;
        }
#endif
        default:
            used = -EAI_FAMILY;
        }
    }
    return used;
}
      
/* This decodes the answer and puts the results into the addrs
   array. This function can deal with IPv6 AAAA records as well as A
   records. Thus its more complex than the parse_answer function in
   the inline code. This complexity is only needed by getaddrinfo, so
   i decided to leave parse_anser alone. */

static int
decode(char *msg, short rr_type, int family, 
       struct sockaddr addrs[], int num, int used, char **canon) {
  
    struct dns_header *dns_hdr;
    struct resource_record rr, *rr_p = NULL;
    char *qname = NULL;
    char *ptr;
    
    dns_hdr = (struct dns_header *)msg;
    
    if (DNS_REPLY_NAME_ERROR == dns_hdr->rcode) {
        h_errno = HOST_NOT_FOUND;
        return -EAI_NONAME;
    }
    
    if ((dns_hdr->qr != 1) ||
        (dns_hdr->opcode != DNS_QUERY)) {
      return -EAI_FAIL;
    }
    
    if (dns_hdr->rcode != DNS_REPLY_NOERR) {
      return -EAI_NONAME;
    }

    dns_hdr->ancount = ntohs(dns_hdr->ancount);
    dns_hdr->qdcount = ntohs(dns_hdr->qdcount);
    ptr = (char *)&dns_hdr[1];
    
    /* Skip over the query section */
    if (dns_hdr->qdcount > 0) {
        while (dns_hdr->qdcount) {
            ptr += qname_len(ptr);
            ptr += 4;                   /* skip type & class */
            dns_hdr->qdcount--;
        }
    }  
    
    /* Read the answers resource records to find an answer of the
       correct type. */
    while (dns_hdr->ancount && (used >= 0) && (used < num)) {
        qname = ptr;
        ptr += qname_len(ptr);
        rr_p = (struct resource_record *)ptr;
        memcpy(&rr, ptr, sizeof(rr));
        if ((rr.rr_type == htons(rr_type)) && 
            (rr.class == htons(DNS_CLASS_IN))) {
            used = add_answer(rr_p->rdata, rr_type, family, addrs, num, used);
            if (canon && !*canon) {
                *canon = real_name(msg,qname);
            }
        }
        ptr += sizeof(struct resource_record) - 
            sizeof(rr.rdata) + ntohs(rr.rdlength);
        dns_hdr->ancount--;
    }
    if (used == 0) {
        return -EAI_NONAME;
    }
    return used;
}

/* Do a lookup for a particular type of resource record. */
static int 
do_lookup (const char * hostname, 
           struct sockaddr addrs[], int num, int used, 
           short rr_type, int family, char **canon) {

    unsigned char msg[MAXDNSMSGSIZE];
    int error;
    int len;

    /* First try the name as passed in */
    memset(msg, 0, sizeof(msg));
    len = build_query(msg, hostname, rr_type);
    if (len < 0) {
        return -EAI_FAIL;
    }
    
    /* Send the query and wait for an answer */
    len = send_recv(msg, len, sizeof(msg));
    if (len < 0) {
        return -EAI_FAIL;
    }
    
    /* Decode the answer */
    error = decode(msg, rr_type, family, addrs, num, used, canon);
    return error;
}

static int do_lookups(const char * hostname, 
                      struct sockaddr addrs[], int num, 
                      int family, char ** canon) {
    int error;
#ifdef CYGPKG_NET_INET6
    int error6;
#endif

    switch (family) {
    case AF_INET:
        error = do_lookup(hostname, addrs, num, 0, DNS_TYPE_A, AF_INET, canon);
        break;
#ifdef CYGPKG_NET_INET6
    case AF_INET6:
        error = do_lookup(hostname, addrs, num, 0, DNS_TYPE_AAAA, AF_INET6, canon);
        break;
#endif
    case PF_UNSPEC:
#ifndef CYGPKG_NET_INET6
        error = do_lookup(hostname, addrs, num, 0, DNS_TYPE_A, AF_INET, canon);
#else 
#ifdef CYGOPT_NS_DNS_FIRST_FAMILTY_AF_INET
        error = do_lookup(hostname, addrs, num, 0, DNS_TYPE_A, AF_INET, canon);
        if (error > 0 ) {
            error6 = do_lookup(hostname, addrs, num, error, DNS_TYPE_AAAA, 
                               AF_INET6, canon);
        } else {
            error6 = do_lookup(hostname, addrs, num, 0, DNS_TYPE_AAAA, 
                               AF_INET6, canon);
        }
        if (error6 > 0) {
            error = error6;
        }
#else // CYGOPT_NS_DNS_FIRST_FAMILY_AF_INET
        error6 = do_lookup(hostname, addrs, num, 0, DNS_TYPE_AAAA, 
                           AF_INET6, canon);
        if (error6> 0 ) {
            error = do_lookup(hostname, addrs, num, error6, DNS_TYPE_A, 
                               AF_INET, canon);
        } else {
            error = do_lookup(hostname, addrs, num, 0, DNS_TYPE_A, 
                               AF_INET, canon);
        }
#endif // CYGOPT_NS_DNS_FIRST_FAMILY_AF_INET
#endif // CYGPKG_NET_INET6
        break;
    default:
        error = -EAI_FAMILY;
    }
    return error;
}

/* This implements the interface between getaddrinfo and the dns
   client. hostent is not used here since that only works with IPv4
   addresses, where as this function needs to be protocol
   independent. */
int 
cyg_dns_getaddrinfo(const char * hostname, 
                    struct sockaddr addrs[], int num,
                    int family,
                    char ** canon)
{
    int error;
    char name[256];
    char * dot;

    CYG_REPORT_FUNCNAMETYPE( "cyg_dns_getaddrinfo", "returning %08x" );
    CYG_REPORT_FUNCARG3( "hostname=%08x, addrs=%08x, num=%2d", 
                         hostname, addrs, num );
    
    if ( !hostname || !addrs || !num ) {
        CYG_REPORT_RETVAL( NULL );
        return -EAI_FAIL;
    }
    
    CYG_CHECK_DATA_PTR( hostname, "hostname is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( addrs, "addrs is not a valid pointer!");
    CYG_ASSERT( num > 0, "Invalid number of sockaddr stuctures");
    
    if (!valid_hostname(hostname)) {
        /* it could be a dot address */
        struct sockaddr_in * sa4 = (struct sockaddr_in *)&addrs[0];
        memset(&addrs[0],0,sizeof(struct sockaddr));
        if (inet_pton(AF_INET, hostname, (char *)&sa4->sin_addr.s_addr)) {
            sa4->sin_family = AF_INET;
            sa4->sin_len = sizeof(*sa4);
            CYG_REPORT_RETVAL (1);
            return 1;
        }
#ifdef CYGPKG_NET_INET6
        {
            /* it could be a colon address */
            struct sockaddr_in6 * sa6 = (struct sockaddr_in6 *)&addrs[0];
            memset(&addrs[0],0,sizeof(struct sockaddr));
            if (inet_pton(AF_INET6, hostname, (char *)&sa6->sin6_addr.s6_addr)) {
                sa6->sin6_family = AF_INET6;
                sa6->sin6_len = sizeof(*sa6);
                CYG_REPORT_RETVAL (1);
                return 1;
            }
        }
#endif
        CYG_REPORT_RETVAL (-EAI_NONAME);
        return -EAI_NONAME;
    }
    
    /* Has the socket to the DNS server been opened? */
    if (s < 0) {
        CYG_REPORT_RETVAL( -EAI_FAIL );
        return -EAI_FAIL;
    }
    
    if (domainname) {
        if ((strlen(hostname) + strlen(domainname)) > 254) {
            cyg_drv_mutex_unlock(&dns_mutex);
            CYG_REPORT_RETVAL( -EAI_FAIL );
            return -EAI_FAIL;
        }
        strcpy(name, hostname);
        strcat(name, ".");
        strcat(name, domainname);
    }
    cyg_drv_mutex_lock(&dns_mutex);

    /* If the hostname ends with . it a FQDN. Don't bother adding the
    domainname. If it does not contain a . , try appending with the
    domainname first. If it does have a . , try without a domain name
    first. */

    dot = rindex(hostname,'.');
    if (dot) {
        if (*(dot+1) == '\0') {
            /* FQDN */
            error = do_lookups(hostname, addrs, num, family, canon);
        } else {
          /* Dot somewhere */
          error = do_lookups(hostname, addrs, num, family, canon);
          if (domainname && (error == -EAI_NONAME)) { 
            error = do_lookups(name, addrs, num, family, canon);
          }
        }
    } else {
    /* No Dot. Try adding domainname first */
        error = -EAI_NONAME;
        if (domainname) {
            error = do_lookups(name, addrs, num, family, canon);
        }
        if (error == -EAI_NONAME) {
            error = do_lookups(hostname, addrs, num, family, canon);
        }
    }
    cyg_drv_mutex_unlock(&dns_mutex);
    CYG_REPORT_RETVAL( error );
    return error;
}

/* This implements the interface between getnameinfo and the dns
   client. */
externC int
cyg_dns_getnameinfo(const struct sockaddr * sa, char * host, size_t hostlen) 
{
    char hostname[80];
    unsigned char msg[MAXDNSMSGSIZE];
    struct hostent * hent;
    int len;

    CYG_REPORT_FUNCNAMETYPE( "cyg_dns_getnameinfo", "returning %08x" );
    CYG_REPORT_FUNCARG3( "sa=%08x, host=%08x, hostlen=%3d", 
                         sa, host, hostlen );
    
    CYG_CHECK_DATA_PTR( sa, "sa is not a valid pointer");
    CYG_CHECK_DATA_PTR( host, "host is not a valid data pointer");
    CYG_ASSERT(hostlen >0, "Invalid host length");
    
    /* Has the socket to the DNS server been opened? */
    if (s < 0) {
        CYG_REPORT_RETVAL( -EAI_FAIL );
        return -EAI_FAIL;
    }
    
    cyg_drv_mutex_lock(&dns_mutex);
    
    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in * sa4 = (struct sockaddr_in *)sa;
        unsigned char * addr = (char *)&sa4->sin_addr.s_addr;
        sprintf(hostname, "%d.%d.%d.%d.IN-ADDR.ARPA.",
                addr[3],addr[2],addr[1],addr[0]);
        break;
    }
#ifdef CYGPKG_NET_INET6
    case AF_INET6: {
        struct sockaddr_in6 * sa6 = (struct sockaddr_in6 *)sa;
        int i;
        
        for (i=15; i >= 0; i--) {
            sprintf(&hostname[(15*2*2) - (i*2*2)], "%x.%x.",
                    sa6->sin6_addr.s6_addr[i] & 0x0f,
                    (sa6->sin6_addr.s6_addr[i] & 0xf0) >> 4);
        }
        sprintf(&hostname[16*2*2],"IP6.INT");
        break;
    }
#endif
    default:
        cyg_drv_mutex_lock(&dns_mutex);
        CYG_REPORT_RETVAL( -EAI_FAMILY);
        return -EAI_FAMILY;
    }

    memset(msg, 0, sizeof(msg));
  
    /* Build a PTR type request using the hostname */
    len = build_query(msg, hostname, DNS_TYPE_PTR);
    if (len < 0) {
        cyg_drv_mutex_unlock(&dns_mutex);
        CYG_REPORT_RETVAL( -EAI_FAIL );
        return -EAI_FAIL;
    }

    /* Send the request and wait for an answer */
    len = send_recv(msg, len, sizeof(msg));
    if (len < 0) {
        cyg_drv_mutex_unlock(&dns_mutex);
        CYG_REPORT_RETVAL( -EAI_FAIL );
        return -EAI_FAIL;
    }
 
    /* Parse the answer for the host name */
    hent = parse_answer(msg, DNS_TYPE_PTR);
    
    /* If no name is known return an error */
    if (!hent) {
        cyg_drv_mutex_unlock(&dns_mutex);
        CYG_REPORT_RETVAL( -EAI_NONAME );
        return -EAI_NONAME;
    }
    
    /* Otherwise copy it into our results buffer and tidy up */
    strncpy(host, hent->h_name,hostlen);
    free_hent(hent);
    
    cyg_drv_mutex_unlock(&dns_mutex);
    CYG_REPORT_RETVAL( -EAI_NONE );
    return -EAI_NONE;
}
