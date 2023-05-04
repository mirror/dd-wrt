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
**  @file       portscan.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      Detect portscans
**
**  NOTES
**    - Marc Norton and Jeremy Hewlett were involved in the requirements and
**      design of this portscan detection engine.
**    - Thanks to Judy Novak for her suggestion to log open ports
**      on hosts that are portscanned.  This idea makes portscan a lot more
**      useful for analysts.
**
**  The philosophy of portscan detection that we use is based on a generic
**  network attack methodology: reconnaissance, network service enumeration,
**  and service exploitation.
**
**  The reconnaissance phase determines what types of network protocols and
**  services that a host supports.  This is the traditional phase where a
**  portscan occurs.  An important requirement of this phase is that an
**  attacker does not already know what protocols and services are supported
**  by the destination host.  If an attacker does know what services are
**  open on the destination host then there is no need for this phase.
**  Because of this requirement, we assume that if an attacker engages in this
**  phase that they do not have prior knowledege to what services are open.
**  So, the attacker will need to query the ports or protocols they are
**  interested in.  Most or at least some of these queries will be negative
**  and take the form of either an invalid response (TCP RSTs, ICMP
**  unreachables) or no response (in which case the host is firewalled or
**  filtered).  We detect portscans from these negative queries.
**
**  The primary goal of this portscan detection engine is to catch nmap and
**  variant scanners.  The engine tracks connection attempts on TCP, UDP,
**  ICMP, and IP Protocols.  If there is a valid response, the connection
**  is marked as valid.  If there is no response or a invalid response
**  (TCP RST), then we track these attempts separately, so we know the
**  number of invalid responses and the number of connection attempts that
**  generated no response.  These two values differentiate between a
**  normal scan and a filtered scan.
**
**  We detect four different scan types, and each scan type has its own
**  negative query characteristics.  This is how we determine what type
**  of scan we are seeing.  The different scans are:
**
**    - Portscan
**    - Decoy Portscan
**    - Distributed Portscan
**    - Portsweep
**
**  Portscan:  A portscan is a basic one host to one host scan where
**  multiple ports are scanned on the destination host.  We detect these
**  scans by looking for a low number of hosts that contacted the
**  destination host and a high number of unique ports and a high number
**  of invalid responses or connections.
**
**  Distributed Portscan:  A distributed portscan occurs when many hosts
**  connect to a single destination host and multiple ports are scanned
**  on the destination host.  We detect these scans by looking for a high
**  number of hosts that contacted the destination host and a high number
**  of unique ports with a high number of invalid responses or connections.
**
**  Decoy Portscan:  A decoy portscan is a variation on a distributed
**  portscan, the difference being that a decoy portscan connects to a
**  single port multiple times.  This shows up in the unqiue port count that
**  is tracked.  There's still many hosts connecting to the destination host.
**
**  Portsweep:  A portsweep is a basic one host to many host scan where
**  one to a few ports are scanned on each host.  We detect these scans by
**  looking at src hosts for a high number of contacted hosts and a low
**  number of unique ports with a high number of invalid responses or
**  connections.
**
**  Each of these scans can also be detected as a filtered portscan, or a
**  portscan where there wasn't invalid responses and the responses have
**  been firewalled in some way.
**
*/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif /* !WIN32 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort.h"
#include "decode.h"
#include "portscan.h"
#include "packet_time.h"
#include "sfxhash.h"
#include "ipobj.h"
#include "session_common.h"
#include "session_api.h"
#include "stream_api.h"
#include "sfPolicyData.h"
#include "sfPolicyUserData.h"
#include "event_wrapper.h"

#ifdef REG_TEST
#include "reg_test.h"
#endif


# define CLEARED &cleared

typedef struct s_PS_HASH_KEY
{
    int protocol;
    struct in6_addr scanner;
    struct in6_addr scanned;
    tSfPolicyId      policyId;
} PS_HASH_KEY;

typedef struct s_PS_ALERT_CONF
{
    short connection_count;
    short priority_count;
    short u_ip_count;
    short u_port_count;

} PS_ALERT_CONF;

extern tSfPolicyUserContextId portscan_config;
extern PortscanConfig *portscan_eval_config;

static SFXHASH *portscan_hash = NULL;

/*
**  Scanning configurations.  This is where we configure what the thresholds
**  are for the different types of scans, protocols, and sense levels.  If
**  you want to tweak the sense levels, change the values here.
*/
/*
**  TCP alert configurations
*/
static PS_ALERT_CONF g_tcp_low_ps =       {0,5,25,5};
static PS_ALERT_CONF g_tcp_low_decoy_ps = {0,15,50,30};
static PS_ALERT_CONF g_tcp_low_sweep =    {0,5,5,15};
static PS_ALERT_CONF g_tcp_low_dist_ps =  {0,15,50,15};

static PS_ALERT_CONF g_tcp_med_ps =       {200,10,60,15};
static PS_ALERT_CONF g_tcp_med_decoy_ps = {200,30,120,60};
static PS_ALERT_CONF g_tcp_med_sweep =    {30,7,7,10};
static PS_ALERT_CONF g_tcp_med_dist_ps =  {200,30,120,30};

static PS_ALERT_CONF g_tcp_hi_ps =        {200,5,100,10};
static PS_ALERT_CONF g_tcp_hi_decoy_ps =  {200,7,200,60};
static PS_ALERT_CONF g_tcp_hi_sweep =     {30,3,3,10};
static PS_ALERT_CONF g_tcp_hi_dist_ps =   {200,5,200,10};

/*
**  UDP alert configurations
*/
static PS_ALERT_CONF g_udp_low_ps =       {0,5,25,5};
static PS_ALERT_CONF g_udp_low_decoy_ps = {0,15,50,30};
static PS_ALERT_CONF g_udp_low_sweep =    {0,5,5,15};
static PS_ALERT_CONF g_udp_low_dist_ps =  {0,15,50,15};

static PS_ALERT_CONF g_udp_med_ps =       {200,10,60,15};
static PS_ALERT_CONF g_udp_med_decoy_ps = {200,30,120,60};
static PS_ALERT_CONF g_udp_med_sweep =    {30,5,5,20};
static PS_ALERT_CONF g_udp_med_dist_ps =  {200,30,120,30};

static PS_ALERT_CONF g_udp_hi_ps =        {200,3,100,10};
static PS_ALERT_CONF g_udp_hi_decoy_ps =  {200,7,200,60};
static PS_ALERT_CONF g_udp_hi_sweep =     {30,3,3,10};
static PS_ALERT_CONF g_udp_hi_dist_ps =   {200,3,200,10};

/*
**  IP Protocol alert configurations
*/
static PS_ALERT_CONF g_ip_low_ps =        {0,10,10,50};
static PS_ALERT_CONF g_ip_low_decoy_ps =  {0,40,50,25};
static PS_ALERT_CONF g_ip_low_sweep =     {0,10,10,10};
static PS_ALERT_CONF g_ip_low_dist_ps =   {0,15,25,50};

static PS_ALERT_CONF g_ip_med_ps =        {200,10,10,50};
static PS_ALERT_CONF g_ip_med_decoy_ps =  {200,40,50,25};
static PS_ALERT_CONF g_ip_med_sweep =     {30,10,10,10};
static PS_ALERT_CONF g_ip_med_dist_ps =   {200,15,25,50};

static PS_ALERT_CONF g_ip_hi_ps =         {200,3,3,10};
static PS_ALERT_CONF g_ip_hi_decoy_ps =   {200,7,15,5};
static PS_ALERT_CONF g_ip_hi_sweep =      {30,3,3,7};
static PS_ALERT_CONF g_ip_hi_dist_ps =    {200,3,11,10};

