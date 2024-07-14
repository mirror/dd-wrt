/*
 * Copyright (c) 2012 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
*/

#ifndef _MC_NETLINK_H_
#define _MC_NETLINK_H_

#include "mc_private2.h"

int __init mc_netlink_init(void);
void mc_netlink_exit(void);
void mc_netlink_event_send(struct mc_struct *mc, u32 event_type, u32 event_len, void *event_data);
void mc_group_notify_one(struct mc_struct *mc, struct mc_ip *pgroup);

#endif


