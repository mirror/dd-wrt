/*
 * spp_ftptelnet.c
 *
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
 * Kevin Liu <kliu@sourcefire.com>
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
 * Description:
 *
 * This file initializes FTPTelnet as a Snort preprocessor.
 *
 * This file registers the FTPTelnet initialization function,
 * adds the FTPTelnet function into the preprocessor list, reads
 * the user configuration in the snort.conf file, and prints out
 * the configuration that is read.
 *
 * In general, this file is a wrapper to FTPTelnet functionality,
 * by interfacing with the Snort preprocessor functions.  The rest
 * of FTPTelnet should be separate from the preprocessor hooks.
 *
 * NOTES:
 * - 16.09.04:  Initial Development.  SAS
 *
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_debug.h"

#include "ftpp_ui_config.h"
#include "snort_ftptelnet.h"
#include "spp_ftptelnet.h"
#include "sf_preproc_info.h"

#include "profiler.h"

#include "sfPolicy.h"
#include "sfPolicyUserData.h"

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 2;
const int BUILD_VERSION = 13;
const char *PREPROC_NAME = "SF_FTPTELNET";

#define SetupFTPTelnet DYNAMIC_PREPROC_SETUP


/*
 * Defines for preprocessor initialization
 */
/*
 * snort.conf preprocessor keyword
 */
#define GLOBAL_KEYWORD   "ftp_telnet"
#define PROTOCOL_KEYWORD "ftp_telnet_protocol"

/*
 * The length of the error string buffer.
 */
#define ERRSTRLEN 1000

/*
 * External Global Variables
 * Variables that we need from Snort to log errors correctly and such.
 */
#ifdef PERF_PROFILING
PreprocStats ftpPerfStats;
PreprocStats telnetPerfStats;
#ifdef TARGET_BASED
PreprocStats ftpdataPerfStats;
#endif
#endif

/*
 * Global Variables
 * This is the only way to work with Snort preprocessors because
 * the user configuration must be kept between the Init function
 * the actual preprocessor.  There is no interaction between the
 * two except through global variable usage.
 */
tSfPolicyUserContextId ftp_telnet_config = NULL;
FTPTELNET_GLOBAL_CONF *ftp_telnet_eval_config = NULL;

#ifdef TARGET_BASED
int16_t ftp_app_id = 0;
int16_t ftp_data_app_id = 0;
int16_t telnet_app_id = 0;
#endif

/* static function prototypes */
static void FTPTelnetReset(int, void *);
static void FTPTelnetResetStats(int, void *);

#ifdef SNORT_RELOAD
static void FtpTelnetReloadGlobal(struct _SnortConfig *, char *, void **);
static void FtpTelnetReload(struct _SnortConfig *, char *, void **);
static int FtpTelnetReloadVerify(struct _SnortConfig *, void *);
static void * FtpTelnetReloadSwap(struct _SnortConfig *, void *);
static void FtpTelnetReloadSwapFree(void *);
#endif

extern char *maxToken;

/*
 * Function: FTPTelnetChecks(Packet *p)
 *
 * Purpose: This function wraps the functionality in the generic FTPTelnet
 *          processing.  We get a Packet structure and pass this into the
 *          FTPTelnet module where the first stage in FTPTelnet is the
 *          Normalization stage where most of the other Snortisms are
 *          taken care of.  After that, the modules are generic.
 *
 * Arguments: p         => pointer to a Packet structure that contains
 *                         Snort info about the packet.
 *
 * Returns: None
 *
 */
void FTPTelnetChecks(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket*)pkt;

    // precondition - what we registered for
    assert(IsTCP(p) && p->payload && p->payload_size);

    SnortFTPTelnet(p);
}

#ifdef TARGET_BASED
void FTPDataTelnetChecks(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket*)pkt;

    // precondition - what we registered for
    assert(IsTCP(p));

    if ( _dpd.fileAPI->get_max_file_depth() >= 0 )
    {
        if ( _dpd.sessionAPI->get_application_protocol_id(p->stream_session)
            == ftp_data_app_id )
        {
            PROFILE_VARS;
            PREPROC_PROFILE_START(ftpdataPerfStats);
            SnortFTPData(p);
            PREPROC_PROFILE_END(ftpdataPerfStats);
            return;
        }
    }
    if ( !p->payload_size || (p->payload == NULL) )
        return;

    SnortFTPTelnet(p);
}
#endif