/*
**  ICMP alert configurations
*/
static PS_ALERT_CONF g_icmp_low_sweep =   {0,5,5,5};
static PS_ALERT_CONF g_icmp_med_sweep =   {20,5,5,5};
static PS_ALERT_CONF g_icmp_hi_sweep =    {10,3,3,5};

static int ps_get_proto(PS_PKT *, int *);

/*
**  NAME
**    ps_tracker_free::
*/
/**
**  This function is passed into the hash algorithm, so that
**  we only reuse nodes that aren't priority nodes.  We have to make
**  sure that we only track so many priority nodes, otherwise we could
**  have all priority nodes and not be able to allocate more.
*/
static int ps_tracker_free(void *key, void *data)
{
    PS_TRACKER *tracker;

    if(!key || !data)
        return 0;

    tracker = (PS_TRACKER *)data;
    if(!tracker->priority_node)
        return 0;

    /*
    **  Cycle through the protos to see if it's past the time.
    **  We only get here if we ARE a priority node.
    */
    if(tracker->proto.window >= packet_time())
        return 1;

    return 0;
}

/*
**  NAME
**    ps_init::
*/
/*
**  Initialize the portscan infrastructure.  We check to make sure that
**  we have enough memory to support at least 100 nodes.
**
**  @return int
**
**  @retval -2 memcap is too low
*/
int ps_init(struct _SnortConfig *sc, PortscanConfig *config, int detect_scans, int detect_scan_type,
            int sense_level, IPSET *scanner, IPSET *scanned, IPSET *watch, unsigned long memcap)
{
    if (getParserPolicy(sc) != getDefaultPolicy())
    {
        /**checks valid for non-default policy only. Default is allowed to specify
         * just memcap.
         */
        if (!(detect_scans & PS_PROTO_ALL))
            return -1;

        if(!(detect_scan_type & PS_TYPE_ALL))
            return -1;

        if(sense_level < 1 || sense_level > 3)
            return -1;

    }
    else
    {
        /**memcap from non-default is ignored, so check is valid for default policy
         * only
         */
        if(memcap <= 0 || (unsigned)memcap < (sizeof(PS_TRACKER) * 100))
            return -2;
    }

    config->memcap           = memcap;
    config->detect_scans     = detect_scans;
    config->detect_scan_type = detect_scan_type;
    config->sense_level      = sense_level;
    config->ignore_scanners  = scanner;
    config->ignore_scanned   = scanned;
    config->watch_ip         = watch;

    return 0;
}

/*
**  NAME
**    ps_cleanup::
*/
/**
**  Cleanup the portscan infrastructure.
*/
void ps_cleanup(void)
{
    if (portscan_hash != NULL)
    {
        sfxhash_delete(portscan_hash);
        portscan_hash = NULL;
    }
}

void ps_init_hash(unsigned long memcap)
{
    int rows = memcap/(sizeof(PS_HASH_KEY) + sizeof(PS_TRACKER));

    portscan_hash = sfxhash_new(rows, sizeof(PS_HASH_KEY), sizeof(PS_TRACKER),
                                memcap, 1, ps_tracker_free, NULL, 1);

    if (portscan_hash == NULL)
        FatalError("Failed to initialize portscan hash table.\n");
}

/*
**  NAME
**    ps_reset::
*/
/**
**  Reset the portscan infrastructure.
*/
void ps_reset(void)
{
    if (portscan_hash != NULL)
        sfxhash_make_empty(portscan_hash);
}

/*
**  NAME
**    ps_ignore_ip::
*/
/**
**  Check scanner and scanned ips to see if we can filter them out.
*/
static int ps_ignore_ip(sfaddr_t* scanner, uint16_t scanner_port,
                        sfaddr_t* scanned, uint16_t scanned_port)
{
    if (portscan_eval_config->ignore_scanners)
    {
        if (ipset_contains(portscan_eval_config->ignore_scanners, scanner, &scanner_port))
            return 1;
    }

    if(portscan_eval_config->ignore_scanned)
    {
        if (ipset_contains(portscan_eval_config->ignore_scanned, scanned, &scanned_port))
            return 1;
    }

    return 0;
}

/*
**  NAME
**    ps_filter_ignore::
*/
/**
**  Check the incoming packet to decide whether portscan detection cares
**  about this packet.  We try to ignore as many packets as possible.
*/
static int ps_filter_ignore(PS_PKT *ps_pkt)
{
    Packet  *p;
    int      reverse_pkt = 0;
    sfaddr_t* scanner;
    sfaddr_t* scanned;

    p = (Packet *)ps_pkt->pkt;

    if(!IPH_IS_VALID(p))
        return 1;

    if(p->tcph)
    {
        if(!(portscan_eval_config->detect_scans & PS_PROTO_TCP))
            return 1;

        /*
        **  This is where we check all of snort's flags for different
        **  TCP session scenarios.  The checks cover:
        **
        **    - dropping packets in established sessions, but not the
        **      TWH packet.
        **    - dropping the SYN/ACK packet from the server on a valid
        **      connection (we'll catch the TWH later if it happens).
        */
        /*
        **  Ignore packets that are already part of an established TCP
        **  stream.
        */
        if(((p->packet_flags & (PKT_STREAM_EST | PKT_STREAM_TWH))
                == PKT_STREAM_EST) && !(p->tcph->th_flags & TH_RST))
        {
            return 1;
        }

        /*
        **  Ignore the server's initial response, unless it's to RST
        **  the connection.
        */
        /*
        if(!(p->tcph->th_flags & TH_RST) &&
           !(p->packet_flags & (PKT_STREAM_EST)) &&
            (p->packet_flags & PKT_FROM_SERVER))
        {
            return 1;
        }
        */
    }
    else if(p->udph)
    {
        if(!(portscan_eval_config->detect_scans & PS_PROTO_UDP))
            return 1;
    }
    else if(p->icmph)
    {
        if(p->icmph->type != ICMP_DEST_UNREACH &&
           !(portscan_eval_config->detect_scans & PS_PROTO_ICMP))
        {
            return 1;
        }
    }
    else
    {
        if(!(portscan_eval_config->detect_scans & PS_PROTO_IP))
            return 1;
    }

    /*
    **  Check if the packet is reversed
    */
    if((p->packet_flags & PKT_FROM_SERVER))
    {
        reverse_pkt = 1;
    }
    else if(p->icmph && p->icmph->type == ICMP_DEST_UNREACH)
    {
        reverse_pkt = 1;
    }
    else if (p->udph && ( p->ssnptr != NULL ) &&
             ( ( SessionControlBlock * ) p->ssnptr )->session_established &&
             session_api && session_api->version >= SESSION_API_VERSION1)
    {
        if (session_api->get_packet_direction(p) & PKT_FROM_SERVER)
            reverse_pkt = 1;
    }

    scanner = GET_SRC_IP(p);
    scanned = GET_DST_IP(p);

    if(reverse_pkt)
    {
        if(ps_ignore_ip(scanned, p->dp, scanner, p->sp))
            return 1;
    }
    else
    {
        if(ps_ignore_ip(scanner, p->sp, scanned, p->dp))
            return 1;
    }

    ps_pkt->reverse_pkt = reverse_pkt;

    if(portscan_eval_config->watch_ip)
    {
        if(ipset_contains(portscan_eval_config->watch_ip, scanner, &(p->sp)))
            return 0;
        if(ipset_contains(portscan_eval_config->watch_ip, scanned, &(p->dp)))
            return 0;

        return 1;
    }
    return 0;
}

