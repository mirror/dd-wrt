/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2007-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#include <string.h>
#include "decode.h" 

#ifdef SUP_IP6

#define FAILURE -1
#define SUCCESS 0
#define IP6_HEADER_LEN 40

/* Version is the first four bits of the uint32_t passed in */
#define IP6_VER(x) ((x) >> 28)

/* The 'Packet' structure is almost always allocated on the stack.
 * Likewise, return buffers will almost always be aswell. 
 * So, for performance reasons, argument validation can be disabled
 * and removed from the code at compile time to prevent unecessary extra 
 * conditionals from being checked at run-time. */
#define ERR_CHK_LVL  0
#if ERR_CHK_LVL == 2
#define VALIDATE(x,y) if(!x || !y) return FAILURE;
#elif ERR_CHK_LVL == 1
#define VALIDATE(x,y) if(!y) return FAILURE;
#else
#define VALIDATE(x,y)
#endif

sfip_t *ip6_ret_src(Packet *p)
{
    VALIDATE(p, 1);

    return &p->ip6h->ip_src;
}

sfip_t *orig_ip6_ret_src(Packet *p)
{
    VALIDATE(p, 1);

    return &p->orig_ip6h->ip_src;
}

sfip_t *ip6_ret_dst(Packet *p)
{
    VALIDATE(p, 1);

    return &p->ip6h->ip_dst;
}


sfip_t *orig_ip6_ret_dst(Packet *p)
{
    VALIDATE(p, 1);

    return &p->orig_ip6h->ip_dst;
}

uint16_t ip6_ret_toc(Packet *p)
{
    uint16_t toc;
    VALIDATE(p,1);

    toc = (uint16_t)((ntohl(p->ip6h->vcl) & 0x0FF00000) >> 20);

    return toc;
}

uint16_t orig_ip6_ret_toc(Packet *p)
{
    uint16_t toc;
    VALIDATE(p,1);

    toc = (uint16_t)((ntohl(p->orig_ip6h->vcl) & 0x0FF00000) >> 20);
    return toc;
}

uint8_t ip6_ret_hops(Packet *p)
{
//    VALIDATE(p,1);

    return p->ip6h->hop_lmt;
}
uint8_t orig_ip6_ret_hops(Packet *p)
{
//    VALIDATE(p,1);

    return p->orig_ip6h->hop_lmt;
}

uint16_t ip6_ret_len(Packet *p)
{
    VALIDATE(p,1);

    /* The length field does not include the header in IPv6, but does in IPv4.
     * To make this analogous to IPv4, for Snort's purposes, we need to tack
     * on the difference. */
    return p->ip6h->len;
}

uint16_t orig_ip6_ret_len(Packet *p)
{
    VALIDATE(p,1);

    return p->orig_ip6h->len;
}

uint32_t ip6_ret_id(Packet *p)
{
    IP6Frag *frag_hdr;
    if (p->ip6_extension_count == 0)
        return 0;

    frag_hdr = (IP6Frag*)p->ip6_extensions[p->ip6_frag_index].data;

    return frag_hdr->ip6f_ident;
}

uint32_t orig_ip6_ret_id(Packet *p)
{
// XXX-IPv6 "NOT YET IMPLEMENTED - IP6 identification"
    return 0;
}

uint8_t ip6_ret_next(Packet *p)
{
    VALIDATE(p,1);
    return p->ip6h->next;
}

uint8_t orig_ip6_ret_next(Packet *p)
{
    VALIDATE(p,1);
    return p->orig_ip6h->next;
}

uint16_t ip6_ret_off(Packet *p)
{
    IP6Frag *frag_hdr;
    if (p->ip6_extension_count == 0)
        return 0;

    frag_hdr = (IP6Frag *)p->ip6_extensions[p->ip6_frag_index].data;

    return frag_hdr->ip6f_offlg;
}

uint16_t orig_ip6_ret_off(Packet *p)
{
// XXX-IPv6 "NOT YET IMPLEMENTED - IP6 frag offset"
    return 0;
}

uint8_t ip6_ret_ver(Packet *p)
{
    return (uint8_t)IP6_VER(p->ip6h->vcl); 
}

