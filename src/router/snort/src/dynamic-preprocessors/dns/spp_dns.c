/* $Id */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2006-2013 Sourcefire, Inc.
**
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


/*
 * DNS preprocessor
 * Author: Steven Sturges
 *
 *
 * Alert for DNS client rdata buffer overflow.
 * Alert for Obsolete or Experimental RData types (per RFC 1035)
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"

#include "preprocids.h"
#include "snort_debug.h"
#include "spp_dns.h"
#include "sf_preproc_info.h"

#include <assert.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <time.h> 

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats dnsPerfStats;
#endif

#include "sf_types.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"

#ifdef DUMP_BUFFER
#include "dns_buffer_dump.h"
#endif

#ifdef TARGET_BASED
int16_t dns_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 4;
const char *PREPROC_NAME = "SF_DNS";

#define SetupDNS DYNAMIC_PREPROC_SETUP

/*
 * Generator id. Define here the same as the official registry
 * in generators.h
 */
#define GENERATOR_SPP_DNS   131

/*
 * Function prototype(s)
 */
DNSSessionData * GetDNSSessionData(SFSnortPacket *, DNSConfig *);
static void DNSInit( struct _SnortConfig *, char* );
static void PrintDNSConfig(DNSConfig *);
static void FreeDNSSessionData( void* );
static void  ParseDNSArgs(DNSConfig *, u_char*);
static void ProcessDNS( void*, void* );
static inline int CheckDNSPort(DNSConfig *, uint16_t);
static void DNSReset(int, void *);
static void DNSResetStats(int, void *);
static void enablePortStreamServices(struct _SnortConfig *, DNSConfig *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static void DNSFreeConfig(tSfPolicyUserContextId config);
static int DNSCheckConfig(struct _SnortConfig *);
static void DNSCleanExit(int, void *);
int dns_print_mem_stats(FILE *fd, char* buffer, PreprocMemInfo *meminfo);

/* Ultimately calls SnortEventqAdd */
/* Arguments are: gid, sid, rev, classification, priority, message, rule_info */
#define DNS_ALERT(x,y) { _dpd.alertAdd(GENERATOR_SPP_DNS, x, 1, 0, 3, y, 0 ); }

/* Convert port value into an index for the dns_config ports array */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

#define DNS_RR_PTR 0xC0

static tSfPolicyUserContextId dns_config = NULL;
DNSConfig *dns_eval_config = NULL;

#ifdef SNORT_RELOAD
static void DNSReload(struct _SnortConfig *, char *, void **);
static int DNSReloadVerify(struct _SnortConfig *, void *);
static void * DNSReloadSwap(struct _SnortConfig *, void *);
static void DNSReloadSwapFree(void *);
#endif


/* Called at preprocessor setup time. Links preprocessor keyword
 * to corresponding preprocessor initialization function.
 *
 * PARAMETERS:  None.
 *
 * RETURNS: Nothing.
 *
 */
void SetupDNS(void)
{
    /* Link preprocessor keyword to initialization function
     * in the preprocessor list. */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc( "dns", DNSInit );
#else
    _dpd.registerPreproc("dns", DNSInit, DNSReload,
                         DNSReloadVerify, DNSReloadSwap, DNSReloadSwapFree);
#endif

#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getDNSBuffers, DNS_BUFFER_DUMP_FUNC);
#endif
    _dpd.registerMemoryStatsFunc(PP_DNS, dns_print_mem_stats);
}

#ifdef REG_TEST
static inline void PrintDNSSize(void)
{
    _dpd.logMsg("\nDNS Session Size: %lu\n", (long unsigned int)sizeof(DNSSessionData));
}
#endif

/* Initializes the DNS preprocessor module and registers
 * it in the preprocessor list.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to argument string to process for config
 *                      data.
 *
 * RETURNS:     Nothing.
 */
static void DNSInit( struct _SnortConfig *sc, char* argp )
{
    int policy_id = _dpd.getParserPolicy(sc);

    DNSConfig *pPolicyConfig = NULL;
#ifdef REG_TEST
    PrintDNSSize();
#endif

    if (dns_config == NULL)
    {
        //create a context
        dns_config = sfPolicyConfigCreate();
        if (dns_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                            "DNS configuration.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Dns preprocessor requires the "
                "stream5 preprocessor to be enabled.\n",
                *(_dpd.config_file), *(_dpd.config_line));
        }

        _dpd.addPreprocReset(DNSReset, NULL, PRIORITY_LAST, PP_DNS);
        _dpd.addPreprocResetStats(DNSResetStats, NULL, PRIORITY_LAST, PP_DNS);
        _dpd.addPreprocConfCheck(sc, DNSCheckConfig);
        _dpd.addPreprocExit(DNSCleanExit, NULL, PRIORITY_LAST, PP_DNS);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("dns", (void *)&dnsPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

#ifdef TARGET_BASED
        dns_app_id = _dpd.findProtocolReference("dns");
        if (dns_app_id == SFTARGET_UNKNOWN_PROTOCOL)
        {
            dns_app_id = _dpd.addProtocolReference("dns");
        }

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_DNS, dns_app_id );
#endif
    }

    sfPolicyUserPolicySet (dns_config, policy_id);
    pPolicyConfig = (DNSConfig *)sfPolicyUserDataGetCurrent(dns_config);
    if (pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Dns preprocessor can only "
            "be configured once.\n", *(_dpd.config_file), *(_dpd.config_line));
    }

    pPolicyConfig = (DNSConfig *)_dpd.snortAlloc(1, sizeof(DNSConfig),
                                                PP_DNS, PP_MEM_CATEGORY_CONFIG);
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                        "DNS configuration.\n");
    }

    sfPolicyUserDataSetCurrent(dns_config, pPolicyConfig);

    ParseDNSArgs(pPolicyConfig, (u_char *)argp);

    _dpd.addPreproc(sc, ProcessDNS, PRIORITY_APPLICATION, PP_DNS, PROTO_BIT__TCP | PROTO_BIT__UDP);
    enablePortStreamServices(sc, pPolicyConfig, policy_id);
#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

/* Parses and processes the configuration arguments
 * supplied in the DNS preprocessor rule.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to string containing the config arguments.
 *
 * RETURNS:     Nothing.
 */
