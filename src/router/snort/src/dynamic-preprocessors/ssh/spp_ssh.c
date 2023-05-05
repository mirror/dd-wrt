/* $Id */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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
 * SSH preprocessor
 * Author: Chris Sherwin
 * Contributors: Adam Keeton, Ryan Jordan
 *
 *
 * Alert for Gobbles, CRC32, protocol mismatch (Cisco catalyst vulnerability),
 * and a SecureCRT vulnerability.  Will also alert if the client or server
 * traffic appears to flow the wrong direction, or if packets appear
 * malformed/spoofed.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "snort_debug.h"

#include "preprocids.h"
#if defined(FEAT_OPEN_APPID)
#include "appId.h"
#endif /* defined(FEAT_OPEN_APPID) */
#include "spp_ssh.h"
#include "sf_preproc_info.h"

#include <assert.h>
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
#ifdef PERF_PROFILING
PreprocStats sshPerfStats;
#endif

#include "sf_types.h"

#ifdef DUMP_BUFFER
#include "ssh_buffer_dump.h"
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 3;
const char *PREPROC_NAME = "SF_SSH";

#define SetupSSH DYNAMIC_PREPROC_SETUP

#ifdef TARGET_BASED
int16_t ssh_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

/*
 * Generator id. Define here the same as the official registry
 * in generators.h
 */
#define GENERATOR_SPP_SSH    128

/*
 * Function prototype(s)
 */
SSHData * SSHGetNewSession(SFSnortPacket *, tSfPolicyId);
static void SSHInit( struct _SnortConfig *, char* );
static void DisplaySSHConfig(SSHConfig *);
static void FreeSSHData( void* );
static void  ParseSSHArgs(SSHConfig *, u_char*);
static void ProcessSSH( void*, void* );
static inline int CheckSSHPort( uint16_t );
static unsigned int ProcessSSHProtocolVersionExchange( SSHData*, SFSnortPacket*,
        uint8_t, uint8_t );
static unsigned int ProcessSSHKeyExchange( SSHData*, SFSnortPacket*, uint8_t, unsigned int );
static unsigned int ProcessSSHKeyInitExchange( SSHData*, SFSnortPacket*, uint8_t, unsigned int);
static void enablePortStreamServices(struct _SnortConfig *, SSHConfig *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static void SSHFreeConfig(tSfPolicyUserContextId config);
static int SSHCheckConfig(struct _SnortConfig *);
static void SSHCleanExit(int, void *);

/* Ultimately calls SnortEventqAdd */
/* Arguments are: gid, sid, rev, classification, priority, message, rule_info */
#define ALERT(x,y) { _dpd.alertAdd(GENERATOR_SPP_SSH, x, 1, 0, 3, y, 0 ); }

/* Convert port value into an index for the ssh_config->ports array */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

/** SSH configuration per Policy
 */
static tSfPolicyUserContextId ssh_config = NULL;
static SSHConfig *ssh_eval_config = NULL;

#ifdef SNORT_RELOAD
static void SSHReload(struct _SnortConfig *, char *, void **);
static int SSHReloadVerify(struct _SnortConfig *, void *);
static void * SSHReloadSwap(struct _SnortConfig *, void *);
static void SSHReloadSwapFree(void *);
#endif

/* Called at preprocessor setup time. Links preprocessor keyword
 * to corresponding preprocessor initialization function.
 *
 * PARAMETERS:    None.
*
 * RETURNS:    Nothing.
 *
 */
void SetupSSH(void)
{
    /* Link preprocessor keyword to initialization function
      * in the preprocessor list. */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc( "ssh", SSHInit );
#else
    _dpd.registerPreproc("ssh", SSHInit, SSHReload,
                         SSHReloadVerify, SSHReloadSwap, SSHReloadSwapFree);
#endif

#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getSSHBuffers, SSH_BUFFER_DUMP_FUNC);
#endif
}

#ifdef REG_TEST
static inline void PrintSSHSize(void)
{
    _dpd.logMsg("\nSSH Session Size: %lu\n", (long unsigned int)sizeof(SSHData));
}
#endif

/* Initializes the SSH preprocessor module and registers
 * it in the preprocessor list.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to argument string to process for config
 *                      data.
 *
 * RETURNS:     Nothing.
 */
static void SSHInit(struct _SnortConfig *sc, char *argp)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SSHConfig *pPolicyConfig = NULL;

#ifdef REG_TEST
    PrintSSHSize();
#endif

    if (ssh_config == NULL)
    {
        //create a context
        ssh_config = sfPolicyConfigCreate();
        if (ssh_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                                            "for SSH config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("SetupSSH(): The Stream preprocessor must be enabled.\n");
        }

        _dpd.addPreprocConfCheck(sc, SSHCheckConfig);
        _dpd.addPreprocExit(SSHCleanExit, NULL, PRIORITY_LAST, PP_SSH);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("ssh", (void *)&sshPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

#ifdef TARGET_BASED
        ssh_app_id = _dpd.findProtocolReference("ssh");
        if (ssh_app_id == SFTARGET_UNKNOWN_PROTOCOL)
            ssh_app_id = _dpd.addProtocolReference("ssh");

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_SSH, ssh_app_id );

#endif
    }

    sfPolicyUserPolicySet (ssh_config, policy_id);
    pPolicyConfig = (SSHConfig *)sfPolicyUserDataGetCurrent(ssh_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("SSH preprocessor can only be "
                                        "configured once.\n");
    }

    pPolicyConfig = (SSHConfig *)calloc(1, sizeof(SSHConfig));
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                        "SSH preprocessor configuration.\n");
    }

    sfPolicyUserDataSetCurrent(ssh_config, pPolicyConfig);

    ParseSSHArgs(pPolicyConfig, (u_char *)argp);

    _dpd.addPreproc( sc, ProcessSSH, PRIORITY_APPLICATION, PP_SSH, PROTO_BIT__TCP );

    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

/* Parses a single numerical value.
 * A fatal error is made if the parsed value is out of bounds.
 *
 * PARAMETERS:
 *
 * token:       String containing argument
 * keyword:     String containing option's name. Used when printing an error.
 * min:         Minimum value of argument
 * max:         Maximum value of argument
 *
 * RETURNS:     bounds-checked integer value of argument.
 */
