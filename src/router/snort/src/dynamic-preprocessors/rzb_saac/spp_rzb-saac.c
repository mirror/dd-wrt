/*
** Copyright (C) 2005-2009 Sourcefire, Inc.
** Copyright (C) 1998-2005 Martin Roesch <roesch@sourcefire.com>
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
** Foundation, Inc., 59 Temple Place - Suite 330, RZBston, MA 02111-1307, USA.
*/

/* $Id$ */
/* Snort Preprocessor Plugin Source File RZB */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "preprocids.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preproc_lib.h"
#include "sf_dynamic_preprocessor.h"
#include "debug.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#define CONF_SEPARATORS " \t\n\r"
#define RZB_CONF "rzb_conf"

#define PP_SAAC 6868

#include <rzb_collection_api.h>
#include "rzb_smtp-collector.h"
#include "rzb_http-server.h"
#include "rzb_http-client.h"

#define RZB_COLLECT_DISP_GID 3535
#define RZB_COLLECT_DISP_SID 3535
#define RZB_COLLECT_DISP_MESG "Bad file found"

void * dlHandle = NULL;  // For the API library

static void RZBCleanExit(int, void *);
static void RZBProcess(void *, void *);

/* list of function prototypes for this preprocessor */
static void RZBInit(char *);

#ifdef SNORT_RELOAD
static void RZBReload(char *);
static void * RZBReloadSwap(void);
static void RZBReloadSwapFree(void *);
#endif

extern char *maxToken;

void __attribute__((constructor)) detect_init() {

   printf("Razorback SaaC Initializing.\n");

   init_HTTP_PCREs();
}

void __attribute__((destructor)) detect_fini() {
   printf("Razorback SaaC shutting down\n");
}

static void RZBCleanExit(int signal, void *unused) {
    rzb_collection.finiRZB(10);
}

#ifdef SNORT_RELOAD
static void RZBReload(char *args) {
    printf("Razorback SaaC RZBReload() not implemented\n");
}

static void * RZBReloadSwap(void) {
   printf("Razorback SaaC RZBReloadSwap() not implemented\n");
   return NULL;
}

static void RZBReloadSwapFree(void *data) {
   printf("Razorback SaaC RZBReloadSwapFree() not implemented\n");
}
#endif


void RZBProcess(void *p, void *context)
{
   SFSnortPacket *sp = (SFSnortPacket *)p;

   if(!sp->ip4_header || sp->ip4_header->proto != IPPROTO_TCP || !sp->tcp_header)
   {
      /* Not for me, return */
      return;
   }

   // Only rebuilt packets from server
   if (sp->src_port == 80 && !(sp->flags & FLAG_REBUILT_STREAM) && sp->payload_size != 0)
   {
      ProcessFromServer(sp);
      return;
   }

   // No rebuilt packets to server, and only packets with data
   if(sp->dst_port == 80 && !(sp->flags & FLAG_REBUILT_STREAM) && sp->payload_size != 0)
   {
      ProcessFromClient(sp);
      return;
   }

   if(sp->dst_port == 25 && (sp->flags & FLAG_REBUILT_STREAM) && sp->payload_size != 0)
   {
      smtpdumpereval(sp);
      return;
   }

   return;
}

static int functionsRegistered = 0;

static void RZBInit(char *args)
{
    if ((args == NULL) || (strlen(args) == 0))
    {
        DynamicPreprocessorFatalMessage("%s(%d) No arguments to RZB SaaC configuration.\n", *_dpd.config_file, *_dpd.config_line);
    }

    if (!functionsRegistered)
    {
        char *pcToken;

        pcToken = strtok(args, CONF_SEPARATORS);
        if (!pcToken)
        {
            DynamicPreprocessorFatalMessage("%s(%d)strtok returned NULL when it should not.\n", __FILE__, __LINE__);
        }
        if (strcmp(RZB_CONF, pcToken) == 0)
        {
            pcToken = strtok(NULL, CONF_SEPARATORS);
            if (!pcToken)
            {
                DynamicPreprocessorFatalMessage("%s(%d)strtok returned NULL when it should not.\n", __FILE__, __LINE__);
            }
            rzb_collection.initRZB(pcToken);
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) Invalid arguments to RZB SaaC configuration.\n", *_dpd.config_file, *_dpd.config_line);
        }

        _dpd.addPreprocExit(RZBCleanExit, NULL, PRIORITY_LAST, PP_SAAC);
        _dpd.addPreproc(RZBProcess, PRIORITY_TUNNEL, PP_SAAC, PROTO_BIT__TCP);
        functionsRegistered = 1;
    }
    else
    {
        DynamicPreprocessorFatalMessage("%s(%d) More than one RZB SaaC configuration.\n", *_dpd.config_file, *_dpd.config_line);
    }
}

void SetupRZB(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("rzb", RZBInit);
#else
    _dpd.registerPreproc("rzb", RZBInit, RZBReload, RZBReloadSwap, RZBReloadSwapFree);
#endif
}

