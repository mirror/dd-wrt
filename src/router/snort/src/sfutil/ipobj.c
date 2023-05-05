/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/*

   ipobj.c

   IP address encapsulation interface

   This module provides encapsulation of single IP ADDRESSes as
   objects, and collections of IP ADDRESSes as objects


*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <ctype.h>

#include "ipobj.h"
#include "util.h"
#include "snort_bounds.h"



/*


   IP COLLECTION INTERFACE


   Snort Accepts:

   IP-Address		192.168.1.1
   IP-Address/MaskBits	192.168.1.0/24
   IP-Address/Mask		192.168.1.0/255.255.255.0


   These can all be handled via the CIDR block notation : IP/MaskBits

   We use collections (lists) of cidr blocks to represent address blocks
   and indivdual addresses.

   For a single IPAddress the implied Mask is 32 bits,or 255.255.255.255, or 0xffffffff, or -1.

*/
IPSET * ipset_new(void)
{
    IPSET * p = (IPSET *)SnortAlloc( sizeof(IPSET));
    sflist_init(&p->ip_list);

    return p;
}

IPSET * ipset_copy( IPSET *ipsp )
{
    IPSET * newset = ipset_new();
    IP_PORT *ip_port;

    for(ip_port =(IP_PORT*)sflist_first( &ipsp->ip_list );
        ip_port !=NULL;
        ip_port =(IP_PORT*)sflist_next( &ipsp->ip_list ) )
    {
        ipset_add(newset, &ip_port->ip, &ip_port->portset, ip_port->notflag);
    }
    return newset;
}

void ipset_free( IPSET * ipc )
{
    if (ipc)
    {

        IP_PORT *p = (IP_PORT *) sflist_first(&ipc->ip_list);
        while ( p )
        {
            sflist_static_free_all(&p->portset.port_list, free);
            p = (IP_PORT *) sflist_next(&ipc->ip_list);
        }
        sflist_static_free_all(&ipc->ip_list, free);
        free( ipc );
    }
}

int     ipset_add     ( IPSET * ipset, sfcidr_t *ip, void * vport, int notflag)
{
    if( !ipset ) return -1;

    {
        PORTSET  * portset = (PORTSET *) vport;
        IP_PORT *p = (IP_PORT*)calloc( 1,sizeof(IP_PORT) );
        if(!p) return -1;

        sfip_set_ip(&p->ip, ip);
        p->portset = *portset;
        p->notflag = (char)notflag;

        if( notflag )sflist_add_head( &ipset->ip_list, p ); // test NOT items 1st
        else         sflist_add_tail( &ipset->ip_list, p );
    }

    return 0;
}

