/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *****************************************************************************/

/**************************************************************************
 *
 * ssl_ha.c
 *
 * Authors: Michael Altizer <mialtize@cisco.com>, Russ Combs <rucombs@cisco.com>, Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * SSL high availability support.
 *
 **************************************************************************/

#ifdef ENABLE_HA
#include "ssl_include.h"
#include "ssl_ha.h"
#include "ssl_config.h"
#include <errno.h>
#include <fcntl.h>

typedef struct _SSLHAFuncsNode
{
    uint16_t id;
    uint16_t mask;
    uint8_t preproc_id;
    uint8_t subcode;
    uint32_t size;
    SSLHAProducerFunc produce;
    SSLHAConsumerFunc consume;
    SSLHAConsumerFunc deletion;
    uint32_t produced;
    uint32_t consumed;
    uint32_t deleted;
} SSLHAFuncsNode;


typedef enum
{
    HA_TYPE_PSD,    // Preprocessor-specific Data
    HA_TYPE_MAX
} SSL_HA_Type;

typedef struct _MsgHeader
{
    uint8_t event;
    uint8_t version;
    uint16_t total_length;
} MsgHeader;

typedef struct _RecordHeader
{
    uint8_t type;
    uint32_t length;
} RecordHeader;

typedef struct _PreprocDataHeader
{
    uint8_t preproc_id;
    uint8_t subcode;
} PreprocDataHeader;

typedef struct
{
    uint32_t update_messages_received;
    uint32_t delete_messages_received;
    uint32_t update_messages_sent;
    uint32_t delete_messages_sent;
} SSLHAStats;

typedef struct _HADebugSessionConstraints
{
    sfcidr_t sip;
    sfcidr_t dip;
    uint16_t sport;
    uint16_t dport;
    uint8_t protocol;
} HADebugSessionConstraints;

#define MAX_SSL_HA_FUNCS 8  // depends on sizeof(SSLLWSession.ha_pending_mask)
#define HA_SSL_MESSAGE_VERSION  0x82

static SSLHAFuncsNode *ssl_ha_funcs[MAX_SSL_HA_FUNCS];
static int n_ssl_ha_funcs = 0;
static int runtime_output_fd = -1;
static uint8_t file_io_buffer[UINT16_MAX];
static SSLHAStats sslha_stats;

/* Runtime debugging stuff. */
//#define HA_DEBUG_SESSION_ID_SIZE    (39+1+5+5+39+1+5+1+3+1) /* "<IPv6 address>:<port> <-> <IPv6 address>:<port> <ipproto>\0" */
//static HADebugSessionConstraints ssl_ha_debug_info;
//static volatile int ssl_ha_debug_flag = 0;
//static char ssl_ha_debug_session[HA_DEBUG_SESSION_ID_SIZE];


#ifdef PERF_PROFILING
PreprocStats sslHAPerfStats;
PreprocStats sslHAConsumePerfStats;
PreprocStats sslHAProducePerfStats;
#endif

#if 0
// NEED TO ADD THIS FOR SSL
//--------------------------------------------------------------------
//  Runtime debugging support.
//--------------------------------------------------------------------
static inline bool SSLHADebugCheck(const SessionKey *key, volatile int debug_flag,
                                        HADebugSessionConstraints *info, char *debug_session, size_t debug_session_len)
{
}

static void SSLHADebugParse(const char *desc, const uint8_t *data, uint32_t length,
        volatile int *debug_flag, HADebugSessionConstraints *info)
{
}

static int SSLDebugHA(uint16_t type, const uint8_t *data, uint32_t length, void **new_context, char *statusBuf, int statusBuf_len)
{
    SSLHADebugParse("S5HA", data, length, &ssl_ha_debug_flag, &ssl_ha_debug_info);
    return 0;
}

#endif


