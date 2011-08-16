/*
** Copyright (C) 2005-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */

/* spo_unified 
 * 
 * Purpose:
 *
 * This plugin generates the new unified alert and logging formats
 *
 * Arguments:
 *   
 * filename of the alert and log spools
 *
 * Effect:
 *
 * Packet logs are written (quickly) to a unified output file
 *
 * Comments:
 *
 * The future...
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <errno.h>
#include <time.h>
#include <sfbpf_dlt.h>

#include "spo_unified.h"
#include "decode.h"
#include "rules.h"
#include "treenodes.h"
#include "util.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "debug.h"
#include "mstring.h"
#include "event.h"
#include "generators.h"
#include "bounds.h"
#include "sfdaq.h"

#include "snort.h"
#include "pcap_pkthdr32.h"

/* For the traversal of reassembled packets */
#include "stream_api.h"

#define SNORT_MAGIC     0xa1b2c3d4
#define ALERT_MAGIC     0xDEAD4137  /* alert magic, just accept it */
#define LOG_MAGIC       0xDEAD1080  /* log magic, what's 31337-speak for G? */
#define SNORT_VERSION_MAJOR   1
#define SNORT_VERSION_MINOR   2

/* From fpdetect.c, for logging reassembled packets */
extern uint16_t   event_id;

/* file header for snort unified format log files
 *
 * Identical to pcap file header, used for portability where the libpcap
 * might not be used after the pa_engine code becomes available
 */ 
typedef struct _UnifiedLogFileHeader
{
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t timezone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t linktype;
} UnifiedLogFileHeader;

typedef struct _UnifiedAlertFileHeader
{
    uint32_t magic;
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t timezone;
} UnifiedAlertFileHeader;

/* unified log packet header format 
 *
 * One of these per packet in the log file, the packets are appended in the 
 * file after each UnifiedLog header (extended pcap format) 
 */
typedef struct _UnifiedLog
{
    Event event;
    uint32_t flags;       /* bitmap for interesting flags */
    struct pcap_pkthdr32 pkth;
} UnifiedLog;


/* Unified alert message format
 *
 * One per event notification, all the important data for people to know
 */
typedef struct _UnifiedAlert
{
    Event event;
    struct sf_timeval32 ts;    /* event timestamp */
    uint32_t sip;             /* src ip */
    uint32_t dip;             /* dest ip */
    uint16_t sp;              /* src port */
    uint16_t dp;              /* dest port */
    uint32_t protocol;        /* protocol id */
    uint32_t flags;           /* any other flags (fragmented, etc) */
} UnifiedAlert;

typedef struct _UnifiedIPv6Alert
{
    Event event;
    struct sf_timeval32 ts;        /* event timestamp */
    snort_ip      sip;             /* src ip */
    snort_ip      dip;             /* dest ip */
    uint16_t sp;              /* src port */
    uint16_t dp;              /* dest port */
    uint32_t protocol;        /* protocol id */
    uint32_t flags;           /* any other flags (fragmented, etc) */
} UnifiedIPv6Alert;


/* ----------------External variables -------------------- */
extern OptTreeNode *otn_tmp;

/* ------------------ Data structures --------------------------*/
typedef struct _UnifiedConfig
{
    char *filename;
    FILE *stream;
    unsigned int limit;
    unsigned int current;
    int nostamp;
} UnifiedConfig;

typedef struct _FileHeader
{
    uint32_t magic;
    uint32_t flags;
} FileHeader;

typedef struct _DataHeader
{
    uint32_t type;
    uint32_t length;
} DataHeader;

#define UNIFIED_MAGIC 0x2dac5ceb

#define UNIFIED_TYPE_ALERT          0x1
#define UNIFIED_TYPE_PACKET_ALERT   0x2
#define UNIFIED_TYPE_IPV6_ALERT      0x3

/* -------------------- Local Functions -----------------------*/
static UnifiedConfig *UnifiedParseArgs(char *, char *);
static void UnifiedCleanExit(int, void *);
static void UnifiedRestart(int, void *);
static void UnifiedLogInitFinalize(int, void *);

/* Unified Output functions */
static void UnifiedInit(char *);
static void UnifiedInitFile(UnifiedConfig *);
static void UnifiedRotateFile(UnifiedConfig *);
static void UnifiedLogAlert(Packet *, char *, void *, Event *);
static void UnifiedLogPacketAlert(Packet *, char *, void *, Event *);
static void RealUnifiedLogAlert(Packet *, char *, void *, Event *, 
        DataHeader *);
static void RealUnifiedLogAlert6(Packet *, char *, void *, Event *, 
        DataHeader *);
static void RealUnifiedLogPacketAlert(Packet *, char *, void *, Event *, 
        DataHeader *);
static void RealUnifiedLogStreamAlert(Packet *,char *,void *,Event *,DataHeader *);
static void UnifiedRotateFile(UnifiedConfig *data);

/* Unified Alert functions (deprecated) */
static void UnifiedAlertInit(char *);
static void UnifiedInitAlertFile(UnifiedConfig *);
static void UnifiedAlertRotateFile(UnifiedConfig *data);
static void OldUnifiedLogAlert(Packet *, char *, void *, Event *);


/* Unified Packet Log functions (deprecated) */
static void UnifiedLogInit(char *);
static void UnifiedInitLogFile(UnifiedConfig *);
static void OldUnifiedLogPacketAlert(Packet *, char *, void *, Event *);
static void UnifiedLogRotateFile(UnifiedConfig *data);
static int UnifiedFirstPacketCallback(DAQ_PktHdr_t *pkth,
        uint8_t *packet_data, void *userdata);

/* Used for buffering header and payload of unified records so only one
 * write is necessary. */
