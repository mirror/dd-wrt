/*
** Copyright (C) 2007-2011 Sourcefire, Inc.
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

/*
 * Adam Keeton
 * SSLPP Preprocessor
 * 10/10/07
 *
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "debug.h"

#include "preprocids.h"
#include "spp_ssl.h"

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#include "profiler.h"

#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#ifdef PERF_PROFILING
PreprocStats sslpp_perf_stats;
#endif

#ifdef TARGET_BASED
int16_t ssl_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif


/* Ultimately calls SnortEventqAdd */
/* Arguments are: gid, sid, rev, classification, priority, message, rule_info */
#define ALERT(x,y) { _dpd.alertAdd(GENERATOR_SPP_SSLPP, x, 1, 0, 3, y, 0 ); }

/* Wraps disabling detect with incrementing the counter */
#define DISABLE_DETECT() { _dpd.disableDetect(packet); counts.disabled++; }

extern DynamicPreprocessorData _dpd;

static tSfPolicyUserContextId ssl_config = NULL;
static SSLPP_counters_t counts;

#ifdef SNORT_RELOAD
static tSfPolicyUserContextId ssl_swap_config = NULL;
static void SSLReload(char *);
static int SSLReloadVerify(void);
static void * SSLReloadSwap(void);
static void SSLReloadSwapFree(void *);
#endif

static INLINE void SSLSetPort(SSLPP_config_t *, int);
static void SSL_UpdateCounts(const uint32_t);
#if DEBUG
static void SSL_PrintFlags(uint32_t);
#endif

static void SSLFreeConfig(tSfPolicyUserContextId config);
static void SSLCleanExit(int, void *);
static void SSLResetStats(int, void *);
static void SSLPP_CheckConfig(void);

static void _addPortsToStream5Filter(SSLPP_config_t *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStream5Filter(tSfPolicyId);
#endif

typedef struct _SslRuleOptData
{
    int flags;
    int mask;

} SslRuleOptData;

static INLINE int SSLPP_is_encrypted(uint32_t ssl_flags, SFSnortPacket *packet) 
{
    SSLPP_config_t *config = NULL;

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    if (config->flags & SSLPP_TRUSTSERVER_FLAG)
    {
        if(ssl_flags & SSL_SAPP_FLAG) 
            return SSLPP_TRUE;
    }

    if (SSL_IS_CLEAN(ssl_flags))
    {
        if (((ssl_flags & SSLPP_ENCRYPTED_FLAGS) == SSLPP_ENCRYPTED_FLAGS) ||
            ((ssl_flags & SSLPP_ENCRYPTED_FLAGS2) == SSLPP_ENCRYPTED_FLAGS2))
        {
            counts.completed_hs++;
            return SSLPP_TRUE;
        }
        /* Check if we're either midstream or if packets were missed after the 
         * connection was established */
        else if ((_dpd.streamAPI->get_session_flags (packet->stream_session_ptr) & SSNFLAG_MIDSTREAM) ||
                 (_dpd.streamAPI->missed_packets(packet->stream_session_ptr, SSN_DIR_BOTH)))
        {
            if ((ssl_flags & (SSL_CAPP_FLAG | SSL_SAPP_FLAG)) == (SSL_CAPP_FLAG | SSL_SAPP_FLAG)) 
            {
                return SSLPP_TRUE;
            }
        }
    }

    return SSLPP_FALSE;
}

static INLINE uint32_t SSLPP_process_alert(
        uint32_t ssn_flags, uint32_t new_flags, SFSnortPacket *packet)
{
    SSLPP_config_t *config = NULL;

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Process Alert\n"););

    ssn_flags |= new_flags;

    /* Check if we've seen a handshake, that this isn't it,
     * that the cipher flags is not set, and that we are disabling detection */
    if(SSL_IS_HANDSHAKE(ssn_flags) && 
       !SSL_IS_HANDSHAKE(new_flags) &&
       !(new_flags & SSL_CHANGE_CIPHER_FLAG) &&
       (config->flags & SSLPP_DISABLE_FLAG))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Disabling detect\n"););
        DISABLE_DETECT();
    }

    /* Need to negate the application flags from the opposing side. */

    if(packet->flags & FLAG_FROM_CLIENT)
        return ssn_flags & ~SSL_SAPP_FLAG;

    else if(packet->flags & FLAG_FROM_SERVER)
        return ssn_flags & ~SSL_CAPP_FLAG;

    return ssn_flags;
}

