/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_device_g997.h"

#ifdef __cplusplus
   extern "C" {
#endif

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_G997

#if defined(INCLUDE_DSL_CEOC)
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_SNMP_MESSAGE_SEND
*/
DSL_Error_t DSL_DRV_G997_SnmpMessageSend(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_G997_Snmp_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_SnmpMessageSend"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Send SNMP message*/
   nErrCode = DSL_CEOC_MessageSend(
                 pContext,
                 DSL_CEOC_SNMP_PROTOCOL_ID,
                 (DSL_CEOC_Message_t*)&(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_SnmpMessageSend, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_SNMP_MESSAGE_RECEIVE
*/
DSL_Error_t DSL_DRV_G997_SnmpMessageReceive(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN  DSL_Context_t *pContext,
   DSL_OUT DSL_G997_Snmp_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_SnmpMessageReceive"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Read SNMP message from the CEOC internal FIFO*/
   nErrCode = DSL_CEOC_FifoMessageRead(
                 pOpenCtx,
                 pContext,
                 DSL_CEOC_SNMP_PROTOCOL_ID,
                 (DSL_CEOC_Message_t*)&(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_SnmpMessageReceive, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_ACTIVATE_CONFIG_SET
*/
DSL_Error_t DSL_DRV_G997_LineActivateConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineActivate_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: IN - DSL_DRV_G997_LineActivateConfigSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Check for the nLDSF parameter*/
   if ( (pData->data.nLDSF < DSL_G997_INHIBIT_LDSF) ||
        (pData->data.nLDSF >= DSL_G997_LDSF_LAST))
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext,"DSL[%02d]: ERROR - Invalid nLDSF=%d specified!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pData->data.nLDSF));
      nErrCode = DSL_ERR_PARAM_RANGE;
   }

   /* Check for the nACSF parameter*/
   if ((pData->data.nACSF < DSL_G997_INHIBIT_ACSF) ||
       (pData->data.nACSF >= DSL_G997_ACSF_LAST) )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext,"DSL[%02d]: ERROR - Invalid nACSF=%d specified!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pData->data.nACSF));
      nErrCode = DSL_ERR_PARAM_RANGE;
   }

   /* Check for the nStartupMode parameter*/
   if ((pData->data.nStartupMode < DSL_G997_NORMAL_STARTUP) ||
       (pData->data.nStartupMode >= DSL_G997_STARTUP_LAST))
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext,"DSL[%02d]: ERROR - Invalid nStartupMode=%d specified!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pData->data.nStartupMode));
      nErrCode = DSL_ERR_PARAM_RANGE;
   }

#ifndef INCLUDE_DSL_DELT
   if (pData->data.nLDSF == DSL_G997_AUTO_LDSF ||
       pData->data.nLDSF == DSL_G997_FORCE_LDSF)
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - DELT not included in the current build,"
                    " nLDSF parameter ignored!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   
   }
#endif /* INCLUDE_DSL_DELT*/

   if (nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_WRITE(pContext, nErrCode, lineActivateConfig, pData->data);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineActivateConfigSet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_ACTIVATE_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_G997_LineActivateConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineActivate_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineActivateConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ(pContext, nErrCode, lineActivateConfig, pData->data);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineActivateConfigGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET
*/
DSL_Error_t DSL_DRV_G997_XTUSystemEnablingConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_XTUSystemEnabling_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS, nRet = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_XTUSystemEnablingConfigSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Check XTSE configuration data*/
   nRet =  DSL_DRV_XtseSettingsCheck(pContext, pData->data.XTSE);

   if (nRet >= 0)
   {
      DSL_CTX_WRITE(pContext, nErrCode, xtseCfg, pData->data.XTSE);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_XTUSystemEnablingConfigSet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nRet));

   return nRet;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_G997_XTUSystemEnablingConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_XTUSystemEnabling_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_XTUSystemEnablingConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ(pContext, nErrCode, xtseCfg, pData->data.XTSE);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_G997_XTUSystemEnablingConfigGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_XTUSystemEnablingStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_XTUSystemEnabling_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_XTUSystemEnablingStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

#ifdef INCLUDE_DSL_CPE_API_VINAX
   if ( (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
        (nCurrentState != DSL_LINESTATE_SHOWTIME_NO_SYNC) &&
        (nCurrentState != DSL_LINESTATE_LOOPDIAGNOSTIC_COMPLETE) &&
        (nCurrentState != DSL_LINESTATE_FULL_INIT))
#else
   if ( (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
        (nCurrentState != DSL_LINESTATE_SHOWTIME_NO_SYNC) &&
        (nCurrentState != DSL_LINESTATE_LOOPDIAGNOSTIC_COMPLETE) &&
        (nCurrentState != DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE))
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - no data in the current line state!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERR_DEVICE_NO_DATA;
   }
   else
   {
      DSL_CTX_READ(pContext, nErrCode, xtseCurr, pData->data.XTSE);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_XTUSystemEnablingStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_G997_ALARM
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_SET
*/
DSL_Error_t DSL_DRV_G997_ChannelDataRateThresholdConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelDataRateThreshold_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_G997_ChannelDataRateThresholdConfigSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_WRITE(pContext, nErrCode, channelDataRateThreshold[pData->nDirection], pData->data);

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_G997_ChannelDataRateThresholdConfigSet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_G997_ChannelDataRateThresholdConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelDataRateThreshold_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_G997_ChannelDataRateThresholdConfigGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_READ(pContext, nErrCode, channelDataRateThreshold[pData->nDirection], pData->data);

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_G997_ChannelDataRateThresholdConfigGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_TRANSMISSION_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_LineTransmissionStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineTransmissionStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineTransmissionStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_LineTransmissionStatusGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineTransmissionStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_INIT_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_LineInitStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInitStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineInitStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ(pContext, nErrCode, lineInitStatus, pData->data);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineInitStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_LineStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_LineStatusGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_STATUS_PER_BAND_GET
*/
DSL_Error_t DSL_DRV_G997_LineStatusPerBandGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatusPerBand_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineStatusPerBandGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_LineStatusPerBandGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineStatusPerBandGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_RATE_ADAPTATION_CONFIG_SET
*/
DSL_Error_t DSL_DRV_G997_RateAdaptationConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_RateAdaptationConfig_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_RateAdaptationConfigSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   if ((pData->data.RA_MODE != DSL_G997_RA_MODE_AT_INIT) &&
       (pData->data.RA_MODE != DSL_G997_RA_MODE_DYNAMIC) &&
       (pData->data.RA_MODE != DSL_G997_RA_MODE_DYNAMIC_SOS))
   {
      nErrCode = DSL_ERR_INVALID_PARAMETER;
   }
   else
   {
#ifdef INCLUDE_DSL_CPE_API_DANUBE
      if (pData->data.RA_MODE == DSL_G997_RA_MODE_DYNAMIC_SOS)
      {
         return DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
      }
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/
      /* Update internal Rate Adaptation Mode Settings for the selected direction*/
      DSL_CTX_WRITE(pContext, nErrCode,
         rateAdaptationMode[pData->nDirection], pData->data.RA_MODE);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_RateAdaptationConfigSet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_RATE_ADAPTATION_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_G997_RateAdaptationConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_RateAdaptationConfig_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_RA_MODE_t nSRAmode;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_RateAdaptationConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Get internal Rate Adaptation Mode Settings for the selected direction*/
   DSL_CTX_READ(pContext, nErrCode,
      rateAdaptationMode[pData->nDirection], nSRAmode);

   pData->data.RA_MODE = (nSRAmode == DSL_G997_RA_MODE_AT_INIT) ||
                         (nSRAmode == DSL_G997_RA_MODE_DYNAMIC_SOS) ?
                            nSRAmode : DSL_G997_RA_MODE_DYNAMIC;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_RateAdaptationConfigGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_CHANNEL_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_ChannelStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_ChannelStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_CHANNEL_RANGE(pData->nChannel);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_ChannelStatusGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_ChannelStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATE_FORCED_TRIGGER
*/
DSL_Error_t DSL_DRV_G997_PowerManagementStateForcedTrigger(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_PowerManagementStateForcedTrigger_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_PowerManagementStateForcedTrigger"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_PowerManagementStateForcedTrigger(
                 pContext, &(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_PowerManagementStateForcedTrigger, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_PowerManagementStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_PowerManagementStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_PowerManagementStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get Actual Power Management Status from the DSL CPE context*/
   DSL_CTX_READ(pContext, nErrCode, powerMgmtStatus, pData->data);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_PowerManagementStatusGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LAST_STATE_TRANSMITTED_GET
*/
DSL_Error_t DSL_DRV_G997_LastStateTransmittedGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LastStateTransmitted_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LastStateTransmittedGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_LastStateTransmittedGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LastStateTransmittedGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_PER_TONE
/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_G997_BitAllocationNSCGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_BitAllocationNsc_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_BitAllocationNSCGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_BitAllocationNSCGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_BitAllocationNSCGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET
*/
DSL_Error_t DSL_DRV_G997_GainAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_GainAllocationNsc_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_GainAllocationNscGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_GainAllocationNscGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_GainAllocationNscGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_SNR_ALLOCATION_NSC_GET
*/
DSL_Error_t DSL_DRV_G997_SnrAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_SnrAllocationNsc_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_SnrAllocationNscGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_SnrAllocationNscGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_SnrAllocationNscGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_G997_ALARM
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_SET
*/
DSL_Error_t DSL_DRV_G997_AlarmMaskLineFailuresConfigSet(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineFailures_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_AlarmMaskLineFailuresConfigSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( pData->nDirection == DSL_NEAR_END )
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nLineFailuresNeAlarmMask,
         pData->data.nLineFailures);
   }
   else
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nLineFailuresFeAlarmMask,
         pData->data.nLineFailures);
   }

   if(DSL_DRV_MUTEX_LOCK(pOpenCtx->eventMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: Couldn't lock event mutex"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }

   /* Manage DSL_EVENT_I_LINE_FAILURES event signalling*/
   if (pData->data.nLineFailures)
   {
      /* Unmask (enable) DSL_EVENT_I_LINE_FAILURES event*/
      pOpenCtx->nEventMask &= ~DSL_EVENT2MASK(DSL_EVENT_I_LINE_FAILURES);
   }
   else
   {
      /* Mask (disable) DSL_EVENT_I_LINE_FAILURES event*/
      pOpenCtx->nEventMask |= DSL_EVENT2MASK(DSL_EVENT_I_LINE_FAILURES);   
   }

   DSL_DRV_MUTEX_UNLOCK(pOpenCtx->eventMutex);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_AlarmMaskLineFailuresConfigSet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_G997_AlarmMaskLineFailuresConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineFailures_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_AlarmMaskLineFailuresConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( pData->nDirection == DSL_NEAR_END )
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineFailuresNeAlarmMask,
         pData->data.nLineFailures);
   }
   else
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineFailuresFeAlarmMask,
         pData->data.nLineFailures);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_AlarmMaskLineFailuresConfigGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_FAILURES_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_LineFailuresStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineFailures_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineFailuresStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pData->nDirection == DSL_NEAR_END)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineFailuresNe, pData->data.nLineFailures);
   }
   else
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineFailuresFe, pData->data.nLineFailures);
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Loss-of-signal failure %s (LOS) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), pData->nDirection == DSL_NEAR_END ? "NE" : "FE",
      (pData->data.nLineFailures & DSL_G997_LINEFAILURE_LOS) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Loss of margin failure %s (LOM) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), pData->nDirection == DSL_NEAR_END ? "NE" : "FE",
      (pData->data.nLineFailures & DSL_G997_LINEFAILURE_LOM) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Loss-of-power failure %s (LPR) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), pData->nDirection == DSL_NEAR_END ? "NE" : "FE",
      (pData->data.nLineFailures & DSL_G997_LINEFAILURE_LPR) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Excessive Severe Errors Failure %s (ESE) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), pData->nDirection == DSL_NEAR_END ? "NE" : "FE",
      (pData->data.nLineFailures & DSL_G997_LINEFAILURE_ESE) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Loss of link failure %s (LOL) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), pData->nDirection == DSL_NEAR_END ? "NE" : "FE",
      (pData->data.nLineFailures & DSL_G997_LINEFAILURE_LOL) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Loss of frame failure %s (LOF) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), pData->nDirection == DSL_NEAR_END ? "NE" : "FE",
      (pData->data.nLineFailures & DSL_G997_LINEFAILURE_LOF) ? 1 : 0 ));

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineFailuresStatusGet"
      " retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_ALARM
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_SET
*/
DSL_Error_t DSL_DRV_G997_AlarmMaskDataPathFailuresConfigSet(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DataPathFailures_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_AlarmMaskDataPathFailuresConfigSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( pData->nDirection == DSL_NEAR_END )
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nDataPathFailuresNeAlarmMask,
         pData->data.nDataPathFailures);
   }
   else
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nDataPathFailuresFeAlarmMask,
         pData->data.nDataPathFailures);
   }

   if(DSL_DRV_MUTEX_LOCK(pOpenCtx->eventMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: Couldn't lock event mutex"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }

   /* Manage DSL_EVENT_I_DATA_PATH_FAILURES event signalling*/
   if (pData->data.nDataPathFailures)
   {
      /* Unmask (enable) DSL_EVENT_I_DATA_PATH_FAILURES event*/
      pOpenCtx->nEventMask &= ~DSL_EVENT2MASK(DSL_EVENT_I_DATA_PATH_FAILURES);
   }
   else
   {
      /* Mask (disable) DSL_EVENT_I_DATA_PATH_FAILURES event*/
      pOpenCtx->nEventMask |= DSL_EVENT2MASK(DSL_EVENT_I_DATA_PATH_FAILURES);   
   }

   DSL_DRV_MUTEX_UNLOCK(pOpenCtx->eventMutex);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_AlarmMaskDataPathFailuresConfigSet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_G997_AlarmMaskDataPathFailuresConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DataPathFailures_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_AlarmMaskDataPathFailuresConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( pData->nDirection == DSL_NEAR_END )
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nDataPathFailuresNeAlarmMask,
         pData->data.nDataPathFailures);
   }
   else
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nDataPathFailuresFeAlarmMask,
         pData->data.nDataPathFailures);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_AlarmMaskDataPathFailuresConfigGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DATA_PATH_FAILURES_STATUS_GET
