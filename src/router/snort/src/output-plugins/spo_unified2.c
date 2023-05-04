/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
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

/* spo_unified2.c
 * Adam Keeton
 *
 * 09/26/06
 * This file is litterally spo_unified.c converted to write unified2
 *
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

#include "sfutil/Unified2_common.h"
#include "spo_unified2.h"
#include "decode.h"
#include "rules.h"
#include "treenodes.h"
#include "util.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "mstring.h"
#include "event.h"
#include "generators.h"
#include "snort_debug.h"
#include "snort_bounds.h"
#include "obfuscation.h"
#include "active.h"
#include "detection_util.h"
#include "detect.h"

#include "snort.h"
#include "pcap_pkthdr32.h"

/* For the traversal of reassembled packets */
#include "stream_api.h"
#include "snort_httpinspect.h"


/* ------------------ Data structures --------------------------*/
typedef struct _Unified2Config
{
    char *base_filename;
    char filepath[STD_BUF];
    uint32_t timestamp;
    FILE *stream;
    unsigned int limit;
    unsigned int current;
    int nostamp;
#ifdef MPLS
    int mpls_event_types;
#endif
    int vlan_event_types;
    int base_proto;
#if defined(FEAT_OPEN_APPID)
    int appid_event_types;
#endif /* defined(FEAT_OPEN_APPID) */
} Unified2Config;

typedef struct _Unified2LogCallbackData
{
    Serial_Unified2Packet *logheader;
    Unified2Config *config;
    Event *event;
    uint32_t num_bytes;

} Unified2LogCallbackData;

Unified2Config *log_config = NULL;
Unified2Config *alert_config = NULL;


/* ----------------External variables -------------------- */
/* From fpdetect.c, for logging reassembled packets */
extern OptTreeNode *otn_tmp;

/* -------------------- Global Variables ----------------------*/
/* Used for buffering header and payload of unified records so only one
 * write is necessary.  Serial_Unified2IDSEventIPv6_legacy is used as Serial_Unified2IDSEvent_legacy size
 * since it is the largest */
static uint8_t write_pkt_buffer[sizeof(Serial_Unified2_Header) +
                                sizeof(Serial_Unified2IDSEventIPv6_legacy) + IP_MAXPACKET];
#define write_pkt_end (write_pkt_buffer + sizeof(write_pkt_buffer))

static uint8_t write_pkt_buffer_v2[sizeof(Serial_Unified2_Header) +
                                     sizeof(Unified2IDSEventIPv6) + IP_MAXPACKET];
#define write_pkt_end_v2 (write_pkt_buffer_v2 + sizeof(write_pkt_buffer_v2))

#define MAX_XDATA_WRITE_BUF_LEN (MAX_XFF_WRITE_BUF_LENGTH - \
        sizeof(struct in6_addr) + DECODE_BLEN)

/* This is the buffer to use for I/O.  Try to make big enough so the system
 * doesn't potentially flush in the middle of a record.  Every write is
 * force flushed to disk immediately after the entire record is written so
 * spoolers get an entire record */
#define UNIFIED2_SETVBUF
#ifndef WIN32
/* use the size of the buffer we copy record data into */
static char io_buffer[sizeof(write_pkt_buffer_v2)];
#else
# ifdef _MSC_VER
#  if _MSC_VER <= 1200
/* use maximum size defined by VC++ 6.0 */
static char io_buffer[32768];
#  else
static char io_buffer[sizeof(write_pkt_buffer_v2)];
#  endif  /* _MSC_VER <= 1200 */
# else
/* no _MSC_VER, don't set I/O buffer */
#  undef UNIFIED2_SETVBUF
# endif  /* _MSC_VER */
#endif  /* WIN32 */

/* -------------------- Local Functions -----------------------*/
static Unified2Config * Unified2ParseArgs(char *, char *);
static void Unified2CleanExit(int, void *);
#ifdef SNORT_RELOAD
static void Unified2Reload(struct _SnortConfig *, int, void *);
#endif

/* Unified2 Output functions */
static void Unified2Init(struct _SnortConfig *, char *);
static void Unified2PostConfig(struct _SnortConfig *, int, void *);
static void Unified2InitFile(Unified2Config *);
static inline void Unified2RotateFile(Unified2Config *);
static void Unified2LogAlert(Packet *, const char *, void *, Event *);
static void _AlertIP4(Packet *, const char *, Unified2Config *, Event *);
static void _AlertIP6(Packet *, const char *, Unified2Config *, Event *);
static void Unified2LogPacketAlert(Packet *, const char *, void *, Event *);
static void _Unified2LogPacketAlert(Packet *, const char *, Unified2Config *, Event *);
static void _Unified2LogStreamAlert(Packet *, const char *, Unified2Config *, Event *);
static int Unified2LogStreamCallback(DAQ_PktHdr_t *, uint8_t *, void *);
static void Unified2Write(uint8_t *, uint32_t, Unified2Config *);

static void _AlertIP4_v2(Packet *, const char *, Unified2Config *, Event *);
static void _AlertIP6_v2(Packet *, const char *, Unified2Config *, Event *);

/* Unified2 Alert functions (deprecated) */
static void Unified2AlertInit(struct _SnortConfig *, char *);

/* Unified2 Packet Log functions (deprecated) */
static void Unified2LogInit(struct _SnortConfig *, char *);

static ObRet Unified2LogObfuscationCallback(const DAQ_PktHdr_t *pkth,
        const uint8_t *packet_data, ob_size_t length, ob_char_t ob_char, void *userdata);

static void AlertExtraData(void *ssnptr, void *data, LogFunction *log_funcs, uint32_t max_count, uint32_t xtradata_mask, uint32_t event_id, uint32_t event_second);

#define U2_PACKET_FLAG 1
/* Obsolete flag as UI wont check the impact_flag field anymore.*/
#define U2_FLAG_BLOCKED 0x20
/* New flags to set the pad field (corresponds to blocked column in UI) with packet action*/
#define U2_BLOCKED_FLAG_ALLOW 0x00
#define U2_BLOCKED_FLAG_BLOCK 0x01
#define U2_BLOCKED_FLAG_WOULD 0x02
#define U2_BLOCKED_FLAG_CANT  0x03

