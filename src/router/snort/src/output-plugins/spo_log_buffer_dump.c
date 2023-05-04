/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** Author:   Rahul Burman <rahburma@cisco.com>
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

/* spo_log_buffer_dump
 *
 * Purpose:
 *
 * This module is used to dump the buffers used by preprocessors during
 * packet inspection
 *
 * Arguments: filename to which buffers are to be dumped
 *
 * Effect: None
 *
 * Comments:
 *
 */

#ifdef DUMP_BUFFER

#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "spo_log_buffer_dump.h"
#include "decode.h"
#include "event.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "preprocids.h"
#include "log_text.h"
#include "snort.h"
#include "strvec.h"

#define DEFAULT_FILE  "stdout"
#define LIMIT (128*M_BYTES)
#define LOG_BUFFER    (4*K_BYTES)

static int maxBuffers[MAX_BUFFER_DUMP_FUNC] = { MAX_HTTP_BUFFER_DUMP,
                                                MAX_SMTP_BUFFER_DUMP,
                                                MAX_SIP_BUFFER_DUMP,
                                                MAX_DNP3_BUFFER_DUMP,
                                                MAX_POP_BUFFER_DUMP,
                                                MAX_MODBUS_BUFFER_DUMP,
                                                MAX_SSH_BUFFER_DUMP,
                                                MAX_DNS_BUFFER_DUMP,
                                                MAX_DCERPC2_BUFFER_DUMP,
                                                MAX_FTPTELNET_BUFFER_DUMP,
                                                MAX_IMAP_BUFFER_DUMP,
                                                MAX_SSL_BUFFER_DUMP,
                                                MAX_GTP_BUFFER_DUMP };

typedef struct _SpoLogBufferDumpData
{
    TextLog* log;
} SpoLogBufferDumpData;

/* list of function prototypes for this output plugin */
static void LogBufferDumpInit(struct _SnortConfig *, char *);
static SpoLogBufferDumpData *InitializeLogBufferDumpOutputStream(SnortConfig *);
static void LogBufferDump(Packet *, const char *, void *, Event *);
static void LogBufferDumpCleanExitFunc(int, void *);

void LogBufferDumpSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("log_buffer_dump", OUTPUT_TYPE_FLAG__LOG, LogBufferDumpInit);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Output plugin: LogBufferDump is setup...\n"););
}


static void LogBufferDumpInit(struct _SnortConfig *sc, char *args)
{
    SpoLogBufferDumpData *data;
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Output: LogBufferDump Initialized\n"););

    /* parse the argument list from the rules file */
    data = InitializeLogBufferDumpOutputStream(sc);

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, LogBufferDump, OUTPUT_TYPE__LOG, data);
    AddBDFuncToOutputList(sc, LogBufferDump, OUTPUT_TYPE__LOG, data);
    AddFuncToCleanExitList(LogBufferDumpCleanExitFunc, data);
}


/*
 * Function: InitializeLogBufferDumpOutputStream(SnortConfig *)
 *
 * Purpose: Initialize the output stream to which buffers will be dumped.
 *
 * Arguments: args => SnortConfig
 *
 * Returns: SpoLogBufferDumpData
 */
static SpoLogBufferDumpData *InitializeLogBufferDumpOutputStream(SnortConfig *sc)
{
    SpoLogBufferDumpData *data;
    char* filename = NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "Output: LogBufferDump output stream initialized\n"););
    data = (SpoLogBufferDumpData *)SnortAlloc(sizeof(SpoLogBufferDumpData));

    if ( !data )
    {
        FatalError("log buffer dump: unable to allocate memory!\n");
    }

    const char** dump_file = StringVector_GetVector(sc->buffer_dump_file);

    if (dump_file && dump_file[0])
        filename = SnortStrdup(dump_file[0]);
    else
        filename = SnortStrdup(DEFAULT_FILE);

    data->log = TextLog_Init(filename, LOG_BUFFER, LIMIT);

    if (filename != NULL)
        free(filename);

    return data;
}

