/*
*****************************************************************************
*
* This file is provided under a dual BSD/GPLv2 license.  When using or 
*   redistributing this file, you may do so under either license.
* 
*   GPL LICENSE SUMMARY
* 
*   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
* 
*   This program is free software; you can redistribute it and/or modify 
*   it under the terms of version 2 of the GNU General Public License as
*   published by the Free Software Foundation.
* 
*   This program is distributed in the hope that it will be useful, but 
*   WITHOUT ANY WARRANTY; without even the implied warranty of 
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
*   General Public License for more details.
* 
*   You should have received a copy of the GNU General Public License 
*   along with this program; if not, write to the Free Software 
*   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
*   The full GNU General Public License is included in this distribution 
*   in the file called LICENSE.GPL.
* 
*   Contact Information:
*   Intel Corporation
* 
*   BSD LICENSE 
* 
*   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
*   All rights reserved.
* 
*   Redistribution and use in source and binary forms, with or without 
*   modification, are permitted provided that the following conditions 
*   are met:
* 
*     * Redistributions of source code must retain the above copyright 
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright 
*       notice, this list of conditions and the following disclaimer in 
*       the documentation and/or other materials provided with the 
*       distribution.
*     * Neither the name of Intel Corporation nor the names of its 
*       contributors may be used to endorse or promote products derived 
*       from this software without specific prior written permission.
* 
*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
*   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
*   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
*   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
*  version: Security.L.1.0.3-98
*
*****************************************************************************/

/**
 *****************************************************************************
 * @file  qatal_cfg.c
 *
 * @ingroup icp_Qatal
 *
 * @description
 *        This file contains the functions to used by ASD to initialise and
 *        shutdown the QATAL and QAT.
 ****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "cpa.h"
#include "cpa_types.h"

#include "icp.h"
#include "icp_osal_types.h"

#include "qatal_mem.h"
     
#include "icp_asd_cfg.h"

#include "icp_rings.h"

#include "icp_dcc_al.h"

#include "icp_arch_interfaces.h"
#include "icp_qat_fw.h"
#include "icp_qat_fw_init.h"
#include "icp_qat_fw_admin.h"
#include "icp_qat_fw_la.h"

#include "icp_accel_handle.h"

#include "halAeApi.h"
#include "uclo.h"
#include "uclo_status.h"
#include "qatal_init_defs.h"
#include "qatal_common.h"
#include "qatal_log.h"
#include "qatal_mem.h"
#include "qatal_thread.h"
#include "qat_comms.h"
#include "icp_qatal_cfg.h"
#include "qatal_rand.h"
#include "qatal_stats.h"

#ifdef HOOK_FUNC
#include "hookFunctions.h"
#endif //HOOK_FUNC

#define MMP_LIB_DATA_SZ 16
#define DATA_WORD_SZ 4
#define DATA_SPACING 3


/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

#define QAT_ICP_DCC_COMPONENT_NAME_LENGTH (16)

/*            Q      A     T    _     F     W      _     _    _     _ */
Cpa8U   FW_NAME[QAT_ICP_DCC_COMPONENT_NAME_LENGTH] =
           {0x51, 0x41, 0x54, 0x5F, 0x46, 0x57, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 
                  0x5F, 0x5F, 0x5F, 0x5F, 0x5F };

/*            Q     A     T    _      A     L      _     _    _     _ */
Cpa8U   IA_NAME[QAT_ICP_DCC_COMPONENT_NAME_LENGTH] =
            {0x51, 0x41, 0x54, 0x5F, 0x41, 0x4C, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F,
                   0x5F, 0x5F, 0x5F, 0x5F, 0x5F };


icp_dcc_ver_info_t IA_Version_Info = {" ", 2, 1, 0};  /* QAT-AL non firmware*/

/*
*******************************************************************************
* STATIC Variables
*******************************************************************************
*/

/* status of QAT-AL */
STATIC qatal_comp_state_t qatalState = QATAL_COMP_SHUT_DOWN;

/* MMP address - used in INIT_MMP msg. */
STATIC icp_asd_cfg_value_t mmpLibAddr = 0;
STATIC icp_asd_cfg_value_t mmpLibSize = 0;

/* Ring Table Information */
STATIC icp_qat_fw_init_ring_table_t *pRingTable = NULL;

/* RAND Entrphy Block */
STATIC Cpa8U *pQATAL_NGB_BLOCK = NULL;

/* mmp Lib Block */
STATIC Cpa8U *pmmpLibCopy = NULL;



/* Thread id being monitored  */
STATIC icp_dcc_thread_id_t ThreadId ;

/* For Version info */

STATIC Cpa32U FW_ModuleId = 1235;
STATIC Cpa32U IA_ModuleId = 1236;



STATIC icp_dcc_ver_info_t Version_Data = {" ", 99, 99, 99};


/* For QAT-FW Liveness */
STATIC Cpa16U g_qat_Liveness_Data = 0;

/* Confirm Admin is Read */
STATIC Cpa16U  ADMIN_READ = 0;

/* indicates that the responce has been received */
STATIC CpaBoolean completion = CPA_FALSE;

icp_asd_cfg_param_get_cb_func_t g_getCfgParamFunc = 0;

/* Command Received by Feirmware */
STATIC   Cpa32U        g_qat_rec_data = 0;
/* Command Sent by Firmware */
STATIC   Cpa32U        g_qat_send_data = 0;

#ifdef HEALTH_CHECK

/* Sample information */
Cpa32U    Sampled[100];

/* Rand Sample address */
Cpa32U    *pRandSample = NULL;

/* index to block of Rand Samples */
Cpa32U    Qat_Random_Index = 0;

/* Init Gather state */
Cpa8U        Gather_state = ICP_QAT_FW_LA_DRBG_GATHER_STATE_RESTART;

#endif //HEALTH_CHECK

icp_accel_dev_t * pAccelHandle = NULL;

/*
*******************************************************************************
* Define STATIC function definitions
*******************************************************************************
*/



/* Callback function to tell user response received */
STATIC void Qatal_InitCallbackFunc(Qat_CallbackTag correlator);

/* Init message response handler */
STATIC void Qatal_InitRespHandler(void *pRespMsg,
                                  icp_arch_if_request_t msgType);



/* Admin message response handler */
STATIC void Qatal_AdminRespHandler(void *pRespMsg,
                                  icp_arch_if_request_t msgType);

/* This builds up, sends and waits for a response for the desired Admin msg*/
STATIC CpaStatus QatalCfg_AdminMsgSendAndWait(
                                         CpaInstanceHandle instanceHandle,
                                         icp_qat_fw_admin_cmd_id_t adminCmdId,
                                         Cpa64U correlator);

/* This builds up, sends and waits for a response for the desired Init msg*/
STATIC CpaStatus QatalCfg_InitMsgSendAndWait(CpaInstanceHandle instanceHandle,
                                         icp_qat_fw_init_cmd_id_t initCmdId,
                                         Cpa64U correlator);



/* This builds the desired Admin msg */
STATIC CpaStatus Qatal_AdminMsgBuild(icp_qat_fw_admin_req_t *pRingInfoRq,
                                       icp_qat_fw_admin_cmd_id_t adminCmdId,
                                       Cpa64U correlator);

/* This builds the desired Init msg */
STATIC CpaStatus Qatal_InitMsgBuild(icp_qat_fw_init_req_t *pRingInfoRq,
                                       icp_qat_fw_init_cmd_id_t initCmdId,
                                       Cpa64U correlator);



/* Put default data into header of message */
STATIC CpaStatus Qatal_ComnReqHdrPopulate(
                                        icp_qat_fw_comn_req_hdr_t *pComnReq,
                                        Cpa64U correlator);



/* This is callback function invoked by DCC to generate Liveness information */
STATIC CpaStatus icp_Qatal_LivenessGet(
                                icp_dcc_thread_id_t  *pThreadId,
                                icp_dcc_thread_status_t *pThreadStatus);



#ifdef HEALTH_CHECK
/* Rand message response handler */
STATIC void Qatal_RandRespHandler(void *pRespMsg,
                                  icp_arch_if_request_t msgType);

/* Put default data into header of message */
STATIC CpaStatus Qatal_RandReqHdrPopulate(
                                        icp_qat_fw_comn_req_hdr_t *pComnReq,
                                        Cpa64U correlator);

/* This builds the desired Rand msg */
STATIC CpaStatus Qatal_RandMsgBuild(icp_qat_fw_la_rng_req_t *pRingInfoRq,
                                       icp_qat_fw_la_cmd_id_t initCmdId,
                                       Cpa64U correlator);

/* This builds up, sends and waits for a response for the desired Rand msg*/
STATIC CpaStatus QatalCfg_RandMsgSendAndWait(CpaInstanceHandle instanceHandle,
                                         icp_qat_fw_la_cmd_id_t randCmdId,
                                         Cpa64U correlator);