static int
ParseNumInRange(char *token, char *keyword, int min, int max)
{
    int value;

    if (( !token ) || !isdigit((int)token[0]) )
    {
        DynamicPreprocessorFatalMessage("Bad value specified for %s. "
                "Please specify a number between %d and %d.\n",
                keyword, min, max);
    }

    value = atoi( token );

    if (value < min || value > max)
    {
        DynamicPreprocessorFatalMessage("Value specified for %s is out of "
                "bounds.  Please specify a number between %d and %d.\n",
                keyword, min, max);
    }

    return value;
}
/* Parses and processes the configuration arguments
 * supplied in the SSH preprocessor rule.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to string containing the config arguments.
 *
 * RETURNS:     Nothing.
 */
static void
ParseSSHArgs(SSHConfig *config, u_char* argp)
{
    char* cur_tokenp = NULL;
    char* argcpyp = NULL;
    int port;

    if (config == NULL)
        return;

    config->MaxEncryptedPackets = SSH_DEFAULT_MAX_ENC_PKTS;
    config->MaxClientBytes = SSH_DEFAULT_MAX_CLIENT_BYTES;
    config->MaxServerVersionLen = SSH_DEFAULT_MAX_SERVER_VERSION_LEN;

    /* Set up default port to listen on */
    config->ports[ PORT_INDEX( 22 ) ] |= CONV_PORT(22);

    /* Sanity check(s) */
    if ( !argp )
    {
        DisplaySSHConfig(config);
        return;
    }

    argcpyp = strdup( (char*) argp );

    if ( !argcpyp )
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse SSH options.\n");
        return;
    }

    cur_tokenp = strtok( argcpyp, " ");

    while ( cur_tokenp )
    {
        if ( !strcmp( cur_tokenp, SSH_SERVERPORTS_KEYWORD ))
        {
            /* If the user specified ports, remove '22' for now since
             * it now needs to be set explicitely. */
            config->ports[ PORT_INDEX( 22 ) ] = 0;

            /* Eat the open brace. */
            cur_tokenp = strtok( NULL, " ");
            if (( !cur_tokenp ) || ( cur_tokenp[0] != '{' ))
            {
                DynamicPreprocessorFatalMessage("Bad value specified for %s.\n",
                                                SSH_SERVERPORTS_KEYWORD);
                //free(argcpyp);
                //return;
            }

            cur_tokenp = strtok( NULL, " ");
            while (( cur_tokenp ) && ( cur_tokenp[0] != '}' ))
            {
                if ( !isdigit( (int)cur_tokenp[0] ))
                {
                    DynamicPreprocessorFatalMessage("Bad port %s.\n", cur_tokenp );
                    //free(argcpyp);
                    //return;
                }
                else
                {
                    port = atoi( cur_tokenp );
                    if( port < 0 || port > MAX_PORTS )
                    {
                        DynamicPreprocessorFatalMessage("Port value illegitimate: %s\n", cur_tokenp);
                        //free(argcpyp);
                        //return;
                    }

                    config->ports[ PORT_INDEX( port ) ] |= CONV_PORT(port);
                }

                cur_tokenp = strtok( NULL, " ");
            }

        }
        else if ( !strcmp( cur_tokenp, SSH_AUTODETECT_KEYWORD ))
        {
            config->AutodetectEnabled = 1;
        }
        else if ( !strcmp( cur_tokenp, SSH_MAX_ENC_PKTS_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, " ");
            config->MaxEncryptedPackets = (uint16_t)ParseNumInRange(cur_tokenp,
                                                SSH_MAX_ENC_PKTS_KEYWORD,
                                                MIN_MAX_ENC_PKTS,
                                                MAX_MAX_ENC_PKTS);
        }
        else if (!strcmp( cur_tokenp, SSH_MAX_CLIENT_BYTES_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, " ");
            config->MaxClientBytes = (uint16_t)ParseNumInRange(cur_tokenp,
                                                SSH_MAX_CLIENT_BYTES_KEYWORD,
                                                MIN_MAX_CLIENT_BYTES,
                                                MAX_MAX_CLIENT_BYTES);
        }
        else if ( !strcmp( cur_tokenp, SSH_MAX_SERVER_VERSION_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, " ");
            config->MaxServerVersionLen = (uint16_t)ParseNumInRange(cur_tokenp,
                                                SSH_MAX_SERVER_VERSION_KEYWORD,
                                                MIN_MAX_SERVER_VERSION_LEN,
                                                MAX_MAX_SERVER_VERSION_LEN);
        }
        else if ( !strcmp( cur_tokenp, SSH_ENABLE_RESPOVERFLOW_KEYWORD ))
        {
            config->EnabledAlerts |= SSH_ALERT_RESPOVERFLOW;
        }
        else if ( !strcmp( cur_tokenp, SSH_ENABLE_CRC32_KEYWORD ))
        {
            config->EnabledAlerts |= SSH_ALERT_CRC32;
        }
        else if (
           !strcmp( cur_tokenp, SSH_ENABLE_SECURECRT_KEYWORD ))
        {
            config->EnabledAlerts |= SSH_ALERT_SECURECRT;
        }
        else if (
           !strcmp( cur_tokenp, SSH_ENABLE_PROTOMISMATCH_KEYWORD ))
        {
            config->EnabledAlerts |= SSH_ALERT_PROTOMISMATCH;
        }
        else if (
           !strcmp( cur_tokenp, SSH_ENABLE_WRONGDIR_KEYWORD ))
        {
            config->EnabledAlerts |= SSH_ALERT_WRONGDIR;
        }
#if 0
        else if ( !strcmp( cur_tokenp, SSH_DISABLE_RULES_KEYWORD ))
        {
            config->DisableRules++;
        }
#endif
        else if( !strcmp( cur_tokenp, SSH_ENABLE_PAYLOAD_SIZE ))
        {
            config->EnabledAlerts |= SSH_ALERT_PAYSIZE;
        }
        else if( !strcmp( cur_tokenp, SSH_ENABLE_UNRECOGNIZED_VER ))
        {
            config->EnabledAlerts |= SSH_ALERT_UNRECOGNIZED;
        }
        else
        {
            DynamicPreprocessorFatalMessage("Invalid argument: %s\n", cur_tokenp);
            return;
        }

        cur_tokenp = strtok( NULL, " " );
    }

    DisplaySSHConfig(config);
    free(argcpyp);
}