/*
 * Function: FTPTelnetInit(char *args)
 *
 * Purpose: This function cleans up FTPTelnet memory from the configuration
 *          data.
 *
 * Arguments: sig       => signal causing this to be called
 *            args      => pointer to a context strucutre
 *
 * Returns: None
 *
 */
void FTPTelnetCleanExit(int sig, void *args)
{
    FTPTelnetFreeConfigs(ftp_telnet_config);
    ftp_telnet_config = NULL;
}

/*
 * Function: FTPTelnetInit(char *args)
 *
 * Purpose: This function initializes FTPTelnetInit with a user configuration.
 *          The function is called when FTPTelnet is configured in snort.conf.
 *          It gets passed a string of arguments, which gets parsed into
 *          configuration constructs that FTPTelnet understands.
 *
 *          This function gets called for every FTPTelnet configure line.  We
 *          use this characteristic to split up the configuration, so each
 *          line is a configuration construct.  We need to keep track of what
 *          part of the configuration has been configured, so we don't
 *          configure one part, then configure it again.
 *
 *          Any upfront memory is allocated here (if necessary).
 *
 * Arguments: args      => pointer to a string to the preprocessor arguments.
 *
 * Returns: None
 *
 */

extern char* mystrtok (char* s, const char* delim);

static void FTPTelnetInit(struct _SnortConfig *sc, char *args)
{
    char  *pcToken;
    char ErrorString[ERRSTRLEN];
    int iErrStrLen = ERRSTRLEN;
    int iRet = 0;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = NULL;

    ErrorString[0] = '\0';

    if ((args == NULL) || (strlen(args) == 0))
    {
        DynamicPreprocessorFatalMessage("%s(%d) No arguments to FtpTelnet "
                "configuration.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* Find out what is getting configured */
    maxToken = args + strlen(args);
    pcToken = mystrtok(args, CONF_SEPARATORS);
    if (pcToken == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d)mystrtok returned NULL when it "
                                        "should not.", __FILE__, __LINE__);
    }

    if (ftp_telnet_config == NULL)
    {
        //create a context
        ftp_telnet_config = sfPolicyConfigCreate();

        if (ftp_telnet_config == NULL)
        {
            DynamicPreprocessorFatalMessage("No memory to allocate "
                                            "FTP/Telnet configuration.\n");
        }

        _dpd.addPreprocExit(FTPTelnetCleanExit, NULL, PRIORITY_APPLICATION, PP_FTPTELNET);
        _dpd.addPreprocReset(FTPTelnetReset, NULL, PRIORITY_APPLICATION, PP_FTPTELNET);
        _dpd.addPreprocResetStats(FTPTelnetResetStats, NULL, PRIORITY_APPLICATION, PP_FTPTELNET);
        _dpd.addPreprocConfCheck(sc, FTPConfigCheck);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("ftptelnet_ftp", (void*)&ftpPerfStats, 0, _dpd.totalPerfStats);
        _dpd.addPreprocProfileFunc("ftptelnet_telnet", (void*)&telnetPerfStats, 0, _dpd.totalPerfStats);
        _dpd.addPreprocProfileFunc("ftptelnet_ftpdata", (void*)&ftpdataPerfStats, 0, _dpd.totalPerfStats);
#endif

#ifdef TARGET_BASED
        if (_dpd.streamAPI != NULL)
        {
            /* Find and store the application ID for FTP & Telnet */
            ftp_app_id = _dpd.addProtocolReference("ftp");
            ftp_data_app_id = _dpd.addProtocolReference("ftp-data");
            telnet_app_id = _dpd.addProtocolReference("telnet");
        }

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_FTPTELNET, ftp_app_id );
        _dpd.sessionAPI->register_service_handler( PP_FTPTELNET, ftp_data_app_id );
        _dpd.sessionAPI->register_service_handler( PP_FTPTELNET, telnet_app_id );
#endif
    }

    /*
     * Global Configuration Processing
     * We only process the global configuration once, but always check for
     * user mistakes, like configuring more than once.  That's why we
     * still check for the global token even if it's been checked.
     */
    sfPolicyUserPolicySet (ftp_telnet_config, policy_id);
    pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(ftp_telnet_config);
    if (pPolicyConfig == NULL)
    {
        if (strcasecmp(pcToken, GLOBAL) != 0)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Must configure the "
                "ftptelnet global configuration first.\n",
                *_dpd.config_file, *_dpd.config_line);
        }

        pPolicyConfig =
            (FTPTELNET_GLOBAL_CONF *)calloc(1, sizeof(FTPTELNET_GLOBAL_CONF));

        if (pPolicyConfig == NULL)
        {
            DynamicPreprocessorFatalMessage("No memory to allocate "
                                            "FTP/Telnet configuration.\n");
        }

        sfPolicyUserDataSetCurrent(ftp_telnet_config, pPolicyConfig);

        iRet = FtpTelnetInitGlobalConfig(pPolicyConfig,
                                         ErrorString, iErrStrLen);

        if (iRet == 0)
        {
            iRet = ProcessFTPGlobalConf(pPolicyConfig,
                                     ErrorString, iErrStrLen);

            if (iRet == 0)
            {
                PrintFTPGlobalConf(pPolicyConfig);

                _dpd.preprocOptRegister(sc, "ftp.bounce", &FTPPBounceInit, &FTPPBounceEval,
                        NULL, NULL, NULL, NULL, NULL);

#ifdef TARGET_BASED
                if (_dpd.streamAPI != NULL)
                {
                    _dpd.streamAPI->set_service_filter_status
                        (sc, ftp_app_id, PORT_MONITOR_SESSION, policy_id, 1);

                    _dpd.streamAPI->set_service_filter_status
                        (sc, telnet_app_id, PORT_MONITOR_SESSION, policy_id, 1);
                }
#endif
            }
        }
    }
    else if (strcasecmp(pcToken, TELNET) == 0)
    {
        iRet = ProcessTelnetConf(pPolicyConfig, ErrorString, iErrStrLen);
        enableFtpTelnetPortStreamServices( sc, &pPolicyConfig->telnet_config->proto_ports, NULL,
                                           SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT ); 
    }
    else if (strcasecmp(pcToken, FTP) == 0)
    {
        pcToken = NextToken(CONF_SEPARATORS);

        if ( !pcToken )
        {
            DynamicPreprocessorFatalMessage(
                "%s(%d) Missing ftp_telnet ftp keyword.\n",
                *(_dpd.config_file), *(_dpd.config_line));
        }
        else if (strcasecmp(pcToken, SERVER) == 0)
        {
            iRet = ProcessFTPServerConf(sc, pPolicyConfig, ErrorString, iErrStrLen);
        }
        else if (strcasecmp(pcToken, CLIENT) == 0)
        {
            iRet = ProcessFTPClientConf(sc, pPolicyConfig, ErrorString, iErrStrLen);
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) Invalid ftp_telnet ftp keyword.\n",
                                            *(_dpd.config_file), *(_dpd.config_line));
        }
    }
    else
    {
        DynamicPreprocessorFatalMessage("%s(%d) Invalid ftp_telnet keyword.\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    if (iRet)
    {
        if(iRet > 0)
        {
            /*
             * Non-fatal Error
             */
            if(*ErrorString)
            {
                _dpd.errMsg("WARNING: %s(%d) => %s\n",
                            *(_dpd.config_file), *(_dpd.config_line), ErrorString);
            }
        }
        else
        {
            /*
             * Fatal Error, log error and exit.
             */
            if(*ErrorString)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => %s\n",
                                                *(_dpd.config_file), *(_dpd.config_line), ErrorString);
            }
            else
            {
                /*
                 * Check if ErrorString is undefined.
                 */
                if(iRet == -2)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => ErrorString is undefined.\n",
                                                    *(_dpd.config_file), *(_dpd.config_line));
                }
                else
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Undefined Error.\n",
                                                    *(_dpd.config_file), *(_dpd.config_line));
                }
            }
        }
    }
}