#endif //HEALTH_CHECK



/*
*******************************************************************************
* Global Variables
*******************************************************************************
*/

/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

/*
 * @fn icp_AsdCfgQatalInit
 */
CpaStatus icp_AsdCfgQatalInit(
                        CpaInstanceHandle instanceHandle,
                        icp_asd_cfg_param_get_cb_func_t getCfgParamFunc)
{

    icp_asd_cfg_param_get_cb_func_t pAsdGetParamFunc=NULL;
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaStatus status2 = CPA_STATUS_SUCCESS;
    CpaStatus tmpStatus = CPA_STATUS_SUCCESS;

    Cpa8U        *pMsg = NULL;
    Cpa32U    mmpAddr_S = 0;
    Cpa32U     i = 0;


    if (QATAL_COMP_SHUT_DOWN != qatalState)
    {
        QATAL_ERROR_LOG("Component is not in correct state for init\n");
        status = CPA_STATUS_FAIL;
        return status;
    }

    if (CPA_STATUS_SUCCESS == status)
    {

        QATAL_DEBUG_LOG_OPTIONAL("QATAL - Starting Qatal INIT\n");

        /* initialize stats  - RC not important*/
        Qatal_StatsInit();

        QATAL_CHECK_NULL_PARAM(getCfgParamFunc);

        g_getCfgParamFunc = getCfgParamFunc;

        /* change state to be in the process of being initialised */
        qatalState = QATAL_COMP_INITIALISING;

        QATAL_DEBUG_LOG_OPTIONAL("Initializing QAT-AL ...\n");
        
    /* allocate memory for ring table */
        status = QATAL_MEM_SHARED_ALLOC(&pRingTable,
                                    sizeof(icp_qat_fw_init_ring_table_t) );

        if (CPA_STATUS_SUCCESS != status)
        {
            QATAL_ERROR_LOG("Failed to allocate ring table memory\n");
            status = CPA_STATUS_RESOURCE;
        }
        else
        {
            QATAL_DEBUG_LOG_OPTIONAL("QATAL - allocated Ring Table\n");
        }
    }

    /* get MMP library base address and store it for start function */
    pAsdGetParamFunc = (icp_asd_cfg_param_get_cb_func_t)(g_getCfgParamFunc);
    status = (*pAsdGetParamFunc)(ICP_ASD_CFG_PARAM_MMP_ADDRESS, &mmpLibAddr);

    status2 = 
      (*pAsdGetParamFunc)(ICP_ASD_CFG_PARAM_MMP_SIZE_BYTES, &mmpLibSize);

    if ((0 == mmpLibAddr)|| (0 == mmpLibSize) || 
        (CPA_STATUS_SUCCESS != status) || (CPA_STATUS_SUCCESS != status2 ))
    {
            QATAL_ERROR_LOG("Failed to Get mmpLib Data \n");
            status = CPA_STATUS_FAIL;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
       /* allocate memory for copy of mmpLib */
        status = QATAL_MEM_SHARED_ALLOC(&pmmpLibCopy,
                                    (int)mmpLibSize );

        if ((CPA_STATUS_SUCCESS == status) && (NULL != pmmpLibCopy))
        {

            ixOsalStdLog("MMP Lib Addr >>");
            ixOsalStdLog("%016llX",(long long unsigned int)mmpLibAddr);
            ixOsalStdLog("\n");

            ixOsalStdLog("MMP Lib Data >>");

            mmpAddr_S = (mmpLibAddr & 0xFFFFFFFF);
            pMsg = (Cpa8U *)(unsigned int)mmpAddr_S;
            for (i = 0; i < MMP_LIB_DATA_SZ ; i++)
            {
                ixOsalStdLog("%02X", pMsg[i]);
                if (i%DATA_WORD_SZ ==DATA_SPACING)
                {
                    ixOsalStdLog(" ");
                }
            }
            ixOsalStdLog(" <<< End of msg \n");


            QATAL_MEM_CPY((void *)pmmpLibCopy,(void *)(unsigned int)mmpAddr_S ,
                            (int) mmpLibSize);

        }
        else
        {
            QATAL_ERROR_LOG("Failed to allocate memory for copy of mmpLib\n");
            status = CPA_STATUS_FAIL;
        }
    }
    if (CPA_STATUS_SUCCESS == status)
    {

        /* initialize QAT Comms sub-component (rings) */

        status = QatComms_Init();
        if (CPA_STATUS_SUCCESS != status)
        {
            QATAL_ERROR_LOG("Failed to initialize QAT comms module\n");
            status = CPA_STATUS_FAIL;
        }
        else
        {
            QATAL_DEBUG_LOG_OPTIONAL("QATAL - initialized Qatcomms\n");

            /* set the CMPAL init msg response callback handler */
            status = QatComms_ResponseCbSet(Qatal_InitRespHandler,
                               ICP_ARCH_IF_REQ_QAT_FW_INIT);

            if (CPA_STATUS_SUCCESS != status)
            {
                QATAL_ERROR_LOG(
                    "Error QAT comms callback on Init Response\n");
                status = CPA_STATUS_FAIL;
            }
            else
            {

                /* set the CMPAL admin msg response callback handler */
                status = QatComms_ResponseCbSet(Qatal_AdminRespHandler,
                               ICP_ARCH_IF_REQ_QAT_FW_ADMIN);

                if (CPA_STATUS_SUCCESS != status)
                {
                    QATAL_ERROR_LOG(
                        "Failed to initialize callback Admin Response\n");
                    status = CPA_STATUS_FAIL;
                }
            }
        }
    }

    /* initialize QAT random number generation access layer module */
    if(CPA_STATUS_SUCCESS == status)
    {
        /* initialize Rand module */
        if(CPA_STATUS_SUCCESS != Qatal_RandInit())
        {
            QATAL_ERROR_LOG(
                "Failed to initialize random number generator module.\n");
            status = CPA_STATUS_FAIL;
        }
        else
        {
            QATAL_DEBUG_LOG_OPTIONAL("QATAL - RAND Entrphy Block \n");
        }
    }


    /* patch symbols into QAT-ME UOF image */
    if(CPA_STATUS_SUCCESS == status)
    {
        pAccelHandle = (icp_accel_dev_t *)instanceHandle;

   
        if(UCLO_SUCCESS != UcLo_BindSymbol(
                                pAccelHandle->icp_firmware_loader_handle,
                                "*",
                                QATAL_ADMIN_RING_SYMBOL_NAME,
                                QATAL_ADMIN_RING_SYMBOL_VAL))
        {
            QATAL_ERROR_LOG("Failed to patch AE symbols\n");
            status = CPA_STATUS_FAIL;
        }
        else
        {
            QATAL_DEBUG_LOG_OPTIONAL("Successfully patched QAT-AE symbols\n");
        }
    }

#ifdef HOOK_FUNC
    if(CPA_STATUS_SUCCESS == status)
    {
        status = patchOtherAESymbols();
    }
#endif //HOOK_FUNC

    if(CPA_STATUS_SUCCESS == status)
    {
        /* change state to initialised */
        qatalState = QATAL_COMP_INITIALISED;
        QATAL_STD_LOG("Successfully initialized QAT-AL component\n");
    }
    else
    {
        if (pmmpLibCopy != NULL)
        {
            /* free allocated memory */
            QATAL_MEM_SHARED_FREE(&pmmpLibCopy);
            pmmpLibCopy = NULL;
        }

        tmpStatus = QatComms_Shutdown();

        if (pRingTable != NULL) 
        { 
            /* free allocated memory */
            QATAL_MEM_SHARED_FREE(&pRingTable);
            pRingTable = NULL;
        } 

        /* Something has gone wrong therefore reporting the success or
           failure of RAND Shutdown is not of any
           good use .. therefor discard the return */
        tmpStatus = Qatal_RandShutdown();

        QATAL_ERROR_LOG("Failed to start QAT-AL component\n");
    }


    return status;
}
/****************************************************************************/

/*
 * @fn icp_AsdCfgQatalStart
 */