/*
 * Function: SetupUnified2()
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
void Unified2Setup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("log_unified2", OUTPUT_TYPE_FLAG__LOG, Unified2LogInit);
    RegisterOutputPlugin("alert_unified2", OUTPUT_TYPE_FLAG__ALERT, Unified2AlertInit);
    RegisterOutputPlugin("unified2", OUTPUT_TYPE_FLAG__LOG | OUTPUT_TYPE_FLAG__ALERT, Unified2Init);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output plugin: Unified2 "
                            "logging/alerting is setup...\n"););
}

/*
 * Function: Unified2Init(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void Unified2Init(struct _SnortConfig *sc, char *args)
{
    Unified2Config *config;

    /* parse the argument list from the rules file */
    config = Unified2ParseArgs(args, "snort-unified");

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, Unified2LogAlert, OUTPUT_TYPE__ALERT, config);
    AddFuncToOutputList(sc, Unified2LogPacketAlert, OUTPUT_TYPE__LOG, config);

    AddFuncToCleanExitList(Unified2CleanExit, config);
#ifdef SNORT_RELOAD
    AddFuncToReloadList(Unified2Reload, config);
#endif
    AddFuncToPostConfigList(sc, Unified2PostConfig, config);
}

static void Unified2PostConfig(struct _SnortConfig *sc, int unused, void *data)
{
    Unified2Config *config = (Unified2Config *)data;
    int status;

    if (config == NULL || config->base_filename == NULL)
    {
        FatalError("%s(%d) Could not initialize unified2 file: Unified2 "
                   "configuration data or file name is NULL.\n",
                   __FILE__, __LINE__);
    }

#ifndef WIN32
    if (config->base_filename[0] == '/')
    {
        status = SnortSnprintf(config->filepath, sizeof(config->filepath),
                               "%s", config->base_filename);
    }
    else
#endif
    {
        status = SnortSnprintf(config->filepath, sizeof(config->filepath),
                               "%s/%s", sc->log_dir, config->base_filename);
    }

    if (status != SNORT_SNPRINTF_SUCCESS)
    {
        FatalError("%s(%d) Failed to copy unified2 file name\n",
                   __FILE__, __LINE__);
    }
    config->base_proto = htonl(DAQ_GetBaseProtocol());


    Unified2InitFile(config);

    if(stream_api)
    {
        stream_api->reg_xtra_data_log(AlertExtraData, (void *)config);
    }
}

/*
 * Function: Unified2InitFile()
 *
 * Purpose: Initialize the unified2 ouput file
 *
 * Arguments: config => pointer to the plugin's reference data struct
 *
 * Returns: void function
 */
static void Unified2InitFile(Unified2Config *config)
{
    char filepath[STD_BUF];
    char *fname_ptr;

    if (config == NULL)
    {
        FatalError("%s(%d) Could not initialize unified2 file: Unified2 "
                   "configuration data is NULL.\n", __FILE__, __LINE__);
    }

    config->timestamp = (uint32_t)time(NULL);

    if (!config->nostamp)
    {
        if (SnortSnprintf(filepath, sizeof(filepath), "%s.%u",
                          config->filepath, config->timestamp) != SNORT_SNPRINTF_SUCCESS)
        {
            FatalError("%s(%d) Failed to copy unified2 file path.\n",
                       __FILE__, __LINE__);
        }

        fname_ptr = filepath;
    }
    else
    {
        fname_ptr = config->filepath;
    }

    if ((config->stream = fopen(fname_ptr, "wb")) == NULL)
    {
        FatalError("%s(%d) Could not open %s: %s\n",
                   __FILE__, __LINE__, fname_ptr, strerror(errno));
    }

#ifdef UNIFIED2_SETVBUF
    /* Set buffer to size of record buffer so the system doesn't flush
     * part of a record if it's greater than BUFSIZ */
    if (setvbuf(config->stream, io_buffer, _IOFBF, sizeof(io_buffer)) != 0)
    {
        ErrorMessage("%s(%d) Could not set I/O buffer: %s. "
                     "Using system default.\n",
                     __FILE__, __LINE__, strerror(errno));
    }
#endif

    /* If test mode, close and delete the file */
    if (ScTestMode())
    {
        fclose(config->stream);
        config->stream = NULL;
        if (unlink(fname_ptr) == -1)
        {
            ErrorMessage("%s(%d) Running in test mode so we want to remove "
                         "test unified2 file. Could not unlink file \"%s\": %s\n",
                         __FILE__, __LINE__, fname_ptr, strerror(errno));
        }
    }
}

static inline void Unified2RotateFile(Unified2Config *config)
{
    fclose(config->stream);
    config->current = 0;
    Unified2InitFile(config);
}

static int s_blocked_flag[] =
{
    U2_BLOCKED_FLAG_ALLOW,
    U2_BLOCKED_FLAG_CANT,
    U2_BLOCKED_FLAG_WOULD,
    U2_BLOCKED_FLAG_BLOCK,
    U2_BLOCKED_FLAG_BLOCK
};

static int GetU2Flags(const Packet* p, uint8_t* pimpact)
{
    tActiveDrop dispos = Active_GetDisposition();

    if ( dispos >= ACTIVE_DROP )
    {
        *pimpact = U2_FLAG_BLOCKED;
        return U2_BLOCKED_FLAG_BLOCK;
    }
    return s_blocked_flag[dispos];
}