static INLINE uint32_t SSLPP_process_hs(uint32_t ssl_flags, uint32_t new_flags)
{
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Process Handshake\n"););

    if(!SSL_BAD_HS(new_flags))
    {
        ssl_flags |= new_flags & (SSL_CLIENT_HELLO_FLAG | 
                                  SSL_SERVER_HELLO_FLAG |
                                  SSL_CLIENT_KEYX_FLAG |
                                  SSL_SFINISHED_FLAG);
    }
    else
    {
        counts.bad_handshakes++;
    }

    return ssl_flags;
}

static INLINE uint32_t SSLPP_process_app(
        uint32_t ssn_flags, uint32_t new_flags, SFSnortPacket *packet) 
{
    SSLPP_config_t *config = NULL;

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Process Application\n"););

    if(!(config->flags & SSLPP_DISABLE_FLAG))
        return ssn_flags | new_flags;

    if(SSLPP_is_encrypted(ssn_flags | new_flags, packet) ) 
    {
        ssn_flags |= SSL_ENCRYPTED_FLAG;

        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "STOPPING INSPECTION (process_app)\n"););
        _dpd.streamAPI->stop_inspection(packet->stream_session_ptr,
                           packet, SSN_DIR_BOTH, -1, 0);

        counts.stopped++;
    }

    return ssn_flags | new_flags;
}

static INLINE void SSLPP_process_other(
        uint32_t ssn_flags, uint32_t new_flags, SFSnortPacket *packet) 
{
    SSLPP_config_t *config = NULL;

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    /* Encrypted SSLv2 will appear unrecognizable.  Check if the handshake was
     * seen and stop inspecting if so. */
    /* Check for an existing handshake from both sides */
    if((ssn_flags & SSL_VER_SSLV2_FLAG) && 
       SSL_IS_CHELLO(ssn_flags) && SSL_IS_SHELLO(ssn_flags) &&
       (config->flags & SSLPP_DISABLE_FLAG) && !(new_flags & SSL_CHANGE_CIPHER_FLAG))
    {
        ssn_flags |= SSL_ENCRYPTED_FLAG | new_flags;

        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "STOPPING INSPECTION (process_other)\n"););
        _dpd.streamAPI->stop_inspection(packet->stream_session_ptr,
                                        packet, SSN_DIR_BOTH, -1, 0);
    }
    else
    {
        counts.unrecognized++;

        /* Special handling for SSLv2 */
        if(new_flags & SSL_VER_SSLV2_FLAG) 
            ssn_flags |= new_flags;

        if(new_flags & SSL_UNKNOWN_FLAG)
            ssn_flags |= new_flags;

/* The following block is intentionally disabled. */
/* If we were unable to decode the packet, and previous packets had been 
 * missed,  we will not assume it is encrypted SSLv2. */
#if 0
        /* More special handling for SSLv2.
         * If both server-side and client-side data was missed, and it has not
         * been identified it as TLS, it is possibly encrypted SSLv2. */
        if( !(ssn_flags & ( SSL_VER_SSLV3_FLAG | SSL_VER_TLS10_FLAG |
                            SSL_VER_TLS11_FLAG | SSL_VER_TLS12_FLAG)) )
        {
            if(packet->stream_session_ptr && 
                _dpd.streamAPI->missed_packets(
                    packet->stream_session_ptr, SSN_DIR_SERVER) && 
                _dpd.streamAPI->missed_packets( 
                    packet->stream_session_ptr, SSN_DIR_CLIENT) )

                ssn_flags |= SSL_VER_SSLV2_FLAG;
        }
#endif
    }

     /* Still need to set application data here because some of the ssn_flags
     * flags were cleared in SSL_CLEAR_TEMPORARY_FLAGS */
    _dpd.streamAPI->set_application_data(packet->stream_session_ptr, PP_SSL,
                                         (void *)(uintptr_t)ssn_flags, NULL);
}

/* SSL Preprocessor process callback. */
static void SSLPP_process(void *raw_packet, void *context)
{
    SFSnortPacket *packet;
    uint32_t ssn_flags;
    uint32_t new_flags;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif
    SSLPP_config_t *config = NULL;
    PROFILE_VARS;

    sfPolicyUserPolicySet (ssl_config, _dpd.getRuntimePolicy());
    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    if (config == NULL)
        return;


    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL Start ================================\n"););

    packet = (SFSnortPacket*)raw_packet;

    if(!packet || !packet->payload || !packet->payload_size || 
            !packet->tcp_header || !packet->stream_session_ptr)
    {
#ifdef DEBUG
        if (packet == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Packet is NULL\n"););
        }

        if (packet->payload == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Packet payload is NULL\n"););
        }

        if (packet->payload_size == 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Packet payload size is 0\n"););
        }

        if (packet->tcp_header == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Packet is not TCP\n"););
        }

        if (packet->stream_session_ptr == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - No stream session pointer\n"););
        }

        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Not inspecting packet\n"););
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
#endif
        return;
    }
#ifdef TARGET_BASED
    /* Check packet based on protocol number */
    app_id = _dpd.streamAPI->get_application_protocol_id(packet->stream_session_ptr);
    if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
    {
        return;
    }
    if (app_id && (app_id != ssl_app_id))
    {
        return;
    }
    /* Fall back to port checking */
    if (!app_id)
    {
#endif
        /* Make sure the packet is on the right port */
        if(!(config->ports[PORT_INDEX(packet->src_port)] & CONV_PORT(packet->src_port)) &&
           !(config->ports[PORT_INDEX(packet->dst_port)] & CONV_PORT(packet->dst_port)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Not configured for these ports\n"););
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
            return;
        }
#ifdef TARGET_BASED
    }
#endif

#ifdef DEBUG
    if (packet->flags & FLAG_FROM_SERVER)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Server packet\n"););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Client packet\n"););
    }

    if (packet->flags & FLAG_REBUILT_STREAM)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Packet is rebuilt\n"););
    }
