/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software may be redistributed and/or modified under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 *
 * File: rhine_cfg.h
 *
 * Purpose: The general and basic info header file.
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#ifndef __RHINE_CONFIG_H__
#define __RHINE_CONFIG_H__

#include <linux/config.h>



#define VID_TABLE_SIZE      64
#define MCAST_TABLE_SIZE    64
#define MCAM_SIZE           32
#define VCAM_SIZE           32
//#define TX_QUEUE_NO         8

#define RHINE_NAME          "rhinefet"

#ifndef MAJOR_VERSION
#define MAJOR_VERSION       5
#endif

#ifndef MINOR_VERSION
#define MINOR_VERSION       04
#endif

#ifndef RHINE_VERSION
#define RHINE_VERSION       "5.04"
#endif

#define PKT_BUF_SZ          1540

#define MALLOC(x,y)         kmalloc((x),(y))
#define FREE(x)             kfree((x))
#define MAX_UINTS           8
#define OPTION_DEFAULT      { [0 ... MAX_UINTS-1] = -1}


#endif // __RHINE_CONFIG_H__
