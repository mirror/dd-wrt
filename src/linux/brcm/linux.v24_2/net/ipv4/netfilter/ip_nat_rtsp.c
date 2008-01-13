/*
 * RTSP extension for TCP NAT alteration
 * (C) 2003 by Tom Marshall <tmarshall@real.com>
 * based on ip_nat_irc.c
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 * Module load syntax:
 *      insmod ip_nat_rtsp.o ports=port1,port2,...port<MAX_PORTS>
 *                           stunaddr=<address>
 *                           destaction=[auto|strip|none]
 *
 * If no ports are specified, the default will be port 554 only.
 *
 * stunaddr specifies the address used to detect that a client is using STUN.
 * If this address is seen in the destination parameter, it is assumed that
 * the client has already punched a UDP hole in the firewall, so we don't
 * mangle the client_port.  If none is specified, it is autodetected.  It
 * only needs to be set if you have multiple levels of NAT.  It should be
 * set to the external address that the STUN clients detect.  Note that in
 * this case, it will not be possible for clients to use UDP with servers
 * between the NATs.
 *
 * If no destaction is specified, auto is used.
 *   destaction=auto:  strip destination parameter if it is not stunaddr.
 *   destaction=strip: always strip destination parameter (not recommended).
 *   destaction=none:  do not touch destination parameter (not recommended).
 */

#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/kernel.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_rtsp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>

#include <linux/inet.h>
#include <linux/ctype.h>
#define NF_NEED_STRNCASECMP
#define NF_NEED_STRTOU16
#include <linux/netfilter_helpers.h>
#define NF_NEED_MIME_NEXTLINE
#include <linux/netfilter_mime.h>

#define INFOP(fmt, args...) printk(KERN_INFO "%s: %s: " fmt, __FILE__, __FUNCTION__ , ## args)
#ifdef IP_NF_RTSP_DEBUG
#define DEBUGP(fmt, args...) printk(KERN_DEBUG "%s: %s: " fmt, __FILE__, __FUNCTION__ , ## args)
#else
#define DEBUGP(fmt, args...)
#endif

#define MAX_PORTS       8
#define DSTACT_AUTO     0
#define DSTACT_STRIP    1
#define DSTACT_NONE     2

static int      ports[MAX_PORTS];
static char*    stunaddr = NULL;
static char*    destaction = NULL;

static int       num_ports = 0;
static u_int32_t extip = 0;
static int       dstact = 0;

MODULE_AUTHOR("Tom Marshall <tmarshall@real.com>");
MODULE_DESCRIPTION("RTSP network address translation module");
MODULE_LICENSE("GPL");
#ifdef MODULE_PARM
MODULE_PARM(ports, "1-" __MODULE_STRING(MAX_PORTS) "i");
MODULE_PARM_DESC(ports, "port numbers of RTSP servers");
MODULE_PARM(stunaddr, "s");
MODULE_PARM_DESC(stunaddr, "Address for detecting STUN");
MODULE_PARM(destaction, "s");
MODULE_PARM_DESC(destaction, "Action for destination parameter (auto/strip/none)");
#endif

/* protects rtsp part of conntracks */
DECLARE_LOCK_EXTERN(ip_rtsp_lock);

#define SKIP_WSPACE(ptr,len,off) while(off < len && isspace(*(ptr+off))) { off++; }

/*** helper functions ***/

static void
get_skb_tcpdata(struct sk_buff* skb, char** pptcpdata, uint* ptcpdatalen)
{
    struct iphdr*   iph  = (struct iphdr*)skb->nh.iph;
    struct tcphdr*  tcph = (struct tcphdr*)((char*)iph + iph->ihl*4);

    *pptcpdata = (char*)tcph + tcph->doff*4;
    *ptcpdatalen = ((char*)skb->h.raw + skb->len) - *pptcpdata;
}

/*** nat functions ***/

/*
 * Mangle the "Transport:" header:
 *   - Replace all occurences of "client_port=<spec>"
 *   - Handle destination parameter
 *
 * In:
 *   ct, ctinfo = conntrack context
 *   pskb       = packet
 *   tranoff    = Transport header offset from TCP data
 *   tranlen    = Transport header length (incl. CRLF)
 *   rport_lo   = replacement low  port (host endian)
 *   rport_hi   = replacement high port (host endian)
 *
 * Returns packet size difference.
 *
 * Assumes that a complete transport header is present, ending with CR or LF
 */
