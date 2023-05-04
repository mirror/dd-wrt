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

/* spo_log_tcpdump
 *
 * Purpose:
 *
 * This plugin generates tcpdump formatted binary log files
 *
 * Arguments:
 *
 * filename of the output log (default: snort.log)
 *
 * Effect:
 *
 * Packet logs are written (quickly) to a tcpdump formatted output
 * file
 *
 * Comments:
 *
 * First logger...
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pcap.h>

#include "spo_log_tcpdump.h"
#include "decode.h"
#include "event.h"
#include "mstring.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "snort.h"
#include "sfbpf_dlt.h"

/* For the traversal of reassembled packets */
#include "stream_api.h"
#define M_BYTES (1024*1024)

#define DEFAULT_FILE  "snort.log"
#define DEFAULT_LIMIT (128*M_BYTES)

/*
 * <pcap file> ::= <pcap file hdr> [<pcap pkt hdr> <packet>]*
 * on 64 bit systems, some fields in the <pcap * hdr> are 8 bytes
 * but still stored on disk as 4 bytes.
 * eg: (sizeof(*pkth) = 24) > (dumped size = 16)
 * so we use PCAP_*_HDR_SZ defines in lieu of sizeof().
 */

#define PCAP_FILE_HDR_SZ (24)
#define PCAP_PKT_HDR_SZ  (16)

typedef struct _LogTcpdumpData
{
    char *filename;
    pcap_dumper_t *dumpd;
    time_t lastTime;
    size_t size;
    size_t limit;
    char logdir[STD_BUF];

} LogTcpdumpData;

/* list of function prototypes for this preprocessor */
static void LogTcpdumpInit(struct _SnortConfig *, char *);
static LogTcpdumpData *ParseTcpdumpArgs(char *);
static void LogTcpdump(Packet *, const char *, void *, Event *);
static void TcpdumpInitLogFileFinalize(struct _SnortConfig *sc, int unused, void *arg);
static void TcpdumpInitLogFile(LogTcpdumpData *, int);
static void TcpdumpRollLogFile(LogTcpdumpData*);
static void SpoLogTcpdumpCleanExitFunc(int, void *);
static void LogTcpdumpSingle(Packet *, const char *, void *, Event *);
static void LogTcpdumpStream(Packet *, const char *, void *, Event *);
//static void DirectLogTcpdump(DAQ_PktHdr_t *, uint8_t *);

/* If you need to instantiate the plugin's data structure, do it here */
static LogTcpdumpData *log_tcpdump_ptr;

/*
 * Function: SetupLogTcpdump()
 *
 * Purpose: Registers the output plugin keyword and initialization
 *          function into the output plugin list.  This is the function that
 *          gets called from InitOutputPlugins() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void LogTcpdumpSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("log_tcpdump", OUTPUT_TYPE_FLAG__LOG, LogTcpdumpInit);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: Log-Tcpdump is setup...\n"););
}


/*
 * Function: LogTcpdumpInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void LogTcpdumpInit(struct _SnortConfig *sc, char *args)
{
    LogTcpdumpData *data;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output: Log-Tcpdump Initialized\n"););

    /* parse the argument list from the rules file */
    data = ParseTcpdumpArgs(args);
    log_tcpdump_ptr = data;

    //TcpdumpInitLogFile(data);
    AddFuncToPostConfigList(sc, TcpdumpInitLogFileFinalize, data);

    snort_conf->log_tcpdump = 1;

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, LogTcpdump, OUTPUT_TYPE__LOG, data);
    AddFuncToCleanExitList(SpoLogTcpdumpCleanExitFunc, data);
}