#endif

    PREPROC_PROFILE_START(sslpp_perf_stats);

    ssn_flags = (uint32_t)(uintptr_t)
        _dpd.streamAPI->get_application_data(packet->stream_session_ptr, PP_SSL);

    /* Flush opposite direction to keep conversation in sync */
    if (!(packet->flags & FLAG_REBUILT_STREAM))
    {
        switch (_dpd.streamAPI->get_reassembly_direction(packet->stream_session_ptr))
        {
            case SSN_DIR_SERVER:
                if (packet->flags & FLAG_FROM_SERVER)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Flushing server side\n"););
                    _dpd.streamAPI->response_flush_stream(packet);
                }

                break;

            case SSN_DIR_CLIENT:
                if (packet->flags & FLAG_FROM_CLIENT)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Flushing client side\n"););
                    _dpd.streamAPI->response_flush_stream(packet);
                }

                break;

            case SSN_DIR_BOTH:
                DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Flushing opposite side\n"););
                _dpd.streamAPI->response_flush_stream(packet);
                break;

            case SSN_DIR_NONE:
            default:
                break;
        }
    }

#if 0
    /* XXX If the preprocessor should in the future need to do any data
     * reassembly, one or the other of raw or reassembled needs to be used */
    if (packet->flags & FLAG_STREAM_INSERT)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Packet is stream inserted - not inspecting\n"););
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
        return;
    }
#endif

#ifdef DEBUG
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Ssn flags before ----------------------\n"););
    SSL_PrintFlags(ssn_flags);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif

    SSL_CLEAR_TEMPORARY_FLAGS(ssn_flags);

#ifdef DEBUG
    if (packet->payload_size >= 5)
    {
        const uint8_t *pkt = packet->payload;

        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Five bytes of data: %02x %02x %02x %02x %02x\n",
                    pkt[0], pkt[1], pkt[2], pkt[3], pkt[4]););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Payload size < 5 bytes"););
    }
#endif

    new_flags = SSL_decode(packet->payload, (int)packet->payload_size, packet->flags);

    if( SSL_IS_CHELLO(new_flags) && SSL_IS_CHELLO(ssn_flags) && SSL_IS_SHELLO(ssn_flags) )
    {
        ALERT(SSL_INVALID_CLIENT_HELLO, SSL_INVALID_CLIENT_HELLO_STR);
    }
    else if(!(config->flags & SSLPP_TRUSTSERVER_FLAG))
    {
        if( (SSL_IS_SHELLO(new_flags) && !SSL_IS_CHELLO(ssn_flags) ))
        {
            if(!(_dpd.streamAPI->missed_packets( packet->stream_session_ptr, SSN_DIR_CLIENT)))
                ALERT(SSL_INVALID_SERVER_HELLO, SSL_INVALID_SERVER_HELLO_STR);
        }
    }


    counts.decoded++;

#ifdef DEBUG
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "New flags -----------------------------\n"););
    SSL_PrintFlags(new_flags);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif

    SSL_UpdateCounts(new_flags);

    /* Note, there can be multiple record types in each SSL packet.
     * Processing them in this order is intentional.  If there is an
     * Alert, we don't care about the other records */

    if(SSL_IS_ALERT(new_flags))
    {
        ssn_flags = SSLPP_process_alert(ssn_flags, new_flags, packet);
    }
    else if(SSL_IS_HANDSHAKE(new_flags))
    {
        ssn_flags = SSLPP_process_hs(ssn_flags, new_flags);
    }
    else if(SSL_IS_APP(new_flags))
    {
        ssn_flags = SSLPP_process_app(ssn_flags, new_flags, packet);
    }
    else 
    {
        /* Different record type that we don't care about.
         * Either it's a 'change cipher spec' or we failed to recognize the
         * record type.  Do not update session data */
        SSLPP_process_other(ssn_flags, new_flags, packet);

        /* Application data is updated inside of SSLPP_process_other */

        PREPROC_PROFILE_END(sslpp_perf_stats);
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
        return;
    }

    ssn_flags |= new_flags;

