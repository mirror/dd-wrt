/*
 * Copyright (c) 2012-2013, 2015-2016 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 */

#define DEBUG_LEVEL HYFI_MC_DEBUG_LEVEL

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "mc_netfilter2.h"
#include "mc_netlink2.h"
#include "mc_snooping2.h"
#include "mc_forward2.h"

int __init mc_init(void)
{
    if (mc_netlink_init())
        goto err0;

    if (mc_netfilter_init())
        goto err1;

    printk("QCA Hy-Fi multicast installation successfully\n");
	return 0;
err1:
    mc_netlink_exit();
err0:
    printk("QCA Hy-Fi multicast installation failed\n");
    return -1;
}

void __exit mc_exit(void)
{
    mc_netlink_exit();
    mc_netfilter_exit();
    mc_snooping_exit();

    printk( "QCA Hy-Fi multicast uninstalled\n" );
}