int ipset_contains( IPSET * ipc, sfaddr_t * ip, void *port)
{
    PORTRANGE *pr;
    unsigned short portu;
    IP_PORT * p;

    if( !ipc ) return 0;

    if ( port )
        portu = *((unsigned short *)port);
    else
        portu = 0;


    for(p =(IP_PORT*)sflist_first( &ipc->ip_list );
        p!=0;
        p =(IP_PORT*)sflist_next( &ipc->ip_list ) )
    {
        if( sfip_contains(&p->ip, ip) == SFIP_CONTAINS)
        {
            for( pr=(PORTRANGE*)sflist_first(&p->portset.port_list);
                 pr != 0;
                 pr=(PORTRANGE*)sflist_next(&p->portset.port_list) )
            {
                /*
                 * If the matching IP has a wildcard port (pr->port_hi == 0 )
                 * or if the ports actually match.
                 */
                if ( (pr->port_hi == 0) ||
                     (portu >= pr->port_lo && portu <= pr->port_hi) )
                {
                    if( p->notflag )
                        return 0;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int ipset_print( IPSET * ipc )
{
    char ip_str[80];
    PORTRANGE * pr;

    if( !ipc ) return 0;

    {
        IP_PORT * p;

        printf("IPSET\n");

        for( p =(IP_PORT*)sflist_first( &ipc->ip_list );
             p!=0;
             p =(IP_PORT*)sflist_next( &ipc->ip_list ) )
        {
            SnortSnprintf(ip_str, 80, "%s", sfip_to_str(&p->ip.addr));

            printf("CIDR BLOCK: %c%s", p->notflag ? '!' : ' ', ip_str);

            for( pr=(PORTRANGE*)sflist_first(&p->portset.port_list);
                 pr != 0;
                 pr=(PORTRANGE*)sflist_next(&p->portset.port_list) )
            {
                printf("  %d", pr->port_lo);
                if ( pr->port_hi != pr->port_lo )
                    printf("-%d", pr->port_hi);
            }
            printf("\n");
        }
    }
    return 0;
}


static void portset_init( PORTSET * portset )
{
    sflist_init(&portset->port_list);
}

static int portset_add(PORTSET * portset, unsigned port_lo, unsigned port_hi)
{
    PORTRANGE *p;

    if( !portset ) return -1;

    p = (PORTRANGE *) calloc( 1,sizeof(PORTRANGE) );
    if(!p) return -1;

    p->port_lo = port_lo;
    p->port_hi = port_hi;

    sflist_add_tail(&portset->port_list, p );

    return 0;
}

static int port_parse(char *portstr, PORTSET *portset)
{
    unsigned port_lo = 0, port_hi = 0;
    char *port1;
    char *port_begin;
    char *port_end;
    char *port2 = NULL;

    port_begin = SnortStrdup(portstr);

    port1 = port_begin;
    port2 = strstr(port_begin, "-");

    {
        if (*port1 == '\0')
        {
            free(port_begin);
            return -1;
        }

        if (port2)
        {
            *port2 = '\0';
            port2++;
        }

        port_lo = strtoul(port1, &port_end, 10);
        if (port_end == port1)
        {
            free(port_begin);
            return -2;
        }

        if (port2)
        {
            port_hi = strtoul(port2, &port_end, 10);
            if (port_end == port2)
            {
                free(port_begin);
                return -3;
            }
        }
        else
        {
            port_hi = port_lo;
        }

        /* check to see if port is out of range */
        if ( port_hi > MAXPORTS-1 || port_lo > MAXPORTS-1)
        {
            free(port_begin);
            return -4;
        }

        /* swap ports if necessary */
        if (port_hi < port_lo)
        {
            unsigned tmp;

            tmp = port_hi;
            port_hi = port_lo;
            port_lo = tmp;
        }

        portset_add(portset, port_lo, port_hi);
    }

    free(port_begin);

    return 0;
}

static int ip_parse(char *ipstr, sfcidr_t *ip, char *not_flag, PORTSET *portset, char **endIP)
{
    char *port_str;
    char *comma;
    char *end_bracket;

    if (*ipstr == '!')
    {
        ipstr++;
        *not_flag = 1;
    }
    else
    {
        *not_flag = 0;
    }

    comma = strchr(ipstr, ',');
    end_bracket = strrchr(ipstr, ']');

    if (comma)
    {
        *comma = '\0';
    }
    else if (end_bracket)
    {
        *end_bracket = '\0';
    }

    if (sfip_pton(ipstr, ip) != SFIP_SUCCESS)
        return -1;

    /* Just to get the IP string out of the way */
    port_str = strtok(ipstr, " \t");
    /* Is either the port after the 1st space, or NULL */
    port_str = strtok(NULL, " \t");

    while (port_str)
    {
        if (!comma)
        {
            comma = strchr(port_str, ',');
            if (comma)
                *comma = '\0';
        }

        if (!end_bracket)
        {
            end_bracket = strrchr(port_str, ']');
            if (end_bracket)
                *end_bracket = '\0';
        }

        port_parse(port_str, portset);
        port_str = strtok(NULL, " \t");
    }

    if (portset->port_list.count == 0)
    {
        /* Make sure we have at least one port range in list, but
         * an invalid port range to convey all is good.  */
        portset_add(portset, 0, 0);
    }

    if (comma)
    {
        *endIP = comma;
        *comma = ',';
    }
    else if (end_bracket)
    {
        *end_bracket = ']';
        *endIP = end_bracket;
    }
    else
    {
        /* Didn't see the comma or end bracket, so set endIP now */
        *endIP = port_str;
    }

    return 0;
}

int ipset_parse(IPSET *ipset, char *ipstr)
{
    char *copy, *startIP, *endIP;
    int parse_count = 0;
    char set_not_flag = 0;
    char item_not_flag;
    char open_bracket = 0;
    sfcidr_t ip;
    PORTSET portset;

    copy = strdup(ipstr);

    if(!copy)
        return -2;

    startIP = copy;

    if (*startIP == '!')
    {
        set_not_flag = 1;
        startIP++;
    }

    while (startIP)
    {
        if (*startIP == '[')
        {
            open_bracket++;
            startIP++;
            if (!*startIP)
                break;
        }

        if ((*startIP == ']') || (*startIP == '\0'))
        {
            open_bracket--;
            break;
        }

        portset_init(&portset);

        if(ip_parse(startIP, &ip, &item_not_flag, &portset, &endIP) != 0)
        {
            free(copy);
            return -5;
        }

        if(ipset_add(ipset, &ip, &portset, (item_not_flag ^ set_not_flag)) != 0)
        {
            free(copy);
            return -6;
        }

        parse_count++;

        if (endIP && (*endIP != ']'))
        {
            endIP++;
        }

        startIP = endIP;
    }

    free(copy);

    if (!parse_count)
        return -7;

    if (open_bracket)
        return -8;

    return 0;
}

#ifdef MAIN_IP

#include <time.h>

#ifndef WIN32
#define rand   random
#define srand srandom
#endif

#define MAXIP 100

#include "sflsq.c"

void test_ip4_parsing(void)
{
    unsigned host, mask, not_flag;
    PORTSET  portset;
    char **curip;
    int ret;
    IPADDRESS *adp;
    char *ips[] = {
        "138.26.1.24:25",
        "1.1.1.1/255.255.255.0:444",
        "1.1.1.1/16:25-28",
        "1.1.1.1/255.255.255.255:25 27-29",
        "z/24",
        "0/0",
        "0.0.0.0/0.0.0.0:25-26 28-29 31",
        "0.0.0.0/0.0.2.0",
        NULL };

    for(curip = ips; curip[0] != NULL; curip++)
    {

        portset_init(&portset);

        /* network byte order stuff */
        if((ret = ip4_parse(curip[0], 1, &not_flag, &host, &mask, &portset)) != 0)
        {
            fprintf(stderr, "Unable to parse %s with ret %d\n", curip[0], ret);
        }
        else
        {
            printf("%c", not_flag ? '!' : ' ');
            printf("%s/", inet_ntoa(*(struct in_addr *) &host));
            printf("%s", inet_ntoa(*(struct in_addr *) &mask));
            printf(" parsed successfully!\n");
        }

        /* host byte order stuff */
        if((ret = ip4_parse(curip[0], 0, &not_flag, &host, &mask, &portset)) != 0)
        {
            fprintf(stderr, "Unable to parse %s with ret %d\n", curip[0], ret);
        }
        else
        {
            adp = ip_new(IPV4_FAMILY);
            ip_set(adp, &host, IPV4_FAMILY);
            ip_fprint(stdout, adp);
            fprintf(stdout, "*****************\n");
            ip_free(adp);
        }
    }

    return;
}


void test_ip4set_parsing(void)
{
    char **curip;
    int ret;
    char *ips[] = {
        "12.24.24.1/32,!24.24.24.1",
        "[0.0.0.0/0.0.2.0,241.242.241.22]",
        "138.26.1.24",
        "1.1.1.1",
        "1.1.1.1/16",
        "1.1.1.1/255.255.255.255",
        "z/24",
        "0/0",
        "0.0.0.0/0.0.0.0",
        "0.0.0.0/0.0.2.0",
        NULL };

    for(curip = ips; curip[0] != NULL; curip++)
    {
        IPSET *ipset = ipset_new(IPV4_FAMILY);

        /* network byte order stuff */
        if((ret = ip4_setparse(ipset, curip[0])) != 0)
        {
            ipset_free(ipset);
            fprintf(stderr, "Unable to parse %s with ret %d\n", curip[0], ret);
        }
        else
        {
            printf("-[%s]\n ", curip[0]);
            ipset_print(ipset);
            printf("---------------------\n ");
        }
    }

    return;
}

//  -----------------------------
void test_ip(void)
{
    int            i,k;
    IPADDRESS    * ipa[MAXIP];
    unsigned       ipaddress,ipx;
    unsigned short ipaddress6[8], ipx6[8];

    printf("IPADDRESS testing\n");

    srand( time(0) );

    for(i=0;i<MAXIP;i++)
    {
        if( i % 2 )
        {
            ipa[i]= ip_new(IPV4_FAMILY);
            ipaddress = rand() * rand();
            ip_set( ipa[i], &ipaddress, IPV4_FAMILY  );

            if( !ip_equal(ipa[i],&ipaddress, IPV4_FAMILY ) )
                printf("error with ip_equal\n");

            ip_get( ipa[i], &ipx, IPV4_FAMILY );
            if( ipx != ipaddress )
                printf("error with ip_get\n");

        }
        else
        {
            ipa[i]= ip_new(IPV6_FAMILY);

            for(k=0;k<8;k++) ipaddress6[k] = (char) (rand() % (1<<16));

            ip_set( ipa[i], ipaddress6, IPV6_FAMILY  );

            if( !ip_equal(ipa[i],&ipaddress6, IPV6_FAMILY ) )
                printf("error with ip6_equal\n");

            ip_get( ipa[i], ipx6, IPV6_FAMILY  );

            for(k=0;k<8;k++)
                if( ipx6[k] != ipaddress6[k] )
                    printf("error with ip6_get\n");

        }

        printf("[%d] ",i);
        ip_fprint(stdout,ipa[i]);
        printf("\n");
    }

    printf("IP testing completed\n");
}



//  -----------------------------
void test_ipset(void)
{
    int      i,k;
    IPSET  * ipset, * ipset6;
    IPSET  * ipset_copyp, * ipset6_copyp;

    unsigned ipaddress, mask;
    unsigned short mask6[8];
    unsigned short ipaddress6[8];
    unsigned port_lo, port_hi;
    PORTSET        portset;

    printf("IPSET testing\n");

    ipset  = ipset_new(IPV4_FAMILY);
    ipset6 = ipset_new(IPV6_FAMILY);

    srand( time(0) );

    for(i=0;i<MAXIP;i++)
    {
        if( i % 2 )
        {
            ipaddress = rand() * rand();
            mask = 0xffffff00;
            port_lo = rand();
            port_hi = rand() % 5 + port_lo;
            portset_init(&portset);
            portset_add(&portset, port_lo, port_hi);

            ipset_add( ipset, &ipaddress, &mask, &portset, 0, IPV4_FAMILY ); //class C cidr blocks

            if( !ipset_contains( ipset, &ipaddress, &port_lo, IPV4_FAMILY ) )
                printf("error with ipset_contains\n");
        }
        else
        {
            for(k=0;k<8;k++) ipaddress6[k] = (char) (rand() % (1<<16));

            for(k=0;k<8;k++) mask6[k] = 0xffff;

            port_lo = rand();
            port_hi = rand() % 5 + port_lo;
            portset_init(&portset);
            portset_add(&portset, port_lo, port_hi);

            ipset_add( ipset6, ipaddress6, mask6, &portset, 0, IPV6_FAMILY );

            if( !ipset_contains( ipset6, &ipaddress6, &port_lo, IPV6_FAMILY ) )
                printf("error with ipset6_contains\n");
        }

    }

    ipset_copyp = ipset_copy( ipset );
    ipset6_copyp = ipset_copy( ipset6 );


    printf("-----IP SET-----\n");
    ipset_print( ipset );
    printf("\n");

    printf("-----IP SET6-----\n");
    ipset_print( ipset6 );
    printf("\n");

    printf("-----IP SET COPY -----\n");
    ipset_print( ipset_copyp );
    printf("\n");

    printf("-----IP SET6 COPY -----\n");
    ipset_print( ipset6_copyp );
    printf("\n");

    printf("IP set testing completed\n");
}


//  -----------------------------
int main( int argc, char ** argv )
{
    printf("ipobj \n");

    test_ip();

    test_ipset();

    test_ip4_parsing();

    test_ip4set_parsing();

    printf("normal pgm completion\n");

    return 0;
}

#endif