static int
rtsp_mangle_tran(struct ip_conntrack* ct, enum ip_conntrack_info ctinfo,
                 struct ip_conntrack_expect* exp,
                 struct sk_buff** pskb, uint tranoff, uint tranlen)
{
    char*       ptcp;
    uint        tcplen;
    char*       ptran;
    char        rbuf1[16];      /* Replacement buffer (one port) */
    uint        rbuf1len;       /* Replacement len (one port) */
    char        rbufa[16];      /* Replacement buffer (all ports) */
    uint        rbufalen;       /* Replacement len (all ports) */
    u_int32_t   newip;
    u_int16_t   loport, hiport;
    uint        off = 0;
    uint        diff;           /* Number of bytes we removed */

    struct ip_ct_rtsp_expect* prtspexp = &exp->help.exp_rtsp_info;
    struct ip_conntrack_tuple t;

    char    szextaddr[15+1];
    uint    extaddrlen;
    int     is_stun;

    get_skb_tcpdata(*pskb, &ptcp, &tcplen);
    ptran = ptcp+tranoff;

    if (tranoff+tranlen > tcplen || tcplen-tranoff < tranlen ||
        tranlen < 10 || !iseol(ptran[tranlen-1]) ||
        nf_strncasecmp(ptran, "Transport:", 10) != 0)
    {
        INFOP("sanity check failed\n");
        return 0;
    }
    off += 10;
    SKIP_WSPACE(ptcp+tranoff, tranlen, off);

    newip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
    t = exp->tuple;
    t.dst.ip = newip;

    extaddrlen = extip ? sprintf(szextaddr, "%u.%u.%u.%u", NIPQUAD(extip))
                       : sprintf(szextaddr, "%u.%u.%u.%u", NIPQUAD(newip));
    DEBUGP("stunaddr=%s (%s)\n", szextaddr, (extip?"forced":"auto"));

    rbuf1len = rbufalen = 0;
    switch (prtspexp->pbtype)
    {
    case pb_single:
        for (loport = prtspexp->loport; loport != 0; loport++) /* XXX: improper wrap? */
        {
            t.dst.u.udp.port = htons(loport);
            if (ip_conntrack_change_expect(exp, &t) == 0)
            {
                DEBUGP("using port %hu\n", loport);
                break;
            }
        }
        if (loport != 0)
        {
            rbuf1len = sprintf(rbuf1, "%hu", loport);
            rbufalen = sprintf(rbufa, "%hu", loport);
        }
        break;
    case pb_range:
        for (loport = prtspexp->loport; loport != 0; loport += 2) /* XXX: improper wrap? */
        {
            t.dst.u.udp.port = htons(loport);
            if (ip_conntrack_change_expect(exp, &t) == 0)
            {
                hiport = loport + ~exp->mask.dst.u.udp.port;
                DEBUGP("using ports %hu-%hu\n", loport, hiport);
                break;
            }
        }
        if (loport != 0)
        {
            rbuf1len = sprintf(rbuf1, "%hu", loport);
            rbufalen = sprintf(rbufa, "%hu-%hu", loport, loport+1);
        }
        break;
    case pb_discon:
        for (loport = prtspexp->loport; loport != 0; loport++) /* XXX: improper wrap? */
        {
            t.dst.u.udp.port = htons(loport);
            if (ip_conntrack_change_expect(exp, &t) == 0)
            {
                DEBUGP("using port %hu (1 of 2)\n", loport);
                break;
            }
        }
        for (hiport = prtspexp->hiport; hiport != 0; hiport++) /* XXX: improper wrap? */
        {
            t.dst.u.udp.port = htons(hiport);
            if (ip_conntrack_change_expect(exp, &t) == 0)
            {
                DEBUGP("using port %hu (2 of 2)\n", hiport);
                break;
            }
        }
        if (loport != 0 && hiport != 0)
        {
            rbuf1len = sprintf(rbuf1, "%hu", loport);
            if (hiport == loport+1)
            {
                rbufalen = sprintf(rbufa, "%hu-%hu", loport, hiport);
            }
            else
            {
                rbufalen = sprintf(rbufa, "%hu/%hu", loport, hiport);
            }
        }
        break;
    }

