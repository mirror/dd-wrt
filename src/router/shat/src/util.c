/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: util.c,v 1.15 2005/04/30 11:36:47 jordan Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#define SYSLOG_NAMES
#include <syslog.h>

#include "util.h"

/* ----------------------------------------------------------------------- *
 * Global variables
 * ----------------------------------------------------------------------- */

char *progname = "<unset>" ;      /* program name */
unsigned verbosity = 0;

/* ----------------------------------------------------------------------- *
 * Local variables
 * ----------------------------------------------------------------------- */

static int log_syslog = 0;
static int want_timestamp = 1 ;

/* ----------------------------------------------------------------------- *
 * private
 * ----------------------------------------------------------------------- */

/* IP checksum */
static 
uint16_t in_cksum (const uint16_t *ptr,
                   uint32_t        sum,
                   int             len) {
    while (len > 1)  {
        sum += ((uint8_t*)ptr) [0] | (((uint8_t*)ptr) [1] << 8);
        ptr ++ ;
        len -= 2;
    }
    if (len == 1) {
        uint16_t odd = 0;
        *(uint8_t*)&odd = *(uint8_t*)ptr;
        sum += odd;                         /* adding [<last-byte>, 0] */
    }
    sum  = (sum >> 16) + (sum & 0xffff);  /* add high-16 to low-16 */
    sum += (sum >> 16);                   /* add carry */
    return ~sum ;          /* ones-complement, then truncate to 16 bits */
}


uint16_t ip_cksum (const struct ip *pkt) {
    uint32_t c_sum = in_cksum
        ((const uint16_t*)pkt, -pkt->ip_sum, pkt->ip_hl*4);
    c_sum   = c_sum + (c_sum >> 16) ;
    c_sum   = c_sum + (c_sum >> 16) ;
    return c_sum ;
}


static 
uint16_t phdr_ip_cksum (const struct ip *pkt,
                        uint16_t     old_cks,
                        unsigned short   max) {
    uint32_t c_sum ;
    long len = ntohs (pkt->ip_len) - sizeof (struct ip);
    if (len > max || len < 0) return -1 ;
    c_sum = in_cksum
        ((const uint16_t*)(((uint8_t*)(pkt+1))-8), -old_cks, len+8);
    /* add more pseudo header stuff */
    c_sum -= ntohs ((uint16_t)len) ;
    c_sum -= ntohs (pkt->ip_p);
    c_sum  = c_sum + (c_sum >> 16) ;
    c_sum  = c_sum + (c_sum >> 16) ;
    return c_sum ;
}


uint16_t tcp_cksum (const struct ip *pkt, unsigned short max) {
    return phdr_ip_cksum (pkt, ((struct tcphdr*)(pkt+1))->check, max);
}


uint16_t udp_cksum (const struct ip *pkt, unsigned short max) {
    return phdr_ip_cksum (pkt, ((struct udphdr*)(pkt+1))->check, max);
}

uint16_t icmp_cksum (const struct ip *pkt, unsigned short max) {
    uint16_t old_cks = ((struct icmphdr*)(pkt+1))->checksum ;
    long len = ntohs (pkt->ip_len) - sizeof (struct ip);
    if (len > max || len < 0) return -1 ;
    return in_cksum
        ((const uint16_t*)((struct icmphdr*)(pkt+1)), -old_cks, len);
}

static
char *cks_ok (uint16_t cks_is, uint16_t cks_should) {
    static char buf [50];
    if (cks_is == cks_should)
        sprintf (buf, "sum %#x ok", ntohs (cks_is));
    else
        sprintf (buf, "sum %#x should be %#x",
                 ntohs (cks_is), ntohs (cks_should));
    return buf ;
}