/*
 * Function: SetupFTPTelnet()
 *
 * Purpose: This function initializes FTPTelnet as a Snort preprocessor.
 *
 *          It registers the preprocessor keyword for use in the snort.conf
 *          and sets up the initialization module for the preprocessor, in
 *          case it is configured.
 *
 *          This function must be called in InitPreprocessors() in plugbase.c
 *          in order to be recognized by Snort.
 *
 * Arguments: None
 *
 * Returns: None
 *
 */
void SetupFTPTelnet(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc(GLOBAL_KEYWORD, FTPTelnetInit);
    _dpd.registerPreproc(PROTOCOL_KEYWORD, FTPTelnetInit);
#else
    _dpd.registerPreproc(GLOBAL_KEYWORD, FTPTelnetInit, FtpTelnetReloadGlobal,
                         FtpTelnetReloadVerify, FtpTelnetReloadSwap,
                         FtpTelnetReloadSwapFree);
    _dpd.registerPreproc(PROTOCOL_KEYWORD, FTPTelnetInit,
                         FtpTelnetReload, NULL, NULL, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET, "Preprocessor: FTPTelnet is "
                "setup . . .\n"););
}

static void FTPTelnetReset(int signal, void *data)
{
    return;
}

static void FTPTelnetResetStats(int signal, void *data)
{
    return;
}