/*
**  NAME
**    ps_tracker_init::
*/
/**
**  Right now all we do is memset, but just in case we want to do more
**  initialization has been extracted.
*/
static int ps_tracker_init(PS_TRACKER *tracker)
{
    memset(tracker, 0x00, sizeof(PS_TRACKER));

    return 0;
}

/*
**  NAME
**    ps_tracker_get::
*/
/**
**  Get a tracker node by either finding one or starting a new one.  We may
**  return NULL, in which case we wait till the next packet.
*/
static int ps_tracker_get(PS_TRACKER **ht, PS_HASH_KEY *key)
{
    int iRet;

    *ht = (PS_TRACKER *)sfxhash_find(portscan_hash, (void *)key);
    if(!(*ht))
    {
        iRet = sfxhash_add(portscan_hash, (void *)key, NULL);
        if(iRet == SFXHASH_OK)
        {
            *ht = (PS_TRACKER *)sfxhash_mru(portscan_hash);
            if(!(*ht))
                return -1;

            ps_tracker_init(*ht);
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

static int ps_tracker_lookup(PS_PKT *ps_pkt, PS_TRACKER **scanner,
                             PS_TRACKER **scanned)
{
    PS_HASH_KEY key;
    Packet *p;

    if (ps_pkt->pkt == NULL)
        return -1;

    p = (Packet *)ps_pkt->pkt;

    if (ps_get_proto(ps_pkt, &key.protocol) == -1)
        return -1;

    ps_pkt->proto = key.protocol;
    key.policyId = getNapRuntimePolicy();

    /*
    **  Let's lookup the host that is being scanned, taking into account
    **  the pkt may be reversed.
    */
    if (portscan_eval_config->detect_scan_type &
        (PS_TYPE_PORTSCAN | PS_TYPE_DECOYSCAN | PS_TYPE_DISTPORTSCAN))
    {
        memset(&key.scanner.s6_addr, 0, sizeof(key.scanner.s6_addr));

        if(ps_pkt->reverse_pkt)
            sfaddr_copy_to_raw(&key.scanned, GET_SRC_IP(p));
        else
            sfaddr_copy_to_raw(&key.scanned, GET_DST_IP(p));

        /*
        **  Get the scanned tracker.
        */
        ps_tracker_get(scanned, &key);
    }

    /*
    **  Let's lookup the host that is scanning.
    */
    if(portscan_eval_config->detect_scan_type & PS_TYPE_PORTSWEEP)
    {
        memset(&key.scanned.s6_addr, 0, sizeof(key.scanned.s6_addr));

        if(ps_pkt->reverse_pkt)
            sfaddr_copy_to_raw(&key.scanner, GET_DST_IP(p));
        else
            sfaddr_copy_to_raw(&key.scanner, GET_SRC_IP(p));

        /*
        **  Get the scanner tracker
        */
        ps_tracker_get(scanner, &key);
    }

    if ((*scanner == NULL) && (*scanned == NULL))
        return -1;

    return 0;
}

/*
**  NAME
**    ps_get_proto_index::
*/
/**
**  This logic finds the index to the proto array based on the
**  portscan configuration.  We need special logic because the
**  index of the protocol changes based on the configuration.
*/
static int ps_get_proto(PS_PKT *ps_pkt, int *proto)
{
    Packet *p;

    if(!ps_pkt || !ps_pkt->pkt || !proto)
        return -1;

    p = (Packet *)ps_pkt->pkt;
    *proto = 0;

    if (portscan_eval_config->detect_scans & PS_PROTO_TCP)
    {
        if ((p->tcph != NULL)
                || ((p->icmph != NULL) && (p->icmph->type == ICMP_DEST_UNREACH)
                    && ((p->icmph->code == ICMP_PORT_UNREACH)
                        || (p->icmph->code == ICMP_PKT_FILTERED))
                    && (p->orig_tcph != NULL)))
        {
            *proto = PS_PROTO_TCP;
            return 0;
        }
    }

    if (portscan_eval_config->detect_scans & PS_PROTO_UDP)
    {
        if ((p->udph != NULL)
                || ((p->icmph != NULL) && (p->icmph->type == ICMP_DEST_UNREACH)
                    && ((p->icmph->code == ICMP_PORT_UNREACH)
                        || (p->icmph->code == ICMP_PKT_FILTERED))
                    && (p->orig_udph != NULL)))
        {
            *proto = PS_PROTO_UDP;
            return 0;
        }
    }

    if (portscan_eval_config->detect_scans & PS_PROTO_IP)
    {
        if ((IPH_IS_VALID(p) && (p->icmph == NULL))
                || ((p->icmph != NULL) && (p->icmph->type == ICMP_DEST_UNREACH)
                    && ((p->icmph->code == ICMP_PROT_UNREACH)
                        || (p->icmph->code == ICMP_PKT_FILTERED))))
        {
            *proto = PS_PROTO_IP;
            return 0;
        }
    }

    if (portscan_eval_config->detect_scans & PS_PROTO_ICMP)
    {
        if (p->icmph != NULL)
        {
            *proto = PS_PROTO_ICMP;
            return 0;
        }
    }

    return -1;
}

/*
**  NAME
**    ps_proto_update_window::
*/
/**
**  Update the proto time windows based on the portscan sensitivity
**  level.
*/
static int ps_proto_update_window(PS_PROTO *proto, time_t pkt_time)
{
    time_t interval;

    switch(portscan_eval_config->sense_level)
    {
        case PS_SENSE_LOW:
            //interval = 15;
            interval = 60;
            break;

        case PS_SENSE_MEDIUM:
            //interval = 15;
            interval = 90;
            break;

        case PS_SENSE_HIGH:
            interval = 600;
            break;

        default:
            return -1;
    }

    /*
    **  If we are outside of the window, reset our ps counters.
    */
    if(pkt_time > proto->window)
    {
        memset(proto, 0x00, sizeof(PS_PROTO));

        proto->window = pkt_time + interval;

        return 0;
    }

    return 0;
}

/*
**  NAME
**    ps_proto_update::
*/
/**
**  This function updates the PS_PROTO structure.
**
**  @param PS_PROTO pointer to structure to update
**  @param int      number to increment portscan counter
**  @param u_long   IP address of other host
**  @param u_short  port/ip_proto to track
**  @param time_t   time the packet was received. update windows.
*/
static int ps_proto_update(PS_PROTO *proto, int ps_cnt, int pri_cnt, sfaddr_t* ip,
        u_short port, time_t pkt_time)
{
    if(!proto)
        return 0;

    /*
    **  If the ps_cnt is negative, that means we are just taking off
    **  for valid connection, and we don't want to do anything else,
    **  like update ip/port, etc.
    */
    if(ps_cnt < 0)
    {
        proto->connection_count += ps_cnt;
        if(proto->connection_count < 0)
            proto->connection_count = 0;

        return 0;
    }

    /*
    **  If we are updating a priority cnt, it means we already did the
    **  unique port and IP on the connection packet.
    **
    **  Priority points are only added for invalid response packets.
    */
    if(pri_cnt)
    {
        proto->priority_count += pri_cnt;
        if(proto->priority_count < 0)
            proto->priority_count = 0;

        return 0;
    }

    /*
    **  Do time check first before we update the counters, so if
    **  we need to reset them we do it before we update them.
    */
    if(ps_proto_update_window(proto, pkt_time))
        return -1;

    /*
    **  Update ps counter
    */
    proto->connection_count += ps_cnt;
    if(proto->connection_count < 0)
        proto->connection_count = 0;

    if(!IP_EQUALITY_UNSET(&proto->u_ips, ip))
    {
        proto->u_ip_count++;
        IP_COPY_VALUE(proto->u_ips, ip);
    }

    /* we need to do the IP comparisons in host order */

    if(sfaddr_is_set(&proto->low_ip))
    {
        if(IP_GREATER(&proto->low_ip, ip))
            IP_COPY_VALUE(proto->low_ip, ip);
    }
    else
    {
        IP_COPY_VALUE(proto->low_ip, ip);
    }

    if(sfaddr_is_set(&proto->high_ip))
    {
        if(IP_LESSER(&proto->high_ip, ip))
            IP_COPY_VALUE(proto->high_ip, ip);
    }
    else
    {
        IP_COPY_VALUE(proto->high_ip, ip);
    }

    if(proto->u_ports != port)
    {
        proto->u_port_count++;
        proto->u_ports = port;
    }

    if(proto->low_p)
    {
        if(proto->low_p > port)
            proto->low_p = port;
    }
    else
    {
        proto->low_p = port;
    }

    if(proto->high_p)
    {
        if(proto->high_p < port)
            proto->high_p = port;
    }
    else
    {
        proto->high_p = port;
    }

    return 0;
}

static int ps_update_open_ports(PS_PROTO *proto, unsigned short port)
{
    int iCtr;

    for(iCtr = 0; iCtr < proto->open_ports_cnt; iCtr++)
    {
        if(port == proto->open_ports[iCtr])
            return 0;
    }

    if(iCtr < (PS_OPEN_PORTS - 1))
    {
        proto->open_ports[iCtr] = port;
        proto->open_ports_cnt++;

        if(proto->alerts == PS_ALERT_GENERATED)
        {
            proto->alerts = PS_ALERT_OPEN_PORT;
        }
    }

    return 0;
}

/*
**  NAME
**    ps_tracker_update_tcp::
*/
/**
**  Determine how to update the portscan counter depending on the type
**  of TCP packet we have.
**
**  We are concerned with three types of TCP packets:
**
**    - initiating TCP packets (we don't care about flags)
**    - TCP 3-way handshake packets (we decrement the counter)
**    - TCP reset packets on unestablished streams.
*/
static int ps_tracker_update_tcp(PS_PKT *ps_pkt, PS_TRACKER *scanner,
                                 PS_TRACKER *scanned)
{
    Packet  *p;
    uint32_t session_flags;
    sfaddr_t cleared;
    IP_CLEAR(cleared);

    p = (Packet *)ps_pkt->pkt;

    /*
    **  Handle the initiating packet.
    **
    **  If this what stream4 considers to be a valid initiator, then
    **  we will use the available stream4 information.  Otherwise, we
    **  can just revert to flow and look for initiators and responders.
    **
    **  For Stream, depending on the configuration, there might not
    **  be a session created only based on the SYN packet.  Stream
    **  by default has code that helps deal with SYN flood attacks,
    **  and may simply ignore the SYN.  In this case, we fall through
    **  to the checks for specific TCP header files (SYN, SYN-ACK, RST).
    **
    **  The "midstream" logic below says that, if we include sessions
    **  picked up midstream, then we don't care about the MIDSTREAM flag.
    **  Otherwise, only consider streams not picked up midstream.
    */
    if( ( p->ssnptr != NULL ) &&
        ( ( SessionControlBlock * ) p->ssnptr )->session_established && session_api )
    {
        session_flags = session_api->get_session_flags(p->ssnptr);

        if((session_flags & SSNFLAG_SEEN_CLIENT) &&
           !(session_flags & SSNFLAG_SEEN_SERVER) &&
           (portscan_eval_config->include_midstream || !(session_flags & SSNFLAG_MIDSTREAM)))
        {
            if(scanned)
            {
                ps_proto_update(&scanned->proto,1,0,
                                 GET_SRC_IP(p),p->dp, packet_time());
            }

            if(scanner)
            {
                ps_proto_update(&scanner->proto,1,0,
                                 GET_DST_IP(p),p->dp, packet_time());
            }
        }
        /*
        **  Handle the final packet of the three-way handshake.
        */
        else if(p->packet_flags & PKT_STREAM_TWH)
        {
            if(scanned)
            {
                ps_proto_update(&scanned->proto,-1,0,CLEARED,0,0);
            }

            if(scanner)
            {
                ps_proto_update(&scanner->proto,-1,0,CLEARED,0,0);
            }
        }
        /*
        **  RST packet on unestablished streams
        */
        else if((p->packet_flags & PKT_FROM_SERVER) &&
                (p->tcph && (p->tcph->th_flags & TH_RST)) &&
                (!(p->packet_flags & PKT_STREAM_EST) ||
                (session_flags & SSNFLAG_MIDSTREAM)))
        {
            if(scanned)
            {
                ps_proto_update(&scanned->proto,0,1,CLEARED,0,0);
                scanned->priority_node = 1;
            }

            if(scanner)
            {
                ps_proto_update(&scanner->proto,0,1,CLEARED,0,0);
                scanner->priority_node = 1;
            }
        }
        /*
        **  We only get here on the server's response to the intial
        **  client connection.
        **
        **  That's why we use the sp, because that's the port that is
        **  open.
        */
        else if((p->packet_flags & PKT_FROM_SERVER) &&
                !(p->packet_flags & PKT_STREAM_EST))
        {
            if(scanned)
            {
                ps_update_open_ports(&scanned->proto, p->sp);
            }

            if(scanner)
            {
                if(scanner->proto.alerts == PS_ALERT_GENERATED)
                    scanner->proto.alerts = PS_ALERT_OPEN_PORT;
            }
        }
    }
    /*
    ** Stream didn't create a session on the SYN packet,
    ** so check specifically for SYN here.
    */
    else if (p->tcph && (p->tcph->th_flags == TH_SYN))
    {
        /* No session established, packet only has SYN.  SYN only
        ** packet always from client, so use dp.
        */
        if(scanned)
        {
            ps_proto_update(&scanned->proto,1,0,
                             GET_SRC_IP(p),p->dp, packet_time());
        }

        if(scanner)
        {
            ps_proto_update(&scanner->proto,1,0,
                             GET_DST_IP(p),p->dp, packet_time());
        }
    }
    /*
    ** Stream didn't create a session on the SYN packet,
    ** so check specifically for SYN & ACK here.  Clear based
    ** on the 'completion' of three-way handshake.
    */
    else if(p->tcph && (p->tcph->th_flags == (TH_SYN|TH_ACK)))
    {
        if(scanned)
        {
            ps_proto_update(&scanned->proto,-1,0,CLEARED,0,0);
        }

        if(scanner)
        {
            ps_proto_update(&scanner->proto,-1,0,CLEARED,0,0);
        }
    }
    /*
    ** No session created, clear based on the RST on non
    ** established session.
    */
    else if (p->tcph && (p->tcph->th_flags & TH_RST))
    {
        if(scanned)
        {
            ps_proto_update(&scanned->proto,0,1,CLEARED,0,0);
            scanned->priority_node = 1;
        }

        if(scanner)
        {
            ps_proto_update(&scanner->proto,0,1,CLEARED,0,0);
            scanner->priority_node = 1;
        }
    }
    /*
    **  If we are an icmp unreachable, deal with it here.
    */
    else if(p->icmph)
    {
        if(scanned)
        {
            ps_proto_update(&scanned->proto,0,1,CLEARED,0,0);
            scanned->priority_node = 1;
        }

        if(scanner)
        {
            ps_proto_update(&scanner->proto,0,1,CLEARED,0,0);
            scanner->priority_node = 1;
        }
    }
    return 0;
}

static int ps_tracker_update_ip(PS_PKT *ps_pkt, PS_TRACKER *scanner,
                                PS_TRACKER *scanned)
{
    Packet *p;
    sfaddr_t cleared;
    IP_CLEAR(cleared);

    p = (Packet *)ps_pkt->pkt;

    if(p->iph)
    {
        /*
         * If the packet is of icmp type 3(destination 
         * unreachable), then unset the connection count so
         * that the connection count will not be incremented for
         * the reply packet and set the priority count.
         */
        if(p->icmph && p->icmph->type == ICMP_DEST_UNREACH &&
            p->icmph->code == ICMP_PROT_UNREACH)
        {
            if(scanned)
            {
                ps_proto_update(&scanned->proto,0,1,CLEARED,0,0);
                scanned->priority_node = 1;
            }

            if(scanner)
            {
                ps_proto_update(&scanner->proto,0,1,CLEARED,0,0);
                scanner->priority_node = 1;
            }
        }
        else
        {
            if(scanned)
            {
                ps_proto_update(&scanned->proto,1,0,
                                GET_SRC_IP(p),p->iph->ip_proto, packet_time());
            }

            if(scanner)
            {
                ps_proto_update(&scanner->proto,1,0,
                                GET_DST_IP(p),p->iph->ip_proto, packet_time());
            }
        }
    }
    return 0;
}

static int ps_tracker_update_udp(PS_PKT *ps_pkt, PS_TRACKER *scanner,
                                 PS_TRACKER *scanned)
{
    Packet  *p;
    sfaddr_t    cleared;
    IP_CLEAR(cleared);

    p = (Packet *)ps_pkt->pkt;

    if(p->icmph)
    {
        if(scanned)
        {
            ps_proto_update(&scanned->proto,0,1,CLEARED,0,0);
            scanned->priority_node = 1;
        }

        if(scanner)
        {
            ps_proto_update(&scanner->proto,0,1,CLEARED,0,0);
            scanner->priority_node = 1;
        }
    }
    else if(p->udph)
    {
        if (session_api && (session_api->version >= SESSION_API_VERSION1) &&
            ( p->ssnptr != NULL ) && ( ( SessionControlBlock * ) p->ssnptr )->session_established)
        {
            uint32_t direction = session_api->get_packet_direction(p);

            if (direction == PKT_FROM_CLIENT)
            {
                if(scanned)
                {
                    ps_proto_update(&scanned->proto,1,0,
                                     GET_SRC_IP(p),p->dp, packet_time());
                }

                if(scanner)
                {
                    ps_proto_update(&scanner->proto,1,0,
                                     GET_DST_IP(p),p->dp, packet_time());
                }
            }
            else if (direction == PKT_FROM_SERVER)
            {
                if(scanned)
                    ps_proto_update(&scanned->proto,-1,0,CLEARED,0,0);

                if(scanner)
                    ps_proto_update(&scanner->proto,-1,0,CLEARED,0,0);
            }
        }
    }

    return 0;
}

static int ps_tracker_update_icmp(PS_PKT *ps_pkt, PS_TRACKER *scanner,
                                  PS_TRACKER *scanned)
{
    Packet  *p;
    sfaddr_t cleared;
    IP_CLEAR(cleared);

    p = (Packet *)ps_pkt->pkt;

    if(p->icmph)
    {
        switch(p->icmph->type)
        {
            case ICMP_ECHO:
            case ICMP_TIMESTAMP:
            case ICMP_ADDRESS:
            case ICMP_INFO_REQUEST:

                if(scanner)
                {
                    ps_proto_update(&scanner->proto,1,0,
                                     GET_DST_IP(p), 0, packet_time());
                }

                break;

            case ICMP_DEST_UNREACH:

                if(scanner)
                {
                    ps_proto_update(&scanner->proto,0,1,CLEARED,0,0);
                    scanner->priority_node = 1;
                }

                break;

            default:
                break;
        }
    }

    return 0;
}

/*
**  NAME
**    ps_tracker_update::
*/
/**
**  At this point, we should only be looking at tranport protocols
**  that we want to.  For instance, if we aren't doing UDP portscans
**  then we won't see UDP packets here because they were ignored.
**
**  This is where we evaluate the packet to add/subtract portscan
**  tracker values and prioritize a tracker.  We also update the
**  time windows.
*/
static int ps_tracker_update(PS_PKT *ps_pkt, PS_TRACKER *scanner,
                             PS_TRACKER *scanned)
{
    if(scanner && scanner->proto.alerts)
        scanner->proto.alerts = PS_ALERT_GENERATED;

    if(scanned && scanned->proto.alerts)
        scanned->proto.alerts = PS_ALERT_GENERATED;

    switch (ps_pkt->proto)
    {
        case PS_PROTO_TCP:
            if(ps_tracker_update_tcp(ps_pkt, scanner, scanned))
                return -1;

            break;

        case PS_PROTO_UDP:
            if(ps_tracker_update_udp(ps_pkt, scanner, scanned))
                return -1;

            break;

        case PS_PROTO_ICMP:
            if(ps_tracker_update_icmp(ps_pkt, scanner, scanned))
                return -1;

            break;

        case PS_PROTO_IP:
            if(ps_tracker_update_ip(ps_pkt, scanner, scanned))
                return -1;

            break;

        default:
            return -1;
    }

    return 0;
}

static int ps_get_tcp_rule_action(int alert_type)
{
    int action = 0;
    switch(alert_type)
    {
        case PS_ALERT_ONE_TO_ONE:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_TCP_PORTSCAN, 0, 0, 3, PSNG_TCP_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_TCP_DECOY_PORTSCAN,0,0,3,PSNG_TCP_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP:
           action = GetSnortEventAction(GENERATOR_PSNG,
                   PSNG_TCP_PORTSWEEP, 0, 0, 3, PSNG_TCP_PORTSWEEP_STR);
           break;

        case PS_ALERT_DISTRIBUTED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_TCP_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_TCP_DISTRIBUTED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_TCP_FILTERED_PORTSCAN,0,0,3,
                    PSNG_TCP_FILTERED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_TCP_FILTERED_DECOY_PORTSCAN, 0,0,3,
                    PSNG_TCP_FILTERED_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                   PSNG_TCP_PORTSWEEP_FILTERED,0,0,3,
                   PSNG_TCP_PORTSWEEP_FILTERED_STR);

            break;
        case PS_ALERT_DISTRIBUTED_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_TCP_FILTERED_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_TCP_FILTERED_DISTRIBUTED_PORTSCAN_STR);
            break;

    }
    return action;
}

static int ps_get_udp_rule_action (int alert_type)
{
    int action = 0;;
    switch(alert_type)
    {
        case PS_ALERT_ONE_TO_ONE:
            action = GetSnortEventAction(GENERATOR_PSNG, PSNG_UDP_PORTSCAN, 0, 0, 3,
                    PSNG_UDP_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY:
            action = GetSnortEventAction(GENERATOR_PSNG,PSNG_UDP_DECOY_PORTSCAN,
                        0, 0, 3, PSNG_UDP_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP:
           action = GetSnortEventAction(GENERATOR_PSNG,PSNG_UDP_PORTSWEEP, 0, 0, 3,
                    PSNG_UDP_PORTSWEEP_STR);
            break;

        case PS_ALERT_DISTRIBUTED:
            action = GetSnortEventAction(GENERATOR_PSNG,PSNG_UDP_DISTRIBUTED_PORTSCAN,
                    0, 0, 3, PSNG_UDP_DISTRIBUTED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,PSNG_UDP_FILTERED_PORTSCAN,
                     0, 0, 3, PSNG_UDP_FILTERED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                        PSNG_UDP_FILTERED_DECOY_PORTSCAN,
                        0, 0, 3, PSNG_UDP_FILTERED_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           action = GetSnortEventAction(GENERATOR_PSNG,
                            PSNG_UDP_PORTSWEEP_FILTERED,0, 0, 3,
                            PSNG_UDP_PORTSWEEP_FILTERED_STR);
            break;

        case PS_ALERT_DISTRIBUTED_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_UDP_FILTERED_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_UDP_FILTERED_DISTRIBUTED_PORTSCAN_STR);
            break;

    }
    return action;
}

static int ps_get_ip_rule_action(int alert_type)
{
    int action = 0;
    switch (alert_type)
    {
        case PS_ALERT_ONE_TO_ONE:
            action = GetSnortEventAction(GENERATOR_PSNG, PSNG_IP_PORTSCAN, 0, 0, 3,
                    PSNG_IP_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY:
            action = GetSnortEventAction(GENERATOR_PSNG,PSNG_IP_DECOY_PORTSCAN,
                            0, 0, 3, PSNG_IP_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP:
           action = GetSnortEventAction(GENERATOR_PSNG,PSNG_IP_PORTSWEEP, 0, 0, 3,
                    PSNG_IP_PORTSWEEP_STR);
            break;

        case PS_ALERT_DISTRIBUTED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                        PSNG_IP_DISTRIBUTED_PORTSCAN,
                        0, 0, 3, PSNG_IP_DISTRIBUTED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                        PSNG_IP_FILTERED_PORTSCAN,
                        0, 0, 3, PSNG_IP_FILTERED_PORTSCAN_STR);
            break;

        case PS_ALERT_ONE_TO_ONE_DECOY_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                        PSNG_IP_FILTERED_DECOY_PORTSCAN,
                        0, 0, 3, PSNG_IP_FILTERED_DECOY_PORTSCAN_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           action = GetSnortEventAction(GENERATOR_PSNG,
                        PSNG_IP_PORTSWEEP_FILTERED, 0, 0, 3,
                        PSNG_IP_PORTSWEEP_FILTERED_STR);
            break;

        case PS_ALERT_DISTRIBUTED_FILTERED:
            action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_IP_FILTERED_DISTRIBUTED_PORTSCAN, 0, 0, 3,
                    PSNG_IP_FILTERED_DISTRIBUTED_PORTSCAN_STR);
            break;
    }
    return action;
}

static int ps_get_icmp_rule_action(int alert_type)
{
    int action = 0;
    switch(alert_type)
    {
        case PS_ALERT_PORTSWEEP:
           action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_ICMP_PORTSWEEP, 0, 0, 3,
                    PSNG_ICMP_PORTSWEEP_STR);
            break;

        case PS_ALERT_PORTSWEEP_FILTERED:
           action = GetSnortEventAction(GENERATOR_PSNG,
                    PSNG_ICMP_PORTSWEEP_FILTERED, 0, 0, 3,
                    PSNG_ICMP_PORTSWEEP_FILTERED_STR);
            break;
    }
    return action;
}

static int ps_get_rule_action (int proto, int alert_type)
{
    int action = 0;
    switch (proto)
    {
        case PS_PROTO_TCP:
            action = ps_get_tcp_rule_action(alert_type);
            break;

        case PS_PROTO_UDP:
            action = ps_get_udp_rule_action(alert_type);
            break;

        case PS_PROTO_ICMP:
            action = ps_get_icmp_rule_action(alert_type);
            break;

        case PS_PROTO_IP:
            action = ps_get_ip_rule_action(alert_type);
            break;
    }
    return action;
}

static int ps_alert_one_to_one(PS_PROTO *scanner, PS_PROTO *scanned,
        PS_ALERT_CONF *conf, int proto)
{
    int action;
    if(!conf)
        return -1;

    /*
    **  Let's evaluate the scanned host.
    */

    if(scanned)
    {
        if(scanned->priority_count >= conf->priority_count)
        {
            action = ps_get_rule_action(proto, PS_ALERT_ONE_TO_ONE);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanned->alerts))
            {
                if(scanned->u_ip_count < conf->u_ip_count &&
                    scanned->u_port_count >= conf->u_port_count)
                {
                    if(scanner)
                    {
                        if(scanner->priority_count >= conf->priority_count)
                        {
                            /*
                             **  Now let's check to make sure this is one
                             **  to one
                             */
                            scanned->alerts = PS_ALERT_ONE_TO_ONE;
                            return 0;
                        }
                    }
                    else
                    {
                        /*
                         **  If there is no scanner, then we do the best we can.
                         */
                        scanned->alerts = PS_ALERT_ONE_TO_ONE;
                        return 0;
                    }
                }
            }
        }
        if(scanned->connection_count >= conf->connection_count)
        {
            action = ps_get_rule_action(proto, PS_ALERT_ONE_TO_ONE_FILTERED);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanned->alerts))
            {
                if(conf->connection_count == 0)
                    return 0;

                if(scanned->u_ip_count < conf->u_ip_count &&
                   scanned->u_port_count >= conf->u_port_count)
                {
                    scanned->alerts = PS_ALERT_ONE_TO_ONE_FILTERED;
                    return 0;
                }
            }
        }
    }

    return 0;

}