#if DEBUG
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Ssn flags after -----------------------\n"););
    SSL_PrintFlags(ssn_flags);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif

    _dpd.streamAPI->set_application_data(
            packet->stream_session_ptr, PP_SSL,
            (void*)(uintptr_t)ssn_flags, NULL);

    PREPROC_PROFILE_END(sslpp_perf_stats);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
}

static void SSL_UpdateCounts(const uint32_t new_flags)
{
    if(new_flags & SSL_CHANGE_CIPHER_FLAG) 
        counts.cipher_change++;

    if (new_flags & SSL_ALERT_FLAG)
        counts.alerts++;

    if (new_flags & SSL_CLIENT_HELLO_FLAG)
        counts.hs_chello++;

    if (new_flags & SSL_SERVER_HELLO_FLAG)
        counts.hs_shello++;

    if (new_flags & SSL_CERTIFICATE_FLAG)
        counts.hs_cert++;

    if (new_flags & SSL_SERVER_KEYX_FLAG)
        counts.hs_skey++;

    if (new_flags & SSL_CLIENT_KEYX_FLAG)
        counts.hs_ckey++;

    if (new_flags & SSL_SFINISHED_FLAG)
        counts.hs_finished++;

    if (new_flags & SSL_HS_SDONE_FLAG)
        counts.hs_sdone++;

    if (new_flags & SSL_SAPP_FLAG)
        counts.sapp++;

    if (new_flags & SSL_CAPP_FLAG)
        counts.capp++;
}

/* Parsing for the ssl_state rule option */
static int SSLPP_state_init(char *name, char *params, void **data) 
{
    int flags = 0, mask = 0;
    char *end = NULL;
    char *tok;
    SslRuleOptData *sdata;

    tok = strtok_r(params, ",", &end);

    if(!tok)
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to"
            "ssl_state keyword\n", *(_dpd.config_file), *(_dpd.config_line));

    do
    {
        int negated = 0;

        if (tok[0] == '!')
        {
            negated = 1;
            tok++;
        }

        if(!strcasecmp("client_hello", tok))
        {
            flags |= SSL_CUR_CLIENT_HELLO_FLAG;
            if (negated)
                mask |= SSL_CUR_CLIENT_HELLO_FLAG;
        }
        else if(!strcasecmp("server_hello", tok))
        {
            flags |= SSL_CUR_SERVER_HELLO_FLAG;
            if (negated)
                mask |= SSL_CUR_SERVER_HELLO_FLAG;
        }
        else if(!strcasecmp("client_keyx", tok))
        {
            flags |= SSL_CUR_CLIENT_KEYX_FLAG;
            if (negated)
                mask |= SSL_CUR_CLIENT_KEYX_FLAG;
        }
        else if(!strcasecmp("server_keyx", tok))
        {
            flags |= SSL_CUR_SERVER_KEYX_FLAG;
            if (negated)
                mask |= SSL_CUR_SERVER_KEYX_FLAG;
        }
        else if(!strcasecmp("unknown", tok))
        {
            flags |= SSL_UNKNOWN_FLAG;
            if (negated)
                mask |= SSL_UNKNOWN_FLAG;
        }
        else 
        {
            DynamicPreprocessorFatalMessage(
                    "%s(%d) => %s is not a recognized argument to %s.\n", 
                    *(_dpd.config_file), _dpd.config_file, tok, name);
        }

    } while( (tok = strtok_r(NULL, ",", &end)) != NULL );

    sdata = (SslRuleOptData *)calloc(1, sizeof(*sdata));
    if (sdata == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                "ssl_state preprocessor rule option.\n");
    }

    sdata->flags = flags;
    sdata->mask = mask;
    *data = (void *)sdata;

    return 1;
}

