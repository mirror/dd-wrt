/*
 * RTSP extension for IP connection tracking
 * (C) 2003 by Tom Marshall <tmarshall@real.com>
 * based on ip_conntrack_irc.c
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 * Module load syntax:
 *   insmod ip_conntrack_rtsp.o ports=port1,port2,...port<MAX_PORTS>
 *                              max_outstanding=n setup_timeout=secs
 *
 * If no ports are specified, the default will be port 554.
 *
 * With max_outstanding you can define the maximum number of not yet
 * answered SETUP requests per RTSP session (default 8).
 * With setup_timeout you can specify how long the system waits for
 * an expected data channel (default 300 seconds).
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_rtsp.h>

#include <linux/ctype.h>
#define NF_NEED_STRNCASECMP
#define NF_NEED_STRTOU16
#define NF_NEED_STRTOU32
#define NF_NEED_NEXTLINE
#include <linux/netfilter_helpers.h>
#define NF_NEED_MIME_NEXTLINE
#include <linux/netfilter_mime.h>

#define MAX_SIMUL_SETUP 8 /* XXX: use max_outstanding */

#define INFOP(fmt, args...) printk(KERN_INFO "%s: %s: " fmt, __FILE__, __FUNCTION__ , ## args)
#ifdef IP_NF_RTSP_DEBUG
#define DEBUGP(fmt, args...) printk(KERN_DEBUG "%s: %s: " fmt, __FILE__, __FUNCTION__ , ## args)
#else
#define DEBUGP(fmt, args...)
#endif

#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int num_ports = 0;
static int max_outstanding = 8;
static unsigned int setup_timeout = 300;

MODULE_AUTHOR("Tom Marshall <tmarshall@real.com>");
MODULE_DESCRIPTION("RTSP connection tracking module");
MODULE_LICENSE("GPL");
#ifdef MODULE_PARM
MODULE_PARM(ports, "1-" __MODULE_STRING(MAX_PORTS) "i");
MODULE_PARM_DESC(ports, "port numbers of RTSP servers");
MODULE_PARM(max_outstanding, "i");
MODULE_PARM_DESC(max_outstanding, "max number of outstanding SETUP requests per RTSP session");
MODULE_PARM(setup_timeout, "i");
MODULE_PARM_DESC(setup_timeout, "timeout on for unestablished data channels");
#endif

DECLARE_LOCK(ip_rtsp_lock);
struct module* ip_conntrack_rtsp = THIS_MODULE;

/*
 * Max mappings we will allow for one RTSP connection (for RTP, the number
 * of allocated ports is twice this value).  Note that SMIL burns a lot of
 * ports so keep this reasonably high.  If this is too low, you will see a
 * lot of "no free client map entries" messages.
 */
#define MAX_PORT_MAPS 16

/*** default port list was here in the masq code: 554, 3030, 4040 ***/

#define SKIP_WSPACE(ptr,len,off) while(off < len && isspace(*(ptr+off))) { off++; }

/*
 * Parse an RTSP packet.
 *
 * Returns zero if parsing failed.
 *
 * Parameters:
 *  IN      ptcp        tcp data pointer
 *  IN      tcplen      tcp data len
 *  IN/OUT  ptcpoff     points to current tcp offset
 *  OUT     phdrsoff    set to offset of rtsp headers
 *  OUT     phdrslen    set to length of rtsp headers
 *  OUT     pcseqoff    set to offset of CSeq header
 *  OUT     pcseqlen    set to length of CSeq header
 */