uint8_t orig_ip6_ret_ver(Packet *p)
{
    return (uint8_t)IP6_VER(p->orig_ip6h->vcl); 
}

sfip_t *ip4_ret_dst(Packet *p)
{
    VALIDATE(p,1);
    return &p->ip4h->ip_dst;
}

sfip_t *orig_ip4_ret_dst(Packet *p)
{
    VALIDATE(p,1);
    return &p->orig_ip4h->ip_dst;
}

sfip_t *ip4_ret_src(Packet *p)
{
    VALIDATE(p,1);
    return &p->ip4h->ip_src;
}

sfip_t *orig_ip4_ret_src(Packet *p)
{
    VALIDATE(p,1);
    return &p->orig_ip4h->ip_src;
}

uint16_t ip4_ret_tos(Packet *p)
{
   VALIDATE(p,1);

   return p->ip4h->ip_tos;
}

uint16_t orig_ip4_ret_tos(Packet *p)
{
   VALIDATE(p,1);

   return p->orig_ip4h->ip_tos;
}

uint8_t ip4_ret_ttl(Packet *p)
{
    VALIDATE(p,1);

    return p->ip4h->ip_ttl;
}

uint8_t orig_ip4_ret_ttl(Packet *p)
{
    VALIDATE(p,1);

    return p->orig_ip4h->ip_ttl;
}

uint16_t ip4_ret_len(Packet *p)
{
    VALIDATE(p,1);

    return p->ip4h->ip_len;
}

uint16_t orig_ip4_ret_len(Packet *p)
{
    VALIDATE(p,1);

    return p->orig_ip4h->ip_len;
}

uint32_t ip4_ret_id(Packet *p)
{
    VALIDATE(p,1);
    
    return (uint32_t)p->ip4h->ip_id;
}

uint32_t orig_ip4_ret_id(Packet *p)
{
    VALIDATE(p,1);
    
    return (uint32_t)p->orig_ip4h->ip_id;
}

uint8_t ip4_ret_proto(Packet *p)
{
    // VALIDATION()
    
    return p->ip4h->ip_proto;
}

uint8_t orig_ip4_ret_proto(Packet *p)
{
    // VALIDATION()
    
    return p->orig_ip4h->ip_proto;
}

uint16_t ip4_ret_off(Packet *p)
{
    return p->ip4h->ip_off;
}

uint16_t orig_ip4_ret_off(Packet *p)
{
    return p->orig_ip4h->ip_off;
}

uint8_t ip4_ret_ver(Packet *p)
{
    return IP_VER(p->iph); 
}

uint8_t orig_ip4_ret_ver(Packet *p)
{
    return IP_VER(p->orig_iph);
}

uint8_t ip4_ret_hlen(Packet *p)
{
    return IP_HLEN(p->iph);
}

uint8_t orig_ip4_ret_hlen(Packet *p)
{
    return IP_HLEN(p->orig_iph);
}

uint8_t ip6_ret_hlen(Packet *p)
{
    /* Snort is expecting this number to be in terms of 32 bit words */
    return IP6_HDR_LEN / 4 ;
}

uint8_t orig_ip6_ret_hlen(Packet *p)
{
    return IP6_HDR_LEN / 4;
}