/* Parsing for the ssl_version rule option */
static int SSLPP_ver_init(char *name, char *params, void **data) 
{
    int flags = 0, mask = 0;
    char *end = NULL;
    char *tok;
    SslRuleOptData *sdata;

    tok = strtok_r(params, ",", &end);

    if(!tok)
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to"
            "ssl_state keyword\n", *(_dpd.config_file), *(_dpd.config_line));

    do
    {
        int negated = 0;

        if (tok[0] == '!')
        {
            negated = 1;
            tok++;
        }

        if(!strcasecmp("sslv2", tok))
        {
            flags |= SSL_VER_SSLV2_FLAG;
            if (negated)
                mask |= SSL_VER_SSLV2_FLAG;
        }
        else if(!strcasecmp("sslv3", tok))
        {
            flags |= SSL_VER_SSLV3_FLAG;
            if (negated)
                mask |= SSL_VER_SSLV3_FLAG;
        }
        else if(!strcasecmp("tls1.0", tok))
        {
            flags |= SSL_VER_TLS10_FLAG;
            if (negated)
                mask |= SSL_VER_TLS10_FLAG;
        }
        else if(!strcasecmp("tls1.1", tok))
        {
            flags |= SSL_VER_TLS11_FLAG;
            if (negated)
                mask |= SSL_VER_TLS11_FLAG;
        }
        else if(!strcasecmp("tls1.2", tok))
        {
            flags |= SSL_VER_TLS12_FLAG;
            if (negated)
                mask |= SSL_VER_TLS12_FLAG;
        }
        else 
        {
            DynamicPreprocessorFatalMessage(
                    "%s(%d) => %s is not a recognized argument to %s.\n", 
                    *(_dpd.config_file), _dpd.config_file, tok, name);
        }

    } while( (tok = strtok_r(NULL, ",", &end)) != NULL );

    sdata = (SslRuleOptData *)calloc(1, sizeof(*sdata));
    if (sdata == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                "ssl_version preprocessor rule option.\n");
    }

    sdata->flags = flags;
    sdata->mask = mask;
    *data = (void *)sdata;

    return 1;
}

/* Rule option evaluation (for both rule options) */
static int SSLPP_rule_eval(void *raw_packet, const uint8_t **cursor, void *data)
{
    int ssn_data; 
    SFSnortPacket *p = (SFSnortPacket*)raw_packet; 
    SslRuleOptData *sdata = (SslRuleOptData *)data;

    if (!p || !p->tcp_header || !p->stream_session_ptr || !data)
        return RULE_NOMATCH; 

    ssn_data = (int)(uintptr_t)_dpd.streamAPI->get_application_data( 
            p->stream_session_ptr, PP_SSL); 

    if ((sdata->flags & ssn_data) ^ sdata->mask)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

/* SSL Preprocessor configuration parsing */
static void SSLPP_config(SSLPP_config_t *config, char *conf)
{
    char *saveptr;
    char *space_tok;
    char *comma_tok;
    char *portptr;
    char *search;
    SFP_errstr_t err;

    if(!conf) 
        return;

    if (config == NULL)
        return;
    
    search = conf;

    while( (comma_tok = strtok_r(search, ",", &saveptr)) != NULL ) 
    {
        search = NULL;

        space_tok = strtok_r(comma_tok, " ", &portptr);

        if(!space_tok)
            return;
        
        if(!strcasecmp(space_tok, "ports"))
        {
            memset(config->ports, 0, sizeof(config->ports));

            if(SFP_ports(config->ports, portptr, err) != SFP_SUCCESS)
                DynamicPreprocessorFatalMessage(
                    "%s(%d) => Failed to parse: %s\n",
                   *(_dpd.config_file), *(_dpd.config_line), SFP_GET_ERR(err));

        }
        else if(!strcasecmp(space_tok, "noinspect_encrypted")) 
        {
            char *tmpChar;
            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar)
            {
        	    DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to the"
        	                    " SSL preprocessor: '%s' in %s\n", 
        	                    *(_dpd.config_file), *(_dpd.config_line), space_tok, tmpChar);
            }
            config->flags |= SSLPP_DISABLE_FLAG;
        }
        else if(!strcasecmp(space_tok, "trustservers"))
        {
            char *tmpChar;
            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to the"
                    " SSL preprocessor: '%s' in %s\n", 
                    *(_dpd.config_file), *(_dpd.config_line), space_tok, tmpChar);
            }
            config->flags |= SSLPP_TRUSTSERVER_FLAG;
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to the"
                " SSL preprocessor: '%s' in %s\n", 
                *(_dpd.config_file), *(_dpd.config_line), comma_tok, conf);
        }
    } 

    /* Verify configured options make sense */
    if ((config->flags & SSLPP_TRUSTSERVER_FLAG) &&
        !(config->flags & SSLPP_DISABLE_FLAG))
    {
        DynamicPreprocessorFatalMessage("%s(%d) => SSL preprocessor: 'trustservers' "
            "requires 'noinspect_encrypted' to be useful.\n",
            *(_dpd.config_file), *(_dpd.config_line));
    }
}

