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

#ifndef __PORTSCAN_H__
#define __PORTSCAN_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#ifndef WIN32
    #include <sys/time.h>
#endif /* !WIN32 */

#include "ipobj.h"

#include "ipv6_port.h"
#include "sfPolicy.h"

#define PS_OPEN_PORTS 8

typedef struct _PortscanConfig
{
    int disabled;
    unsigned long memcap;
    int detect_scans;
    int detect_scan_type;
    int sense_level;
    int proto_cnt;
    int include_midstream;
    int print_tracker;
    char *logfile;
    IPSET *ignore_scanners;
    IPSET *ignore_scanned;
    IPSET *watch_ip;

} PortscanConfig;

typedef struct s_PS_PROTO
{
    short          connection_count;
    short          priority_count;
    short          u_ip_count;
    short          u_port_count;

    unsigned short high_p;
    unsigned short low_p;
    unsigned short u_ports;

    sfaddr_t           high_ip;
    sfaddr_t           low_ip;
    sfaddr_t           u_ips;

    unsigned short open_ports[PS_OPEN_PORTS];
    unsigned char  open_ports_cnt;

    struct timeval event_time;
    unsigned int   event_ref;

    unsigned char  alerts;

    time_t         window;

} PS_PROTO;

typedef struct s_PS_TRACKER
{
    int priority_node;
    int protocol;
    PS_PROTO proto;

} PS_TRACKER;

typedef struct s_PS_PKT
{
    void *pkt;
    int proto;
    int reverse_pkt;
    PS_TRACKER *scanner;
    PS_TRACKER *scanned;

} PS_PKT;

#define PS_PROTO_NONE        0x00
#define PS_PROTO_TCP         0x01
#define PS_PROTO_UDP         0x02
#define PS_PROTO_ICMP        0x04
#define PS_PROTO_IP          0x08
#define PS_PROTO_ALL         0x0f

#define PS_PROTO_OPEN_PORT   0x80

#define PS_TYPE_PORTSCAN     0x01
#define PS_TYPE_PORTSWEEP    0x02
#define PS_TYPE_DECOYSCAN    0x04
#define PS_TYPE_DISTPORTSCAN 0x08
#define PS_TYPE_ALL          0x0f

#define PS_SENSE_HIGH        1
#define PS_SENSE_MEDIUM      2
#define PS_SENSE_LOW         3

#define PS_ALERT_ONE_TO_ONE                1
#define PS_ALERT_ONE_TO_ONE_DECOY          2
#define PS_ALERT_PORTSWEEP                 3
#define PS_ALERT_DISTRIBUTED               4
#define PS_ALERT_ONE_TO_ONE_FILTERED       5
#define PS_ALERT_ONE_TO_ONE_DECOY_FILTERED 6
#define PS_ALERT_DISTRIBUTED_FILTERED      7
#define PS_ALERT_PORTSWEEP_FILTERED        8
#define PS_ALERT_OPEN_PORT                 9

#define PS_ALERT_GENERATED                 255

int ps_init(struct _SnortConfig *, PortscanConfig *, int, int, int, IPSET *, IPSET *, IPSET *, unsigned long);
void ps_cleanup(void);
void ps_reset(void);

int  ps_detect(PS_PKT *p);
void ps_tracker_print(PS_TRACKER *tracker);

int ps_get_protocols(struct _SnortConfig *sc, tSfPolicyId policyId);
void ps_init_hash(unsigned long);
#ifdef SNORT_RELOAD
bool ps_reload_adjust(unsigned long memcap, unsigned max_work);
unsigned int ps_hash_overhead_bytes();
#endif

#endif