int RegisterSSLHAFuncs(uint32_t preproc_id, uint8_t subcode, uint32_t size,
                            SSLHAProducerFunc produce, SSLHAConsumerFunc consume, SSLHAConsumerFunc deletion)
{
    SSLHAFuncsNode *node;
    int i, idx;

    if (produce == NULL || consume == NULL)
    {
        DynamicPreprocessorFatalMessage("One must be both a producer and a consumer to participate in SSL HA!\n");
    }

    if (preproc_id > UINT8_MAX)
    {
        DynamicPreprocessorFatalMessage("Preprocessor ID must be between 0 and %d to participate in SSL HA!\n", UINT8_MAX);
    }

    idx = n_ssl_ha_funcs;
    for (i = 0; i < n_ssl_ha_funcs; i++)
    {
        node = ssl_ha_funcs[i];
        if (node)
        {
            if (preproc_id == node->preproc_id && subcode == node->subcode)
            {
                DynamicPreprocessorFatalMessage("Duplicate SSL HA registration attempt for preprocessor %hu with subcode %hu\n",
                           node->preproc_id, node->subcode);
            }
        }
        else if (idx == n_ssl_ha_funcs)
            idx = i;
    }

    if (idx == MAX_SSL_HA_FUNCS)
    {
        DynamicPreprocessorFatalMessage("Attempted to register more than %d SSL HA types!\n", MAX_SSL_HA_FUNCS);
    }

    if (idx == n_ssl_ha_funcs)
        n_ssl_ha_funcs++;

    node = (SSLHAFuncsNode *)calloc(1, sizeof(SSLHAFuncsNode));
    if (!node)
    {
        DynamicPreprocessorFatalMessage("SSLHA: Failed to allocate memory for new node\n");
    }
    node->id = idx;
    node->mask = (1 << idx);
    node->preproc_id = (uint8_t) preproc_id;
    node->subcode = subcode;
    node->size = size;
    node->produce = produce;
    node->consume = consume;
    node->deletion = deletion;

    ssl_ha_funcs[idx] = node;

    _dpd.logMsg("SSLHA: Registered node %hu for preprocessor ID %hhu with subcode %hhu (size %hhu)\n",
                node->id, node->preproc_id, node->subcode, node->size);

    return idx;
}


void UnregisterSSLHAFuncs(uint32_t preproc_id, uint8_t subcode)
{
    SSLHAFuncsNode *node;
    int i;

    for (i = 0; i < n_ssl_ha_funcs; i++)
    {
        node = ssl_ha_funcs[i];
        if (node && preproc_id == node->preproc_id && subcode == node->subcode)
        {
            ssl_ha_funcs[i] = NULL;
            free(node);
            break;
        }
    }

    if ((i + 1) == n_ssl_ha_funcs)
        n_ssl_ha_funcs--;
}

