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
 * File: ssl_config.c
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 * Brief: Configuration for the SSL preprocessor
*/


#include "ssl_config.h"
#ifdef ENABLE_HA
#include "ssl_ha.h"
#endif
#include <errno.h>

#define PATH_MAX 4096
#define MIN_HB_LEN 0
#define MAX_HB_LEN 65535

#ifdef TARGET_BASED
int16_t ssl_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

#ifdef PERF_PROFILING
PreprocStats sslpp_perf_stats;
#endif


tSfPolicyUserContextId ssl_config = NULL;

static void SSLFreeConfig(tSfPolicyUserContextId config, bool full_cleanup);
static void SSLCleanExit(int, void *);
static void SSLResetStats(int, void *);
static int SSLPP_CheckConfig(struct _SnortConfig *);

/* Parsing for the ssl_state rule option */
static int SSLPP_state_init(struct _SnortConfig *sc, char *name, char *params, void **data)
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
static int SSLPP_ver_init(struct _SnortConfig *sc, char *name, char *params, void **data)
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

static void UpdatePathToDir(char *full_path_dirname, unsigned int max_size, char *dirname)
{
    int iRet;
    char *snort_conf_dir = *(_dpd.snort_conf_dir);

    if (!snort_conf_dir || !(*snort_conf_dir) || !full_path_dirname || !dirname)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => can't create path.\n",
                *(_dpd.config_file), *(_dpd.config_line));
    }
    /*dirname is too long*/
    if ( max_size < strlen(dirname) )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => the dir name length %u is longer than allowed %u.\n",
                *(_dpd.config_file), *(_dpd.config_line), strlen(dirname), max_size);
    }

    /*
     *  If an absolute path is specified, then use that.
     */
#ifndef WIN32
    if(dirname[0] == '/')
    {
        iRet = snprintf(full_path_dirname, max_size, "%s", dirname);
    }
    else
    {
        /*
         * Set up the dir name directory.
         */
        if (snort_conf_dir[strlen(snort_conf_dir) - 1] == '/')
        {
            iRet = snprintf(full_path_dirname,max_size,
                    "%s%s", snort_conf_dir, dirname);
        }
        else
        {
            iRet = snprintf(full_path_dirname, max_size,
                    "%s/%s", snort_conf_dir, dirname);
        }
    }
#else
    if(strlen(dirname)>3 && dirname[1]==':' && dirname[2]=='\\')
    {
        iRet = snprintf(full_path_dirname, max_size, "%s", dirname);
    }
    else
    {
        /*
         **  Set up the dir name directory
         */
        if (snort_conf_dir[strlen(snort_conf_dir) - 1] == '\\' ||
                snort_conf_dir[strlen(snort_conf_dir) - 1] == '/' )
        {
            iRet = snprintf(full_path_dirname,max_size,
                    "%s%s", snort_conf_dir, dirname);
        }
        else
        {
            iRet = snprintf(full_path_dirname, max_size,
                    "%s\\%s", snort_conf_dir, dirname);
        }
    }
#endif

    if(iRet < 0)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => the dir name length %u is longer than allowed %u.\n",
                *(_dpd.config_file), *(_dpd.config_line), strlen(dirname), max_size);
    }
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
        else if(!strcasecmp(space_tok, "pki_dir"))
        {
            char *tmpChar;
            char full_path_dirname[PATH_MAX+1];
            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }
            UpdatePathToDir(full_path_dirname, PATH_MAX, tmpChar);
            config->pki_dir = strdup(full_path_dirname);
            if (!config->pki_dir)
            {
                DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                    "option in SSL preprocessor\n", __FILE__, __LINE__);
            }
        }
        else if(!strcasecmp(space_tok, "ssl_rules_dir"))
        {
            char *tmpChar;
            char full_path_dirname[PATH_MAX+1];
            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }
            UpdatePathToDir(full_path_dirname, PATH_MAX, tmpChar);
            config->ssl_rules_dir = strdup(full_path_dirname);
            if (!config->ssl_rules_dir)
            {
                DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                    "option in SSL preprocessor\n", __FILE__, __LINE__);
            }
        }
        else if(!strcasecmp(space_tok, "memcap"))
        {
            int value;
            char *endStr = NULL;
            char *tmpChar;

            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }

            value = _dpd.SnortStrtol( tmpChar, &endStr, 10);

            if (( *endStr) || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }

            config->memcap = value;

        }
        else if(!strcasecmp(space_tok, "decrypt_memcap"))
        {
            int value;
            char *endStr = NULL;
            char *tmpChar;

            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }

            value = _dpd.SnortStrtol( tmpChar, &endStr, 10);

            if (( *endStr) || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }

            config->decrypt_memcap = value;

        }