static int ps_alert_one_to_one_decoy(PS_PROTO *scanner, PS_PROTO *scanned,
        PS_ALERT_CONF *conf, int proto)
{
    int action;
    if(!conf)
        return -1;

    if(scanned)
    {
        if(scanned->priority_count >= conf->priority_count)
        {
            action = ps_get_rule_action(proto, PS_ALERT_ONE_TO_ONE_DECOY);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanned->alerts))
            {
                if(scanned->u_ip_count >= conf->u_ip_count &&
                   scanned->u_port_count >= conf->u_port_count)
                {
                    scanned->alerts = PS_ALERT_ONE_TO_ONE_DECOY;
                    return 0;
                }
            }
        }
        if(scanned->connection_count >= conf->connection_count)
        {
            action = ps_get_rule_action(proto, PS_ALERT_ONE_TO_ONE_DECOY_FILTERED);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanned->alerts))
            {

                if(conf->connection_count == 0)
                    return 0;

                if(scanned->u_ip_count >= conf->u_ip_count &&
                   scanned->u_port_count >= conf->u_port_count)
                {
                    scanned->alerts = PS_ALERT_ONE_TO_ONE_DECOY_FILTERED;
                    return 0;
                }
            }
        }
    }

    return 0;
}

static int ps_alert_many_to_one(PS_PROTO *scanner, PS_PROTO *scanned,
        PS_ALERT_CONF *conf, int proto)
{
    int action;

    if(!conf)
        return -1;

    if(scanned)
    {
        if(scanned->priority_count >= conf->priority_count)
        {
            action = ps_get_rule_action(proto, PS_ALERT_DISTRIBUTED);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanned->alerts))
            {
                if(scanned->u_ip_count >= conf->u_ip_count &&
                   scanned->u_port_count <= conf->u_port_count)
                {
                    scanned->alerts = PS_ALERT_DISTRIBUTED;
                    return 0;
                }
            }
        }
        if(scanned->connection_count >= conf->connection_count)
        {
            if(conf->connection_count == 0)
                return 0;
 
            action = ps_get_rule_action(proto, PS_ALERT_DISTRIBUTED_FILTERED);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanned->alerts))
            {
                if(scanned->u_ip_count >= conf->u_ip_count &&
                   scanned->u_port_count <= conf->u_port_count)
                {
                    scanned->alerts = PS_ALERT_DISTRIBUTED_FILTERED;
                    return 0;
                }
            }
        }
    }

    return 0;
}