static void ParseDNSArgs(DNSConfig *config, u_char* argp)
{
    char* cur_tokenp = NULL;
    char* argcpyp = NULL;
    int port;

    if (config == NULL)
        return;

    /* Set up default port to listen on */
    config->ports[ PORT_INDEX( DNS_PORT ) ] |= CONV_PORT(DNS_PORT);

    /* Sanity check(s) */
    if ( !argp )
    {
        PrintDNSConfig(config);
        return;
    }

    argcpyp = strdup( (char*) argp );

    if ( !argcpyp )
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse DNS options.\n");
        return;
    }

    cur_tokenp = strtok( argcpyp, " ");

    while ( cur_tokenp )
    {
        if ( !strcmp( cur_tokenp, DNS_PORTS_KEYWORD ))
        {
            /* If the user specified ports, remove 'DNS_PORT' for now since
             * it now needs to be set explicitely. */
            config->ports[ PORT_INDEX( DNS_PORT ) ] = 0;

            /* Eat the open brace. */
            cur_tokenp = strtok( NULL, " ");
            if (( !cur_tokenp ) || ( strcmp(cur_tokenp, "{" )))
            {
                DynamicPreprocessorFatalMessage("%s(%d) Bad value specified for %s.  Must start "
                                                "with '{' and be space separated.\n",
                                                *(_dpd.config_file), *(_dpd.config_line),
                                                DNS_PORTS_KEYWORD);
                //free(argcpyp);
                //return;
            }

            cur_tokenp = strtok( NULL, " ");
            while (( cur_tokenp ) && strcmp(cur_tokenp, "}" ))
            {
                if ( !isdigit( (int)cur_tokenp[0] ))
                {
                    DynamicPreprocessorFatalMessage("%s(%d) Bad port %s.\n",
                                                    *(_dpd.config_file), *(_dpd.config_line), cur_tokenp );
                    //free(argcpyp);
                    //return;
                }
                else
                {
                    port = atoi( cur_tokenp );
                    if( port < 0 || port > MAX_PORTS )
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) Port value illegitimate: %s\n",
                                                        *(_dpd.config_file), *(_dpd.config_line),
                                                        cur_tokenp );
                        //free(argcpyp);
                        //return;
                    }

                    config->ports[ PORT_INDEX( port ) ] |= CONV_PORT(port);
                }

                cur_tokenp = strtok( NULL, " ");
            }
        }
        else if ( !strcmp( cur_tokenp, DNS_ENABLE_RDATA_OVERFLOW_KEYWORD ))
        {
            config->enabled_alerts |= DNS_ALERT_RDATA_OVERFLOW;
        }
        else if ( !strcmp( cur_tokenp, DNS_ENABLE_OBSOLETE_TYPES_KEYWORD ))
        {
            config->enabled_alerts |= DNS_ALERT_OBSOLETE_TYPES;
        }
        else if ( !strcmp( cur_tokenp, DNS_ENABLE_EXPERIMENTAL_TYPES_KEYWORD ))
        {
            config->enabled_alerts |= DNS_ALERT_EXPERIMENTAL_TYPES;
        }
#if 0
        else if ( !strcmp( cur_tokenp, DNS_AUTODETECT_KEYWORD ))
        {
            config->autodetect++;
        }
#endif
        else
        {
            DynamicPreprocessorFatalMessage("Invalid argument: %s\n", cur_tokenp);
            return;
        }

        cur_tokenp = strtok( NULL, " " );
    }

    PrintDNSConfig(config);
    free(argcpyp);
}

/* Display the configuration for the DNS preprocessor.
 *
 * PARAMETERS:  None.
 *
 * RETURNS: Nothing.
 */
static void PrintDNSConfig(DNSConfig *config)
{
    int index;

    if (config == NULL)
        return;

    _dpd.logMsg("DNS config: \n");
#if 0
    _dpd.logMsg("    Autodetection: %s\n",
        config->autodetect ?
        "ENABLED":"DISABLED");
#endif
    _dpd.logMsg("    DNS Client rdata txt Overflow Alert: %s\n",
        config->enabled_alerts & DNS_ALERT_RDATA_OVERFLOW ?
        "ACTIVE" : "INACTIVE" );
    _dpd.logMsg("    Obsolete DNS RR Types Alert: %s\n",
        config->enabled_alerts & DNS_ALERT_OBSOLETE_TYPES ?
        "ACTIVE" : "INACTIVE" );
    _dpd.logMsg("    Experimental DNS RR Types Alert: %s\n",
        config->enabled_alerts & DNS_ALERT_EXPERIMENTAL_TYPES ?
        "ACTIVE" : "INACTIVE" );

    /* Printing ports */
    _dpd.logMsg("    Ports:");
    for(index = 0; index < MAX_PORTS; index++)
    {
        if( config->ports[ PORT_INDEX(index) ] & CONV_PORT(index) )
        {
            _dpd.logMsg(" %d", index);
        }
    }
    _dpd.logMsg("\n");
}

int dns_print_mem_stats(FILE *fd, char* buffer, PreprocMemInfo *meminfo)
{
    time_t curr_time = time(NULL);
    int len = 0;
    size_t total_heap_memory = meminfo[PP_MEM_CATEGORY_SESSION].used_memory 
                              + meminfo[PP_MEM_CATEGORY_CONFIG].used_memory; 
    if (fd)
    {
        len = fprintf(fd, ",%lu,%u,%u,%lu,%u,%u,%lu"
                       , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
                       , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
                       , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
                       , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
                       , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
                       , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
                       , total_heap_memory);
       return len;
    }
    if (buffer)
    {
        /*
         * No stats apart from the one by the Infra
         */  
        len = snprintf(buffer, CS_STATS_BUF_SIZE,
                       "\n\nMemory Statistics for DNS at: %s\n"
                       "DNS Preprocessor Statistics:\n"
                       , ctime(&curr_time));
    }
    else 
    {
        /*
         * No stats apart from the one by the Infra
         */  
        _dpd.logMsg("\n");
        _dpd.logMsg("Memory Statistics of DNS at: %s\n",
                    ctime(&curr_time));
    }
    return len;
}

/* Retrieves the DNS data block registered with the stream
 * session associated w/ the current packet. If none exists,
 * allocates it and registers it with the stream API.
 *
 * PARAMETERS:
 *
 * p: Pointer to the packet from which/in which to
 *      retrieve/store the DNS data block.
 *
 * RETURNS: Pointer to an DNS data block, upon success.
 *      NULL, upon failure.
 */