CpaStatus icp_AsdCfgQatalStart(CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_dcc_ver_info_t FW_Version_Info = {" ", 0, 0, 0};

    Cpa32U    Random_Loop = 0;
    Cpa32U    GetSend = 0;
    Cpa32U    GetRec = 0;
    Cpa32U    GetRetry = 0;

#ifdef HEALTH_CHECK
    CpaStatus status2 = CPA_STATUS_SUCCESS;
    icp_qat_fw_la_rng_req_t randMsg;
    Cpa64U correlator=0;
#endif //HEALTH_CHECK

    /* increment No. of Start commands */
    Qatal_StatsInc(QATAL_STATS_START_REQ);

    if (QATAL_COMP_INITIALISED != qatalState)
    {
        QATAL_ERROR_LOG("Component is not in correct state for starting\n");
        return CPA_STATUS_FAIL;
    }
    /* change state to be in the process of being starting */
    qatalState = QATAL_COMP_STARTING;

    QATAL_STD_LOG("Starting QAT-AL ...\n");

    /* start the QAT-AE */
    if (UCLO_SUCCESS != halAe_Start((unsigned char)QATAL_QAT_AE_NUM,
                                          QATAL_ENABLE_CONTEXT_MASK))
    {
        QATAL_ERROR_LOG("QAT-AE Start Failed\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        QATAL_DEBUG_LOG_OPTIONAL("Successfully started QAT-AE\n");
    }

#ifdef HOOK_FUNC
    if(CPA_STATUS_SUCCESS != startOtherAE())
    {
        QATAL_ERROR_LOG("QAT-AE Start Failed\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        QATAL_DEBUG_LOG_OPTIONAL("Successfully started QAT-AE\n");
    }
#endif //HOOK_FUNC


    /* Send init msgs to QAT-AE */
    /******************************************************/

    /* Send the CONFIG_NRBG message */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatalCfg_InitMsgSendAndWait(
                    instanceHandle,
                    ICP_QAT_FW_INIT_CMD_CONFIG_NRBG,
                    QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
    }
    /* Checking CONFIG NRBG Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        QATAL_DEBUG_LOG_OPTIONAL("QATAL - CONFIG NRGB OK\n");
    }
    else
    {
        QATAL_ERROR_LOG("Sending CONFIG_NRBG msg Failed\n");

        status = CPA_STATUS_FAIL;
    }


    /* Test QAT random number generation access layer module */
    if(CPA_STATUS_SUCCESS == status)
    {
        /* initialize Rand module */
        if(CPA_STATUS_SUCCESS != icp_QatalRandEntrophyTestRun() )
        {
            QATAL_ERROR_LOG("Failed TEST of RBG \n");
            status = CPA_STATUS_FAIL;
        }
        else
        {
            QATAL_DEBUG_LOG_OPTIONAL("QATAL - Qatal Rand Test Passed\n");
        }
    }

    /* Enable DRBG message */
    if (CPA_STATUS_SUCCESS == status)
    {
         status = QatalCfg_InitMsgSendAndWait(
                            instanceHandle,
                            ICP_QAT_FW_INIT_CMD_ENABLE_DRBG,
                            QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
    }
    /* Checking ENABLE DRBG Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        QATAL_DEBUG_LOG_OPTIONAL("QATAL - Send ENABLE DRBG ok\n");
    }
    else
    {
        QATAL_ERROR_LOG("Sending ENABLE_DRBG msg Failed\n");
        status = CPA_STATUS_FAIL;
    }

    /* Send SET CONSTANTS msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatalCfg_InitMsgSendAndWait(
                        instanceHandle,
                        ICP_QAT_FW_INIT_CMD_SET_CONSTS,
                        QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
    }

    /* Checking Set Constants Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        QATAL_DEBUG_LOG_OPTIONAL("QATAL - Sending SET CONSTANTS ok\n");
    }
    else
    {

        QATAL_ERROR_LOG("Sending SET CONSTANTS msg Failed\n");
        status = CPA_STATUS_FAIL;

    }

    /* Send INIT_MMP msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatalCfg_InitMsgSendAndWait(
                            instanceHandle,
                            ICP_QAT_FW_INIT_CMD_INIT_MMP,
                            QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
    }

    /* Checking INIT MMP Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        QATAL_DEBUG_LOG_OPTIONAL("QATAL - Sending INIT MMP ok\n");
    }
    else
    {
        QATAL_ERROR_LOG("Sending INIT_MMP msg Failed\n");
        status = CPA_STATUS_FAIL;
    }

    /* Send the SET_RING_INFO message */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatalCfg_InitMsgSendAndWait(
                    instanceHandle,
                    ICP_QAT_FW_INIT_CMD_SET_RING_INFO,
                    QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
    }

    /* Checking Ring_Info Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        QATAL_DEBUG_LOG_OPTIONAL("QATAL - Sending Set Ring info ok\n");
    }
    else
    {
        QATAL_ERROR_LOG("Sending SET_RING_INFO msg Failed\n");
        status = CPA_STATUS_FAIL;
    }

    /* Send INIT_FINAL msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatalCfg_InitMsgSendAndWait(
                    instanceHandle,
                    ICP_QAT_FW_INIT_CMD_INIT_FINAL,
                    QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        QATAL_DEBUG_LOG_OPTIONAL("QATAL - Sending INIT FINAL ok\n");
    }
    else
    {
        QATAL_ERROR_LOG("Sending INIT_FINAL msg Failed\n");
        status = CPA_STATUS_FAIL;
    }
   /******************************************************/
   /* end of sending init messages */

    QATAL_DEBUG_LOG_OPTIONAL("About to send admin message\n");

    /* Send Version Info with DCC */
    if (CPA_STATUS_SUCCESS == status)
    {

        status = QatalCfg_AdminMsgSendAndWait(
                        instanceHandle,
                        ICP_QAT_FW_ADMIN_CMD_GET,
                        QATAL_MEM_CAST_PTR_TO_UINT64(&completion));

        if (CPA_STATUS_SUCCESS == status)
        {
            QATAL_MEM_CPY(IA_Version_Info.name,IA_NAME,
                            ICP_DCC_COMPONENT_NAME_LENGTH);
            QATAL_MEM_CPY(FW_Version_Info.name,FW_NAME,
                              ICP_DCC_COMPONENT_NAME_LENGTH);


            status = icp_DccComponentVersionRegister(
                            &IA_Version_Info,
                            &IA_ModuleId) ;

            if (CPA_STATUS_SUCCESS == status)
            {

                FW_Version_Info.majorVersion    = Version_Data.majorVersion;
                FW_Version_Info.minorVersion    = Version_Data.minorVersion;
                FW_Version_Info.patchVersion    = Version_Data.patchVersion;

                status = icp_DccComponentVersionRegister(
                                &FW_Version_Info,
                                &FW_ModuleId) ;
                if (CPA_STATUS_SUCCESS != status)
                {
                    QATAL_ERROR_LOG(
                          "Sending icp_DccComponentVersionRegister Failed\n");
                }
            }
            else
            {
                QATAL_ERROR_LOG(
                    "Sending icp_DccComponentVersionRegister IA Failed\n");
            }
        }
        else
        {
            QATAL_ERROR_LOG("Sending get status failed Failed\n");
        }

    }


    /* Register liveness with DCC */
   if (CPA_STATUS_SUCCESS == status)
   {
        status = icp_DccLivenessVerificationHandlerRegister (
            &ThreadId,
            (icp_DccLivenessVerificationHandler)(& icp_Qatal_LivenessGet),
            NULL) ;


        if(CPA_STATUS_SUCCESS == status)
        {
            QATAL_DEBUG_LOG_OPTIONAL(
                "Callback Function AsdQatalLivenessGet Registered\n");
        }
        else
        {
            QATAL_ERROR_LOG("Failed Callback Function AsdQatalLivenessGet\n");
        }
    }


    if(CPA_STATUS_SUCCESS == status)
    {
        /* change state to started */
        qatalState = QATAL_COMP_STARTED;
        QATAL_STD_LOG("Successfully started QAT-AL component\n");
    }
    else
    {
        QATAL_ERROR_LOG("Failed to start QAT-AL component\n");
        return status;
    }
#ifdef HEALTH_CHECK

    /* set the QATAL admin msg response callback handler */
    status = QatComms_ResponseCbSet(Qatal_RandRespHandler,
                               ICP_ARCH_IF_REQ_QAT_FW_LA);

    if (CPA_STATUS_SUCCESS != status)
    {
        QATAL_ERROR_LOG(
                        "Failed to initialize callback Admin Response\n");
        return CPA_STATUS_FAIL;
    }

    QATAL_DEBUG_LOG_OPTIONAL("QAT-AL Building Random Sample\n");

       /* allocate memory for copy of Rand Sample */
    status = QATAL_MEM_SHARED_ALLOC(&pRandSample,
                                    4 );

    for (Random_Loop =0; Random_Loop < 1000; Random_Loop ++)
    {
     ixOsalStdLog("Stage 1/2 Loop %d of 1000 \n",Random_Loop);

       /* Send the CONFIG_NRBG message */
        if (CPA_STATUS_SUCCESS == status)
        {
            status = QatalCfg_RandMsgSendAndWait(
                    instanceHandle,
                    ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM,
                    QATAL_MEM_CAST_PTR_TO_UINT64(&completion));
        }

        /* Checking Random Msg */
        if (CPA_STATUS_SUCCESS == status)
        {
            QATAL_DEBUG_LOG_OPTIONAL("QATAL - Get Random OK\n");
        }
        else
        {
            QATAL_ERROR_LOG("Sending Get Random Failed\n");
            return CPA_STATUS_FAIL;
        }
        QATAL_SLEEP(1000); /* Wait for Debug messages to output */

    }

    for (Random_Loop =0; Random_Loop < 96; Random_Loop ++)
    {
        if ((Sampled[Random_Loop] == Sampled[Random_Loop +1]) &&
            (Sampled[Random_Loop+1] == Sampled[Random_Loop+2]) &&
            (Sampled[Random_Loop+2] == Sampled[Random_Loop+3]) )
        {
            QATAL_ERROR_LOG("Error on Send/Wait/Receive Command Not Random\n");
            return CPA_STATUS_FAIL;
        }
    }

    for (Random_Loop =0; Random_Loop < 50000; Random_Loop ++)
    {

    ixOsalStdLog("Stage 2/2 Loop %d of 50000 \n",Random_Loop); 

        status = Qatal_RandMsgBuild(&randMsg,
                                ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM,
                 correlator);

        if(CPA_STATUS_SUCCESS == status)
        {
            /* Send message to the QAT */
            status = QatComms_ReqHdrCreate(&randMsg,
                                           ICP_ARCH_IF_REQ_QAT_FW_LA);

            if(CPA_STATUS_SUCCESS == status)
            {
                  status = QatComms_MsgSend(&randMsg,
                                      ICP_ARCH_IF_REQ_QAT_FW_LA,
                                      QAT_COMMS_PRIORITY_NORMAL,
                                      instanceHandle);

                  if (CPA_STATUS_SUCCESS != status)
                  {
                       status2 = QatComms_MsgCountGet(
                                     ICP_ARCH_IF_REQ_QAT_FW_LA,
                                     &GetSend, &GetRec, &GetRetry);
                  }
            }
            else
            {
                QATAL_ERROR_LOG("Error during heath Check: Create header \n");
            }
        }
        else
        {
            QATAL_ERROR_LOG("Error during heath Check: build msg \n");
        }

    }

    for (Random_Loop =0; Random_Loop < 5; Random_Loop ++)
    {
        status2 = QatComms_MsgCountGet(ICP_ARCH_IF_REQ_QAT_FW_LA,
                                       &GetSend, &GetRec,
                                       &GetRetry);
        QATAL_SLEEP(QATAL_ADMIN_TIMEOUT_IN_MS * 300);
    }

    status = QatalCfg_AdminMsgSendAndWait(
                        instanceHandle,
                        ICP_QAT_FW_ADMIN_CMD_GET,
                        QATAL_MEM_CAST_PTR_TO_UINT64(&completion));

    /* unset the QATAL LAC msg response callback handler */
    status = QatComms_ResponseCbSet(NULL,ICP_ARCH_IF_REQ_QAT_FW_LA);

    if (CPA_STATUS_SUCCESS != status)
    {
        QATAL_ERROR_LOG(
                        "Failed to initialize callback Unset LA \n");
        return CPA_STATUS_FAIL;
    }
    QATAL_STD_LOG("Successfully Health-check of QAT-AL component\n");

#endif //HEALTH_CHECK

    /* Check we have response for every command */
    if(QAT_COMMS_STATISTICS_ON == QatComms_getQatCollectStatistics())
    {
        for (Random_Loop =0;
                Random_Loop < ICP_ARCH_IF_REQ_DELIMITER;
                Random_Loop ++)
        {
            status = QatComms_MsgCountGet((icp_arch_if_request_t)Random_Loop,
                    &GetSend, &GetRec, &GetRetry);

            if (CPA_STATUS_SUCCESS != status)
            {
                QATAL_ERROR_LOG(
                        "Failed to initialize GetCount Correctly\n");
                return CPA_STATUS_FAIL;
            }
            if (GetSend != GetRec)
            {
                QATAL_ERROR_LOG("Unequal no. of Send/Rec Msgs\n");

                return CPA_STATUS_FAIL;
            }
        }

        /* do self-check regarding sending messages confirmed with the
         * change in internal statistics only when the statistics are not OFF 
         */
        status = Qatal_FWCountGet( &GetSend, &GetRec);

        if (CPA_STATUS_SUCCESS != status)
        {
            QATAL_ERROR_LOG(
                "Failed to initialize GetCount Correctly\n");
            return CPA_STATUS_FAIL;
        }
        if (0 == GetSend)
        {
            QATAL_ERROR_LOG("Unequal no. of Send/Rec Msgs\n");

            return CPA_STATUS_FAIL;
        }
        else if (0 == GetRec)
        {
            QATAL_ERROR_LOG(
                    "Failed to initialize FWGetCount Correctly\n");
            return CPA_STATUS_FAIL;
        }
    } /* endif QAT_COMMS_STATISTICS_ON */

    return status;
}


/*
 * @fn icp_AsdCfgQatalStop
 */
CpaStatus icp_AsdCfgQatalStop(CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    if((icp_accel_dev_t *) instanceHandle != pAccelHandle)
    {
        QATAL_STD_LOG("Warning: Invalid instanceHandle!!\n");
    }

    if (QATAL_COMP_STARTED != qatalState)
    {
        QATAL_ERROR_LOG("Component is not in correct state for stopping\n");
        status = CPA_STATUS_FAIL;
    }
    else
    {
        /* change state to be in the process of being stopped */
        qatalState = QATAL_COMP_STOPPING;
        QATAL_STD_LOG("Stopping QAT-AL ...\n");
    }


    /* unregister liveness */
    if(CPA_STATUS_SUCCESS == status)
    {
        status = icp_DccLivenessVerificationHandlerUnregister(&ThreadId) ;


    }
    if( status == CPA_STATUS_SUCCESS )
    {
        QATAL_DEBUG_LOG_OPTIONAL(
            "Callback Function AsdQatalLivenessGet UnRegistered\n");
    }
    else
    {
        QATAL_ERROR_LOG("Failed Unregister AsdQatalLivenessGet\n");
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        /* change state to be stopped */
        qatalState = QATAL_COMP_STOPPED;
        QATAL_STD_LOG("Successfully stopped QAT-AL component\n");
    }
    else
    {
        QATAL_ERROR_LOG("Failed to stop QAT-AL\n");
    }

    return status;
}

/*
 * @fn icp_AsdCfgQatalShutdown
 */
CpaStatus icp_AsdCfgQatalShutdown(CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    if((icp_accel_dev_t *) instanceHandle != pAccelHandle)
    {
        QATAL_STD_LOG("Warning: Invalid instanceHandle!!\n");
    }

    if ( !( (QATAL_COMP_STOPPED == qatalState) ||
             (QATAL_COMP_INITIALISED == qatalState)))
    {
        QATAL_ERROR_LOG("Component is not in correct state for shutdown\n");
        return CPA_STATUS_FAIL;
    }

    /* change state to be in the process of shutting down */
    qatalState = QATAL_COMP_SHUTTING_DOWN;
    QATAL_STD_LOG("Shutting down QAT-AL ...\n");

    /* shut-down the RAND component */
    if (CPA_STATUS_SUCCESS != Qatal_RandShutdown())
    {
        QATAL_ERROR_LOG(
            "failed to shut down random number generator component\n");
        status = CPA_STATUS_FAIL;
    }

    /* stop the QAT-AE */
    if (HALAE_SUCCESS != halAe_Stop(QATAL_QAT_AE_NUM,0xFFFFFFFF))
    {
        QATAL_ERROR_LOG("failed to stop QAT-AE\n");
        status = CPA_STATUS_FAIL;
    }
    else
    {
        QATAL_DEBUG_LOG_OPTIONAL("Successfully stopped QAT-AE\n");
    }

    /* destroy the QAT init/admin rings */
    if(CPA_STATUS_SUCCESS != QatComms_Shutdown())
    {
        QATAL_ERROR_LOG("Failed to shut down QAT comms component\n");
        status = CPA_STATUS_FAIL;
    }
    else
    {
        QATAL_STD_LOG("Successfully shut down QAT-Comms\n");
    }

    /* free allocated memory */
    if(CPA_STATUS_SUCCESS != QATAL_MEM_SHARED_FREE(&pRingTable))
    {
        QATAL_ERROR_LOG("Memory Allocation error - ring table\n");
        status = CPA_STATUS_RESOURCE;
    }

    /* free allocated memory */
    if(CPA_STATUS_SUCCESS != QATAL_MEM_SHARED_FREE(&pmmpLibCopy))
    {
        QATAL_ERROR_LOG("Memory Allocation error - mmplibCopy table\n");
        status = CPA_STATUS_RESOURCE;
    }

    /* change state to be shutdown */
    if(CPA_STATUS_SUCCESS == status)
    {
        qatalState = QATAL_COMP_SHUT_DOWN;
        QATAL_STD_LOG("Successfully shut down QAT-AL component\n");
    }
    else
    {
        QATAL_ERROR_LOG("Failed to shutdown QAT-AL component\n");
    }

    return status;
}

/*
 * @fn Qatal_FWCountGet
 */

CpaStatus Qatal_FWCountGet(Cpa32U *pNumResponsesSentByFW, 
                           Cpa32U *pNumRequestsReceivedByFW)
{
     CpaStatus status = CPA_STATUS_SUCCESS;
     CpaInstanceHandle instanceHandle = 0;

    QATAL_DEBUG_LOG_OPTIONAL("Qatal_FWCountGet \n");

    if (NULL == pNumResponsesSentByFW)
    {
        QATAL_ERROR_LOG("Qatal_FWCountGet Failed pNumSent = NULL \n");
        return CPA_STATUS_FAIL;
    }

    if (NULL == pNumRequestsReceivedByFW)
    {
        QATAL_ERROR_LOG("Qatal_FWCountGet Failed pNumReceived = NULL\n");
        return CPA_STATUS_FAIL;
    }

    /* Initialize Liveness data */
    g_qat_Liveness_Data = 0;
    g_qat_send_data = 0;
    g_qat_rec_data = 0;

    /* Send the Get Status */
    status = QatalCfg_AdminMsgSendAndWait(
                        instanceHandle,
                        ICP_QAT_FW_ADMIN_CMD_GET,
                        QATAL_MEM_CAST_PTR_TO_UINT64(&completion));

    /* Checking Ring_Info Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        /* increment No. of heartbeats */
        Qatal_StatsInc(QATAL_HEATBEAT_INVOKED);

        /* If all threads are ok */
        if (0xFF == g_qat_Liveness_Data)
        {
#ifdef ICP_DEBUG 
            QATAL_STD_LOG("Qatal_FWCountGet Rec Msgs. = %d\n",
                g_qat_rec_data);
            QATAL_STD_LOG("Qatal_FWCountGet Send Msgs. = %d\n",
                g_qat_send_data);
#endif //ICP_DEBUG
            *pNumResponsesSentByFW = g_qat_send_data;
            *pNumRequestsReceivedByFW = g_qat_rec_data;
            return CPA_STATUS_SUCCESS;
        }
        /* If a thread is dead */
        else
        {
            *pNumResponsesSentByFW = 0;
            *pNumRequestsReceivedByFW = 0;
            return CPA_STATUS_FAIL;
        }
    }
    else
    {
        QATAL_ERROR_LOG
            ("Qatal_FWCountGet Get no of command from firmware - Failed\n");
        *pNumResponsesSentByFW = 0;
        *pNumRequestsReceivedByFW = 0;
        return CPA_STATUS_FAIL;
    }

    return status;
}


/*
 * @fn icp_Qatal_LivenessGet
 */

STATIC CpaStatus
icp_Qatal_LivenessGet(icp_dcc_thread_id_t  *pThreadId,
                      icp_dcc_thread_status_t *pLiveness)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaInstanceHandle instanceHandle = 0;

    QATAL_DEBUG_LOG_OPTIONAL("Liveness Get \n");

    if (NULL == pThreadId)
    {
        QATAL_ERROR_LOG("Failed Liveness component -pThreadId \n");
        return CPA_STATUS_FAIL;
    }

    if (NULL == pLiveness)
    {
        QATAL_ERROR_LOG("Failed Liveness component -pThreadId\n");
        return CPA_STATUS_FAIL;
    }

    /* Result of liveness */
    *pLiveness = ICP_DCC_THREAD_ID_DEAD;

    /* Initialize Liveness data */
    g_qat_Liveness_Data = 0;
    g_qat_send_data = 0;
    g_qat_rec_data = 0;

    /* Send the Get Status */
    status = QatalCfg_AdminMsgSendAndWait(
                        instanceHandle,
                        ICP_QAT_FW_ADMIN_CMD_GET,
                        QATAL_MEM_CAST_PTR_TO_UINT64(&completion));

    /* Checking Ring_Info Msg */
    if (CPA_STATUS_SUCCESS == status)
    {
        /* increment No. of heartbeats */
        Qatal_StatsInc(QATAL_HEATBEAT_INVOKED);
        if (0xFF == g_qat_Liveness_Data)
        {
            *pLiveness = ICP_DCC_THREAD_ID_LIVE;
        }
        else
        {
            *pLiveness = ICP_DCC_THREAD_ID_DEAD;
        }
    }
    else
    {
        QATAL_ERROR_LOG("Sending GET Status msg for HEARTBEAT INFO Failed\n");
    }

    return status;
}

STATIC void
Qatal_InitCallbackFunc(Qat_CallbackTag correlator)
{
    CpaBoolean *pCompletion = (CpaBoolean *)(QATAL_PTR)correlator;

    if (NULL != pCompletion)
    {
        /* set the completion flag to true to unblock the
           function pending on this */
        *pCompletion = CPA_TRUE;
    }
    else
    {
/*        QATAL_ERROR_LOG("Error in callback for init/admin msg\n"); */
    }
}


STATIC void
Qatal_InitRespHandler(void *pRespMsg,
                                  icp_arch_if_request_t msgType)
{
    CpaBoolean respStatusOk = CPA_TRUE;
    Cpa64U correlator=0;
    icp_qat_fw_comn_flags cmnRespFlags = 0;
    Cpa8U qatalCmdId = 0;
    icp_qat_fw_init_resp_t *pInitRespMsg=NULL;

    msgType = msgType;

    if (NULL == pRespMsg)
    {
        QATAL_ERROR_LOG("Qatal_InitRespHandler.. Response message is NULL\n");
        return;
    }

    /* Cast pRespMsg to init struct */
    pInitRespMsg = (icp_qat_fw_init_resp_t *)pRespMsg;

    QATAL_MEM_SHARED_READ(pInitRespMsg->comn_resp.serv_cmd_id, qatalCmdId);

    if (qatalCmdId >= ICP_QAT_FW_INIT_CMD_DELIMITER)
    {
        QATAL_ERROR_LOG("Qatal_InitRespHandler.. Invalid init command ID\n");
        return;
    }
    else
    {
        QATAL_MEM_SHARED_READ(pInitRespMsg->comn_resp.comn_resp_flags,
                              cmnRespFlags);

        /* Check the status of the response message */
        if (ICP_QAT_FW_COMN_STATUS_FLAG_OK ==
            ICP_QAT_FW_COMN_STATUS_GET(cmnRespFlags) )
        {
            respStatusOk = CPA_TRUE;
        }
        else
        {
            QATAL_ERROR_LOG(
                "Qatal_InitRespHandler...Response status NOK\n");
            respStatusOk = CPA_FALSE;
        }


        /* Read the correlator back from the opaque data field */
        QATAL_MEM_SHARED_READ(pInitRespMsg->comn_resp.opaque_data,
                              correlator);

        /* call the admin callback function */
        Qatal_InitCallbackFunc((Qat_CallbackTag)(Cpa32U) correlator);
    }
}

STATIC void
Qatal_AdminRespHandler(void *pRespMsg,
                                  icp_arch_if_request_t msgType)
{
    CpaBoolean respStatusOk = CPA_TRUE;
    Cpa64U correlator=0;
    icp_qat_fw_comn_flags cmnRespFlags = 0;
    Cpa8U qatalCmdId = 0;
    icp_qat_fw_admin_resp_t *pAdminRespMsg=NULL;

    msgType = msgType;

    if (NULL == pRespMsg)
    {
        QATAL_ERROR_LOG("Qatal_AdminRespHandler..Response message is NULL\n");
        return;
    }

    /* Cast pRespMsg to init struct */
    pAdminRespMsg = (icp_qat_fw_admin_resp_t *)pRespMsg;

    QATAL_MEM_SHARED_READ(pAdminRespMsg->comn_resp.serv_cmd_id, qatalCmdId);

    if (qatalCmdId >= ICP_QAT_FW_ADMIN_CMD_DELIMITER)
    {
        QATAL_ERROR_LOG("Qatal_AdminRespHandler..Invalid ADMIN command ID\n");
        return;
    }
    else
    {
        QATAL_MEM_SHARED_READ(pAdminRespMsg->comn_resp.comn_resp_flags,
                              cmnRespFlags);

        /* Check the status of the response message */
        if (ICP_QAT_FW_COMN_STATUS_FLAG_OK ==
            ICP_QAT_FW_COMN_STATUS_GET(cmnRespFlags) )
        {

            QATAL_MEM_SHARED_READ(pAdminRespMsg->version_major_num,
                Version_Data.majorVersion);
            QATAL_MEM_SHARED_READ(pAdminRespMsg->version_minor_num,
                Version_Data.minorVersion);
            QATAL_MEM_SHARED_READ(pAdminRespMsg->version_patch_num, 
                Version_Data.patchVersion);
            QATAL_MEM_SHARED_READ(pAdminRespMsg->request_recvd_cnt, 
                g_qat_rec_data);
            QATAL_MEM_SHARED_READ(pAdminRespMsg->response_sent_cnt, 
                g_qat_send_data);
#ifdef ICP_DEBUG 
            QATAL_STD_LOG("Major Version = %d\n",
                Version_Data.majorVersion);
            QATAL_STD_LOG("Minor Version = %d\n",
                Version_Data.minorVersion);
            QATAL_STD_LOG("Patch Version = %d\n",
                Version_Data.patchVersion);
            QATAL_STD_LOG("Rec Msgs. = %d\n",
                g_qat_rec_data);
            QATAL_STD_LOG("Send Msgs. = %d\n",
                g_qat_send_data);
#endif //ICP_DEBUG
            QATAL_MEM_SHARED_READ(pAdminRespMsg->liveness_info, 
                g_qat_Liveness_Data);
            g_qat_Liveness_Data =  pAdminRespMsg->liveness_info;

#ifdef ICP_DEBUG 
            QATAL_STD_LOG("Liveness = 0x%x\n", g_qat_Liveness_Data);
#endif //ICP_DEBUG
            respStatusOk = CPA_TRUE;
        }
        else
        {
            respStatusOk = CPA_FALSE;
        }

        if (CPA_TRUE != respStatusOk)
        {
            QATAL_ERROR_LOG("Qatal_AdminRespHandler ...Response status NOK\n");
        }
        else
        {
            QATAL_DEBUG_LOG_OPTIONAL("Qatal_AdminRespHandler..OK\n");
        }

        /* Read the correlator back from the opaque data field */
        QATAL_MEM_SHARED_READ(pAdminRespMsg->comn_resp.opaque_data,
                              correlator);

        ADMIN_READ = 0;

        /* call the admin callback function */
        Qatal_InitCallbackFunc((Qat_CallbackTag)(Cpa32U) correlator);
    }
}
#ifdef HEALTH_CHECK

STATIC void
Qatal_RandRespHandler(void *pRespMsg,
                                  icp_arch_if_request_t msgType)
{
    CpaBoolean respStatusOk = CPA_TRUE;
    Cpa64U correlator=0;
    icp_qat_fw_comn_flags cmnRespFlags = 0;
    Cpa8U qatalCmdId = 0;
    Cpa32U RandNo = 0;
    icp_qat_fw_la_resp_t *pRandRespMsg=NULL;

    msgType = msgType;

    if (NULL == pRespMsg)
    {
        QATAL_ERROR_LOG("Qatal_RandRespHandler..Response message is NULL\n");
        return;
    }

    /* Cast pRespMsg to LA struct */
    pRandRespMsg = (icp_qat_fw_la_resp_t *)pRespMsg;

    QATAL_MEM_SHARED_READ(pRandRespMsg->comn_resp.serv_cmd_id, qatalCmdId);

    /* This is for Get Random Only */
    QATAL_MEM_SHARED_READ(pRandRespMsg->comn_resp.comn_resp_flags,
                              cmnRespFlags);

    /* Check the status of the response message */
    if (ICP_QAT_FW_COMN_STATUS_FLAG_OK ==
            ICP_QAT_FW_COMN_STATUS_GET(cmnRespFlags) )
    {
        RandNo = *pRandSample ;
        Sampled[Qat_Random_Index] = RandNo;
        Qat_Random_Index++;
        if (Qat_Random_Index == 100)
        {
            Qat_Random_Index = 0;
        }
        respStatusOk = CPA_TRUE;
    }
    else
    {
        respStatusOk = CPA_FALSE;
    }

    if (CPA_TRUE != respStatusOk)
    {
        QATAL_ERROR_LOG("Qatal_RandRespHandler ...Response status NOK \n");
    }

    /* Read the correlator back from the opaque data field */
    QATAL_MEM_SHARED_READ(pRandRespMsg->comn_resp.opaque_data,
                              correlator);

    /* call the admin callback function */
    Qatal_InitCallbackFunc((Qat_CallbackTag)(Cpa32U) correlator);
}

#endif //HEALTH_CHECK

STATIC CpaStatus
QatalCfg_InitMsgSendAndWait(CpaInstanceHandle instanceHandle,
                            icp_qat_fw_init_cmd_id_t initCmdId,
                            Cpa64U correlator )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_init_req_t initMsg;
    Cpa32U count = 0;

    switch(initCmdId)
    {
        case ICP_QAT_FW_INIT_CMD_CONFIG_NRBG:
        {
            /* Build the ICP_QAT_FW_INIT_CMD_CONFIG_NRBG request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_INIT_CMD_CONFIG_NRBG ...");

            status = Qatal_InitMsgBuild(&initMsg,
                                        ICP_QAT_FW_INIT_CMD_CONFIG_NRBG,
                                        correlator);
        }
        break;

        case ICP_QAT_FW_INIT_CMD_ENABLE_DRBG:
        {
            /* Build the ICP_QAT_FW_INIT_CMD_ENABLE_DRBG request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_INIT_CMD_ENABLE_DRBG ...");

            status = Qatal_InitMsgBuild(&initMsg,
                                        ICP_QAT_FW_INIT_CMD_ENABLE_DRBG,
                                        correlator);
        }
        break;

        case ICP_QAT_FW_INIT_CMD_SET_CONSTS:
        {
            /* Build the ICP_QAT_FW_INIT_CMD_SET_CONSTS request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_INIT_CMD_SET_CONSTS ...");

            status = Qatal_InitMsgBuild(&initMsg,
                                        ICP_QAT_FW_INIT_CMD_SET_CONSTS,
                                        correlator);
        }
        break;

        case ICP_QAT_FW_INIT_CMD_SET_RING_INFO:
        {
            /* Build the SET_RING_INFO request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_INIT_CMD_SET_RING_INFO\n");

            status = Qatal_InitMsgBuild(&initMsg,
                                        ICP_QAT_FW_INIT_CMD_SET_RING_INFO,
                                        correlator);
        }
        break;

        case ICP_QAT_FW_INIT_CMD_INIT_MMP:
        {
            /* Build the INIT_MMP request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_INIT_CMD_INIT_MMP ...\n");

            status = Qatal_InitMsgBuild(&initMsg,
                                        ICP_QAT_FW_INIT_CMD_INIT_MMP,
                                        correlator);
        }
        break;

        case ICP_QAT_FW_INIT_CMD_INIT_FINAL:
        {
            /* Build the INIT_FINAL request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_INIT_FINAL ...\n");

            status = Qatal_InitMsgBuild(&initMsg,
                                        ICP_QAT_FW_INIT_CMD_INIT_FINAL,
                                        correlator);
        }
        break;

        default:
        {
            /* Unsupported message message */
            QATAL_ERROR_LOG("Invalid QAT init message ID\n");
            status = CPA_STATUS_INVALID_PARAM;
        }
        break;
    }

    if(CPA_STATUS_SUCCESS == status)
    {

        /* Send message to the QAT */
        status = QatComms_ReqHdrCreate(&initMsg,
                                       ICP_ARCH_IF_REQ_QAT_FW_INIT);

        if(CPA_STATUS_SUCCESS == status)
        {
            status = QatComms_MsgSend(&initMsg,
                                      ICP_ARCH_IF_REQ_QAT_FW_INIT,
                                      QAT_COMMS_PRIORITY_NORMAL,
                                      instanceHandle);
        }
    }
    if(CPA_STATUS_SUCCESS != status)
    {
        QATAL_ERROR_LOG("Failed to send QAT message");
        status = CPA_STATUS_FAIL;
    }
    else
    {
        QATAL_DEBUG_LOG_OPTIONAL("Sent QAT message");
    }


    /* pend until the completion flag is set by the callback function*/
    if (CPA_STATUS_SUCCESS == status)
    {
        while ((CPA_FALSE == completion) &&
           (count < QATAL_ADMIN_WAIT_COUNT))

        {
            QATAL_SLEEP(QATAL_ADMIN_TIMEOUT_IN_MS);
            count++;
        }

        if (CPA_FALSE == completion)
        {
            QATAL_ERROR_LOG("Timeout for msg callback has expired");

            status = CPA_STATUS_FAIL;
        }
        completion = CPA_FALSE ;
    }


    return status;
}

