/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc.
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
 */

/*
 * File: ssl_inspect.c
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 * Brief: SSL traffic inspection
*/

#include "ssl_inspect.h"
#include "ssl_session.h"
#include "sf_types.h"
#ifdef ENABLE_HA
#include "ssl_ha.h"
#endif
#include <assert.h>

#ifdef DUMP_BUFFER
#include "../ssl/ssl_buffer_dump.h"
#endif

/* Ultimately calls SnortEventqAdd */
/* Arguments are: gid, sid, rev, classification, priority, message, rule_info */
#define ALERT(x,y) { _dpd.alertAdd(GENERATOR_SPP_SSLPP, x, 1, 0, 3, y, 0 ); }

/* Wraps disabling detect with incrementing the counter */
#define DISABLE_DETECT() { _dpd.disableDetect(packet); counts.disabled++; }

#define SSL_ERROR_FLAGS  (SSL_BOGUS_HS_DIR_FLAG | \
                               SSL_BAD_VER_FLAG | \
                               SSL_BAD_TYPE_FLAG | \
                               SSL_UNKNOWN_FLAG)

#define SSL3_FIRST_BYTE 0x16
#define SSL3_SECOND_BYTE 0x03
#define SSL2_CHELLO_BYTE 0x01
#define SSL2_SHELLO_BYTE 0x04

/* very simplistic - just enough to say this is binary data - the rules will make a final
 * judgement.  Should maybe add an option to the imap configuration to enable the
 * continuing of command inspection like ftptelnet. */
bool IsTlsClientHello(const uint8_t *ptr, const uint8_t *end)
{
    /* at least 3 bytes of data - see below */
    if ((end - ptr) < 3)
        return false;

    if ((ptr[0] == SSL3_FIRST_BYTE) && (ptr[1] == SSL3_SECOND_BYTE))
    {
        /* TLS v1 or SSLv3 */
        return true;
    }
    else if ((ptr[2] == SSL2_CHELLO_BYTE) || (ptr[3] == SSL2_CHELLO_BYTE))
    {
        /* SSLv2 */
        return true;
    }

    return false;
}

/* this may at least tell us whether the server accepted the client hello by the presence
 * of binary data */
bool IsTlsServerHello(const uint8_t *ptr, const uint8_t *end)
{
    /* at least 3 bytes of data - see below */
    if ((end - ptr) < 3)
        return false;

    if ((ptr[0] == SSL3_FIRST_BYTE) && (ptr[1] == SSL3_SECOND_BYTE))
    {
        /* TLS v1 or SSLv3 */
        return true;
    }
    else if (ptr[2] == SSL2_SHELLO_BYTE)
    {
        /* SSLv2 */
        return true;
    }

    return false;
}

bool IsSSL(const uint8_t *ptr, int len, int pkt_flags)
{
    uint32_t ssl_flags = SSL_decode(ptr, len, pkt_flags, 0, NULL, NULL, 0);

    if ((ssl_flags != SSL_ARG_ERROR_FLAG) &&
            !(ssl_flags & SSL_ERROR_FLAGS))
    {
        return true;
    }

    return false;
}



static void SSL_SsnFree(void *);
static SSL_SsnData *SSL_NewSession(SFSnortPacket *);
void SSL_Set_flow_id( void *app_data, uint32_t fid );

static SSLPP_counters_t counts;


void SSL_InitGlobals(void)
{
    memset(&counts, 0, sizeof(counts));
}

#ifdef DEBUG_MSGS
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