/*
 * Function: ParseTcpdumpArgs(char *)
 *
 * Purpose: Process positional args, if any.  Syntax is:
 * output log_tcpdump: [<logpath> [<limit>]]
 * limit ::= <number>('G'|'M'|K')
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 */
static LogTcpdumpData *ParseTcpdumpArgs(char *args)
{
    char **toks;
    int num_toks;
    LogTcpdumpData *data;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "ParseTcpdumpArgs: %s\n", args););
    data = (LogTcpdumpData *) SnortAlloc(sizeof(LogTcpdumpData));

    if ( !data )
    {
        FatalError("log_tcpdump: unable to allocate memory!\n");
    }
    data->filename = NULL;
    data->limit = DEFAULT_LIMIT;

    if ( !args ) args = "";
    toks = mSplit((char*)args, " \t", 0, &num_toks, '\\');

    for (i = 0; i < num_toks; i++)
    {
        const char* tok = toks[i];
        char *end;

        switch (i)
        {
            case 0:
                data->filename = SnortStrdup(tok);
                break;

            case 1:
                data->limit = strtol(tok, &end, 10);

                if ( tok == end )
                    FatalError("log_tcpdump error in %s(%i): %s\n",
                        file_name, file_line, tok);

                if ( end && toupper(*end) == 'G' )
                    data->limit <<= 30; /* GB */

                else if ( end && toupper(*end) == 'M' )
                    data->limit <<= 20; /* MB */

                else if ( end && toupper(*end) == 'K' )
                    data->limit <<= 10; /* KB */
                break;

            case 2:
                FatalError("log_tcpdump: error in %s(%i): %s\n",
                    file_name, file_line, tok);
                break;
        }
    }
    mSplitFree(&toks, num_toks);
    if ( !data->filename ) data->filename = SnortStrdup(DEFAULT_FILE);
    if (ScPcapLogFile() != NULL)
    {
        free(data->filename);
        data->filename = SnortStrdup(ScPcapLogFile());
    }

    DEBUG_WRAP(DebugMessage(
        DEBUG_INIT, "log_tcpdump: '%s' %ld\n", data->filename, data->limit
    ););
    return data;
}

/*
 * Function: PreprocFunction(Packet *)
 *
 * Purpose: Perform the preprocessor's intended function.  This can be
 *          simple (statistics collection) or complex (IP defragmentation)
 *          as you like.  Try not to destroy the performance of the whole
 *          system by trying to do too much....
 *
 * Arguments: p => pointer to the current packet data struct
 *
 * Returns: void function
 */
static void LogTcpdump(Packet *p, const char *msg, void *arg, Event *event)
{

    if(p)
    {
        if(p->packet_flags & PKT_REBUILT_STREAM)
        {
            LogTcpdumpStream(p, msg, arg, event);
        }
        else
        {
            LogTcpdumpSingle(p, msg, arg, event);
        }
    }
}

static inline size_t SizeOf (const DAQ_PktHdr_t *pkth)
{
    return PCAP_PKT_HDR_SZ + pkth->caplen;
}

static int SizeOfCallback(DAQ_PktHdr_t *pkth,
                             uint8_t *packet_data, void *userdata)
{
    size_t* pSize = (size_t*)userdata;
    (*pSize) += SizeOf(pkth);

    return 0;
}

static void LogTcpdumpSingle(Packet *p, const char *msg, void *arg, Event *event)
{
    LogTcpdumpData *data = (LogTcpdumpData *)arg;
    size_t dumpSize = SizeOf(p->pkth);

    if ( data->size + dumpSize > data->limit )
        TcpdumpRollLogFile(data);

    pcap_dump((u_char *)data->dumpd,(struct pcap_pkthdr*)p->pkth,p->pkt);
    data->size += dumpSize;

    if (!ScLineBufferedLogging())
    {
#ifdef WIN32
        fflush( NULL );  /* flush all open output streams */
#else
        /* we happen to know that pcap_dumper_t* is really just a FILE* */
        fflush( (FILE*) data->dumpd );
#endif
    }
}

static int LogTcpdumpStreamCallback(DAQ_PktHdr_t *pkth,
                             uint8_t *packet_data, void *userdata)
{
    LogTcpdumpData *data = (LogTcpdumpData *)userdata;

    pcap_dump((u_char*)data->dumpd,
              (struct pcap_pkthdr*)pkth,
              (u_char*)packet_data);

    return 0;
}

static void LogTcpdumpStream(Packet *p, const char *msg, void *arg, Event *event)
{
    LogTcpdumpData *data = (LogTcpdumpData *)arg;
    size_t dumpSize = 0;

    if (stream_api)
        stream_api->traverse_reassembled(p, SizeOfCallback, &dumpSize);

    if ( data->size + dumpSize > data->limit )
        TcpdumpRollLogFile(data);

    if (stream_api)
        stream_api->traverse_reassembled(p, LogTcpdumpStreamCallback, data);

    data->size += dumpSize;

    if (!ScLineBufferedLogging())
    {
#ifdef WIN32
        fflush( NULL );  /* flush all open output streams */
#else
        /* we happen to know that pcap_dumper_t* is really just a FILE* */
        fflush( (FILE*) data->dumpd );
#endif
    }
}

static void TcpdumpInitLogFileFinalize(struct _SnortConfig *sc, int unused, void *arg)
{
    TcpdumpInitLogFile((LogTcpdumpData *)arg, ScNoOutputTimestampNewConf(sc));
}

