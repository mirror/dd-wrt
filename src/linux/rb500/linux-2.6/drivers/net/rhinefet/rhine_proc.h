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
 * File: rhine_proc.h
 *
 * Purpose: The header file for proc entries.
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#ifndef __RHINE_PROC_H__
#define __RHINE_PROC_H__
//#include "rhine.h"
#include "osdep.h"
#define RHINE_PROC_READ 0x0001
#define RHINE_PROC_WRITE    0x0002
#define RHINE_PROC_DIR      0x0004
#define RHINE_PROC_EOT      0xFFFF
#define RHINE_PROC_FILE     (RHINE_PROC_READ|RHINE_PROC_WRITE)

struct __rhine_info;
struct __rhine_proc_entry;

typedef int (*RHINE_PROC_READ_FUNC) (struct __rhine_proc_entry*, char* buf);
typedef int (*RHINE_PROC_WRITE_FUNC) (struct __rhine_proc_entry*, const char* buf, unsigned long len);

typedef struct __rhine_proc_entry {
    char                                name[128];
    int                                 type;
    RHINE_PROC_READ_FUNC                read_proc;
    RHINE_PROC_WRITE_FUNC               write_proc;
    int                                 data;
    const struct __rhine_proc_entry*    childs;
    __u8                                byRevId;
    struct proc_dir_entry*              pOsEntry;
    struct proc_dir_entry*              pOsParent;
    struct __rhine_info*                pInfo;
    const struct __rhine_proc_entry*    siblings;
} RHINE_PROC_ENTRY, *PRHINE_PROC_ENTRY;



BOOL    rhine_init_proc_fs(struct __rhine_info* pInfo);
BOOL    rhine_create_proc_entry(struct __rhine_info* pInfo);

void    rhine_free_proc_fs(struct __rhine_info* pInfo);
void    rhine_free_proc_entry(struct __rhine_info* pInfo);

#endif // __RHINE_PROC_H__
