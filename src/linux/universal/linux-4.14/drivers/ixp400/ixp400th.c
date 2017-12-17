/*
 * ixp400th.h - ixp400 Access Library thunking layer for the
 *              Linux Kernel GPL-only API
 *
 * Copyright (c) 2006 Barnabas Kalman <barnik@sednet.hu>
 *
 * This file is released under the GPLv2
 *
 */
	   
#include "ixp400th.h"

MODULE_DESCRIPTION("IXP400 Access Library Linux API thunking layer");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenIXP team");
	   
extern int  ixp_platform_device_register(struct platform_device *);
extern void ixp_platform_device_unregister(struct platform_device *);
	   
EXPORT_SYMBOL(ixp_platform_device_register);
int ixp_platform_device_register(struct platform_device * pdev)
{
	return platform_device_register(pdev);
}

EXPORT_SYMBOL(ixp_platform_device_unregister);
void ixp_platform_device_unregister(struct platform_device * pdev)
{
	platform_device_unregister(pdev);
}


static int __init ixp400th_init(void)
{
	return 0;
}

void __exit ixp400th_exit(void)
{
}

module_init(ixp400th_init);
module_exit(ixp400th_exit);