static char write_pkt_buffer[sizeof(DataHeader) + 
                             sizeof(UnifiedLog) + IP_MAXPACKET];

/*
 * Function: SetupUnified()
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
void UnifiedSetup(void)
{
    /* link the preprocessor keyword to the init function in 
       the preproc list */
    RegisterOutputPlugin("log_unified", OUTPUT_TYPE_FLAG__LOG, UnifiedLogInit);
    RegisterOutputPlugin("alert_unified", OUTPUT_TYPE_FLAG__ALERT, UnifiedAlertInit);
    RegisterOutputPlugin("unified", OUTPUT_TYPE_FLAG__LOG | OUTPUT_TYPE_FLAG__ALERT, UnifiedInit);
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output plugin: Unified logging/alerting "
                "is setup...\n"););
}

/*
 * Function: UnifiedInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
void UnifiedInit(char *args)
{
    UnifiedConfig *unifiedConfig;

    //DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: Unified Initialized\n"););

    /* parse the argument list from the rules file */
    unifiedConfig = UnifiedParseArgs(args, "snort-unified");

    UnifiedInitFile(unifiedConfig);

    //LogMessage("UnifiedFilename = %s\n", unifiedConfig->filename);
    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(UnifiedLogAlert, OUTPUT_TYPE__ALERT, unifiedConfig);
    AddFuncToOutputList(UnifiedLogPacketAlert, OUTPUT_TYPE__LOG, unifiedConfig);

    AddFuncToCleanExitList(UnifiedCleanExit, unifiedConfig);
    AddFuncToRestartList(UnifiedRestart, unifiedConfig);
}

/*
 * Function: InitOutputFile()
 *
 * Purpose: Initialize the unified ouput file 
 *
 * Arguments: data => pointer to the plugin's reference data struct 
 *
 * Returns: void function
 */
static void UnifiedInitFile(UnifiedConfig *data)
{
    time_t curr_time;      /* place to stick the clock data */
    char logdir[STD_BUF];
    FileHeader hdr;
    int value;

    bzero(logdir, STD_BUF);
    curr_time = time(NULL);

    if(data == NULL)
        FatalError("SpoUnified: Unable to get context data\n");

    if(data->nostamp) 
    {
        if(*(data->filename) == '/')
            value = SnortSnprintf(logdir, STD_BUF, "%s", data->filename);
        else
            value = SnortSnprintf(logdir, STD_BUF, "%s/%s", snort_conf->log_dir,  
                              data->filename);
    }
    else 
    {
        if(*(data->filename) == '/')
            value = SnortSnprintf(logdir, STD_BUF, "%s.%u", data->filename, 
                              (uint32_t)curr_time);
        else
            value = SnortSnprintf(logdir, STD_BUF, "%s/%s.%u", snort_conf->log_dir,  
                              data->filename, (uint32_t)curr_time);
    }

    if(value != SNORT_SNPRINTF_SUCCESS)
        FatalError("SpoUnified: filepath too long\n");

    //printf("Opening %s\n", logdir);

    if((data->stream = fopen(logdir, "wb")) == NULL)
        FatalError("UnifiedInitLogFile(%s): %s\n", logdir, strerror(errno));

    /* write the log file header */
    hdr.magic = UNIFIED_MAGIC;
    hdr.flags = 0;  /* XXX: not used yet */

    if(fwrite((char *)&hdr, sizeof(hdr), 1, data->stream) != 1)
    {
        FatalError("SpoUnified: InitOutputFile(): %s", strerror(errno));
    }

    fflush(data->stream);

    return;
}

void UnifiedRotateFile(UnifiedConfig *data)
{
    fclose(data->stream);
    data->current = 0;
    UnifiedInitFile(data);
}

void UnifiedLogAlert(Packet *p, char *msg, void *arg, Event *event)
{
    DataHeader dHdr;
   
    /* check for a pseudo-packet, we don't want to log those */
    if(IS_IP4(p))
    {
        dHdr.type = UNIFIED_TYPE_ALERT;
        dHdr.length = sizeof(UnifiedAlert);
 
        RealUnifiedLogAlert(p, msg, arg, event, &dHdr);
    }
    else
    {
        dHdr.type = UNIFIED_TYPE_IPV6_ALERT;
        dHdr.length = sizeof(UnifiedIPv6Alert);
 
        RealUnifiedLogAlert6(p, msg, arg, event, &dHdr);
    }
}
  
static int UnifiedFirstPacketCallback(DAQ_PktHdr_t *pkth,
                               uint8_t *packet_data, void *userdata)
{
    UnifiedAlert *alertdata = (UnifiedAlert*)userdata;

    /* loop thru all the packets in the stream */
    if(pkth != NULL )
    {
        alertdata->ts.tv_sec  = (uint32_t)pkth->ts.tv_sec;
        alertdata->ts.tv_usec = (uint32_t)pkth->ts.tv_usec;
    } 

    /* return non-zero so we only do this once */
    return 1;
}