STATIC CpaStatus
QatalCfg_AdminMsgSendAndWait(CpaInstanceHandle instanceHandle,
                             icp_qat_fw_admin_cmd_id_t adminCmdId,
                             Cpa64U correlator )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_admin_req_t adminMsg;
    Cpa32U count = 0;

    /* Check if stucture of Admin Msg has changed by checking offsets */
    if ((abs((uint32_t)&adminMsg.resrvd1 - (uint32_t)&adminMsg.admin_cmd_id)
                        == sizeof(uint8_t))   &&
       ( abs((uint32_t)&adminMsg.status_tbl_sz - (uint32_t)&adminMsg.resrvd1)
                        == sizeof(uint8_t) )  &&
       ( abs((uint32_t)&adminMsg.resrvd2 - (uint32_t)&adminMsg.status_tbl_sz)
                        == sizeof(uint16_t)) &&
       ( abs((uint32_t)&adminMsg.status_tbl_addr -(uint32_t)&adminMsg.resrvd2)
                        == sizeof(uint32_t)) &&
       ( abs((uint32_t)&adminMsg.resrvd3 -(uint32_t)&adminMsg.status_tbl_addr)
                        == sizeof(uint64_t)) &&
       ( abs((uint32_t)&adminMsg.resrvd4 - (uint32_t)&adminMsg.resrvd3)
                        == sizeof(uint32_t)) )
    {
       QATAL_DEBUG_LOG_OPTIONAL("QATAL - Message Structure OK");
    }
    else
     {
        QATAL_ERROR_LOG("INVALID STRUCTURE - Msg structure order changed");
        status = ICP_STATUS_FAIL;
    }

    if (ICP_STATUS_SUCCESS == status)
    {
      switch(adminCmdId)
        {
            case ICP_QAT_FW_ADMIN_CMD_GET:
            {

                ADMIN_READ  = 1;
                /* Admin command may include data to be read in */


                /* Build the ICP_QAT_FW_ADMIN_CMD_GET request message */
                QATAL_DEBUG_LOG_OPTIONAL(
                    "Sending QAT message - ICP_QAT_FW_ADMIN_CMD_GET ...");

                status = Qatal_AdminMsgBuild(&adminMsg,
                                        ICP_QAT_FW_ADMIN_CMD_GET,
                                        correlator);
            }
            break;
            default:
            {
                /* Unsupported message message */
                QATAL_ERROR_LOG("Invalid QAT ADMIN message ID\n");
                status = CPA_STATUS_INVALID_PARAM;
            }
            break;
        }
    }


    if(CPA_STATUS_SUCCESS == status)
    {

        /* Send message to the QAT */
        status = QatComms_ReqHdrCreate(&adminMsg,
                                       ICP_ARCH_IF_REQ_QAT_FW_ADMIN);

        if(CPA_STATUS_SUCCESS == status)
        {
            status = QatComms_MsgSend(&adminMsg,
                                      ICP_ARCH_IF_REQ_QAT_FW_ADMIN,
                                      QAT_COMMS_PRIORITY_NORMAL,
                                      instanceHandle);
        }
    }
    if(CPA_STATUS_SUCCESS != status)
    {
        QATAL_ERROR_LOG("Failed to send QAT ADMIN message");

    status = CPA_STATUS_FAIL;
    }
    else
    {
       QATAL_DEBUG_LOG_OPTIONAL("Sent QAT ADMIN message");
    }


    /* pend until the completion flag is set by the callback function */
    if (CPA_STATUS_SUCCESS == status)
    {
        while ((CPA_FALSE == completion) &&
               (count < QATAL_ADMIN_WAIT_COUNT ))
        {
            QATAL_SLEEP(QATAL_ADMIN_TIMEOUT_IN_MS);
            count++;
        }

        if (CPA_FALSE == completion)
        {
            QATAL_ERROR_LOG("Timeout for msg callback has expired");
            status = CPA_STATUS_FAIL;
        }
        count = 0;
        while( (1 == ADMIN_READ)&&
               (count < (QATAL_ADMIN_WAIT_COUNT * 5) ))
        {
            QATAL_SLEEP(QATAL_ADMIN_TIMEOUT_IN_MS * 3);
            count++;

        }
    }
    completion = CPA_FALSE ;

    return status;
}