static inline bool SSLPP_is_encrypted(uint32_t ssl_flags, SFSnortPacket *packet)
{
    SSLPP_config_t *config = NULL;

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    if (config->flags & SSLPP_TRUSTSERVER_FLAG)
    {
        if(ssl_flags & SSL_SAPP_FLAG)
            return true;
    }

    if (SSL_IS_CLEAN(ssl_flags))
    {
        if (((ssl_flags & SSLPP_ENCRYPTED_FLAGS) == SSLPP_ENCRYPTED_FLAGS) ||
            ((ssl_flags & SSLPP_ENCRYPTED_FLAGS2) == SSLPP_ENCRYPTED_FLAGS2))
        {
            counts.completed_hs++;
            return true;
        }
        /* Check if we're either midstream or if packets were missed after the
         * connection was established */
        else if ((_dpd.sessionAPI->get_session_flags (packet->stream_session) & SSNFLAG_MIDSTREAM) ||
                 (_dpd.streamAPI->missed_packets(packet->stream_session, SSN_DIR_BOTH)))
        {
            if ((ssl_flags & (SSL_CAPP_FLAG | SSL_SAPP_FLAG)) == (SSL_CAPP_FLAG | SSL_SAPP_FLAG))
            {
                return true;
            }
        }
    }

    return false;
}

static inline uint32_t SSLPP_process_alert(
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
       (config->flags & SSLPP_DISABLE_FLAG) && !(new_flags & SSL_HEARTBEAT_SEEN))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Disabling detect\n"););
        DISABLE_DETECT();
    }

#ifdef DUMP_BUFFER
    dumpBuffer(SSL_PROCESS_ALERT_DUMP,packet->payload,packet->payload_size);
#endif
    /* Need to negate the application flags from the opposing side. */

    if(packet->flags & FLAG_FROM_CLIENT)
        return ssn_flags & ~SSL_SAPP_FLAG;

    else if(packet->flags & FLAG_FROM_SERVER)
        return ssn_flags & ~SSL_CAPP_FLAG;

    return ssn_flags;
}

static inline uint32_t SSLPP_process_hs(uint32_t ssl_flags, uint32_t new_flags)
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

static inline uint32_t SSLPP_process_app(
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

        // Heartbleed check is disabled. Stop inspection on this session.
        if(!config->max_heartbeat_len)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "STOPPING INSPECTION (process_app)\n"););
            _dpd.sessionAPI->stop_inspection(packet->stream_session,
                               packet, SSN_DIR_BOTH, -1, 0);
            counts.stopped++;
        }
        else if(!(new_flags & SSL_HEARTBEAT_SEEN))
        {
            DISABLE_DETECT();
        }

    }

#ifdef DUMP_BUFFER
        dumpBuffer(SSL_PROCESS_APP_DUMP,packet->payload,packet->payload_size);
#endif
    return ssn_flags | new_flags;
}

static inline void SSLPP_process_other(
        SSL_SsnData *sd, uint32_t new_flags, SFSnortPacket *packet)
{
    SSLPP_config_t *config = NULL;

    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    /* Encrypted SSLv2 will appear unrecognizable.  Check if the handshake was
     * seen and stop inspecting if so. */
    /* Check for an existing handshake from both sides */
    if((sd->ssn_flags & SSL_VER_SSLV2_FLAG) &&
       SSL_IS_CHELLO(sd->ssn_flags) && SSL_IS_SHELLO(sd->ssn_flags) &&
       (config->flags & SSLPP_DISABLE_FLAG) && !(new_flags & SSL_CHANGE_CIPHER_FLAG) && !(new_flags & SSL_HEARTBEAT_SEEN))
    {
        sd->ssn_flags |= SSL_ENCRYPTED_FLAG | new_flags;

        if(!config->max_heartbeat_len)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "STOPPING INSPECTION (process_other)\n"););
            _dpd.sessionAPI->stop_inspection(packet->stream_session,
                                            packet, SSN_DIR_BOTH, -1, 0);
        }
        else if(!(new_flags & SSL_HEARTBEAT_SEEN))
        {
            DISABLE_DETECT();
        } 
    }
    else
    {
        counts.unrecognized++;

        /* Special handling for SSLv2 */
        if(new_flags & SSL_VER_SSLV2_FLAG)
            sd->ssn_flags |= new_flags;

        if(new_flags & SSL_UNKNOWN_FLAG)
            sd->ssn_flags |= new_flags;

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
            if(packet->stream_session &&
                _dpd.streamAPI->missed_packets(
                    packet->stream_session, SSN_DIR_SERVER) &&
                _dpd.streamAPI->missed_packets(
                    packet->stream_session, SSN_DIR_CLIENT) )

                ssn_flags |= SSL_VER_SSLV2_FLAG;
        }
