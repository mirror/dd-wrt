/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_pm_core.h"

#if defined (INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_PM)

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_PM

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_Start(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* No limitations to data access in case of Danube, Amazon-Se and AR9 */
   DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid = DSL_FALSE;

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_Restart(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* Nothing to do*/

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_DEV_Suspend(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* Reset bPmDataValid flag*/
   DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid = DSL_FALSE;

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   /* Reset bNeChannelCntReset flag*/
   DSL_DRV_PM_CONTEXT(pContext)->bNeChannelCntReset = DSL_FALSE;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_DEV_ShowtimeReachedHandle(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   DSL_uint16_t info103 = 0;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   /* Get Channel counters reset indication from the FW*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE, 1, 1, &info103);

   if ( nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - INFO 103 read failed!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }

   /* Check for the Channel counters FW autonomous reset*/
   if ( (info103 & 0x20) == 0 )
   {
      /* Set bNeChannelCntReset flag. Flag will be cleared on the 1st
         Channel Counters FW access*/
      DSL_DRV_PM_CONTEXT(pContext)->bNeChannelCntReset = DSL_TRUE;

      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: FW channel counters autonomous reset is enabled" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

   /* Set bPmDataValid flag*/
   DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid = DSL_TRUE;

   return nErrCode;
}


#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_ChannelCountersGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_ChannelData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t nIdx = 0;
   DSL_uint16_t nAddrCRC = 0, nAddrFEC = 0, nVal = 0;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_LatencyPath_t nLPath = DSL_LATENCY_DISABLED;

   DSL_uint16_t hdlcCmd[2] = {0};
   DSL_uint16_t hdlcRxBuffer[29] = {0};
   DSL_int_t nHdlcRxLen = 0;

   DSL_PM_ChannelData_t *pCurrCounters = DSL_NULL;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_ChannelCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ATU_DIRECTION(nDirection);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if ((nDirection == DSL_FAR_END) && (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC))
   {
      return nErrCode;
   }

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   if (nDirection == DSL_NEAR_END)
   {
      nAddrCRC = DSL_CMV_ADDRESS_PLAM_CRC_NE;
      nAddrFEC = DSL_CMV_ADDRESS_PLAM_FEC_NE;
   }
   else
   {
      nAddrCRC = DSL_CMV_ADDRESS_PLAM_CRC_FE;
      nAddrFEC = DSL_CMV_ADDRESS_PLAM_FEC_FE;
   }

   if (!bAdsl1 && nDirection == DSL_FAR_END)
   {
      hdlcCmd[0] = 0x105;

      /* Lock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

      if ((nErrCode = DSL_DRV_DANUBE_HdlcWrite(pContext, DSL_FALSE,
         (DSL_uint8_t *)hdlcCmd, 2)) == DSL_SUCCESS)
      {
         DSL_WAIT (1);

         nErrCode = DSL_DRV_DANUBE_HdlcRead(pContext, DSL_FALSE,
            (DSL_uint8_t*)hdlcRxBuffer, sizeof(hdlcRxBuffer), &nHdlcRxLen);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nFEC            = (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[2]);
            pCounters->nCodeViolations = (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[4]);
         }
      }

      /* Unlock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);
   }
   else
   {
      if (DSL_DRV_DANUBE_ActLatencyGet(pContext, nChannel, nDirection, &nLPath) >= DSL_SUCCESS)
      {
         if (nLPath == DSL_LATENCY_IP_LP0) nIdx = 0;
         else nIdx = 1;

         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrCRC, nIdx, 1, &nVal);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nCodeViolations = nVal;
         }

         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
               nAddrFEC, nIdx, 1, &nVal);
            if (nErrCode == DSL_SUCCESS)
            {
               pCounters->nFEC = nVal;
            }
         }
      }
      else
      {
         pCounters->nCodeViolations = 0;
         pCounters->nFEC = 0;
      }
   }


   /* Check for the NE counters FW autonomous reset*/
   if ((DSL_DRV_PM_CONTEXT(pContext)->bNeChannelCntReset == DSL_TRUE) &&
       (nDirection == DSL_NEAR_END))
   {
      /* Reset bNeChannelCntReset flag*/
      DSL_DRV_PM_CONTEXT(pContext)->bNeChannelCntReset = DSL_FALSE;

      /* Get pointer to the current channel counters*/
      pCurrCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_CURR(nChannel,nDirection);
      if (pCurrCounters == DSL_NULL)
      {
         nErrCode = DSL_ERR_INTERNAL;
      }
      else
      {
         DSL_DEBUG( DSL_DBG_MSG,
            (pContext, "DSL[%02d]: Channel reference counters reset due to the enabled"
            " FW autonomous reset" DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         /* Reset current (reference) values*/
         pCurrCounters->nFEC = 0;
         pCurrCounters->nCodeViolations = 0;
      }
   }

   if (nErrCode == DSL_ERR_DEVICE_NO_DATA)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Timed out while waiting for HDLC data"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));
      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_ChannelCountersCurrentGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_ChannelCountersExtGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_ChannelDataExt_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t sprFrame[2] = {0};
   DSL_uint32_t nSuperFrameCnt = 0, nSavedSuperFrameCnt;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_ChannelCountersExtGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ATU_DIRECTION(nDirection);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   /* Get SuperFrame counter LSW*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_PLAM,
                 20, 0, 1, &sprFrame[0]);
   if (nErrCode == DSL_SUCCESS)
   {
      /* Get SuperFrame counter MSW*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_PLAM,
                    21, 0, 1, &sprFrame[1]);

      if (nErrCode == DSL_SUCCESS)
      {
         nSuperFrameCnt = (sprFrame[1] << 16) | sprFrame[0];

         nSavedSuperFrameCnt =
            DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->ChannelCounters.data_ne_ext[nChannel].nSuperFrame;

         /* SuperFrame counter is not restored in the FW. Current value = Saved value + FW value*/
         pCounters->nSuperFrame = nSavedSuperFrameCnt + nSuperFrameCnt;
      }
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_ChannelCountersExtGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_ChannelCountersSet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_ChannelData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t nIdx = 0;
   DSL_uint16_t nVal;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_LatencyPath_t nLPath = DSL_LATENCY_DISABLED;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_ChannelCountersCurrentSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   if( nDirection != DSL_NEAR_END )
   {
      return DSL_SUCCESS;
   }

   /* Get ADSL mode information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   if ((!bAdsl1) && (DSL_CHANNELS_PER_LINE > 1))
   {
      if (DSL_DRV_DANUBE_ActLatencyGet(pContext, nChannel, nDirection, &nLPath) >= DSL_SUCCESS)
      {
         if (nLPath == DSL_LATENCY_IP_LP0) nIdx = 0;
         else nIdx = 1;

         /* FW value pecision is less than API*/
         nVal = (DSL_uint16_t)pCounters->nCodeViolations;
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
                       DSL_CMV_ADDRESS_PLAM_CRC_NE, nIdx, 1, &nVal);

         if (nErrCode == DSL_SUCCESS)
         {
            /* FW value pecision is less than API*/
            nVal = (DSL_uint16_t)pCounters->nFEC;
            nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
                          DSL_CMV_ADDRESS_PLAM_FEC_NE, nIdx, 1, &nVal);
         }
      }
   }
   else
   {
      /* In case of ADSL1 or single latency write values to both interl/LP0 and
         fast/LP1 */
      for (nIdx = 0; nIdx < 2; nIdx++)
      {
         /* FW value pecision is less than API*/
         nVal = (DSL_uint16_t)pCounters->nCodeViolations;
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
                       DSL_CMV_ADDRESS_PLAM_CRC_NE, nIdx, 1, &nVal);

         if (nErrCode < DSL_SUCCESS) break;

         /* FW value pecision is less than API*/
         nVal = (DSL_uint16_t)pCounters->nFEC;
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
                       DSL_CMV_ADDRESS_PLAM_FEC_NE, nIdx, 1, &nVal);

         if (nErrCode < DSL_SUCCESS) break;
      }
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_ChannelCountersCurrentSet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_DataPathCountersGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nAddrHEC, nAddrIBE, nAddrTotalCells,
                nAddrUserTotalCells = (DSL_uint16_t)(-1), nVal[2];
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nLineState = DSL_LINESTATE_UNKNOWN;

   DSL_uint16_t hdlcCmd[2] = {0};
   DSL_uint16_t hdlcRxBuffer[29] = {0};
   DSL_int_t nHdlcRxLen = 0;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_DataPathCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ATU_DIRECTION(nDirection);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if ((nDirection == DSL_FAR_END) && (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC))
   {
      return nErrCode;
   }

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   if (nDirection == DSL_NEAR_END)
   {
      nAddrHEC = DSL_CMV_ADDRESS_PLAM_HEC_NE;
      nAddrTotalCells = DSL_CMV_ADDRESS_PLAM_TOTAL_CELLS_NE;
      nAddrUserTotalCells = DSL_CMV_ADDRESS_PLAM_USER_TOTAL_CELLS_NE;
      nAddrIBE = DSL_CMV_ADDRESS_PLAM_IBE_NE;
   }
   else
   {
      nAddrHEC = DSL_CMV_ADDRESS_PLAM_HEC_FE;
      nAddrTotalCells = DSL_CMV_ADDRESS_PLAM_TOTAL_CELLS_FE;
/*$$      nAddrUserTotalCells = DSL_CMV_ADDRESS_PLAM_USER_TOTAL_CELLS_FE;*/
      nAddrIBE = DSL_CMV_ADDRESS_PLAM_IBE_FE;
   }

   if ((!bAdsl1) && (nDirection == DSL_FAR_END))
   {
      /* ADSL2/2+ only*/
      hdlcCmd[0] = 0x105;

      /* Lock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

      if ((nErrCode = DSL_DRV_DANUBE_HdlcWrite(pContext, DSL_FALSE,
         (DSL_uint8_t *)hdlcCmd, 2))
         == DSL_SUCCESS)
      {
         DSL_WAIT (1);

         nErrCode = DSL_DRV_DANUBE_HdlcRead(pContext, DSL_FALSE,
            (DSL_uint8_t*)hdlcRxBuffer, sizeof(hdlcRxBuffer), &nHdlcRxLen);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nHEC =
               ((DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[15]) << 16) +
               (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[16]);
            pCounters->nTotalCells =
               ((DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[17]) << 16) +
               (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[18]);
            pCounters->nUserTotalCells =
               ((DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[19]) << 16) +
               (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[20]);
            pCounters->nIBE =
               ((DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[21]) << 16) +
               (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[22]);
         }
      }

      /* Unlock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);
   }
   else
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
         nAddrHEC, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrHEC, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);

         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nHEC = (nVal[1] << 16) | nVal[0];
         }
      }

      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrTotalCells, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
               nAddrTotalCells, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);

            if (nErrCode == DSL_SUCCESS)
            {
               pCounters->nTotalCells = (nVal[1] << 16) | nVal[0];
            }
         }
      }

      if (nErrCode == DSL_SUCCESS && nDirection == DSL_NEAR_END)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrUserTotalCells, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
               nAddrUserTotalCells, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);

            if (nErrCode == DSL_SUCCESS)
            {
               pCounters->nUserTotalCells = (nVal[1] << 16) | nVal[0];
            }
         }
      }
      else if (nDirection == DSL_FAR_END)
      {
         pCounters->nUserTotalCells =
            (DSL_DRV_PM_PTR_DATAPATH_COUNTERS_CURR(nChannel, DSL_FAR_END))->nUserTotalCells;
      }

      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrIBE, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
               nAddrIBE, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);

            if (nErrCode == DSL_SUCCESS)
            {
               pCounters->nIBE = (nVal[1] << 16) | nVal[0];
            }
         }
      }
   }

   /* Not supported yet*/
   pCounters->nTxUserTotalCells = 0;
   pCounters->nTxIBE            = 0;

   if (nErrCode == DSL_ERR_DEVICE_NO_DATA)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Timed out while waiting for HDLC data"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));
      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_DataPathCountersCurrentGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_DataPathCountersSet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal[2];

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_DataPathCountersCurrentSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   if( nDirection != DSL_NEAR_END )
   {
      return DSL_SUCCESS;
   }

   nVal[0] = pCounters->nHEC & 0xFFFF;
   nVal[1] = (pCounters->nHEC >> 16) & 0xFFFF;
   nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
      DSL_CMV_ADDRESS_PLAM_HEC_NE, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_HEC_NE, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);
   }

   nVal[0] = pCounters->nTotalCells & 0xFFFF;
   nVal[1] = (pCounters->nTotalCells >> 16) & 0xFFFF;
   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_TOTAL_CELLS_NE, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
            DSL_CMV_ADDRESS_PLAM_TOTAL_CELLS_NE, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);
      }
   }

   nVal[0] = pCounters->nUserTotalCells & 0xFFFF;
   nVal[1] = (pCounters->nUserTotalCells >> 16) & 0xFFFF;
   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_USER_TOTAL_CELLS_NE, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
            DSL_CMV_ADDRESS_PLAM_USER_TOTAL_CELLS_NE, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);
      }
   }

   nVal[0] = pCounters->nIBE & 0xFFFF;
   nVal[1] = (pCounters->nIBE >> 16) & 0xFFFF;
   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_IBE_NE, (DSL_uint16_t)(2*nChannel), 1, &nVal[0]);
      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
            DSL_CMV_ADDRESS_PLAM_IBE_NE, (DSL_uint16_t)(2*nChannel + 1), 1, &nVal[1]);
      }
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_DataPathCountersCurrentSet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_DataPathFailureCountersGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_DataPathFailureData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_DataPathFailureCountersGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: Couldn't lock data mutex"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Copy Data Path failure counters from the CPE API Context*/
   memcpy(pCounters,
      nDirection == DSL_NEAR_END ?
      &(pContext->pmDataPathFailureCounters.data_ne[nChannel]) :
      &(pContext->pmDataPathFailureCounters.data_fe[nChannel]),
      sizeof(DSL_PM_DataPathFailureData_t));

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_DataPathFailureCountersGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_LineSecCountersGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineSecData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nAddrES, nAddrSES, nAddrLOSS, nAddrUAS = (DSL_uint16_t)(-1), nVal;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nLineState = DSL_LINESTATE_UNKNOWN;

   DSL_uint16_t hdlcCmd[2] = {0};
   DSL_uint16_t hdlcRxBuffer[29] = {0};
   DSL_int_t nHdlcRxLen = 0;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_LineSecCountersGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(nDirection);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if ((nDirection == DSL_FAR_END) && (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC))
   {
      return nErrCode;
   }

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   if (nDirection == DSL_NEAR_END)
   {
      nAddrES   = DSL_CMV_ADDRESS_PLAM_ES_NE;
      nAddrSES  = DSL_CMV_ADDRESS_PLAM_SES_NE;
      nAddrLOSS = DSL_CMV_ADDRESS_PLAM_LOSS_NE;
      nAddrUAS  = DSL_CMV_ADDRESS_PLAM_UAS_NE;
   }
   else
   {
      nAddrES   = DSL_CMV_ADDRESS_PLAM_ES_FE;
      nAddrSES  = DSL_CMV_ADDRESS_PLAM_SES_FE;
      nAddrLOSS = DSL_CMV_ADDRESS_PLAM_LOSS_FE;
      nAddrUAS  = DSL_CMV_ADDRESS_PLAM_UAS_FE;
   }

   if ((!bAdsl1) && (nDirection == DSL_FAR_END))
   {
      /* ADSL2/2+ only*/
      hdlcCmd[0] = 0x105;

      /* Lock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

      if ((nErrCode = DSL_DRV_DANUBE_HdlcWrite(pContext, DSL_FALSE,
         (DSL_uint8_t *)hdlcCmd, 2))
         == DSL_SUCCESS)
      {
         DSL_WAIT (1);

         nErrCode = DSL_DRV_DANUBE_HdlcRead(pContext, DSL_FALSE,
            (DSL_uint8_t*)hdlcRxBuffer, sizeof(hdlcRxBuffer), &nHdlcRxLen);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nES   = (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[8]);
            pCounters->nSES  = (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[10]);
            pCounters->nLOSS = (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[12]);
            pCounters->nUAS  = (DSL_uint32_t) DSL_Le2Cpu(hdlcRxBuffer[14]);
         }
      }

      /* Unlock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);
   }
   else
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
         nAddrES, 0, 1, &nVal);
      if (nErrCode == DSL_SUCCESS)
      {
         pCounters->nES = (DSL_uint32_t)nVal;
      }

      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrSES, 0, 1, &nVal);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nSES = (DSL_uint32_t)nVal;
         }
      }

      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrLOSS, 0, 1, &nVal);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nLOSS = (DSL_uint32_t)nVal;
         }
      }

      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
            nAddrUAS, 0, 1, &nVal);
         if (nErrCode == DSL_SUCCESS)
         {
            pCounters->nUAS = (DSL_uint32_t)nVal;
         }
      }
   }

   pCounters->nLOFS = 0;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: DSL_DRV_PM_DEV_LineSecCountersGet:"
      DSL_DRV_CRLF "\tnDirection=%lu"DSL_DRV_CRLF"\tnES=%lu"
      DSL_DRV_CRLF"\tnSES=%lu"DSL_DRV_CRLF"\tnLOSS=%lu"
      DSL_DRV_CRLF"\tnUAS=%lu"DSL_DRV_CRLF, DSL_DEV_NUM(pContext),
      nDirection, pCounters->nES, pCounters->nSES,
      pCounters->nLOSS, pCounters->nUAS));

   if (nErrCode == DSL_ERR_DEVICE_NO_DATA)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Timed out while waiting for HDLC data"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));
      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_LineSecCountersGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_LineInitCountersGet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_LineInitCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: Couldn't lock data mutex"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERR_SEMAPHORE_GET;
   }

   memcpy(pCounters, &pContext->pDevCtx->data.lineInitCounters,
      sizeof(DSL_PM_LineInitData_t));

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_LineInitCountersCurrentGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_DEV_LineInitCountersSet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitData_t *pCounters)
{
   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_LineInitCountersSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_LineInitCountersSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return DSL_SUCCESS;
}
#endif
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_LineSecCountersSet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineSecData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_LineSecCountersSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(nDirection);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   if (nDirection != DSL_NEAR_END)
   {
      return DSL_SUCCESS;
   }

   /* FW value width is less than the API one*/
   nVal = (DSL_uint16_t)pCounters->nES;
   nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
      DSL_CMV_ADDRESS_PLAM_ES_NE, 0, 1, &nVal);
   if (nErrCode == DSL_SUCCESS)
   {
      /* FW value width is less than the API one*/
      nVal = (DSL_uint16_t)pCounters->nSES;
      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_SES_NE, 0, 1, &nVal);
   }

   if (nErrCode == DSL_SUCCESS)
   {
      /* FW value width is less than the API one*/
      nVal = (DSL_uint16_t)pCounters->nLOSS;
      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_LOSS_NE, 0, 1, &nVal);
   }

   if (nErrCode == DSL_SUCCESS)
   {
      nVal = (DSL_uint16_t)pCounters->nUAS;

      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_UAS_NE, 0, 1, &nVal);
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: DSL_DRV_PM_DEV_LineSecCountersSet:"
      DSL_DRV_CRLF "\tnDirection=%lu"DSL_DRV_CRLF"\tnES=%lu"
      DSL_DRV_CRLF"\tnSES=%lu"DSL_DRV_CRLF"\tnLOSS=%lu"
      DSL_DRV_CRLF"\tnUAS=%lu"DSL_DRV_CRLF/*"\tnNewUAS=%lu"DSL_DRV_CRLF*/,
      DSL_DEV_NUM(pContext), nDirection, pCounters->nES,
      pCounters->nSES, pCounters->nLOSS, pCounters->nUAS/*, nVal*/));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_LineSecCountersSet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_LineEventShowtimeCountersGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineEventShowtimeData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_LineEventShowtimeCountersGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: Couldn't lock data mutex"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Copy Line failure counters from the CPE API Context*/
   memcpy(pCounters,
      nDirection == DSL_NEAR_END ?
      &(pContext->pmLineEventShowtimeCounters.data_ne) :
      &(pContext->pmLineEventShowtimeCounters.data_fe),
      sizeof(DSL_PM_LineEventShowtimeData_t));

   /* Not supported yet*/
   pCounters->nSosSuccess = 0;

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_LineEventShowtimeCountersGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_LineEventShowtimeCountersSet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineEventShowtimeData_t *pCounters)
{
   return DSL_SUCCESS;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DEV_ReTxCountersGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_ReTxData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nData[8] = {0}, i;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DEV_ReTxCountersGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(nDirection);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   memset(pCounters, 0x0, sizeof(DSL_PM_ReTxData_t));
      
   if (nDirection == DSL_FAR_END)
   {
      return nErrCode;
   }

   /* Get all ReTx counters from the FW*/
   for (i = 0; i < 8; i++)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_RETX_COUNTERS, i, 1, &nData[i]);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - INFO 112 %u read failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), i));
         break;
      }
   }

   /* Fill the output counters structure*/
   if (nErrCode == DSL_SUCCESS)
   {
      /* RxDtuCorruptedCNT */
      pCounters->nRxCorruptedTotal =
         (DSL_uint32_t)((((DSL_uint32_t)nData[1]) << 16) | nData[0]);

      /* RxRetxDtuUnCorrected */
      pCounters->nRxUncorrectedProtected =
         (DSL_uint32_t)((((DSL_uint32_t)nData[3]) << 16) | nData[2]);

      /* RxDtuRetransmittedCNT */
      pCounters->nRxRetransmitted =
         (DSL_uint32_t)((((DSL_uint32_t)nData[5]) << 16) | nData[4]);

      /* RxDtuCorrectedCNT */
      pCounters->nRxCorrected =
         (DSL_uint32_t)((((DSL_uint32_t)nData[7]) << 16) | nData[6]);
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DEV_ReTxCountersGet (retCode=%d)"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_DEV_HistoryIntervalAlign(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* Nothing to do*/

   return nErrCode;   
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_PM)*/