#ifdef HEALTH_CHECK

STATIC CpaStatus
QatalCfg_RandMsgSendAndWait(CpaInstanceHandle instanceHandle,
                            icp_qat_fw_la_cmd_id_t randCmdId,
                            Cpa64U correlator )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_la_rng_req_t randMsg;
    Cpa32U count = 0;

    switch(randCmdId)
    {
        case ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM:
        {
            /* Build the ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM request message */
            QATAL_DEBUG_LOG_OPTIONAL(
                "Sending QAT message - ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM ...");

            status = Qatal_RandMsgBuild(&randMsg,
                                        ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM,
                                        correlator);
        }
        break;

        default:
        {
            /* Unsupported message message */
            QATAL_ERROR_LOG("Invalid QAT rand message ID\n");
            status = CPA_STATUS_INVALID_PARAM;
        }
        break;
    }

    if(CPA_STATUS_SUCCESS == status)
    {

        /* Send message to the QAT */
        status = QatComms_ReqHdrCreate(&randMsg,
                                       ICP_ARCH_IF_REQ_QAT_FW_LA);

        if(CPA_STATUS_SUCCESS == status)
        {
            status = QatComms_MsgSend(&randMsg,
                                      ICP_ARCH_IF_REQ_QAT_FW_LA,
                                      QAT_COMMS_PRIORITY_NORMAL,
                                      instanceHandle);
        }
    }
    if(CPA_STATUS_SUCCESS != status)
    {
        QATAL_ERROR_LOG("Failed to send QAT message");
        status = CPA_STATUS_FAIL;
    }
    else
    {
        QATAL_DEBUG_LOG_OPTIONAL("Sent QAT message");
    }


    /* pend until the completion flag is set by the callback function*/
    if (CPA_STATUS_SUCCESS == status)
    {
        while ((CPA_FALSE == completion) &&
               (count < (QATAL_ADMIN_WAIT_COUNT * 10)))
        {
            QATAL_SLEEP((QATAL_ADMIN_TIMEOUT_IN_MS * 10));
            count++;
        }

        if (CPA_FALSE == completion)
        {
            QATAL_ERROR_LOG("Timeout for msg callback has expired");
            status = CPA_STATUS_FAIL;
        }
        completion = CPA_FALSE ;
    }


    return status;
}