static void RealUnifiedLogAlert(Packet *p, char *msg, void *arg, Event *event, 
        DataHeader *dHdr)
{
    UnifiedConfig *data = (UnifiedConfig *)arg;
    UnifiedAlert alertdata;

    bzero(&alertdata, sizeof(alertdata));

    if(event != NULL)
    {
        alertdata.event.sig_generator = event->sig_generator;
        alertdata.event.sig_id = event->sig_id;
        alertdata.event.sig_rev = event->sig_rev;
        alertdata.event.classification = event->classification;
        alertdata.event.priority = event->priority;
        alertdata.event.event_id = event->event_id;
        alertdata.event.event_reference = event->event_reference;
        alertdata.event.ref_time.tv_sec = event->ref_time.tv_sec;
        alertdata.event.ref_time.tv_usec = event->ref_time.tv_usec;

    }

    if(p)
    {
        alertdata.ts.tv_sec = (uint32_t)p->pkth->ts.tv_sec;
        alertdata.ts.tv_usec = (uint32_t)p->pkth->ts.tv_usec;
       
        if((p->packet_flags & PKT_REBUILT_STREAM) && stream_api)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_LOG, "man:Logging rebuilt stream data.\n");); 

            stream_api->traverse_reassembled(p, UnifiedFirstPacketCallback, &alertdata);

       }

        if(IPH_IS_VALID(p))
        {
            /* everything needs to be written in host order */
            alertdata.sip = ntohl(p->iph->ip_src.s_addr);
            alertdata.dip = ntohl(p->iph->ip_dst.s_addr);
            if(GET_IPH_PROTO(p) == IPPROTO_ICMP)
            {
                if(p->icmph != NULL)
                {
                    alertdata.sp = p->icmph->type;
                    alertdata.dp = p->icmph->code;
                }
            }
            else
            {
                alertdata.sp = p->sp;
                alertdata.dp = p->dp;
            }
            alertdata.protocol = GET_IPH_PROTO(p);
            alertdata.flags = p->packet_flags;
        }
    }
    
    /* backward compatibility stuff */
    if(dHdr == NULL)
    {
        if((data->current + sizeof(UnifiedAlert)) > data->limit)
            UnifiedAlertRotateFile(data);
    }
    else
    {
        if((data->current + sizeof(UnifiedAlert)) > data->limit)
            UnifiedRotateFile(data);
    }

    if(dHdr)
    {
        if(fwrite((char *)dHdr, sizeof(DataHeader), 1, data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));
        data->current += sizeof(DataHeader);
    }
    
    if(fwrite((char *)&alertdata, sizeof(UnifiedAlert), 1, data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));


    fflush(data->stream);
    data->current += sizeof(UnifiedAlert);
}

static void RealUnifiedLogAlert6(Packet *p, char *msg, void *arg, Event *event, 
        DataHeader *dHdr)
{
    UnifiedConfig *data = (UnifiedConfig *)arg;
    UnifiedIPv6Alert alertdata;

    bzero(&alertdata, sizeof(alertdata));

    if(event != NULL)
    {
        alertdata.event.sig_generator = event->sig_generator;
        alertdata.event.sig_id = event->sig_id;
        alertdata.event.sig_rev = event->sig_rev;
        alertdata.event.classification = event->classification;
        alertdata.event.priority = event->priority;
        alertdata.event.event_id = event->event_id;
        alertdata.event.event_reference = event->event_reference;
        alertdata.event.ref_time.tv_sec = event->ref_time.tv_sec;
        alertdata.event.ref_time.tv_usec = event->ref_time.tv_usec;

    }

    if(p)
    {
        alertdata.ts.tv_sec = (uint32_t)p->pkth->ts.tv_sec;
        alertdata.ts.tv_usec = (uint32_t)p->pkth->ts.tv_usec;
       
        if((p->packet_flags & PKT_REBUILT_STREAM) && stream_api)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_LOG, "man:Logging rebuilt stream data.\n");); 

            stream_api->traverse_reassembled(p, UnifiedFirstPacketCallback, &alertdata);

       }

        if(IPH_IS_VALID(p))
        {
            /* everything needs to be written in host order */
            IP_COPY_VALUE(alertdata.sip, GET_SRC_IP(p));
            IP_COPY_VALUE(alertdata.dip, GET_DST_IP(p));
            if(GET_IPH_PROTO(p) == IPPROTO_ICMP)
            {
                if(p->icmph != NULL)
                {
                    alertdata.sp = p->icmph->type;
                    alertdata.dp = p->icmph->code;
                }
            }
            else
            {
                alertdata.sp = p->sp;
                alertdata.dp = p->dp;
            }
            alertdata.protocol = GET_IPH_PROTO(p);
            alertdata.flags = p->packet_flags;
        }
    }
    
    /* backward compatibility stuff */
    if(dHdr == NULL)
    {
        if((data->current + sizeof(UnifiedIPv6Alert)) > data->limit)
            UnifiedAlertRotateFile(data);
    }
    else
    {
        if((data->current + sizeof(UnifiedIPv6Alert)) > data->limit)
            UnifiedRotateFile(data);
    }

    if(dHdr)
    {
        if(fwrite((char *)dHdr, sizeof(DataHeader), 1, data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));
        data->current += sizeof(DataHeader);
    }
    
    if(fwrite((char *)&alertdata, sizeof(UnifiedIPv6Alert), 1, data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));


    fflush(data->stream);
    data->current += sizeof(UnifiedIPv6Alert);
}


void UnifiedLogPacketAlert(Packet *p, char *msg, void *arg, Event *event)
{
    DataHeader dHdr;
    dHdr.type = UNIFIED_TYPE_PACKET_ALERT;
    dHdr.length = sizeof(UnifiedLog);
    
    if(p->packet_flags & PKT_REBUILT_STREAM)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, 
                    "[*] Reassembled packet, dumping stream packets\n"););
        RealUnifiedLogStreamAlert(p, msg, arg, event, &dHdr);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "[*] Logging unified packets...\n"););
        RealUnifiedLogPacketAlert(p, msg, arg, event, &dHdr);
    }

}