static
char *ipkt_pp_appnd (char *s, int max, const struct ip *pkt, int verbose) {
    char *p;
    unsigned long n ;

    p = memchr (s, 0, max); if (p == 0) return s; else max -= (p - s);
    switch (pkt->ip_p) {
    case IPPROTO_ICMP: {
        struct icmphdr *i = (struct icmphdr *)(pkt + 1);
        sprintf (p, "%s > %s: (%u): ",
                 ipaddr_pp (&pkt->ip_src), 
                 ipaddr_pp (&pkt->ip_dst),
                 (int)i->type);
        if ((n = icmp_cksum (pkt, -1)) != i->checksum || verbose) {
            p = memchr (s, 0, max);
            if (p == 0) return s; else max -= (p - s);
            sprintf (p, "[icmp %s]", cks_ok (i->checksum, n));
        }
        else
            sprintf (p, "[icmp sum ok]");
        break ;
    }
    case IPPROTO_UDP: {
        struct udphdr *u = (struct udphdr *)(pkt + 1);
        sprintf (p, "%s.%u > %s.%u: ",
                 ipaddr_pp (&pkt->ip_src), (int)ntohs (u->source),
                 ipaddr_pp (&pkt->ip_dst), (int)ntohs (u->dest));  
        if ((n = udp_cksum (pkt, -1)) != u->check || verbose) {
            if (u->check) {
                p = memchr (s, 0, max);
                if (p == 0) return s; else max -= (p - s);
                sprintf (p, "[udp %s]", cks_ok (u->check, n));
            }
            else
                sprintf (p, "[udp no-cks]");
        }
        else
            sprintf (p, "[udp sum ok]");
        break ;
    }
    case IPPROTO_TCP: {
        struct tcphdr *t = (struct tcphdr *)(pkt + 1);
        sprintf (p, "%s.%u > %s.%u: ",
                 ipaddr_pp (&pkt->ip_src), (int)ntohs (t->source),
                 ipaddr_pp (&pkt->ip_dst), (int)ntohs (t->dest));  
        n = 0 ;
        if (t->psh) { strcat (p, "P"); n ++; }
        if (t->syn) { strcat (p, "S"); n ++; }
        if (t->fin) { strcat (p, "F"); n ++; }
        if (t->rst) { strcat (p, "R"); n ++; }
        if (t->urg) { strcat (p, "U"); n ++; }
        if (n == 0)   strcat (p, ".");

        if ((n = tcp_cksum (pkt, -1)) != t->check || verbose) {
            p = memchr (s, 0, max);
            if (p == 0) return s; else max -= (p - s);
            sprintf (p, " [tcp %s]", cks_ok (t->check, n));
        }
        else
            strcat (p, " [tcp sum ok]");
    
        p = memchr (s, 0, max); if (p == 0) return s; else max -= (p - s);
        n = ntohs (pkt->ip_len) - 4*pkt->ip_hl - 4*t->doff;
        sprintf (p, " %lu:%lu(%lu)",
                 (long)ntohl (t->seq), (long)ntohl (t->seq) + n, n);

        if (t->ack) {
            p = memchr (s, 0, max);
            if (p == 0) return s; else max -= (p - s);
            sprintf (p, " ack %u", ntohl (t->ack_seq));
        }
        break ;
    }
    default:
        sprintf (p, "%s > %s ipproto=%u",
                 ipaddr_pp (&pkt->ip_src), 
                 ipaddr_pp (&pkt->ip_dst),
                 (int)pkt->ip_p);
    }

    /* sub protocol options here */
    if (ntohs (pkt->ip_off) & IP_DF)
        strcat (s, " (DF)");
    if (ntohs (pkt->ip_off) & IP_MF)
        strcat (s, " (MF)");

    /* fragmentation */
    if (ntohs (pkt->ip_off) & IP_OFFMASK) {
        p = memchr (s, 0, max); if (p == 0) return s; else max -= (p - s);
        sprintf (p, " (offset %d,", ntohs (pkt->ip_off));
    }
    else
        strcat (s, " (");

    /* general ip specs */
    p = memchr (s, 0, max); if (p == 0) return s; else max -= (p - s);
    if ((n = ip_cksum (pkt)) != pkt->ip_sum || verbose) {
        sprintf (p, "ttl %u, id %u, len %d, %s)",
                 (int)pkt->ip_ttl, 
                 (int)ntohs (pkt->ip_id),
                 (int)ntohs (pkt->ip_len),
                 cks_ok (pkt->ip_sum, n));
    }
    else
        sprintf (p, "ttl %u, id %u, len %d)", 
                 (int)pkt->ip_ttl, 
                 (int)ntohs (pkt->ip_id),
                 (int)ntohs (pkt->ip_len));

    return s ;
}