#endif //HEALTH_CHECK




STATIC CpaStatus Qatal_InitMsgBuild(icp_qat_fw_init_req_t *pInitReq,
                                       icp_qat_fw_init_cmd_id_t initCmdId,
                                       Cpa64U correlator)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U i=0;
    Cpa16U table_sz = 0;
    Cpa16U data_sz = 0;
    Cpa16U in_data_sz = 0;
    Cpa32U bulk_rng_msk = 0;
    Cpa32U pke_rng_msk = 0;

    QATAL_CHECK_NULL_PARAM(pInitReq);

    /* Clear the message contents */
    QATAL_MEM_SET(pInitReq, 0, sizeof(icp_qat_fw_init_req_t));

    if(CPA_STATUS_SUCCESS !=
        Qatal_ComnReqHdrPopulate(&(pInitReq->comn_req), correlator))
    {
        return CPA_STATUS_FAIL;
    }

    /* set the Init command ID */
    QATAL_MEM_SHARED_WRITE(pInitReq->init_cmd_id, initCmdId);

    if(initCmdId == ICP_QAT_FW_INIT_CMD_SET_RING_INFO)
    {
        /* Build the ring table */
        for(i=0;i<INIT_RING_TABLE_SZ;i++)
        {
            /*Set reserved field to zero*/
            QATAL_MEM_SHARED_WRITE(pRingTable->bulk_rings[i].reserved,0);

            switch(i)
            {
                case ICP_RING_QATAL_ADMIN_REQUEST:
                {
                    pRingTable->bulk_rings[i].curr_weight =
                        QATAL_ADMIN_RING_WEIGHTING;
                    pRingTable->bulk_rings[i].init_weight =
                        QATAL_ADMIN_RING_WEIGHTING;
                }
                break;

                case ICP_RING_LAC_LA_HI_REQUEST:
                {
                    pRingTable->bulk_rings[i].curr_weight =
                        QATAL_LAC_HIGH_PRIORITY_RING_WEIGHTING;
                    pRingTable->bulk_rings[i].init_weight =
                        QATAL_LAC_HIGH_PRIORITY_RING_WEIGHTING;
                }
                break;

                case ICP_RING_LAC_LA_LO_REQUEST:
                {
                    pRingTable->bulk_rings[i].curr_weight =
                        QATAL_LAC_LOW_PRIORITY_RING_WEIGHTING;
                    pRingTable->bulk_rings[i].init_weight =
                        QATAL_LAC_LOW_PRIORITY_RING_WEIGHTING;
                }
                break;

                case ICP_RING_IPSEC_REQUEST:
                {
                    pRingTable->bulk_rings[i].curr_weight =
                        QATAL_IPSEC_RING_WEIGHTING;
                    pRingTable->bulk_rings[i].init_weight =
                        QATAL_IPSEC_RING_WEIGHTING;
                }
                break;

                case ICP_RING_LAC_PKE_REQUEST:
                {
                    pRingTable->bulk_rings[i].curr_weight =
                        QATAL_PKE_RING_WEIGHTING;
                    pRingTable->bulk_rings[i].init_weight =
                        QATAL_PKE_RING_WEIGHTING;
                }
                break;

                default:
                {
                    pRingTable->bulk_rings[i].curr_weight =
                        QATAL_DEFAULT_RING_WEIGHTING ;
                    pRingTable->bulk_rings[i].init_weight =
                        QATAL_DEFAULT_RING_WEIGHTING ;
                }
                break;
            }
        }

        /* set the ring table address */
        QATAL_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
            pInitReq->data_addr, pRingTable);

        /* set the ring table size in bytes */
        table_sz = sizeof(icp_qat_fw_init_ring_table_t);
        QATAL_MEM_SHARED_WRITE(pInitReq->data_sz, table_sz);

        bulk_rng_msk = QATAL_BULK_RINGS_MASK;
        /* set the bulk ring mask */
        QATAL_MEM_SHARED_WRITE(
            pInitReq->u1.bulk_rings_mask, bulk_rng_msk);

        pke_rng_msk = QATAL_PKE_RINGS_MASK;
        /* set the pke ring mask */
        QATAL_MEM_SHARED_WRITE(
            pInitReq->u2.pke_rings_mask, pke_rng_msk);
    }
    else if(initCmdId == ICP_QAT_FW_INIT_CMD_INIT_MMP)
    {
        /* set the MMP lib base address */
        QATAL_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
            pInitReq->data_addr,
            (void *)(QATAL_PTR)pmmpLibCopy);

    }
    else if(initCmdId == ICP_QAT_FW_INIT_CMD_INIT_FINAL)
    {
        /* no other fields to add */
    }
    else if(initCmdId == ICP_QAT_FW_INIT_CMD_CONFIG_NRBG)
    {
        /* Save table size as defined in HLD (8B) not actual table size */

        in_data_sz = 0x8B ;
        QATAL_MEM_SHARED_WRITE(data_sz, in_data_sz );
        pInitReq->data_sz = data_sz;

        /* save the ENTRPY block size */
        QATAL_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
            pInitReq->data_addr,(void *)pQATAL_NGB_BLOCK);
    }
    else if(initCmdId == ICP_QAT_FW_INIT_CMD_ENABLE_DRBG)
    {

        /* No extra data to send */
    }
    else if(initCmdId == ICP_QAT_FW_INIT_CMD_SET_CONSTS)
    {
        /* no other fields to add */
    }
    else
    {
        /* Unsupported message */
        QATAL_ERROR_LOG("Invalid QAT init message ID\n");
        status = CPA_STATUS_INVALID_PARAM;
    }

    return status;
}