void RealUnifiedLogPacketAlert(Packet *p, char *msg, void *arg, Event *event,
        DataHeader *dHdr)
{
    UnifiedLog logheader;
    UnifiedConfig *data = (UnifiedConfig *)arg;
    int offset = 0;

    if(event != NULL)
    {
        logheader.event.sig_generator = event->sig_generator;
        logheader.event.sig_id = event->sig_id;
        logheader.event.sig_rev = event->sig_rev;
        logheader.event.classification = event->classification;
        logheader.event.priority = event->priority;
        logheader.event.event_id = event->event_id;
        logheader.event.event_reference = event->event_reference;
        logheader.event.ref_time.tv_sec = event->ref_time.tv_sec;
        logheader.event.ref_time.tv_usec = event->ref_time.tv_usec;

        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "------------\n");
        DebugMessage(DEBUG_LOG, "gen: %u\n", logheader.event.sig_generator);
        DebugMessage(DEBUG_LOG, "sid: %u\n", logheader.event.sig_id);
        DebugMessage(DEBUG_LOG, "rev: %u\n", logheader.event.sig_rev);
        DebugMessage(DEBUG_LOG, "cls: %u\n", logheader.event.classification);
        DebugMessage(DEBUG_LOG, "pri: %u\n", logheader.event.priority);
        DebugMessage(DEBUG_LOG, "eid: %u\n", logheader.event.event_id);
        DebugMessage(DEBUG_LOG, "erf: %u\n", logheader.event.event_reference);
        DebugMessage(DEBUG_LOG, "sec: %lu\n", logheader.event.ref_time.tv_sec);
        DebugMessage(DEBUG_LOG, "usc: %lu\n", logheader.event.ref_time.tv_usec););
    }

    if(p)
    {
        logheader.flags = p->packet_flags;

        /* 
         * this will have to be fixed when we transition to the pa_engine
         * code (p->pkth is libpcap specific)
         */ 
        logheader.pkth.ts.tv_sec = (uint32_t)p->pkth->ts.tv_sec;
        logheader.pkth.ts.tv_usec = (uint32_t)p->pkth->ts.tv_usec;
        logheader.pkth.caplen = p->pkth->caplen;
        logheader.pkth.len = p->pkth->pktlen;

    }
    else
    {
        logheader.flags = 0;
        logheader.pkth.ts.tv_sec = 0;
        logheader.pkth.ts.tv_usec = 0;
        logheader.pkth.caplen = 0;
        logheader.pkth.len = 0;
    }
    
    /* backward compatibility stuff */
    if(dHdr == NULL)
    {
        if((data->current + sizeof(UnifiedLog) + logheader.pkth.caplen) > 
                data->limit)
            UnifiedLogRotateFile(data);
    }
    else
    {   
        if((data->current + sizeof(UnifiedLog) + sizeof(DataHeader) 
                    + logheader.pkth.caplen) > data->limit)
            UnifiedRotateFile(data);
    }
    if(dHdr)
    {
        SafeMemcpy(write_pkt_buffer, dHdr, sizeof(DataHeader),
                write_pkt_buffer, write_pkt_buffer + 
                sizeof(DataHeader) + sizeof(UnifiedLog) + IP_MAXPACKET);

        data->current += sizeof(DataHeader);
        offset = sizeof(DataHeader);

    }
        
    SafeMemcpy(write_pkt_buffer + offset, &logheader, sizeof(UnifiedLog),
                write_pkt_buffer, write_pkt_buffer + 
                sizeof(DataHeader) + sizeof(UnifiedLog) + IP_MAXPACKET);

    data->current += sizeof(UnifiedLog);
    offset += sizeof(UnifiedLog);
    
    if(p)
    {
        SafeMemcpy(write_pkt_buffer + offset, p->pkt, p->pkth->caplen,
                write_pkt_buffer, write_pkt_buffer + 
                sizeof(DataHeader) + sizeof(UnifiedLog) + IP_MAXPACKET);

        if(fwrite(write_pkt_buffer, offset + p->pkth->caplen, 
                  1, data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));

        data->current += p->pkth->caplen;
    }
    else 
    {
        if(fwrite(write_pkt_buffer, sizeof(DataHeader) + 
               sizeof(UnifiedLog), 1, data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));
    }

    fflush(data->stream);
}

typedef struct _UnifiedLogStreamCallbackData
{
    UnifiedLog *logheader;
    UnifiedConfig *data;
    DataHeader *dHdr;
    Event *event;
    int once;
} UnifiedLogStreamCallbackData;

/**
 * Callback for the Stream reassembler to log packets
 *
 */