static DNSSessionData udpSessionData;
#define MIN_UDP_PAYLOAD 0x1FFF
DNSSessionData * GetDNSSessionData(SFSnortPacket *p, DNSConfig *config)
{
    DNSSessionData* dnsSessionData = NULL;

    if (config == NULL)
        return NULL;

    if (p->udp_header)
    {
        if (!(config->enabled_alerts & DNS_ALERT_OBSOLETE_TYPES) &&
            !(config->enabled_alerts & DNS_ALERT_EXPERIMENTAL_TYPES))
        {
            if (config->enabled_alerts & DNS_ALERT_RDATA_OVERFLOW)
            {
                /* Checking RData Overflow... */
                if (p->payload_size <
                     (sizeof(DNSHdr) + sizeof(DNSRR) + MIN_UDP_PAYLOAD))
                {
                    /* But we don't have sufficient data.  Go away. */
                    return NULL;
                }
            }
            else
            {
                /* Not checking for experimental or obsolete types. Go away. */
                return NULL;
            }
        }

        /* Its a UDP packet, use the "stateless" one */
        dnsSessionData = &udpSessionData;
        memset(dnsSessionData, 0, sizeof(DNSSessionData));
        return dnsSessionData;
    }

    /* More Sanity check(s) */
    if ( !p->stream_session )
    {
        return NULL;
    }

    dnsSessionData = _dpd.snortAlloc(1, sizeof(DNSSessionData), 
                                     PP_DNS, PP_MEM_CATEGORY_SESSION);

    if ( !dnsSessionData )
        return NULL;

    /*Register the new DNS data block in the stream session. */
    _dpd.sessionAPI->set_application_data(
        p->stream_session,
        PP_DNS, dnsSessionData, FreeDNSSessionData );

    return dnsSessionData;
}

/* Registered as a callback with the DNS data when they are
 * added to the stream session. Called by stream when a
 * session is about to be destroyed to free that data.
 *
 * PARAMETERS:
 *
 * application_data:  Pointer to the DNS data
 *
 * RETURNS: Nothing.
 */
static void FreeDNSSessionData( void* application_data )
{
    DNSSessionData* dnsSessionData = (DNSSessionData*)application_data;
    if ( dnsSessionData )
    {
        _dpd.snortFree(dnsSessionData, sizeof(DNSSessionData),
                           PP_DNS, PP_MEM_CATEGORY_SESSION);
    }
}

/* Validates given port as an DNS server port.
 *
 * PARAMETERS:
 *
 * port:    Port to validate.
 *
 * RETURNS: DNS_TRUE, if the port is indeed an DNS server port.
 *      DNS_FALSE, otherwise.
 */
static inline int CheckDNSPort(DNSConfig *config, uint16_t port)
{
    return config->ports[PORT_INDEX(port)] & CONV_PORT(port);
}

static uint16_t ParseDNSHeader(const unsigned char *data,
                                uint16_t bytes_unused,
                                DNSSessionData *dnsSessionData)
{
    if (bytes_unused == 0)
    {
        return bytes_unused;
    }

    switch (dnsSessionData->state)
    {
    case DNS_RESP_STATE_LENGTH:
        /* First two bytes are length in TCP */
        dnsSessionData->length = ((uint8_t)*data) << 8;
        dnsSessionData->state = DNS_RESP_STATE_LENGTH_PART;
        data++;
        bytes_unused--;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_LENGTH_PART:
        dnsSessionData->length |= ((uint8_t)*data);
        dnsSessionData->state = DNS_RESP_STATE_HDR_ID;
        data++;
        bytes_unused--;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_ID:
        dnsSessionData->hdr.id = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_ID_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_ID_PART:
        dnsSessionData->hdr.id |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_FLAGS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_FLAGS:
        dnsSessionData->hdr.flags = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_FLAGS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_FLAGS_PART:
        dnsSessionData->hdr.flags |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_QS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_QS:
        dnsSessionData->hdr.questions = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_QS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_QS_PART:
        dnsSessionData->hdr.questions |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_ANSS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_ANSS:
        dnsSessionData->hdr.answers = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_ANSS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_ANSS_PART:
        dnsSessionData->hdr.answers |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_AUTHS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_AUTHS:
        dnsSessionData->hdr.authorities = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_AUTHS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_AUTHS_PART:
        dnsSessionData->hdr.authorities |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_ADDS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_ADDS:
        dnsSessionData->hdr.additionals = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_HDR_ADDS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_HDR_ADDS_PART:
        dnsSessionData->hdr.additionals |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->state = DNS_RESP_STATE_QUESTION;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    default:
        /* Continue -- we're beyond the header */
        break;
    }

    return bytes_unused;
}


uint16_t ParseDNSName(const unsigned char *data,
                       uint16_t bytes_unused,
                       DNSSessionData *dnsSessionData)
{
    uint16_t bytes_required = dnsSessionData->curr_txt.txt_len - dnsSessionData->curr_txt.txt_bytes_seen;

    while (dnsSessionData->curr_txt.name_state != DNS_RESP_STATE_NAME_COMPLETE)
    {
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }

        switch (dnsSessionData->curr_txt.name_state)
        {
        case DNS_RESP_STATE_NAME_SIZE:
            dnsSessionData->curr_txt.txt_len = (uint8_t)*data;
            data++;
            bytes_unused--;
            dnsSessionData->bytes_seen_curr_rec++;
            if (dnsSessionData->curr_txt.txt_len == 0)
            {
                dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_NAME_COMPLETE;
                return bytes_unused;
            }

            dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_NAME;
            dnsSessionData->curr_txt.txt_bytes_seen = 0;

            if ((dnsSessionData->curr_txt.txt_len & DNS_RR_PTR) == DNS_RR_PTR)
            {
                /* A reference to another location... */
                /* This is an offset */
                dnsSessionData->curr_txt.offset = (dnsSessionData->curr_txt.txt_len & ~0xC0) << 8;
                bytes_required = dnsSessionData->curr_txt.txt_len = 1;
                dnsSessionData->curr_txt.relative = 1;
                /* Setup to read 2nd Byte of Location */
            }
            else
            {
                bytes_required = dnsSessionData->curr_txt.txt_len;
                dnsSessionData->curr_txt.offset = 0;
                dnsSessionData->curr_txt.relative = 0;
            }

            if (bytes_unused == 0)
            {
                return bytes_unused;
            }

            /* Fall through */
        case DNS_RESP_STATE_NAME:
            if (bytes_required <= bytes_unused)
            {
                bytes_unused -= bytes_required;
                if (dnsSessionData->curr_txt.relative)
                {
                    /* If this one is a relative offset, read that extra byte */
                    dnsSessionData->curr_txt.offset |= *data;
                }
                data += bytes_required;
                dnsSessionData->bytes_seen_curr_rec += bytes_required;
                dnsSessionData->curr_txt.txt_bytes_seen += bytes_required;

                if (bytes_unused == 0)
                {
                    return bytes_unused;
                }
            }
            else
            {
                dnsSessionData->bytes_seen_curr_rec+= bytes_unused;
                dnsSessionData->curr_txt.txt_bytes_seen += bytes_unused;
                return 0;
            }
            if (dnsSessionData->curr_txt.relative)
            {
                /* And since its relative, we're done */
                dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_NAME_COMPLETE;
                return bytes_unused;
            }
            break;
        }

        /* Go to the next portion of the name */
        dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_NAME_SIZE;
    }

    return bytes_unused;
}

