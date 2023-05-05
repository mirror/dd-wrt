/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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
**  @file       spp_sfportscan.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      Portscan detection
**
**  NOTES
**    - User Configuration:  The following is a list of parameters that can
**      be configured through the user interface:
**
**      proto  { tcp udp icmp ip all }
**      scan_type { portscan portsweep decoy_portscan distributed_portscan all }
**      sense_level { high }    # high, medium, low
**      watch_ip { }            # list of IPs, CIDR blocks
**      ignore_scanners { }     # list of IPs, CIDR blocks
**      ignore_scanned { }      # list of IPs, CIDR blocks
**      memcap { 10000000 }     # number of max bytes to allocate
**      logfile { /tmp/ps.log } # file to log detailed portscan info
*/

#include <assert.h>
#include <sys/types.h>
#include <errno.h>

#ifndef WIN32
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif /* !WIN32 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "decode.h"
#include "encode.h"
#include "plugbase.h"
#include "generators.h"
#include "event_wrapper.h"
#include "util.h"
#include "ipobj.h"
#include "checksum.h"
#include "packet_time.h"
#include "snort.h"
#include "sfthreshold.h"
#include "sfsnprintfappend.h"
#include "sf_iph.h"
#include "session_api.h"
#include "sfdaq.h"

#include "portscan.h"

#include "profiler.h"
#include "reload.h"

#ifdef REG_TEST
#include "reg_test.h"
#endif


#define DELIMITERS " \t\n"
#define TOKEN_ARG_BEGIN "{"
#define TOKEN_ARG_END   "}"

#define PROTO_BUFFER_SIZE 256

extern char *file_name;
extern int   file_line;

tSfPolicyUserContextId portscan_config = NULL;
PortscanConfig *portscan_eval_config = NULL;

static Packet *g_tmp_pkt = NULL;
static FILE   *g_logfile = NULL;

#ifdef PERF_PROFILING
PreprocStats sfpsPerfStats;
#endif

static void PortscanResetFunction(int signal, void *foo);
static void PortscanResetStatsFunction(int signal, void *foo);
static void ParsePortscan(struct _SnortConfig *, PortscanConfig *, char *);
static void PortscanFreeConfigs(tSfPolicyUserContextId );
static void PortscanFreeConfig(PortscanConfig *);
static void PortscanOpenLogFile(struct _SnortConfig *, void *);
static int PortscanGetProtoBits(int);

#ifdef SNORT_RELOAD
static void PortscanReload(struct _SnortConfig *, char *, void **);
static int PortscanReloadVerify(struct _SnortConfig *, void *);
static void * PortscanReloadSwap(struct _SnortConfig *, void *);
static void PortscanReloadSwapFree(void *);
#endif

/*
**  NAME
**    PortscanPacketInit::
*/
/**
**  Initialize the Packet structure buffer so we can generate our
**  alert packets for portscan.  We initialize the various fields in
**  the Packet structure and set the hardware layer for easy identification
**  by user interfaces.
**
**  @return int
**
**  @retval !0 initialization failed
**  @retval  0 success
*/
#define IPPROTO_PS 0xFF

static int PortscanPacketInit(void)
{
    g_tmp_pkt = Encode_New();
    return 0;
}

void PortscanCleanExitFunction(int signal, void *foo)
{
    ps_cleanup();

    Encode_Delete(g_tmp_pkt);
    g_tmp_pkt = NULL;

    PortscanFreeConfigs(portscan_config);
    portscan_config = NULL;
}


static void PortscanResetFunction(int signal, void *foo)
{
    ps_reset();
}

static void PortscanResetStatsFunction(int signal, void *foo)
{
    return;
}

/*
**  NAME
**    MakeProtoInfo::
*/
/**
**  This routine makes the portscan payload for the events.  The listed
**  info is:
**    - priority count (number of error transmissions RST/ICMP UNREACH)
**    - connection count (number of protocol connections SYN)
**    - ip count (number of IPs that communicated with host)
**    - ip range (low to high range of IPs)
**    - port count (number of port changes that occurred on host)
**    - port range (low to high range of ports connected too)
**
**  @return integer
**
**  @retval -1 buffer not large enough
**  @retval  0 successful
*/
static int MakeProtoInfo(PS_PROTO *proto, u_char *buffer, u_int *total_size)
{
    int             dsize;
    sfaddr_t        *ip1, *ip2;


    if(!total_size || !buffer)
        return -1;

    dsize = (g_tmp_pkt->max_dsize - *total_size);

    if(dsize < PROTO_BUFFER_SIZE)
       return -1;

    ip1 = &proto->low_ip;
    ip2 = &proto->high_ip;

    if(proto->alerts == PS_ALERT_PORTSWEEP ||
       proto->alerts == PS_ALERT_PORTSWEEP_FILTERED)
    {
        SnortSnprintf((char *)buffer, PROTO_BUFFER_SIZE,
                      "Priority Count: %d\n"
                      "Connection Count: %d\n"
                      "IP Count: %d\n"
                      "Scanned IP Range: %s:",
                      proto->priority_count,
                      proto->connection_count,
                      proto->u_ip_count,
                      inet_ntoa(ip1));

        /* Now print the high ip into the buffer.  This saves us
         * from having to copy the results of inet_ntoa (which is
         * a static buffer) to avoid the reuse of that buffer when
         * more than one use of inet_ntoa is within the same printf.
         */
        SnortSnprintfAppend((char *)buffer, PROTO_BUFFER_SIZE,
                      "%s\n"
                      "Port/Proto Count: %d\n"
                      "Port/Proto Range: %d:%d\n",
                      inet_ntoa(ip2),
                      proto->u_port_count,
                      proto->low_p,
                      proto->high_p);
    }
    else
    {
        SnortSnprintf((char *)buffer, PROTO_BUFFER_SIZE,
                      "Priority Count: %d\n"
                      "Connection Count: %d\n"
                      "IP Count: %d\n"
                      "Scanner IP Range: %s:",
                      proto->priority_count,
                      proto->connection_count,
                      proto->u_ip_count,
                      inet_ntoa(ip1)
                      );

        /* Now print the high ip into the buffer.  This saves us
         * from having to copy the results of inet_ntoa (which is
         * a static buffer) to avoid the reuse of that buffer when
         * more than one use of inet_ntoa is within the same printf.
         */
        SnortSnprintfAppend((char *)buffer, PROTO_BUFFER_SIZE,
                      "%s\n"
                      "Port/Proto Count: %d\n"
                      "Port/Proto Range: %d:%d\n",
                      inet_ntoa(ip2),
                      proto->u_port_count,
                      proto->low_p,
                      proto->high_p);
    }

    dsize = SnortStrnlen((const char *)buffer, PROTO_BUFFER_SIZE);
    *total_size += dsize;

    /*
    **  Set the payload size.  This is protocol independent.
    */
    g_tmp_pkt->dsize = dsize;

    return 0;
}

