/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * yportenv.h: Portable services used by yaffs. This is done to allow
 * simple migration from kernel space into app space for testing.
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 *
 * $Id: yportenv.h,v 1.6 2005/07/31 00:26:57 charles Exp $
 *
 */
 
#ifndef __YPORTENV_H__
#define __YPORTENV_H__


#if defined CONFIG_YAFFS_WINCE

#include "ywinceenv.h"

#elif  defined __KERNEL__



// Linux kernel
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>

#define YCHAR char
#define YUCHAR unsigned char
#define _Y(x)     x
#define yaffs_strcpy(a,b)    strcpy(a,b)
#define yaffs_strncpy(a,b,c) strncpy(a,b,c)
#define yaffs_strlen(s)	     strlen(s)
#define yaffs_sprintf	     sprintf
#define yaffs_toupper(a)     toupper(a)

#define Y_INLINE inline

#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"

//#define YPRINTF(x) printk x
#define YMALLOC(x) kmalloc(x,GFP_KERNEL)
#define YFREE(x)   kfree(x)

#define YAFFS_ROOT_MODE				0666
#define YAFFS_LOSTNFOUND_MODE		0666

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0))
#define Y_CURRENT_TIME CURRENT_TIME.tv_sec
#define Y_TIME_CONVERT(x) (x).tv_sec
#else
#define Y_CURRENT_TIME CURRENT_TIME
#define Y_TIME_CONVERT(x) (x)
#endif

#define yaffs_SumCompare(x,y) ((x) == (y))
#define yaffs_strcmp(a,b) strcmp(a,b)

#define TENDSTR "\n"
#define TSTR(x) KERN_WARNING x
#define TOUT(p) printk p


#elif defined CONFIG_YAFFS_DIRECT

// Direct interface
#include "ydirectenv.h"

#elif defined CONFIG_YAFFS_UTIL

// Stuff for YAFFS utilities

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "devextras.h"

#define YMALLOC(x) malloc(x)
#define YFREE(x)   free(x)


#define YCHAR char
#define YUCHAR unsigned char
#define _Y(x)     x
#define yaffs_strcpy(a,b)    strcpy(a,b)
#define yaffs_strncpy(a,b,c) strncpy(a,b,c)
#define yaffs_strlen(s)	     strlen(s)
#define yaffs_sprintf	     sprintf
#define yaffs_toupper(a)     toupper(a)

#define Y_INLINE inline

//#define YINFO(s) YPRINTF(( __FILE__ " %d %s\n",__LINE__,s))
//#define YALERT(s) YINFO(s)


#define TENDSTR "\n"
#define TSTR(x) x
#define TOUT(p) printf p


#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"
//#define YPRINTF(x) printf x


#define YAFFS_ROOT_MODE				0666
#define YAFFS_LOSTNFOUND_MODE		0666

#define yaffs_SumCompare(x,y) ((x) == (y))
#define yaffs_strcmp(a,b) strcmp(a,b)

#else
// Should have specified a configuration type
#error Unknown configuration

#endif 


extern unsigned yaffs_traceMask;

#define YAFFS_TRACE_ERROR		0x00000001
#define YAFFS_TRACE_OS			0x00000002
#define YAFFS_TRACE_ALLOCATE		0x00000004
#define YAFFS_TRACE_SCAN		0x00000008
#define YAFFS_TRACE_BAD_BLOCKS		0x00000010
#define YAFFS_TRACE_ERASE		0x00000020
#define YAFFS_TRACE_GC			0x00000040
#define YAFFS_TRACE_WRITE		0x00000080
#define YAFFS_TRACE_TRACING		0x00000100
#define YAFFS_TRACE_DELETION		0x00000200
#define YAFFS_TRACE_BUFFERS		0x00000400
#define YAFFS_TRACE_NANDACCESS		0x00000800
#define YAFFS_TRACE_GC_DETAIL		0x00001000
#define YAFFS_TRACE_SCAN_DEBUG		0x00002000
#define YAFFS_TRACE_MTD			0x00004000
#define YAFFS_TRACE_ALWAYS		0x40000000
#define YAFFS_TRACE_BUG			0x80000000

#define T(mask,p) do{ if((mask) & (yaffs_traceMask | YAFFS_TRACE_ERROR)) TOUT(p);} while(0) 


#ifndef CONFIG_YAFFS_WINCE
#define YBUG() T(YAFFS_TRACE_BUG,(TSTR("==>> yaffs bug: " __FILE__ " %d" TENDSTR),__LINE__))
#endif

#endif