static int ps_alert_one_to_many(PS_PROTO *scanner, PS_PROTO *scanned,
        PS_ALERT_CONF *conf, int proto)
{
    int action;

    if(!conf)
        return -1;

    if(scanner)
    {
        if(scanner->priority_count >= conf->priority_count)
        {
            action = ps_get_rule_action(proto, PS_ALERT_PORTSWEEP);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanner->alerts))
            {
                if(scanner->u_ip_count >= conf->u_ip_count &&
                   scanner->u_port_count <= conf->u_port_count)
                {
                    scanner->alerts = PS_ALERT_PORTSWEEP;
                    return 1;
                }
            }
        }
        if(scanner->connection_count >= conf->connection_count)
        {
            if(conf->connection_count == 0)
                return 0;
 
            action = ps_get_rule_action(proto, PS_ALERT_PORTSWEEP);
            if ((action == RULE_TYPE__DROP) ||
                (action == RULE_TYPE__SDROP) ||
                (action == RULE_TYPE__REJECT) ||
                (!scanner->alerts))
            {
                if(scanner->u_ip_count >= conf->u_ip_count &&
                   scanner->u_port_count <= conf->u_port_count)
                {
                    scanner->alerts = PS_ALERT_PORTSWEEP_FILTERED;
                    return 1;
                }
            }
        }
    }

    return 0;
}