static void SSLParseHAArgs(struct _SnortConfig *sc, SSLHAConfig *config, char *args)
{
    char **toks;
    int num_toks;
    int i;
    char **stoks = NULL;
    int s_toks;
    char *endPtr = NULL;
    unsigned long int value;

    if (config == NULL)
        return;

    if ((args == NULL) || (strlen(args) == 0))
        return;

    toks = _dpd.tokenSplit(args, ",", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        stoks = _dpd.tokenSplit(toks[i], " ", 2, &s_toks, 0);

        if (s_toks == 0)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Missing parameter in SSL HA config.\n",
                    __FILE__, __LINE__);
        }

        if (!strcmp(stoks[0], "min_sync_interval"))
        {
            if (stoks[1])
                value = strtoul(stoks[1], &endPtr, 10);
            else
                value = 0;

            if (!stoks[1] || (endPtr == &stoks[1][0]))
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid '%s' in config file. Requires integer parameter.\n",
                           __FILE__, __LINE__, stoks[0]);
            }

            if (value > UINT16_MAX)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s %lu' invalid: value must be between 0 and %d milliseconds.\n",
                           __FILE__, __LINE__, stoks[0], value, UINT16_MAX);
            }

            config->min_sync_interval.tv_sec = 0;
            while (value >= 1000)
            {
                config->min_sync_interval.tv_sec++;
                value -= 1000;
            }
            config->min_sync_interval.tv_usec = value * 1000;
        }
        else if (!strcmp(stoks[0], "startup_input_file"))
        {
            if (!stoks[1])
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' missing an argument\n", __FILE__, __LINE__, stoks[0]);
            }
            if (config->startup_input_file)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' specified multiple times\n", __FILE__, __LINE__, stoks[0]);
            }
            config->startup_input_file = strdup(stoks[1]);
            if (!config->startup_input_file)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Memory allocation failure.\n", __FILE__, __LINE__);
            }
        }
        else if (!strcmp(stoks[0], "runtime_output_file"))
        {
            if (!stoks[1])
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' missing an argument\n", __FILE__, __LINE__, stoks[0]);
            }
            if (config->runtime_output_file)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' specified multiple times\n", __FILE__, __LINE__, stoks[0]);
            }
            config->runtime_output_file = strdup(stoks[1]);
            if (!config->runtime_output_file)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Memory allocation failure.\n", __FILE__, __LINE__);
            }

        }
        else if (!strcmp(stoks[0], "shutdown_output_file"))
        {
            if (!stoks[1])
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' missing an argument\n", __FILE__, __LINE__, stoks[0]);
            }
            if (config->shutdown_output_file)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' specified multiple times\n", __FILE__, __LINE__, stoks[0]);
            }
            config->shutdown_output_file = strdup(stoks[1]);
            if (!config->shutdown_output_file)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Memory allocation failure.\n", __FILE__, __LINE__);
            }
        }
        else if (!strcmp(stoks[0], "use_side_channel"))
        {
#ifdef SIDE_CHANNEL
            if (!_dpd.isSCEnabled())
            {
                DynamicPreprocessorFatalMessage("%s(%d) => '%s' cannot be specified without enabling the Snort side channel.\n",
                            __FILE__, __LINE__, stoks[0]);
            }
            config->use_side_channel = 1;
#else
            DynamicPreprocessorFatalMessage("%s(%d) => Snort has been compiled without Side Channel support.\n", __FILE__, __LINE__);
#endif
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Invalid SSL HA config option '%s'\n",
                    __FILE__, __LINE__, stoks[0]);
        }

        _dpd.tokenFree(&stoks, s_toks);
    }

    _dpd.tokenFree(&toks, num_toks);

}

static void SSLPrintHAConfig(SSLHAConfig *config)
{
    if (config == NULL)
        return;

    _dpd.logMsg("SSL HA config:\n");
    _dpd.logMsg("    Minimum Sync Interval: %lu milliseconds\n",
                config->min_sync_interval.tv_sec * 1000 + config->min_sync_interval.tv_usec / 1000);
    if (config->startup_input_file)
        _dpd.logMsg("    Startup Input File:    %s\n", config->startup_input_file);
    if (config->runtime_output_file)
        _dpd.logMsg("    Runtime Output File:   %s\n", config->runtime_output_file);
    if (config->shutdown_output_file)
        _dpd.logMsg("    Shutdown Output File:  %s\n", config->shutdown_output_file);
}

int DisplaySSLHAStats(char *buffer)
{
    SSLHAFuncsNode *node;
    int i;
	int len = 0;

    len += snprintf(buffer, CS_STATS_BUF_SIZE,  "  High Availability\n"
			"          Updates Received: %u\n"
			"        Deletions Received: %u\n"
			"        Updates Sent: %u\n"
			"            Deletions Sent: %u\n"
			, sslha_stats.update_messages_received
			, sslha_stats.delete_messages_received
			, sslha_stats.update_messages_sent
			, sslha_stats.delete_messages_sent);

    for (i = 0; i < n_ssl_ha_funcs && len < CS_STATS_BUF_SIZE; i++) {
        node = ssl_ha_funcs[i];
        if (!node)
            continue;
        len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "        Node %hhu/%hhu: %u produced, %u consumed, %u deleted\n",
                    node->preproc_id, node->subcode, node->produced, node->consumed, node->deleted);
    }

    return len;
}

