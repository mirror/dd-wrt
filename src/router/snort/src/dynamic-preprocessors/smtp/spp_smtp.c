/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
/**************************************************************************
 *
 * spp_smtp.c
 *
 * Author: Andy Mullican
 *
 * Description:
 *
 * This file initializes SMTP as a Snort preprocessor.
 *
 * This file registers the SMTP initialization function,
 * adds the SMTP function into the preprocessor list.
 *
 * In general, this file is a wrapper to SMTP functionality,
 * by interfacing with the Snort preprocessor functions.  The rest
 * of SMTP should be separate from the preprocessor hooks.
 *
 **************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "spp_smtp.h"
#include "snort_smtp.h"
#include "smtp_config.h"
#include "smtp_log.h"

#include "preprocids.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "debug.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats smtpPerfStats;
PreprocStats smtpDetectPerfStats;
int smtpDetectCalled = 0;
#endif

#include "sf_types.h"
#include "mempool.h"

MemPool *smtp_mime_mempool = NULL;

tSfPolicyUserContextId smtp_config = NULL;
SMTPConfig *smtp_eval_config = NULL;

extern DynamicPreprocessorData _dpd;
extern SMTP smtp_no_session;
extern int16_t smtp_proto_id;

static void SMTPInit(char *);
static void SMTPDetect(void *, void *context);
static void SMTPCleanExitFunction(int, void *);
static void SMTPResetFunction(int, void *);
static void SMTPResetStatsFunction(int, void *);
static void _addPortsToStream5Filter(SMTPConfig *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStream5Filter(tSfPolicyId);
#endif
static void SMTPCheckConfig(void);

#ifdef SNORT_RELOAD
tSfPolicyUserContextId smtp_swap_config = NULL;
static void SMTPReload(char *);
static int SMTPReloadVerify(void);
static void * SMTPReloadSwap(void);
static void SMTPReloadSwapFree(void *);
#endif


/*
 * Function: SetupSMTP()
 *
 * Purpose: Registers the preprocessor keyword and initialization 
 *          function into the preprocessor list.  This is the function that
 *          gets called from InitPreprocessors() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void SetupSMTP(void)
{
    /* link the preprocessor keyword to the init function in the preproc list */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("smtp", SMTPInit);
#else
    _dpd.registerPreproc("smtp", SMTPInit, SMTPReload,
                         SMTPReloadSwap, SMTPReloadSwapFree);
#endif
}


/*
 * Function: SMTPInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void SMTPInit(char *args)
{
    SMTPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy();
    SMTPConfig * pPolicyConfig = NULL;

    if (smtp_config == NULL)
    {
        //create a context
        smtp_config = sfPolicyConfigCreate();
        if (smtp_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                            "configuration.\n");
        }

        /* Initialize the searches not dependent on configuration.
         * headers, reponsed, data, mime boundary regular expression */
        SMTP_SearchInit();

        /* zero out static SMTP global used for stateless SMTP or if there
         * is no session pointer */
        memset(&smtp_no_session, 0, sizeof(SMTP));

        /* Put the preprocessor function into the function list */
        /* _dpd.addPreproc(SMTPDetect, PRIORITY_APPLICATION, PP_SMTP, PROTO_BIT__TCP);*/
        _dpd.addPreprocExit(SMTPCleanExitFunction, NULL, PRIORITY_LAST, PP_SMTP);
        _dpd.addPreprocReset(SMTPResetFunction, NULL, PRIORITY_LAST, PP_SMTP);
        _dpd.addPreprocResetStats(SMTPResetStatsFunction, NULL, PRIORITY_LAST, PP_SMTP);
        _dpd.addPreprocConfCheck(SMTPCheckConfig);

#ifdef TARGET_BASED
        smtp_proto_id = _dpd.findProtocolReference(SMTP_PROTO_REF_STR);
        if (smtp_proto_id == SFTARGET_UNKNOWN_PROTOCOL)
            smtp_proto_id = _dpd.addProtocolReference(SMTP_PROTO_REF_STR);

        DEBUG_WRAP(DebugMessage(DEBUG_SMTP,"SMTP: Target-based: Proto id for %s: %u.\n",
                                SMTP_PROTO_REF_STR, smtp_proto_id););
