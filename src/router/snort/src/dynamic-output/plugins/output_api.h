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

#ifndef _OUTPUT_API_H
#define _OUTPUT_API_H

#include "output_common.h"

typedef enum _DynamicOutputTypeFlag
{
    DYNAMIC_OUTPUT_TYPE_FLAG__ALERT = 0x00000001,
    DYNAMIC_OUTPUT_TYPE_FLAG__LOG   = 0x00000002,
    DYNAMIC_OUTPUT_TYPE_FLAG__ALL   = 0x7fffffff

} DynamicOutputTypeFlag;

struct _output_dict_entry
{
    char *key;
    char *value;
    struct _output_dict_entry *next;
};

typedef struct _output_module
{
    /* The version of the API this module implements.
       This *must* be the first element in the structure. */
    const uint32_t api_major_version;
    const uint32_t api_minor_version;
    /* The version of the OUTPUT module itself - can be completely arbitrary. */
    const uint32_t module_version;
    /* The name of the module (tcpdump, alert_full, unified, etc.) */
    const char *name;
    /* Various flags describing the module and its capabilities (alert, log etc.) */
    const uint32_t type;
    /* The name of the default log file */
    const char *default_file;
    /* load output module*/
    void (*load) (struct _SnortConfig *, char *arg);
    /* Parse the output device configuration --required*/
    int (*parse_args) (void **config, char *arg, const char *default_output_file);
    /* Post configuration*/
    void (*postconfig)(struct _SnortConfig *, int unused, void *data);
    /* Alert function */
    void (*alert_output) (void *packet, char *msg, void *arg, void *event);
    /* Log function */
    void (*log_output) (void *packet, char *msg, void *arg, void *event);
    /* Restart/rotate the device */
    void (*rotate) (struct _SnortConfig *, int signal, void *arg);
    /* Close the device and clean up --required */
    void (*shutdown) (int signal, void *arg);
    void  *next;

} Output_Module_t;

void init_output_module(struct _SnortConfig *, Output_Module_t *, char *);

#define OUTPUT_API_MAJOR_VERSION    0x00020000
#define OUTPUT_API_MINOR_VERSION    0x00000001

#endif /* _OUTPUT_API_H */
