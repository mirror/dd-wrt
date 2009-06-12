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
// Copyright (C) 2003 Gary Thomas
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-09-26
// Description: Provides DNS lookup as per RFC 1034/1035.
// 
// Note:        This is a stripped down clone of dns.c from the CYGPKG_NS_DNS
//              package which does not use malloc/free and has been tweaked to
//              use UDP via RedBoot's network stack. Also adds commands
//              to set the DNS server IP at runtime.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/drv_api.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         /* Tracing support */

#include <net/net.h>
#include <redboot.h>
/* #include <cyg/ns/dns/dns.h> - it's been moved to redboot.h */
#include <cyg/ns/dns/dns_priv.h>

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <flash_config.h>

RedBoot_config_option("DNS server IP address",
                      dns_ip,
                      ALWAYS_ENABLED, true,
                      CONFIG_IP,
                      0
    );
#endif

/* So we remember which ports have been used */
static int get_port = 7700;

#define DOMAIN_PORT           53

/* Some magic to make dns_impl.inl compile under RedBoot */
#define sprintf diag_sprintf

/* DNS server address possibly returned from bootp */
struct in_addr __bootp_dns_addr;
cyg_bool __bootp_dns_set = false;

struct sockaddr_in server;

/* static buffers so we can make do without malloc */
static struct hostent _hent;
static char* _h_addr_list[2];
static struct in_addr _h_addr_list0;
static int _hent_alloc = 0;

#define _STRING_COUNT  2
#define _STRING_LENGTH 64
static char _strings[_STRING_COUNT][_STRING_LENGTH];
static int _strings_alloc = 0;

/* as in dns.c proper */
static short id = 0;              /* ID of the last query */
static int s = -1;                /* Socket to the DNS server */
static cyg_drv_mutex_t dns_mutex; /* Mutex to stop multiple queries as once */
static char * domainname=NULL;    /* Domain name used for queries */


/* Allocate space for string of length (len). Return NULL on
   failure. */
static char*
alloc_string(int len)
{
    int i;

    if (len > _STRING_LENGTH)
        return NULL;

    for (i = 0; i < _STRING_COUNT; i++) {
        if (_strings_alloc & (1 << i)) continue;
        _strings_alloc |= (1<<i);
        return _strings[i];
    }
    return NULL;
}

static void
free_string(char* s)
{
    int i;
    for (i = 0; i < _STRING_COUNT; i++) {
        if (_strings[i] == s) {
            _strings_alloc &= ~(1<<i);
            break;
        }
    }
}

/* Deallocate the memory taken to hold a hent structure */
static void
free_hent(struct hostent * hent)
{
    if (hent->h_name) {
        free_string(hent->h_name);
    }
    _hent_alloc = 0;
}

/* Allocate hent structure with room for one in_addr. Returns NULL on
   failure. */
static struct hostent*
alloc_hent(void)
{
    struct hostent *hent;

    if (_hent_alloc) return NULL;

    hent = &_hent;
    memset(hent, 0, sizeof(struct hostent));
    hent->h_addr_list = _h_addr_list;
    hent->h_addr_list[0] = (char*)&_h_addr_list0;
    hent->h_addr_list[1] = NULL;
    _hent_alloc = 1;

    return hent;
}

static __inline__ void
free_stored_hent(void)
{
    free_hent( &_hent );
}

static __inline__ void
store_hent(struct hostent *hent)
{
    hent=hent; // avoid warning
}

/* Send the query to the server and read the response back. Return -1
   if it fails, otherwise put the response back in msg and return the
   length of the response. */
static int 
send_recv(char * msg, int len, int msglen)
{
    struct dns_header *dns_hdr;
    int finished = false;
    int read = 0;

    dns_hdr = (struct dns_header *) msg;

    do { 
        int len_togo = len;
        struct timeval timeout;
        struct sockaddr_in local_addr, from_addr;

        memset((char *)&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_addr.sin_port = htons(get_port++);

        if (__udp_sendto(msg, len_togo, &server, &local_addr) < 0)
            return -1;

        memset((char *)&from_addr, 0, sizeof(from_addr));

        timeout.tv_sec = CYGNUM_REDBOOT_NETWORKING_DNS_TIMEOUT;
        timeout.tv_usec = 0;

        read = __udp_recvfrom(msg, len, &from_addr, &local_addr, &timeout);
        if (read < 0)
            return -1;

        /* Reply to an old query. Ignore it */
        if (ntohs(dns_hdr->id) != (id-1)) {
            continue;
        }
        finished = true;
    } while (!finished);

    return read;
}
    
void
set_dns(char* new_ip)
{
    in_addr_t dns_ip;

    memset(&server.sin_addr, 0, sizeof(server.sin_addr));
    if (!inet_aton(new_ip, &dns_ip)) {
        diag_printf("Bad DNS server address: %s\n", new_ip);
    } else {
        memcpy(&server.sin_addr, &dns_ip, sizeof(dns_ip));
        /* server config is valid */
        s = 0;
    }
}

void
show_dns(void)
{
    diag_printf(", DNS server IP: %s", inet_ntoa((in_addr_t *)&server.sin_addr));
    if (0 == server.sin_addr.s_addr) {
        s = -1;
    }
}

/* Initialise the resolver. Open a socket and bind it to the address
   of the server.  return -1 if something goes wrong, otherwise 0 */
int  
redboot_dns_res_init(void)
{
    memset((char *)&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(DOMAIN_PORT);
    cyg_drv_mutex_init(&dns_mutex);

    /* If we got a DNS server address from the DHCP/BOOTP, then use that address */
    if ( __bootp_dns_set ) {
	memcpy(&server.sin_addr, &__bootp_dns_addr, sizeof(__bootp_dns_addr) );
	s = 0;
    }
    else {
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    {
        ip_addr_t dns_ip;

        flash_get_config("dns_ip", &dns_ip, CONFIG_IP);
        if (dns_ip[0] == 0 && dns_ip[1] == 0 && dns_ip[2] == 0 && dns_ip[3] == 0)
            return -1;
        memcpy(&server.sin_addr, &dns_ip, sizeof(dns_ip));
        /* server config is valid */
        s = 0;
    }
#else
      // Use static configuration
	set_dns(__Xstr(CYGPKG_REDBOOT_NETWORKING_DNS_IP));
#endif
    }

    return 0;
}

/* Include the DNS client implementation code */
#include <cyg/ns/dns/dns_impl.inl>