#endif

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("smtp", (void*)&smtpPerfStats, 0, _dpd.totalPerfStats);        
#endif
    }

    sfPolicyUserPolicySet (smtp_config, policy_id);
    pPolicyConfig = (SMTPConfig *)sfPolicyUserDataGetCurrent(smtp_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("Can only configure SMTP preprocessor once.\n");
    }

    pPolicyConfig = (SMTPConfig *)calloc(1, sizeof(SMTPConfig));
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                        "configuration.\n");
    }
 
    sfPolicyUserDataSetCurrent(smtp_config, pPolicyConfig);

    SMTP_InitCmds(pPolicyConfig);
    SMTP_ParseArgs(pPolicyConfig, args);

    SMTP_CheckConfig(pPolicyConfig, smtp_config);

    if(pPolicyConfig->disabled)
        return;

    _dpd.addPreproc(SMTPDetect, PRIORITY_APPLICATION, PP_SMTP, PROTO_BIT__TCP);

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                "for SMTP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "command search.\n");
    }

    for (tmp = pPolicyConfig->cmds; tmp->name != NULL; tmp++)
    {
        pPolicyConfig->cmd_search[tmp->search_id].name = tmp->name;
        pPolicyConfig->cmd_search[tmp->search_id].name_len = tmp->name_len;
        
        _dpd.searchAPI->search_instance_add(pPolicyConfig->cmd_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(pPolicyConfig->cmd_search_mpse);

    _addPortsToStream5Filter(pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStream5Filter(policy_id);
#endif
}

/*
 * Function: SMTPDetect(void *, void *)
 *
 * Purpose: Perform the preprocessor's intended function.  This can be
 *          simple (statistics collection) or complex (IP defragmentation)
 *          as you like.  Try not to destroy the performance of the whole
 *          system by trying to do too much....
 *
 * Arguments: p => pointer to the current packet data struct 
 *
 * Returns: void function
 *
 */
static void SMTPDetect(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    tSfPolicyId policy_id = _dpd.getRuntimePolicy();
    PROFILE_VARS;

    if ((p->payload_size == 0) || !IsTCP(p) || (p->payload == NULL))
        return;

    PREPROC_PROFILE_START(smtpPerfStats);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP Start (((((((((((((((((((((((((((((((((((((((\n"););

    sfPolicyUserPolicySet (smtp_config, policy_id);

    SnortSMTP(p);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP End )))))))))))))))))))))))))))))))))))))))))\n\n"););

    PREPROC_PROFILE_END(smtpPerfStats);
#ifdef PERF_PROFILING
    if (PROFILING_PREPROCS && smtpDetectCalled)
    {
        smtpPerfStats.ticks -= smtpDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        smtpDetectPerfStats.ticks = 0;
        smtpDetectCalled = 0;
    }
#endif

}


/* 
 * Function: SMTPCleanExitFunction(int, void *)
 *
 * Purpose: This function gets called when Snort is exiting, if there's
 *          any cleanup that needs to be performed (e.g. closing files)
 *          it should be done here.
 *
 * Arguments: signal => the code of the signal that was issued to Snort
 *            data => any arguments or data structs linked to this 
 *                    function when it was registered, may be
 *                    needed to properly exit
 *       
 * Returns: void function
 */                   
static void SMTPCleanExitFunction(int signal, void *data)
{    
    SMTP_Free();
    if (mempool_destroy(smtp_mime_mempool) == 0)
    {
        free(smtp_mime_mempool);
        smtp_mime_mempool = NULL;
    }

}


static void SMTPResetFunction(int signal, void *data)
{
    return;
}

static void SMTPResetStatsFunction(int signal, void *data)
{
    return;
}

static void _addPortsToStream5Filter(SMTPConfig *config, tSfPolicyId policy_id)
{
    unsigned int portNum;

    if (config == NULL)
        return;

    for (portNum = 0; portNum < MAXPORTS; portNum++)
    {
        if(config->ports[(portNum/8)] & (1<<(portNum%8)))
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status(IPPROTO_TCP, (uint16_t)portNum,
                                                   PORT_MONITOR_SESSION, policy_id, 1);
        }
    }
}