/* Display the configuration for the SSH preprocessor.
 *
 * PARAMETERS:    None.
 *
 * RETURNS: Nothing.
 */
static void
DisplaySSHConfig(SSHConfig *config)
{
    int index;
    int newline;

    if (config == NULL)
        return;

    _dpd.logMsg("SSH config: \n");
    _dpd.logMsg("    Autodetection: %s\n",
            config->AutodetectEnabled ?
            "ENABLED":"DISABLED");
    _dpd.logMsg("    Challenge-Response Overflow Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_RESPOVERFLOW ?
            "ENABLED" : "DISABLED" );
    _dpd.logMsg("    SSH1 CRC32 Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_CRC32 ?
            "ENABLED" : "DISABLED" );

    _dpd.logMsg("    Server Version String Overflow Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_SECURECRT ?
            "ENABLED" : "DISABLED" );
    _dpd.logMsg("    Protocol Mismatch Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_PROTOMISMATCH?
            "ENABLED" : "DISABLED" );
    _dpd.logMsg("    Bad Message Direction Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_WRONGDIR ?
            "ENABLED" : "DISABLED" );
    _dpd.logMsg("    Bad Payload Size Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_PAYSIZE ?
            "ENABLED" : "DISABLED" );
    _dpd.logMsg("    Unrecognized Version Alert: %s\n",
            config->EnabledAlerts & SSH_ALERT_UNRECOGNIZED ?
            "ENABLED" : "DISABLED" );
    _dpd.logMsg("    Max Encrypted Packets: %d %s \n",
            config->MaxEncryptedPackets,
            config->MaxEncryptedPackets
                == SSH_DEFAULT_MAX_ENC_PKTS ?
                "(Default)" : "" );
    _dpd.logMsg("    Max Server Version String Length: %d %s \n",
            config->MaxServerVersionLen,
            config->MaxServerVersionLen
                == SSH_DEFAULT_MAX_SERVER_VERSION_LEN ?
                "(Default)" : "" );

    if ( config->EnabledAlerts &
        (SSH_ALERT_RESPOVERFLOW | SSH_ALERT_CRC32))
    {
        _dpd.logMsg("    MaxClientBytes: %d %s \n",
            config->MaxClientBytes,
            config->MaxClientBytes
                == SSH_DEFAULT_MAX_CLIENT_BYTES ?
                "(Default)" : "" );
    }

    /* Traverse list, printing ports, 5 per line */
    newline = 1;
    _dpd.logMsg("    Ports:\n");
    for(index = 0; index < MAX_PORTS; index++)
    {
        if( config->ports[ PORT_INDEX(index) ] & CONV_PORT(index) )
        {
            _dpd.logMsg("\t%d", index);
            if ( !((newline++)% 5) )
            {
                _dpd.logMsg("\n");
            }
        }
    }
    _dpd.logMsg("\n");
}

/* Returns the true length of the ssh packet, including
 * the ssh packet header and all padding.
 *
 * If the packet length is invalid, 0 is returned.
 * The return value is never larger than buflen.
 *
 * PARAMETERS:
 * p: Pointer to the SSH packet.
 * buflen: the size of packet buffer.
*/
static unsigned int SSHPacket_GetLength(SSH2Packet *p, size_t buflen)
{
    unsigned int ssh_length;

    if (buflen < sizeof(SSH2Packet))
        return 0;

    ssh_length = ntohl(p->packet_length);
    if ((ssh_length < sizeof(SSH2Packet) + 1) || ssh_length > SSH2_PACKET_MAX_SIZE)
        return 0;
   
   /* Everything after packet length field (including padding) is included in the packet_length */ 
    ssh_length += sizeof(p->packet_length);
        
    if (buflen < ssh_length)
        return buflen; /* truncated */
    
    return ssh_length;
}
        

/* Main runtime entry point for SSH preprocessor.
 * Analyzes SSH packets for anomalies/exploits.
 *
 * PARAMETERS:
 *
 * packetp:    Pointer to current packet to process.
 * contextp:    Pointer to context block, not used.
 *
 * RETURNS:     Nothing.
 */
