//==========================================================================
//
//      tests/dns1.c
//
//      Simple test of DNS client support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Andrew Lunn
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
// Author(s):    andrew.lunn
// Contributors: andrew.lunn, jskov
// Date:         2001-09-18
// Purpose:      
// Description:  Test DNS functions. Note that the structure below that
//               control what addresses the test uses. These must be
//               changed to match the particular testing network in which
//               the test is to be run.
//              
//####DESCRIPTIONEND####
//
//==========================================================================
#include <pkgconf/ns_dns.h>

#include <network.h>
#include <netdb.h>

#include <arpa/inet.h>

#include <cyg/infra/testcase.h>

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

#define NELEM(x) (sizeof(x) / sizeof(x[0]))

struct test_info_s {
    char * dns_server_v4;
    char * dns_server_v6;
    char * domain_name;
    char * hostname_v4;
    char * cname_v4;
    char * ip_addr_v4;
    char * hostname_v6;
    char * cname_v6;
    char * ip_addr_v6;
    char * hostname_v46;
    char * cname_v46;
    char * ip_addr_v46_v4;
    char * ip_addr_v46_v6;
};

struct test_info_s test_info[] = {
#if CYGPKG_NS_DNS_TESTS_ELSIS
    {
        "194.249.198.85",
        NULL,
        "test.ecos.",
        "hostnamev4",
        "cnamev4",
        "192.168.88.1",
        "hostnamev6",
        "cnamev6",
        "fec0::88:4:3:2:1",
        "hostnamev46",
        "cnamev46",
        "192.168.88.2",
        "fec0::88:4:3:2:2"
    },
#endif
#ifdef CYGPKG_NS_DNS_TESTS_LUNN
    {
        "80.238.139.98",
        "3ffe:bc0:8000::839",
        "test.ecos.",
        "hostnamev4",
        "cnamev4",
        "192.168.88.1",
        "hostnamev6",
        "cnamev6",
        "fec0::88:4:3:2:1",
        "hostnamev46",
        "cnamev46",
        "192.168.88.2",
        "fec0::88:4:3:2:2"
    }
#endif
};

char * familytoa(int family) {
    
    switch (family) {
    case AF_INET:
        return "AF_INET";
#ifdef CYGPKG_NET_INET6
    case AF_INET6:
        return "AF_INET6";
#endif
    default:
        return "Unknown";
    }
}

