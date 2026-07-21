/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#ifndef BRIDGE_CTL_H
#define BRIDGE_CTL_H

#include <stdbool.h>
#include <net/if.h>
#include <linux/if_ether.h>

typedef struct
{
    int if_index;
    __u8 macaddr[ETH_ALEN];
    char name[IFNAMSIZ];

    bool up;
} sysdep_br_data_t;

typedef struct
{
    int if_index;
    __u8 macaddr[ETH_ALEN];
    char name[IFNAMSIZ];

    bool up;
    int speed, duplex;
} sysdep_if_data_t;

#define GET_PORT_SPEED(port)    ((port)->sysdeps.speed)
#define GET_PORT_DUPLEX(port)   ((port)->sysdeps.duplex)

/* Logging macros for mstp.c - they use system dependent info */
#define ERROR_BRNAME(_br, _fmt, _args...) ERROR("%s " _fmt, \
    _br->sysdeps.name, ##_args)
#define INFO_BRNAME(_br, _fmt, _args...)   INFO("%s " _fmt, \
    _br->sysdeps.name, ##_args)
#define LOG_BRNAME(_br, _fmt, _args...)     LOG("%s " _fmt, \
    _br->sysdeps.name, ##_args)
#define ERROR_PRTNAME(_prt, _fmt, _args...) ERROR("%s:%s " _fmt, \
    _prt->bridge->sysdeps.name, _prt->sysdeps.name, ##_args)
#define INFO_PRTNAME(_prt, _fmt, _args...)   INFO("%s:%s " _fmt, \
    _prt->bridge->sysdeps.name, _prt->sysdeps.name, ##_args)
#define LOG_PRTNAME(_prt, _fmt, _args...)    LOG("%s:%s " _fmt,  \
    _prt->bridge->sysdeps.name, _prt->sysdeps.name, ##_args)
#define ERROR_MSTINAME(_ptp,_fmt,_args...) ERROR("%s:%s:%hu " _fmt, \
    _ptp->port->bridge->sysdeps.name, _ptp->port->sysdeps.name,     \
    __be16_to_cpu(ptp->MSTID), ##_args)
#define INFO_MSTINAME(_ptp,_fmt,_args...)  INFO("%s:%s:%hu " _fmt,  \
    _ptp->port->bridge->sysdeps.name, _ptp->port->sysdeps.name,     \
    __be16_to_cpu(ptp->MSTID), ##_args)
#define LOG_MSTINAME(_ptp,_fmt,_args...)    LOG("%s:%s:%hu " _fmt,  \
    _ptp->port->bridge->sysdeps.name, _ptp->port->sysdeps.name,     \
    __be16_to_cpu(ptp->MSTID), ##_args)
#define SMLOG_MSTINAME(_ptp, _fmt, _args...)                         \
    PRINT(LOG_LEVEL_STATE_MACHINE_TRANSITION, "%s: %s:%s:%hu " _fmt, \
          __PRETTY_FUNCTION__, _ptp->port->bridge->sysdeps.name,     \
         _ptp->port->sysdeps.name, __be16_to_cpu(ptp->MSTID), ##_args)

extern struct rtnl_handle rth_state;

int init_bridge_ops(void);

int bridge_notify(int br_index, int if_index, bool newlink, unsigned flags);

void bridge_bpdu_rcv(int ifindex, const unsigned char *data, int len);

void bridge_one_second(void);

#endif /* BRIDGE_CTL_H */