static void
ProcessSSH( void* ipacketp, void* contextp )
{
    SSHData* sessp = NULL;
    uint8_t source = 0;
    uint8_t dest = 0;
    uint8_t known_port = 0;
    uint8_t direction;
    unsigned int offset = 0;
    SFSnortPacket* packetp;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

#ifdef DUMP_BUFFER
    dumpBufferInit();
#endif

    uint32_t search_dir_ver, search_dir_keyinit;
    char flags = STREAM_FLPOLICY_SET_ABSOLUTE;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    PROFILE_VARS;

    packetp = (SFSnortPacket*) ipacketp;
    sfPolicyUserPolicySet (ssh_config, policy_id);

    // preconditions - what we registered for
    assert(packetp->payload && packetp->payload_size &&
        IPH_IS_VALID(packetp) && packetp->tcp_header);

    PREPROC_PROFILE_START(sshPerfStats);

    ssh_eval_config = sfPolicyUserDataGetCurrent(ssh_config);

#ifdef DUMP_BUFFER
    dumpBuffer(SSH_PAYLOAD_DUMP,packetp->payload,packetp->payload_size);
#endif

    /* Attempt to get a previously allocated SSH block. */
	sessp = _dpd.sessionAPI->get_application_data(packetp->stream_session, PP_SSH);
    if (sessp != NULL)
    {
        ssh_eval_config = sfPolicyUserDataGet(sessp->config, sessp->policy_id);
        known_port = 1;
    }
    else
    {
        /* If not doing autodetection, check the ports to make sure this is
         * running on an SSH port, otherwise no need to examine the traffic.
         */
#ifdef TARGET_BASED
        app_id = _dpd.sessionAPI->get_application_protocol_id(packetp->stream_session);
        if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
        {
            PREPROC_PROFILE_END(sshPerfStats);
            return;
        }

        if (app_id && (app_id != ssh_app_id))
        {
            PREPROC_PROFILE_END(sshPerfStats);
            return;
        }

        if (app_id == ssh_app_id)
        {
            known_port = 1;
        }

        if (!app_id)
        {
#endif
            source = (uint8_t)CheckSSHPort( packetp->src_port );
            dest = (uint8_t)CheckSSHPort( packetp->dst_port );

            if ( !ssh_eval_config->AutodetectEnabled && !source && !dest )
            {
                /* Not one of the ports we care about. */
                PREPROC_PROFILE_END(sshPerfStats);
                return;
            }
#ifdef TARGET_BASED
        }
#endif
        /* Check the stream session. If it does not currently
         * have our SSH data-block attached, create one.
         */
        sessp = SSHGetNewSession(packetp, policy_id);

        if ( !sessp )
        {
            /* Could not get/create the session data for this packet. */
            PREPROC_PROFILE_END(sshPerfStats);
            return;
        }

        /* See if a known server port is involved. */
        if (!known_port)
        {
            known_port = ( source || dest ? 1 : 0 );

            /* If this is a non-SSH port, but autodetect is on, we flag this
               session to reduce false positives later on. */
            if (!known_port && ssh_eval_config->AutodetectEnabled)
            {
                sessp->state_flags |= SSH_FLG_AUTODETECTED;
                flags = STREAM_FLPOLICY_SET_APPEND;
            }
        }
    }

    /* Don't process if we've missed packets */
    if (sessp->state_flags & SSH_FLG_MISSED_PACKETS)
    {
        PREPROC_PROFILE_END(sshPerfStats);
        return;
    }
    /* We're interested in this session. Turn on stream reassembly. */
    if ( !(sessp->state_flags & SSH_FLG_REASSEMBLY_SET ))
    {
        _dpd.streamAPI->set_reassembly(packetp->stream_session,
                        STREAM_FLPOLICY_FOOTPRINT, SSN_DIR_BOTH, flags);
        sessp->state_flags |= SSH_FLG_REASSEMBLY_SET;
    }

    /* Make sure this preprocessor should run. */
    /* check if we're waiting on stream reassembly */
    if ( packetp->flags & FLAG_STREAM_INSERT )
    {
        PREPROC_PROFILE_END(sshPerfStats);
        return;
    }

    /* If we picked up mid-stream or missed any packets (midstream pick up
     * means we've already missed packets) set missed packets flag and make
     * sure we don't do any more reassembly on this session */
    if ((_dpd.sessionAPI->get_session_flags(packetp->stream_session) & SSNFLAG_MIDSTREAM)
            || _dpd.streamAPI->missed_packets(packetp->stream_session, SSN_DIR_BOTH))
    {
        /* Order only matters if the packets are not encrypted */
        if ( !(sessp->state_flags & SSH_FLG_SESS_ENCRYPTED ))
        {
            /* Don't turn off reassembly if autodetected since another preprocessor
             * may actually be looking at this session as well and the SSH
             * autodetect of this session may be wrong. */
            if (!(sessp->state_flags & SSH_FLG_AUTODETECTED))
            {
                _dpd.streamAPI->set_reassembly(packetp->stream_session,
                        STREAM_FLPOLICY_IGNORE, SSN_DIR_BOTH,
                        STREAM_FLPOLICY_SET_APPEND);
            }

            sessp->state_flags |= SSH_FLG_MISSED_PACKETS;

            PREPROC_PROFILE_END(sshPerfStats);
            return;
        }
    }


    /* Get the direction of the packet. */
    if( packetp->flags & FLAG_FROM_SERVER )
    {
        direction = SSH_DIR_FROM_SERVER;
        search_dir_ver = SSH_FLG_SERV_IDSTRING_SEEN;
        search_dir_keyinit = SSH_FLG_SERV_PKEY_SEEN | SSH_FLG_SERV_KEXINIT_SEEN;
    }
    else
    {
        direction = SSH_DIR_FROM_CLIENT;
        search_dir_ver = SSH_FLG_CLIENT_IDSTRING_SEEN;
        search_dir_keyinit = SSH_FLG_CLIENT_SKEY_SEEN | SSH_FLG_CLIENT_KEXINIT_SEEN;
    }
    if ( !(sessp->state_flags & SSH_FLG_SESS_ENCRYPTED ))
    {
        /* If server and client have not performed the protocol
         * version exchange yet, must look for version strings.
          */
        if ( !(sessp->state_flags & search_dir_ver) )
        {
            offset = ProcessSSHProtocolVersionExchange( sessp, packetp, direction, known_port);
            if (!offset)
            {
                /*Error processing protovers exchange msg */
                PREPROC_PROFILE_END(sshPerfStats);
                return;
            }
            /* found protocol version.  Stream reassembly might have appended an ssh packet,
             * such as the key exchange init.  Thus call ProcessSSHKeyInitExchange() too.
             */

        }

        /* Expecting to see the key init exchange at this point
         * (in SSH2) or the actual key exchange if SSH1
         */
        if ( !(sessp->state_flags & search_dir_keyinit) )
        {
            offset = ProcessSSHKeyInitExchange( sessp, packetp, direction, offset);

            if (!offset)
            {
                if ( !(sessp->state_flags & SSH_FLG_SESS_ENCRYPTED ))
                {
                    PREPROC_PROFILE_END(sshPerfStats);
                    return;
                }
            }
        }

        /* If SSH2, need to process the actual key exchange msgs.
         * The actual key exchange type was negotiated in the
         * key exchange init msgs. SSH1 won't arrive here.
         */
        offset = ProcessSSHKeyExchange( sessp, packetp, direction, offset);
        if (!offset)
        {
            PREPROC_PROFILE_END(sshPerfStats);
            return;
        }
    }
    if ( (sessp->state_flags & SSH_FLG_SESS_ENCRYPTED ))
    {
        /* Traffic on this session is currently encrypted.
         * Two of the major SSH exploits, SSH1 CRC-32 and
          * the Challenge-Response Overflow attack occur within
         * the encrypted portion of the SSH session. Therefore,
         * the only way to detect these attacks is by examining
         * amounts of data exchanged for anomalies.
           */
        sessp->num_enc_pkts++;

        if ( sessp->num_enc_pkts <= ssh_eval_config->MaxEncryptedPackets )
        {
            if ( direction == SSH_DIR_FROM_CLIENT )
            {
                if(!offset)
                {
                    sessp->num_client_bytes += packetp->payload_size;
                }
                else
                {
                    sessp->num_client_bytes += (packetp->payload_size - offset);
                }

                if ( (sessp->num_client_bytes >= ssh_eval_config->MaxClientBytes)
#if defined(FEAT_OPEN_APPID)
                     && (_dpd.getAppId(packetp->stream_session) != APP_ID_SFTP)
#endif /* defined(FEAT_OPEN_APPID) */
                   )
                {
                    /* Probable exploit in progress.*/
                    if (sessp->version == SSH_VERSION_1)
                    {
                        if ( ssh_eval_config->EnabledAlerts & SSH_ALERT_CRC32 )
                        {
                            ALERT(SSH_EVENT_CRC32, SSH_EVENT_CRC32_STR);

                            _dpd.sessionAPI->stop_inspection(
                                                            packetp->stream_session,
                                                            packetp,
                                                            SSN_DIR_BOTH, -1, 0 );
                        }
                    }
                    else
                    {
                        if (ssh_eval_config->EnabledAlerts & SSH_ALERT_RESPOVERFLOW )
                        {
                            ALERT(SSH_EVENT_RESPOVERFLOW, SSH_EVENT_RESPOVERFLOW_STR);

                            _dpd.sessionAPI->stop_inspection(
                                                            packetp->stream_session,
                                                            packetp,
                                                            SSN_DIR_BOTH, -1, 0 );
                        }
                    }
                }
            }
            else
            {
                /*
                 * Have seen a server response, so
                 * this appears to be a valid exchange.
                 * Reset suspicious byte count to zero.
                 */
                sessp->num_client_bytes = 0;
            }
        }
        else
        {
            /* Have already examined more than the limit
             * of encrypted packets. Both the Gobbles and
             * the CRC32 attacks occur during authentication
             * and therefore cannot be used late in an
             * encrypted session. For performance purposes,
             * stop examining this session.
             */
			_dpd.sessionAPI->stop_inspection(
				packetp->stream_session,
                packetp,
                SSN_DIR_BOTH, -1, 0 );

        }

    }
    PREPROC_PROFILE_END(sshPerfStats);
}