int UnifiedLogStreamCallback(DAQ_PktHdr_t *pkth,
                             uint8_t *packet_data, void *userdata)
{
    UnifiedLogStreamCallbackData *unifiedData;
    int offset = 0;

    if (!userdata)
        return -1;

    unifiedData = (UnifiedLogStreamCallbackData *)userdata;

    /* copy it's pktheader data into the logheader */
    unifiedData->logheader->pkth.ts.tv_sec = (uint32_t)pkth->ts.tv_sec;
    unifiedData->logheader->pkth.ts.tv_usec = (uint32_t)pkth->ts.tv_usec;
    unifiedData->logheader->pkth.caplen = (uint32_t)pkth->caplen;
    unifiedData->logheader->pkth.len = (uint32_t)pkth->pktlen;

    /* backward compatibility stuff */
    if(unifiedData->dHdr == NULL)
    {
        if((unifiedData->data->current +
            sizeof(UnifiedLog)+
            unifiedData->logheader->pkth.caplen) > 
            unifiedData->data->limit)
        {
            UnifiedLogRotateFile(unifiedData->data);
        }
    }
    else
    {   
        if((unifiedData->data->current + sizeof(UnifiedLog) + sizeof(DataHeader) 
                    + unifiedData->logheader->pkth.caplen) > unifiedData->data->limit)
            UnifiedRotateFile(unifiedData->data);
    }

    if(unifiedData->dHdr)
    {
        SafeMemcpy(write_pkt_buffer, unifiedData->dHdr, sizeof(DataHeader),
                   write_pkt_buffer, write_pkt_buffer +
                   sizeof(DataHeader) + sizeof(UnifiedLog) + IP_MAXPACKET);

        offset = sizeof(DataHeader);

        unifiedData->data->current += sizeof(DataHeader);
    }

    SafeMemcpy(write_pkt_buffer + offset, unifiedData->logheader, 
                sizeof(UnifiedLog), write_pkt_buffer, 
                write_pkt_buffer + sizeof(DataHeader) + 
                sizeof(UnifiedLog) + IP_MAXPACKET);

    offset += sizeof(UnifiedLog);

    unifiedData->data->current += sizeof(UnifiedLog);

    if(packet_data)
    {
        SafeMemcpy(write_pkt_buffer + offset, packet_data, 
               offset + unifiedData->logheader->pkth.caplen,
               write_pkt_buffer, write_pkt_buffer + 
               sizeof(DataHeader) + sizeof(UnifiedLog) + IP_MAXPACKET);

        if(fwrite(write_pkt_buffer, offset + unifiedData->logheader->pkth.caplen,
                  1, unifiedData->data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));

        unifiedData->data->current += unifiedData->logheader->pkth.caplen;
    }
    else 
    {
        if(fwrite(write_pkt_buffer, offset,
                  1, unifiedData->data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", strerror(errno));
    }

    /* after the first logged packet modify the event headers */
    if(!unifiedData->once++)
    {
        unifiedData->logheader->event.sig_generator = GENERATOR_TAG;
        unifiedData->logheader->event.sig_id = TAG_LOG_PKT;
        unifiedData->logheader->event.sig_rev = 1;
        unifiedData->logheader->event.classification = 0;
        unifiedData->logheader->event.priority = unifiedData->event->priority;
        /* Note that event_id is now incorrect. 
         * See OldUnifiedLogPacketAlert() for details. */
    }

    return 0;
}


/**
 * Log a set of packets stored in the stream reassembler
 *
 */
static void RealUnifiedLogStreamAlert(Packet *p, char *msg, void *arg, Event *event,
        DataHeader *dHdr)
{
    UnifiedLogStreamCallbackData unifiedData;
    UnifiedLog logheader;
    UnifiedConfig *data = (UnifiedConfig *)arg;
    int once = 0;

    /* setup the event header */
    if(event != NULL)
    {
        logheader.event.sig_generator = event->sig_generator;
        logheader.event.sig_id = event->sig_id;
        logheader.event.sig_rev = event->sig_rev;
        logheader.event.classification = event->classification;
        logheader.event.priority = event->priority;
        logheader.event.event_id = event->event_id;
        logheader.event.event_reference = event->event_reference;
        /* Note that ref_time is probably incorrect.  
         * See OldUnifiedLogPacketAlert() for details. */
        logheader.event.ref_time.tv_sec = event->ref_time.tv_sec;
        logheader.event.ref_time.tv_usec = event->ref_time.tv_usec;

        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "------------\n");
        DebugMessage(DEBUG_LOG, "gen: %u\n", logheader.event.sig_generator);
        DebugMessage(DEBUG_LOG, "sid: %u\n", logheader.event.sig_id);
        DebugMessage(DEBUG_LOG, "rev: %u\n", logheader.event.sig_rev);
        DebugMessage(DEBUG_LOG, "cls: %u\n", logheader.event.classification);
        DebugMessage(DEBUG_LOG, "pri: %u\n", logheader.event.priority);
        DebugMessage(DEBUG_LOG, "eid: %u\n", logheader.event.event_id);
        DebugMessage(DEBUG_LOG, "erf: %u\n", 
               logheader.event.event_reference);
        DebugMessage(DEBUG_LOG, "sec: %lu\n", 
               logheader.event.ref_time.tv_sec);
        DebugMessage(DEBUG_LOG, "usc: %lu\n", 
               logheader.event.ref_time.tv_usec););
    }

    /* queue up the stream for logging */
    if(p && stream_api)
    {
        unifiedData.logheader = &logheader;
        unifiedData.data = data;
        unifiedData.dHdr = dHdr;
        unifiedData.event = event;
        unifiedData.once = once;
        stream_api->traverse_reassembled(p, UnifiedLogStreamCallback, &unifiedData);
    }
    
    fflush(data->stream);
}

/*
 * Function: UnifiedParseArgs(char *)
 *
 * Purpose: Process the preprocessor arguements from the rules file and 
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init 
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
UnifiedConfig *UnifiedParseArgs(char *args, char *default_filename)
{
    UnifiedConfig *tmp;
    int limit = 0;

    tmp = (UnifiedConfig *)calloc(sizeof(UnifiedConfig), sizeof(char));

    if(tmp == NULL)
    {
        FatalError("Unable to allocate Unified Data struct!\n");
    }

    /* This is so the if 'nostamps' option is used on the command line,
     * it will be honored by unified, and only one variable is used. */
    tmp->nostamp = ScNoOutputTimestamp();

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Args: %s\n", args););

    if(args != NULL)
    {
        char **toks;
        int num_toks;
        int i = 0;
        toks = mSplit(args, ",", 31, &num_toks, '\\');
        for(i = 0; i < num_toks; ++i)
        {
            char **stoks;
            int num_stoks;
            char *index = toks[i];
            while(isspace((int)*index))
                ++index;
          
            stoks = mSplit(index, " \t", 2, &num_stoks, 0);
            
            if(strcasecmp("filename", stoks[0]) == 0)
            {
                if(num_stoks > 1 && tmp->filename == NULL)
                    tmp->filename = strdup(stoks[1]);
                else
                    LogMessage("Argument Error in %s(%i): %s\n",
                            file_name, file_line, index);
            }
            else if(strcasecmp("limit", stoks[0]) == 0)
            {
                if(num_stoks > 1 && limit == 0)
                {
                    limit = atoi(stoks[1]);
                }
                else
                {
                    LogMessage("Argument Error in %s(%i): %s\n",
                            file_name, file_line, index);
                }
            }
            else if(strcasecmp("nostamp", stoks[0]) == 0)
            {   
                tmp->nostamp = 1;
            }
            else
            {
                LogMessage("Argument Error in %s(%i): %s\n",
                        file_name, file_line, index);
            }
            
            mSplitFree(&stoks, num_stoks);
        }
        mSplitFree(&toks, num_toks);
    }

    if(tmp->filename == NULL)
        tmp->filename = strdup(default_filename);
    
    //LogMessage("limit == %i\n", limit);

    if(limit <= 0)
    {
        limit = 128;
    }
    if(limit > 512)
    {
        LogMessage("spo_unified %s(%d)=> Lowering limit of %iMB to 512MB\n", file_name, file_line, limit);
        limit = 512;
    }

    /* convert the limit to "MB" */
    tmp->limit = limit << 20;

    return tmp;
}