static void SSLPP_print_config(SSLPP_config_t *config)
{
    char buf[1024];    /* For syslog printing */
    int i;
    int newline;

    if (config == NULL)
        return;

    memset(buf, 0, sizeof(buf));

    _dpd.logMsg("SSLPP config:\n");
    _dpd.logMsg("    Encrypted packets: %s\n",
           config->flags & SSLPP_DISABLE_FLAG ? "not inspected" : "inspected");

    _dpd.logMsg("    Ports:\n");

    for(newline = 0, i = 0; i < MAXPORTS; i++) 
    {
        if( config->ports[ PORT_INDEX(i) ] & CONV_PORT(i) )
        {
            SFP_snprintfa(buf, sizeof(buf), "    %5d", i);
            if( !((++newline) % 5) ) 
            {
                SFP_snprintfa(buf, sizeof(buf), "\n");
                _dpd.logMsg(buf);
                memset(buf, 0, sizeof(buf));
            }
        }
    }

    if(newline % 5)
        SFP_snprintfa(buf, sizeof(buf), "\n");

    _dpd.logMsg(buf);
    
    if ( config->flags & SSLPP_TRUSTSERVER_FLAG )
    {
        _dpd.logMsg("    Server side data is trusted\n");
    }
}

static void SSLPP_init_config(SSLPP_config_t *config)
{
    if (config == NULL)
        return;

    /* Setup default ports */
    SSLSetPort(config, 443); /* HTTPS */
    SSLSetPort(config, 465); /* SMTPS */
    SSLSetPort(config, 563); /* NNTPS */
    SSLSetPort(config, 636); /* LDAPS */
    SSLSetPort(config, 989); /* FTPS */
    SSLSetPort(config, 992); /* TelnetS */
    SSLSetPort(config, 993); /* IMAPS */
    SSLSetPort(config, 994); /* IRCS */
    SSLSetPort(config, 995); /* POPS */
}

static INLINE void SSLSetPort(SSLPP_config_t *config, int port)
{
    if (config == NULL)
        return;

    config->ports[ PORT_INDEX(port) ] |= CONV_PORT(port);  
}

static void SSLPP_drop_stats(int exiting) 
{
    if(!counts.decoded)
        return;

    _dpd.logMsg("SSL Preprocessor:\n");
    _dpd.logMsg("   SSL packets decoded: " FMTu64("-10") "\n", counts.decoded);
    _dpd.logMsg("          Client Hello: " FMTu64("-10") "\n", counts.hs_chello);
    _dpd.logMsg("          Server Hello: " FMTu64("-10") "\n", counts.hs_shello);
    _dpd.logMsg("           Certificate: " FMTu64("-10") "\n", counts.hs_cert);
    _dpd.logMsg("           Server Done: " FMTu64("-10") "\n", counts.hs_sdone);
    _dpd.logMsg("   Client Key Exchange: " FMTu64("-10") "\n", counts.hs_ckey);
    _dpd.logMsg("   Server Key Exchange: " FMTu64("-10") "\n", counts.hs_skey);
    _dpd.logMsg("         Change Cipher: " FMTu64("-10") "\n", counts.cipher_change);
    _dpd.logMsg("              Finished: " FMTu64("-10") "\n", counts.hs_finished);
    _dpd.logMsg("    Client Application: " FMTu64("-10") "\n", counts.capp);
    _dpd.logMsg("    Server Application: " FMTu64("-10") "\n", counts.sapp);
    _dpd.logMsg("                 Alert: " FMTu64("-10") "\n", counts.alerts);
    _dpd.logMsg("  Unrecognized records: " FMTu64("-10") "\n", counts.unrecognized);
    _dpd.logMsg("  Completed handshakes: " FMTu64("-10") "\n", counts.completed_hs);
    _dpd.logMsg("        Bad handshakes: " FMTu64("-10") "\n", counts.bad_handshakes);
    _dpd.logMsg("      Sessions ignored: " FMTu64("-10") "\n", counts.stopped);
    _dpd.logMsg("    Detection disabled: " FMTu64("-10") "\n", counts.disabled);
}

static void SSLPP_init(char *args)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy();
    SSLPP_config_t *pPolicyConfig = NULL;

    if (ssl_config == NULL)
    {
        //create a context
        ssl_config = sfPolicyConfigCreate();

        if (ssl_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                                            "SSL preprocessor configuration.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage(
                                            "SSLPP_init(): The Stream preprocessor must be enabled.\n");
        }

        memset(&counts, 0, sizeof(counts));

        _dpd.registerPreprocStats("ssl", SSLPP_drop_stats);
        _dpd.addPreprocConfCheck(SSLPP_CheckConfig);
        _dpd.addPreprocExit(SSLCleanExit, NULL, PRIORITY_LAST, PP_SSL);
        _dpd.addPreprocResetStats(SSLResetStats, NULL, PRIORITY_LAST, PP_SSL);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("ssl", (void *)&sslpp_perf_stats, 0, _dpd.totalPerfStats);
#endif