/* Retrieves the SSH data block registered with the stream
 * session associated w/ the current packet. If none exists,
 * allocates it and registers it with the stream API.
 *
 * PARAMETERS:
 *
 * packetp:    Pointer to the packet from which/in which to
 *         retrieve/store the SSH data block.
 *
 * RETURNS:    Pointer to an SSH data block, upon success.
 *        NULL, upon failure.
 */
SSHData * SSHGetNewSession(SFSnortPacket *packetp, tSfPolicyId policy_id)
{
    SSHData* datap = NULL;

    /* Sanity check(s) */
	if (( !packetp ) || ( !packetp->stream_session ))
    {
        return NULL;
    }

    datap = (SSHData *)calloc(1, sizeof(SSHData));

    if ( !datap )
        return NULL;

    /*Register the new SSH data block in the stream session. */
    _dpd.sessionAPI->set_application_data( packetp->stream_session, PP_SSH, datap, FreeSSHData );

    datap->policy_id = policy_id;
    datap->config = ssh_config;
    ((SSHConfig *)sfPolicyUserDataGetCurrent(ssh_config))->ref_count++;

    return datap;
}

static int SshFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    SSHConfig *pPolicyConfig = (SSHConfig *)pData;

    //do any housekeeping before freeing SSHConfig

    sfPolicyUserDataClear (config, policyId);
    free(pPolicyConfig);
    return 0;
}

static void SSHFreeConfig(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, SshFreeConfigPolicy);
    sfPolicyConfigDelete(config);
}

/* Registered as a callback with our SSH data blocks when
 * they are added to the underlying stream session. Called
 * by the stream preprocessor when a session is about to be
 * destroyed.
 *
 * PARAMETERS:
 *
 * idatap:    Pointer to the moribund data.
 *
 * RETURNS:    Nothing.
 */
static void
FreeSSHData( void* idatap )
{
    SSHData *ssn = (SSHData *)idatap;
    SSHConfig *config = NULL;

    if (ssn == NULL)
        return;

    if (ssn->config != NULL)
    {
        config = (SSHConfig *)sfPolicyUserDataGet(ssn->config, ssn->policy_id);
    }

    if (config != NULL)
    {
        config->ref_count--;
        if ((config->ref_count == 0) &&
            (ssn->config != ssh_config))
        {
            sfPolicyUserDataClear (ssn->config, ssn->policy_id);
            free(config);

            if (sfPolicyUserPolicyGetActive(ssn->config) == 0)
            {
                /* No more outstanding configs - free the config array */
                SSHFreeConfig(ssn->config);
            }

        }
    }

    free(ssn);
}

/* Validates given port as an SSH server port.
 *
 * PARAMETERS:
 *
 * port:    Port to validate.
 *
 * RETURNS:    SSH_TRUE, if the port is indeed an SSH server port.
 *        SSH_FALSE, otherwise.
 */
static inline int
CheckSSHPort( uint16_t port )
{
    if ( ssh_eval_config->ports[ PORT_INDEX(port) ] & CONV_PORT( port ) )
    {
        return SSH_TRUE;
    }

    return SSH_FALSE;
}

/* Checks if the string 'str' is 'max' bytes long or longer.
 * Returns 0 if 'str' is less than or equal to 'max' bytes;
 * returns 1 otherwise.
*/

static inline int SSHCheckStrlen(char *str, int max) {
    while(*(str++) && max--) ;

    if(max > 0) return 0;   /* str size is <= max bytes */

    return 1;
}

/* Attempts to process current packet as a protocol version exchange
 * packet. This function will be called if either the client or server
 * protocol version message (or both) has not been sent.
 *
 * PARAMETERS:
 *
 * sessionp:    Pointer to SSH data for packet's session.
 * packetp:    Pointer to the packet to inspect.
 * direction:     Which direction the packet is going.
 * known_port:  A pre-configured or default server port is involved.
 *
 * RETURNS:    SSH_SUCCESS, if successfully processed a proto exch msg
 *        SSH_FAILURE, otherwise.
 */