static int LogPortscanAlert(Packet *p, const char *msg, uint32_t event_id,
        uint32_t event_ref, uint32_t gen_id, uint32_t sig_id)
{
    char timebuf[TIMEBUF_SIZE];
    sfaddr_t* src_addr;
    sfaddr_t* dst_addr;

    if(!p->iph_api)
        return -1;

    /* Do not log if being suppressed */
    src_addr = GET_SRC_IP(p);
    dst_addr = GET_DST_IP(p);

    if( sfthreshold_test(gen_id, sig_id, src_addr, dst_addr, p->pkth->ts.tv_sec) )
    {
        return 0;
    }

    ts_print((struct timeval *)&p->pkth->ts, timebuf);

    fprintf(g_logfile, "Time: %s\n", timebuf);

    if(event_id)
        fprintf(g_logfile, "event_id: %u\n", event_id);
    else
        fprintf(g_logfile, "event_ref: %u\n", event_ref);

    fprintf(g_logfile, "%s ", inet_ntoa(GET_SRC_ADDR(p)));
    fprintf(g_logfile, "-> %s %s\n", inet_ntoa(GET_DST_ADDR(p)), msg);
    fprintf(g_logfile, "%.*s\n", p->dsize, p->data);

    fflush(g_logfile);

    return 0;
}

static int GeneratePSSnortEvent(Packet *p,uint32_t gen_id,uint32_t sig_id,
        uint32_t sig_rev, uint32_t class, uint32_t priority, const char *msg)
{
    unsigned int event_id;

    event_id = GenerateSnortEvent(p,gen_id,sig_id,sig_rev,class,priority,msg);

    if(g_logfile)
        LogPortscanAlert(p, msg, event_id, 0, gen_id, sig_id);

    return event_id;
}

/*
**  NAME
**    GenerateOpenPortEvent::
*/
/**
**  We have to generate open port events differently because we tag these
**  to the original portscan event.
**
**  @return int
**
**  @retval 0 success
*/
static int GenerateOpenPortEvent(Packet *p, uint32_t gen_id, uint32_t sig_id,
        uint32_t sig_rev, uint32_t class, uint32_t pri,
        uint32_t event_ref, struct timeval *event_time, const char *msg)
{
    Event event;

    /*
    **  This means that we logged an open port, but we don't have a event
    **  reference for it, so we don't log a snort event.  We still keep
    **  track of it though.
    */
    if(!event_ref)
        return 0;

    /* reset the thresholding subsystem checks for this packet */
    sfthreshold_reset();

#if !defined(FEAT_OPEN_APPID)
    SetEvent(&event, gen_id, sig_id, sig_rev, class, pri, event_ref);
#else /* defined(FEAT_OPEN_APPID) */
    SetEvent(&event, gen_id, sig_id, sig_rev, class, pri, event_ref, NULL);
#endif /* defined(FEAT_OPEN_APPID) */
    //CallAlertFuncs(p,msg,NULL,&event);

    event.ref_time.tv_sec  = event_time->tv_sec;
    event.ref_time.tv_usec = event_time->tv_usec;

    if(p)
    {
        /*
         * Do threshold test for suppression and thresholding.  We have to do it
         * here since these are tagged packets, which aren't subject to thresholding,
         * but we want to do it for open port events.
         */
        if( sfthreshold_test(gen_id, sig_id, GET_SRC_IP(p),
                            GET_DST_IP(p), p->pkth->ts.tv_sec) )
        {
            return 0;
        }

        CallLogFuncs(p,msg,NULL,&event);
    }
    else
    {
        return -1;
    }

    if(g_logfile)
        LogPortscanAlert(p, msg, 0, event_ref, gen_id, sig_id);

    return event.event_id;
}

/*
**  NAME
**    MakeOpenPortInfo::
*/
/**
**  Write out the open ports info for open port alerts.
**
**  @return integer
*/
static int MakeOpenPortInfo(PS_PROTO *proto, u_char *buffer, u_int *total_size,
         void *user)
{
    int dsize;

    if(!total_size || !buffer)
        return -1;

    dsize = (g_tmp_pkt->max_dsize - *total_size);

    if(dsize < PROTO_BUFFER_SIZE)
       return -1;

    SnortSnprintf((char *)buffer, PROTO_BUFFER_SIZE,
                  "Open Port: %u\n", *((unsigned short *)user));

    dsize = SnortStrnlen((const char *)buffer, PROTO_BUFFER_SIZE);
    *total_size += dsize;

    /*
    **  Set the payload size.  This is protocol independent.
    */
    g_tmp_pkt->dsize = dsize;

    return 0;
}