void dns_test(struct test_info_s *info) {
    struct in_addr addr;
    struct hostent *hent;
    char dn[256];
    char name[256];
    char cname[256];
    char buffer[256];
    char buff[64];
    size_t hostlen = 128;
    char host[hostlen];
    struct addrinfo * res;
    struct addrinfo hints;
    struct sockaddr_in sa4;
#ifdef CYGPKG_NET_INET6
    struct sockaddr_in6 sa6;
    struct addrinfo *ai4, *ai6;
#endif
    int error;
    
    if (inet_pton(AF_INET, info->dns_server_v4, (void *)&addr) < 0) {
      CYG_TEST_FAIL_FINISH("Error with DNS server address");
    }
    cyg_dns_res_init(&addr);

    setdomainname(NULL,0);
    
    inet_aton(info->ip_addr_v4, &addr);
    strcpy(name,info->hostname_v4);
    strcat(name,".");
    strcat(name,info->domain_name);
    strcpy(cname,info->cname_v4);
    strcat(cname,".");
    strcat(cname,info->domain_name);
    
    // Lookup the IPv4 FQDN hostname 
    hent = gethostbyname(name);
    if (hent != NULL) {
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s>", 
                     name,
                     hent->h_name, 
                     inet_ntoa(*(struct in_addr *)hent->h_addr));
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void*)&addr, 
                                        (void*)(hent->h_addr), 
                                        sizeof(struct in_addr))),
                           "IPv4 FQDN hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, hent->h_name)), 
                           "IPv4 FQDN hostname name");
    } else {
        diag_sprintf(buffer,"IPv4 FQDN hostname: error %s", 
                     hstrerror(h_errno));
        CYG_TEST_FAIL(buffer);
    }
    
    // Lookup the IPv4 FQDN cname
    hent = gethostbyname(cname);
    if (hent != NULL) {
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s>", 
                     cname,
                     hent->h_name, 
                     inet_ntoa(*(struct in_addr *)hent->h_addr));
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void*)&addr, 
                                        (void*)(hent->h_addr), 
                                        sizeof(struct in_addr))),
                           "IPv4 FQDN cname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, hent->h_name)), 
                           "IPv4 FQDN hostname name");
    } else {
        diag_sprintf(buffer,"IPv4 FQDN cname: error %s", hstrerror(h_errno));
        CYG_TEST_FAIL(buffer);
    }
    
    // Lookup the IP address as a string.  This does not require a DNS
    // lookup. Just turn the value into binary
    hent = gethostbyname(info->ip_addr_v4);
    if (hent != NULL) {
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s>", 
                     info->ip_addr_v4,
                     hent->h_name, 
                     inet_ntoa(*(struct in_addr *)hent->h_addr));
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void*)&addr, 
                                        (void*)(hent->h_addr), 
                                        sizeof(struct in_addr))),
                           "IPv4 IP address");
        CYG_TEST_PASS_FAIL((0 == strcmp(info->ip_addr_v4, hent->h_name)), 
                           "IPv4 IP address name");
        
    } else {
        diag_sprintf(buffer,"IPv4 IP address: error %s", hstrerror(h_errno));
        CYG_TEST_FAIL(buffer);
    }
    
    // Reverse lookup the IPv4 address, expect the FQDN hostname
    hent = gethostbyaddr((char *)&addr, sizeof(struct in_addr), AF_INET);
    if (hent != NULL) {
        inet_ntop(AF_INET,(void *)&addr, buff, sizeof(buff));
        diag_sprintf(buffer,"Reverse lookup %s: Result <%s is %s>", 
                     buff,
                     hent->h_name, 
                     inet_ntoa(*(struct in_addr *)hent->h_addr));
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void*)&addr, 
                                        (void*)(hent->h_addr), 
                                        sizeof(struct in_addr))),
                           "Reverse lookup IPv4 IP address");
        
        CYG_TEST_PASS_FAIL((0 == strcmp(name, hent->h_name)), 
                           "Reverse lookup IPv4 IP address name");
    } else {
        diag_sprintf(buffer,"Reverse lookup IPv4 IP address: error %s", 
                     hstrerror(h_errno));
        CYG_TEST_FAIL(buffer);
    }
    
    // Setup a domainname. We now don't have to use fully qualified
    // domain names
    setdomainname(info->domain_name, strlen(info->domain_name));
    getdomainname(dn, sizeof(dn));
    CYG_TEST_PASS_FAIL(0 == strcmp(info->domain_name, dn), 
                       "{get|set}domainname");
    
    // Make sure FQDN still work
    strcpy(name,info->hostname_v4);
    strcat(name,".");
    strcat(name,info->domain_name);
    
    hent = gethostbyname(name);
    if (hent != NULL) {
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s>", 
                     name,
                     hent->h_name, 
                     inet_ntoa(*(struct in_addr *)hent->h_addr));
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void*)&addr, 
                                        (void*)(hent->h_addr), 
                                        sizeof(struct in_addr))),
                           "IPv4 FQDN hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, hent->h_name)), 
                           "IPv4 FQDN hostname name");
    } else {
        diag_sprintf(buffer,"IPv4 FQDN hostname: error %s", 
                     hstrerror(h_errno));
        CYG_TEST_FAIL(buffer);
    }
    
    // Now just the hostname
    hent = gethostbyname(info->hostname_v4);
    if (hent != NULL) {
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s>", 
                     info->hostname_v4,
                     hent->h_name, 
                     inet_ntoa(*(struct in_addr *)hent->h_addr));
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void*)&addr, 
                                        (void*)(hent->h_addr), 
                                        sizeof(struct in_addr))),
                           "IPv4 hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, hent->h_name)), 
                           "IPv4 hostname name");
    } else {
        diag_sprintf(buffer,"IPv4 hostname: error %s", hstrerror(h_errno));
        CYG_TEST_FAIL(buffer);
    }
    
    // Run the same tests as above, but this time use getaddrinfo and
    // getnameinfo.
    setdomainname(NULL,0);
    
    memset(&hints,0,sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    memset(&sa4,0,sizeof(sa4));
    memcpy(&sa4.sin_addr, &addr, sizeof(addr));
    sa4.sin_family=AF_INET;
    sa4.sin_len = sizeof(sa4);
    
    // Lookup the IPv4 FQDN hostname 
    error = getaddrinfo(name, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     name,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)res->ai_addr,
                                        sizeof(sa4))) &&
                           (res->ai_family == AF_INET),
                           "IPv4 FQDN hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv4 FQDN hostname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv4 FQDN hostname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv4 FQDN hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    
    // Lookup the IPv4 FQDN cname 
    error = getaddrinfo(cname, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     cname,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)res->ai_addr,
                                        sizeof(sa4))) &&
                           (res->ai_family == AF_INET),
                           "IPv4 FQDN cname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv4 FQDN cname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv4 FQDN cname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv4 FQDN cname: error %s", gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    
    // Lookup the IP address as a string.  This does not require a DNS
    // lookup. Just turn the value into binary
    hints.ai_flags = AI_NUMERICHOST;
    error = getaddrinfo(info->ip_addr_v4, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s %s>", 
                     info->ip_addr_v4,
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)res->ai_addr,
                                        sizeof(sa4))) &&
                           (res->ai_family == AF_INET),
                           "IPv4 IP address");
        CYG_TEST_PASS_FAIL((NULL == res->ai_canonname), 
                           "IPv4 IP address name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv4 IP address one result");        
        // Don't free results - use for next test.
    } else {
        diag_sprintf(buffer,"IPv4 IP address: error %s", gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }

    // Reverse lookup the IPv4 address, expect the FQDN hostname
    error = getnameinfo(res->ai_addr,res->ai_addrlen,
                       host, hostlen, NULL, 0, 0);
    if (error == EAI_NONE) {
        diag_sprintf(buffer, "IPv4 Reverse lookup FDQN %s: Result %s",
                    info->ip_addr_v4,
                    host);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == strcmp(name, host)), 
                           "IPv4 Reverse lookup FQDN");
    } else {
        diag_sprintf(buffer,"IPv4 Reverse lookup FQDN: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    // Reverse lookup the IPv4 address, expect just the hostname part
    error = getnameinfo(res->ai_addr,res->ai_addrlen,
                       host, hostlen, NULL, 0, NI_NOFQDN);
    if (error == EAI_NONE) {
        diag_sprintf(buffer, "IPv4 Reverse lookup hostname part %s: Result %s",
                    info->ip_addr_v4,
                    host);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == strcmp(info->hostname_v4, host)), 
                           "IPv4 Reverse lookup hostname part");
    } else {
        diag_sprintf(buffer,"IPv4 Reverse lookup hostname part: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    freeaddrinfo(res);

    // Setup a domainname. We now don't have to use fully qualified
    // domain names
    setdomainname(info->domain_name, strlen(info->domain_name));
    getdomainname(dn, sizeof(dn));
    CYG_TEST_PASS_FAIL(0 == strcmp(info->domain_name, dn), 
                       "{get|set}domainname");
    
    // Lookup the IPv4 FQDN hostname to make sure it still works
    hints.ai_flags = AI_CANONNAME;
    error = getaddrinfo(name, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     name,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)res->ai_addr,
                                        sizeof(sa4))) &&
                           (res->ai_family == AF_INET),
                           "IPv4 FQDN hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv4 FQDN hostname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv4 FQDN hostname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv4 FQDN hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    
    // Now the host name without the domain name 
    error = getaddrinfo(info->hostname_v4, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     info->hostname_v4,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)res->ai_addr,
                                        sizeof(sa4))) &&
                           (res->ai_family == AF_INET),
                           "IPv4 hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv4 hostname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv4 hostname one result");        
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv4 hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
#ifdef CYGPKG_NET_INET6
    // Check we have the needed information for the IPv6 tests
    if (!info->hostname_v6 || !info->cname_v6 || !info->ip_addr_v6) {
        return;
    }

    setdomainname(NULL,0);
    
    strcpy(name,info->hostname_v6);
    strcat(name,".");
    strcat(name,info->domain_name);
    strcpy(cname,info->cname_v6);
    strcat(cname,".");
    strcat(cname,info->domain_name);
    
    memset(&sa6,0,sizeof(sa6));
    sa6.sin6_family = AF_INET6;
    sa6.sin6_len = sizeof(sa6);
    inet_pton(AF_INET6, info->ip_addr_v6, (void *)&sa6.sin6_addr);

    // Lookup the IPv6 FQDN hostname 
    error = getaddrinfo(name, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     name,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)res->ai_addr,
                                        sizeof(sa6))) &&
                           (res->ai_family == AF_INET6),
                           "IPv6 FQDN hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv6 FQDN hostname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv6 FQDN hostname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv6 FQDN hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }

    // Lookup the IPv6 FQDN cname 
    error = getaddrinfo(cname, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     cname,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)res->ai_addr,
                                        sizeof(sa6))) &&
                           (res->ai_family == AF_INET6),
                           "IPv6 FQDN cname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv6 FQDN cname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv6 FQDN cname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv6 FQDN cname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }

    // Lookup the IP address as a string.  This does not require a DNS
    // lookup. Just turn the value into binary
    hints.ai_flags = AI_NUMERICHOST;
    error = getaddrinfo(info->ip_addr_v6, NULL, &hints, &res);

    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s %s>", 
                     info->ip_addr_v6,
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)res->ai_addr,
                                        sizeof(sa6))) &&
                           (res->ai_family == AF_INET6),
                           "IPv6 IP address");
        CYG_TEST_PASS_FAIL((NULL == res->ai_canonname), 
                           "IPv6 IP address name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv6 IP address one result");
        // Don't free the result, use it in the next tests
    } else {
        diag_sprintf(buffer,"IPv6 IP address: error %s", gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }

    // Reverse lookup the IPv6 address, expect the FQDN hostname
    error = getnameinfo(res->ai_addr,res->ai_addrlen,
                       host, hostlen, NULL, 0, 0);
    if (error == EAI_NONE) {
        diag_sprintf(buffer, "IPv6 Reverse lookup FDQN %s: Result %s",
                    info->ip_addr_v6,
                    host);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == strcmp(name, host)), 
                           "IPv6 Reverse lookup FQDN");
    } else {
        diag_sprintf(buffer,"IPv6 Reverse lookup FQDN: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    // Reverse lookup the IPv6 address, expect just the hostname part
    error = getnameinfo(res->ai_addr,res->ai_addrlen,
                       host, hostlen, NULL, 0, NI_NOFQDN);
    if (error == EAI_NONE) {
        diag_sprintf(buffer, "IPv6 Reverse lookup hostname part %s: Result %s",
                    info->ip_addr_v6,
                    host);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == strcmp(info->hostname_v6, host)), 
                           "IPv6 Reverse lookup hostname part");
    } else {
        diag_sprintf(buffer,"IPv6 Reverse lookup hostname part: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    freeaddrinfo(res);

    // Setup a domainname. We now don't have to use fully qualified
    // domain names
    setdomainname(info->domain_name, strlen(info->domain_name));
    getdomainname(dn, sizeof(dn));
    CYG_TEST_PASS_FAIL(0 == strcmp(info->domain_name, dn), 
                       "{get|set}domainname");

    // Lookup the IPv6 FQDN hostname to make sure it still works
    hints.ai_flags = AI_CANONNAME;
    error = getaddrinfo(name, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     name,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)res->ai_addr,
                                        sizeof(sa6))) &&
                           (res->ai_family == AF_INET6),
                           "IPv6 FQDN hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv6 FQDN hostname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv6 FQDN hostname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv6 FQDN hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }

    // Now the host name without the domain name
    error = getaddrinfo(info->hostname_v6, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
        getnameinfo(res->ai_addr, res->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     info->hostname_v6,
                     res->ai_canonname, 
                     familytoa(res->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)res->ai_addr,
                                        sizeof(sa6))) &&
                           (res->ai_family == AF_INET6),
                           "IPv6 hostname address");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, res->ai_canonname)), 
                           "IPv6 hostname name");
        CYG_TEST_PASS_FAIL((NULL == res->ai_next),
                           "IPv6 hostname one result");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv6 hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }

    // Check if we have the information to do tests which result in two
    // answers, an AF_INET and an AF_INET6
    if (!info->hostname_v46 || !info->cname_v46 || 
        !info->ip_addr_v46_v4 || !info->ip_addr_v46_v6 ) {
        return;
    }
    setdomainname(NULL,0);
    
    strcpy(name,info->hostname_v46);
    strcat(name,".");
    strcat(name,info->domain_name);
    strcpy(cname,info->cname_v46);
    strcat(cname,".");
    strcat(cname,info->domain_name);

    inet_aton(info->ip_addr_v46_v4, &addr);
    memset(&sa4,0,sizeof(sa4));
    memcpy(&sa4.sin_addr, &addr, sizeof(addr));
    sa4.sin_family=AF_INET;
    sa4.sin_len = sizeof(sa4);
    
    memset(&sa6,0,sizeof(sa6));
    sa6.sin6_family = AF_INET6;
    sa6.sin6_len = sizeof(sa6);
    inet_pton(AF_INET6, info->ip_addr_v46_v6, (void *)&sa6.sin6_addr);
    
    // Lookup the IPv4 and IPv6 FQDN hostname 
    error = getaddrinfo(name, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
#ifdef CYGOPT_NS_DNS_FIRST_FAMILY_AF_INET6
        ai6 = res;
        ai4 = res->ai_next;
        CYG_TEST_PASS_FAIL((NULL != ai6->ai_next),
                           "IPv6 FQDN hostname not one result");
        CYG_TEST_PASS_FAIL((NULL == ai4->ai_next),
                           "IPv4 & IPv6 FQDN hostname two results");
#else
        ai4 = res;
        ai6 = res->ai_next;
        CYG_TEST_PASS_FAIL((NULL != ai4->ai_next),
                           "IPv6 FQDN hostname not one result");
        CYG_TEST_PASS_FAIL((NULL == ai6->ai_next),
                           "IPv4 & IPv6 FQDN hostname two results");
#endif
        getnameinfo(ai4->ai_addr, ai4->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     name,
                     ai4->ai_canonname, 
                     familytoa(ai4->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)ai4->ai_addr,
                                        sizeof(sa4))) &&
                           (ai4->ai_family == AF_INET),
                           "IPv4 & IPv6 FQDN hostname address IPv4");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, ai4->ai_canonname)), 
                           "IPv4 & IPv6 FQDN hostname name");
        getnameinfo(ai6->ai_addr, ai6->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s %s>", 
                     name,
                     familytoa(ai6->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)ai6->ai_addr,
                                        sizeof(sa6))) &&
                           (ai6->ai_family == AF_INET6),
                           "IPv4 & IPv6 FQDN hostname address IPv6");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv4 & IPv6 FQDN hostname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    
    // Lookup the IPv4 and IPv6 FQDN cname 
    error = getaddrinfo(cname, NULL, &hints, &res);
    
    if (error == EAI_NONE) {
#ifdef CYGOPT_NS_DNS_FIRST_FAMILY_AF_INET6
        ai6 = res;
        ai4 = res->ai_next;
        CYG_TEST_PASS_FAIL((NULL != ai6->ai_next),
                           "IPv6 FQDN hostname not one result");
        CYG_TEST_PASS_FAIL((NULL == ai4->ai_next),
                           "IPv4 & IPv6 FQDN hostname two results");
#else
        ai4 = res;
        ai6 = res->ai_next;
        CYG_TEST_PASS_FAIL((NULL != ai4->ai_next),
                           "IPv6 FQDN hostname not one result");
        CYG_TEST_PASS_FAIL((NULL == ai6->ai_next),
                           "IPv4 & IPv6 FQDN hostname two results");
#endif
        getnameinfo(ai4->ai_addr, ai4->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s is %s %s>", 
                     cname,
                     ai4->ai_canonname, 
                     familytoa(ai4->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                        (void*)ai4->ai_addr,
                                        sizeof(sa4))) &&
                           (ai4->ai_family == AF_INET),
                           "IPv4 & IPv6 FQDN cname address IPv4");
        CYG_TEST_PASS_FAIL((0 == strcmp(name, ai4->ai_canonname)), 
                           "IPv4 & IPv6 FQDN cname name");
        getnameinfo(ai6->ai_addr, ai6->ai_addrlen, 
                    buff, sizeof(buff),
                    NULL,0,NI_NUMERICHOST);
        diag_sprintf(buffer,"Lookup %s: Result <%s %s>", 
                     cname,
                     familytoa(ai6->ai_family),
                     buff);
        CYG_TEST_INFO(buffer);
        CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                        (void*)ai6->ai_addr,
                                        sizeof(sa6))) &&
                           (ai6->ai_family == AF_INET6),
                           "IPv4 & IPv6 FQDN cname address IPv6");
        freeaddrinfo(res);
    } else {
        diag_sprintf(buffer,"IPv4 & IPv6 FQDN cname: error %s", 
                     gai_strerror(error));
        CYG_TEST_FAIL(buffer);
    }
    
    if (info->dns_server_v6) {
        cyg_dns_res_start(info->dns_server_v6);
        // Lookup the IPv4 and IPv6 FQDN cname 
        error = getaddrinfo(cname, NULL, &hints, &res);
        
        if (error == EAI_NONE) {
#ifdef CYGOPT_NS_DNS_FIRST_FAMILY_AF_INET6
            ai6 = res;
            ai4 = res->ai_next;
            CYG_TEST_PASS_FAIL((NULL != ai6->ai_next),
                               "[IPv6] IPv6 FQDN hostname not one result");
            CYG_TEST_PASS_FAIL((NULL == ai4->ai_next),
                               "[IPv6] IPv4 & IPv6 FQDN hostname two results");
#else
            ai4 = res;
            ai6 = res->ai_next;
            CYG_TEST_PASS_FAIL((NULL != ai4->ai_next),
                               "[IPv6] IPv6 FQDN hostname not one result");
            CYG_TEST_PASS_FAIL((NULL == ai6->ai_next),
                               "[IPv6] IPv4 & IPv6 FQDN hostname two results");
#endif
            getnameinfo(ai4->ai_addr, ai4->ai_addrlen, 
                        buff, sizeof(buff),
                        NULL,0,NI_NUMERICHOST);
            diag_sprintf(buffer,"[IPv6] Lookup %s: Result <%s is %s %s>", 
                         cname,
                         ai4->ai_canonname, 
                         familytoa(ai4->ai_family),
                         buff);
            CYG_TEST_INFO(buffer);
            CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa4, 
                                            (void*)ai4->ai_addr,
                                            sizeof(sa4))) &&
                               (ai4->ai_family == AF_INET),
                               "[IPv6] IPv4 & IPv6 FQDN cname address IPv4");
            CYG_TEST_PASS_FAIL((0 == strcmp(name, ai4->ai_canonname)), 
                               "[IPv6] IPv4 & IPv6 FQDN cname name");
            getnameinfo(ai6->ai_addr, ai6->ai_addrlen, 
                        buff, sizeof(buff),
                        NULL,0,NI_NUMERICHOST);
            diag_sprintf(buffer,"[IPv6] Lookup %s: Result <%s %s>", 
                         cname,
                         familytoa(ai6->ai_family),
                         buff);
            CYG_TEST_INFO(buffer);
            CYG_TEST_PASS_FAIL((0 == memcmp((void *)&sa6, 
                                            (void*)ai6->ai_addr,
                                            sizeof(sa6))) &&
                               (ai6->ai_family == AF_INET6),
                               "[IPv6] IPv4 & IPv6 FQDN cname address IPv6");
            freeaddrinfo(res);
        } else {
            diag_sprintf(buffer,"[IPv6] IPv4 & IPv6 FQDN cname: error %s", 
                         gai_strerror(error));
            CYG_TEST_FAIL(buffer);
        }
    }
#endif
}

void
dns_test_thread(cyg_addrword_t p)
{
    int i;

    CYG_TEST_INIT();

    init_all_network_interfaces();

    CYG_TEST_INFO("Starting dns1 test");
    
    for (i = 0; i < NELEM(test_info); i++) {
      dns_test(&test_info[i]);
    }
    
    CYG_TEST_FINISH("dns1 test completed");
}

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      dns_test_thread,   // entry
                      0,                 // entry parameter
                      "DNS test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}