    if (rbuf1len == 0)
    {
        return 0;   /* cannot get replacement port(s) */
    }

    /* Transport: tran;field;field=val,tran;field;field=val,... */
    while (off < tranlen)
    {
        uint        saveoff;
        const char* pparamend;
        uint        nextparamoff;

        pparamend = memchr(ptran+off, ',', tranlen-off);
        pparamend = (pparamend == NULL) ? ptran+tranlen : pparamend+1;
        nextparamoff = pparamend-ptcp;

        /*
         * We pass over each param twice.  On the first pass, we look for a
         * destination= field.  It is handled by the security policy.  If it
         * is present, allowed, and equal to our external address, we assume
         * that STUN is being used and we leave the client_port= field alone.
         */
        is_stun = 0;
        saveoff = off;
        while (off < nextparamoff)
        {
            const char* pfieldend;
            uint        nextfieldoff;

            pfieldend = memchr(ptran+off, ';', nextparamoff-off);
            nextfieldoff = (pfieldend == NULL) ? nextparamoff : pfieldend-ptran+1;

            if (dstact != DSTACT_NONE && strncmp(ptran+off, "destination=", 12) == 0)
            {
                if (strncmp(ptran+off+12, szextaddr, extaddrlen) == 0)
                {
                    is_stun = 1;
                }
                if (dstact == DSTACT_STRIP || (dstact == DSTACT_AUTO && !is_stun))
                {
                    diff = nextfieldoff-off;
                    if (!ip_nat_mangle_tcp_packet(pskb, ct, ctinfo,
                                                         off, diff, NULL, 0))
                    {
                        /* mangle failed, all we can do is bail */
                        return 0;
                    }
                    get_skb_tcpdata(*pskb, &ptcp, &tcplen);
                    ptran = ptcp+tranoff;
                    tranlen -= diff;
                    nextparamoff -= diff;
                    nextfieldoff -= diff;
                }
            }

            off = nextfieldoff;
        }
        if (is_stun)
        {
            continue;
        }
        off = saveoff;
        while (off < nextparamoff)
        {
            const char* pfieldend;
            uint        nextfieldoff;

            pfieldend = memchr(ptran+off, ';', nextparamoff-off);
            nextfieldoff = (pfieldend == NULL) ? nextparamoff : pfieldend-ptran+1;

            if (strncmp(ptran+off, "client_port=", 12) == 0)
            {
                u_int16_t   port;
                uint        numlen;
                uint        origoff;
                uint        origlen;
                char*       rbuf    = rbuf1;
                uint        rbuflen = rbuf1len;

                off += 12;
                origoff = (ptran-ptcp)+off;
                origlen = 0;
                numlen = nf_strtou16(ptran+off, &port);
                off += numlen;
                origlen += numlen;
                if (port != prtspexp->loport)
                {
                    DEBUGP("multiple ports found, port %hu ignored\n", port);
                }
                else
                {
                    if (ptran[off] == '-' || ptran[off] == '/')
                    {
                        off++;
                        origlen++;
                        numlen = nf_strtou16(ptran+off, &port);
                        off += numlen;
                        origlen += numlen;
                        rbuf = rbufa;
                        rbuflen = rbufalen;
                    }

                    /*
                     * note we cannot just memcpy() if the sizes are the same.
                     * the mangle function does skb resizing, checks for a
                     * cloned skb, and updates the checksums.
                     *
                     * parameter 4 below is offset from start of tcp data.
                     */
                    diff = origlen-rbuflen;
                    if (!ip_nat_mangle_tcp_packet(pskb, ct, ctinfo,
                                              origoff, origlen, rbuf, rbuflen))
                    {
                        /* mangle failed, all we can do is bail */
                        return 0;
                    }
                    get_skb_tcpdata(*pskb, &ptcp, &tcplen);
                    ptran = ptcp+tranoff;
                    tranlen -= diff;
                    nextparamoff -= diff;
                    nextfieldoff -= diff;
                }
            }

            off = nextfieldoff;
        }

        off = nextparamoff;
    }