static uint16_t ParseDNSQuestion(const unsigned char *data,
                                  uint16_t data_size,
                                  uint16_t bytes_unused,
                                  DNSSessionData *dnsSessionData)
{
    uint16_t bytes_used = 0;
    uint16_t new_bytes_unused = 0;

    if (bytes_unused == 0)
    {
        return bytes_unused;
    }

    if (dnsSessionData->curr_rec_state < DNS_RESP_STATE_Q_NAME_COMPLETE)
    {
        new_bytes_unused = ParseDNSName(data, bytes_unused, dnsSessionData);
        bytes_used = bytes_unused - new_bytes_unused;

#ifdef DUMP_BUFFER
        dumpBuffer(DNS_QUESTION_DUMP,data,bytes_used);
#endif
        if (dnsSessionData->curr_txt.name_state == DNS_RESP_STATE_NAME_COMPLETE)
        {
            dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_TYPE;
            memset(&dnsSessionData->curr_txt, 0, sizeof(DNSNameState));
            data = data + bytes_used;
            bytes_unused = new_bytes_unused;

            if (bytes_unused == 0)
            {
                /* ran out of data */
                return bytes_unused;
            }
        }
        else
        {
            /* Should be 0 -- ran out of data */
            return new_bytes_unused;
        }
    }

    switch (dnsSessionData->curr_rec_state)
    {
    case DNS_RESP_STATE_Q_TYPE:
        dnsSessionData->curr_q.type = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_TYPE_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_Q_TYPE_PART:
        dnsSessionData->curr_q.type |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_CLASS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_Q_CLASS:
        dnsSessionData->curr_q.dns_class = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_CLASS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_Q_CLASS_PART:
        dnsSessionData->curr_q.dns_class |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_COMPLETE;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    default:
        /* Continue -- we're beyond this question */
        break;
    }

    return bytes_unused;
}

uint16_t ParseDNSAnswer(const unsigned char *data,
                         uint16_t data_size,
                         uint16_t bytes_unused,
                         DNSSessionData *dnsSessionData)
{
    uint16_t bytes_used = 0;
    uint16_t new_bytes_unused = 0;

    if (bytes_unused == 0)
    {
        return bytes_unused;
    }

    if (dnsSessionData->curr_rec_state < DNS_RESP_STATE_RR_NAME_COMPLETE)
    {
        new_bytes_unused = ParseDNSName(data, bytes_unused, dnsSessionData);
        bytes_used = bytes_unused - new_bytes_unused;

#ifdef DUMP_BUFFER
        dumpBuffer(DNS_ANSWER_DUMP,data,bytes_used);
#endif
        if (dnsSessionData->curr_txt.name_state == DNS_RESP_STATE_NAME_COMPLETE)
        {
            dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_TYPE;
            memset(&dnsSessionData->curr_txt, 0, sizeof(DNSNameState));
            data = data + bytes_used;
        }
        bytes_unused = new_bytes_unused;

        if (bytes_unused == 0)
        {
            /* ran out of data */
            return bytes_unused;
        }
    }

    switch (dnsSessionData->curr_rec_state)
    {
    case DNS_RESP_STATE_RR_TYPE:
        dnsSessionData->curr_rr.type = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_TYPE_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_RR_TYPE_PART:
        dnsSessionData->curr_rr.type |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_CLASS;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_RR_CLASS:
        dnsSessionData->curr_rr.dns_class = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_CLASS_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_RR_CLASS_PART:
        dnsSessionData->curr_rr.dns_class |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_TTL;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_RR_TTL:
        dnsSessionData->curr_rr.ttl = (uint8_t)*data << 24;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_TTL_PART;
        dnsSessionData->bytes_seen_curr_rec = 1;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_RR_TTL_PART:
        while (dnsSessionData->bytes_seen_curr_rec < 4)
        {
            dnsSessionData->bytes_seen_curr_rec++;
            dnsSessionData->curr_rr.ttl |=
                (uint8_t)*data << (4-dnsSessionData->bytes_seen_curr_rec)*8;
            data++;
            bytes_unused--;
            if (bytes_unused == 0)
            {
                return bytes_unused;
            }
        }
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_RDLENGTH;
        /* Fall through */
    case DNS_RESP_STATE_RR_RDLENGTH:
        dnsSessionData->curr_rr.length = (uint8_t)*data << 8;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_RDLENGTH_PART;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    case DNS_RESP_STATE_RR_RDLENGTH_PART:
        dnsSessionData->curr_rr.length |= (uint8_t)*data;
        data++;
        bytes_unused--;
        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_RDATA_START;
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }
        /* Fall through */
    default:
        /* Continue -- we're beyond this answer */
        break;
    }

    return bytes_unused;
}

/* The following check is to look for an attempt to exploit
 * a vulnerability in the DNS client, per MS 06-041.
 *
 * For details, see:
 * http://www.microsoft.com/technet/security/bulletin/ms06-007.mspx
 * http://cve.mitre.org/cgi-bin/cvename.cgi?name=2006-3441
 *
 * Vulnerability Research by Lurene Grenier, Judy Novak,
 * and Brian Caswell.
 */