static unsigned int
ProcessSSHProtocolVersionExchange( SSHData* sessionp, SFSnortPacket* packetp,
    uint8_t direction, uint8_t known_port )
{
    char* version_stringp = (char*) packetp->payload;
    uint8_t version;
    char *version_end;

#ifdef DUMP_BUFFER
    dumpBuffer(SSH_PAYLOAD_DUMP,packetp->payload,packetp->payload_size);
#endif

    /* Get the version. */
    if ( packetp->payload_size >= 6 &&
         !strncasecmp( version_stringp, "SSH-1.", 6))
    {
        if (( packetp->payload_size > 7 ) && ( version_stringp[6] == '9')
            && (version_stringp[7] == '9'))
        {
            /* SSH 1.99 which is the same as SSH2.0 */
            version = SSH_VERSION_2;
        }
        else
        {
            version = SSH_VERSION_1;
        }

        /* CAN-2002-0159 */
        /* Verify the version string is not greater than
         * the configured maximum.
         * We've already verified the first 6 bytes, so we'll start
         * check from &version_string[6] */
        if( (ssh_eval_config->EnabledAlerts & SSH_ALERT_SECURECRT ) &&
            /* First make sure the payload itself is sufficiently large */
             (packetp->payload_size > ssh_eval_config->MaxServerVersionLen) &&
            /* CheckStrlen will check if the version string up to
             * MaxServerVersionLen+1 since there's no reason to
             * continue checking after that point*/
             (SSHCheckStrlen(&version_stringp[6], ssh_eval_config->MaxServerVersionLen-6)))
        {
            ALERT(SSH_EVENT_SECURECRT, SSH_EVENT_SECURECRT_STR);
        }
    }
    else if ( packetp->payload_size >= 6 &&
              !strncasecmp( version_stringp, "SSH-2.", 6))
    {
        version = SSH_VERSION_2;
    }
    else
    {
        /* Not SSH on SSH port, CISCO vulnerability */
        if ((direction == SSH_DIR_FROM_CLIENT) &&
            ( known_port != 0 ) &&
            ( !(sessionp->state_flags & SSH_FLG_AUTODETECTED) ) &&
            ( ssh_eval_config->EnabledAlerts &
                SSH_ALERT_PROTOMISMATCH ))
        {
            ALERT(SSH_EVENT_PROTOMISMATCH, SSH_EVENT_PROTOMISMATCH_STR);
        }

        return 0;
    }

    /* Saw a valid protocol exchange message. Mark the session
     * according to the direction.
     */
    switch( direction )
    {
        case SSH_DIR_FROM_SERVER:
            sessionp->state_flags |= SSH_FLG_SERV_IDSTRING_SEEN;
            break;
        case SSH_DIR_FROM_CLIENT:
            sessionp->state_flags |= SSH_FLG_CLIENT_IDSTRING_SEEN;
            break;
    }

    sessionp->version = version;
    version_end = memchr(version_stringp, '\n', packetp->payload_size);
    if (version_end)
        return ((version_end - version_stringp) + 1);
    /* incomplete version string, should end with \n or \r\n for sshv2 */
    return packetp->payload_size;
}

/* Called to process SSH1 key exchange or SSH2 key exchange init
 * messages.  On failure, inspection will be continued, but the packet
 * will be alerted on, and ignored.
 *
 * PARAMETERS:
 *
 * sessionp:    Pointer to SSH data for packet's session.
 * packetp:    Pointer to the packet to inspect.
 * direction:     Which direction the packet is going.
 *
 * RETURN:    SSH_SUCCESS, if a valid key exchange message is processed
 *        SSH_FAILURE, otherwise.
 */
static unsigned int
ProcessSSHKeyInitExchange( SSHData* sessionp, SFSnortPacket* packetp,
    uint8_t direction, unsigned int offset )
{
    SSH2Packet* ssh2packetp = NULL;
    uint16_t payload_size = packetp->payload_size;
    const unsigned char *payload = packetp->payload;
    unsigned int ssh_length = 0;

    if (payload_size < sizeof(SSH2Packet) || (payload_size < (offset + sizeof(SSH2Packet)))
            || (payload_size <= offset))
        return 0;
    
    payload_size -= offset;
    payload += offset;

    if ( sessionp->version == SSH_VERSION_1 )
    {
        uint32_t length;
        uint8_t padding_length;
        uint8_t message_type;

        /*
         * Validate packet payload.
         * First 4 bytes should have the SSH packet length,
         * minus any padding.
         */
        if ( payload_size < 4 )
        {
            if(ssh_eval_config->EnabledAlerts & SSH_ALERT_PAYSIZE)
            {
                ALERT(SSH_EVENT_PAYLOAD_SIZE, SSH_PAYLOAD_SIZE_STR);
            }

            return 0;
        }

        /*
         * SSH1 key exchange is very simple and
          * consists of only two messages, a server
         * key and a client key message.`
         */
        memcpy(&length, payload, sizeof(length));
        length = ntohl(length);

        /* Packet payload should be larger than length, due to padding. */
        if ( payload_size < length )
        {
            if(ssh_eval_config->EnabledAlerts & SSH_ALERT_PAYSIZE)
            {
                ALERT(SSH_EVENT_PAYLOAD_SIZE, SSH_PAYLOAD_SIZE_STR);
            }

            return 0;
        }

        padding_length = (uint8_t)(8 - (length % 8));

        /*
         * With the padding calculated, verify payload is sufficiently large
         * to include the message type.
         */
        if ( payload_size < (padding_length + 4 + 1 + offset))
        {
            if((offset == 0) && (ssh_eval_config->EnabledAlerts & SSH_ALERT_PAYSIZE))
            {
                ALERT(SSH_EVENT_PAYLOAD_SIZE, SSH_PAYLOAD_SIZE_STR);
            }

            return 0;
        }

        message_type = *( (uint8_t*) (payload + padding_length + 4));

        switch( message_type )
        {
            case SSH_MSG_V1_SMSG_PUBLIC_KEY:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_V1_SMSG_PUBLIC_KEY_DUMP,payload,payload_size);
#endif
                if ( direction == SSH_DIR_FROM_SERVER )
                {
                    sessionp->state_flags |=
                        SSH_FLG_SERV_PKEY_SEEN;
                }
                else if ( ssh_eval_config->EnabledAlerts &
                    SSH_ALERT_WRONGDIR )
                {
                    /* Server msg not from server. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            case SSH_MSG_V1_CMSG_SESSION_KEY:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_V1_CMSG_SESSION_KEY_DUMP,payload,payload_size);
#endif
                if ( direction == SSH_DIR_FROM_CLIENT )
                {
                    sessionp->state_flags |=
                        SSH_FLG_CLIENT_SKEY_SEEN;
                }
                else if ( ssh_eval_config->EnabledAlerts &
                    SSH_ALERT_WRONGDIR )
                {
                    /* Client msg not from client. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            default:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_TYPE_INVALID_DUMP,payload,payload_size);
#endif
                /* Invalid msg type*/
                break;
        }

        /* Once the V1 key exchange is done, remainder of
         * communications are encrypted.
         */
        ssh_length = length + padding_length + sizeof(length) + offset;

        if ( (sessionp->state_flags & SSH_FLG_V1_KEYEXCH_DONE) ==
            SSH_FLG_V1_KEYEXCH_DONE )
        {
            sessionp->state_flags |= SSH_FLG_SESS_ENCRYPTED;
        }
    }
    else if ( sessionp->version == SSH_VERSION_2 )
    {
        /* We want to overlay the payload on our data packet struct,
         * so first verify that the payload size is big enough.
         * This may legitimately occur such as in the case of a
         * retransmission.
         */
        if ( payload_size < sizeof(SSH2Packet) )
        {
            return 0;
        }

        /* Overlay the SSH2 binary data packet struct on the packet */
        ssh2packetp = (SSH2Packet*) payload;
        if ( payload_size < SSH2_HEADERLEN + 1)
        {
            /* Invalid packet length. */

            return 0;
        }

        ssh_length = offset + ntohl(ssh2packetp->packet_length) + sizeof(ssh2packetp->packet_length);

        switch ( payload[SSH2_HEADERLEN] )
        {
            case SSH_MSG_KEXINIT:
                sessionp->state_flags |=
                    (direction == SSH_DIR_FROM_SERVER ?
                        SSH_FLG_SERV_KEXINIT_SEEN :
                        SSH_FLG_CLIENT_KEXINIT_SEEN );
                break;
            default:
                /* Unrecognized message type. */
                break;
        }
    }
    else
    {
        if(ssh_eval_config->EnabledAlerts & SSH_ALERT_UNRECOGNIZED)
        {
            /* Unrecognized version. */
            ALERT(SSH_EVENT_VERSION, SSH_VERSION_STR);
        }

        return 0;
    }

    if(ssh_length < packetp->payload_size)
        return ssh_length;
    else
        return 0;
}
/* Called to process SSH2 key exchange msgs (key exch init msgs already
 * processed earlier). On failure, inspection will be continued, but the
 * packet will be alerted on, and ignored.
 *
 * PARAMETERS:
 *
 * sessionp:    Pointer to SSH data for packet's session.
 * packetp:    Pointer to the packet to inspect.
 * direction:     Which direction the packet is going.
 *
 * RETURN:    SSH_SUCCESS, if a valid key exchange message is processed
 *        SSH_FAILURE, otherwise.
 */