#ifdef SNORT_RELOAD
static void _FtpTelnetReload(struct _SnortConfig *sc, tSfPolicyUserContextId ftp_telnet_swap_config, char *args)
{
    char  *pcToken;
    char ErrorString[ERRSTRLEN];
    int iErrStrLen = ERRSTRLEN;
    int iRet = 0;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = NULL;

    ErrorString[0] = '\0';

    if ((args == NULL) || (strlen(args) == 0))
    {
        DynamicPreprocessorFatalMessage("%s(%d) No arguments to FtpTelnet "
                "configuration.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* Find out what is getting configured */
    maxToken = args + strlen(args);
    pcToken = mystrtok(args, CONF_SEPARATORS);
    if (pcToken == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d)mystrtok returned NULL when it "
                                        "should not.", __FILE__, __LINE__);
    }

    /*
     * Global Configuration Processing
     * We only process the global configuration once, but always check for
     * user mistakes, like configuring more than once.  That's why we
     * still check for the global token even if it's been checked.
     */
    sfPolicyUserPolicySet (ftp_telnet_swap_config, policy_id);
    pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(ftp_telnet_swap_config);

    if (pPolicyConfig == NULL)
    {
        if (strcasecmp(pcToken, GLOBAL) != 0)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Must configure the "
                "ftptelnet global configuration first.\n",
                *_dpd.config_file, *_dpd.config_line);
        }

        pPolicyConfig =
            (FTPTELNET_GLOBAL_CONF *)calloc(1, sizeof(FTPTELNET_GLOBAL_CONF));

        if (pPolicyConfig == NULL)
        {
            DynamicPreprocessorFatalMessage("No memory to allocate "
                                            "FTP/Telnet configuration.\n");
        }

        sfPolicyUserDataSetCurrent(ftp_telnet_swap_config, pPolicyConfig);

        iRet = FtpTelnetInitGlobalConfig(pPolicyConfig,
                                         ErrorString, iErrStrLen);

        if (iRet == 0)
        {
            iRet = ProcessFTPGlobalConf(pPolicyConfig,
                                     ErrorString, iErrStrLen);

            if (iRet == 0)
            {
                PrintFTPGlobalConf(pPolicyConfig);

                _dpd.preprocOptRegister(sc, "ftp.bounce", &FTPPBounceInit, &FTPPBounceEval,
                        NULL, NULL, NULL, NULL, NULL);
            }
        }
    }
    else if (strcasecmp(pcToken, TELNET) == 0)
    {
        iRet = ProcessTelnetConf(pPolicyConfig, ErrorString, iErrStrLen);
        enableFtpTelnetPortStreamServices( sc, &pPolicyConfig->telnet_config->proto_ports, NULL,
                                           SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT ); 
    }
    else if (strcasecmp(pcToken, FTP) == 0)
    {
        pcToken = NextToken(CONF_SEPARATORS);

        if ( !pcToken )
        {
            DynamicPreprocessorFatalMessage(
                "%s(%d) Missing ftp_telnet ftp keyword.\n",
                *(_dpd.config_file), *(_dpd.config_line));
        }
        else if (strcasecmp(pcToken, SERVER) == 0)
        {
            iRet = ProcessFTPServerConf(sc, pPolicyConfig, ErrorString, iErrStrLen);
        }
        else if (strcasecmp(pcToken, CLIENT) == 0)
        {
            iRet = ProcessFTPClientConf(sc, pPolicyConfig, ErrorString, iErrStrLen);
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) Invalid ftp_telnet ftp keyword.\n",
                                            *(_dpd.config_file), *(_dpd.config_line));
        }
    }
    else
    {
        DynamicPreprocessorFatalMessage("%s(%d) Invalid ftp_telnet keyword.\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    if (iRet)
    {
        if(iRet > 0)
        {
            /*
             * Non-fatal Error
             */
            if(*ErrorString)
            {
                _dpd.errMsg("WARNING: %s(%d) => %s\n",
                            *(_dpd.config_file), *(_dpd.config_line), ErrorString);
            }
        }
        else
        {
            /*
             * Fatal Error, log error and exit.
             */
            if(*ErrorString)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => %s\n",
                                                *(_dpd.config_file), *(_dpd.config_line), ErrorString);
            }
            else
            {
                /*
                 * Check if ErrorString is undefined.
                 */
                if(iRet == -2)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => ErrorString is undefined.\n",
                                                    *(_dpd.config_file), *(_dpd.config_line));
                }
                else
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Undefined Error.\n",
                                                    *(_dpd.config_file), *(_dpd.config_line));
                }
            }
        }
    }
}