uint16_t CheckRRTypeTXTVuln(const unsigned char *data,
                       uint16_t bytes_unused,
                       DNSSessionData *dnsSessionData)
{
    uint16_t bytes_required = dnsSessionData->curr_txt.txt_len - dnsSessionData->curr_txt.txt_bytes_seen;

    while (dnsSessionData->curr_txt.name_state != DNS_RESP_STATE_RR_NAME_COMPLETE)
    {
        if (dnsSessionData->bytes_seen_curr_rec == dnsSessionData->curr_rr.length)
        {
            /* Done with the name */
            dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_RR_NAME_COMPLETE;
            /* Got to the end of the rdata in this packet! */
            dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_COMPLETE;
            return bytes_unused;
        }
#ifdef DUMP_BUFFER
        dumpBuffer(DNS_RR_TYPE_TXT_VULN_DUMP,data,bytes_unused);
#endif
        if (bytes_unused == 0)
        {
            return bytes_unused;
        }

        switch (dnsSessionData->curr_txt.name_state)
        {
        case DNS_RESP_STATE_RR_NAME_SIZE:
            dnsSessionData->curr_txt.txt_len = (uint8_t)*data;
            dnsSessionData->curr_txt.txt_count++;
            dnsSessionData->curr_txt.total_txt_len += dnsSessionData->curr_txt.txt_len + 1; /* include the NULL */

            if (!dnsSessionData->curr_txt.alerted)
            {
                uint32_t overflow_check = (dnsSessionData->curr_txt.txt_count * 4) +
                                           (dnsSessionData->curr_txt.total_txt_len * 2) + 4;
                /* if txt_count * 4 + total_txt_len * 2 + 4 > FFFF, vulnerability! */
                if (overflow_check > 0xFFFF)
                {
                    if (dns_eval_config->enabled_alerts & DNS_ALERT_RDATA_OVERFLOW)
                    {
                        /* Alert on obsolete DNS RR types */
                        DNS_ALERT(DNS_EVENT_RDATA_OVERFLOW, DNS_EVENT_RDATA_OVERFLOW_STR);
                    }

                    dnsSessionData->curr_txt.alerted = 1;
                }
            }

            data++;
            bytes_unused--;
            dnsSessionData->bytes_seen_curr_rec++;
            if (dnsSessionData->curr_txt.txt_len > 0)
            {
                dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_RR_NAME;
                dnsSessionData->curr_txt.txt_bytes_seen = 0;
                bytes_required = dnsSessionData->curr_txt.txt_len;
            }
            else
            {
                continue;
            }
            if (bytes_unused == 0)
            {
                return bytes_unused;
            }
            /* Fall through */
        case DNS_RESP_STATE_RR_NAME:
            if (bytes_required <= bytes_unused)
            {
                bytes_unused -= bytes_required;
                dnsSessionData->bytes_seen_curr_rec += bytes_required;
                data += bytes_required;
                dnsSessionData->curr_txt.txt_bytes_seen += bytes_required;
                if (bytes_unused == 0)
                {
                    return bytes_unused;
                }
            }
            else
            {
                dnsSessionData->curr_txt.txt_bytes_seen += bytes_unused;
                dnsSessionData->bytes_seen_curr_rec += bytes_unused;
                return 0;
            }
            break;
        }

        /* Go to the next portion of the name */
        dnsSessionData->curr_txt.name_state = DNS_RESP_STATE_RR_NAME_SIZE;
    }

    return bytes_unused;
}

uint16_t SkipDNSRData(const unsigned char *data,
                       uint16_t bytes_unused,
                       DNSSessionData *dnsSessionData)
{
    uint16_t bytes_required = dnsSessionData->curr_rr.length - dnsSessionData->bytes_seen_curr_rec;

    if (bytes_required <= bytes_unused)
    {
        bytes_unused -= bytes_required;
        data += bytes_required;
        dnsSessionData->bytes_seen_curr_rec += bytes_required;
    }
    else
    {
        dnsSessionData->bytes_seen_curr_rec += bytes_unused;
        return 0;
    }

    /* Got to the end of the rdata in this packet! */
    dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_COMPLETE;
    return bytes_unused;
}

uint16_t ParseDNSRData(SFSnortPacket *p,
                        const unsigned char *data,
                        uint16_t bytes_unused,
                        DNSSessionData *dnsSessionData)
{
    if (bytes_unused == 0)
    {
        return bytes_unused;
    }

    switch (dnsSessionData->curr_rr.type)
    {
    case DNS_RR_TYPE_TXT:
        /* Check for RData Overflow */
        bytes_unused = CheckRRTypeTXTVuln(data, bytes_unused, dnsSessionData);
        break;

    case DNS_RR_TYPE_MD:
    case DNS_RR_TYPE_MF:
        if (dns_eval_config->enabled_alerts & DNS_ALERT_OBSOLETE_TYPES)
        {
            /* Alert on obsolete DNS RR types */
            DNS_ALERT(DNS_EVENT_OBSOLETE_TYPES, DNS_EVENT_OBSOLETE_TYPES_STR);
        }
        bytes_unused = SkipDNSRData(data, bytes_unused, dnsSessionData);
#ifdef DUMP_BUFFER
        dumpBuffer(DNS_OBSOLETE_TYPES_DUMP,data,bytes_unused);
#endif
        break;

    case DNS_RR_TYPE_MB:
    case DNS_RR_TYPE_MG:
    case DNS_RR_TYPE_MR:
    case DNS_RR_TYPE_NULL:
    case DNS_RR_TYPE_MINFO:
        if (dns_eval_config->enabled_alerts & DNS_ALERT_EXPERIMENTAL_TYPES)
        {
            /* Alert on experimental DNS RR types */
            DNS_ALERT(DNS_EVENT_EXPERIMENTAL_TYPES, DNS_EVENT_EXPERIMENTAL_TYPES_STR);
        }
        bytes_unused = SkipDNSRData(data, bytes_unused, dnsSessionData);
#ifdef DUMP_BUFFER
        dumpBuffer(DNS_EXPERIMENTAL_TYPES_DUMP,data,bytes_unused);
#endif
        break;
    case DNS_RR_TYPE_A:
    case DNS_RR_TYPE_NS:
    case DNS_RR_TYPE_CNAME:
    case DNS_RR_TYPE_SOA:
    case DNS_RR_TYPE_WKS:
    case DNS_RR_TYPE_PTR:
    case DNS_RR_TYPE_HINFO:
    case DNS_RR_TYPE_MX:
        bytes_unused = SkipDNSRData(data, bytes_unused, dnsSessionData);
#ifdef DUMP_BUFFER
        dumpBuffer(DNS_SKIP_RDATA_DUMP,data,bytes_unused);
#endif
        break;
    default:
        /* Not one of the known types.  Stop looking at this session
         * as DNS. */
        dnsSessionData->flags |= DNS_FLAG_NOT_DNS;
        break;
    }

    return bytes_unused;
}