#ifdef ENABLE_HA
        else if(!strcasecmp(space_tok, "enable_ssl_ha"))
        {
            char *tmpChar;
            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }
            if(!strcasecmp(tmpChar, "yes"))
                config->enable_ssl_ha = true;
            else if(!strcasecmp(tmpChar, "no"))
                config->enable_ssl_ha = false;
            else
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument '%s' to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), tmpChar, space_tok);
            }
        }
#endif
        else if(!strcasecmp(space_tok, "max_heartbeat_length"))
        {
            int value;
            char *endStr = NULL;
            char *tmpChar;

            tmpChar = strtok_r(NULL, " \t\n", &portptr);
            if(tmpChar == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }

            value = _dpd.SnortStrtol( tmpChar, &endStr, 10);

            if (( *endStr) || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Invalid argument to '%s' option in the"
                    " SSL preprocessor\n",
                    *(_dpd.config_file), *(_dpd.config_line), space_tok);
            }

            if (value < MIN_HB_LEN || value > MAX_HB_LEN)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Value specified for %s is out of "
                        "bounds.  Please specify an integer between %d and %d.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        space_tok, MIN_HB_LEN, MAX_HB_LEN);
            }

            config->max_heartbeat_len = value;
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

    if ( config->pki_dir )
    {
        _dpd.logMsg("    PKI Directory: %s\n", config->pki_dir);
    }

    if ( config->ssl_rules_dir )
    {
        _dpd.logMsg("    SSL Rules Directory: %s\n", config->ssl_rules_dir);
    }

#ifdef ENABLE_HA
        _dpd.logMsg("    SSL HA enabled     : %s\n", config->enable_ssl_ha ? "YES" : "NO" );
#endif
    _dpd.logMsg("    Maximum SSL Heartbeat length: %d\n", config->max_heartbeat_len);

}

static inline void SSLSetPort(SSLPP_config_t *config, int port)
{
    if (config == NULL)
        return;

    config->ports[ PORT_INDEX(port) ] |= CONV_PORT(port);
}



static void SSLPP_init_config(SSLPP_config_t *config)
{
    if (config == NULL)
        return;
    config->ssl_rules_dir = NULL;
    config->pki_dir = NULL;
    config->memcap = 100000;
    config->enable_ssl_ha = false;
    config->decrypt_memcap = 100000;
    config->current_handle = NULL;
    config->reload_handle = NULL;
    config->max_heartbeat_len = 0;

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

static void registerPortsForDispatch( struct _SnortConfig *sc, SSLPP_config_t *policy )
{
    int port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( policy->ports[ port / 8 ] & ( 1 << ( port % 8 ) ) )
        {
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_SSL, PROTO_BIT__TCP, port );
        }
    }
}

static void registerPortsForReassembly( SSLPP_config_t *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( policy->ports[ port / 8 ] & ( 1 << ( port % 8 ) ) )
        {
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
        }
    }
}


static void _addPortsToStream5Filter(struct _SnortConfig *sc, SSLPP_config_t *config, tSfPolicyId policy_id)
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
                (sc, IPPROTO_TCP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
        }
    }
}

#ifdef TARGET_BASED
static void _addServicesToStream5Filter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status
        (sc, ssl_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

void SSLPP_init(struct _SnortConfig *sc, char *args)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SSLPP_config_t *pPolicyConfig = NULL;

	/* For SFR CLI*/
	_dpd.controlSocketRegisterHandler(CS_TYPE_SSL_STATS, NULL, NULL, &DisplaySSLPPStats);
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

        SSL_InitGlobals();

        _dpd.registerPreprocStats("ssl", SSLPP_drop_stats);
        _dpd.addPreprocConfCheck(sc, SSLPP_CheckConfig);
        _dpd.addPreprocExit(SSLCleanExit, NULL, PRIORITY_LAST, PP_SSL);
        _dpd.addPreprocResetStats(SSLResetStats, NULL, PRIORITY_LAST, PP_SSL);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("ssl", (void *)&sslpp_perf_stats, 0, _dpd.totalPerfStats, NULL);
#endif

#ifdef ENABLE_HA
        _dpd.addFuncToPostConfigList(sc, SSLHAPostConfigInit, NULL);
#endif

#ifdef TARGET_BASED
        ssl_app_id = _dpd.findProtocolReference("ssl");
        if (ssl_app_id == SFTARGET_UNKNOWN_PROTOCOL)
        {
            ssl_app_id = _dpd.addProtocolReference("ssl");
        }
        _dpd.sessionAPI->register_service_handler( PP_SSL, ssl_app_id );
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

    _dpd.preprocOptRegister(sc, "ssl_state", SSLPP_state_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, "ssl_version", SSLPP_ver_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);

	_dpd.addPreproc( sc, SSLPP_process, PRIORITY_APPLICATION, PP_SSL, PROTO_BIT__TCP );

    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStream5Filter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStream5Filter(sc, policy_id);
#endif

}

