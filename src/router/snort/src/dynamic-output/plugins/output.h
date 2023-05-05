/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2012-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
** Date: 01-27-2012
** Author: Hui Cao <hcao@sourcefire.com>
*/

#ifndef _OUTPUT_H
#define _OUTPUT_H

#include <stdio.h>
#include "output_common.h"


struct _SnortConfig;

/* Definition of the structures returned by output_get_module_list(). */
typedef struct {
    char *name;         /* Module name */
    uint32_t version;   /* Module version */
    uint32_t type;      /* Module capabilities */
} Output_Module_Info_t;

/* Iterate through each of the output modules */
void * GetNextOutputModule(void *);
const char * GetOutputModuleName(void *);
uint32_t GetOutputModuleVersion(void *);

/* Functions for loading, handling, and unloading output modules. */

/*Load, unload output modules from directory*/
int output_load(const char *dir);
void output_unload(void);

int output_load_module(const char *filename);

int initOutputPlugin(void *);
#endif /* _OUTPUT_H */