void ParseDNSResponseMessage(SFSnortPacket *p, DNSSessionData *dnsSessionData)
{
    uint16_t bytes_unused = p->payload_size;
    int i;
    const unsigned char *data = p->payload;

    while (bytes_unused)
    {
        /* Parse through the DNS Header */
        if (dnsSessionData->state < DNS_RESP_STATE_QUESTION)
        {
            /* Length only applies on a TCP packet, skip to header ID
             * if at beginning of a UDP Response.
             */
            if ((dnsSessionData->state == DNS_RESP_STATE_LENGTH) &&
                (p->udp_header))
            {
                dnsSessionData->state = DNS_RESP_STATE_HDR_ID;
            }
            bytes_unused = ParseDNSHeader(data, bytes_unused, dnsSessionData);
            if (bytes_unused > 0)
            {
                data = p->payload + (p->payload_size - bytes_unused);
            }
            else
            {
                /* No more data */
                return;
            }

            dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_NAME;
            dnsSessionData->curr_rec = 0;
        }

        /* Print out the header (but only once -- when we're ready to parse the Questions */
#ifdef DEBUG_MSGS
        if ((dnsSessionData->curr_rec_state == DNS_RESP_STATE_Q_NAME) &&
            (dnsSessionData->curr_rec == 0))
        {
            DebugMessage(DEBUG_DNS,
                            "DNS Header: length %d, id 0x%x, flags 0x%x, "
                            "questions %d, answers %d, authorities %d, additionals %d\n",
                            dnsSessionData->length, dnsSessionData->hdr.id,
                            dnsSessionData->hdr.flags, dnsSessionData->hdr.questions,
                            dnsSessionData->hdr.answers,
                            dnsSessionData->hdr.authorities,
                            dnsSessionData->hdr.additionals);
        }
#endif

        if (!(dnsSessionData->hdr.flags & DNS_HDR_FLAG_RESPONSE))
        {
            /* Not a response */
            return;
        }

        /* Handle the DNS Queries */
        if (dnsSessionData->state == DNS_RESP_STATE_QUESTION)
        {
            /* Skip over the 4 byte question records... */
            for (i=dnsSessionData->curr_rec; i< dnsSessionData->hdr.questions; i++)
            {
                bytes_unused = ParseDNSQuestion(data, p->payload_size, bytes_unused, dnsSessionData);

                if (dnsSessionData->curr_rec_state == DNS_RESP_STATE_Q_COMPLETE)
                {
                    DEBUG_WRAP(
                        DebugMessage(DEBUG_DNS,
                            "DNS Question %d: type %d, class %d\n",
                            i, dnsSessionData->curr_q.type,
                            dnsSessionData->curr_q.dns_class);
                            );
                    dnsSessionData->curr_rec_state = DNS_RESP_STATE_Q_NAME;
                    dnsSessionData->curr_rec++;
                }
                if (bytes_unused > 0)
                {
                    data = p->payload + (p->payload_size - bytes_unused);
                }
                else
                {
                    /* No more data */
                    return;
                }
            }
            dnsSessionData->state = DNS_RESP_STATE_ANS_RR;
            dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_NAME_SIZE;
            dnsSessionData->curr_rec = 0;
        }

        /* Handle the RRs */
        switch (dnsSessionData->state)
        {
        case DNS_RESP_STATE_ANS_RR: /* ANSWERS section */
            for (i=dnsSessionData->curr_rec; i<dnsSessionData->hdr.answers; i++)
            {
                bytes_unused = ParseDNSAnswer(data, p->payload_size,
                                                bytes_unused, dnsSessionData);
#ifdef DUMP_BUFFER
                dumpBuffer(DNS_RESP_STATE_ANS_RR_DUMP,data,bytes_unused);
#endif
                if (bytes_unused == 0)
                {
                    /* No more data */
                    return;
                }

                switch (dnsSessionData->curr_rec_state)
                {
                case DNS_RESP_STATE_RR_RDATA_START:
                    DEBUG_WRAP(
                        DebugMessage(DEBUG_DNS,
                                    "DNS ANSWER RR %d: type %d, class %d, "
                                    "ttl %d rdlength %d\n", i,
                                    dnsSessionData->curr_rr.type,
                                    dnsSessionData->curr_rr.dns_class,
                                    dnsSessionData->curr_rr.ttl,
                                    dnsSessionData->curr_rr.length);
                            );

                    dnsSessionData->bytes_seen_curr_rec = 0;
                    dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_RDATA_MID;
                    /* Fall through */
                case DNS_RESP_STATE_RR_RDATA_MID:
                    /* Data now points to the beginning of the RDATA */
                    data = p->payload + (p->payload_size - bytes_unused);
                    bytes_unused = ParseDNSRData(p, data, bytes_unused, dnsSessionData);
                    if (dnsSessionData->curr_rec_state != DNS_RESP_STATE_RR_COMPLETE)
                    {
                        /* Out of data, pick up on the next packet */
                        return;
                    }
                    else
                    {
                        /* Go to the next record */
                        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_NAME_SIZE;
                        dnsSessionData->curr_rec++;

                        if (dnsSessionData->curr_rr.type == DNS_RR_TYPE_TXT)
                        {
                            /* Reset the state tracking for this record */
                            memset(&dnsSessionData->curr_txt, 0, sizeof(DNSNameState));
                        }
                        data = p->payload + (p->payload_size - bytes_unused);
                    }
                }
            }
            dnsSessionData->state = DNS_RESP_STATE_AUTH_RR;
            dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_NAME_SIZE;
            dnsSessionData->curr_rec = 0;
            /* Fall through */
        case DNS_RESP_STATE_AUTH_RR: /* AUTHORITIES section */
            for (i=dnsSessionData->curr_rec; i<dnsSessionData->hdr.authorities; i++)
            {
                bytes_unused = ParseDNSAnswer(data, p->payload_size,
                                                bytes_unused, dnsSessionData);
#ifdef DUMP_BUFFER
                dumpBuffer(DNS_RESP_STATE_AUTH_RR_DUMP,data,bytes_unused);
#endif
                if (bytes_unused == 0)
                {
                    /* No more data */
                    return;
                }

                switch (dnsSessionData->curr_rec_state)
                {
                case DNS_RESP_STATE_RR_RDATA_START:
                    DEBUG_WRAP(
                        DebugMessage(DEBUG_DNS,
                                    "DNS AUTH RR %d: type %d, class %d, "
                                    "ttl %d rdlength %d\n", i,
                                    dnsSessionData->curr_rr.type,
                                    dnsSessionData->curr_rr.dns_class,
                                    dnsSessionData->curr_rr.ttl,
                                    dnsSessionData->curr_rr.length);
                            );

                    dnsSessionData->bytes_seen_curr_rec = 0;
                    dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_RDATA_MID;
                    /* Fall through */
                case DNS_RESP_STATE_RR_RDATA_MID:
                    /* Data now points to the beginning of the RDATA */
                    data = p->payload + (p->payload_size - bytes_unused);
                    bytes_unused = ParseDNSRData(p, data, bytes_unused, dnsSessionData);
                    if (dnsSessionData->curr_rec_state != DNS_RESP_STATE_RR_COMPLETE)
                    {
                        /* Out of data, pick up on the next packet */
                        return;
                    }
                    else
                    {
                        /* Go to the next record */
                        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_NAME_SIZE;
                        dnsSessionData->curr_rec++;

                        if (dnsSessionData->curr_rr.type == DNS_RR_TYPE_TXT)
                        {
                            /* Reset the state tracking for this record */
                            memset(&dnsSessionData->curr_txt, 0, sizeof(DNSNameState));
                        }
                        data = p->payload + (p->payload_size - bytes_unused);
                    }
                }
            }
            dnsSessionData->state = DNS_RESP_STATE_ADD_RR;
            dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_NAME_SIZE;
            dnsSessionData->curr_rec = 0;
            /* Fall through */
        case DNS_RESP_STATE_ADD_RR: /* ADDITIONALS section */
            for (i=dnsSessionData->curr_rec; i<dnsSessionData->hdr.authorities; i++)
            {
                bytes_unused = ParseDNSAnswer(data, p->payload_size,
                                                bytes_unused, dnsSessionData);
#ifdef DUMP_BUFFER
                dumpBuffer(DNS_RESP_STATE_ADD_RR_DUMP,data,bytes_unused);
#endif
                if (bytes_unused == 0)
                {
                    /* No more data */
                    return;
                }

                switch (dnsSessionData->curr_rec_state)
                {
                case DNS_RESP_STATE_RR_RDATA_START:
                    DEBUG_WRAP(
                        DebugMessage(DEBUG_DNS,
                                    "DNS ADDITONAL RR %d: type %d, class %d, "
                                    "ttl %d rdlength %d\n", i,
                                    dnsSessionData->curr_rr.type,
                                    dnsSessionData->curr_rr.dns_class,
                                    dnsSessionData->curr_rr.ttl,
                                    dnsSessionData->curr_rr.length);
                            );

                    dnsSessionData->bytes_seen_curr_rec = 0;
                    dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_RDATA_MID;
                    /* Fall through */
                case DNS_RESP_STATE_RR_RDATA_MID:
                    /* Data now points to the beginning of the RDATA */
                    data = p->payload + (p->payload_size - bytes_unused);
                    bytes_unused = ParseDNSRData(p, data, bytes_unused, dnsSessionData);
                    if (dnsSessionData->curr_rec_state != DNS_RESP_STATE_RR_COMPLETE)
                    {
                        /* Out of data, pick up on the next packet */
                        return;
                    }
                    else
                    {
                        /* Go to the next record */
                        dnsSessionData->curr_rec_state = DNS_RESP_STATE_RR_NAME_SIZE;
                        dnsSessionData->curr_rec++;

                        if (dnsSessionData->curr_rr.type == DNS_RR_TYPE_TXT)
                        {
                            /* Reset the state tracking for this record */
                            memset(&dnsSessionData->curr_txt, 0, sizeof(DNSNameState));
                        }
                        data = p->payload + (p->payload_size - bytes_unused);
                    }
                }
            }
            /* Done with this one, onto the next -- may also be in this packet */
            dnsSessionData->state = DNS_RESP_STATE_LENGTH;
            dnsSessionData->curr_rec_state = 0;
            dnsSessionData->curr_rec = 0;
        }
    }

    return;
}