static unsigned int
ProcessSSHKeyExchange( SSHData* sessionp, SFSnortPacket* packetp,
    uint8_t direction, unsigned int offset)
{
    SSH2Packet* ssh2packetp = NULL;
    uint16_t payload_size = packetp->payload_size;
    const unsigned char *payload = packetp->payload;
    unsigned int ssh_length;
    bool next_packet = true;
    unsigned int npacket_offset = 0;

    if (payload_size < sizeof(SSH2Packet) || (payload_size < (offset + sizeof(SSH2Packet)))
                || (payload_size <= offset))
    {
        return 0;
    }
    payload_size -= offset;
    payload += offset;

#ifdef DUMP_DUFFER
dumpBuffer(SSH_KEY_DUMP,payload,payload_size);
#endif
    while(next_packet)
    {
        ssh2packetp = (SSH2Packet*) (payload + npacket_offset);
        ssh_length = SSHPacket_GetLength(ssh2packetp, payload_size);

        if (ssh_length == 0)
        {
            if( sessionp->state_flags & SSH_FLG_SESS_ENCRYPTED )
            {
                return ( npacket_offset + offset );
            }
            if(ssh_eval_config->EnabledAlerts & SSH_ALERT_PAYSIZE)
            {
                /* Invalid packet length. */
                ALERT(SSH_EVENT_PAYLOAD_SIZE, SSH_PAYLOAD_SIZE_STR);
            }

            return 0;
        }

        switch(payload[npacket_offset + SSH2_HEADERLEN] )
        {
            case SSH_MSG_KEXDH_INIT:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_KEXDH_INIT_DUMP,(const uint8_t *)ssh2packetp,ssh_length);
#endif
                if ( direction == SSH_DIR_FROM_CLIENT )
                {
                    sessionp->state_flags |=
                        SSH_FLG_KEXDH_INIT_SEEN;
                }
                else if ( ssh_eval_config->EnabledAlerts &
                        SSH_ALERT_WRONGDIR )
                {
                    /* Client msg from server. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            case SSH_MSG_KEXDH_REPLY:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_KEXDH_REPLY_DUMP,(const uint8_t *)ssh2packetp,ssh_length);
#endif
                if ( direction == SSH_DIR_FROM_SERVER )
                {
                    /* KEXDH_REPLY has the same msg
                      * type as the new style GEX_REPLY
                     */
                    sessionp->state_flags |=
                        SSH_FLG_KEXDH_REPLY_SEEN |
                        SSH_FLG_GEX_REPLY_SEEN;

                }
                else if ( ssh_eval_config->EnabledAlerts &
                        SSH_ALERT_WRONGDIR )
                {
                    /* Server msg from client. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            case SSH_MSG_KEXDH_GEX_REQ:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_KEXDH_GEX_REQ_DUMP,(const uint8_t *)ssh2packetp,ssh_length);
#endif
                if ( direction == SSH_DIR_FROM_CLIENT )
                {
                    sessionp->state_flags |=
                        SSH_FLG_GEX_REQ_SEEN;
                }
                else if ( ssh_eval_config->EnabledAlerts &
                        SSH_ALERT_WRONGDIR )
                {
                    /* Server msg from client. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            case SSH_MSG_KEXDH_GEX_GRP:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_KEXDH_GEX_GRP_DUMP,(const uint8_t *)ssh2packetp,ssh_length);
#endif
                if ( direction == SSH_DIR_FROM_SERVER )
                {
                    sessionp->state_flags |=
                        SSH_FLG_GEX_GRP_SEEN;
                }
                else if ( ssh_eval_config->EnabledAlerts &
                        SSH_ALERT_WRONGDIR )
                {
                    /* Client msg from server. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            case SSH_MSG_KEXDH_GEX_INIT:
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_KEXDH_GEX_INIT_DUMP,(const uint8_t *)ssh2packetp,ssh_length);
#endif
                if ( direction == SSH_DIR_FROM_CLIENT )
                {
                    sessionp->state_flags |=
                        SSH_FLG_GEX_INIT_SEEN;
                }
                else if ( ssh_eval_config->EnabledAlerts &
                        SSH_ALERT_WRONGDIR )
                {
                    /* Server msg from client. */
                    ALERT(SSH_EVENT_WRONGDIR, SSH_EVENT_WRONGDIR_STR);
                }
                break;
            case SSH_MSG_NEWKEYS:
                /* This message is required to complete the
                 * key exchange. Both server and client should
                 * send one, but as per Alex Kirk's note on this,
                 * in some implementations the server does not
                 * actually send this message. So receving a new
                 * keys msg from the client is sufficient.
                 */
#ifdef DUMP_BUFFER
    dumpBuffer(SSH_MSG_NEWKEYS_DUMP,(const uint8_t *)ssh2packetp,ssh_length);
#endif
                if ( direction == SSH_DIR_FROM_CLIENT )
                {
                    sessionp->state_flags |= SSH_FLG_NEWKEYS_SEEN;
                }
                break;
            default:
                /* Unrecognized message type. Possibly encrypted */
                sessionp->state_flags |= SSH_FLG_SESS_ENCRYPTED;
                return ( npacket_offset + offset);
        }

        /* If either an old-style or new-style Diffie Helman exchange
         * has completed, the session will enter encrypted mode.
         */
        if (( (sessionp->state_flags &
            SSH_FLG_V2_DHOLD_DONE) == SSH_FLG_V2_DHOLD_DONE )
            || ( (sessionp->state_flags &
                SSH_FLG_V2_DHNEW_DONE) == SSH_FLG_V2_DHNEW_DONE ))
        {
            sessionp->state_flags |= SSH_FLG_SESS_ENCRYPTED;
            if(ssh_length < payload_size)
            {
                if( ssh_length >= 4 )
                {
                    npacket_offset += ssh_length;
                    payload_size -= ssh_length;
                    continue;
                }
                return ( npacket_offset + offset );
            }
            else
                return 0;
        }

        if ((ssh_length < payload_size) && (ssh_length >= 4))
        {
            npacket_offset += ssh_length;
            payload_size -= ssh_length;
        }
        else
        {
            next_packet = false;
            npacket_offset = 0;
            if(ssh_eval_config->EnabledAlerts & SSH_ALERT_PAYSIZE)
            {
                /* Invalid packet length. */
                ALERT(SSH_EVENT_PAYLOAD_SIZE, SSH_PAYLOAD_SIZE_STR);
            }
        }
    }

    return (npacket_offset + offset);
}