/*
**  NAME
**    MakePortscanPkt::
*/
/*
**  We have to create this fake packet so portscan data can be passed
**  through the unified output.
**
**  We want to copy the network and transport layer headers into our
**  fake packet.
**
*/
static int MakePortscanPkt(PS_PKT *ps_pkt, PS_PROTO *proto, int proto_type,
        void *user)
{
    unsigned int ip_size = 0;
    Packet* p = (Packet *)ps_pkt->pkt;
    EncodeFlags flags = ENC_FLAG_NET;

    if (!IsIP(p))
        return -1;

    if ( !ps_pkt->reverse_pkt )
        flags |= ENC_FLAG_FWD;

    if (p != g_tmp_pkt)
    {
#if defined(HAVE_DAQ_ADDRESS_SPACE_ID) && defined(DAQ_VERSION) && DAQ_VERSION > 6
      DAQ_PktHdr_t phdr;
      memcpy(&phdr, &p->pkth, sizeof(*p->pkth));
      if (p->pkth->flags & DAQ_PKT_FLAG_REAL_ADDRESSES)
      {
        phdr.flags &= ~(DAQ_PKT_FLAG_REAL_SIP_V6 | DAQ_PKT_FLAG_REAL_DIP_V6);
        if (flags & ENC_FLAG_FWD)
        {
          phdr.flags |= phdr.flags & (DAQ_PKT_FLAG_REAL_SIP_V6 | DAQ_PKT_FLAG_REAL_DIP_V6);
          phdr.real_sIP = p->pkth->real_sIP;
          phdr.real_dIP = p->pkth->real_dIP;
        }
        else
        {
          if (p->pkth->flags & DAQ_PKT_FLAG_REAL_SIP_V6)
            phdr.flags |= DAQ_PKT_FLAG_REAL_DIP_V6;
          if (p->pkth->flags & DAQ_PKT_FLAG_REAL_DIP_V6)
            phdr.flags |= DAQ_PKT_FLAG_REAL_SIP_V6;
          phdr.real_sIP = p->pkth->real_dIP;
          phdr.real_dIP = p->pkth->real_sIP;
        }

      }
      Encode_Format_With_DAQ_Info(flags, p, g_tmp_pkt, PSEUDO_PKT_PS, &phdr, 0);
#elif defined(HAVE_DAQ_ACQUIRE_WITH_META) && defined(DAQ_VERSION) && DAQ_VERSION > 6
      Encode_Format_With_DAQ_Info(flags, p, g_tmp_pkt, PSEUDO_PKT_PS, 0);
#else
        Encode_Format(flags, p, g_tmp_pkt, PSEUDO_PKT_PS);
#endif
    }

    switch (proto_type)
    {
        case PS_PROTO_TCP:
            g_tmp_pkt->ps_proto = IPPROTO_TCP;
            break;
        case PS_PROTO_UDP:
            g_tmp_pkt->ps_proto = IPPROTO_UDP;
            break;
        case PS_PROTO_ICMP:
            g_tmp_pkt->ps_proto = IPPROTO_ICMP;
            break;
        case PS_PROTO_IP:
            g_tmp_pkt->ps_proto = IPPROTO_IP;
            break;
        case PS_PROTO_OPEN_PORT:
            g_tmp_pkt->ps_proto = GET_IPH_PROTO(p);
            break;
        default:
            return -1;
    }

    if(IS_IP4(p))
    {
        ((IPHdr*)g_tmp_pkt->iph)->ip_proto = IPPROTO_PS;
        g_tmp_pkt->inner_ip4h.ip_proto = IPPROTO_PS;
    }
    else
    {
        if ( g_tmp_pkt->raw_ip6h )
            ((IP6RawHdr*)g_tmp_pkt->raw_ip6h)->ip6nxt = IPPROTO_PS;
        g_tmp_pkt->inner_ip6h.next = IPPROTO_PS;
        g_tmp_pkt->ip6h = &g_tmp_pkt->inner_ip6h;
    }

    switch(proto_type)
    {
        case PS_PROTO_TCP:
        case PS_PROTO_UDP:
        case PS_PROTO_ICMP:
        case PS_PROTO_IP:
            if(MakeProtoInfo(proto, (u_char*)g_tmp_pkt->data, &ip_size))
                return -1;

            break;

        case PS_PROTO_OPEN_PORT:
            if(MakeOpenPortInfo(proto, (u_char*)g_tmp_pkt->data, &ip_size, user))
                return -1;

            break;

        default:
            return -1;
    }

    /*
    **  Let's finish up the IP header and checksum.
    */
    Encode_Update(g_tmp_pkt);

    if(IS_IP4(g_tmp_pkt))
    {
        g_tmp_pkt->inner_ip4h.ip_len = ((IPHdr *)g_tmp_pkt->iph)->ip_len;
    }
    else if (IS_IP6(g_tmp_pkt))
    {
        g_tmp_pkt->inner_ip6h.len = htons((uint16_t)ip_size);
    }

    return 0;
}

static int PortscanAlertTcp(Packet *p, PS_PROTO *proto, int proto_type)
{
    int iCtr;
    unsigned int event_ref;
    int portsweep = 0;

    if(!proto)
        return -1;

    switch(proto->alerts)
    {
        case PS_ALERT_ONE_TO_ONE:
            event_ref = GeneratePSSnortEvent(p, GENERATOR_PSNG,
                    PSNG_TCP_PORTSCAN, 0, 0, 3, PSNG_TCP_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY:
            event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_TCP_DECOY_PORTSCAN,0,0,3,PSNG_TCP_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP:
           event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                   PSNG_TCP_PORTSWEEP, 0, 0, 3, PSNG_TCP_PORTSWEEP_STR);
           portsweep = 1;

           break;

        case PS_ALERT_DISTRIBUTED:
            event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_TCP_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_TCP_DISTRIBUTED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_FILTERED:
            event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_TCP_FILTERED_PORTSCAN,0,0,3,
                    PSNG_TCP_FILTERED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY_FILTERED:
            event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_TCP_FILTERED_DECOY_PORTSCAN, 0,0,3,
                    PSNG_TCP_FILTERED_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                   PSNG_TCP_PORTSWEEP_FILTERED,0,0,3,
                   PSNG_TCP_PORTSWEEP_FILTERED_STR);
           portsweep = 1;

           return 0;

        case PS_ALERT_DISTRIBUTED_FILTERED:
            event_ref = GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_TCP_FILTERED_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_TCP_FILTERED_DISTRIBUTED_PORTSCAN_STR);
            break;

        default:
            return 0;
    }

    /*
    **  Set the current event reference information for any open ports.
    */
    proto->event_ref  = event_ref;
    proto->event_time.tv_sec  = p->pkth->ts.tv_sec;
    proto->event_time.tv_usec = p->pkth->ts.tv_usec;

    /*
    **  Only log open ports for portsweeps after the alert has been
    **  generated.
    */
    if(proto->open_ports_cnt && !portsweep)
    {
        for(iCtr = 0; iCtr < proto->open_ports_cnt; iCtr++)
        {
            DAQ_PktHdr_t *pkth = (DAQ_PktHdr_t *)g_tmp_pkt->pkth;
            PS_PKT ps_pkt;

            memset(&ps_pkt, 0x00, sizeof(PS_PKT));
            ps_pkt.pkt = (void *)p;

            if(MakePortscanPkt(&ps_pkt, proto, PS_PROTO_OPEN_PORT,
                        (void *)&proto->open_ports[iCtr]))
                return -1;

            pkth->ts.tv_usec += 1;
            GenerateOpenPortEvent(g_tmp_pkt,GENERATOR_PSNG,PSNG_OPEN_PORT,
                    0,0,3, proto->event_ref, &proto->event_time,
                    PSNG_OPEN_PORT_STR);
        }
    }

    return 0;
}