static void _AlertIP4(Packet *p, const char *msg, Unified2Config *config, Event *event)
{
    Serial_Unified2_Header hdr;
    Serial_Unified2IDSEvent_legacy alertdata;
    uint32_t write_len = sizeof(Serial_Unified2_Header) + sizeof(Serial_Unified2IDSEvent_legacy);

    memset(&alertdata, 0, sizeof(alertdata));

    alertdata.event_id = htonl(event->event_id);
    alertdata.event_second = htonl(event->ref_time.tv_sec);
    alertdata.event_microsecond = htonl(event->ref_time.tv_usec);
    alertdata.generator_id = htonl(event->sig_generator);
    alertdata.signature_id = htonl(event->sig_id);
    alertdata.signature_revision = htonl(event->sig_rev);
    alertdata.classification_id = htonl(event->classification);
    alertdata.priority_id = htonl(event->priority);

    if (p != NULL)
    {
        alertdata.blocked = GetU2Flags(p, &alertdata.impact_flag);

        if(IPH_IS_VALID(p))
        {
            alertdata.ip_source = p->iph->ip_src.s_addr;
            alertdata.ip_destination = p->iph->ip_dst.s_addr;
            alertdata.protocol = GetEventProto(p);

            if ((alertdata.protocol == IPPROTO_ICMP) && p->icmph)
            {
                alertdata.sport_itype = htons(p->icmph->type);
                alertdata.dport_icode = htons(p->icmph->code);
            }
            else if (!IsPortscanPacket(p))
            {
                alertdata.sport_itype = htons(p->sp);
                alertdata.dport_icode = htons(p->dp);
            }
        }
    }

    if ((config->current + write_len) > config->limit)
        Unified2RotateFile(config);

    hdr.length = htonl(sizeof(Serial_Unified2IDSEvent_legacy));
    hdr.type = htonl(UNIFIED2_IDS_EVENT);

    if (SafeMemcpy(write_pkt_buffer, &hdr, sizeof(Serial_Unified2_Header),
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header),
                   &alertdata, sizeof(Serial_Unified2IDSEvent_legacy),
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2IDSEvent_legacy. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    Unified2Write(write_pkt_buffer, write_len, config);
}

static void _AlertIP4_v2(Packet *p, const char *msg, Unified2Config *config, Event *event)
{
    Serial_Unified2_Header hdr;
    Unified2IDSEvent alertdata;
    uint32_t write_len = sizeof(Serial_Unified2_Header) + sizeof(Unified2IDSEvent);

    memset(&alertdata, 0, sizeof(alertdata));

    alertdata.event_id = htonl(event->event_id);
    alertdata.event_second = htonl(event->ref_time.tv_sec);
    alertdata.event_microsecond = htonl(event->ref_time.tv_usec);
    alertdata.generator_id = htonl(event->sig_generator);
    alertdata.signature_id = htonl(event->sig_id);
    alertdata.signature_revision = htonl(event->sig_rev);
    alertdata.classification_id = htonl(event->classification);
    alertdata.priority_id = htonl(event->priority);
#if defined(FEAT_OPEN_APPID)
    memcpy(alertdata.app_name, event->app_name, sizeof(alertdata.app_name));
#endif /* defined(FEAT_OPEN_APPID) */

    if(p)
    {
        alertdata.blocked = GetU2Flags(p, &alertdata.impact_flag);

        if(IPH_IS_VALID(p))
        {
            alertdata.ip_source = p->iph->ip_src.s_addr;
            alertdata.ip_destination = p->iph->ip_dst.s_addr;
            alertdata.protocol = GetEventProto(p);

            if ((alertdata.protocol == IPPROTO_ICMP) && p->icmph)
            {
                alertdata.sport_itype = htons(p->icmph->type);
                alertdata.dport_icode = htons(p->icmph->code);
            }
            else if (!IsPortscanPacket(p))
            {
                alertdata.sport_itype = htons(p->sp);
                alertdata.dport_icode = htons(p->dp);
            }

#ifdef MPLS
            if((p->mpls) && (config->mpls_event_types))
            {
                alertdata.mpls_label = htonl(p->mplsHdr.label);
            }
#endif
            if(config->vlan_event_types)
            {
                if(p->vh)
                {
                    alertdata.vlanId = htons(VTH_VLAN(p->vh));
                }

                alertdata.pad2 = htons(p->configPolicyId);
            }

#if defined(FEAT_OPEN_APPID)
            if((event->app_name[0]) && (config->appid_event_types))
            {
                memcpy(alertdata.app_name, event->app_name, sizeof(alertdata.app_name));
            }
#endif /* defined(FEAT_OPEN_APPID) */
        }
    }

    if ((config->current + write_len) > config->limit)
        Unified2RotateFile(config);

    hdr.length = htonl(sizeof(Unified2IDSEvent));
#if !defined(FEAT_OPEN_APPID)
    hdr.type = htonl(UNIFIED2_IDS_EVENT_VLAN);
#else /* defined(FEAT_OPEN_APPID) */
    hdr.type = htonl(UNIFIED2_IDS_EVENT_APPID);
#endif /* defined(FEAT_OPEN_APPID) */

    if (SafeMemcpy(write_pkt_buffer_v2, &hdr, sizeof(Serial_Unified2_Header),
                   write_pkt_buffer_v2, write_pkt_end_v2) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    if (SafeMemcpy(write_pkt_buffer_v2 + sizeof(Serial_Unified2_Header),
                   &alertdata, sizeof(Unified2IDSEvent),
                   write_pkt_buffer_v2, write_pkt_end_v2) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2IDSEvent_legacy. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    Unified2Write(write_pkt_buffer_v2, write_len, config);
}

static void _AlertIP6(Packet *p, const char *msg, Unified2Config *config, Event *event)
{
    Serial_Unified2_Header hdr;
    Serial_Unified2IDSEventIPv6_legacy alertdata;
    uint32_t write_len = sizeof(Serial_Unified2_Header) + sizeof(Serial_Unified2IDSEventIPv6_legacy);

    memset(&alertdata, 0, sizeof(alertdata));

    alertdata.event_id = htonl(event->event_id);
    alertdata.event_second = htonl(event->ref_time.tv_sec);
    alertdata.event_microsecond = htonl(event->ref_time.tv_usec);
    alertdata.generator_id = htonl(event->sig_generator);
    alertdata.signature_id = htonl(event->sig_id);
    alertdata.signature_revision = htonl(event->sig_rev);
    alertdata.classification_id = htonl(event->classification);
    alertdata.priority_id = htonl(event->priority);

    if(p)
    {
        alertdata.blocked = GetU2Flags(p, &alertdata.impact_flag);

        if(IPH_IS_VALID(p))
        {
            sfaddr_t* ip;

            ip = GET_SRC_IP(p);
            alertdata.ip_source = *(struct in6_addr*)sfaddr_get_ip6_ptr(ip);

            ip = GET_DST_IP(p);
            alertdata.ip_destination = *(struct in6_addr*)sfaddr_get_ip6_ptr(ip);

            alertdata.protocol = GetEventProto(p);

            if ((alertdata.protocol == IPPROTO_ICMP) && p->icmph)
            {
                alertdata.sport_itype = htons(p->icmph->type);
                alertdata.dport_icode = htons(p->icmph->code);
            }
            else if (!IsPortscanPacket(p))
            {
                alertdata.sport_itype = htons(p->sp);
                alertdata.dport_icode = htons(p->dp);
            }
        }
    }

    if ((config->current + write_len) > config->limit)
        Unified2RotateFile(config);

    hdr.length = htonl(sizeof(Serial_Unified2IDSEventIPv6_legacy));
    hdr.type = htonl(UNIFIED2_IDS_EVENT_IPV6);

    if (SafeMemcpy(write_pkt_buffer, &hdr, sizeof(Serial_Unified2_Header),
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header),
                   &alertdata, sizeof(Serial_Unified2IDSEventIPv6_legacy),
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2IDSEventIPv6_legacy. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    Unified2Write(write_pkt_buffer, write_len, config);
}

static void _AlertIP6_v2(Packet *p, const char *msg, Unified2Config *config, Event *event)
{
    Serial_Unified2_Header hdr;
    Unified2IDSEventIPv6 alertdata;
    uint32_t write_len = sizeof(Serial_Unified2_Header) + sizeof(Unified2IDSEventIPv6);

    memset(&alertdata, 0, sizeof(alertdata));

    alertdata.event_id = htonl(event->event_id);
    alertdata.event_second = htonl(event->ref_time.tv_sec);
    alertdata.event_microsecond = htonl(event->ref_time.tv_usec);
    alertdata.generator_id = htonl(event->sig_generator);
    alertdata.signature_id = htonl(event->sig_id);
    alertdata.signature_revision = htonl(event->sig_rev);
    alertdata.classification_id = htonl(event->classification);
    alertdata.priority_id = htonl(event->priority);
#if defined(FEAT_OPEN_APPID)
    memcpy(alertdata.app_name, event->app_name, sizeof(alertdata.app_name));
#endif /* defined(FEAT_OPEN_APPID) */

    if(p)
    {
        alertdata.blocked = GetU2Flags(p, &alertdata.impact_flag);

        if(IPH_IS_VALID(p))
        {
            sfaddr_t* ip;

            ip = GET_SRC_IP(p);
            alertdata.ip_source = *(struct in6_addr*)sfaddr_get_ip6_ptr(ip);

            ip = GET_DST_IP(p);
            alertdata.ip_destination = *(struct in6_addr*)sfaddr_get_ip6_ptr(ip);

            alertdata.protocol = GetEventProto(p);

            if ((alertdata.protocol == IPPROTO_ICMP) && p->icmph)
            {
                alertdata.sport_itype = htons(p->icmph->type);
                alertdata.dport_icode = htons(p->icmph->code);
            }
            else if (!IsPortscanPacket(p))
            {
                alertdata.sport_itype = htons(p->sp);
                alertdata.dport_icode = htons(p->dp);
            }

#ifdef MPLS
            if((p->mpls) && (config->mpls_event_types))
            {
                alertdata.mpls_label = htonl(p->mplsHdr.label);
            }
#endif
            if(config->vlan_event_types)
            {
                if(p->vh)
                {
                    alertdata.vlanId = htons(VTH_VLAN(p->vh));
                }

                alertdata.pad2 = htons(p->configPolicyId);
            }
#if defined(FEAT_OPEN_APPID)

            if((event->app_name[0]) && (config->appid_event_types))
            {
                memcpy(alertdata.app_name, event->app_name, sizeof(alertdata.app_name));
            }
#endif /* defined(FEAT_OPEN_APPID) */
        }
    }

    if ((config->current + write_len) > config->limit)
        Unified2RotateFile(config);

    hdr.length = htonl(sizeof(Unified2IDSEventIPv6));
#if !defined(FEAT_OPEN_APPID)
    hdr.type = htonl(UNIFIED2_IDS_EVENT_IPV6_VLAN);
#else
    hdr.type = htonl(UNIFIED2_IDS_EVENT_APPID_IPV6);
#endif

    if (SafeMemcpy(write_pkt_buffer_v2, &hdr, sizeof(Serial_Unified2_Header),
                   write_pkt_buffer_v2, write_pkt_end_v2) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    if (SafeMemcpy(write_pkt_buffer_v2 + sizeof(Serial_Unified2_Header),
                   &alertdata, sizeof(Unified2IDSEventIPv6),
                   write_pkt_buffer_v2, write_pkt_end_v2) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Unified2IDSEventIPv6. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    Unified2Write(write_pkt_buffer_v2, write_len, config);
}

void _WriteExtraData(Unified2Config *config, uint32_t event_id, uint32_t event_second, uint8_t *buffer, uint32_t len, uint32_t type )
{

    Serial_Unified2_Header hdr;
    SerialUnified2ExtraData alertdata;
    Unified2ExtraDataHdr alertHdr;
    uint8_t write_buffer[MAX_XDATA_WRITE_BUF_LEN];
    uint8_t *write_end = NULL;
    uint8_t *ptr = NULL;


    uint32_t write_len;

    write_len = sizeof(Serial_Unified2_Header) + sizeof(Unified2ExtraDataHdr);

    alertdata.sensor_id = 0;
    alertdata.event_id = htonl(event_id);
    alertdata.event_second = htonl(event_second);
    alertdata.data_type = htonl(EVENT_DATA_TYPE_BLOB);

    alertdata.type = htonl(type);
    alertdata.blob_length = htonl(sizeof(alertdata.data_type) +
                sizeof(alertdata.blob_length) + len);


    write_len = write_len + sizeof(alertdata) + len;
    alertHdr.event_type = htonl(EVENT_TYPE_EXTRA_DATA);
    alertHdr.event_length = htonl(write_len - sizeof(Serial_Unified2_Header));


    if ((config->current + write_len) > config->limit)
        Unified2RotateFile(config);

    hdr.length = htonl(write_len - sizeof(Serial_Unified2_Header));
    hdr.type = htonl(UNIFIED2_EXTRA_DATA);

    write_end = write_buffer + sizeof(write_buffer);


    ptr = write_buffer;

    if (SafeMemcpy(ptr, &hdr, sizeof(hdr),
                   write_buffer, write_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    ptr = ptr +  sizeof(hdr);

    if (SafeMemcpy(ptr, &alertHdr, sizeof(alertHdr),
                   write_buffer, write_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Unified2ExtraDataHdr. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    ptr = ptr + sizeof(alertHdr);

    if (SafeMemcpy(ptr, &alertdata, sizeof(alertdata),
                   write_buffer, write_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy SerialUnified2ExtraData. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    ptr = ptr + sizeof(alertdata);

    if (SafeMemcpy(ptr, buffer, len,
                write_buffer, write_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Gzip Decompressed Buffer. "
                "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    Unified2Write(write_buffer, write_len, config);
}

static void AlertExtraData(
    void *ssnptr, void *data,
    LogFunction *log_funcs, uint32_t max_count,
    uint32_t xtradata_mask,
    uint32_t event_id, uint32_t event_second)
{
    Unified2Config *config = (Unified2Config *)data;
    uint32_t xid;

    if((config == NULL) || !xtradata_mask || !event_second)
        return;

    xid = ffs(xtradata_mask);

    while ( xid && (xid <= max_count) )
    {
        uint32_t len = 0;
        uint32_t type = 0;
        uint8_t *write_buffer;

        if ( log_funcs[xid-1](ssnptr, &write_buffer, &len, &type) && (len > 0) )
        {
            _WriteExtraData(config, event_id, event_second, write_buffer, len, type);
        }
        xtradata_mask ^= BIT(xid);
        xid = ffs(xtradata_mask);
    }
}

static void Unified2LogAlert(Packet *p, const char *msg, void *arg, Event *event)
{
    Unified2Config *config = (Unified2Config *)arg;

    if (config == NULL)
        return;

    if(!event) return;

    if(IS_IP4(p))
    {
#ifdef MPLS
#if !defined(FEAT_OPEN_APPID)
        if((config->vlan_event_types) || (config->mpls_event_types))
#else /* defined(FEAT_OPEN_APPID) */
        if((config->vlan_event_types) || (config->mpls_event_types) || (config->appid_event_types))
#endif /* defined(FEAT_OPEN_APPID) */
#else
#if !defined(FEAT_OPEN_APPID)
        if(config->vlan_event_types)
#else /* defined(FEAT_OPEN_APPID) */
        if(config->vlan_event_types || config->appid_event_types)
#endif /* defined(FEAT_OPEN_APPID) */
#endif
        {
            _AlertIP4_v2(p, msg, config, event);
        }
        else
            _AlertIP4(p, msg, config, event);
    }
    else
    {
#ifdef MPLS
#if !defined(FEAT_OPEN_APPID)
        if((config->vlan_event_types) || (config->mpls_event_types))
#else /* defined(FEAT_OPEN_APPID) */
        if((config->vlan_event_types) || (config->mpls_event_types) || (config->appid_event_types))
#endif /* defined(FEAT_OPEN_APPID) */
#else
#if !defined(FEAT_OPEN_APPID)
        if(config->vlan_event_types)
#else /* defined(FEAT_OPEN_APPID) */
        if(config->vlan_event_types || config->appid_event_types)
#endif /* defined(FEAT_OPEN_APPID) */
#endif
        {
            _AlertIP6_v2(p, msg, config, event);
        }
        else
            _AlertIP6(p, msg, config, event);

        if(ScLogIPv6Extra() && IS_IP6(p))
        {
            sfaddr_t* ip = GET_SRC_IP(p);
            _WriteExtraData(config, event->event_id, event->ref_time.tv_sec,
                (uint8_t*)sfaddr_get_ip6_ptr(ip), sizeof(struct in6_addr),  EVENT_INFO_IPV6_SRC);
            ip = GET_DST_IP(p);
            _WriteExtraData(config, event->event_id, event->ref_time.tv_sec,
                (uint8_t*)sfaddr_get_ip6_ptr(ip), sizeof(struct in6_addr),  EVENT_INFO_IPV6_DST);
        }
    }

    if ( p->ssnptr )
        stream_api->update_session_alert(
            p->ssnptr, p, event->sig_generator, event->sig_id,
            event->event_id, event->ref_time.tv_sec);

    if ( p->xtradata_mask && !(p->packet_flags & PKT_STREAM_INSERT) 
        && !(p->packet_flags & PKT_REBUILT_STREAM) )
    {
        LogFunction *log_funcs;
        uint32_t max_count = stream_api->get_xtra_data_map(&log_funcs);

        if ( max_count > 0 )
            AlertExtraData(
                p->ssnptr, config, log_funcs, max_count, p->xtradata_mask,
                event->event_id, event->ref_time.tv_sec);
    }

    return;
}

static void Unified2LogPacketAlert(Packet *p, const char *msg, void *arg, Event *event)
{
    Unified2Config *config = (Unified2Config *)arg;

    if (config == NULL)
        return;

    if(p)
    {
        if ((p->packet_flags & PKT_REBUILT_STREAM) && stream_api)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_LOG,
                        "[*] Reassembled packet, dumping stream packets\n"););
            _Unified2LogStreamAlert(p, msg, config, event);
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_LOG, "[*] Logging unified 2 packets...\n"););
            _Unified2LogPacketAlert(p, msg, config, event);
        }
   }
}

static void _Unified2LogPacketAlert(Packet *p, const char *msg,
                                    Unified2Config *config, Event *event)
{
    Serial_Unified2_Header hdr;
    Serial_Unified2Packet logheader;
    uint32_t pkt_length = 0;
    uint32_t write_len = sizeof(Serial_Unified2_Header) + sizeof(Serial_Unified2Packet) - 4;

    logheader.sensor_id = 0;
    logheader.linktype = config->base_proto;

    if (event != NULL)
    {
        logheader.event_id = htonl(event->event_reference);
        logheader.event_second = htonl(event->ref_time.tv_sec);

        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "------------\n"));
    }
    else
    {
        logheader.event_id = 0;
        logheader.event_second = 0;
    }

    if ((p != NULL) && (p->pkt != NULL) && (p->pkth != NULL)
            && obApi->payloadObfuscationRequired(p))
    {
        Unified2LogCallbackData unifiedData;

        unifiedData.logheader = &logheader;
        unifiedData.config = config;
        unifiedData.event = event;
        unifiedData.num_bytes = 0;

        if (obApi->obfuscatePacket(p, Unified2LogObfuscationCallback,
                (void *)&unifiedData) == OB_RET_SUCCESS)
        {
            /* Write the last record */
            if (unifiedData.num_bytes != 0)
                Unified2Write(write_pkt_buffer, unifiedData.num_bytes, config);
            return;
        }
    }

    if(p && p->pkt && p->pkth)
    {
        logheader.packet_second = htonl((uint32_t)p->pkth->ts.tv_sec);
        logheader.packet_microsecond = htonl((uint32_t)p->pkth->ts.tv_usec);
        logheader.packet_length = htonl(p->pkth->caplen);

        pkt_length = p->pkth->caplen;
        write_len += pkt_length;
    }
    else
    {
        logheader.packet_second = 0;
        logheader.packet_microsecond = 0;
        logheader.packet_length = 0;
    }

    if ((config->current + write_len) > config->limit)
        Unified2RotateFile(config);

    hdr.length = htonl(sizeof(Serial_Unified2Packet) - 4 + pkt_length);
    hdr.type = htonl(UNIFIED2_PACKET);

    if (SafeMemcpy(write_pkt_buffer, &hdr, sizeof(Serial_Unified2_Header),
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header),
                   &logheader, sizeof(Serial_Unified2Packet) - 4,
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2Packet. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return;
    }

    if (pkt_length != 0)
    {
        if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header) +
                       sizeof(Serial_Unified2Packet) - 4,
                       p->pkt, pkt_length,
                       write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
        {
            ErrorMessage("%s(%d) Failed to copy packet data. "
                         "Not writing unified2 event.\n", __FILE__, __LINE__);
            return;
        }
    }

    Unified2Write(write_pkt_buffer, write_len, config);
   
    if ( p->xtradata_mask && (Active_GetDisposition() >= ACTIVE_DROP) )
    {
        LogFunction *log_funcs;
        uint32_t max_count = stream_api->get_xtra_data_map(&log_funcs);

        if ( max_count > 0 )
            AlertExtraData(
                p->ssnptr, config, log_funcs, max_count, p->xtradata_mask,
                event->event_id, event->ref_time.tv_sec);
    }
}

/**
 * Callback for the Stream reassembler to log packets
 *
 */
static int Unified2LogStreamCallback(DAQ_PktHdr_t *pkth,
                                     uint8_t *packet_data, void *userdata)
{
    Unified2LogCallbackData *unifiedData = (Unified2LogCallbackData *)userdata;
    Serial_Unified2_Header hdr;
    uint32_t write_len = sizeof(Serial_Unified2_Header) + sizeof(Serial_Unified2Packet) - 4;

    if (!userdata || !pkth || !packet_data)
        return -1;

    write_len += pkth->caplen;
    if ((unifiedData->config->current + write_len) > unifiedData->config->limit)
        Unified2RotateFile(unifiedData->config);

    hdr.type = htonl(UNIFIED2_PACKET);
    hdr.length = htonl(sizeof(Serial_Unified2Packet) - 4 + pkth->caplen);

    /* Event data will already be set */

    unifiedData->logheader->packet_second = htonl((uint32_t)pkth->ts.tv_sec);
    unifiedData->logheader->packet_microsecond = htonl((uint32_t)pkth->ts.tv_usec);
    unifiedData->logheader->packet_length = htonl(pkth->caplen);

    if (SafeMemcpy(write_pkt_buffer, &hdr, sizeof(Serial_Unified2_Header),
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return -1;
    }

    if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header),
                   unifiedData->logheader, sizeof(Serial_Unified2Packet) - 4,
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy Serial_Unified2Packet. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return -1;
    }

    if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header) +
                   sizeof(Serial_Unified2Packet) - 4,
                   packet_data, pkth->caplen,
                   write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
    {
        ErrorMessage("%s(%d) Failed to copy packet data. "
                     "Not writing unified2 event.\n", __FILE__, __LINE__);
        return -1;
    }

    Unified2Write(write_pkt_buffer, write_len, unifiedData->config);

#if 0
    /* DO NOT DO THIS FOR UNIFIED2.
     * The event referenced below in the unifiedData is a pointer
     * to the actual event and this changes its gid & sid to 2:1.
     * That is baaaaad.
     */
    /* after the first logged packet modify the event headers */
    if(!unifiedData->once++)
    {
        unifiedData->event->sig_generator = GENERATOR_TAG;
        unifiedData->event->sig_id = TAG_LOG_PKT;
        unifiedData->event->sig_rev = 1;
        unifiedData->event->classification = 0;
        unifiedData->event->priority = unifiedData->event->priority;
        /* Note that event_id is now incorrect.
         * See OldUnified2LogPacketAlert() for details. */
    }
#endif

    return 0;
}

static ObRet Unified2LogObfuscationCallback(const DAQ_PktHdr_t *pkth,
        const uint8_t *packet_data, ob_size_t length,
        ob_char_t ob_char, void *userdata)
{
    Unified2LogCallbackData *unifiedData = (Unified2LogCallbackData *)userdata;

    if (userdata == NULL)
        return OB_RET_ERROR;

    if (pkth != NULL)
    {
        Serial_Unified2_Header hdr;
        uint32_t record_len = (pkth->caplen + sizeof(Serial_Unified2_Header)
                + (sizeof(Serial_Unified2Packet) - 4));

        /* Write the last buffer if present.  Want to write an entire record
         * at a time in case of failures, we don't corrupt the log file. */
        if (unifiedData->num_bytes != 0)
            Unified2Write(write_pkt_buffer, unifiedData->num_bytes, unifiedData->config);

        if ((write_pkt_buffer + record_len) > write_pkt_end)
        {
            ErrorMessage("%s(%d) Too much data. Not writing unified2 event.\n",
                    __FILE__, __LINE__);
            return OB_RET_ERROR;
        }

        if ((unifiedData->config->current + record_len) > unifiedData->config->limit)
            Unified2RotateFile(unifiedData->config);

        hdr.type = htonl(UNIFIED2_PACKET);
        hdr.length = htonl((sizeof(Serial_Unified2Packet) - 4) + pkth->caplen);

        /* Event data will already be set */

        unifiedData->logheader->packet_second = htonl((uint32_t)pkth->ts.tv_sec);
        unifiedData->logheader->packet_microsecond = htonl((uint32_t)pkth->ts.tv_usec);
        unifiedData->logheader->packet_length = htonl(pkth->caplen);

        if (SafeMemcpy(write_pkt_buffer, &hdr, sizeof(Serial_Unified2_Header),
                    write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
        {
            ErrorMessage("%s(%d) Failed to copy Serial_Unified2_Header. "
                    "Not writing unified2 event.\n", __FILE__, __LINE__);
            return OB_RET_ERROR;
        }

        if (SafeMemcpy(write_pkt_buffer + sizeof(Serial_Unified2_Header),
                    unifiedData->logheader, sizeof(Serial_Unified2Packet) - 4,
                    write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
        {
            ErrorMessage("%s(%d) Failed to copy Serial_Unified2Packet. "
                    "Not writing unified2 event.\n", __FILE__, __LINE__);
            return OB_RET_ERROR;
        }

        /* Reset this for the new record */
        unifiedData->num_bytes = (record_len - pkth->caplen);
    }

    if (packet_data != NULL)
    {
        if (SafeMemcpy(write_pkt_buffer + unifiedData->num_bytes,
                    packet_data, (size_t)length,
                    write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
        {
            ErrorMessage("%s(%d) Failed to copy packet data "
                    "Not writing unified2 event.\n", __FILE__, __LINE__);
            return OB_RET_ERROR;
        }
    }
    else
    {
        if (SafeMemset(write_pkt_buffer + unifiedData->num_bytes,
                    (uint8_t)ob_char, (size_t)length,
                    write_pkt_buffer, write_pkt_end) != SAFEMEM_SUCCESS)
        {
            ErrorMessage("%s(%d) Failed to obfuscate packet data "
                    "Not writing unified2 event.\n", __FILE__, __LINE__);
            return OB_RET_ERROR;
        }
    }

    unifiedData->num_bytes += length;

    return OB_RET_SUCCESS;
}


/**
 * Log a set of packets stored in the stream reassembler
 *
 */
static void _Unified2LogStreamAlert(Packet *p, const char *msg, Unified2Config *config, Event *event)
{
    Unified2LogCallbackData unifiedData;
    Serial_Unified2Packet logheader;

    logheader.sensor_id = 0;
    logheader.linktype = config->base_proto;

    /* setup the event header */
    if (event != NULL)
    {
        logheader.event_id = htonl(event->event_reference);
        logheader.event_second = htonl(event->ref_time.tv_sec);
    }
    else
    {
        logheader.event_id = 0;
        logheader.event_second = 0;
    }

    /* queue up the stream for logging */
    unifiedData.logheader = &logheader;
    unifiedData.config = config;
    unifiedData.event = event;
    unifiedData.num_bytes = 0;

    if ((p != NULL) && (p->pkt != NULL) && (p->pkth != NULL)
            && obApi->payloadObfuscationRequired(p))
    {
        if (obApi->obfuscatePacketStreamSegments(p, Unified2LogObfuscationCallback,
                (void *)&unifiedData) == OB_RET_SUCCESS)
        {
            /* Write the last record */
            if (unifiedData.num_bytes != 0)
                Unified2Write(write_pkt_buffer, unifiedData.num_bytes, config);
            return;
        }

        /* Reset since we failed */
        unifiedData.num_bytes = 0;
    }

    if (!p)
        return;

    stream_api->traverse_reassembled(p, Unified2LogStreamCallback, &unifiedData);
}

/*
 * Function: Unified2ParseArgs(char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
static Unified2Config * Unified2ParseArgs(char *args, char *default_filename)
{
    Unified2Config *config = (Unified2Config *)SnortAlloc(sizeof(Unified2Config));

    /* This is so the if 'nostamps' option is used on the command line,
     * it will be honored by unified2, and only one variable is used. */
    config->nostamp = ScNoOutputTimestamp();

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Args: %s\n", args););

    if(args != NULL)
    {
        char **toks;
        int num_toks;
        int i = 0;
        toks = mSplit((char *)args, ",", 31, &num_toks, '\\');
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
                if(num_stoks > 1 && config->base_filename == NULL)
                    config->base_filename = SnortStrdup(stoks[1]);
                else
                    FatalError("Argument Error in %s(%i): %s\n",
                            file_name, file_line, index);
            }
            else if(strcasecmp("limit", stoks[0]) == 0)
            {
                char *end;

                if ((num_stoks > 1) && (config->limit == 0))
                {
                    config->limit = SnortStrtoul(stoks[1], &end, 10);
                    if ((stoks[1] == end) || (errno == ERANGE))
                    {
                        FatalError("Argument Error in %s(%i): %s\n",
                                   file_name, file_line, index);
                    }
                }
                else
                {
                    FatalError("Argument Error in %s(%i): %s\n",
                               file_name, file_line, index);
                }
            }
            else if(strcasecmp("nostamp", stoks[0]) == 0)
            {
                config->nostamp = 1;
            }
#ifdef MPLS
            else if(strcasecmp("mpls_event_types", stoks[0]) == 0)
            {
                config->mpls_event_types = 1;
            }
#endif
            else if(strcasecmp("vlan_event_types", stoks[0]) == 0)
            {
                config->vlan_event_types = 1;
#if defined(FEAT_OPEN_APPID)
            }
            else if(strcasecmp("appid_event_types", stoks[0]) == 0)
            {
                config->appid_event_types = 1;
#endif /* defined(FEAT_OPEN_APPID) */
            }
            else
            {
                FatalError("Argument Error in %s(%i): %s\n",
                        file_name, file_line, index);
            }

            mSplitFree(&stoks, num_stoks);
        }
        mSplitFree(&toks, num_toks);
    }

    if (config->base_filename == NULL)
        config->base_filename = SnortStrdup(default_filename);

    if (config->limit == 0)
    {
        config->limit = 128;
    }
    else if (config->limit > 512)
    {
        LogMessage("spo_unified2 %s(%d)=> Lowering limit of %iMB to 512MB\n",
            file_name, file_line, config->limit);
        config->limit = 512;
    }

    /* convert the limit to "MB" */
    config->limit <<= 20;

    return config;
}

/*
 * Function: Unified2CleanExitFunc()
 *
 * Purpose: Cleanup at exit time
 *
 * Arguments: signal => signal that caused this event
 *            arg => data ptr to reference this plugin's data
 *
 * Returns: void function
 */
static void Unified2CleanExit(int signal, void *arg)
{
    /* cast the arg pointer to the proper type */
    Unified2Config *config = (Unified2Config *)arg;

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "SpoUnified2: CleanExit\n"););

    /* free up initialized memory */
    if (config != NULL)
    {
        if (config->stream != NULL)
            fclose(config->stream);

        if (config->base_filename != NULL)
            free(config->base_filename);

        free(config);
    }
}

#ifdef SNORT_RELOAD
/*
 * Function: Reload()
 *
 * Purpose: For reloads (SIGHUP usually), over the output
 *
 * Arguments: signal => signal that caused this event
 *            arg => data ptr to reference this plugin's data
 *
 * Returns: void function
 */
static void Unified2Reload(struct _SnortConfig *sc, int signal, void *arg)
{
    Unified2Config *config = (Unified2Config *)arg;

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "SpoUnified2: Reload\n"););

    Unified2RotateFile(config);
}
#endif

/* Unified2 Alert functions (deprecated) */
static void Unified2AlertInit(struct _SnortConfig *sc, char *args)
{
    Unified2Config *config;
    int signal = 0;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: Unified2 Alert Initialized\n"););

    /* parse the argument list from the rules file */
    config = Unified2ParseArgs(args, "snort-unified.alert");

    alert_config = config;

    if(log_config && log_config->base_filename)
    {
        if(strcmp(config->base_filename, log_config->base_filename) == 0)
        {
            Unified2CleanExit(signal , (void *)log_config);
            Unified2CleanExit(signal, (void *)config);
            FatalError("Argument Error in %s(%i). Cannot reuse the filename in config option '%s'\n",
                                    file_name, file_line, "alert_unified2");
        }
    }

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, Unified2LogAlert, OUTPUT_TYPE__ALERT, config);
    AddFuncToCleanExitList(Unified2CleanExit, config);
#ifdef SNORT_RELOAD
    AddFuncToReloadList(Unified2Reload, config);
#endif
    AddFuncToPostConfigList(sc, Unified2PostConfig, config);
}

/* Unified2 Packet Log functions (deprecated) */
static void Unified2LogInit(struct _SnortConfig *sc, char *args)
{
    Unified2Config *config;
    int signal = 0;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: Unified2 Log Initialized\n"););

    /* parse the argument list from the rules file */
    config = Unified2ParseArgs(args, "snort-unified.log");

    log_config = config;

    if(alert_config && alert_config->base_filename)
    {
        if(strcmp(config->base_filename, alert_config->base_filename) == 0)
        {
            Unified2CleanExit(signal, (void *)alert_config);
            Unified2CleanExit(signal, (void *)config);
            FatalError("Argument Error in %s(%i). Cannot reuse the filename in config option '%s'\n",
                                    file_name, file_line, "log_unified2");
        }
    }

    //LogMessage("Unified2LogFilename = %s\n", Unified2Info->filename);

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, Unified2LogPacketAlert, OUTPUT_TYPE__LOG, config);
    AddFuncToCleanExitList(Unified2CleanExit, config);
#ifdef SNORT_RELOAD
    AddFuncToReloadList(Unified2Reload, config);
#endif
    AddFuncToPostConfigList(sc, Unified2PostConfig, config);
}

/******************************************************************************
 * Function: Unified2Write()
 *
 * Main function for writing to the unified2 file.
 *
 * For low level I/O errors, the current unified2 file is closed and a new
 * one created and a write to the new unified2 file is done.  It was found
 * that when writing to an NFS mounted share that is using a soft mount option,
 * writes sometimes fail and leave the unified2 file corrupted.  If the write
 * to the newly created unified2 file fails, Snort will fatal error.
 *
 * In the case of interrupt errors, the write is retried, but only for a
 * finite number of times.
 *
 * All other errors are treated as non-recoverable and Snort will fatal error.
 *
 * Upon successful completion of write, the length of the data written is
 * added to the current amount of total data written thus far to the
 * unified2 file.
 *
 * Arguments
 *  uint8_t *
 *      The buffer containing the data to write
 *  uint32_t
 *      The length of the data to write
 *  Unified2Config *
 *      A pointer to the unified2 configuration data
 *
 * Returns: None
 *
 ******************************************************************************/
static void Unified2Write(uint8_t *buf, uint32_t buf_len, Unified2Config *config)
{
    size_t fwcount = 0;
    int ffstatus = 0;

    /* Nothing to write or nothing to write to */
    if ((buf == NULL) || (config == NULL) || (config->stream == NULL))
        return;

    /* Don't use fsync().  It is a total performance killer */
    if (((fwcount = fwrite(buf, (size_t)buf_len, 1, config->stream)) != 1) ||
        ((ffstatus = fflush(config->stream)) != 0))
    {
        /* errno is saved just to avoid other intervening calls
         * (e.g. ErrorMessage) potentially reseting it to something else. */
        int error = errno;
        int max_retries = 3;

        /* On iterations other than the first, the only non-zero error will be
         * EINTR or interrupt.  Only iterate a maximum of max_retries times so
         * there is no chance of infinite looping if for some reason the write
         * is constantly interrupted */
        while ((error != 0) && (max_retries != 0))
        {
            if (config->nostamp)
            {
                ErrorMessage("%s(%d) Failed to write to unified2 file (%s): %s\n",
                             __FILE__, __LINE__, config->filepath, strerror(error));
            }
            else
            {
                ErrorMessage("%s(%d) Failed to write to unified2 file (%s.%u): %s\n",
                             __FILE__, __LINE__, config->filepath,
                             config->timestamp, strerror(error));
            }

            while ((error == EINTR) && (max_retries != 0))
            {
                max_retries--;

                /* Supposedly an interrupt can only occur before anything
                 * has been written.  Try again */
                ErrorMessage("%s(%d) Got interrupt. Retry write to unified2 "
                             "file.\n", __FILE__, __LINE__);

                if (fwcount != 1)
                {
                    /* fwrite() failed.  Redo fwrite and fflush */
                    if (((fwcount = fwrite(buf, (size_t)buf_len, 1, config->stream)) == 1) &&
                        ((ffstatus = fflush(config->stream)) == 0))
                    {
                        ErrorMessage("%s(%d) Write to unified2 file succeeded!\n",
                                     __FILE__, __LINE__);
                        error = 0;
                        break;
                    }
                }
                else if ((ffstatus = fflush(config->stream)) == 0)
                {
                    ErrorMessage("%s(%d) Write to unified2 file succeeded!\n",
                                 __FILE__, __LINE__);
                    error = 0;
                    break;
                }

                error = errno;

                ErrorMessage("%s(%d) Retrying write to unified2 file failed.\n",
                             __FILE__, __LINE__);
            }

            /* If we've reached the maximum number of interrupt retries,
             * just bail out of the main while loop */
            if (max_retries == 0)
                continue;

            switch (error)
            {
                case 0:
                    break;

                case EIO:
                    ErrorMessage("%s(%d) Unified2 file is possibly corrupt. "
                                 "Closing this unified2 file and creating "
                                 "a new one.\n", __FILE__, __LINE__);

                    Unified2RotateFile(config);

                    if (config->nostamp)
                    {
                        ErrorMessage("%s(%d) New unified2 file: %s\n",
                                     __FILE__, __LINE__, config->filepath);
                    }
                    else
                    {
                        ErrorMessage("%s(%d) New unified2 file: %s.%u\n",
                                     __FILE__, __LINE__,
                                     config->filepath, config->timestamp);
                    }

                    if (((fwcount = fwrite(buf, (size_t)buf_len, 1, config->stream)) == 1) &&
                        ((ffstatus = fflush(config->stream)) == 0))
                    {
                        ErrorMessage("%s(%d) Write to unified2 file succeeded!\n",
                                     __FILE__, __LINE__);
                        error = 0;
                        break;
                    }

                    error = errno;

                    /* Loop again if interrupt */
                    if (error == EINTR)
                        break;

                    /* Write out error message again, then fall through and fatal */
                    if (config->nostamp)
                    {
                        ErrorMessage("%s(%d) Failed to write to unified2 file (%s): %s\n",
                                     __FILE__, __LINE__, config->filepath, strerror(error));
                    }
                    else
                    {
                        ErrorMessage("%s(%d) Failed to write to unified2 file (%s.%u): %s\n",
                                     __FILE__, __LINE__, config->filepath,
                                     config->timestamp, strerror(error));
                    }

                    /* Fall through */

                case EAGAIN:  /* We're not in non-blocking mode */
                case EBADF:
                case EFAULT:
                case EFBIG:
                case EINVAL:
                case ENOSPC:
                case EPIPE:
                default:
                    FatalError("%s(%d) Cannot write to device.\n", __FILE__, __LINE__);
            }
        }

        if ((max_retries == 0) && (error != 0))
        {
            FatalError("%s(%d) Maximum number of interrupts exceeded. "
                       "Cannot write to device.\n", __FILE__, __LINE__);
        }
    }

    config->current += buf_len;
}