#ifdef TARGET_BASED
static void _addServicesToStream5Filter(tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(smtp_proto_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int SMTPEnableMimeDecoding(tSfPolicyUserContextId config,
            tSfPolicyId policyId, void *pData)
{
    SMTPConfig *context = (SMTPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(context->enable_mime_decoding)
        return 1;

    return 0;
}

static int SMTPCheckPolicyConfig(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    SMTPConfig *context = (SMTPConfig *)pData;

    _dpd.setParserPolicy(policyId);

    /* In a multiple-policy setting, the SMTP preproc can be turned on in a
       "disabled" state. In this case, we don't require Stream5. */
    if (context->disabled)
        return 0;

    if (!_dpd.isPreprocEnabled(PP_STREAM5))
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for SMTP preprocessor\n");
    }

    return 0;
}

static void SetMaxMimeSession(SMTPConfig *pPolicyConfig)
{
    /*decode length will be shorter than encode length in base64 encoding and 
     * hence this calculation. */
    pPolicyConfig->max_mime_decode_bytes =
        ( (pPolicyConfig->max_mime_depth/4)*3 );

    if ( pPolicyConfig->max_mime_decode_bytes & 7)
    {
        (pPolicyConfig->max_mime_decode_bytes += 8 -
             (pPolicyConfig->max_mime_decode_bytes & 7));
    }
    pPolicyConfig->max_mime_sessions =
        pPolicyConfig->max_mime_mem / 
        (pPolicyConfig->max_mime_depth + pPolicyConfig->max_mime_decode_bytes);

}

static void SMTPCheckConfig(void)
{
    sfPolicyUserDataIterate (smtp_config, SMTPCheckPolicyConfig);
    {
        SMTPConfig *defaultConfig =
                (SMTPConfig *)sfPolicyUserDataGetDefault(smtp_config);

        if (sfPolicyUserDataIterate(smtp_config, SMTPEnableMimeDecoding) != 0)
        {
            int encode_depth;
            int decode_depth;

            if (defaultConfig == NULL)
            {
                /*error message */
                DynamicPreprocessorFatalMessage("SMTP: Must configure a default "
                        "configuration if you want to enable mime decoding.\n");
            }

            encode_depth = defaultConfig->max_mime_depth;

            if (encode_depth & 7)
            {
                encode_depth += (8 - (encode_depth & 7));
                defaultConfig->max_mime_depth = encode_depth;
            }

            SetMaxMimeSession(defaultConfig);

            decode_depth = defaultConfig->max_mime_decode_bytes;

            smtp_mime_mempool = (MemPool *)calloc(1, sizeof(MemPool));

            if (mempool_init(smtp_mime_mempool, defaultConfig->max_mime_sessions, 
                        (encode_depth + decode_depth)) != 0)
            {
                if(defaultConfig->max_mime_sessions)
                {
                    DynamicPreprocessorFatalMessage("SMTP: Error setting the \"max_mime_mem\" \n");
                }
                else
                {
                    DynamicPreprocessorFatalMessage("SMTP:  Could not allocate SMTP mempool.\n");
                }
            }
        }
    }

}

#ifdef SNORT_RELOAD
static void SMTPReload(char *args)
{
    SMTPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy();
    SMTPConfig *pPolicyConfig = NULL;

    if (smtp_swap_config == NULL)
    {
        //create a context
        smtp_swap_config = sfPolicyConfigCreate();
        if (smtp_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                            "configuration.\n");
        }

        _dpd.addPreprocReloadVerify(SMTPReloadVerify);
    }

    sfPolicyUserPolicySet (smtp_swap_config, policy_id);
    pPolicyConfig = (SMTPConfig *)sfPolicyUserDataGetCurrent(smtp_swap_config);

    if (pPolicyConfig != NULL)
        DynamicPreprocessorFatalMessage("Can only configure SMTP preprocessor once.\n");

    pPolicyConfig = (SMTPConfig *)calloc(1, sizeof(SMTPConfig));
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(smtp_swap_config, pPolicyConfig);

    SMTP_InitCmds(pPolicyConfig);
    SMTP_ParseArgs(pPolicyConfig, args);

    SMTP_CheckConfig(pPolicyConfig, smtp_swap_config);

    if( pPolicyConfig->disabled )
        return;

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for SMTP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "command search.\n");
    }

    for (tmp = pPolicyConfig->cmds; tmp->name != NULL; tmp++)
    {
        pPolicyConfig->cmd_search[tmp->search_id].name = tmp->name;
        pPolicyConfig->cmd_search[tmp->search_id].name_len = tmp->name_len;
        
        _dpd.searchAPI->search_instance_add(pPolicyConfig->cmd_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(pPolicyConfig->cmd_search_mpse);

    _dpd.addPreproc(SMTPDetect, PRIORITY_APPLICATION, PP_SMTP, PROTO_BIT__TCP);

    _addPortsToStream5Filter(pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStream5Filter(policy_id);
#endif
}

static int SMTPReloadVerify(void)
{
    SMTPConfig *config = NULL;
    SMTPConfig *configNext = NULL;

    if (smtp_swap_config == NULL)
        return 0;

    if (smtp_config != NULL)
    {
        config = (SMTPConfig *)sfPolicyUserDataGet(smtp_config, _dpd.getDefaultPolicy());
    }

    configNext = (SMTPConfig *)sfPolicyUserDataGet(smtp_swap_config, _dpd.getDefaultPolicy());

    if (config == NULL)
    {
        return 0;
    }

    if (smtp_mime_mempool != NULL)
    {
        if (configNext == NULL)
        {
            _dpd.errMsg("SMTP reload: Changing the max_mime_depth or max_mime_mem requires a restart.\n");
            SMTP_FreeConfigs(smtp_swap_config);
            smtp_swap_config = NULL;
            return -1;
        }
        if (configNext->max_mime_mem != config->max_mime_mem)
        {
            _dpd.errMsg("SMTP reload: Changing the max_mime_mem requires a restart.\n");
            SMTP_FreeConfigs(smtp_swap_config);
            smtp_swap_config = NULL;
            return -1;
        }
        if (configNext->max_mime_depth & 7)
            configNext->max_mime_depth += (8 - (configNext->max_mime_depth & 7));

        SetMaxMimeSession(configNext);

        if (configNext->max_mime_depth != config->max_mime_depth)
        {
            _dpd.errMsg("SMTP reload: Changing the max_mime_depth requires a restart.\n");
            SMTP_FreeConfigs(smtp_swap_config);
            smtp_swap_config = NULL;
            return -1;
        }
    }
    else if(configNext != NULL)
    {

        if (sfPolicyUserDataIterate(smtp_swap_config, SMTPEnableMimeDecoding) != 0)
        {
            int encode_depth;
            int decode_depth;

            encode_depth = configNext->max_mime_depth;

            if (encode_depth & 7)
            {
                encode_depth += (8 - (encode_depth & 7));
                configNext->max_mime_depth = encode_depth;
            }

            SetMaxMimeSession(configNext);
            decode_depth = configNext->max_mime_decode_bytes;

            smtp_mime_mempool = (MemPool *)calloc(1, sizeof(MemPool));

            if (mempool_init(smtp_mime_mempool, configNext->max_mime_sessions, 
                        (encode_depth + decode_depth)) != 0)
            {
                if(configNext->max_mime_sessions)
                {
                    DynamicPreprocessorFatalMessage("SMTP: Error setting the \"max_mime_mem\" \n");
                }
                else
                {
                    DynamicPreprocessorFatalMessage("SMTP:  Could not allocate SMTP mempool.\n");
                }
            }
        }

    }
       

    if ( configNext->disabled )
        return 0;


    if (!_dpd.isPreprocEnabled(PP_STREAM5))
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for SMTP preprocessor\n");
    }

    return 0;
}

static int SMTPReloadSwapPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    SMTPConfig *pPolicyConfig = (SMTPConfig *)pData;

    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        SMTP_FreeConfig(pPolicyConfig);
    }
     
    return 0;
}

static void * SMTPReloadSwap(void)
{
    tSfPolicyUserContextId old_config = smtp_config;

    if (smtp_swap_config == NULL)
        return NULL;

    smtp_config = smtp_swap_config;
    smtp_swap_config = NULL;

    sfPolicyUserDataIterate (old_config, SMTPReloadSwapPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        SMTP_FreeConfigs(old_config);

    return NULL;
}

static void SMTPReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    SMTP_FreeConfigs((tSfPolicyUserContextId)data);
}
#endif