/*
 * Function: UnifiedCleanExitFunc()
 *
 * Purpose: Cleanup at exit time
 *
 * Arguments: signal => signal that caused this event
 *            arg => data ptr to reference this plugin's data
 *
 * Returns: void function
 */
static void UnifiedCleanExit(int signal, void *arg)
{
    /* cast the arg pointer to the proper type */
    UnifiedConfig *data = (UnifiedConfig *)arg;

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "SpoUnified: CleanExit\n"););

    fclose(data->stream);

    /* free up initialized memory */
    free(data->filename);
    free(data);
}



/*
 * Function: Restart()
 *
 * Purpose: For restarts (SIGHUP usually) clean up structs that need it
 *
 * Arguments: signal => signal that caused this event
 *            arg => data ptr to reference this plugin's data
 *
 * Returns: void function
 */
static void UnifiedRestart(int signal, void *arg)
{
    UnifiedConfig *data = (UnifiedConfig *)arg;

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "SpoUnified: Restart\n"););

    fclose(data->stream);
    free(data->filename);
    free(data);
}



/* Unified Alert functions (deprecated) */
void UnifiedAlertInit(char *args)
{
    UnifiedConfig *data;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: Unified Alert Initialized\n"););

    /* parse the argument list from the rules file */
    data = UnifiedParseArgs(args, "snort-unified.alert");

    UnifiedInitAlertFile(data);


    //LogMessage("UnifiedAlertFilename = %s\n", data->filename);
    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(OldUnifiedLogAlert, OUTPUT_TYPE__ALERT, data);
    AddFuncToCleanExitList(UnifiedCleanExit, data);
    AddFuncToRestartList(UnifiedRestart, data);
}
/*
 * Function: UnifiedInitAlertFile()
 *
 * Purpose: Initialize the unified log alert file
 *
 * Arguments: data => pointer to the plugin's reference data struct 
 *
 * Returns: void function
 */
void UnifiedInitAlertFile(UnifiedConfig *data)
{
    time_t curr_time;      /* place to stick the clock data */
    char logdir[STD_BUF];
    int value;
    UnifiedAlertFileHeader hdr;

    bzero(logdir, STD_BUF);
    curr_time = time(NULL);

    if(data->nostamp) 
    {
        if(data->filename[0] == '/')
            value = SnortSnprintf(logdir, STD_BUF, "%s", data->filename);
        else
            value = SnortSnprintf(logdir, STD_BUF, "%s/%s", snort_conf->log_dir, 
                              data->filename);
    }
    else
    {
        if(data->filename[0] == '/')
            value = SnortSnprintf(logdir, STD_BUF, "%s.%u", data->filename, 
                                  (uint32_t)curr_time);
        else
            value = SnortSnprintf(logdir, STD_BUF, "%s/%s.%u", snort_conf->log_dir, 
                                  data->filename, (uint32_t)curr_time);
    }

    if(value != SNORT_SNPRINTF_SUCCESS)
    {
        FatalError("unified log file logging path and file name are "
                   "too long, aborting!\n");
    }

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "Opening %s\n", logdir););

    if((data->stream = fopen(logdir, "wb+")) == NULL)
    {
        FatalError("UnifiedInitAlertFile(%s): %s\n", logdir, strerror(errno));
    }

    hdr.magic = ALERT_MAGIC;
    hdr.version_major = 1;
    hdr.version_minor = 81;
    hdr.timezone = snort_conf->thiszone;

    if(fwrite((char *)&hdr, sizeof(hdr), 1, data->stream) != 1)
    {
        FatalError("UnifiedAlertInit(): %s\n", strerror(errno));
    }
        
    fflush(data->stream);

    return;
}


static void OldUnifiedLogAlert(Packet *p, char *msg, void *arg, Event *event)
{
    RealUnifiedLogAlert(p, msg, arg, event, NULL);
}

static void UnifiedAlertRotateFile(UnifiedConfig *data)
{

    fclose(data->stream);
    data->current = 0;
    UnifiedInitAlertFile(data);
}

/* Unified Packet Log functions (deprecated) */

static void UnifiedLogInit(char *args)
{
    UnifiedConfig *UnifiedInfo;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: Unified Log Initialized\n"););

    /* parse the argument list from the rules file */
    UnifiedInfo = UnifiedParseArgs(args, "snort-unified.log");

    //LogMessage("UnifiedLogFilename = %s\n", UnifiedInfo->filename);

    UnifiedInitLogFile(UnifiedInfo);
    AddFuncToPostConfigList(UnifiedLogInitFinalize, UnifiedInfo);

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(OldUnifiedLogPacketAlert, OUTPUT_TYPE__LOG, UnifiedInfo);
    AddFuncToCleanExitList(UnifiedCleanExit, UnifiedInfo);
    AddFuncToRestartList(UnifiedRestart, UnifiedInfo);
}