    return 1;
}

static unsigned int
expected(struct sk_buff **pskb, uint hooknum, struct ip_conntrack* ct, struct ip_nat_info* info)
{
    struct ip_nat_multi_range mr;
    u_int32_t newdstip, newsrcip, newip;

    struct ip_conntrack *master = master_ct(ct);

    IP_NF_ASSERT(info);
    IP_NF_ASSERT(master);

    IP_NF_ASSERT(!(info->initialized & (1 << HOOK2MANIP(hooknum))));

    newdstip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
    newsrcip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
    newip = (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC) ? newsrcip : newdstip;

    DEBUGP("newsrcip=%u.%u.%u.%u, newdstip=%u.%u.%u.%u, newip=%u.%u.%u.%u\n",
           NIPQUAD(newsrcip), NIPQUAD(newdstip), NIPQUAD(newip));

    mr.rangesize = 1;
    /* We don't want to manip the per-protocol, just the IPs. */
    mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
    mr.range[0].min_ip = mr.range[0].max_ip = newip;

    return ip_nat_setup_info(ct, &mr, hooknum);
}

static uint
help_out(struct ip_conntrack* ct, enum ip_conntrack_info ctinfo,
         struct ip_conntrack_expect* exp, struct sk_buff** pskb)
{
    char*   ptcp;
    uint    tcplen;
    uint    hdrsoff;
    uint    hdrslen;
    uint    lineoff;
    uint    linelen;
    uint    off;

    struct iphdr* iph = (struct iphdr*)(*pskb)->nh.iph;
    struct tcphdr* tcph = (struct tcphdr*)((void*)iph + iph->ihl*4);

    struct ip_ct_rtsp_expect* prtspexp = &exp->help.exp_rtsp_info;

    get_skb_tcpdata(*pskb, &ptcp, &tcplen);

    hdrsoff = exp->seq - ntohl(tcph->seq);
    hdrslen = prtspexp->len;
    off = hdrsoff;

    while (nf_mime_nextline(ptcp, hdrsoff+hdrslen, &off, &lineoff, &linelen))
    {
        if (linelen == 0)
        {
            break;
        }
        if (off > hdrsoff+hdrslen)
        {
            INFOP("!! overrun !!");
            break;
        }
        DEBUGP("hdr: len=%u, %.*s", linelen, (int)linelen, ptcp+lineoff);

        if (nf_strncasecmp(ptcp+lineoff, "Transport:", 10) == 0)
        {
            uint oldtcplen = tcplen;
            if (!rtsp_mangle_tran(ct, ctinfo, exp, pskb, lineoff, linelen))
            {
                break;
            }
            get_skb_tcpdata(*pskb, &ptcp, &tcplen);
            hdrslen -= (oldtcplen-tcplen);
            off -= (oldtcplen-tcplen);
            lineoff -= (oldtcplen-tcplen);
            linelen -= (oldtcplen-tcplen);
            DEBUGP("rep: len=%u, %.*s", linelen, (int)linelen, ptcp+lineoff);
        }
    }

    return NF_ACCEPT;
}

static uint
help_in(struct ip_conntrack* ct, enum ip_conntrack_info ctinfo,
         struct ip_conntrack_expect* exp, struct sk_buff** pskb)
{
    /* XXX: unmangle */
    return NF_ACCEPT;
}

static uint
help(struct ip_conntrack* ct,
     struct ip_conntrack_expect* exp,
     struct ip_nat_info* info,
     enum ip_conntrack_info ctinfo,
     unsigned int hooknum,
     struct sk_buff** pskb)
{
    struct iphdr*  iph  = (struct iphdr*)(*pskb)->nh.iph;
    struct tcphdr* tcph = (struct tcphdr*)((char*)iph + iph->ihl * 4);
    uint datalen;
    int dir;
    struct ip_ct_rtsp_expect* ct_rtsp_info;
    int rc = NF_ACCEPT;

    if (ct == NULL || exp == NULL || info == NULL || pskb == NULL)
    {
        DEBUGP("!! null ptr (%p,%p,%p,%p) !!\n", ct, exp, info, pskb);
        return NF_ACCEPT;
    }