void SSLPrintHAStats(void)
{
    SSLHAFuncsNode *node;
    int i;

    _dpd.logMsg("  High Availability\n");
    _dpd.logMsg("          Updates Received: %u\n", sslha_stats.update_messages_received);
    _dpd.logMsg("        Deletions Received: %u\n", sslha_stats.delete_messages_received);
    _dpd.logMsg("        Updates Sent: %u\n", sslha_stats.update_messages_sent);
    _dpd.logMsg("            Deletions Sent: %u\n", sslha_stats.delete_messages_sent);
    for (i = 0; i < n_ssl_ha_funcs; i++)
    {
        node = ssl_ha_funcs[i];
        if (!node)
            continue;
        _dpd.logMsg("        Node %hhu/%hhu: %u produced, %u consumed, %u deleted\n",
                    node->preproc_id, node->subcode, node->produced, node->consumed, node->deletion);
    }
}

void SSLResetHAStats(void)
{
    memset(&sslha_stats, 0, sizeof(sslha_stats));
}

void SSLHAInit(struct _SnortConfig *sc, char *args)
{
    SSLPP_config_t *pDefaultPolicyConfig = NULL;

    if (ssl_config == NULL)
        DynamicPreprocessorFatalMessage("Tried to config SSL HA policy without global config!\n");

    if( _dpd.getParserPolicy(sc) != _dpd.getDefaultPolicy() )
        return;

    pDefaultPolicyConfig = (SSLPP_config_t *)sfPolicyUserDataGetDefault(ssl_config);

    if (pDefaultPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Tried to config SSL HA policy without SSL config!\n");
    }

    if (pDefaultPolicyConfig->ssl_ha_config != NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) ==> Cannot duplicate SSL HA configuration\n", __FILE__, __LINE__);
    }

    if (!pDefaultPolicyConfig->enable_ssl_ha)
    {
        return;
    }

    pDefaultPolicyConfig->ssl_ha_config = (SSLHAConfig*)calloc(1, sizeof( SSLHAConfig ));
    if (!pDefaultPolicyConfig->ssl_ha_config)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate storage for Session HA configuration.\n");
    }


    SSLParseHAArgs(sc, pDefaultPolicyConfig->ssl_ha_config, args);

#ifdef PERF_PROFILING
    _dpd.addPreprocProfileFunc("sslHAProduce", &sslHAProducePerfStats, 2, &sslHAPerfStats, NULL);
    _dpd.addPreprocProfileFunc("sslHAConsume", &sslHAConsumePerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

    SSLPrintHAConfig(pDefaultPolicyConfig->ssl_ha_config);
}

#if defined(SNORT_RELOAD)
void SSLHAReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    SSLHAConfig *ssl_ha_config = ( SSLHAConfig *) *new_config;
    SSLPP_config_t *config;

    if( _dpd.getParserPolicy(sc) != _dpd.getDefaultPolicy( ) )
        return;

    config = (SSLPP_config_t *)sfPolicyUserDataGetDefault(ssl_config);

    if( !config || !config->enable_ssl_ha )
        return;

    if (ssl_ha_config == NULL)
    {
        ssl_ha_config = (SSLHAConfig *)calloc(1, sizeof(SSLHAConfig));
        if ( ssl_ha_config == NULL )
            DynamicPreprocessorFatalMessage("Failed to allocate new storage for Session HA configuration.\n");
        *new_config = ssl_ha_config;
    }
    else
    {
        DynamicPreprocessorFatalMessage("%s(%d) ==> Cannot duplicate SSL HA configuration\n", __FILE__, __LINE__);
    }

    SSLParseHAArgs(sc, ssl_ha_config, args);
    SSLPrintHAConfig(ssl_ha_config);
}
#endif