static int ps_alert_tcp(PS_PROTO *scanner, PS_PROTO *scanned)
{
    static PS_ALERT_CONF *one_to_one;
    static PS_ALERT_CONF *one_to_one_decoy;
    static PS_ALERT_CONF *one_to_many;
    static PS_ALERT_CONF *many_to_one;

    /*
    ** Set the configurations depending on the sensitivity
    ** level.
    */
    switch(portscan_eval_config->sense_level)
    {
        case PS_SENSE_HIGH:
            one_to_one       = &g_tcp_hi_ps;
            one_to_one_decoy = &g_tcp_hi_decoy_ps;
            one_to_many      = &g_tcp_hi_sweep;
            many_to_one      = &g_tcp_hi_dist_ps;

            break;

        case PS_SENSE_MEDIUM:
            one_to_one       = &g_tcp_med_ps;
            one_to_one_decoy = &g_tcp_med_decoy_ps;
            one_to_many      = &g_tcp_med_sweep;
            many_to_one      = &g_tcp_med_dist_ps;

            break;

        case PS_SENSE_LOW:
            one_to_one       = &g_tcp_low_ps;
            one_to_one_decoy = &g_tcp_low_decoy_ps;
            one_to_many      = &g_tcp_low_sweep;
            many_to_one      = &g_tcp_low_dist_ps;

            break;

        default:
            return -1;
    }

    /*
    **  Do detection on the different portscan types.
    */
    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSCAN) &&
        ps_alert_one_to_one(scanner, scanned, one_to_one, PS_PROTO_TCP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_DECOYSCAN) &&
        ps_alert_one_to_one_decoy(scanner, scanned, one_to_one_decoy, PS_PROTO_TCP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSWEEP) &&
        ps_alert_one_to_many(scanner, scanned, one_to_many, PS_PROTO_TCP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_DISTPORTSCAN) &&
        ps_alert_many_to_one(scanner, scanned, many_to_one, PS_PROTO_TCP))
    {
        return 0;
    }

    return 0;
}