static void LogBufferDump(Packet *p, const char *msg, void *arg, Event *event)
{
    // dump the buffers used during packet processing
    dumped_state = true;
    SpoLogBufferDumpData *data = (SpoLogBufferDumpData *)arg;
        TextLog_Puts(data->log, "\n=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n");
    if (ScOutputAppData() && (p->dsize > 0) && PacketWasCooked(p))
    {
      switch ( p->pseudo_type ) {
            case PSEUDO_PKT_SMB_SEG:
                TextLog_Print(data->log, "\n%s", "SMB desegmented packet:");
                break;
            case PSEUDO_PKT_DCE_SEG:
                TextLog_Print(data->log, "\n%s", "DCE/RPC desegmented packet:");
                break;
            case PSEUDO_PKT_DCE_FRAG:
                TextLog_Print(data->log, "\n%s", "DCE/RPC defragmented packet:");
                break;
            case PSEUDO_PKT_SMB_TRANS:
                TextLog_Print(data->log, "\n%s", "SMB Transact reassembled packet:");
                break;
            case PSEUDO_PKT_DCE_RPKT:
                TextLog_Print(data->log, "\n%s", "DCE/RPC reassembled packet:");
                break;
            case PSEUDO_PKT_TCP:
                TextLog_Print(data->log, "\n%s", "Stream Reassembled Packet:\n");
                break;
            case PSEUDO_PKT_IP:
                TextLog_Print(data->log, "\n%s", "Frag reassembled packet:");
                break;
            default:
                // FIXTHIS do we get here for portscan or sdf?
                break;
    }
  }
  else
  {
      TextLog_Print(data->log, "\n%s", "Raw Packet:\n");
  }
    LogTimeStamp(data->log, p);
    if(p && IPH_IS_VALID(p))
    {
       if (ScOutputDataLink())
        {
            Log2ndHeader(data->log, p);
        }

        LogIPHeader(data->log, p);

        if(!p->frag_flag)
        {
            switch(GET_IPH_PROTO(p))
            {
                case IPPROTO_TCP:
                   LogTCPHeader(data->log, p);
                    break;

                case IPPROTO_UDP:
                   LogUDPHeader(data->log, p);
                    break;

                case IPPROTO_ICMP:
                   LogICMPHeader(data->log, p);
                    break;

                default:
                    break;
            }
        }
        LogXrefs(data->log, 1);

        TextLog_Putc(data->log, '\n');
    }
    TextLog_Flush(data->log);

    bool bufferDumpFlag = false;

    if (p->dsize)
    {
      TraceBuffer *bufs;
      int i, j;

      for (i = 0; i < MAX_BUFFER_DUMP_FUNC; i++)
      {
        if ((bdmask & (UINT64_C(1) << i)) && getBuffers[i])
        {
           bufs = getBuffers[i]();
           if (bufs != NULL)
           {
              for (j = 0; j < maxBuffers[i]; j++)
              {
                  if (bufs[j].length != 0)
                  {
                     if (!bufferDumpFlag) // To Print Dumping Buffers string only once
                     {
                        bufferDumpFlag = true;
                        TextLog_Print(data->log, "\nDumping Buffers\n");
                     }

                     if (event != NULL)
                     {
                        TextLog_Print(data->log, "%s [%lu:%lu:%lu]\n",
                        (char *)"Event",
                        (unsigned long) event->sig_generator,
                        (unsigned long) event->sig_id,
                        (unsigned long) event->sig_rev);
                     }
                     LogBuffer(data->log, bufs[j].buf_name, bufs[j].buf_content, bufs[j].length);
                     // free the http buffers after logging
                     if (i == HTTP_BUFFER_DUMP_FUNC)
                        free(bufs[j].buf_content);
                     bufs[j].buf_content = NULL;
                     bufs[j].length = 0;
                  }
              }
           }
        }
      }
    }

    if (!bufferDumpFlag) // No data available in all the files
    {
       TextLog_Print(data->log, "\nNo Buffers to Dump\n");
    }
}

static void LogBufferDumpCleanup(int signal, void *arg, const char* msg)
{
    SpoLogBufferDumpData *data = (SpoLogBufferDumpData *)arg;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "%s\n", msg););

    /* free memory from SpoLogBufferDumpData */
    if ( data->log ) TextLog_Term(data->log);
    free(data);

    /* Checkout for any allocated HTTP buffers not freed yet */
    TraceBuffer *bufs;
    int j;

    if ((bdmask & (UINT64_C(1) << HTTP_BUFFER_DUMP_FUNC)) && getBuffers[HTTP_BUFFER_DUMP_FUNC])
    {
       bufs = getBuffers[HTTP_BUFFER_DUMP_FUNC]();
       if (bufs != NULL)
       {
          for (j = 0; j < maxBuffers[HTTP_BUFFER_DUMP_FUNC]; j++)
          {
             if (bufs[j].buf_content)
             {
                free(bufs[j].buf_content);
                bufs[j].buf_content = NULL;
                bufs[j].length = 0;
             }
          }
       }
    }
}


static void LogBufferDumpCleanExitFunc(int signal, void *arg)
{
    LogBufferDumpCleanup(signal, arg, "LogBufferDumpCleanExit");
}

#endif