static int PortscanAlertUdp(Packet *p, PS_PROTO *proto, int proto_type)
{
    if(!proto)
        return -1;

    switch(proto->alerts)
    {
        case PS_ALERT_ONE_TO_ONE:
            GeneratePSSnortEvent(p, GENERATOR_PSNG, PSNG_UDP_PORTSCAN, 0, 0, 3,
                    PSNG_UDP_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_UDP_DECOY_PORTSCAN, 0, 0, 3,
                    PSNG_UDP_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP:
           GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_UDP_PORTSWEEP, 0, 0, 3,
                    PSNG_UDP_PORTSWEEP_STR);
            break;

        case PS_ALERT_DISTRIBUTED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_UDP_DISTRIBUTED_PORTSCAN,
                    0, 0, 3, PSNG_UDP_DISTRIBUTED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_FILTERED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_UDP_FILTERED_PORTSCAN,0,0,3,
                    PSNG_UDP_FILTERED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY_FILTERED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_UDP_FILTERED_DECOY_PORTSCAN,
                    0,0,3, PSNG_UDP_FILTERED_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_UDP_PORTSWEEP_FILTERED,0,0,3,
                    PSNG_UDP_PORTSWEEP_FILTERED_STR);
            break;

        case PS_ALERT_DISTRIBUTED_FILTERED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_UDP_FILTERED_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_UDP_FILTERED_DISTRIBUTED_PORTSCAN_STR);
            break;

        default:
            break;
    }

    return 0;
}

static int PortscanAlertIp(Packet *p, PS_PROTO *proto, int proto_type)
{
    if(!proto)
        return -1;

    switch(proto->alerts)
    {
        case PS_ALERT_ONE_TO_ONE:
            GeneratePSSnortEvent(p, GENERATOR_PSNG, PSNG_IP_PORTSCAN, 0, 0, 3,
                    PSNG_IP_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_IP_DECOY_PORTSCAN, 0, 0, 3,
                    PSNG_IP_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP:
           GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_IP_PORTSWEEP, 0, 0, 3,
                    PSNG_IP_PORTSWEEP_STR);
            break;

        case PS_ALERT_DISTRIBUTED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_IP_DISTRIBUTED_PORTSCAN,
                    0, 0, 3, PSNG_IP_DISTRIBUTED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_FILTERED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_IP_FILTERED_PORTSCAN,0,0,3,
                    PSNG_IP_FILTERED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY_FILTERED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_IP_FILTERED_DECOY_PORTSCAN,
                    0,0,3, PSNG_IP_FILTERED_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_IP_PORTSWEEP_FILTERED,0,0,3,
                    PSNG_IP_PORTSWEEP_FILTERED_STR);
            break;

        case PS_ALERT_DISTRIBUTED_FILTERED:
            GeneratePSSnortEvent(p,GENERATOR_PSNG,
                    PSNG_IP_FILTERED_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_IP_FILTERED_DISTRIBUTED_PORTSCAN_STR);
            break;

        default:
            break;
    }

    return 0;
}

static int PortscanAlertIcmp(Packet *p, PS_PROTO *proto, int proto_type)
{
    if(!proto)
        return -1;

    switch(proto->alerts)
    {
        case PS_ALERT_PORTSWEEP:
           GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_ICMP_PORTSWEEP, 0, 0, 3,
                    PSNG_ICMP_PORTSWEEP_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           GeneratePSSnortEvent(p,GENERATOR_PSNG,PSNG_ICMP_PORTSWEEP_FILTERED,0,0,3,
                    PSNG_ICMP_PORTSWEEP_FILTERED_STR);
            break;

        default:
            break;
    }

    return 0;
}

static int PortscanAlert(PS_PKT *ps_pkt, PS_PROTO *proto, int proto_type)
{
    Packet *p;

    if(!ps_pkt || !ps_pkt->pkt)
        return -1;

    p = (Packet *)ps_pkt->pkt;

    if(proto->alerts == PS_ALERT_OPEN_PORT)
    {
        if(MakePortscanPkt(ps_pkt, proto, PS_PROTO_OPEN_PORT, (void *)&p->sp))
            return -1;

        GenerateOpenPortEvent(g_tmp_pkt,GENERATOR_PSNG,PSNG_OPEN_PORT,0,0,3,
                proto->event_ref, &proto->event_time, PSNG_OPEN_PORT_STR);
    }
    else
    {
        if(MakePortscanPkt(ps_pkt, proto, proto_type, NULL))
            return -1;

        switch(proto_type)
        {
            case PS_PROTO_TCP:
                PortscanAlertTcp(g_tmp_pkt, proto, proto_type);
                break;

            case PS_PROTO_UDP:
                PortscanAlertUdp(g_tmp_pkt, proto, proto_type);
                break;

            case PS_PROTO_ICMP:
                PortscanAlertIcmp(g_tmp_pkt, proto, proto_type);
                break;

            case PS_PROTO_IP:
                PortscanAlertIp(g_tmp_pkt, proto, proto_type);
                break;
        }
    }

    sfthreshold_reset();

    return 0;
}

static void PortscanDetect(Packet *p, void *context)
{
    PS_PKT ps_pkt;
    tSfPolicyId policy_id = getNapRuntimePolicy();
    PortscanConfig *pPolicyConfig = NULL;
    PROFILE_VARS;

    assert(IPH_IS_VALID(p));

    if ( p->packet_flags & PKT_REBUILT_STREAM )
        return;

    sfPolicyUserPolicySet (portscan_config, policy_id);
    pPolicyConfig = (PortscanConfig *)sfPolicyUserDataGetCurrent(portscan_config);

    if ( pPolicyConfig == NULL )
        return;

    portscan_eval_config = pPolicyConfig;

    PREPROC_PROFILE_START(sfpsPerfStats);

    memset(&ps_pkt, 0x00, sizeof(PS_PKT));
    ps_pkt.pkt = (void *)p;

    /* See if there is already an exisiting node in the hash table */
    ps_detect(&ps_pkt);

    if (ps_pkt.scanner && ps_pkt.scanner->proto.alerts &&
       (ps_pkt.scanner->proto.alerts != PS_ALERT_GENERATED))
    {
        PortscanAlert(&ps_pkt, &ps_pkt.scanner->proto, ps_pkt.proto);
    }

    if (ps_pkt.scanned && ps_pkt.scanned->proto.alerts &&
        (ps_pkt.scanned->proto.alerts != PS_ALERT_GENERATED))
    {
        PortscanAlert(&ps_pkt, &ps_pkt.scanned->proto, ps_pkt.proto);
    }

    PREPROC_PROFILE_END(sfpsPerfStats);
}

