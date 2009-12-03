/*
 * ixp400th.h - ixp400 Access Library thunking layer for the
 *              Linux Kernel GPL-only API
 *
 * Copyright (c) 2006 Barnabas Kalman <barnik@sednet.hu>
 *
 * This file is released under the GPLv2
 *
 */
		
#include <linux/platform_device.h>

extern int  ixp_platform_device_register(struct platform_device *);
extern void ixp_platform_device_unregister(struct platform_device *);