static int
rtsp_parse_message(char* ptcp, uint tcplen, uint* ptcpoff,
                   uint* phdrsoff, uint* phdrslen,
                   uint* pcseqoff, uint* pcseqlen)
{
    uint    entitylen = 0;
    uint    lineoff;
    uint    linelen;

    if (!nf_nextline(ptcp, tcplen, ptcpoff, &lineoff, &linelen))
    {
        return 0;
    }

    *phdrsoff = *ptcpoff;
    while (nf_mime_nextline(ptcp, tcplen, ptcpoff, &lineoff, &linelen))
    {
        if (linelen == 0)
        {
            if (entitylen > 0)
            {
                *ptcpoff += min(entitylen, tcplen - *ptcpoff);
            }
            break;
        }
        if (lineoff+linelen > tcplen)
        {
            INFOP("!! overrun !!\n");
            break;
        }

        if (nf_strncasecmp(ptcp+lineoff, "CSeq:", 5) == 0)
        {
            *pcseqoff = lineoff;
            *pcseqlen = linelen;
        }
        if (nf_strncasecmp(ptcp+lineoff, "Content-Length:", 15) == 0)
        {
            uint off = lineoff+15;
            SKIP_WSPACE(ptcp+lineoff, linelen, off);
            nf_strtou32(ptcp+off, &entitylen);
        }
    }
    *phdrslen = (*ptcpoff) - (*phdrsoff);

    return 1;
}

/*
 * Find lo/hi client ports (if any) in transport header
 * In:
 *   ptcp, tcplen = packet
 *   tranoff, tranlen = buffer to search
 *
 * Out:
 *   pport_lo, pport_hi = lo/hi ports (host endian)
 *
 * Returns nonzero if any client ports found
 *
 * Note: it is valid (and expected) for the client to request multiple
 * transports, so we need to parse the entire line.
 */
static int
rtsp_parse_transport(char* ptran, uint tranlen,
                     struct ip_ct_rtsp_expect* prtspexp)
{
    int     rc = 0;
    uint    off = 0;

    if (tranlen < 10 || !iseol(ptran[tranlen-1]) ||
        nf_strncasecmp(ptran, "Transport:", 10) != 0)
    {
        INFOP("sanity check failed\n");
        return 0;
    }
    DEBUGP("tran='%.*s'\n", (int)tranlen, ptran);
    off += 10;
    SKIP_WSPACE(ptran, tranlen, off);

    /* Transport: tran;field;field=val,tran;field;field=val,... */
    while (off < tranlen)
    {
        const char* pparamend;
        uint        nextparamoff;

        pparamend = memchr(ptran+off, ',', tranlen-off);
        pparamend = (pparamend == NULL) ? ptran+tranlen : pparamend+1;
        nextparamoff = pparamend-ptran;

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

                off += 12;
                numlen = nf_strtou16(ptran+off, &port);
                off += numlen;
                if (prtspexp->loport != 0 && prtspexp->loport != port)
                {
                    DEBUGP("multiple ports found, port %hu ignored\n", port);
                }
                else
                {
                    prtspexp->loport = prtspexp->hiport = port;
                    if (ptran[off] == '-')
                    {
                        off++;
                        numlen = nf_strtou16(ptran+off, &port);
                        off += numlen;
                        prtspexp->pbtype = pb_range;
                        prtspexp->hiport = port;

                        // If we have a range, assume rtp:
                        // loport must be even, hiport must be loport+1
                        if ((prtspexp->loport & 0x0001) != 0 ||
                            prtspexp->hiport != prtspexp->loport+1)
                        {
                            DEBUGP("incorrect range: %hu-%hu, correcting\n",
                                   prtspexp->loport, prtspexp->hiport);
                            prtspexp->loport &= 0xfffe;
                            prtspexp->hiport = prtspexp->loport+1;
                        }
                    }
                    else if (ptran[off] == '/')
                    {
                        off++;
                        numlen = nf_strtou16(ptran+off, &port);
                        off += numlen;
                        prtspexp->pbtype = pb_discon;
                        prtspexp->hiport = port;
                    }
                    rc = 1;
                }
            }

            /*
             * Note we don't look for the destination parameter here.
             * If we are using NAT, the NAT module will handle it.  If not,
             * and the client is sending packets elsewhere, the expectation
             * will quietly time out.
             */

            off = nextfieldoff;
        }

        off = nextparamoff;
    }

    return rc;
}

/*** conntrack functions ***/