static int SSLFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    SSLPP_config_t *pPolicyConfig = (SSLPP_config_t *)pData;

    //do any housekeeping before freeing SSLPP_config_t
    sfPolicyUserDataClear (config, policyId);

    if(pPolicyConfig->pki_dir)
        free(pPolicyConfig->pki_dir);

    if(pPolicyConfig->ssl_rules_dir)
        free(pPolicyConfig->ssl_rules_dir);


    free(pPolicyConfig);

    return 0;
}

static void SSLFreeConfig(tSfPolicyUserContextId config, bool full_cleanup)
{
    SSLPP_config_t *defaultConfig;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();
    if (config == NULL)
        return;

   defaultConfig = (SSLPP_config_t *)sfPolicyUserDataGetDefault(config);

    if(defaultConfig && ssl_cb)
    {
        ssl_cb->policy_free(&(defaultConfig->current_handle), full_cleanup);
#ifdef ENABLE_HA
        if(defaultConfig->ssl_ha_config)
        {
            SSLHAConfigFree(defaultConfig->ssl_ha_config);
            defaultConfig->ssl_ha_config = NULL;
        }
#endif
    }

    sfPolicyUserDataFreeIterate (config, SSLFreeConfigPolicy);
    sfPolicyConfigDelete(config);
}

static void SSLCleanExit(int signal, void *data)
{
    if (ssl_config != NULL)
    {
#ifdef ENABLE_HA
        SSLCleanHA();
#endif
        SSLFreeConfig(ssl_config, true);
        ssl_config = NULL;
    }
}

static void SSLResetStats(int signal, void *data)
{
    SSL_InitGlobals();
#ifdef ENABLE_HA
    SSLResetHAStats();
#endif
}

static int SSLPP_SetSSLPolicy(struct _SnortConfig *sc,
                tSfPolicyUserContextId config,
                tSfPolicyId policyId,
                void* pData
                )
{
    _dpd.setSSLPolicyEnabled(sc, policyId, true);

    return 0;
}

static int SSLPP_PolicyInit(struct _SnortConfig *sc, tSfPolicyUserContextId ssl_config, SSLPP_config_t *pPolicyConfig, tSfPolicyId policyId, bool reloading)
{
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();
    // Load SSL once for default policy
    if (pPolicyConfig != NULL)
    {
        if(pPolicyConfig->pki_dir && pPolicyConfig->ssl_rules_dir && ssl_cb)
        {
            if ( ssl_cb->policy_initialize(pPolicyConfig, reloading) != 0 )
            {
                _dpd.errMsg(
                    "SSLPP_PolicyInit(): Failed to initialize ssl_rules_dir and pki_dir.\n");
                return -1;
            }
            else
            {
                if((sfPolicyUserDataIterate (sc, ssl_config, SSLPP_SetSSLPolicy))!= 0)
                {
                    _dpd.errMsg(
                        "SSLPP_PolicyInit(): SetSSLpolicy failed.\n");
                    return -1;
                }
            }
        }
    }

    return 0;
}

static int SSLPP_CheckPolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    _dpd.setParserPolicy(sc, policyId);

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg(
            "SSLPP_CheckPolicyConfig(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    return 0;
}


//  Enable SSL preproc for any policies that have inspection turned on.
static int SSLPP_CheckPolicyEnabled(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    if(_dpd.isSSLPolicyEnabled(sc))
    {
        //  Send packets to SSL preproc even when only doing Network Discovery.
        _dpd.reenablePreprocBit(sc, PP_SSL);
    }

    return 0;
}