#endif
    }

#ifdef DUMP_BUFFER
        dumpBuffer(SSL_PROCESS_OTHER_DUMP,packet->payload,packet->payload_size);
#endif
}

/* SSL Preprocessor process callback. */
void SSLPP_process(void *raw_packet, void *context)
{
    SFSnortPacket *packet;
    uint32_t new_flags;
    SSL_SsnData *sd = NULL;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif
    uint8_t heartbleed_type = 0;
    SSLPP_config_t *config = NULL;
    PROFILE_VARS;
    sfPolicyUserPolicySet (ssl_config, _dpd.getNapRuntimePolicy());
    config = (SSLPP_config_t *)sfPolicyUserDataGetCurrent(ssl_config);

    if (config == NULL)
        return;


    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL Start ================================\n"););

    packet = (SFSnortPacket*)raw_packet;

    assert(IsTCP(packet) && packet->payload && packet->payload_size);

    if(!packet->stream_session)
    {
#ifdef DEBUG_MSGS
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - Not inspecting packet\n"););
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
#endif
        return;
    }

    if ( ssl_cb && ssl_cb->is_session_ssl( packet ) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL - SSL Packet\n"););
    }
    else
    {
#ifdef TARGET_BASED
    /* Check packet based on protocol number */
        app_id = _dpd.sessionAPI->get_application_protocol_id(packet->stream_session);
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
    }

#ifdef DEBUG_MSGS
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
    /* Flush opposite direction to keep conversation in sync */
    if (!(packet->flags & FLAG_REBUILT_STREAM))
    {
        switch (_dpd.streamAPI->get_reassembly_direction(packet->stream_session))
        {
            case SSN_DIR_TO_SERVER:
                if (packet->flags & FLAG_FROM_SERVER)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Flushing server side\n"););
                    _dpd.streamAPI->response_flush_stream(packet);
                }

                break;

            case SSN_DIR_TO_CLIENT:
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
    sd = (SSL_SsnData *)SSL_GetAppData(packet);
    if (sd == NULL)
    {
        sd = SSL_NewSession(packet);
        if (sd == NULL)
        {
            PREPROC_PROFILE_END(sslpp_perf_stats);
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
            return;
        }
        sd->is_ssl = true;
    }

#ifdef DEBUG_MSGS
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Ssn flags before ----------------------\n"););
    SSL_PrintFlags(sd->ssn_flags);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif

    SSL_CLEAR_TEMPORARY_FLAGS(sd->ssn_flags);

#ifdef DEBUG_MSGS
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

    if(ssl_cb && !(ssl_cb->get_ssl_flow_flags(packet, sd, &new_flags)))
    {
#ifdef DEBUG_MSGS
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Skipping SSL_decode\n"););
#endif
    }
    else
    {
        // Indexing into an array of partial record lengths partial_rec_len[raw+c][raw+s][reassm+c][reassm+s]
        uint8_t dir = (packet->flags & FLAG_FROM_SERVER)? 1 : 0;
        uint8_t index = (packet->flags & FLAG_REBUILT_STREAM)? 2 : 0;
        new_flags = SSL_decode(packet->payload, (int)packet->payload_size, packet->flags, sd->ssn_flags, &heartbleed_type, &(sd->partial_rec_len[dir+index]), config->max_heartbeat_len);
#ifdef DUMP_BUFFER
        dumpBuffer(SSL_DECODE_DUMP,packet->payload,packet->payload_size);
#endif
        if(heartbleed_type & SSL_HEARTBLEED_REQUEST)
        {
            ALERT(SSL_ALERT_HB_REQUEST, SSL_HEARTBLEED_REQUEST_STR);
        }
        else if(heartbleed_type & SSL_HEARTBLEED_RESPONSE)
        {
            ALERT(SSL_ALERT_HB_RESPONSE, SSL_HEARTBLEED_RESPONSE_STR);
        }
        else if(heartbleed_type & SSL_HEARTBLEED_UNKNOWN)
        {
            if(!dir)
            {
                ALERT(SSL_ALERT_HB_REQUEST, SSL_HEARTBLEED_REQUEST_STR);
            }
            else
            {
                ALERT(SSL_ALERT_HB_RESPONSE, SSL_HEARTBLEED_RESPONSE_STR);
            }
        }
    }

    if(sd->ssn_flags & SSL_ENCRYPTED_FLAG )
    {
        counts.decoded++;
#ifdef DEBUG_MSGS
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "New flags -----------------------------\n"););
        SSL_PrintFlags(new_flags);
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif

        SSL_UpdateCounts(new_flags);

        if(!(new_flags & SSL_HEARTBEAT_SEEN))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Disabling detect\n"););
            DISABLE_DETECT();

            PREPROC_PROFILE_END(sslpp_perf_stats);
            DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
        }

        sd->ssn_flags |= new_flags;