void sfiph_build(Packet *p, const void *hdr, int family)
{
    IP6RawHdr *hdr6;
    IPHdr *hdr4;

    if(!p || !hdr)
        return;

    /* If family is already set, we've been here before.
     * That means this is a nested IP.  */
    if (p->family != NO_IP)
    {
        if (p->iph_api->ver == IPH_API_V4)
            memcpy(&p->outer_ip4h, &p->inner_ip4h, sizeof(IP4Hdr));
        else if (p->iph_api->ver == IPH_API_V6)
            memcpy(&p->outer_ip6h, &p->inner_ip6h, sizeof(IP6Hdr));

        p->outer_iph_api = p->iph_api;
        p->outer_family = p->family;
    }

    set_callbacks(p, family, CALLBACK_IP);

    if(family == AF_INET)
    {
        hdr4 = (IPHdr*)hdr;

        /* The struct Snort uses is identical to the actual IP6 struct, 
         * with the exception of the IP addresses. Copy over everything but
         * the IPs */
        memcpy(&p->inner_ip4h, hdr4, sizeof(IPHdr) - 8);
        sfip_set_raw(&p->inner_ip4h.ip_src, &hdr4->ip_src, p->family);
        sfip_set_raw(&p->inner_ip4h.ip_dst, &hdr4->ip_dst, p->family);
        p->actual_ip_len = ntohs(p->inner_ip4h.ip_len);
        p->ip4h = &p->inner_ip4h;
    }
    else
    {
        hdr6 = (IP6RawHdr*)hdr;
           
        /* The struct Snort uses is identical to the actual IP6 struct, 
         * with the exception of the IP addresses. Copy over everything but
         * the IPs*/
        memcpy(&p->inner_ip6h, hdr6, sizeof(IP6RawHdr) - 32);
        sfip_set_raw(&p->inner_ip6h.ip_src, &hdr6->ip6_src, p->family);
        sfip_set_raw(&p->inner_ip6h.ip_dst, &hdr6->ip6_dst, p->family);
        p->actual_ip_len = ntohs(p->inner_ip6h.len) + IP6_HDR_LEN;
        p->ip6h = &p->inner_ip6h;
    }
}

void sfiph_orig_build(Packet *p, const void *hdr, int family)
{
    IP6RawHdr *hdr6;
    IPHdr *hdr4;

    if(!p || !hdr)
        return;

    /* If iph_api is already set, we've been here before.
     * That means this is a nested IP.  */
    if (p->orig_iph_api && (p->orig_iph_api->ver == IPH_API_V4))
    {
        memcpy(&p->outer_orig_ip4h, &p->inner_orig_ip4h, sizeof(IP4Hdr));
        p->outer_orig_iph_api = p->orig_iph_api;
    }
    else if (p->orig_iph_api && (p->orig_iph_api->ver == IPH_API_V6))
    {
        memcpy(&p->outer_orig_ip6h, &p->inner_orig_ip6h, sizeof(IP6Hdr));
        p->outer_orig_iph_api = p->orig_iph_api;
    }

    set_callbacks(p, family, CALLBACK_ICMP_ORIG);

    if(family == AF_INET)
    {
        hdr4 = (IPHdr*)hdr;

        /* The struct Snort uses is identical to the actual IP6 struct, 
         * with the exception of the IP addresses. Copy over everything but
         * the IPs */
        memcpy(&p->inner_orig_ip4h, hdr4, sizeof(IPHdr) - 8);
        sfip_set_raw(&p->inner_orig_ip4h.ip_src, &hdr4->ip_src, p->family);
        sfip_set_raw(&p->inner_orig_ip4h.ip_dst, &hdr4->ip_dst, p->family);
        p->actual_ip_len = ntohs(p->inner_orig_ip4h.ip_len);
        p->orig_ip4h = &p->inner_orig_ip4h;
    }
    else
    {
        hdr6 = (IP6RawHdr*)hdr;
           
        /* The struct Snort uses is identical to the actual IP6 struct, 
         * with the exception of the IP addresses. Copy over everything but
         * the IPs*/
        memcpy(&p->inner_orig_ip6h, hdr6, sizeof(IP6RawHdr) - 32);
        sfip_set_raw(&p->inner_orig_ip6h.ip_src, &hdr6->ip6_src, p->family);
        sfip_set_raw(&p->inner_orig_ip6h.ip_dst, &hdr6->ip6_dst, p->family);
        p->actual_ip_len = ntohs(p->inner_orig_ip6h.len) + IP6_HDR_LEN;
        p->orig_ip6h = &p->inner_orig_ip6h;
    }
}

#ifdef TESTER
int main()
{
    Packet p;
    IP4Hdr i4;
    IP6Hdr i6;

    /* This test assumes we get an IPv4 packet and verifies
     * that the correct callbacks are setup, and they return
     * the correct values. */

    set_callbacks(&p, AF_INET, CALLBACK_IP);

    /* Same test as above, but with IPv6 */
    set_callbacks(&p, AF_INET6, CALLBACK_IP);

    return 0;
}
#endif
#endif /* #ifdef SUP_IP6 */
