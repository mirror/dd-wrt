/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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
*/
/* $Id$ */

/* spo_log_null
 *
 * Purpose:
 *
 * This module is a NULL placeholder for people that want to turn off
 * logging for whatever reason.  Please note that logging is separate from
 * alerting, they are completely separate output facilities within Snort.
 *
 * Arguments:
 *
 * None.
 *
 * Effect:
 *
 * None.
 *
 * Comments:
 *
 */

#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "spo_log_null.h"
#include "decode.h"
#include "event.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "snort_debug.h"

#include "snort.h"

/* list of function prototypes for this output plugin */
static void LogNullInit(struct _SnortConfig *, char *);
static void LogNull(Packet *, const char *, void *, Event *);
static void LogNullCleanExitFunc(int, void *);

void LogNullSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("log_null", OUTPUT_TYPE_FLAG__LOG, LogNullInit);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Output plugin: LogNull is setup...\n"););
}


static void LogNullInit(struct _SnortConfig *sc, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Output: LogNull Initialized\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, LogNull, OUTPUT_TYPE__LOG, NULL);
    AddFuncToCleanExitList(LogNullCleanExitFunc, NULL);
}



static void LogNull(Packet *p, const char *msg, void *arg, Event *event)
{
    return;
}


static void LogNullCleanExitFunc(int signal, void *arg)
{
    return;
}