#ifdef DEBUG_MSGS
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Ssn flags after -----------------------\n"););
        SSL_PrintFlags(sd->ssn_flags);
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif
        return;

    }

    if(SSL_IS_CHELLO(new_flags))
    {
        if(ssl_cb)
            ssl_cb->session_initialize(packet, sd, SSL_Set_flow_id);
    }

    // If the client used an SSLv2 ClientHello with an SSLv3/TLS version and
    // the server replied with an SSLv3/TLS ServerHello, remove the backward
    // compatibility flag and the SSLv2 flag since this session will continue
    // as SSLv3/TLS.
    if ((sd->ssn_flags & SSL_V3_BACK_COMPAT_V2) && SSL_V3_SERVER_HELLO(new_flags))
        sd->ssn_flags &= ~(SSL_VER_SSLV2_FLAG|SSL_V3_BACK_COMPAT_V2);

    if( SSL_IS_CHELLO(new_flags) && SSL_IS_CHELLO(sd->ssn_flags) && SSL_IS_SHELLO(sd->ssn_flags) )
    {
        ALERT(SSL_INVALID_CLIENT_HELLO, SSL_INVALID_CLIENT_HELLO_STR);
    }
    else if(!(config->flags & SSLPP_TRUSTSERVER_FLAG))
    {
        if( (SSL_IS_SHELLO(new_flags) && !SSL_IS_CHELLO(sd->ssn_flags) ))
        {
            if(!(_dpd.streamAPI->missed_packets( packet->stream_session, SSN_DIR_FROM_CLIENT)))
                ALERT(SSL_INVALID_SERVER_HELLO, SSL_INVALID_SERVER_HELLO_STR);
        }
    }


    counts.decoded++;

#ifdef DEBUG_MSGS
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
        sd->ssn_flags = SSLPP_process_alert(sd->ssn_flags, new_flags, packet);
    }
    else if(SSL_IS_HANDSHAKE(new_flags))
    {
        sd->ssn_flags = SSLPP_process_hs(sd->ssn_flags, new_flags);
    }
    else if(SSL_IS_APP(new_flags))
    {
        sd->ssn_flags = SSLPP_process_app(sd->ssn_flags, new_flags, packet);
    }
    else
    {
        /* Different record type that we don't care about.
         * Either it's a 'change cipher spec' or we failed to recognize the
         * record type.  Do not update session data */
        SSLPP_process_other(sd, new_flags, packet);

        /* Application data is updated inside of SSLPP_process_other */

        PREPROC_PROFILE_END(sslpp_perf_stats);
        DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
        return;
    }

    sd->ssn_flags |= new_flags;

#ifdef DEBUG_MSGS
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "Ssn flags after -----------------------\n"););
    SSL_PrintFlags(sd->ssn_flags);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "---------------------------------------\n"););