*/
DSL_Error_t DSL_DRV_G997_DataPathFailuresStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DataPathFailures_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_CHANNEL_RANGE(pData->nChannel);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DataPathFailuresStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pData->nDirection == DSL_NEAR_END)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nDataPathFailuresNe[pData->nChannel],
         pData->data.nDataPathFailures);
   }
   else
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nDataPathFailuresFe[pData->nChannel],
         pData->data.nDataPathFailures);
   }

   DSL_DEBUG( DSL_DBG_WRN,
      (pContext, "DSL[%02d]: No Cell Delineation (NCD(-FE)) failure (NCD) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext),
      (pData->data.nDataPathFailures & DSL_G997_DATAPATHFAILURE_NCD) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_WRN,
      (pContext, "DSL[%02d]: Loss of Cell Delineation (LCD(-FE)) failure (LCD) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext),
      (pData->data.nDataPathFailures & DSL_G997_DATAPATHFAILURE_LCD) ? 1 : 0 ));

   DSL_DEBUG( DSL_DBG_WRN,
      (pContext, "DSL[%02d]: Out of Sync (OOS(-FE)) failure (OOS) = %d" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext),
      (pData->data.nDataPathFailures & DSL_G997_DATAPATHFAILURE_OOS) ? 1 : 0 ));

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_DataPathFailuresStatusGet,"
      " retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_FRAMING_PARAMETER_STATUS_GET