/* Main runtime entry point for DNS preprocessor.
 * Analyzes DNS packets for anomalies/exploits.
 *
 * PARAMETERS:
 *
 * p:           Pointer to current packet to process.
 * context:     Pointer to context block, not used.
 *
 * RETURNS:     Nothing.
 */
static void ProcessDNS( void* packetPtr, void* context )
{
    DNSSessionData* dnsSessionData = NULL;
    uint8_t src = 0;
    uint8_t dst = 0;
    uint8_t known_port = 0;
    uint8_t direction = 0;
    SFSnortPacket* p;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif
    DNSConfig *config = NULL;
    PROFILE_VARS;

    sfPolicyUserPolicySet (dns_config, _dpd.getNapRuntimePolicy());
    config = (DNSConfig *)sfPolicyUserDataGetCurrent(dns_config);

    if (config == NULL)
        return;

#ifdef DUMP_BUFFER
    dumpBufferInit();
#endif

    dns_eval_config = config;

    p = (SFSnortPacket*) packetPtr;

    // preconditions - what we registered for
    assert((IsUDP(p) || IsTCP(p)) && p->payload_size && p->payload);

    /* Attempt to get a previously allocated DNS block. If none exists,
     * allocate and register one with the stream layer. */
    dnsSessionData = _dpd.sessionAPI->get_application_data(
        p->stream_session, PP_DNS );

#ifdef DUMP_BUFFER
    dumpBuffer(DNS_PAYLOAD_DUMP,p->payload,p->payload_size);
#endif

    if (dnsSessionData == NULL)
    {
        /* Check the ports to make sure this is a DNS port.
         * Otherwise no need to examine the traffic.
         */
#ifdef TARGET_BASED
        app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

        if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
            return;

        if (app_id && (app_id != dns_app_id))
            return;

        if (!app_id)
        {
#endif
            src = CheckDNSPort(config, p->src_port);
            dst = CheckDNSPort(config, p->dst_port);
#ifdef TARGET_BASED
        }
#endif

        /* See if a known server port is involved. */
        known_port = ( src || dst ? 1 : 0 );

#if 0
        if ( !dns_config->autodetect && !src && !dst )
        {
            /* Not one of the ports we care about. */
            return;
        }
#endif
#ifdef TARGET_BASED
        if (!app_id && !known_port)
#else
        if (!known_port)
#endif
        {
            /* Not one of the ports we care about. */
            return;
        }
    }

    /* For TCP, do a few extra checks... */
    if (p->tcp_header)
    {
        /* If session picked up mid-stream, do not process further.
         * Would be almost impossible to tell where we are in the
         * data stream. */
        if ( _dpd.sessionAPI->get_session_flags(
            p->stream_session) & SSNFLAG_MIDSTREAM )
        {
            return;
        }

        if ( !_dpd.streamAPI->is_stream_sequenced(p->stream_session,
                    SSN_DIR_TO_SERVER))
        {
            return;
        }

        if (!(_dpd.streamAPI->get_reassembly_direction(p->stream_session) & SSN_DIR_TO_SERVER))
        {
            /* This should only happen for the first packet (SYN or SYN-ACK)
             * in the TCP session */
            _dpd.streamAPI->set_reassembly(p->stream_session,
                STREAM_FLPOLICY_FOOTPRINT, SSN_DIR_TO_SERVER,
                STREAM_FLPOLICY_SET_ABSOLUTE);

            return;
        }

        /* If we're waiting on stream reassembly, don't process this packet. */
        if ( p->flags & FLAG_STREAM_INSERT)
        {
            return;
        }

        /* Get the direction of the packet. */
        direction = ( (p->flags & FLAG_FROM_SERVER ) ?
                        DNS_DIR_FROM_SERVER : DNS_DIR_FROM_CLIENT );
    }
    else if (p->udp_header)
    {
#ifdef TARGET_BASED
        if (app_id == dns_app_id)
        {
            direction = ( (p->flags & FLAG_FROM_SERVER ) ?
                        DNS_DIR_FROM_SERVER : DNS_DIR_FROM_CLIENT );
        }
        else {
#endif
        if (src)
            direction = DNS_DIR_FROM_SERVER;
        else if (dst)
            direction = DNS_DIR_FROM_CLIENT;
#ifdef TARGET_BASED
        }
#endif
    }

    PREPROC_PROFILE_START(dnsPerfStats);

    /* Check the stream session. If it does not currently
     * have our DNS data-block attached, create one.
     */
    if (dnsSessionData == NULL)
        dnsSessionData = GetDNSSessionData(p, config);

    if ( !dnsSessionData )
    {
        /* Could not get/create the session data for this packet. */
        PREPROC_PROFILE_END(dnsPerfStats);
        return;
    }

    if (dnsSessionData->flags & DNS_FLAG_NOT_DNS)
    {
        /* determined that this session wasn't DNS, we're done */
        PREPROC_PROFILE_END(dnsPerfStats);
        return;
    }

    if (direction == DNS_DIR_FROM_SERVER)
    {
        ParseDNSResponseMessage(p, dnsSessionData);
    }

    PREPROC_PROFILE_END(dnsPerfStats);
}