NORETURN static void FatalErrorNoOption(u_char *option)
{
    FatalError("%s(%d) => No argument to '%s' config option.\n",
            file_name, file_line, option);
}

NORETURN static void FatalErrorNoEnd(char *option)
{
    FatalError("%s(%d) => No ending brace to '%s' config option.\n",
            file_name, file_line, option);
}

NORETURN static void FatalErrorInvalidArg(char *option)
{
    FatalError("%s(%d) => Invalid argument to '%s' config option.\n",
            file_name, file_line, option);
}

NORETURN static void FatalErrorInvalidOption(char *option)
{
    FatalError("%s(%d) => Invalid option '%s' to portscan preprocessor.\n",
            file_name, file_line, option);
}

static void ParseProtos(int *protos, char **savptr)
{
    char *pcTok;

    if(!protos)
        return;

    *protos = 0;

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    while(pcTok)
    {
        if(!strcasecmp(pcTok, "tcp"))
            *protos |= PS_PROTO_TCP;
        else if(!strcasecmp(pcTok, "udp"))
            *protos |= PS_PROTO_UDP;
        else if(!strcasecmp(pcTok, "icmp"))
            *protos |= PS_PROTO_ICMP;
        else if(!strcasecmp(pcTok, "ip"))
            *protos |= PS_PROTO_IP;
        else if(!strcasecmp(pcTok, "all"))
            *protos = PS_PROTO_ALL;
        else if(!strcasecmp(pcTok, TOKEN_ARG_END))
            return;
        else
            FatalErrorInvalidArg("proto");

        pcTok = strtok_r(NULL, DELIMITERS, savptr);
    }

    if(!pcTok)
        FatalErrorNoEnd("proto");

    return;
}

static void ParseScanType(int *scan_types, char **savptr)
{
    char *pcTok;

    if(!scan_types)
        return;

    *scan_types = 0;

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    while(pcTok)
    {
        if(!strcasecmp(pcTok, "portscan"))
            *scan_types |= PS_TYPE_PORTSCAN;
        else if(!strcasecmp(pcTok, "portsweep"))
            *scan_types |= PS_TYPE_PORTSWEEP;
        else if(!strcasecmp(pcTok, "decoy_portscan"))
            *scan_types |= PS_TYPE_DECOYSCAN;
        else if(!strcasecmp(pcTok, "distributed_portscan"))
            *scan_types |= PS_TYPE_DISTPORTSCAN;
        else if(!strcasecmp(pcTok, "all"))
            *scan_types = PS_TYPE_ALL;
        else if(!strcasecmp(pcTok, TOKEN_ARG_END))
            return;
        else
            FatalErrorInvalidArg("scan_type");

        pcTok = strtok_r(NULL, DELIMITERS, savptr);
    }

    if(!pcTok)
        FatalErrorNoEnd("scan_type");

    return;
}

static void ParseSenseLevel(int *sense_level, char **savptr)
{
    char *pcTok;

    if(!sense_level)
        return;

    *sense_level = 0;

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    while(pcTok)
    {
        if(!strcasecmp(pcTok, "low"))
            *sense_level = PS_SENSE_LOW;
        else if(!strcasecmp(pcTok, "medium"))
            *sense_level = PS_SENSE_MEDIUM;
        else if(!strcasecmp(pcTok, "high"))
            *sense_level = PS_SENSE_HIGH;
        else if(!strcmp(pcTok, TOKEN_ARG_END))
            return;
        else
            FatalErrorInvalidArg("sense_level");

        pcTok = strtok_r(NULL, DELIMITERS, savptr);
    }

    if(!pcTok)
        FatalErrorNoEnd("sense_level");

    return;
}

static void ParseIpList(IPSET **ip_list, char *option, char **savptr)
{
    char *pcTok;

    if(!ip_list)
        return;

    pcTok = strtok_r(NULL, TOKEN_ARG_END, savptr);
    if(!pcTok)
        FatalErrorInvalidArg(option);

    *ip_list = ipset_new();
    if(!*ip_list)
        FatalError("Failed to initialize ip_list in portscan preprocessor.\n");

    if(ipset_parse(*ip_list, pcTok))
        FatalError("%s(%d) => Invalid ip_list to '%s' option.\n",
                file_name, file_line, option);

    return;
}

static void ParseMemcap(unsigned long *memcap, char **savptr)
{
    char *pcTok;
    char *p;

    if(!memcap)
        return;

    *memcap = 0;

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    if(!pcTok)
        FatalErrorNoEnd("memcap");

    *memcap = strtoul(pcTok, &p, 10);

    if(!*pcTok || *pcTok == '-' || *p)
        FatalErrorInvalidArg("memcap");

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    if(!pcTok)
        FatalErrorNoEnd("memcap");

    if(strcmp(pcTok, TOKEN_ARG_END))
        FatalErrorInvalidArg("memcap");

    return;
}

static void PrintIPPortSet(IP_PORT *p)
{
    char ip_str[80], output_str[80];
    PORTRANGE *pr;

    SnortSnprintf(ip_str, sizeof(ip_str), "%s", sfip_to_str(&p->ip.addr));

    if(p->notflag)
        SnortSnprintf(output_str, sizeof(output_str), "        !%s", ip_str);
    else
        SnortSnprintf(output_str, sizeof(output_str), "        %s", ip_str);

    if (p->ip.bits != 128)
        SnortSnprintfAppend(output_str, sizeof(output_str), "/%d",
                            (sfaddr_family(&p->ip.addr) == AF_INET) ? ((p->ip.bits >= 96) ? p->ip.bits - 96 : -1) : p->ip.bits);

    pr=(PORTRANGE*)sflist_first(&p->portset.port_list);
    if ( pr && pr->port_lo != 0 )
        SnortSnprintfAppend(output_str, sizeof(output_str), " : ");
    for( ; pr != 0;
        pr=(PORTRANGE*)sflist_next(&p->portset.port_list) )
    {
        if ( pr->port_lo != 0)
        {
            SnortSnprintfAppend(output_str, sizeof(output_str), "%d", pr->port_lo);
            if ( pr->port_hi != pr->port_lo )
            {
                SnortSnprintfAppend(output_str, sizeof(output_str), "-%d", pr->port_hi);
            }
            SnortSnprintfAppend(output_str, sizeof(output_str), " ");
        }
    }
    LogMessage("%s\n", output_str);
}