    ct_rtsp_info = &exp->help.exp_rtsp_info;

    /*
     * Only mangle things once: original direction in POST_ROUTING
     * and reply direction on PRE_ROUTING.
     */
    dir = CTINFO2DIR(ctinfo);
    if (!((hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
          || (hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY)))
    {
        DEBUGP("Not touching dir %s at hook %s\n",
               dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
               hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
               : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
               : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???");
        return NF_ACCEPT;
    }
    DEBUGP("got beyond not touching\n");

    datalen = (*pskb)->len - iph->ihl * 4 - tcph->doff * 4;

    LOCK_BH(&ip_rtsp_lock);
    /* Ensure the packet contains all of the marked data */
    if (!between(exp->seq + ct_rtsp_info->len,
                 ntohl(tcph->seq), ntohl(tcph->seq) + datalen))
    {
        /* Partial retransmission?  Probably a hacker. */
        if (net_ratelimit())
        {
            INFOP("partial packet %u/%u in %u/%u\n",
                   exp->seq, ct_rtsp_info->len, ntohl(tcph->seq), ntohl(tcph->seq) + datalen);
        }
        UNLOCK_BH(&ip_rtsp_lock);
        return NF_DROP;
    }

    switch (dir)
    {
    case IP_CT_DIR_ORIGINAL:
        rc = help_out(ct, ctinfo, exp, pskb);
        break;
    case IP_CT_DIR_REPLY:
        rc = help_in(ct, ctinfo, exp, pskb);
        break;
    }
    UNLOCK_BH(&ip_rtsp_lock);

    return rc;
}

static struct ip_nat_helper ip_nat_rtsp_helpers[MAX_PORTS];
static char rtsp_names[MAX_PORTS][10];

/* This function is intentionally _NOT_ defined as  __exit */
static void
fini(void)
{
    int i;

    for (i = 0; i < num_ports; i++)
    {
        DEBUGP("unregistering helper for port %d\n", ports[i]);
        ip_nat_helper_unregister(&ip_nat_rtsp_helpers[i]);
    }
}

static int __init
init(void)
{
    int ret = 0;
    int i;
    struct ip_nat_helper* hlpr;
    char* tmpname;

    printk("ip_nat_rtsp v" IP_NF_RTSP_VERSION " loading\n");

    if (ports[0] == 0)
    {
        ports[0] = RTSP_PORT;
    }

    for (i = 0; (i < MAX_PORTS) && ports[i] != 0; i++)
    {
        hlpr = &ip_nat_rtsp_helpers[i];
        memset(hlpr, 0, sizeof(struct ip_nat_helper));

        hlpr->tuple.dst.protonum = IPPROTO_TCP;
        hlpr->tuple.src.u.tcp.port = htons(ports[i]);
        hlpr->mask.src.u.tcp.port = 0xFFFF;
        hlpr->mask.dst.protonum = 0xFFFF;
        hlpr->help = help;
        hlpr->flags = 0;
        hlpr->me = THIS_MODULE;
        hlpr->expect = expected;

        tmpname = &rtsp_names[i][0];
        if (ports[i] == RTSP_PORT)
        {
                sprintf(tmpname, "rtsp");
        }
        else
        {
                sprintf(tmpname, "rtsp-%d", i);
        }
        hlpr->name = tmpname;

        DEBUGP("registering helper for port %d: name %s\n", ports[i], hlpr->name);
        ret = ip_nat_helper_register(hlpr);

        if (ret)
        {
            printk("ip_nat_rtsp: error registering helper for port %d\n", ports[i]);
            fini();
            return 1;
        }
        num_ports++;
    }
    if (stunaddr != NULL)
    {
        extip = in_aton(stunaddr);
    }
    if (destaction != NULL)
    {
        if (strcmp(destaction, "auto") == 0)
        {
            dstact = DSTACT_AUTO;
        }
        if (strcmp(destaction, "strip") == 0)
        {
            dstact = DSTACT_STRIP;
        }
        if (strcmp(destaction, "none") == 0)
        {
            dstact = DSTACT_NONE;
        }
    }
    return ret;
}

module_init(init);
module_exit(fini);