static void DNSReset(int signal, void *data)
{
    return;
}

static void DNSResetStats(int signal, void *data)
{
    return;
}

static void enablePortStreamServices(struct _SnortConfig *sc, DNSConfig *config, tSfPolicyId policy_id)
{
    uint32_t port;

    if (config == NULL)
        return;

    for (port = 0; port < MAXPORTS; port++)
    {
        if( isPortEnabled( config->ports, port ) )
        {
            // set port filter status 
            _dpd.streamAPI->set_port_filter_status
                (sc, IPPROTO_TCP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);

            _dpd.streamAPI->set_port_filter_status
                (sc, IPPROTO_UDP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);
            
            // register for reassembly
            _dpd.streamAPI->register_reassembly_port( NULL,
                                                      port,
                                                      SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
            // enable dns preproc for dispatch on configure ports
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_DNS, PROTO_BIT__TCP | PROTO_BIT__UDP, port );
        }
    }
}
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status
        (sc, dns_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int DnsFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    DNSConfig *pPolicyConfig = (DNSConfig *)pData;

    //do any housekeeping before freeing DnsConfig

    sfPolicyUserDataClear (config, policyId);
    _dpd.snortFree(pPolicyConfig, sizeof(DNSConfig),
                   PP_DNS, PP_MEM_CATEGORY_CONFIG);
    return 0;
}

static void DNSFreeConfig(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, DnsFreeConfigPolicy);
    sfPolicyConfigDelete(config);
}

static int DNSCheckPolicyConfig(
            struct _SnortConfig *sc,
            tSfPolicyUserContextId config,
            tSfPolicyId policyId,
            void* pData
            )
{
    _dpd.setParserPolicy(sc, policyId);

    if (_dpd.streamAPI == NULL)
     {
        _dpd.errMsg("Streaming & reassembly must be enabled for DNS preprocessor\n");
        return -1;
    }

    return 0;
}

static int DNSCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate (sc, dns_config, DNSCheckPolicyConfig)))
        return rval;
    return 0;
}

static void DNSCleanExit(int signal, void *data)
{
    DNSFreeConfig(dns_config);
    dns_config = NULL;
}

#ifdef SNORT_RELOAD
static void DNSReload(struct _SnortConfig *sc, char *argp, void **new_config)
{
    tSfPolicyUserContextId dns_swap_config = (tSfPolicyUserContextId)*new_config;
    int policy_id = _dpd.getParserPolicy(sc);
    DNSConfig *pPolicyConfig = NULL;

    if (dns_swap_config == NULL)
    {
        //create a context
        dns_swap_config = sfPolicyConfigCreate();
        if (dns_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                            "DNS configuration.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Dns preprocessor requires the "
                "stream5 preprocessor to be enabled.\n",
                *(_dpd.config_file), *(_dpd.config_line));
        }
        *new_config = (void *)dns_swap_config;
    }

    sfPolicyUserPolicySet (dns_swap_config, policy_id);
    pPolicyConfig = (DNSConfig *)sfPolicyUserDataGetCurrent(dns_swap_config);
    if (pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Dns preprocessor can only "
            "be configured once.\n", *(_dpd.config_file), *(_dpd.config_line));
    }

    pPolicyConfig = (DNSConfig *)_dpd.snortAlloc(1, sizeof(DNSConfig), 
                                                PP_DNS, PP_MEM_CATEGORY_CONFIG);
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                        "DNS configuration.\n");
    }

    sfPolicyUserDataSetCurrent(dns_swap_config, pPolicyConfig);

    ParseDNSArgs(pPolicyConfig, (u_char *)argp);

    _dpd.addPreproc(sc, ProcessDNS, PRIORITY_APPLICATION, PP_DNS, PROTO_BIT__TCP | PROTO_BIT__UDP);
    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static int DNSReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    int rval;

    if( ( rval = sfPolicyUserDataIterate( sc, swap_config, DNSCheckPolicyConfig ) ) )
        return rval;
    return 0;
}

static void * DNSReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId dns_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = dns_config;

    if (dns_swap_config == NULL)
        return NULL;

    dns_config = dns_swap_config;

    return (void *)old_config;
}

static void DNSReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    DNSFreeConfig((tSfPolicyUserContextId)data);
}
#endif