static void PrintPortscanConf(int detect_scans, int detect_scan_type,
        int sense_level, IPSET *scanner, IPSET *scanned, IPSET *watch,
        unsigned long memcap, char *logpath, int disabled)
{
    char buf[STD_BUF + 1];
    int proto_cnt = 0;
    IP_PORT *p;

    LogMessage("Portscan Detection Config:\n");
    if(disabled)
    {
           LogMessage("    Portscan Detection: INACTIVE\n");
    }
    memset(buf, 0, STD_BUF + 1);
    if (!disabled)
    {
        SnortSnprintf(buf, STD_BUF + 1, "    Detect Protocols:  ");
        if(detect_scans & PS_PROTO_TCP)  { sfsnprintfappend(buf, STD_BUF, "TCP ");  proto_cnt++; }
        if(detect_scans & PS_PROTO_UDP)  { sfsnprintfappend(buf, STD_BUF, "UDP ");  proto_cnt++; }
        if(detect_scans & PS_PROTO_ICMP) { sfsnprintfappend(buf, STD_BUF, "ICMP "); proto_cnt++; }
        if(detect_scans & PS_PROTO_IP)   { sfsnprintfappend(buf, STD_BUF, "IP");    proto_cnt++; }
        LogMessage("%s\n", buf);
    }

    if (!disabled)
    {
        memset(buf, 0, STD_BUF + 1);
        SnortSnprintf(buf, STD_BUF + 1, "    Detect Scan Type:  ");
        if(detect_scan_type & PS_TYPE_PORTSCAN)
            sfsnprintfappend(buf, STD_BUF, "portscan ");
        if(detect_scan_type & PS_TYPE_PORTSWEEP)
            sfsnprintfappend(buf, STD_BUF, "portsweep ");
        if(detect_scan_type & PS_TYPE_DECOYSCAN)
            sfsnprintfappend(buf, STD_BUF, "decoy_portscan ");
        if(detect_scan_type & PS_TYPE_DISTPORTSCAN)
            sfsnprintfappend(buf, STD_BUF, "distributed_portscan");
        LogMessage("%s\n", buf);
    }

    if (!disabled)
    {
        memset(buf, 0, STD_BUF + 1);
        SnortSnprintf(buf, STD_BUF + 1, "    Sensitivity Level: ");
        if(sense_level == PS_SENSE_HIGH)
            sfsnprintfappend(buf, STD_BUF, "High/Experimental");
        if(sense_level == PS_SENSE_MEDIUM)
            sfsnprintfappend(buf, STD_BUF, "Medium");
        if(sense_level == PS_SENSE_LOW)
            sfsnprintfappend(buf, STD_BUF, "Low");
        LogMessage("%s\n", buf);
    }

    LogMessage("    Memcap (in bytes): %lu\n", memcap);

    if (!disabled)
    {
        LogMessage("    Number of Nodes:   %ld\n",
            memcap / (sizeof(PS_PROTO)*proto_cnt-1));

        if (logpath != NULL)
            LogMessage("    Logfile:           %s\n", logpath);

        if(scanner)
        {
            LogMessage("    Ignore Scanner IP List:\n");
            for(p = (IP_PORT*)sflist_first(&scanner->ip_list);
                p;
                p = (IP_PORT*)sflist_next(&scanner->ip_list))
            {
                PrintIPPortSet(p);
            }
        }

        if(scanned)
        {
            LogMessage("    Ignore Scanned IP List:\n");
            for(p = (IP_PORT*)sflist_first(&scanned->ip_list);
                p;
                p = (IP_PORT*)sflist_next(&scanned->ip_list))
            {
                PrintIPPortSet(p);
            }
        }

        if(watch)
        {
            LogMessage("    Watch IP List:\n");
            for(p = (IP_PORT*)sflist_first(&watch->ip_list);
                p;
                p = (IP_PORT*)sflist_next(&watch->ip_list))
            {
                PrintIPPortSet(p);
            }
        }
    }
}

static void ParseLogFile(struct _SnortConfig *sc, PortscanConfig *config, char **savptr)
{
    char *pcTok;

    if (config == NULL)
        return;

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    if (pcTok == NULL)
    {
        FatalError("%s(%d) => No ending brace to '%s' config option.\n",
                   file_name, file_line, "logfile");
    }

    config->logfile = ProcessFileOption(sc, pcTok);

    pcTok = strtok_r(NULL, DELIMITERS, savptr);
    if (pcTok == NULL)
    {
        FatalError("%s(%d) => No ending brace to '%s' config option.\n",
                   file_name, file_line, "logfile");
    }

    if (strcmp(pcTok, TOKEN_ARG_END) != 0)
    {
        FatalError("%s(%d) => Invalid argument to '%s' config option.\n",
                   file_name, file_line, "logfile");
    }
}

#ifdef REG_TEST
static inline void PrintPORTSCANSize(void)
{
    LogMessage("\nPORTSCAN Session Size: %lu\n", (long unsigned int)sizeof(PS_TRACKER));
}
#endif