static
char *arpkt_pp_appnd (char                     *s,
                      int                     max, 
                      const struct ether_arp *pkt, 
                      int                 verbose) {
    char *p;

    if (LDX16B (pkt->ea_hdr.ar_hrd) != ARPHRD_ETHER) {
        sprintf (s, "arp hwtype=%#x", LDX16B (pkt->ea_hdr.ar_hrd));
        return s;
    }
    if (LDX16B (pkt->ea_hdr.ar_pro) != ETHERTYPE_IP) {
        sprintf (s, "arp prto=%#x", LDX16B (pkt->ea_hdr.ar_pro));
        return s;
    }
    if (pkt->ea_hdr.ar_hln != ETH_ALEN) {
        sprintf (s, "arp hwaddr_len=%#x", pkt->ea_hdr.ar_hln);
        return s;
    }
    if (pkt->ea_hdr.ar_pln != IP4_ALEN) {
        sprintf (s, "arp ipaddr_len=%#x", pkt->ea_hdr.ar_pln);
        return s;
    }

    p = s ;
    switch (LDX16B (pkt->ea_hdr.ar_op)) {
    case ARPOP_REQUEST:
        if (verbose) 
            sprintf (p, "who-has %s tell %s(%s)",
                     ip4addr_pp (ip2ip4addr (pkt->arp_tpa)),
                     ip4addr_pp (ip2ip4addr (pkt->arp_spa)),
                     hw6addr_pp (mp2hw6addr (pkt->arp_sha)));
        else
            sprintf (p, "who-has %s tell %s",
                     ip4addr_pp (ip2ip4addr (pkt->arp_tpa)),
                     ip4addr_pp (ip2ip4addr (pkt->arp_spa)));
        return s;
    case ARPOP_REPLY:
        if (verbose)
            sprintf (p, "%s is-at %s telling %s(%s)",
                     ip4addr_pp (ip2ip4addr (pkt->arp_spa)),
                     hw6addr_pp (mp2hw6addr (pkt->arp_sha)),
                     ip4addr_pp (ip2ip4addr (pkt->arp_tpa)),
                     hw6addr_pp (mp2hw6addr (pkt->arp_tha)));
        else
            sprintf (p, "%s is-at %s",
                     ip4addr_pp (ip2ip4addr (pkt->arp_spa)), 
                     hw6addr_pp (mp2hw6addr (pkt->arp_sha)));
        return s;
    }

    sprintf (s, "arp op=%#x", LDX16B (pkt->ea_hdr.ar_op));
    return s;
}


/* ----------------------------------------------------------------------- *
 * Public
 * ----------------------------------------------------------------------- */

/* secure malloc */
VOID *xmalloc (size_t n) {

    char *p = malloc (n) ;
    if (p == 0) {
        logger (LOG_INFO, "%s: malloc (%u) returned NULL!\n", progname, n);
        exit (2) ;
    }
    return memset (p, 0, n);
}


/* secure realloc, does not initialize memory */
VOID *xrealloc (void  *s,
                size_t n) {

    char *p = (s == 0 ? xmalloc (n) : realloc (s, n)) ;

    if (p == 0) {
        logger (LOG_INFO, "%s: realloc (%#x,%u) returned NULL!\n", 
                 progname, s, n);
        exit (2) ;
    }
    return (VOID*)p;
}



/* pretty print the given time stamp */
char *stamp_pp (const struct timeval *tv) {

    struct timeval _tv ;
    struct tm *tm ;
    char fm [50] ;
    int  sz ;

    static char buf [60] ;
  
    if (tv == 0) {
        if (gettimeofday (&_tv, 0) < 0)
            memset (&_tv, 0, sizeof (_tv));
        tv = &_tv ;
    }

    tm = localtime (&tv->tv_sec) ;
    sz = tm ?strftime (fm, sizeof (fm), "%Y-%d-%m %H:%M:%S.%%06u", tm) :0 ;

    if (sz > 0)
        sprintf (buf, fm, tv->tv_usec);
    else
        sprintf (buf, "%u.%06u",
                 (unsigned)tv->tv_sec, (unsigned)tv->tv_usec);

    return buf ;
}


/* pretty print the given integer target */
char *ulongp_pp (const long *p) {
    static char buf [1 + sizeof (long) * 3] ;
    if (p == 0)
        return "<no-int>";
    sprintf (buf, "%lu", *p);
    return buf ;
}


char *ushortp_pp (const short *p) {
    static char buf [1 + sizeof (long) * 3] ;
    if (p == 0)
        return "<no-int>";
    sprintf (buf, "%hu", *p);
    return buf ;
}


/* pretty print the given integer target */
char *xlongp_pp (const long *p) {
    static char buf [5 + sizeof (long) * 2] ;
    if (p == 0)
        return "<no-int>";
    sprintf (buf, "%lx", *p);
    return buf ;
}


/* pretty print the given time integer */
char *time_pp (time_t t) {
    struct tm *tm = localtime (&t) ;
    static char buf [50] ;

    if (tm) {
        size_t sz = strftime (buf, sizeof (buf), "%Y-%d-%m %H:%M:%S", tm);
        if (sz > 0)
            return buf ;
    }
    
    sprintf (buf, "%lu", t);
    return buf ;
}


/* pretty print an ipv4 address */
char *ipaddr_pp (const struct in_addr *ia) { /* ia probably unaligned */
  
    char *p ;
    struct in_addr i; /* aligned on stack */

    /* provide for some buffers */
    static char buf [16 * 4] ;
    static int  inx ;

    if (ia == 0)
        return "<no-ip>";

    /* rotate: get the other buffer */
    if ((inx += 16) >= sizeof (buf))
        inx = 0 ;

    if (ia == 0 ||
        (p = inet_ntoa (*(struct in_addr*)memcpy (&i,ia,sizeof (i)))) == 0 ||
        strlen (p) >= 16)
        return "???.???.???.???" ;
  
    return strcpy (buf + inx, p);
}