STATIC CpaStatus Qatal_AdminMsgBuild(icp_qat_fw_admin_req_t *pAdminReq,
                                       icp_qat_fw_admin_cmd_id_t adminCmdId,
                                       Cpa64U correlator)
{

    QATAL_CHECK_NULL_PARAM(pAdminReq);

    /* Clear the message contents */
    QATAL_MEM_SET(pAdminReq, 0, sizeof(icp_qat_fw_admin_req_t));

    if(CPA_STATUS_SUCCESS !=
        Qatal_ComnReqHdrPopulate(&(pAdminReq->comn_req), correlator))
    {
        return CPA_STATUS_FAIL;
    }

    /* set the Init command ID */
    QATAL_MEM_SHARED_WRITE(pAdminReq->admin_cmd_id, adminCmdId);


    return CPA_STATUS_SUCCESS;
}

#ifdef HEALTH_CHECK

STATIC CpaStatus Qatal_RandMsgBuild(icp_qat_fw_la_rng_req_t *pRandReq,
                                       icp_qat_fw_la_cmd_id_t randCmdId,
                                       Cpa64U correlator)
{

    QATAL_CHECK_NULL_PARAM(pRandReq);

    /* Clear the message contents */
    QATAL_MEM_SET(pRandReq, 0, sizeof(icp_qat_fw_la_rng_req_t));

    if(CPA_STATUS_SUCCESS !=
        Qatal_RandReqHdrPopulate(&(pRandReq->comn_req),
        correlator))
    {
        return CPA_STATUS_FAIL;
    }

    /* set the Rand command ID */
    QATAL_MEM_SHARED_WRITE(pRandReq->comn_la_req.la_cmd_id, randCmdId);

    /* Size of Requested Data */
    QATAL_MEM_SHARED_WRITE(pRandReq->length, 4);

    /* Gather state type */
    QATAL_MEM_SHARED_WRITE(pRandReq->gather_state,(Cpa8U)Gather_state);

    /* Reset Gather State */
    Gather_state = ICP_QAT_FW_LA_DRBG_GATHER_STATE_IGNORE;

    return CPA_STATUS_SUCCESS;
}
#endif //HEALTH_CHECK