static void UnifiedLogInitFinalize(int unused, void *arg)
{
    UnifiedConfig *data = (UnifiedConfig *)arg;
    UnifiedLogFileHeader hdr;
    int datalink = DAQ_GetBaseProtocol();

    /* write the log file header */
    hdr.magic = LOG_MAGIC;
    hdr.version_major = SNORT_VERSION_MAJOR;
    hdr.version_minor = SNORT_VERSION_MINOR;
    hdr.timezone = snort_conf->thiszone;
    hdr.snaplen = DAQ_GetSnapLen();
    hdr.sigfigs = 0;
    hdr.linktype = (datalink == DLT_RAW) ? DLT_EN10MB : datalink;

    if(fwrite((char *)&hdr, sizeof(hdr), 1, data->stream) != 1)
    {
        FatalError("UnifiedLogInitFinalize(): %s", strerror(errno));
    }

    fflush(data->stream);
}

/*
 * Function: UnifiedInitLogFile()
 *
 * Purpose: Initialize the unified log file header
 *
 * Arguments: data => pointer to the plugin's reference data struct 
 *
 * Returns: void function
 */
static void UnifiedInitLogFile(UnifiedConfig *data)
{
    time_t curr_time;      /* place to stick the clock data */
    char logdir[STD_BUF];
    int value;
    //UnifiedLogFileHeader hdr;

    bzero(logdir, STD_BUF);
    curr_time = time(NULL);

    if(data == NULL)
    {
        FatalError("Can't get unified plugin context, that's bad\n");
    }

    if(data->nostamp)
    {
        if(*(data->filename) == '/')
            value = SnortSnprintf(logdir, STD_BUF, "%s", data->filename);
        else
            value = SnortSnprintf(logdir, STD_BUF, "%s/%s", snort_conf->log_dir,  
                              data->filename);
    }
    else
    {
        if(*(data->filename) == '/')
            value = SnortSnprintf(logdir, STD_BUF, "%s.%u", data->filename, 
                              (uint32_t)curr_time);
        else
            value = SnortSnprintf(logdir, STD_BUF, "%s/%s.%u", snort_conf->log_dir,  
                              data->filename, (uint32_t)curr_time);
    }

    if(value != SNORT_SNPRINTF_SUCCESS)
    {
        FatalError("unified log file logging path and file name are "
                   "too long, aborting!\n");
    }

    if((data->stream = fopen(logdir, "wb")) == NULL)
    {
        FatalError("UnifiedInitLogFile(%s): %s\n", logdir, strerror(errno));
    }

#if 0
    /* write the log file header */
    hdr.magic = LOG_MAGIC;
    hdr.version_major = SNORT_VERSION_MAJOR;
    hdr.version_minor = SNORT_VERSION_MINOR;
    hdr.timezone = snort_conf->thiszone;
    hdr.snaplen = DAQ_GetSnapLen();
    hdr.sigfigs = 0;
    hdr.linktype = (datalink == DLT_RAW) ? DLT_EN10MB : datalink;

    if(fwrite((char *)&hdr, sizeof(hdr), 1, data->stream) != 1)
    {
        FatalError("UnifiedLogInit(): %s", strerror(errno));
    }
#endif

    fflush(data->stream);

    return;
}

typedef struct _OldUnifiedLogStreamCallbackData
{
    UnifiedLog *logheader;
    UnifiedConfig *data;
    Event *event;
    int first_time;
    int packet_flags;
    const void *eh;
} OldUnifiedLogStreamCallbackData;

/**
 * Callback for the Stream reassembler to log packets
 *
 */
static int OldUnifiedLogStreamCallback(DAQ_PktHdr_t *pkth,
                                uint8_t *packet_data, void *userdata)
{
    OldUnifiedLogStreamCallbackData *unifiedData;

    if (!userdata)
        return -1;

    unifiedData = (OldUnifiedLogStreamCallbackData *)userdata;

    unifiedData->logheader->flags = unifiedData->packet_flags;

    /* copy it's pktheader data into the logheader */
    unifiedData->logheader->pkth.ts.tv_sec = (uint32_t)pkth->ts.tv_sec;
    unifiedData->logheader->pkth.ts.tv_usec = (uint32_t)pkth->ts.tv_usec;
    unifiedData->logheader->pkth.caplen = (uint32_t)pkth->caplen;
    unifiedData->logheader->pkth.len = (uint32_t)pkth->pktlen;

    /*
    **  Add the ethernet header size to the total len.
    **  if the ethernet header is not already set.  We always
    **  log the ethernet header, even for raw packets.
    */
    if(!unifiedData->eh)
    {
        unifiedData->logheader->pkth.caplen += sizeof(EtherHdr);
        unifiedData->logheader->pkth.len += sizeof(EtherHdr);
    }

   /*  Set reference time equal to log time for the first packet  */
    if (unifiedData->first_time)
    {                    
        unifiedData->logheader->event.ref_time.tv_sec = unifiedData->logheader->pkth.ts.tv_sec;
        unifiedData->logheader->event.ref_time.tv_usec = unifiedData->logheader->pkth.ts.tv_usec;
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "sec: %lu\n", 
                    unifiedData->logheader->event.ref_time.tv_sec););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "usc: %lu\n", 
                    unifiedData->logheader->event.ref_time.tv_usec););

    }

    if(fwrite((char*)unifiedData->logheader,sizeof(UnifiedLog),1,unifiedData->data->stream) != 1)
        FatalError("SpoUnified: write failed: %s\n", 
                strerror(errno));

    unifiedData->data->current += sizeof(UnifiedLog);

    if(packet_data)
    {
        if(!unifiedData->eh)
        {
            EtherHdr eth;
            memset(eth.ether_src,0x00,6);
            memset(eth.ether_dst,0x00,6);
            eth.ether_type = htons(ETHERNET_TYPE_IP);

            if(fwrite((char*)&eth,sizeof(eth),1,unifiedData->data->stream) != 1)
                FatalError("SpoUnified: write failed: %s\n", strerror(errno));
            unifiedData->data->current += sizeof(EtherHdr);
        }
        
        if(fwrite((char*)packet_data,pkth->caplen,1,
                    unifiedData->data->stream) != 1)
            FatalError("SpoUnified: write failed: %s\n", 
                    strerror(errno));

        unifiedData->data->current += pkth->caplen;
    }

    /* after the first logged packet modify the event headers */
    if (unifiedData->first_time)
    {                    
        unifiedData->logheader->event.sig_generator = GENERATOR_TAG;
        unifiedData->logheader->event.sig_id = TAG_LOG_PKT;
        unifiedData->logheader->event.sig_rev = 1;
        unifiedData->logheader->event.classification = 0;
        unifiedData->logheader->event.priority = unifiedData->event->priority;    
        unifiedData->first_time = 0;
    }

    /* Update event ID for subsequent logged packets */
    unifiedData->logheader->event.event_id = ++event_id | ScEventLogId();

    return 0;
}

