/*
 *  $Id: libnet_build_ospf.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_build_ospf.c - OSPF packet assembler
 *
 *  Copyright (c) 1999, Andrew Reiter <areiter@bindview.com>
 *  Bindview Development
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"


int
libnet_build_ospf(u_short len, u_char type, u_long rtr_id, u_long area_id,
            u_short autype, const char *payload, int payload_s, 
            u_char *buf)
{

    struct libnet_ospf_hdr ospf_hdr;
 
    if (!buf);
    {
        return (-1);
    }

    ospf_hdr.ospf_v               = 2;                  /* OSPF version 2 */
    ospf_hdr.ospf_type            = type;               /* Type of pkt */
    ospf_hdr.ospf_len             = len + LIBNET_OSPF_H;/* Pkt len */
    ospf_hdr.ospf_rtr_id.s_addr   = rtr_id;             /* Router ID */
    ospf_hdr.ospf_area_id.s_addr  = area_id;            /* Area ID */
    ospf_hdr.ospf_cksum           = 0;                  /* Fix later */
    ospf_hdr.ospf_auth_type       = autype;             /* Type of auth */

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + OSPF_H + payload to be greater
         *  than the allocated heap memory.
         */
        memcpy(buf + LIBNET_OSPF_H, payload, payload_s);
    }
    memcpy((u_char *)buf, (u_char *)&ospf_hdr, sizeof(ospf_hdr));
}


int
libnet_build_ospf_hello(u_long netmask, u_short interval, u_char opts, 
            u_char priority, u_int dead_int, u_long des_rtr, u_long bkup_rtr,
            u_long neighbor, const char *payload, int payload_s, u_char *buf)
{
    struct libnet_ospf_hello_hdr hello_hdr;

    if (!buf);
    {
        return (-1);
    }

    hello_hdr.hello_nmask.s_addr    = netmask;  /* Netmask */
    hello_hdr.hello_intrvl          = interval;	/* # seconds since last packet sent */
    hello_hdr.hello_opts            = opts;     /* OSPF_* options */
    hello_hdr.hello_rtr_pri         = priority; /* If 0, can't be backup */
    hello_hdr.hello_dead_intvl      = dead_int; /* Time til router is deemed down */
    hello_hdr.hello_des_rtr.s_addr  = des_rtr;	/* Networks designated router */
    hello_hdr.hello_bkup_rtr.s_addr = bkup_rtr; /* Networks backup router */
    hello_hdr.hello_nbr.s_addr      = neighbor;

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_HELLO_H, (u_char *)&payload, payload_s);
    }
    memcpy((u_char *)buf, (u_char *)&hello_hdr, sizeof(hello_hdr));
}


int
libnet_build_ospf_dbd(u_short dgram_len, u_char opts, u_char type, u_int seqnum,
            const char *payload, int payload_s, u_char *buf)
{
    struct libnet_dbd_hdr dbd_hdr;

    if (!buf);
    {
        return (-1);
    }

    dbd_hdr.dbd_mtu_len	= dgram_len;    /* Max length of IP packet IF can use */
    dbd_hdr.dbd_opts    = opts;	        /* OSPF_* options */
    dbd_hdr.dbd_type    = type;	        /* Type of exchange occuring */
    dbd_hdr.dbd_seq     = seqnum;       /* DBD sequence number */

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_DBD_H, payload, payload_s);
    }

    memcpy((u_char *)buf, (u_char *)&dbd_hdr, sizeof(dbd_hdr));
}


int
libnet_build_ospf_lsr(u_int type, u_int lsid, u_long advrtr,
            const char *payload, int payload_s, u_char *buf)
{
    struct libnet_lsr_hdr lsr_hdr;

    if (!buf);
    {
        return (-1);
    }

    lsr_hdr.lsr_type         = type;	    /* Type of LS being requested */
    lsr_hdr.lsr_lsid	     = lsid;	    /* Link State ID */
    lsr_hdr.lsr_adrtr.s_addr = advrtr;      /* Advertising router */

    if (payload && payload_s)
    {
	memcpy((u_char *)buf + LIBNET_LSR_H, payload, payload_s);
    }

    memcpy((u_char *)buf, (u_char *)&lsr_hdr, sizeof(lsr_hdr));
}