#ifdef TARGET_BASED
        ssl_app_id = _dpd.findProtocolReference("ssl");
        if (ssl_app_id == SFTARGET_UNKNOWN_PROTOCOL)
        {
            ssl_app_id = _dpd.addProtocolReference("ssl");
        }
#endif
    }

    sfPolicyUserPolicySet (ssl_config, policy_id);
    pPolicyConfig = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("SSL preprocessor can only be "
                                        "configured once.\n");
    }

    pPolicyConfig = (SSLPP_config_t *)calloc(1, sizeof(SSLPP_config_t));
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                                        "SSL preprocessor configuration.\n");
    }
 
    sfPolicyUserDataSetCurrent(ssl_config, pPolicyConfig);

    SSLPP_init_config(pPolicyConfig);
	SSLPP_config(pPolicyConfig, args);
    SSLPP_print_config(pPolicyConfig);

    _dpd.preprocOptRegister("ssl_state", SSLPP_state_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister("ssl_version", SSLPP_ver_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);

	_dpd.addPreproc( SSLPP_process, PRIORITY_TUNNEL, PP_SSL, PROTO_BIT__TCP );

    _addPortsToStream5Filter(pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStream5Filter(policy_id);
#endif
}

void SetupSSLPP(void)
{
#ifndef SNORT_RELOAD
	_dpd.registerPreproc( "ssl", SSLPP_init);
#else
	_dpd.registerPreproc("ssl", SSLPP_init, SSLReload,
                         SSLReloadSwap, SSLReloadSwapFree);
#endif
}

#if DEBUG
static void SSL_PrintFlags(uint32_t flags)
{
    if (flags & SSL_CHANGE_CIPHER_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CHANGE_CIPHER_FLAG\n"););
    }

    if (flags & SSL_ALERT_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_ALERT_FLAG\n"););
    }

    if (flags & SSL_POSSIBLE_HS_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_POSSIBLE_HS_FLAG\n"););
    }

    if (flags & SSL_CLIENT_HELLO_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CLIENT_HELLO_FLAG\n"););
    }

    if (flags & SSL_SERVER_HELLO_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_SERVER_HELLO_FLAG\n"););
    }

    if (flags & SSL_CERTIFICATE_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CERTIFICATE_FLAG\n"););
    }

    if (flags & SSL_SERVER_KEYX_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_SERVER_KEYX_FLAG\n"););
    }

    if (flags & SSL_CLIENT_KEYX_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CLIENT_KEYX_FLAG\n"););
    }

    if (flags & SSL_CIPHER_SPEC_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CIPHER_SPEC_FLAG\n"););
    }

    if (flags & SSL_SFINISHED_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_SFINISHED_FLAG\n"););
    }

    if (flags & SSL_SAPP_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_SAPP_FLAG\n"););
    }

    if (flags & SSL_CAPP_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CAPP_FLAG\n"););
    }

    if (flags & SSL_HS_SDONE_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_HS_SDONE_FLAG\n"););
    }

    if (flags & SSL_POSSIBLY_ENC_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_POSSIBLY_ENC_FLAG\n"););
    }

    if (flags & SSL_VER_SSLV2_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_VER_SSLV2_FLAG\n"););
    }

    if (flags & SSL_VER_SSLV3_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_VER_SSLV3_FLAG\n"););
    }

    if (flags & SSL_VER_TLS10_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_VER_TLS10_FLAG\n"););
    }

    if (flags & SSL_VER_TLS11_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_VER_TLS11_FLAG\n"););
    }

    if (flags & SSL_VER_TLS12_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_VER_TLS12_FLAG\n"););
    }

#if 0
SSL_VERFLAGS (SSL_VER_SSLV2_FLAG | SSL_VER_SSLV3_FLAG | \
              SSL_VER_TLS10_FLAG | SSL_VER_TLS11_FLAG | \
              SSL_VER_TLS12_FLAG)
#endif

    if (flags & SSL_CUR_CLIENT_HELLO_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CUR_CLIENT_HELLO_FLAG\n"););
    }

    if (flags & SSL_CUR_SERVER_HELLO_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CUR_SERVER_HELLO_FLAG\n"););
    }

    if (flags & SSL_CUR_SERVER_KEYX_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CUR_SERVER_KEYX_FLAG\n"););
    }

    if (flags & SSL_CUR_CLIENT_KEYX_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_CUR_CLIENT_KEYX_FLAG\n"););
    }

    if (flags & SSL_ENCRYPTED_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_ENCRYPTED_FLAG\n"););
    }

    if (flags & SSL_UNKNOWN_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_UNKNOWN_FLAG\n"););
    }

#if 0
SSL_STATEFLAGS (SSL_CUR_CLIENT_HELLO_FLAG | SSL_CUR_SERVER_HELLO_FLAG | \
                SSL_CUR_SERVER_KEYX_FLAG | SSL_CUR_CLIENT_KEYX_FLAG | \
                SSL_UNKNOWN_FLAG)
#endif

    if (flags & SSL_BOGUS_HS_DIR_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_BOGUS_HS_DIR_FLAG\n"););
    }

    if (flags & SSL_TRAILING_GARB_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_TRAILING_GARB_FLAG\n"););
    }

    if (flags & SSL_BAD_TYPE_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_BAD_TYPE_FLAG\n"););
    }

    if (flags & SSL_BAD_VER_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_BAD_VER_FLAG\n"););
    }

    if (flags & SSL_TRUNCATED_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_TRUNCATED_FLAG\n"););
    }

    if (flags == SSL_ARG_ERROR_FLAG)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL_ARG_ERROR_FLAG\n"););
    }
}
#endif