static int ps_alert_ip(PS_PROTO *scanner, PS_PROTO *scanned)
{
    static PS_ALERT_CONF *one_to_one;
    static PS_ALERT_CONF *one_to_one_decoy;
    static PS_ALERT_CONF *one_to_many;
    static PS_ALERT_CONF *many_to_one;

    /*
    ** Set the configurations depending on the sensitivity
    ** level.
    */
    switch(portscan_eval_config->sense_level)
    {
        case PS_SENSE_HIGH:
            one_to_one       = &g_ip_hi_ps;
            one_to_one_decoy = &g_ip_hi_decoy_ps;
            one_to_many      = &g_ip_hi_sweep;
            many_to_one      = &g_ip_hi_dist_ps;

            break;

        case PS_SENSE_MEDIUM:
            one_to_one       = &g_ip_med_ps;
            one_to_one_decoy = &g_ip_med_decoy_ps;
            one_to_many      = &g_ip_med_sweep;
            many_to_one      = &g_ip_med_dist_ps;

            break;

        case PS_SENSE_LOW:
            one_to_one       = &g_ip_low_ps;
            one_to_one_decoy = &g_ip_low_decoy_ps;
            one_to_many      = &g_ip_low_sweep;
            many_to_one      = &g_ip_low_dist_ps;

            break;

        default:
            return -1;
    }

    /*
    **  Do detection on the different portscan types.
    */
    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSCAN) &&
        ps_alert_one_to_one(scanner, scanned, one_to_one, PS_PROTO_IP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_DECOYSCAN) &&
        ps_alert_one_to_one_decoy(scanner, scanned, one_to_one_decoy, PS_PROTO_IP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSWEEP) &&
        ps_alert_one_to_many(scanner, scanned, one_to_many, PS_PROTO_IP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_DISTPORTSCAN) &&
        ps_alert_many_to_one(scanner, scanned, many_to_one, PS_PROTO_IP))
    {
        return 0;
    }

    return 0;
}

static int ps_alert_udp(PS_PROTO *scanner, PS_PROTO *scanned)
{
    static PS_ALERT_CONF *one_to_one;
    static PS_ALERT_CONF *one_to_one_decoy;
    static PS_ALERT_CONF *one_to_many;
    static PS_ALERT_CONF *many_to_one;

    /*
    ** Set the configurations depending on the sensitivity
    ** level.
    */
    switch(portscan_eval_config->sense_level)
    {
        case PS_SENSE_HIGH:
            one_to_one       = &g_udp_hi_ps;
            one_to_one_decoy = &g_udp_hi_decoy_ps;
            one_to_many      = &g_udp_hi_sweep;
            many_to_one      = &g_udp_hi_dist_ps;

            break;

        case PS_SENSE_MEDIUM:
            one_to_one       = &g_udp_med_ps;
            one_to_one_decoy = &g_udp_med_decoy_ps;
            one_to_many      = &g_udp_med_sweep;
            many_to_one      = &g_udp_med_dist_ps;

            break;

        case PS_SENSE_LOW:
            one_to_one       = &g_udp_low_ps;
            one_to_one_decoy = &g_udp_low_decoy_ps;
            one_to_many      = &g_udp_low_sweep;
            many_to_one      = &g_udp_low_dist_ps;

            break;

        default:
            return -1;
    }

    /*
    **  Do detection on the different portscan types.
    */
    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSCAN) &&
        ps_alert_one_to_one(scanner, scanned, one_to_one, PS_PROTO_UDP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_DECOYSCAN) &&
        ps_alert_one_to_one_decoy(scanner, scanned, one_to_one_decoy, PS_PROTO_UDP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSWEEP) &&
        ps_alert_one_to_many(scanner, scanned, one_to_many, PS_PROTO_UDP))
    {
        return 0;
    }

    if((portscan_eval_config->detect_scan_type & PS_TYPE_DISTPORTSCAN) &&
        ps_alert_many_to_one(scanner, scanned, many_to_one, PS_PROTO_UDP))
    {
        return 0;
    }

    return 0;
}