char *ip4addr_pp (ip4addr_t ip) {
    struct in_addr ia;
    ia.s_addr = htonl (ip) ;
    return ipaddr_pp (&ia);
}


/* pretty print an ethernet address */
char *ethaddr_pp (const struct ether_addr *ea) { /* ea probably unaligned */

    char *p ;
    struct ether_addr e; /* aligned on stack */

    /* provide for some buffers */
    static char buf [18 * 4] ;
    static int  inx ;

    if (ea == 0)
        return "<no-mac>";

    /* rotate: get the other buffer */
    if ((inx += 18) >= sizeof (buf))
        inx = 0 ;
  
    if (ea == 0 ||
        (p = ether_ntoa (memcpy (&e,ea,sizeof (e)))) == 0 ||
        strlen (p) >= 18)
        return "??:??:??:??:??:??" ;
  
    return strcpy (buf + inx, p);
}


char *hw6addr_pp (hw6addr_t mac) {
    struct ether_addr ea;
    hw6addr2ip (ea.ether_addr_octet, mac) ;
    return ethaddr_pp (&ea);
}


char *ipkt_pp (const struct ip *pkt, int verbose) {
    static char buf [1024]; /* should be big enough */
    buf [0] = '\0';
    return ipkt_pp_appnd (buf, sizeof buf, pkt, verbose);
}


char *arpkt_pp (const struct ether_arp *pkt, int verbose) {
    static char buf [1024]; /* should be big enough */
    buf [0] = '\0';
    return arpkt_pp_appnd (buf, sizeof buf, pkt, verbose);
}


char *ethpkt_pp (const struct ethhdr *pkt, int verbose) {
    static char buf [1024]; /* should be big enough */
    char *p;

    sprintf (buf, "%s > %s; ", 
             ethaddr_pp ((struct ether_addr*)pkt->h_source),
             ethaddr_pp ((struct ether_addr*)pkt->h_dest));

    switch (ntohs (pkt->h_proto)) {
    case ETH_P_IP:
        return ipkt_pp_appnd 
            (buf, sizeof buf, (const struct ip*)(pkt+1), verbose);
    case ETH_P_ARP: 
        return arpkt_pp_appnd 
            (buf, sizeof buf, (const struct ether_arp*)(pkt+1), verbose);
    }

    p = memchr (buf, 0, sizeof buf);
    sprintf (buf, "ethertype=%#x", ntohs (pkt->h_proto));
    return buf;
}


char *ethhdr_pp (const struct ethhdr *pkt, int verbose) {
    static char buf [128];
    char *p;

    sprintf (buf, "%s > %s; ", 
             ethaddr_pp ((struct ether_addr*)pkt->h_source),
             ethaddr_pp ((struct ether_addr*)pkt->h_dest));

    switch (ntohs (pkt->h_proto)) {
    case ETH_P_IP:  strcat (buf,  "ip"); return buf;
    case ETH_P_ARP: strcat (buf, "arp"); return buf;
    }

    p = memchr (buf, 0, sizeof buf);
    sprintf (buf, "ethertype=%#x", ntohs (pkt->h_proto));
    return buf;
}


void logger (int level, const char *format, ...) {
    char f [1000];

    va_list ap;
    va_start (ap, format);

    if (log_syslog) {
        if (want_timestamp) {
            f [sizeof (f) - 1] = '\0';
            snprintf (f, sizeof (f) - 1, "%s %s", stamp_pp (0), format);
            format = f ;
        }
        vsyslog (level, format, ap);
    }
    else {
        f [sizeof (f) - 1] = '\0';
        if (want_timestamp)
            snprintf (f, sizeof (f) - 1, "%s %s\n", stamp_pp (0), format);
        else
            snprintf (f, sizeof (f) - 1, "%s\n", format);
        vfprintf (stderr, f, ap);
    }

    va_end (ap);
}


unsigned syslog_init (char *fac, int timestamp) {

    want_timestamp = timestamp ;

    if (fac) {
        int i = -1;
        while (facilitynames [++ i].c_name) {
            if (strcasecmp (facilitynames [i].c_name, fac) == 0) {
                openlog (progname, LOG_PID, facilitynames [i].c_val);
                return log_syslog = 1 ;
            }
        }
    }

    if (log_syslog)
        closelog ();
    return log_syslog = 0 ; 
}

/* parse non-negative integer in decimal, octal or decimal format */
unsigned integer (unsigned *l, const char *s) {
    if (s [0] != '-') {
        char *p ;
        unsigned n = strtol (s, &p, 0);
        if (p && *p == 0) {
            *l = n ;
            return 1 ;
        }
    }
    return 0 ;
}

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