int SSLVerifyHAConfig(struct _SnortConfig *sc, void *config)
{
    if (config == NULL)
        return -1;

    return 0;
}

void *SSLHASwapReload( struct _SnortConfig *sc, void *data )
{
    SSLPP_config_t *config;

    config = (SSLPP_config_t *)sfPolicyUserDataGet(ssl_config, _dpd.getDefaultPolicy());

    if(config)
        config->ssl_ha_config = ( SSLHAConfig * ) data;
    return NULL;
}

void SSLHAConfigFree(void *data)
{
    SSLHAConfig *config = (SSLHAConfig *)data;

    if (config == NULL)
        return;

    if (config->startup_input_file)
        free(config->startup_input_file);

    if (config->runtime_output_file)
        free(config->runtime_output_file);

    if (config->shutdown_output_file)
        free(config->shutdown_output_file);

    free(config);
}


static inline int DeserializeSSLPreprocData(uint8_t event, uint8_t preproc_id,
                                         uint8_t subcode, const uint8_t *data, uint32_t length)
{
    SSLHAFuncsNode *node;
    int i;

    for (i = 0; i < n_ssl_ha_funcs; i++)
    {
        node = ssl_ha_funcs[i];
        if (node && preproc_id == node->preproc_id && subcode == node->subcode)
        {
            if (length < node->size)
            {
                _dpd.errMsg("SSL HA preprocessor data record's length is less than expected size! (%u vs %u)\n",
                        length, node->size);
                return -1;
            }
            if ( event == HA_EVENT_UPDATE )
            {
                node->consumed++;
                sslha_stats.update_messages_received++;
                return node->consume(data, length);
            }
            else
            {
                node->deleted++;
                sslha_stats.delete_messages_received++;
                return node->deletion(data, length);
            }
        }
    }

    _dpd.errMsg("SSL HA preprocessor data record received with unrecognized preprocessor ID/subcode! (%hhu:%hhu)\n",
            preproc_id, subcode);
    return -1;
}

static int ConsumeSSLHAMessage(const uint8_t *msg, uint32_t msglen)
{
    MsgHeader *msg_hdr;
    RecordHeader *rec_hdr;
    PreprocDataHeader *psd_hdr;
    uint32_t offset;
    int rval = 1;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sslHAConsumePerfStats);

    /* Read the message header */
    if (msglen < sizeof(*msg_hdr))
    {
        _dpd.errMsg("SSL HA message length shorter than header length! (%u)\n", msglen);
        goto ssl_consume_exit;
    }
    msg_hdr = (MsgHeader *) msg;
    offset = sizeof(*msg_hdr);

    if (msg_hdr->total_length != msglen)
    {
        _dpd.errMsg("SSL HA message header's total length does not match actual length! (%u vs %u)\n",
                msg_hdr->total_length, msglen);
        goto ssl_consume_exit;
    }

    if (msg_hdr->event != HA_EVENT_UPDATE && msg_hdr->event != HA_EVENT_DELETE)
    {
        _dpd.errMsg("SSL HA message has unknown event type: %hhu!\n", msg_hdr->event);
        goto ssl_consume_exit;
    }

    /* Read the key */

    /* Read any/all records. */
    while (offset < msglen)
    {
        if (sizeof(*rec_hdr) > (msglen - offset))
        {
            _dpd.errMsg("SSL HA message contains a truncated record header! (%zu vs %u)\n",
                    sizeof(*rec_hdr), msglen - offset);
            goto ssl_consume_exit;
        }
        rec_hdr = (RecordHeader *) (msg + offset);
        offset += sizeof(*rec_hdr);

        switch (rec_hdr->type)
        {
            case HA_TYPE_PSD:
                if (sizeof(*psd_hdr) > (msglen - offset))
                {
                    _dpd.errMsg("SSL HA message contains a truncated preprocessor data record header! (%zu vs %u)\n",
                            sizeof(*psd_hdr), msglen - offset);
                    goto ssl_consume_exit;
                }
                psd_hdr = (PreprocDataHeader *) (msg + offset);
                offset += sizeof(*psd_hdr);
                if (rec_hdr->length > (msglen - offset))
                {
                    _dpd.errMsg("SSL HA message contains truncated preprocessor data record data! (%u vs %u)\n",
                            rec_hdr->length, msglen - offset);
                    goto ssl_consume_exit;
                }
                if (DeserializeSSLPreprocData(msg_hdr->event, psd_hdr->preproc_id, psd_hdr->subcode,
                                            msg + offset, rec_hdr->length) != 0)
                {
                    _dpd.errMsg("SSL HA message contained invalid preprocessor data record!\n");
                    goto ssl_consume_exit;
                }
                offset += rec_hdr->length;
                break;

            default:
                _dpd.errMsg("SSL HA message contains unrecognized record type: %hhu!\n", rec_hdr->type);
                goto ssl_consume_exit;
        }
    }
    /* Mark the session as being in standby mode since we just received an update. */
    rval = 0;