static int ps_alert_icmp(PS_PROTO *scanner, PS_PROTO *scanned)
{
    static PS_ALERT_CONF *one_to_many;

    /*
    ** Set the configurations depending on the sensitivity
    ** level.
    */
    switch(portscan_eval_config->sense_level)
    {
        case PS_SENSE_HIGH:
            one_to_many = &g_icmp_hi_sweep;

            break;

        case PS_SENSE_MEDIUM:
            one_to_many = &g_icmp_med_sweep;

            break;

        case PS_SENSE_LOW:
            one_to_many = &g_icmp_low_sweep;

            break;

        default:
            return -1;
    }

    /*
    **  Do detection on the different portscan types.
    */
    if((portscan_eval_config->detect_scan_type & PS_TYPE_PORTSWEEP) &&
        ps_alert_one_to_many(scanner, scanned, one_to_many, PS_PROTO_ICMP))
    {
        return 0;
    }

    return 0;
}
/*
**  NAME
**    ps_tracker_alert::
*/
/**
**  This function evaluates the scanner and scanned trackers and if
**  applicable, generate an alert or alerts for either of the trackers.
**
**  The following alerts can be generated:
**    - One to One Portscan
**    - One to One Decoy Portscan
**    - One to Many Portsweep
**    - Distributed Portscan (Many to One)
**    - Filtered Portscan?
*/
static int ps_tracker_alert(PS_PKT *ps_pkt, PS_TRACKER *scanner,
        PS_TRACKER *scanned)
{
    if(!ps_pkt)
        return -1;

    switch(ps_pkt->proto)
    {
        case PS_PROTO_TCP:
            ps_alert_tcp((scanner ? &scanner->proto : NULL),
                         (scanned ? &scanned->proto : NULL));
            break;

        case PS_PROTO_UDP:
            ps_alert_udp((scanner ? &scanner->proto : NULL),
                         (scanned ? &scanned->proto : NULL));
            break;

        case PS_PROTO_ICMP:
            ps_alert_icmp((scanner ? &scanner->proto : NULL),
                          (scanned ? &scanned->proto : NULL));
            break;

        case PS_PROTO_IP:
            ps_alert_ip((scanner ? &scanner->proto : NULL),
                        (scanned ? &scanned->proto : NULL));
            break;

        default:
            return -1;
    }

    return 0;
}

/*
**  NAME
**    ps_detect::
*/
/**
**  The design of portscan is as follows:
**
**    - Filter Packet.  Is the packet part of the ignore or watch list?  Is
**      the packet part of an established TCP session (we ignore it)?
**
**    - Tracker Lookup.  We lookup trackers for src and dst if either is in
**      the watch list, or not in the ignore list if there is no watch list.
**      If there is not tracker, we create a new one and keep track, both of
**      the scanned host and the scanning host.
**
**    - Tracker Update.  We update the tracker using the incoming packet.  If
**      the update causes a portscan alert, then we move into the log alert
**      phase.
**
**    - Tracker Evaluate.  Generate an alert from the updated tracker.  We
**      decide whether we are logging a portscan or sweep (based on the
**      scanning or scanned host, we decide which is more relevant).
*/
int ps_detect(PS_PKT *ps_pkt)
{
    PS_TRACKER *scanner = NULL;
    PS_TRACKER *scanned = NULL;
    int check_tcp_rst_other_dir = 1;
    Packet *p;

    if(!ps_pkt || !ps_pkt->pkt)
        return -1;

    if(ps_filter_ignore(ps_pkt))
        return 0;

    p = (Packet *)ps_pkt->pkt;

    do
    {
        if(ps_tracker_lookup(ps_pkt, &scanner, &scanned))
            return 0;

        if(ps_tracker_update(ps_pkt, scanner, scanned))
            return 0;

        if(ps_tracker_alert(ps_pkt, scanner, scanned))
            return 0;

        /* This is added to address the case of no Stream
         * session and a RST packet going back from the Server. */
        if (p->tcph && (p->tcph->th_flags & TH_RST)
            && ( p->ssnptr != NULL ) && !( ( SessionControlBlock * ) p->ssnptr )->session_established
            && stream_api )
        {
            if (ps_pkt->reverse_pkt == 1)
            {
                check_tcp_rst_other_dir = 0;
            }
            else
            {
                ps_pkt->reverse_pkt = 1;
            }
        }
        else
        {
            check_tcp_rst_other_dir = 0;
        }
    } while (check_tcp_rst_other_dir);

    //printf("** alert\n");
    ps_pkt->scanner = scanner;
    ps_pkt->scanned = scanned;

    return 1;
}

#if 0
/* Not currently used */
static void ps_proto_print(PS_PROTO *proto)
{
// XXX-IPv6 debugging

    return;
}

void ps_tracker_print(PS_TRACKER* ps_tracker)
{
    int proto_index = 0;
    tPortScanPolicyConfig *ppConfig = &psPolicyConfig[getCurrentPolicy()];

    if(!ps_tracker)
        return;

    printf("    -- PS_TRACKER --\n");
    printf("    priority_node = %d\n", ps_tracker->priority_node);

    if(portscan_eval_config->detect_scans & PS_PROTO_TCP)
    {
        printf("    ** TCP **\n");
        ps_proto_print(&ps_tracker->proto);
        proto_index++;
    }
    if(portscan_eval_config->detect_scans & PS_PROTO_UDP)
    {
        printf("    ** UDP **\n");
        ps_proto_print(&ps_tracker->proto);
        proto_index++;
    }
    if(portscan_eval_config->detect_scans & PS_PROTO_IP)
    {
        printf("    ** IP **\n");
        ps_proto_print(&ps_tracker->proto);
        proto_index++;
    }
    if(portscan_eval_config->detect_scans & PS_PROTO_ICMP)
    {
        printf("    ** ICMP **\n");
        ps_proto_print(&ps_tracker->proto);
        proto_index++;
    }

    printf("    -- END --\n\n");

    return;
}
#endif

int ps_get_protocols(struct _SnortConfig *sc, tSfPolicyId policyId)
{
    tSfPolicyUserContextId config = portscan_config;
    PortscanConfig *pPolicyConfig = NULL;

#ifdef SNORT_RELOAD
    /* This is called during configuration time so use the swap
     * config if it exists */
    tSfPolicyUserContextId portscan_swap_config;
    portscan_swap_config = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "sfportscan");
    if (portscan_swap_config != NULL)
        config = portscan_swap_config;
#endif
    if( config == NULL)
        return 0;
    pPolicyConfig = (PortscanConfig *)sfPolicyUserDataGet(config, policyId);

    if (pPolicyConfig == NULL)
        return 0;

    /* Disabled in this policy */
    if (pPolicyConfig->disabled == 1)
        return 0;

    return pPolicyConfig->detect_scans;
}

#ifdef SNORT_RELOAD
bool ps_reload_adjust(unsigned long memcap, unsigned max_work)
{
    int ret = sfxhash_change_memcap(portscan_hash, memcap, &max_work);
#ifdef REG_TEST
    if (REG_TEST_FLAG_PORTSCAN_RELOAD & getRegTestFlags() && ret == SFXHASH_OK)
    {
        printf("portscanhash memused:%lu\n",portscan_hash->mc.memused);
        printf("portscanhash memcap:%lu\n",portscan_hash->mc.memcap);
    }
#endif
    return ret == SFXHASH_OK;
}

unsigned int ps_hash_overhead_bytes()
{
    if(portscan_hash)
    {
        return portscan_hash->overhead_bytes;
    }
    return 0;
}
#endif