/*
 * Function: LogUnified(Packet *, char *msg, void *arg)
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
static void OldUnifiedLogPacketAlert(Packet *p, char *msg, void *arg, Event *event)
{
    OldUnifiedLogStreamCallbackData unifiedData;
    int first_time = 1;
    UnifiedLog logheader;
    UnifiedConfig *data = (UnifiedConfig *)arg;

    if(event != NULL)
    {
        logheader.event.sig_generator = event->sig_generator;
        logheader.event.sig_id = event->sig_id;
        logheader.event.sig_rev = event->sig_rev;
        logheader.event.classification = event->classification;
        logheader.event.priority = event->priority;
        logheader.event.event_id = event->event_id;
        logheader.event.event_reference = event->event_reference;
        logheader.event.ref_time.tv_sec = event->ref_time.tv_sec;
        logheader.event.ref_time.tv_usec = event->ref_time.tv_usec;

        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "------------\n"););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "gen: %u\n", 
                    logheader.event.sig_generator););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "sid: %u\n", 
                    logheader.event.sig_id););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "rev: %u\n", 
                    logheader.event.sig_rev););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "cls: %u\n", 
                    logheader.event.classification););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "pri: %u\n", 
                    logheader.event.priority););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "eid: %u\n", 
                    logheader.event.event_id););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "erf: %u\n", 
                    logheader.event.event_reference););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "sec: %lu\n",
                    logheader.event.ref_time.tv_sec););
        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "usc: %lu\n",
                    logheader.event.ref_time.tv_usec););
    }

    if(p && (p->packet_flags & PKT_REBUILT_STREAM) && stream_api)
    {
        unifiedData.logheader = &logheader;
        unifiedData.data = data;
        unifiedData.event = event;
        unifiedData.first_time = first_time;
        unifiedData.packet_flags = p->packet_flags;
        unifiedData.eh = p->eh;
        stream_api->traverse_reassembled(p, OldUnifiedLogStreamCallback, &unifiedData);
    }
    else
    {
        if(p)
        {
            logheader.flags = p->packet_flags;

            logheader.pkth.ts.tv_sec = (uint32_t)p->pkth->ts.tv_sec;
            logheader.pkth.ts.tv_usec = (uint32_t)p->pkth->ts.tv_usec;
            logheader.pkth.caplen = p->pkth->caplen;
            logheader.pkth.len = p->pkth->pktlen;


            /*
            **  Add the ethernet header size to the total len.
            **  if the ethernet header is not already set.  We always
            **  log the ethernet header, even for raw packets.
            */
            if(!p->eh)
            {
                logheader.pkth.caplen += sizeof(EtherHdr);
                logheader.pkth.len += sizeof(EtherHdr);
            }
        }
        else
        {
            logheader.flags = 0;
            logheader.pkth.ts.tv_sec = 0;
            logheader.pkth.ts.tv_usec = 0;
            logheader.pkth.caplen = 0;
            logheader.pkth.len = 0;
        }

        if((data->current + sizeof(UnifiedLog) + logheader.pkth.caplen) > 
                data->limit)
            UnifiedLogRotateFile(data);

        fwrite((char*)&logheader, sizeof(UnifiedLog), 1, data->stream);

        if(p)
        {
            if(!p->eh)
            {
                EtherHdr eth;
                memset(eth.ether_src,0x00,6);
                memset(eth.ether_dst,0x00,6);
                eth.ether_type = htons(ETHERNET_TYPE_IP);

                if(fwrite((char*)&eth,sizeof(eth),1,data->stream) != 1)
                    FatalError("SpoUnified: write failed: %s\n", strerror(errno));
                data->current += sizeof(EtherHdr);
            }
            fwrite((char*)p->pkt, p->pkth->caplen, 1, data->stream);
        }
    }

    fflush(data->stream);
    
    data->current += sizeof(UnifiedLog);
   
    if(p && p->pkth)
        data->current += p->pkth->caplen;
}


static void UnifiedLogRotateFile(UnifiedConfig *data)
{

    fclose(data->stream);
    data->current = 0;
    UnifiedInitLogFile(data);
    UnifiedLogInitFinalize(0, (void *)data);
}