ssl_consume_exit:
    PREPROC_PROFILE_END(sslHAConsumePerfStats);
    return rval;
}


/*
 * File I/O
 */
static inline ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t n;
    errno = 0;

    while ((n = read(fd, buf, count)) <= (ssize_t) count)
    {
        if (n == (ssize_t) count)
            return 0;

        if (n > 0)
        {
            buf = (uint8_t *) buf + n;
            count -= n;
        }
        else if (n == 0)
            break;
        else if (errno != EINTR)
        {
            _dpd.errMsg("Error reading from SSL HA message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }
    return -1;
}

static int ReadHAMessagesFromFile(const char *filename)
{
    MsgHeader *msg_header;
    uint8_t *msg;
    int rval, fd;

    fd = open(filename, O_RDONLY, 0664);
    if (fd < 0)
    {
        DynamicPreprocessorFatalMessage("Could not open %s for reading HA messages from: %s (%d)\n", filename, strerror(errno), errno);
    }

    _dpd.logMsg("Reading SSL HA messages from '%s'...\n", filename);
    msg = file_io_buffer;
    while ((rval = Read(fd, msg, sizeof(*msg_header))) == 0)
    {
        msg_header = (MsgHeader *) msg;
        if (msg_header->total_length < sizeof(*msg_header))
        {
            _dpd.errMsg("SSL HA Message total length (%hu) is way too short!\n", msg_header->total_length);
            close(fd);
            return -1;
        }
        else if (msg_header->total_length > (UINT16_MAX - sizeof(*msg_header)))
        {
            _dpd.errMsg("SSL HA Message total length (%hu) is too long!\n", msg_header->total_length);
            close(fd);
            return -1;
        }
        else if (msg_header->total_length > sizeof(*msg_header))
        {
            if ((rval = Read(fd, msg + sizeof(*msg_header), msg_header->total_length - sizeof(*msg_header))) != 0)
            {
                _dpd.errMsg("Error reading the remaining %zu bytes of an HA message from file: %s (%d)\n",
                        msg_header->total_length - sizeof(*msg_header), strerror(errno), errno);
                close(fd);
                return rval;
            }
        }
        if( msg_header->version == HA_SSL_MESSAGE_VERSION )
        {
            if ((rval = ConsumeSSLHAMessage(msg, msg_header->total_length)) != 0)
            {
                close(fd);
                return rval;
            }
        }
    }
    close(fd);

    return 0;
}

static inline ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t n;
    errno = 0;

    while ((n = write(fd, buf, count)) <= (ssize_t) count)
    {
        if (n == (ssize_t) count)
            return 0;

        if (n > 0)
            count -= n;
        else if (errno != EINTR)
        {
            _dpd.errMsg("Error writing to SSL HA message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }

    return -1;
}

static uint32_t WriteSSLHAMessageHeader(uint8_t event, uint16_t msglen, uint8_t *msg)
{
    MsgHeader *msg_hdr;
    uint32_t offset;

    msg_hdr = (MsgHeader *) msg;
    offset = sizeof(*msg_hdr);
    msg_hdr->event = event;
    msg_hdr->version = HA_SSL_MESSAGE_VERSION;
    msg_hdr->total_length = msglen;

    return offset;
}

static uint32_t WriteSSLPreprocDataRecord(SSLHAFuncsNode *node, uint8_t *msg, void *data, int size)
{
    RecordHeader *rec_hdr;
    PreprocDataHeader *psd_hdr;
    uint32_t offset;

    rec_hdr = (RecordHeader *) msg;
    offset = sizeof(*rec_hdr);
    rec_hdr->type = HA_TYPE_PSD;

    psd_hdr = (PreprocDataHeader *) (msg + offset);
    offset += sizeof(*psd_hdr);
    psd_hdr->preproc_id = node->preproc_id;
    psd_hdr->subcode = node->subcode;

    rec_hdr->length = node->produce(msg + offset, data, size);
    offset += rec_hdr->length;
    node->produced++;

    return offset;
}

static void UpdateSSLHAMessageHeaderLength(uint8_t *msg, uint16_t msglen)
{
    MsgHeader *msg_hdr;

    msg_hdr = (MsgHeader *) msg;
    msg_hdr->total_length = msglen;
}

static uint32_t CalculateSSLHAMessageSize(uint8_t event, int index, uint32_t ssl_size)
{
    SSLHAFuncsNode *node;
    uint32_t msg_size;
    msg_size = sizeof(MsgHeader);
    if (event == HA_EVENT_UPDATE)
    {
        /* Preprocessor data records */
        if( index < n_ssl_ha_funcs )
        {
            node = ssl_ha_funcs[index];

            if(!node)
                return 0;
            msg_size += sizeof(RecordHeader) + sizeof(PreprocDataHeader) + ssl_size;
        }
    }

    return msg_size;
}

static uint32_t GenerateSSLHAUpdateMessage(uint8_t *msg, uint32_t msg_size, int index, uint8_t event, void *data, uint32_t size)
{
    SSLHAFuncsNode *node;
    uint32_t offset;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sslHAProducePerfStats);

    offset = WriteSSLHAMessageHeader(event, msg_size, msg);
    if( index < n_ssl_ha_funcs )
    {
        node = ssl_ha_funcs[index];
        if(!node)
            return 0;
        offset += WriteSSLPreprocDataRecord(node, msg + offset, data, size);
    }
    /* Update the message header length since it might be shorter than originally anticipated. */
    UpdateSSLHAMessageHeaderLength(msg, offset);
    if( event == HA_EVENT_UPDATE )
    {
        sslha_stats.update_messages_sent++;
    }
    else
    {
        sslha_stats.delete_messages_sent++;
    }


    PREPROC_PROFILE_END(sslHAProducePerfStats);


    return offset;
}

#ifdef SIDE_CHANNEL
static void SendSSLSCUpdateMessage(int index, uint32_t msg_size, uint8_t event, void *data, uint32_t size)
{
    SCMsgHdr *schdr;
    void *msg_handle;
    uint8_t *msg;
    int rval;

    /* Allocate space for the message. */
    if ((rval = _dpd.scAllocMessageTX(msg_size, &schdr, &msg, &msg_handle)) != 0)
    {
        /* TODO: Error stuff goes here. */
        return;
    }

    /* Gnerate the message. */
    msg_size = GenerateSSLHAUpdateMessage(msg, msg_size, index, event, data, size);

    /* Send the message. */
    schdr->type = SC_MSG_TYPE_SSL_STATE_TRACKING;
    schdr->timestamp = _dpd.pktTime();
    _dpd.scEnqueueMessageTX(schdr, msg, msg_size, msg_handle, NULL);
}
#endif

void SSLProcessHA(int ssl_index, bool update, void *data, uint32_t ssl_size)
{
    uint32_t msg_size;
    SSLPP_config_t *config = NULL;
    SSLPP_config_t *defaultconfig = NULL;
    uint8_t event = (update ? HA_EVENT_UPDATE : HA_EVENT_DELETE);
    PROFILE_VARS;

    PREPROC_PROFILE_START(sslHAPerfStats);

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);
    defaultconfig = (SSLPP_config_t *)sfPolicyUserDataGet(ssl_config, _dpd.getDefaultPolicy());

    if (!config || !defaultconfig || !defaultconfig->ssl_ha_config || !config->enable_ssl_ha)
    {
        PREPROC_PROFILE_END(sslHAPerfStats);
        return;
    }

    /* Calculate the size of the update message. */
    msg_size = CalculateSSLHAMessageSize(event, ssl_index, ssl_size);

    if (runtime_output_fd >= 0)
    {
        msg_size = GenerateSSLHAUpdateMessage(file_io_buffer, msg_size, ssl_index, event, data, ssl_size);
        if (Write(runtime_output_fd, file_io_buffer, msg_size) == -1)
        {
            /* TODO: Error stuff here. */
        }
    }

#ifdef SIDE_CHANNEL
    if (defaultconfig->ssl_ha_config->use_side_channel)
    {
        SendSSLSCUpdateMessage(ssl_index, msg_size, event, data, ssl_size);
    }
#endif
}

#ifdef SIDE_CHANNEL
static int SSLHASCMsgHandler(SCMsgHdr *hdr, const uint8_t *msg, uint32_t msglen)
{
    int rval;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sslHAPerfStats);

    rval = ConsumeSSLHAMessage(msg, msglen);

    PREPROC_PROFILE_END(sslHAPerfStats);

    return rval;
}
#endif

void SSLHAPostConfigInit(struct _SnortConfig *sc, int unused, void *arg)
{
    SSLPP_config_t *pDefaultPolicyConfig;
    int rval;

    pDefaultPolicyConfig = (SSLPP_config_t *)sfPolicyUserDataGet(ssl_config, _dpd.getDefaultPolicy());
    if (!pDefaultPolicyConfig->enable_ssl_ha || !(pDefaultPolicyConfig->ssl_ha_config))
        return;

    if (pDefaultPolicyConfig->ssl_ha_config->startup_input_file)
    {
        if ((rval = ReadHAMessagesFromFile(pDefaultPolicyConfig->ssl_ha_config->startup_input_file)) != 0)
        {
            _dpd.errMsg("Errors were encountered while reading HA messages from file!");
        }
    }

    if (pDefaultPolicyConfig->ssl_ha_config->runtime_output_file)
    {
        runtime_output_fd = open(pDefaultPolicyConfig->ssl_ha_config->runtime_output_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        if (runtime_output_fd < 0)
        {
            DynamicPreprocessorFatalMessage("Could not open %s for writing HA messages to: %s (%d)\n",
                        pDefaultPolicyConfig->ssl_ha_config->runtime_output_file, strerror(errno), errno);
        }
    }

#ifdef SIDE_CHANNEL
    if (pDefaultPolicyConfig->ssl_ha_config->use_side_channel)
    {
        if ((rval = _dpd.scRegisterRXHandler(SC_MSG_TYPE_SSL_STATE_TRACKING, SSLHASCMsgHandler, NULL)) != 0)
        {
            /* TODO: Fatal error here or something. */
        }
    }
#endif
}

void SSLCleanHA(void)
{
    int i;

    for (i = 0; i < n_ssl_ha_funcs; i++)
    {
        if (ssl_ha_funcs[i])
        {
            free(ssl_ha_funcs[i]);
            ssl_ha_funcs[i] = NULL;
        }
    }
    if (runtime_output_fd >= 0)
    {
        close(runtime_output_fd);
        runtime_output_fd = -1;
    }
}
#endif