static void FtpTelnetReloadGlobal(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId ftp_telnet_swap_config = (tSfPolicyUserContextId)*new_config;

    if (ftp_telnet_swap_config == NULL)
    {
        //create a context
        ftp_telnet_swap_config = sfPolicyConfigCreate();

        if (ftp_telnet_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("No memory to allocate "
                                            "FTP/Telnet swap_configuration.\n");
        }
        *new_config = (void *)ftp_telnet_swap_config;
    }
    _FtpTelnetReload(sc, ftp_telnet_swap_config, args);
}

static void FtpTelnetReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId ftp_telnet_swap_config;
    ftp_telnet_swap_config = (tSfPolicyUserContextId)_dpd.getRelatedReloadData(sc, GLOBAL_KEYWORD);
    _FtpTelnetReload(sc, ftp_telnet_swap_config, args);
}

static int FtpTelnetReloadVerifyPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    return FTPTelnetCheckConfigs( sc, pData, policyId );
}

static int FtpTelnetReloadVerify(struct _SnortConfig *sc, void *new_config)
{
    tSfPolicyUserContextId ftp_telnet_swap_config = (tSfPolicyUserContextId)new_config;
    if (ftp_telnet_swap_config == NULL)
        return 0;

    if (sfPolicyUserDataIterate (sc, ftp_telnet_swap_config, FtpTelnetReloadVerifyPolicy))
        return -1;

    return 0;
}

static int FtpTelnetReloadSwapPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)pData;

    //do any housekeeping before freeing FTPTELNET_GLOBAL_CONF
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        FTPTelnetFreeConfig(pPolicyConfig);
    }

    return 0;
}

static void * FtpTelnetReloadSwap(struct _SnortConfig *sc, void *new_config)
{
    tSfPolicyUserContextId ftp_telnet_swap_config = (tSfPolicyUserContextId)new_config;
    tSfPolicyUserContextId old_config = ftp_telnet_config;

    if (ftp_telnet_swap_config == NULL)
        return NULL;

    ftp_telnet_config = ftp_telnet_swap_config;

    sfPolicyUserDataIterate (sc, old_config, FtpTelnetReloadSwapPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        return (void *)old_config;

    return NULL;
}

static void FtpTelnetReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    FTPTelnetFreeConfigs((tSfPolicyUserContextId)data);
}

#endif