*/
#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
DSL_Error_t DSL_DRV_G997_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_FramingParameterStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_CHANNEL_RANGE(pData->nChannel);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_FramingParameterStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_FramingParameterStatusGet(
                 pContext, pData->nDirection, pData->nChannel, &(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_FramingParameterStatusGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_INVENTORY_GET
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_G997_LineInventoryGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventory_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   DSL_boolean_t bFeLineInventoryValid = DSL_FALSE, bFeLineInventoryIncomplete = DSL_FALSE;
      
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineInventoryGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get Current Line State*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   /* Check FE line inventory availability Line State*/
   if (pData->nDirection == DSL_FAR_END)
   {
      if (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
      {
         return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
      }

      /* Get FE line inventory flag*/
      DSL_CTX_READ(pContext, nErrCode, bFeLineInventoryValid, bFeLineInventoryValid);
      
      /* Check FE line inventory indication flag*/
      if (!bFeLineInventoryValid)
      {
         return DSL_ERR_DEVICE_NO_DATA;   
      }

      /* Get FE line inventory incomplete flag*/
      DSL_CTX_READ(pContext, nErrCode, bFeLineInventoryIncomplete, bFeLineInventoryIncomplete);
   }

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_LineInventoryGet(pContext, pData);

   /* Check for incomplete return values*/
   if ((nErrCode == DSL_SUCCESS) && bFeLineInventoryIncomplete)
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LineInventoryGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_INVENTORY_SET
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_G997_LineInventorySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventoryNe_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LineInventorySet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_LineInventorySet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_LineInventorySet, retCode=%d",
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_DELT
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_HLIN_SCALE_GET
*/
DSL_Error_t DSL_DRV_G997_DeltHlinScaleGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltHlinScale_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DeltHlinScaleGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_DeltHlinScaleGet(
                 pContext, pData->nDirection,
                 pData->nDeltDataType,&(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_DeltHlinScaleGet, retCode=%d",
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_HLIN_GET
*/
DSL_Error_t DSL_DRV_G997_DeltHlinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltHlin_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DeltHlinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_DeltHlinGet(
                 pContext, pData->nDirection,
                 pData->nDeltDataType,&(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_DeltHlinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_HLOG_GET
*/
DSL_Error_t DSL_DRV_G997_DeltHlogGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltHlog_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DeltHlogGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_DeltHlogGet(
                 pContext, pData->nDirection,
                 pData->nDeltDataType, &(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_DeltHlogGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_QLN_GET
*/
DSL_Error_t DSL_DRV_G997_DeltQLNGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltQln_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DeltQLNGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_DeltQLNGet(
                 pContext, pData->nDirection,
                 pData->nDeltDataType, &(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_DeltQLNGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_SNR_GET
*/
DSL_Error_t DSL_DRV_G997_DeltSNRGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltSnr_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DeltSNRGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_G997_DeltSNRGet(
                 pContext, pData->nDirection,
                 pData->nDeltDataType, &(pData->data));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_DeltSNRGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_SNR_GET
*/
DSL_Error_t DSL_DRV_G997_DeltFreeResources(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltFreeResources_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_DeltFreeResources"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifndef DSL_CPE_STATIC_DELT_DATA
   if (pContext->DELT != DSL_NULL)
   {
      DSL_DRV_MemFree(pContext->DELT);
      pContext->DELT = DSL_NULL;
   }
#ifdef INCLUDE_DSL_CPE_API_VINAX
   if (pContext->DELT_SHOWTIME != DSL_NULL)
   {
      DSL_DRV_MemFree(pContext->DELT_SHOWTIME);
      pContext->DELT_SHOWTIME = DSL_NULL;
   }
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#else
   nErrCode = DSL_WRN_NOT_SUPPORTED_DUE_TO_BUILD_CONFIG;
#endif

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext,"DSL[%02d]: OUT - DSL_DRV_G997_DeltFreeResources, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_DELT*/

#ifdef __cplusplus
}
#endif