#endif

    PREPROC_PROFILE_END(sslpp_perf_stats);
    DEBUG_WRAP(DebugMessage(DEBUG_SSL, "SSL End ================================\n"););
}

/* Rule option evaluation (for both rule options) */
int SSLPP_rule_eval(void *raw_packet, const uint8_t **cursor, void *data)
{
    SSL_SsnData *sd;
    SFSnortPacket *p = (SFSnortPacket*)raw_packet;
    SslRuleOptData *sdata = (SslRuleOptData *)data;

    if (!p || !p->tcp_header || !p->stream_session || !data)
        return RULE_NOMATCH;

    sd = (SSL_SsnData *)SSL_GetAppData(p);
    if(!sd)
        return RULE_NOMATCH;

    if ((sdata->flags & sd->ssn_flags) ^ sdata->mask)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

void DisplaySSLPPStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int len = 0;

    if(counts.decoded) {
        len += snprintf(buffer, CS_STATS_BUF_SIZE, "SSL Preprocessor:\n"
                "   SSL packets decoded: " FMTu64("-10") "\n"
                "          Client Hello: " FMTu64("-10") "\n"
                "          Server Hello: " FMTu64("-10") "\n"
                "           Certificate: " FMTu64("-10") "\n"
                "           Server Done: " FMTu64("-10") "\n"
                "   Client Key Exchange: " FMTu64("-10") "\n"
                "   Server Key Exchange: " FMTu64("-10") "\n"
                "         Change Cipher: " FMTu64("-10") "\n"
                "              Finished: " FMTu64("-10") "\n"
                "    Client Application: " FMTu64("-10") "\n"
                "    Server Application: " FMTu64("-10") "\n"
                "                 Alert: " FMTu64("-10") "\n"
                "  Unrecognized records: " FMTu64("-10") "\n"
                "  Completed handshakes: " FMTu64("-10") "\n"
                "        Bad handshakes: " FMTu64("-10") "\n"
                "      Sessions ignored: " FMTu64("-10") "\n"
                "    Detection disabled: " FMTu64("-10") "\n"
                , counts.decoded
                , counts.hs_chello
                , counts.hs_shello
                , counts.hs_cert
                , counts.hs_sdone
                , counts.hs_ckey
                , counts.hs_skey
                , counts.cipher_change
                , counts.hs_finished
                , counts.capp
                , counts.sapp
                , counts.alerts
                , counts.unrecognized
                , counts.completed_hs
                , counts.bad_handshakes
                , counts.stopped
                , counts.disabled);
#ifdef ENABLE_HA
        len += DisplaySSLHAStats(buffer + len);
#endif
    } else {
        len  = snprintf(buffer, CS_STATS_BUF_SIZE, "SSL Packet Details Not available\n SSL packets decoded: " FMTu64("-10") "\n", counts.decoded);
    }

    if (-1 == f(te, (const uint8_t *)buffer, len)) {
        _dpd.logMsg("Unable to send data to the frontend\n");
    }
}

void SSLPP_drop_stats(int exiting)
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

#ifdef ENABLE_HA
     SSLPrintHAStats();
#endif
}

static void SSL_SsnFree(void *data)
{
    SSL_SsnData *sd = (SSL_SsnData *)data;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    if(sd == NULL)
        return;
    if(ssl_cb)
        ssl_cb->session_free(sd->flow_id);
    free(sd);

    return;
}

static SSL_SsnData *SSL_NewSession(SFSnortPacket *p)
{
    SSL_SsnData *sd = NULL;
    if (p->stream_session == NULL)
        return NULL;
    sd = (SSL_SsnData *)calloc(1, sizeof(SSL_SsnData));
    if(sd == NULL)
        return NULL;
    sd->flow_id = 0;
    SSL_SetAppData(p, (void *)sd, SSL_SsnFree);
    return sd;
}

void SSL_Set_flow_id( void *app_data, uint32_t fid )
{
    SSL_SsnData *sd = (SSL_SsnData *)app_data;
    if( sd )
        sd->flow_id = fid;
}