static int SSLPP_CheckConfig(struct _SnortConfig *sc)
{
#ifdef ENABLE_HA
    int haNotConfigured = 0;
#endif
    int rval;
    SSLPP_config_t *defaultConfig =
                    (SSLPP_config_t *)sfPolicyUserDataGetDefault(ssl_config);

    if ((rval = sfPolicyUserDataIterate (sc, ssl_config, SSLPP_CheckPolicyConfig)))
        return rval;

    // Load SSL once for default policy
    if (defaultConfig)
    {
        if( SSLPP_PolicyInit(sc, ssl_config, defaultConfig, _dpd.getDefaultPolicy(), false) != 0 )
            return -1;

#ifdef ENABLE_HA
        if (defaultConfig->enable_ssl_ha)
        {
            haNotConfigured = (SSLVerifyHAConfig(sc, defaultConfig->ssl_ha_config) != 0);
            if (haNotConfigured)
            {
                _dpd.errMsg("WARNING: SSL HA misconfigured.\n");
                return -1;
            }
        }
#endif
    }

    if ((rval = sfPolicyUserDataIterate (sc, ssl_config, SSLPP_CheckPolicyEnabled)))
        return rval;

    return 0;
}

#ifdef SNORT_RELOAD
void SSLReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId ssl_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
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
        *new_config = (void *)ssl_swap_config;
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

    _dpd.preprocOptRegister(sc, "ssl_state", SSLPP_state_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, "ssl_version", SSLPP_ver_init, SSLPP_rule_eval,
            free, NULL, NULL, NULL, NULL);

	_dpd.addPreproc(sc, SSLPP_process, PRIORITY_APPLICATION, PP_SSL, PROTO_BIT__TCP);

    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStream5Filter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStream5Filter(sc, policy_id);
#endif
}

int SSLReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId ssl_swap_config = (tSfPolicyUserContextId)swap_config;
    SSLPP_config_t *reload_config = NULL;
    SSLPP_config_t *start_config = NULL;
    tSfPolicyId policyId = _dpd.getDefaultPolicy();
    int ret = 0;
    bool register_sfssl_preproc_reload_flag = false;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("SSLPP_init(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    if (!ssl_swap_config || !ssl_config)
        return 0;

    reload_config = (SSLPP_config_t *)sfPolicyUserDataGet(ssl_swap_config, policyId);
    start_config = (SSLPP_config_t *)sfPolicyUserDataGet(ssl_config, policyId);

    if( !reload_config || !start_config )
    {
        _dpd.errMsg("SSL reload: Turning on or off SSL preprocessor requires a restart.\n");
        return -1;
    }

    if (ssl_cb && ssl_cb->reload_mem_adjust_available())
    {
       _dpd.logMsg("SSL reload: SFSSL reload memcap adjust is available.\n");
       register_sfssl_preproc_reload_flag = true;
    }

    if (!register_sfssl_preproc_reload_flag &&
        reload_config->memcap != start_config->memcap)
    {
        /* Reload mem adjust not available, restart snort. */
        _dpd.errMsg("SSL reload: Changing the memcap requires a restart.\n");
        return -1;
    }

    if (!register_sfssl_preproc_reload_flag &&
        reload_config->decrypt_memcap != start_config->decrypt_memcap)
    {
      /* Reload mem adjust not available, restart snort. */
      _dpd.errMsg("SSL reload: Changing the decrypt_memcap requires a restart.\n");
      return -1;
    }

    if (reload_config->memcap != start_config->memcap)
    {
        /* reload mem adjust is available, any change in sfssl memcap, adjust the difference in sftls memcap */
        reload_config->decrypt_memcap += reload_config->memcap - start_config->memcap;
        _dpd.logMsg("SSL reload: Change in sfssl memcap:%d, sftls memcap:%d.\n", reload_config->memcap, reload_config->decrypt_memcap);
    }

    ret = SSLPP_PolicyInit(sc, ssl_swap_config, reload_config, policyId, true);

    if (!ret)
    {
        start_config->reload_handle = reload_config->current_handle;
    }

    if (register_sfssl_preproc_reload_flag)
    {
      /* Always register the SFSSL preproc reload, if sftls memcap adjuster is available
       * even in case where decrypt memcap doesn't change, may be we will add sftls_memcap in
       * ssl_tuning.conf file which might be differrent.
       */
      ssl_cb->register_reload_mem_adjust(sc, reload_config);
    }

    return ret;
}


void * SSLReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId ssl_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = ssl_config;

    if (ssl_swap_config == NULL)
        return NULL;

    ssl_config = ssl_swap_config;

    return (void *)old_config;
}

void SSLReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    SSLFreeConfig((tSfPolicyUserContextId)data, false);
}
#endif