static void PortscanInit(struct _SnortConfig *sc, char *args)
{
    tSfPolicyId policy_id = getParserPolicy(sc);
    PortscanConfig *pPolicyConfig = NULL;

#ifdef REG_TEST
    PrintPORTSCANSize();
#endif

    if (portscan_config == NULL)
    {
        portscan_config = sfPolicyConfigCreate();
        PortscanPacketInit();

        AddFuncToPreprocCleanExitList(PortscanCleanExitFunction, NULL, PRIORITY_SCANNER, PP_SFPORTSCAN);
        AddFuncToPreprocResetList(PortscanResetFunction, NULL, PRIORITY_SCANNER, PP_SFPORTSCAN);
        AddFuncToPreprocResetStatsList(PortscanResetStatsFunction, NULL, PRIORITY_SCANNER, PP_SFPORTSCAN);
        AddFuncToPreprocPostConfigList(sc, PortscanOpenLogFile, NULL);

#ifdef PERF_PROFILING
        RegisterPreprocessorProfile("sfportscan", &sfpsPerfStats, 0, &totalPerfStats, NULL);
#endif
    }

    if ((policy_id != 0) && (((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config)) == NULL))
    {
        ParseError("Portscan: Must configure default policy if other "
                   "policies are going to be configured.");
    }

    sfPolicyUserPolicySet (portscan_config, policy_id);
    pPolicyConfig = (PortscanConfig *)sfPolicyUserDataGetCurrent(portscan_config);
    if (pPolicyConfig)
    {
        ParseError("Can only configure sfportscan once.\n");
    }
    pPolicyConfig = (PortscanConfig *)SnortAlloc(sizeof(PortscanConfig));
    if (!pPolicyConfig)
    {
        ParseError("SFPORTSCAN preprocessor: memory allocate failed.\n");
    }

    sfPolicyUserDataSetCurrent(portscan_config, pPolicyConfig);
    ParsePortscan(sc, pPolicyConfig, args);

    if (policy_id == 0)
    {
        ps_init_hash(pPolicyConfig->memcap);
    }
    else
    {
        pPolicyConfig->memcap = ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config))->memcap;

        if (pPolicyConfig->logfile != NULL)
        {
            ParseError("Portscan:  logfile can only be configured in "
                       "default policy.\n");
        }
    }

    if ( !pPolicyConfig->disabled )
    {
        AddFuncToPreprocList(sc, PortscanDetect, PRIORITY_SCANNER, PP_SFPORTSCAN,
                             PortscanGetProtoBits(pPolicyConfig->detect_scans));
        session_api->enable_preproc_all_ports( sc,
                                               PP_SFPORTSCAN,
                                               PortscanGetProtoBits(pPolicyConfig->detect_scans) );
    }
}

void SetupSfPortscan(void)
{
#ifndef SNORT_RELOAD
    RegisterPreprocessor("sfportscan", PortscanInit);
#else
    RegisterPreprocessor("sfportscan", PortscanInit, PortscanReload,
                         PortscanReloadVerify, PortscanReloadSwap,
                         PortscanReloadSwapFree);
#endif
}

static void ParsePortscan(struct _SnortConfig *sc, PortscanConfig *config, char *args)
{
    int    sense_level = PS_SENSE_LOW;
    int    protos      = (PS_PROTO_TCP | PS_PROTO_UDP);
    int    scan_types  = PS_TYPE_ALL;
    unsigned long memcap = 1048576;
    IPSET *ignore_scanners = NULL;
    IPSET *ignore_scanned = NULL;
    IPSET *watch_ip = NULL;
    char  *pcTok, *savpcTok = NULL;
    int    iRet;

    if (args != NULL)
    {
        pcTok = strtok_r(args, DELIMITERS, &savpcTok);
        //pcTok = strtok(args, DELIMITERS);
        while(pcTok)
        {
            if(!strcasecmp(pcTok, "proto"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                //pcTok = strtok(NULL, DELIMITERS);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"proto");

                ParseProtos(&protos, &savpcTok);
            }
            else if(!strcasecmp(pcTok, "scan_type"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"scan_type");

                ParseScanType(&scan_types, &savpcTok);
            }
            else if(!strcasecmp(pcTok, "sense_level"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"sense_level");

                ParseSenseLevel(&sense_level, &savpcTok);
            }
            else if(!strcasecmp(pcTok, "ignore_scanners"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"ignore_scanners");

                ParseIpList(&ignore_scanners, "ignore_scanners", &savpcTok);
            }
            else if(!strcasecmp(pcTok, "ignore_scanned"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"ignore_scanned");

                ParseIpList(&ignore_scanned, "ignore_scanned", &savpcTok);
            }
            else if(!strcasecmp(pcTok, "watch_ip"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"watch_ip");

                ParseIpList(&watch_ip, "watch_ip", &savpcTok);
            }
            else if(!strcasecmp(pcTok, "print_tracker"))
            {
                config->print_tracker = 1;
            }
            else if(!strcasecmp(pcTok, "memcap"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"memcap");

                ParseMemcap(&memcap, &savpcTok);
            }
            else if(!strcasecmp(pcTok, "logfile"))
            {
                pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
                if(!pcTok || strcmp(pcTok, TOKEN_ARG_BEGIN))
                    FatalErrorNoOption((u_char *)"logfile");

                ParseLogFile(sc, config, &savpcTok);
            }
            else if(!strcasecmp(pcTok, "include_midstream"))
            {
                /* Do not ignore packets in sessions picked up mid-stream */
                config->include_midstream = 1;
            }
            else if(!strcasecmp(pcTok, "detect_ack_scans"))
            {
                /*
                 *  We will only see ack scan packets if we are looking at sessions that the
                 *    have been flagged as being picked up mid-stream
                 */
                config->include_midstream = 1;
            }
            else if(!strcasecmp(pcTok, "disabled"))
            {
                config->disabled = 1;
            }
            else
            {
                FatalErrorInvalidOption(pcTok);
            }

            pcTok = strtok_r(NULL, DELIMITERS, &savpcTok);
        }
    }

    iRet = ps_init(sc, config, protos, scan_types, sense_level, ignore_scanners,
                   ignore_scanned, watch_ip, memcap);
    if (iRet)
    {
        if(iRet == -2)
        {
            FatalError("%s(%d) => 'memcap' limit not sufficient to run "
                       "sfportscan preprocessor.  Please increase this "
                       "value or keep the default memory usage.\n",
                       file_name, file_line);
        }

        FatalError("Failed to initialize the sfportscan detection module.  "
                   "Please check your configuration before submitting a "
                   "bug.\n");
    }

    PrintPortscanConf(protos, scan_types, sense_level, ignore_scanners,
                      ignore_scanned, watch_ip, memcap, config->logfile, config->disabled);
}

static void PortscanOpenLogFile(struct _SnortConfig *sc, void *data)
{
    PortscanConfig *pPolicyConfig = (PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config) ;
    if ((  pPolicyConfig== NULL) ||
        (pPolicyConfig->logfile == NULL))
    {
        return;
    }

    g_logfile = fopen(pPolicyConfig->logfile, "a+");
    if (g_logfile == NULL)
    {
        FatalError("Portscan log file '%s' could not be opened: %s.\n",
                   pPolicyConfig->logfile, strerror(errno));
    }
}

static int PortscanFreeConfigPolicy(tSfPolicyUserContextId config,tSfPolicyId policyId, void* pData )
{
    PortscanConfig *pPolicyConfig = (PortscanConfig *)pData;
    sfPolicyUserDataClear (config, policyId);
    PortscanFreeConfig(pPolicyConfig);
    return 0;
}
static void PortscanFreeConfigs(tSfPolicyUserContextId config)
{

    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, PortscanFreeConfigPolicy);
    sfPolicyConfigDelete(config);

}