CpaStatus Qatal_ComnReqHdrPopulate(
    icp_qat_fw_comn_req_hdr_t *pComnReq,
    Cpa64U correlator)
{
    icp_qat_fw_comn_flags cmnReqFlags = 0;

    QATAL_CHECK_NULL_PARAM(pComnReq);

    /* QAT comms will write the Common fields of the message */

    /* === set the common request flags  ==== */

    ICP_QAT_FW_COMN_ORD_SET(cmnReqFlags,
                            ICP_QAT_FW_COMN_ORD_FLAG_STRICT);

    /* Other flags are unused for admin msgs */

    /* write the the common request flags into the message */
    QATAL_MEM_SHARED_WRITE(pComnReq->comn_req_flags, cmnReqFlags);

    /* write the the response correlator into the
       opaque data field of the message */
    QATAL_MEM_SHARED_WRITE(pComnReq->opaque_data, correlator);

    /* Other fields are unused for admin msgs */
    return CPA_STATUS_SUCCESS;
}


#ifdef HEALTH_CHECK

CpaStatus Qatal_RandReqHdrPopulate(
    icp_qat_fw_comn_req_hdr_t *pComnReq,
    Cpa64U correlator)
{
    icp_qat_fw_comn_flags cmnReqFlags = 0;

    QATAL_CHECK_NULL_PARAM(pComnReq);

    /* QAT comms will write the Common fields of the message */

    /* === set the common request flags  ==== */

    ICP_QAT_FW_COMN_ORD_SET(cmnReqFlags,
                            ICP_QAT_FW_COMN_ORD_FLAG_STRICT);

    /* Other flags are unused for admin msgs */

    /* write the the common request flags into the message */
    QATAL_MEM_SHARED_WRITE(pComnReq->comn_req_flags, cmnReqFlags);

    /* write the the response correlator into the
       opaque data field of the message */
    QATAL_MEM_SHARED_WRITE(pComnReq->opaque_data, correlator);

    /* set the address for Random Number*/
    QATAL_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
            pComnReq->dest_data_addr,
            (void *)pRandSample);

    /* Other fields are unused for Rand msgs */
    return CPA_STATUS_SUCCESS;
}
#endif //HEALTH_CHECK



/*
 * @fn Qatal_GetRANDDataBlockPtr
 */
CpaStatus
Qatal_GetRANDDataBlockPtr(Cpa8U **ppRANDDataBlock)
{

    if (NULL == ppRANDDataBlock)
    {
        return  CPA_STATUS_FAIL;
    }

    *ppRANDDataBlock = pQATAL_NGB_BLOCK;

    if (pQATAL_NGB_BLOCK == NULL)
    {
        return  CPA_STATUS_FAIL;
    }
    else
    {
        return CPA_STATUS_SUCCESS;
    }
}

/*
 * @fn Qatal_PutRANDDataBlockPtr
 */
CpaStatus
Qatal_PutRANDDataBlockPtr(Cpa8U **ppRANDDataBlock)
{
    if (ppRANDDataBlock == NULL)
    {
        return  CPA_STATUS_FAIL;
    }

    pQATAL_NGB_BLOCK = *ppRANDDataBlock;

    if (pQATAL_NGB_BLOCK == NULL)
    {
        return  CPA_STATUS_FAIL;
    }
    else
    {
        return CPA_STATUS_SUCCESS;
    }
}