static void _addPortsToStream5Filter(SSLPP_config_t *config, tSfPolicyId policy_id)
{
    unsigned int portNum;

    if (config == NULL)
        return;

    for (portNum = 0; portNum < MAXPORTS; portNum++)
    {
        if(config->ports[(portNum/8)] & (1<<(portNum%8)))
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status
                (IPPROTO_TCP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
        }
    }
}
#ifdef TARGET_BASED
static void _addServicesToStream5Filter(tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status
        (ssl_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int SSLFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    SSLPP_config_t *pPolicyConfig = (SSLPP_config_t *)pData;

    //do any housekeeping before freeing SSLPP_config_t

    sfPolicyUserDataClear (config, policyId);
    free(pPolicyConfig);
    return 0;
}

static void SSLFreeConfig(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataIterate (config, SSLFreeConfigPolicy);
    sfPolicyConfigDelete(config);
}

static void SSLCleanExit(int signal, void *data)
{
    if (ssl_config != NULL)
    {
        SSLFreeConfig(ssl_config);
        ssl_config = NULL;
    }
}

static void SSLResetStats(int signal, void *data)
{
    memset(&counts, 0, sizeof(counts));
}

static int SSLPP_CheckPolicyConfig(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    _dpd.setParserPolicy(policyId);

    if (!_dpd.isPreprocEnabled(PP_STREAM5))
    {
        DynamicPreprocessorFatalMessage(
            "SSLPP_CheckPolicyConfig(): The Stream preprocessor must be enabled.\n");
    }
    return 0;
}

static void SSLPP_CheckConfig(void)
{
    sfPolicyUserDataIterate (ssl_config, SSLPP_CheckPolicyConfig);
}

#ifdef SNORT_RELOAD
static void SSLReload(char *args)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy();
    SSLPP_config_t * pPolicyConfig = NULL;

    if (ssl_swap_config == NULL)
    {
        //create a context
        ssl_swap_config = sfPolicyConfigCreate();

        if (ssl_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                                            "SSL preprocessor configuration.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage(
                                            "SSLPP_init(): The Stream preprocessor must be enabled.\n");
        }
    }

    sfPolicyUserPolicySet (ssl_swap_config, policy_id);
    pPolicyConfig = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_swap_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("SSL preprocessor can only be "
                                        "configured once.\n");
    }

    pPolicyConfig = (SSLPP_config_t *)calloc(1, sizeof(SSLPP_config_t));
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                                        "SSL preprocessor configuration.\n");
    }

    sfPolicyUserDataSetCurrent(ssl_swap_config, pPolicyConfig);

    SSLPP_init_config(pPolicyConfig);
	SSLPP_config(pPolicyConfig, args);
    SSLPP_print_config(pPolicyConfig);

    _dpd.preprocOptRegister("ssl_state", SSLPP_state_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister("ssl_version", SSLPP_ver_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);

	_dpd.addPreproc(SSLPP_process, PRIORITY_TUNNEL, PP_SSL, PROTO_BIT__TCP);
    _dpd.addPreprocReloadVerify(SSLReloadVerify);

    _addPortsToStream5Filter(pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStream5Filter(policy_id);
#endif
}

static int SSLReloadVerify(void)
{
    if (!_dpd.isPreprocEnabled(PP_STREAM5))
    {
        DynamicPreprocessorFatalMessage(
            "SSLPP_init(): The Stream preprocessor must be enabled.\n");
    }

    return 0;
}


static void * SSLReloadSwap(void)
{
    tSfPolicyUserContextId old_config = ssl_config;

    if (ssl_swap_config == NULL)
        return NULL;

    ssl_config = ssl_swap_config;
    ssl_swap_config = NULL;

    return (void *)old_config;
}

static void SSLReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    SSLFreeConfig((tSfPolicyUserContextId)data);
}
#endif