static void PortscanFreeConfig(PortscanConfig *config)
{
    if (config == NULL)
        return;

    if (config->logfile)
        free(config->logfile);

    if (config->ignore_scanners != NULL)
        ipset_free(config->ignore_scanners);

    if (config->ignore_scanned != NULL)
        ipset_free(config->ignore_scanned);

    if (config->watch_ip != NULL)
        ipset_free(config->watch_ip);

    free(config);
}

static int PortscanGetProtoBits(int detect_scans)
{
    int proto_bits = PROTO_BIT__IP;

    if (detect_scans & PS_PROTO_IP)
    {
        proto_bits |= PROTO_BIT__ICMP;
    }

    if (detect_scans & PS_PROTO_UDP)
    {
        proto_bits |= PROTO_BIT__ICMP;
        proto_bits |= PROTO_BIT__UDP;
    }

    if (detect_scans & PS_PROTO_ICMP)
        proto_bits |= PROTO_BIT__ICMP;

    if (detect_scans & PS_PROTO_TCP)
    {
        proto_bits |= PROTO_BIT__ICMP;
        proto_bits |= PROTO_BIT__TCP;
    }

    return proto_bits;
}

#ifdef SNORT_RELOAD
static void PortscanReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId portscan_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = getParserPolicy(sc);
    PortscanConfig *pPolicyConfig = NULL;

    if (!portscan_swap_config)
    {
        portscan_swap_config = sfPolicyConfigCreate();
        *new_config = (void *)portscan_swap_config;
    }

    if ((policy_id != 0) && (((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config)) == NULL))
    {
        ParseError("Portscan: Must configure default policy if other "
                   "policies are going to be configured.");
    }


    sfPolicyUserPolicySet (portscan_swap_config, policy_id);

    pPolicyConfig = (PortscanConfig *)sfPolicyUserDataGetCurrent(portscan_swap_config);
    if (pPolicyConfig)
    {
        ParseError("Can only configure sfportscan once.\n");
    }

    pPolicyConfig = (PortscanConfig *)SnortAlloc(sizeof(PortscanConfig));
    if (!pPolicyConfig)
    {
        ParseError("SFPORTSCAN preprocessor: memory allocate failed.\n");
    }


    sfPolicyUserDataSetCurrent(portscan_swap_config, pPolicyConfig);
    ParsePortscan(sc, pPolicyConfig, args);

    if (policy_id != 0)
    {
        pPolicyConfig->memcap = ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config))->memcap;

        if (pPolicyConfig->logfile != NULL)
        {
            ParseError("Portscan:  logfile can only be configured in "
                       "default policy.\n");
        }
    }

    if ( !pPolicyConfig->disabled )
    {
        AddFuncToPreprocList(sc, PortscanDetect, PRIORITY_SCANNER, PP_SFPORTSCAN,
                             PortscanGetProtoBits(pPolicyConfig->detect_scans));
        session_api->enable_preproc_all_ports( sc,
                                              PP_SFPORTSCAN,
                                              PortscanGetProtoBits(pPolicyConfig->detect_scans) );
    }
}

static bool PortscanReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned max_work = idle ? 512 : 32;
    unsigned long memcap = *(unsigned long *)userData;
    return ps_reload_adjust(memcap, max_work);
}

static int PortscanReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId portscan_swap_config = (tSfPolicyUserContextId)swap_config;
    static unsigned long new_memcap;
    unsigned long old_memcap = ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config))->memcap;
    new_memcap = ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config))->memcap;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if ((portscan_swap_config == NULL) || (((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config)) == NULL) ||
        (portscan_config == NULL) || (((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config)) == NULL))
    {
        return 0;
    }

    if (old_memcap != new_memcap)
    {
#ifdef REG_TEST
        if (REG_TEST_FLAG_PORTSCAN_RELOAD & getRegTestFlags())
        {
            printf("portscan memcap old conf : %lu new conf : %lu\n",old_memcap,new_memcap);
        }
#endif
        /* If memcap is less than  hash overhead bytes, restart is needed */
        if( new_memcap > ps_hash_overhead_bytes())
        {
            ReloadAdjustRegister(sc, "PortscanReload", policy_id, &PortscanReloadAdjust, &new_memcap, NULL);
        }
        else
        {
            ErrorMessage("Portscan Reload: New memcap %lu s lower than  minimum memory needed for hash table %u, and it requires a restart.\n", new_memcap, ps_hash_overhead_bytes());
            return -1;
        }
    }
    return 0;
}

static void * PortscanReloadSwap(struct _SnortConfig *sc, void  *swap_config)
{
    tSfPolicyUserContextId portscan_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = portscan_config;
    bool log_file_swap = false;

    if (portscan_swap_config == NULL)
        return NULL;

    if ((((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config))->logfile != NULL) &&
        (((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config))->logfile != NULL))
    {
        if (strcasecmp(((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config))->logfile,
                       ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config))->logfile) != 0)
        {
            log_file_swap = true;
        }
    }
    else if (((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config))->logfile != ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config))->logfile)
    {
        log_file_swap = true;
    }

    if(log_file_swap)
    {
        char *new_logfile = ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_swap_config))->logfile;
#ifdef REG_TEST
        char *old_logfile = ((PortscanConfig *)sfPolicyUserDataGetDefault(portscan_config))->logfile;
        if (REG_TEST_FLAG_PORTSCAN_RELOAD & getRegTestFlags())
        {
            printf("portscan Logfile  old: %s , new: %s \n", old_logfile?old_logfile:"NULL", new_logfile?new_logfile:"NULL");
        }
#endif
        if(g_logfile)
        {
            fclose(g_logfile);
            g_logfile = NULL;
        }
        if(new_logfile)
        {
            g_logfile = fopen(new_logfile, "a");
            if (g_logfile == NULL)
            {
                FatalError("Portscan log file '%s' could not be opened: %s.\n",
                        new_logfile, strerror(errno));
            }
        }
    }

    portscan_config = portscan_swap_config;
    return (void *)old_config;
}

static void PortscanReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    PortscanFreeConfigs((tSfPolicyUserContextId)data);
}
#endif