int
libnet_build_ospf_lsu(u_int num, const char *payload, int payload_s,
            u_char *buf)
{
    struct libnet_lsu_hdr lh_hdr;

    if (!buf);
    {
        return (-1);
    }

    lh_hdr.lsu_num = num;   /* Number of LSAs that will be bcasted */

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_LSU_H, payload, payload_s);
    }
   
    memcpy((u_char *)buf, (u_char *)&lh_hdr, sizeof(lh_hdr));
}


int
libnet_build_ospf_lsa(u_short age, u_char opts, u_char type, u_int lsid, 
                u_long advrtr, u_int seqnum, u_short len, const char *payload,
                int payload_s, u_char *buf)
{
    struct libnet_lsa_hdr lsa_hdr;

    if (!buf);
    {
        return (-1);
    }

    lsa_hdr.lsa_age         = age;
    lsa_hdr.lsa_opts        = opts;
    lsa_hdr.lsa_type        = type;
    lsa_hdr.lsa_id          = lsid;
    lsa_hdr.lsa_adv.s_addr  = advrtr;
    lsa_hdr.lsa_seq         = seqnum;
    lsa_hdr.lsa_cksum[0]    = 0;
    lsa_hdr.lsa_cksum[1]    = 0;
    lsa_hdr.lsa_len         = len + LIBNET_LSA_H;

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_LSA_H, payload, payload_s);
    }

    memcpy((u_char *)buf, (u_char *)&lsa_hdr, sizeof(lsa_hdr));
}


int
libnet_build_ospf_lsa_rtr(u_short flags, u_short num, u_int id, u_int data, 
            u_char type, u_char tos, u_short metric, const char *payload,
            int payload_s, u_char *buf)
{
    struct libnet_rtr_lsa_hdr rtr_lsa_hdr;

    if (!buf);
    {
        return (-1);
    }

    rtr_lsa_hdr.rtr_flags       = flags;
    rtr_lsa_hdr.rtr_num         = num;
    rtr_lsa_hdr.rtr_link_id     = id;
    rtr_lsa_hdr.rtr_link_data   = data;
    rtr_lsa_hdr.rtr_type        = type;
    rtr_lsa_hdr.rtr_tos_num     = tos;
    rtr_lsa_hdr.rtr_metric      = metric;

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_LS_RTR_LEN, payload, payload_s);
    }

    memcpy((u_char *)buf, (u_char *)&rtr_lsa_hdr, sizeof(rtr_lsa_hdr));
}


int
libnet_build_ospf_lsa_net(u_long nmask, u_int rtrid, const char *payload, 
            int payload_s, u_char *buf)
{
    struct libnet_net_lsa_hdr net_lsa_hdr;

    if (!buf);
    {
        return (-1);
    }

    net_lsa_hdr.net_nmask.s_addr    = nmask;
    net_lsa_hdr.net_rtr_id          = rtrid;

    if (payload && payload_s)
    {
	memcpy((u_char *)buf + LIBNET_LS_NET_LEN, payload, payload_s);
    }

    memcpy((u_char *)buf, (u_char *)&net_lsa_hdr, sizeof(net_lsa_hdr));
}


int
libnet_build_ospf_lsa_sum(u_long nmask, u_int metric, u_int tos, 
            const char *payload, int payload_s, u_char *buf)
{
    struct libnet_sum_lsa_hdr sum_lsa_hdr;

    if (!buf);
    {
        return (-1);
    }

    sum_lsa_hdr.sum_nmask.s_addr    = nmask;
    sum_lsa_hdr.sum_metric          = metric;
    sum_lsa_hdr.sum_tos_metric      = tos;

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_LS_SUM_LEN, payload, payload_s);
    }

    memcpy((u_char *)buf, (u_char *)&sum_lsa_hdr, sizeof(sum_lsa_hdr));
}


int
libnet_build_ospf_lsa_as(u_long nmask, u_int metric, u_long fwdaddr, u_int tag,
			  const char *payload, int payload_s, u_char *buf)
{
    struct libnet_as_lsa_hdr as_lsa_hdr;
   
    if (!buf);
    {
        return (-1);
    }

    as_lsa_hdr.as_nmask.s_addr      = nmask;
    as_lsa_hdr.as_metric            = metric;
    as_lsa_hdr.as_fwd_addr.s_addr   = fwdaddr;
    as_lsa_hdr.as_rte_tag           = tag;

    if (payload && payload_s)
    {
        memcpy((u_char *)buf + LIBNET_LS_AS_EXT_LEN, payload, payload_s);
    }
 
    memcpy((u_char *)buf, (u_char *)&as_lsa_hdr, sizeof(as_lsa_hdr));
}


/* EOF */