/*
 * Function: TcpdumpInitLogFile()
 *
 * Purpose: Initialize the tcpdump log file header
 *
 * Arguments: data => pointer to the plugin's reference data struct
 *
 * Returns: void function
 */
static void TcpdumpInitLogFile(LogTcpdumpData *data, int nostamps)
{
    int value;
    data->lastTime = time(NULL);

    if (nostamps)
    {
        if(data->filename[0] == '/')
            value = SnortSnprintf(data->logdir, STD_BUF, "%s", data->filename);
        else
            value = SnortSnprintf(data->logdir, STD_BUF, "%s/%s", snort_conf->log_dir,
                                  data->filename);
    }
    else
    {
        if(data->filename[0] == '/')
            value = SnortSnprintf(data->logdir, STD_BUF, "%s.%u", data->filename,
                                  (uint32_t)data->lastTime);
        else
            value = SnortSnprintf(data->logdir, STD_BUF, "%s/%s.%u", snort_conf->log_dir,
                                  data->filename, (uint32_t)data->lastTime);
    }

    if(value != SNORT_SNPRINTF_SUCCESS)
        FatalError("log file logging path and file name are too long\n");

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "Opening %s\n", data->logdir););

    if (!ScTestMode())
    {
        pcap_t* pcap;
        int dlt = DAQ_GetBaseProtocol();

        // convert these flavors of raw to the generic
        // for compatibility with libpcap 1.0.0
        if ( dlt == DLT_IPV4 || dlt == DLT_IPV6 )
            dlt = DLT_RAW;

        pcap = pcap_open_dead(dlt, DAQ_GetSnapLen());
        data->dumpd = pcap ? pcap_dump_open(pcap, data->logdir) : NULL;

        if(data->dumpd == NULL)
        {
            FatalError("log_tcpdump: Failed to open log file \"%s\": %s\n",
                       data->logdir, strerror(errno));
        }
        pcap_close(pcap);
    }

    data->size = PCAP_FILE_HDR_SZ;
}

static void TcpdumpRollLogFile(LogTcpdumpData* data)
{
    time_t now = time(NULL);

    /* don't roll over any sooner than resolution
     * of filename discriminator
     */
    if ( now <= data->lastTime ) return;

    /* close the output file */
    if( data->dumpd != NULL )
    {
        pcap_dump_close(data->dumpd);
        data->dumpd = NULL;
        data->size = 0;
    }

    /* Have to add stamps now to distinguish files */
    TcpdumpInitLogFile(data, 0);
}

/*
 * Function: SpoLogTcpdumpCleanExitFunc()
 *
 * Purpose: Cleanup at exit time
 *
 * Arguments: signal => signal that caused this event
 *            arg => data ptr to reference this plugin's data
 *
 * Returns: void function
 */
static void SpoLogTcpdumpCleanup(int signal, void *arg, const char* msg)
{
    /* cast the arg pointer to the proper type */
    LogTcpdumpData *data = (LogTcpdumpData *) arg;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"%s\n", msg););

    /* close the output file */
    if( data->dumpd != NULL )
    {
        pcap_dump_close(data->dumpd);
        data->dumpd = NULL;
    }

    /*
     * if we haven't written any data, dump the output file so there aren't
     * fragments all over the disk
     */
    if(!ScTestMode() && *data->logdir && !pc.log_pkts && !pc.total_alert_pkts)
    {
        int ret;

        ret = unlink(data->logdir);

        if (ret != 0)
        {
            ErrorMessage("Could not remove tcpdump output file %s: %s\n",
                         data->logdir, strerror(errno));
        }
    }

    if (data->filename)
    {
        free (data->filename);
    }

    memset(data, 0, sizeof(LogTcpdumpData));
    free(data);
}

static void SpoLogTcpdumpCleanExitFunc(int signal, void *arg)
{
    SpoLogTcpdumpCleanup(signal, arg, "SpoLogTcpdumpCleanExitFunc");
}

void LogTcpdumpReset(void)
{
    TcpdumpRollLogFile(log_tcpdump_ptr);
}

#if 0
/* Not currently used */
void DirectLogTcpdump(DAQ_PktHdr_t *ph, uint8_t *pkt)
{
    size_t dumpSize = SizeOf(ph);

    if ( log_tcpdump_ptr->size + dumpSize > log_tcpdump_ptr->limit )
        TcpdumpRollLogFile(log_tcpdump_ptr);

    pc.log_pkts++;
    pcap_dump((u_char *)log_tcpdump_ptr->dumpd, ph, pkt);

    log_tcpdump_ptr->size += dumpSize;
}
#endif