static void enablePortStreamServices(struct _SnortConfig *sc, SSHConfig *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return;

    if (_dpd.streamAPI)
    {
        uint32_t portNum;

        for (portNum = 0; portNum < MAXPORTS; portNum++)
        {
            if(config->ports[(portNum/8)] & (1<<(portNum%8)))
            {
                //Add port the port
                _dpd.streamAPI->set_port_filter_status( sc, IPPROTO_TCP, (uint16_t)portNum,
                                                        PORT_MONITOR_SESSION, policy_id, 1 );
                _dpd.streamAPI->register_reassembly_port( NULL, 
                                                          (uint16_t) portNum, 
                                                          SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
                _dpd.sessionAPI->enable_preproc_for_port( sc, PP_SSH, PROTO_BIT__TCP, (uint16_t) portNum ); 
            }
        }
    }
}
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, ssh_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int SSHCheckPolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    _dpd.setParserPolicy(sc, policyId);

    if (_dpd.streamAPI == NULL)
    {
        _dpd.errMsg("SSHCheckPolicyConfig(): The Stream preprocessor must be enabled.\n");
        return -1;
    }
    return 0;
}
static int SSHCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate (sc, ssh_config, SSHCheckPolicyConfig)))
        return rval;

    return 0;
}

static void SSHCleanExit(int signal, void *data)
{
    if (ssh_config != NULL)
    {
        SSHFreeConfig(ssh_config);
        ssh_config = NULL;
    }
}

#ifdef SNORT_RELOAD
static void SSHReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId ssh_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SSHConfig * pPolicyConfig = NULL;

    if (ssh_swap_config == NULL)
    {
        //create a context
        ssh_swap_config = sfPolicyConfigCreate();
        if (ssh_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                                            "for SSH config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            DynamicPreprocessorFatalMessage("SetupSSH(): The Stream preprocessor must be enabled.\n");
        }
        *new_config = (void *)ssh_swap_config;
    }

    sfPolicyUserPolicySet (ssh_swap_config, policy_id);
    pPolicyConfig = (SSHConfig *)sfPolicyUserDataGetCurrent(ssh_swap_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("SSH preprocessor can only be "
                                        "configured once.\n");
    }

    pPolicyConfig = (SSHConfig *)calloc(1, sizeof(SSHConfig));
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                                        "SSH preprocessor configuration.\n");
    }
    sfPolicyUserDataSetCurrent(ssh_swap_config, pPolicyConfig);

    ParseSSHArgs(pPolicyConfig, (u_char *)args);

    _dpd.addPreproc( sc, ProcessSSH, PRIORITY_APPLICATION, PP_SSH, PROTO_BIT__TCP );

    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static int SSHReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    if (_dpd.streamAPI == NULL)
    {
        _dpd.errMsg("SetupSSH(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    return 0;
}
static int SshFreeUnusedConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    SSHConfig *pPolicyConfig = (SSHConfig *)pData;

    //do any housekeeping before freeing SSHConfig
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        free(pPolicyConfig);
    }
    return 0;
}

static void * SSHReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId ssh_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = ssh_config;

    if (ssh_swap_config == NULL)
        return NULL;

    ssh_config = ssh_swap_config;

    sfPolicyUserDataFreeIterate (old_config, SshFreeUnusedConfigPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_config;
    }

    return NULL;
}

static void SSHReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    SSHFreeConfig((tSfPolicyUserContextId)data);
}
#endif