/* outbound packet: client->server */
static int
help_out(const struct iphdr* iph, size_t pktlen,
                struct ip_conntrack* ct, enum ip_conntrack_info ctinfo)
{
    int dir = CTINFO2DIR(ctinfo);   /* = IP_CT_DIR_ORIGINAL */
    struct  tcphdr* tcph = (void*)iph + iph->ihl * 4;
    uint    tcplen = pktlen - iph->ihl * 4;
    char*   pdata = (char*)tcph + tcph->doff * 4;
    uint    datalen = tcplen - tcph->doff * 4;
    uint    dataoff = 0;

    struct ip_conntrack_expect exp;

    while (dataoff < datalen)
    {
        uint    cmdoff = dataoff;
        uint    hdrsoff = 0;
        uint    hdrslen = 0;
        uint    cseqoff = 0;
        uint    cseqlen = 0;
        uint    lineoff = 0;
        uint    linelen = 0;
        uint    off;
        int     rc;

        if (!rtsp_parse_message(pdata, datalen, &dataoff,
                                &hdrsoff, &hdrslen,
                                &cseqoff, &cseqlen))
        {
            break;      /* not a valid message */
        }

        if (strncmp(pdata+cmdoff, "SETUP ", 6) != 0)
        {
            continue;   /* not a SETUP message */
        }
        DEBUGP("found a setup message\n");

        memset(&exp, 0, sizeof(exp));

        off = 0;
        while (nf_mime_nextline(pdata+hdrsoff, hdrslen, &off,
                                &lineoff, &linelen))
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

            if (nf_strncasecmp(pdata+hdrsoff+lineoff, "Transport:", 10) == 0)
            {
                rtsp_parse_transport(pdata+hdrsoff+lineoff, linelen,
                                     &exp.help.exp_rtsp_info);
            }
        }

        if (exp.help.exp_rtsp_info.loport == 0)
        {
            DEBUGP("no udp transports found\n");
            continue;   /* no udp transports found */
        }

        DEBUGP("udp transport found, ports=(%d,%hu,%hu)\n",
              (int)exp.help.exp_rtsp_info.pbtype,
              exp.help.exp_rtsp_info.loport,
              exp.help.exp_rtsp_info.hiport);

        LOCK_BH(&ip_rtsp_lock);
        exp.seq = ntohl(tcph->seq) + hdrsoff; /* mark all the headers */
        exp.help.exp_rtsp_info.len = hdrslen;

        exp.tuple.src.ip = ct->tuplehash[!dir].tuple.src.ip;
        exp.mask.src.ip  = 0xffffffff;
        exp.tuple.dst.ip = ct->tuplehash[dir].tuple.src.ip;
        exp.mask.dst.ip  = 0xffffffff;
        exp.tuple.dst.u.udp.port = exp.help.exp_rtsp_info.loport;
        exp.mask.dst.u.udp.port  = (exp.help.exp_rtsp_info.pbtype == pb_range) ? 0xfffe : 0xffff;
        exp.tuple.dst.protonum = IPPROTO_UDP;
        exp.mask.dst.protonum  = 0xffff;

        DEBUGP("expect_related %u.%u.%u.%u:%u-%u.%u.%u.%u:%u\n",
                NIPQUAD(exp.tuple.src.ip),
                ntohs(exp.tuple.src.u.tcp.port),
                NIPQUAD(exp.tuple.dst.ip),
                ntohs(exp.tuple.dst.u.tcp.port));

        /* pass the request off to the nat helper */
        rc = ip_conntrack_expect_related(ct, &exp);
        UNLOCK_BH(&ip_rtsp_lock);
        if (rc == 0)
        {
            DEBUGP("ip_conntrack_expect_related succeeded\n");
        }
        else
        {
            INFOP("ip_conntrack_expect_related failed (%d)\n", rc);
        }
    }

    return NF_ACCEPT;
}

/* inbound packet: server->client */
static int
help_in(const struct iphdr* iph, size_t pktlen,
                struct ip_conntrack* ct, enum ip_conntrack_info ctinfo)
{
    return NF_ACCEPT;
}

static int
help(const struct iphdr* iph, size_t pktlen,
                struct ip_conntrack* ct, enum ip_conntrack_info ctinfo)
{
    /* tcplen not negative guarenteed by ip_conntrack_tcp.c */
    struct tcphdr* tcph = (void*)iph + iph->ihl * 4;
    u_int32_t tcplen = pktlen - iph->ihl * 4;

    /* Until there's been traffic both ways, don't look in packets. */
    if (ctinfo != IP_CT_ESTABLISHED && ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY)
    {
        DEBUGP("conntrackinfo = %u\n", ctinfo);
        return NF_ACCEPT;
    }

    /* Not whole TCP header? */
    if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff * 4)
    {
        DEBUGP("tcplen = %u\n", (unsigned)tcplen);
        return NF_ACCEPT;
    }

    /* Checksum invalid?  Ignore. */
    /* FIXME: Source route IP option packets --RR */
    if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
                     csum_partial((char*)tcph, tcplen, 0)))
    {
        DEBUGP("bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
               tcph, tcplen, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
        return NF_ACCEPT;
    }

    switch (CTINFO2DIR(ctinfo))
    {
    case IP_CT_DIR_ORIGINAL:
        help_out(iph, pktlen, ct, ctinfo);
        break;
    case IP_CT_DIR_REPLY:
        help_in(iph, pktlen, ct, ctinfo);
        break;
    }

    return NF_ACCEPT;
}

static struct ip_conntrack_helper rtsp_helpers[MAX_PORTS];
static char rtsp_names[MAX_PORTS][10];

/* This function is intentionally _NOT_ defined as __exit */
static void
fini(void)
{
    int i;
    for (i = 0; i < num_ports; i++)
    {
        DEBUGP("unregistering port %d\n", ports[i]);
        ip_conntrack_helper_unregister(&rtsp_helpers[i]);
    }
}

static int __init
init(void)
{
    int i, ret;
    struct ip_conntrack_helper *hlpr;
    char *tmpname;

    printk("ip_conntrack_rtsp v" IP_NF_RTSP_VERSION " loading\n");

    if (max_outstanding < 1)
    {
        printk("ip_conntrack_rtsp: max_outstanding must be a positive integer\n");
        return -EBUSY;
    }
    if (setup_timeout < 0)
    {
        printk("ip_conntrack_rtsp: setup_timeout must be a positive integer\n");
        return -EBUSY;
    }

    /* If no port given, default to standard rtsp port */
    if (ports[0] == 0)
    {
        ports[0] = RTSP_PORT;
    }

    for (i = 0; (i < MAX_PORTS) && ports[i]; i++)
    {
        hlpr = &rtsp_helpers[i];
        memset(hlpr, 0, sizeof(struct ip_conntrack_helper));
        hlpr->tuple.src.u.tcp.port = htons(ports[i]);
        hlpr->tuple.dst.protonum = IPPROTO_TCP;
        hlpr->mask.src.u.tcp.port = 0xFFFF;
        hlpr->mask.dst.protonum = 0xFFFF;
        hlpr->max_expected = max_outstanding;
        hlpr->timeout = setup_timeout;
        hlpr->flags = IP_CT_HELPER_F_REUSE_EXPECT;
        hlpr->me = ip_conntrack_rtsp;
        hlpr->help = help;

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

        DEBUGP("port #%d: %d\n", i, ports[i]);

        ret = ip_conntrack_helper_register(hlpr);

        if (ret)
        {
            printk("ip_conntrack_rtsp: ERROR registering port %d\n", ports[i]);
            fini();
            return -EBUSY;
        }
        num_ports++;
    }
    return 0;
}

#ifdef CONFIG_IP_NF_NAT_NEEDED
EXPORT_SYMBOL(ip_rtsp_lock);
#endif

module_init(init);
module_exit(fini);
