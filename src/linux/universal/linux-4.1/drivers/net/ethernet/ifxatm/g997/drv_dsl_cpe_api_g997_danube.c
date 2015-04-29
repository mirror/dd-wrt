/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#if defined (INCLUDE_DSL_CPE_API_DANUBE)

#include "drv_dsl_cpe_danube.h"

#ifdef __cplusplus
   extern "C" {
#endif

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_DEVICE

#ifdef INCLUDE_DSL_DELT
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltValuesSpread(
   DSL_uint8_t dataType,
   DSL_uint8_t nGroupSize,
   DSL_uint16_t nOrigDataSize,
   DSL_void_t *pOrigData,
   DSL_void_t *pSpreadData);

static DSL_Error_t DSL_DRV_DANUBE_G997_DeltHlogGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_DeltHlogData_t *pHlogData);

static DSL_Error_t DSL_DRV_DANUBE_G997_ShowtimeHlogUsGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_G997_DeltHlogData_t *pHlogData);

static DSL_Error_t DSL_DRV_DANUBE_G997_DeltQLNGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_DeltQlnData_t *pQlnData);

static DSL_Error_t DSL_DRV_DANUBE_G997_ShowtimeQlnUsGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_G997_DeltQlnData_t *pQlnData);

static DSL_Error_t DSL_DRV_DANUBE_G997_DeltSNRGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_DeltSnrData_t *pSnrData);
#endif /* INCLUDE_DSL_DELT*/

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_BitAllocationNSCGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_BitAllocationNsc_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nIndex = 0, nSize = 12, nMsgIdx = 0;
   DSL_uint16_t buf[12];
   DSL_uint16_t nAddr;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_uint16_t nNSC;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_BitAllocationNSCGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Could not read current line state"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Call to this function is possible in "
         "the SHOWTIME state only!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   /* Get number of used tones according to the current ADSL mode*/
   nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, pData->nDirection, &nNSC);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   nAddr = pData->nDirection == DSL_DOWNSTREAM ?
              DSL_CMV_ADDRESS_INFO_BAT_DS :
              DSL_CMV_ADDRESS_INFO_BAT_US;

   for (nIndex = 0; (nIndex < nNSC / 2) && (nIndex < DSL_MAX_NSC / 2) ; nIndex += nSize)
   {
      if (nIndex + nSize >= nNSC / 2)
      {
         nSize = nNSC / 2 - nIndex;
      }

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: read BAT: [%d:%d]"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nIndex, nSize));
      
      if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         nAddr, nIndex, nSize, buf)) == DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_MSG, (pContext, "   "));
         for (nMsgIdx = 0; (nMsgIdx < nSize) &&
              (nSize <= sizeof(buf)/sizeof(buf[0])) && ((nIndex + nMsgIdx) * 2 + 1 < DSL_MAX_NSC);
              nMsgIdx++)
         {
            pData->data.bitAllocationNsc.nNSCData[(nIndex + nMsgIdx) * 2]
               = (DSL_uint8_t) (buf[nMsgIdx] & 0xFF);
            pData->data.bitAllocationNsc.nNSCData[(nIndex + nMsgIdx) * 2 + 1]
               = (DSL_uint8_t) ((buf[nMsgIdx] >> 8) & 0xFF);
            DSL_DEBUG(DSL_DBG_MSG, (pContext,
               "%d %d ", pData->data.bitAllocationNsc.nNSCData[(nIndex + nMsgIdx) * 2],
               pData->data.bitAllocationNsc.nNSCData[(nIndex + nMsgIdx) * 2 + 1]));
         }
         DSL_DEBUG(DSL_DBG_MSG, (pContext, "   "DSL_DRV_CRLF));
      }
      else
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - CMV fail, Group INFO Address %d Index %d!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nAddr, nIndex));
      }
   }

   pData->data.bitAllocationNsc.nNumData = nNSC;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_BitAllocationNSCGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_G997_PER_TONE
/*
      Description:
      Computes the decimal equivalent of the fine gain in 3.9 format,
               left-justified, i.e. least significant 4 bits are zeroes.
      Assumes the the input gain_db is in 8.8 format and is within the
      range [-14.0 dB, +6 dB].
      Special value 0x8000 indicates infinite dB (linear 0)
      Returns 10^(gain_db/20) in 3.13 format (i.e. 8192 is eqv to 1.0)

      Algorithm:
          1) Map input x to range [256, 768] (=[+1.0 dB, 3.0 dB])

          2) Approximates 10^(x/20) in the desired range by
             p(x) = 0.00104309497*x^2 + 3.57736185790*x + 8208.07332753051

          3) Adjust approximation if original input x was not in
             range [256, 768]

       Input Arguments:
               s_gain_db -- file gain in dB (Q8.8 format)

       Return:
               decimal gain (in Q3.13 format)

*/
static DSL_int16_t DSL_DRV_DANUBE_G997_Q8_8dB_to_Q3_13Linear(DSL_int16_t s_gain_db)
{
   DSL_int32_t s_adj_factor = (1 << 14);   /* nominal adjustment factor*/
   DSL_int32_t l_Acc, l_temp;
   DSL_int16_t s_DecimalValue;

   if ((DSL_uint16_t)s_gain_db == 0x8000)
   {
      l_Acc = 0;
   }/* Check that input is within allowed range of [-14.0dB, +6dB]*/
   else if (s_gain_db < -14<<8)
   { /* if x < -14.0 dB*/
      l_Acc = (DSL_int32_t) 1634; /* return 0.1995*8192*/
   }
   else if (s_gain_db > 6<<8)
   {
      l_Acc = (DSL_int32_t) 16345; /* return 1.9952*8192*/
   }
   else
   {
      if (s_gain_db > 3<<8)
      {
         while (s_gain_db > 3<<8) {
            s_gain_db -= 2<<8;
            /* for every 2dB, we adjust the final result by 10^(2/20) = (20626/16384)*/
            l_temp = (DSL_int32_t)(20626 * s_adj_factor);
            s_adj_factor = (DSL_int16_t)(l_temp >> 14);
         }
      }
      else if (s_gain_db < 256)
      {
         /* Map input to range [+1, +3], where the approximation is optimized for*/
         while (s_gain_db < 256) {
            s_gain_db += 2<<8;
            /* for every 2dB, we adjust the final result by 10^(-2/20) = (13014/16384)*/
            l_temp = (DSL_int32_t)(13014 * s_adj_factor);
            s_adj_factor = (DSL_int16_t)(l_temp >> 14);
         }
      }

      /* The coefficients c0, c1 and c2 are represented as mantissa
         plus exponent pairs to get the maximum dynamic range. Hence
         the rightshifts after the multiplications. c0 is a constant and
         can be greater than 16 bits.*/
      l_Acc = (DSL_int32_t)(s_gain_db * s_gain_db);

      l_Acc  = l_Acc  >> 5;

      l_temp = l_Acc * 17500;
      l_Acc  = l_temp >> 6;                           // c2*x*x
      l_temp = (DSL_int32_t)(29306 * s_gain_db);
      l_Acc += l_temp;

      l_Acc +=  67240537;                         // c2*x*x + c1*x + c0
      l_Acc  = (l_Acc + (1 << 12)) >> 13;         // round result

      // Adjust and round in case original input was not in range [+1, +3]
      l_temp = l_Acc * s_adj_factor;
      l_Acc  = (l_temp + (1 << 13)) >> 14;

   }

   s_DecimalValue = (DSL_int16_t) l_Acc;

   return (s_DecimalValue);
}

DSL_Error_t DSL_DRV_DEV_G997_GainAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_GainAllocationNsc_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nIndex = 0, nSize = 12, nMsgIdx = 0;
   DSL_uint16_t buf[12];
   DSL_uint16_t nAddr;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_uint16_t nNSC;
   DSL_uint16_t nGainPs = 0, intPart = 0;
   DSL_uint16_t ExcMrgRed[2]={0};

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_GainAllocationNscGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Call to this function is possible in "
         "the SHOWTIME state only!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   /* Get number of used tones according to the current ADSL mode*/
   nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, pData->nDirection, &nNSC);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   nAddr = pData->nDirection == DSL_DOWNSTREAM ?
              DSL_CMV_ADDRESS_INFO_GAIN_DS :
              DSL_CMV_ADDRESS_INFO_GAIN_US;
              
   if (pData->nDirection == DSL_DOWNSTREAM)
   {
      if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
          DSL_CMV_ADDRESS_INFO_EXCESS_MARGIN, 0, 2, ExcMrgRed)
          != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get Excess Margin Reduction!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         nErrCode = DSL_ERROR;
      }
   }

   for (nIndex = 0; (nIndex < nNSC) && (nErrCode == DSL_SUCCESS) &&
          (nIndex < DSL_MAX_NSC); nIndex += nSize)
   {
      if (nIndex + nSize >= nNSC)
      {
         nSize = nNSC - nIndex;
      }

      DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: read Gain allocation: [%d:%d]"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nIndex, nSize));
      if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         nAddr, nIndex, nSize, buf)) == DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_MSG, (pContext, "   "));
         for (nMsgIdx = 0; (nMsgIdx < nSize) &&
             (nSize <= sizeof(buf)/sizeof(buf[0])) && (nIndex + nMsgIdx < DSL_MAX_NSC); nMsgIdx++)
         {
            DSL_DEBUG(DSL_DBG_MSG, (pContext,
               "(%hu)", buf[nMsgIdx]));
            if (pData->nDirection == DSL_DOWNSTREAM)
            {
               if (buf[nMsgIdx] != 0x8000)
               {
                  /* Apply Excess Margin Reduction and Convert Q8.8dB values to Q3.13 linear and*/
                  nGainPs =
                     (DSL_uint16_t)(DSL_DRV_DANUBE_G997_Q8_8dB_to_Q3_13Linear(
                                      (DSL_int16_t)buf[nMsgIdx] - (DSL_int16_t)ExcMrgRed[0]));
                  intPart = nGainPs >> 4;
                  nGainPs = (nGainPs & 0xF) < 8 ? intPart : intPart + 1;
               }
               else
               {
                  nGainPs = 0;
               }
            }
            else if (pData->nDirection == DSL_UPSTREAM)
            {
               nGainPs = buf[nMsgIdx] >> 4;
            }

            pData->data.gainAllocationNsc.nNSCData[nIndex + nMsgIdx] = nGainPs;

            DSL_DEBUG(DSL_DBG_MSG, (pContext,
               "%hu ", pData->data.gainAllocationNsc.nNSCData[nIndex + nMsgIdx]));
         }
         DSL_DEBUG(DSL_DBG_MSG, (pContext, "   "DSL_DRV_CRLF));
      }
      else
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - CMV fail, Group INFO Address %d Index %d!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nAddr, nIndex));
      }
   }

   pData->data.gainAllocationNsc.nNumData = nNSC;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_GainAllocationNscGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_SnrAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_SnrAllocationNsc_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nIndex = 0, nSize = 12, nMsgIdx = 0;
   DSL_uint16_t buf[12];
   DSL_uint16_t nAddr, nSnrAdd, snrQ88 = 0, intPart = 0;
   DSL_boolean_t bAdsl1 = DSL_FALSE, bAdsl2p = DSL_FALSE;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_uint16_t nNSC = 0;
   DSL_uint16_t hdlcCmd[8] = {0}, nHdlcRxLenMax = 0;
   DSL_int_t nHdlcRxLen;
   DSL_uint16_t hdlcRxBuffer[68] = {0};
   DSL_uint16_t nDsRangeD, nDsRangeU;
   DSL_G997_PowerManagement_t nPowerMode = DSL_G997_PMS_NA;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_SnrAllocationNSCGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /*$$ ND FIXME: Get the maximum length of Rx buffer and apply it */

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Call to this function is possible in "
         "the SHOWTIME state only!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);
   /* Get ADSL2+ mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl2p, bAdsl2p);

   /* read HDLC max rx buffer length in case of UpStream and ADSL2/2+ */
   if ((pData->nDirection == DSL_UPSTREAM) && (!bAdsl1))
   {
      if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_ME_HDLC_PARAMS, 1,
         1, &nHdlcRxLenMax)) != DSL_SUCCESS)
      {
         return nErrCode;
      }
   }

   /* Get number of used tones according to the current ADSL mode*/
   nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, pData->nDirection, &nNSC);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   if (pData->nDirection == DSL_DOWNSTREAM)
   {
      nAddr = DSL_CMV_ADDRESS_INFO_SNRA_DS;

      for (nIndex = 0; (nIndex < nNSC) && (nIndex < DSL_MAX_NSC); nIndex += nSize)
      {
         if (nIndex + nSize >= nNSC)
         {
            nSize = nNSC - nIndex;
         }

         DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: read Snr allocation table: [%d:%d]"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nIndex, nSize));

         if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            nAddr, nIndex, nSize, buf)) == DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_MSG, (pContext, "   "));
            for (nMsgIdx = 0; (nMsgIdx < nSize) &&
                (nSize <= sizeof(buf)/sizeof(buf[0])) && (nIndex + nMsgIdx < DSL_MAX_NSC);
                nMsgIdx++)
            {
               snrQ88  = (DSL_uint16_t)(((buf[nMsgIdx]) + (32 << 8)) * 2);
               intPart = (snrQ88 >> 8) & 0xFF;

               pData->data.snrAllocationNsc.nNSCData[nIndex + nMsgIdx] =
                  (DSL_uint8_t)((snrQ88 & 0xFF) < 128 ? intPart : intPart + 1);

               DSL_DEBUG(DSL_DBG_MSG, (pContext,
                  "(%d) %d ", buf[nMsgIdx],
                  pData->data.snrAllocationNsc.nNSCData[nIndex + nMsgIdx]));
            }
            DSL_DEBUG(DSL_DBG_MSG, (pContext, "   "DSL_DRV_CRLF));
         }
         else
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - CMV fail, Group INFO Address %d Index %d!"
               DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nAddr, nIndex));
         }
      }
   }
   else if (pData->nDirection == DSL_UPSTREAM)
   {
      /* In L2 power mode, do not read the OHC related parameters, instead give
         the indication to the calling IOCTL, that the readout fails (just
         return DSL_WRN_INCOMPLETE_RETURN_VALUES).  */
      nErrCode = DSL_DRV_DANUBE_L3StatusGet(pContext, &nPowerMode);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Could not get power mode!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         return nErrCode;
      }

      if (nPowerMode == DSL_G997_PMS_L2)
      {
         return DSL_ERR_DEVICE_NO_DATA;
      }
      
      if ((bAdsl1) || (nNSC == 0))
      {
         /* Adsl1 */
         memset(&pData->data.snrAllocationNsc.nNSCData, 0, DSL_MAX_NSC);
         return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
      }

      /* 64 + 4 - size of allocated on stack rx buffer */
      nSize = (nHdlcRxLenMax > 64) ? 64 : nHdlcRxLenMax;
      for (nIndex = 0; (nIndex < nNSC) && (nIndex < DSL_MAX_NSC); nIndex += nSize)
      {
         if (nIndex + nSize >= nNSC)
         {
            nSize = nNSC - nIndex;
         }

         if (bAdsl2p)
         {
            hdlcCmd[0] = 0x181;
            hdlcCmd[1] = 0x4 | ((nIndex & 0xFF) << 8);
            hdlcCmd[2] = (((nIndex + nSize - 1) & 0xFF) << 8)
               | ((nIndex & 0xFF00) >> 8);
            hdlcCmd[3] = (((nIndex + nSize - 1) & 0xFF00) >> 8);
         }
         else
         {
            hdlcCmd[0] = 0x181;
            hdlcCmd[1] = 0x4 | ((nIndex & 0xFF) << 8);
            hdlcCmd[2] = ((nIndex + nSize - 1) & 0xFF);
         }

         /* Lock HDLC handling*/
         DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

         if ((nErrCode = DSL_DRV_DANUBE_HdlcWrite(pContext, DSL_FALSE,
            (DSL_uint8_t *)hdlcCmd, (bAdsl2p) ? 8 : 6)) == DSL_SUCCESS)
         {
            DSL_WAIT (1);

            nErrCode = DSL_DRV_DANUBE_HdlcRead(pContext, DSL_FALSE,
               (DSL_uint8_t*)hdlcRxBuffer, nSize + 4, &nHdlcRxLen);
            if (nErrCode == DSL_SUCCESS)
            {
               DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: HDLC READ ADSL2%s %d"
                  DSL_DRV_CRLF, DSL_DEV_NUM(pContext),
                  (bAdsl2p) ? "+" : "", nHdlcRxLen));
               for (nMsgIdx = 0; (nMsgIdx < nSize/2) &&
                                 (nSize <= sizeof(hdlcRxBuffer)/sizeof(hdlcRxBuffer[0]));
                                 nMsgIdx++)
               {
                  pData->data.snrAllocationNsc.nNSCData[2 *
                     (nMsgIdx + nIndex) + 1] =
                     (hdlcRxBuffer[nMsgIdx + 2] >> 8) & 0xFF;
                  pData->data.snrAllocationNsc.nNSCData[2 *
                     (nMsgIdx + nIndex)] =
                     (hdlcRxBuffer[nMsgIdx + 2]) & 0xFF;
               }
               DSL_DEBUG(DSL_DBG_MSG, (pContext, DSL_DRV_CRLF));
            }
         }

         /* Unlock HDLC handling*/
         DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Direction was not selected!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      nErrCode = DSL_ERROR;
   }

   if (nErrCode == DSL_SUCCESS)
   {
      pData->data.snrAllocationNsc.nNumData = nNSC;

      /* convert values for DS */
      if (pData->nDirection == DSL_DOWNSTREAM)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_DS_BIT_LOAD_FIRST, 0, 1, &nDsRangeD);
         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
               DSL_CMV_ADDRESS_INFO_DS_BIT_LOAD_LAST, 0, 1, &nDsRangeU);
            if (nErrCode == DSL_SUCCESS)
            {
               if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
                  DSL_CMV_ADDRESS_INFO_EXCESS_MARGIN, 0, 1, &nSnrAdd))
                  != DSL_SUCCESS)
               {
                  DSL_DEBUG( DSL_DBG_ERR,
                     (pContext, "DSL[%02d]: ERROR - Couldn't get EXCESSMGN for %s!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
                  nErrCode = DSL_ERROR;
               }
               else
               {
                  DSL_DEBUG(DSL_DBG_MSG, (pContext, " nSnrAdd=%h "DSL_DRV_CRLF,
                     (DSL_int16_t)nSnrAdd));
                  for (nIndex = 0; nIndex < nDsRangeD; nIndex++)
                  {
                     if (pData->data.snrAllocationNsc.nNSCData[nIndex] == 0x40)
                     {
                        pData->data.snrAllocationNsc.nNSCData[nIndex] = 0xFF;
                     }
                  }
                  for (nIndex = nDsRangeD; nIndex <= nDsRangeU; nIndex++)
                  {
                     pData->data.snrAllocationNsc.nNSCData[nIndex] += ((DSL_uint8_t)nSnrAdd);
                  }
                  for (nIndex = nDsRangeU + 1; nIndex < nNSC; nIndex++)
                  {
                     if (pData->data.snrAllocationNsc.nNSCData[nIndex] == 0x40)
                     {
                        pData->data.snrAllocationNsc.nNSCData[nIndex] = 0xFF;
                     }
                  }
               }
            }
         }
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_SnrAllocationNSCGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#define DSL_ABS_16(x) ((DSL_int16_t)(((x) < 0) ? (-1 * x) : (x)))

/*
   This function does not check pContext pointer, modem availability and
   pData pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_ActPSD_Get(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_AccessDir_t nDirection,
   DSL_OUT DSL_int16_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t XTSE[DSL_G997_NUM_XTSE_OCTETS] = {0};
   DSL_uint16_t nData = 0;
   DSL_uint16_t nPcb = 0;
   DSL_int16_t nRmsgi_Q88 = 0, nRmsgiSub_Q88 = 0, nRmsgi10_int = 0,
               nRmsgi10 = 0, nRmsgi10_fract = 0, nNomPsdAbs = 0;
   DSL_int_t nIndexNomPsdAbs = 0,
             nIndexPcb = 0, nIndexRmsgi = 0;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_ActPSD_Get"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   *pData = -901;

   if (nDirection == DSL_DOWNSTREAM)
   {
      nIndexPcb = DSL_CMV_PMD_INDEX_PCB_DS;
      nIndexRmsgi = DSL_CMV_PMD_INDEX_RMSGI_DS;
      nIndexNomPsdAbs = DSL_CMV_PMD_INDEX_NOMINAL_PSD_DS;
   }
   else
   {
      nIndexPcb = DSL_CMV_PMD_INDEX_PCB_US;
      nIndexRmsgi = DSL_CMV_PMD_INDEX_RMSGI_US;
      nIndexNomPsdAbs = DSL_CMV_PMD_INDEX_NOMINAL_PSD_US;
   }

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if (nLineState == DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE ||
       nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      *pData = 0;
      return DSL_SUCCESS;
   }

   /*Get current xTSE octets*/
   DSL_CTX_READ(pContext, nErrCode, xtseCurr, XTSE);

   /*Check for the ADSL1 modes*/
   if ((XTSE[0] & (XTSE_1_03_A_1_NO | XTSE_1_01_A_T1_413 |
                   XTSE_1_02_C_TS_101388 | XTSE_1_05_B_1_NO)) ||
       (XTSE[1] & XTSE_2_01_A_2_NO))
   {
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   /* Get Get Power Cutback*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_PMD,
                 (DSL_uint16_t)nIndexPcb, 1, &nData);
                 
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get PCB for %s!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[nDirection + 1]));
      return nErrCode;
   }
   
   /* TODO: The format of the CMV is nor clear. Assume (dB/Hz)*10*/
   nPcb = (DSL_uint16_t)(nData*10);

   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_PMD,
                 (DSL_uint16_t)nIndexNomPsdAbs, 1, &nData);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get NOMPSDAbs for %s!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[nDirection + 1]));
      return nErrCode;
   }
  
   /* Format (dB/Hz)*10 */
   nNomPsdAbs = (DSL_int16_t)nData;

   if (nDirection == DSL_DOWNSTREAM)
   {
      /* Get DS Excess Margin Reduction*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_EXCESS_MARGIN, 0, 1, &nData);
      
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get EXCESSMGN for %s!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[nDirection + 1]));
         return nErrCode = DSL_ERROR;
      }

      nRmsgiSub_Q88 = (DSL_int16_t) nData;
   }
   else
   {
      nRmsgiSub_Q88 = 0;
   }

   /* Get Fine Gain RMS*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_PMD,
                (DSL_uint16_t)nIndexRmsgi, 1, &nData);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get RMSGI for %s!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[nDirection + 1]));
      return nErrCode;
   }

   nRmsgi_Q88 = ((DSL_int16_t)nData) - nRmsgiSub_Q88;

   /* Convert Fine Gain RMS to (dB/Hz) format*/
   nRmsgi10_int   = (DSL_int16_t)((nRmsgi_Q88*10) / 256);
   nRmsgi10_fract = (DSL_int16_t)(nRmsgi_Q88*10 - nRmsgi10_int*256);

   nRmsgi10 = DSL_ABS_16(nRmsgi10_fract) < 128 ?
              nRmsgi10_int :
              ((nRmsgi10_int < 0) ? (nRmsgi10_int - 1) : (nRmsgi10_int + 1));

   *pData = (DSL_int16_t)((nNomPsdAbs - nPcb) + nRmsgi10);
   
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_ActPSD_Get,retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}


/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_EocLineStatusGet(
   DSL_Context_t *pContext,
   DSL_uint16_t *pHdlcCmd,
   DSL_uint16_t hdlcCmdSize,
   DSL_uint16_t *pHdlcRxBuff,
   DSL_uint16_t hdlcRxBuffSize)
{
   DSL_Error_t nErrCode = DSL_SUCCESS, nRet = DSL_SUCCESS;
   DSL_G997_PowerManagement_t nPowerMode = DSL_G997_PMS_NA;
   DSL_int_t nHdlcRxLen = 0;
   DSL_InteropFeatureConfigData_t iopFeatureConfigData =
      {DSL_FALSE, {DSL_SNRM_REBOOT_AUTOMODE_API, 0}, DSL_FALSE, DSL_FALSE, DSL_FALSE};
   
   if (!pContext || !pHdlcCmd || !pHdlcRxBuff)
   {
      return DSL_ERR_POINTER;
   }

   if ((hdlcCmdSize == 0) || (hdlcRxBuffSize == 0))
   {
      return DSL_ERR_INVALID_PARAMETER;
   }

   DSL_CTX_READ(pContext, nErrCode, interopFeatureConfigData, iopFeatureConfigData);

   /* Check for the turned of FE request*/
   if (iopFeatureConfigData.bFeRequestOff)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - IOP feature turned off FE request!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERR_DEVICE_NO_DATA;
   }
   
   /* In L2 power mode, do not read the OHC related parameters, instead give
      the indication to the calling IOCTL, that the readout fails (just
      return DSL_WRN_INCOMPLETE_RETURN_VALUES).  */
   nRet = DSL_DRV_DANUBE_L3StatusGet(pContext, &nPowerMode);
   if (nRet != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Could not get power mode!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nRet;
   }

   /* Lock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

   /* Only proceed if not in the L2 power mode*/
   if (nPowerMode != DSL_G997_PMS_L2)
   {
      /* Write HDLC*/
      nRet = DSL_DRV_DANUBE_HdlcWrite(
                pContext, DSL_FALSE, (DSL_uint8_t *)pHdlcCmd, (DSL_int_t)hdlcCmdSize);
      
      if (nRet == DSL_SUCCESS)
      {
         DSL_WAIT (1);

         /* Read HDLC*/
         nRet = DSL_DRV_DANUBE_HdlcRead(
                   pContext, DSL_FALSE, (DSL_uint8_t *)pHdlcRxBuff,
                   (DSL_int_t)hdlcRxBuffSize, &nHdlcRxLen);
         
         if (nRet == DSL_SUCCESS)
         {
            if (nHdlcRxLen <= 0)
            {
               DSL_DEBUG( DSL_DBG_WRN,
                  (pContext, "DSL[%02d]: WARNING - HDLC read invalid RxLen!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
               nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
            }
         }
         else
         {
            if (nRet == DSL_ERR_DEVICE_NO_DATA)
            {
               DSL_DEBUG( DSL_DBG_WRN,
                  (pContext, "DSL[%02d]: WARNING - HDLC Read failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
               
               nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
            }
            else
            {
               nErrCode = nRet;
            }
         }
      }
      else
      {
         if (nRet == DSL_ERR_DEVICE_NO_DATA)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: WARNING - HDLC Send failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
         }
         else
         {
            nErrCode = nRet;
         }
      }
   }
   else
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;      
   }

   /* Unlock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);

   return nErrCode;
}

DSL_Error_t DSL_DRV_DANUBE_G997_LineTrainingStatusUpdate(
   DSL_IN DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_LineStatusBackupData_t lineStatusFe = {DSL_FALSE, -32768, -32768, -32768, 0};
   DSL_uint16_t nData16[2] = {0};

   /* Get Line Status backup values*/
   DSL_CTX_READ(pContext, nErrCode, lineStatusFe, lineStatusFe);

   /* Check if the Line Status was already updated*/
   if (lineStatusFe.bValid)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: Line Status backup values are already updated!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_SUCCESS;
   }

   /* Reset Line Status structure according to "special values"*/
   lineStatusFe.LATN = -32768;
   lineStatusFe.SATN = -32768;
   lineStatusFe.SNR  = -32768;
   /* No special value*/
   lineStatusFe.ATTNDR = 0;

   /* Get US LATN value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_LINE_STATUS_US,
                 1, 1, &nData16[0]);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - US LATN get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   lineStatusFe.LATN = (DSL_int16_t)nData16[0];

   /* Get US SATN value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_LINE_STATUS_US,
                 2, 1, &nData16[0]);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - US SATN get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   lineStatusFe.SATN = (DSL_int16_t)nData16[0];

   /* Get US SNR value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_LINE_STATUS_US,
                 3, 1, &nData16[0]);
   
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - US SNR get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   lineStatusFe.SNR = (DSL_int16_t)nData16[0];

   /* Get US ATTNDR value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, DSL_CMV_ADDRESS_INFO_LINE_STATUS_US,
                 4, 2, nData16);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - US ATTNDR get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   lineStatusFe.ATTNDR = ((DSL_uint32_t)(nData16[1]) << 16) | (nData16[0]);

   lineStatusFe.bValid = DSL_TRUE;

   /* Set Line Status backup values*/
   DSL_CTX_WRITE(pContext, nErrCode, lineStatusFe, lineStatusFe);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Line Training Status backup values complete"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Line Training Status backup values:"
      "LATN=%d SATN=%d SNR=%d ATTNDR=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), lineStatusFe.LATN, lineStatusFe.SATN,
      lineStatusFe.SNR, lineStatusFe.ATTNDR));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DANUBE_G997_LineStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS, nRet = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   DSL_G997_LineStatusBackupData_t lineStatusFe = {DSL_FALSE, -32768, -32768, -32768, 0};
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nAddr, nData;
   DSL_uint16_t hdlcCmd[2];
   DSL_uint16_t hdlcRxBuffer[32];

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_LineStatusGet(%s)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   if (pData->nDeltDataType == DSL_DELT_DATA_DIAGNOSTICS)
   {
      return DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
   }
   else if(pData->nDeltDataType != DSL_DELT_DATA_SHOWTIME)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown DELT data type!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   /* Only proceed if the line is in SHOWTIME state.*/
   if ( (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC) )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Function is only available in the SHOWTIME!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }
   
   if (pData->nDirection == DSL_DOWNSTREAM)
   {
      nAddr = DSL_CMV_ADDRESS_INFO_LINE_STATUS_DS;
   }
   else
   {
      nAddr = DSL_CMV_ADDRESS_INFO_LINE_STATUS_US;
   }

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   memset(&pData->data, 0, sizeof(DSL_G997_LineStatusData_t));

   /* Get Line Status backup values*/
   if (pData->nDirection == DSL_UPSTREAM)
   {
      DSL_CTX_READ(pContext, nErrCode, lineStatusFe, lineStatusFe);
   }
   
   /* Reset Line Status structure according to "special values"*/
   pData->data.LATN   = -32768;
   pData->data.SATN   = -32768;
   pData->data.SNR    = -32768;
   /* No special value*/
   pData->data.ATTNDR = 0;
   pData->data.ACTPS  = -901;
   pData->data.ACTATP = -512;

   /* Get ACTATP*/
   nRet = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO, nAddr,6, 1, &nData);
   if (nRet != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get ACTATP for %s!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
         
      return nRet;
   }
   /* Set ACTATP values*/
   pData->data.ACTATP = nData;

   /* Get ACTPS*/
   nRet = DSL_DRV_DANUBE_G997_ActPSD_Get(pContext, pData->nDirection, &(pData->data.ACTPS));
   if (nRet != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Couldn't get ACTPS for %s!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   /* SNR value will be read using CMV for ADSL modes in downstream */
   if (pData->nDirection == DSL_DOWNSTREAM)
   {
      nRet = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
                DSL_CMV_ADDRESS_PLAM_SNRM_0_1DB,
                0, 1, (DSL_uint16_t*)&pData->data.SNR);
      if (nRet != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get SNR for %s!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
         return nRet;
      }
   }
   else
   {
      /* Upstream SNR value for ADSL1 will be read using CMV that reports
         the training SNR value (this is NOT changing during showtime!) */
      if (bAdsl1)
      {
         nRet = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO, nAddr,
                   3, 1, (DSL_uint16_t*)&pData->data.SNR);
         
         if (nRet != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't get SNR for %s!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
            return nRet;
         }
      }
      else
      {
         /* Upstream SNR value for ADSL2/2+ will be read via OHC channel to get
            a value that is always up-to-date */
         hdlcCmd[0] = 0x0181;
         hdlcCmd[1] = 0x23;

         nRet = DSL_DRV_DANUBE_G997_EocLineStatusGet(
                   pContext, hdlcCmd, 4, hdlcRxBuffer, 64);

         if (nRet == DSL_SUCCESS)
         {
            pData->data.SNR = DSL_Le2Cpu(hdlcRxBuffer[1]);
         }
      }
   }

   /* Update Line Status SNR backup value*/
   if (pData->nDirection == DSL_UPSTREAM)
   {
      if (nRet == DSL_SUCCESS)
      {
         /* Update internal SNR backup value*/
         lineStatusFe.SNR = pData->data.SNR;
      }
      else
      {
         if (lineStatusFe.bValid)
         {
            /* Report backup value*/
            pData->data.SNR = lineStatusFe.SNR;
            nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
         }
         else
         {
            /* No valid data to report*/
            return DSL_ERR_DEVICE_NO_DATA;
         }
      }
   }

   /* LATN value will be read using CMV for
      ADSL1   : upstream and downstream
      ADSL2/2+: downstream only */
   if (bAdsl1 || pData->nDirection == DSL_DOWNSTREAM)
   {
      nRet = DSL_DRV_DANUBE_CmvRead(
                pContext, DSL_CMV_GROUP_INFO, nAddr,
                1, 1, (DSL_uint16_t*)&pData->data.LATN);
      if (nRet != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get LATN for %s!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
         return nRet;
      }
   }
   else
   {
      if (nRet == DSL_SUCCESS)
      {
         /* In case of ADSL2/2+ upstream value for LATN will be requested via OHC
            channel to get a value that is always up-to-date */
         hdlcCmd[0] = 0x0181;
         hdlcCmd[1] = 0x21;

         nRet = DSL_DRV_DANUBE_G997_EocLineStatusGet(
                   pContext, hdlcCmd, 4, hdlcRxBuffer, 64);

         if (nRet == DSL_SUCCESS)
         {
            pData->data.LATN = DSL_Le2Cpu(hdlcRxBuffer[1]);      
         }
      }
   }

   /* Update Line Status LATN backup value*/
   if (pData->nDirection == DSL_UPSTREAM)
   {
      if (nRet == DSL_SUCCESS)
      {
         /* Update internal LATN backup value*/
         lineStatusFe.LATN = pData->data.LATN;
      }
      else
      {
         if (lineStatusFe.bValid)
         {
            /* Report backup value*/
            pData->data.LATN = lineStatusFe.LATN;
            nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
         }
         else
         {
            /* No valid data to report*/
            return DSL_ERR_DEVICE_NO_DATA;
         }
      }
   }

   /* SATN value will be read using CMV for
      ADSL1   : upstream and downstream
      ADSL2/2+: downstream only */
   if (bAdsl1 || pData->nDirection == DSL_DOWNSTREAM)
   {
      nRet = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO, nAddr,
                2, 1, (DSL_uint16_t*)&pData->data.SATN);
                
      if (nRet != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get SATN for %s!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
         return nRet;
      }
   }
   else
   {
      if (nRet == DSL_SUCCESS)
      {
         /* In case of ADSL2/2+ upstream value for SATN will be requested via OHC
            channel to get a value that is always up-to-date */
         hdlcCmd[0] = 0x0181;
         hdlcCmd[1] = 0x22;

         nRet = DSL_DRV_DANUBE_G997_EocLineStatusGet(
                   pContext, hdlcCmd, 4, hdlcRxBuffer, 64);

         if (nRet == DSL_SUCCESS)
         {
            pData->data.SATN = DSL_Le2Cpu(hdlcRxBuffer[1]);
         }
      }
   }

   /* Update Line Status SATN backup value*/
   if (pData->nDirection == DSL_UPSTREAM)
   {
      if (nRet == DSL_SUCCESS)
      {
         /* Update internal SATN backup value*/
         lineStatusFe.SATN = pData->data.SATN;
      }
      else
      {
         if (lineStatusFe.bValid)
         {
            /* Report backup value*/
            pData->data.SATN = lineStatusFe.SATN;
            nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
         }
         else
         {
            /* No valid data to report*/
            return DSL_ERR_DEVICE_NO_DATA;
         }
      }
   }

   /* Get ATTNDR*/
   if (bAdsl1 || pData->nDirection == DSL_DOWNSTREAM)
   {
      /* Check for the ADSL1 mode*/
      if (bAdsl1)
      {
         DSL_uint32_t nAttAggregateDataRate = 0;
         DSL_boolean_t bDualLatencyActive = DSL_FALSE;

         /* Get AttDrAggregate parameter*/
         nRet = DSL_DRV_DANUBE_AttAggregateDataRateGet(
                   pContext, pData->nDirection, &nAttAggregateDataRate);

         if (nRet != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - %s AttAggregateDataRate get failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
            return nRet;
         }

         /* Check if both fast and interleaved paths are active*/
         nRet = DSL_DRV_DANUBE_DualLatencyStatusGet(
                    pContext, pData->nDirection, &bDualLatencyActive);

         if (nRet != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext,"DSL[%02d]: ERROR - %s Dual Latency Status get failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), pData->nDirection == DSL_UPSTREAM ? "US" : "DS"));
            return nRet;
         }

         /* ATTNDR = nAttAggregateDataRate - Framing_Overhead*/
         pData->data.ATTNDR = (nAttAggregateDataRate - (bDualLatencyActive ? 2 : 1)*8)*4000;
      }
      else
      {
         nRet = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO, nAddr,
                   4, 2, (DSL_uint16_t*)&pData->data.ATTNDR);
      
         if (nRet != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't get ATTNDR for %s!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));
            return nRet;
         }
         else
         {
            pData->data.ATTNDR = ((pData->data.ATTNDR & 0xFFFF) << 16) |
               (pData->data.ATTNDR & 0xFFFF0000) >> 16;
         }
      }
   }
   else
   {
      if (nRet == DSL_SUCCESS)
      {
         /* Upstream ATTNDR value for ADSL2/2+ will be read via OHC channel to get
            a value that is always up-to-date */
         hdlcCmd[0] = 0x0181;
         hdlcCmd[1] = 0x24;

         nRet = DSL_DRV_DANUBE_G997_EocLineStatusGet(
                   pContext, hdlcCmd, 4, hdlcRxBuffer, 64);

         if (nRet == DSL_SUCCESS)
         {
            pData->data.ATTNDR = (((DSL_uint32_t)DSL_Le2Cpu(hdlcRxBuffer[1])) << 16 )|
                                  ((DSL_uint32_t)DSL_Le2Cpu(hdlcRxBuffer[2]));
         }
      }
   }

   /* Update Line Status ATTNDR backup value*/
   if (pData->nDirection == DSL_UPSTREAM)
   {
      if (nRet == DSL_SUCCESS)
      {
         /* Update internal ATTNDR backup value*/
         lineStatusFe.ATTNDR = pData->data.ATTNDR;
      }
      else
      {
         if (lineStatusFe.bValid)
         {
            /* Report backup value*/
            pData->data.ATTNDR = lineStatusFe.ATTNDR;
            nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
         }
         else
         {
            /* No valid data to report*/
            return DSL_ERR_DEVICE_NO_DATA;
         }
      }
   }

   /* Update Line Status internal backup values*/
   if ((pData->nDirection == DSL_UPSTREAM) && (nErrCode == DSL_SUCCESS))
   {
      lineStatusFe.bValid = DSL_TRUE;
      DSL_CTX_WRITE(pContext, nRet, lineStatusFe, lineStatusFe);

      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Line Status backup values complete"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Line Status backup values:"
         "LATN=%d SATN=%d SNR=%d ATTNDR=%d"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), lineStatusFe.LATN, lineStatusFe.SATN,
         lineStatusFe.SNR, lineStatusFe.ATTNDR));
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_LineStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DEV_G997_LineStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_LineStatusGet(%s)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DBG_PRN_DIR[pData->nDirection + 1]));

   nErrCode = DSL_DRV_DANUBE_G997_LineStatusGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_LineStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DEV_G997_LineInventoryGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventory_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_LineInventoryGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pData->nDirection == DSL_NEAR_END)
   {
      DSL_CTX_READ(pContext, nErrCode, lineInventoryNe, pData->data);   
   }
   else
   {
      DSL_CTX_READ(pContext, nErrCode, lineInventoryFe, pData->data);      
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_LineInventoryGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DEV_G997_LineInventorySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventoryNe_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_LineInventorySet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_DRV_MUTEX_LOCK(pContext->dataMutex);

   /* Set VersionNumber*/
   memcpy(
      pContext->lineInventoryNe.VersionNumber,
      pData->data.VersionNumber,
      DSL_G997_LI_MAXLEN_VERSION);

   /* Set SystemVendorID*/
   memcpy(
      pContext->lineInventoryNe.SystemVendorID,
      pData->data.SystemVendorID,
      DSL_G997_LI_MAXLEN_VENDOR_ID);

   /* Set SerialNumber*/
   memcpy(
      pContext->lineInventoryNe.SerialNumber,
      pData->data.SerialNumber,
      DSL_G997_LI_MAXLEN_SERIAL);

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_LineInventorySet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_LineTransmissionStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineTransmissionStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_LineTransmissionStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   /* Only proceed if the specified line is in SHOWTIME state.*/
   if ( nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC ||
        nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC )
   {
      pData->data.nLineTransmissionStatus = DSL_G997_LINE_TRANSMISSION_AVAILABLE;
   }
   else
   {
      pData->data.nLineTransmissionStatus = DSL_G997_LINE_TRANSMISSION_NOT_AVAILABLE;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_LineTransmissionStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_ChannelStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_ChannelStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   /* Only proceed if the line is in SHOWTIME state.*/
   if ( (nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC) ||
        (nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC) )
   {
      nErrCode = DSL_DRV_DANUBE_ChannelStatusGet(
                    pContext, pData->nChannel,
                    pData->nDirection, &pData->data);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Function is only available in the SHOWTIME!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_ChannelStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_LastStateTransmittedGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LastStateTransmitted_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DEV_G997_LastStateTransmittedGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO,
                 pData->nDirection == DSL_DOWNSTREAM ?
                 DSL_CMV_ADDRESS_INFO_DELT_PREV_STATS_DS:
                 DSL_CMV_ADDRESS_INFO_DELT_PREV_STATS_US,
                 1, 1, &pData->data.nLastStateTransmitted);

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DEV_G997_LastStateTransmittedGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DANUBE_G997_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN const DSL_uint8_t nChannel,
   DSL_OUT DSL_G997_FramingParameterStatusData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nIdx = 0;
   DSL_uint16_t nAddrNFEC = 0, nAddrRFEC, nAddrLSYMB, nAddrINTLVDEPTH;
   DSL_boolean_t bNFECAddrWr = DSL_FALSE;
   DSL_LatencyPath_t nLp = DSL_LATENCY_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_FramingParameterStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Update channel status for the case if it was not apdated yet */
   nErrCode = DSL_DRV_DANUBE_ChannelStatusUpdate(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][nChannel], nLp);
   
   pData->nLPATH = (nLp == DSL_LATENCY_IP_LP0 ? 0 :
                           nLp == DSL_LATENCY_FP_LP1 ? 1 : 0);

   if (nDirection == DSL_UPSTREAM)
   {
      if (nLp == DSL_LATENCY_IP_LP0)
      {
         nAddrNFEC = DSL_CMV_ADDRESS_RATE_DATA_NFEC_US_LP0;
      }
      else if (nLp == DSL_LATENCY_FP_LP1)
      {
         nAddrNFEC = DSL_CMV_ADDRESS_RATE_DATA_NFEC_US_LP1;
      }
      else
      {
         bNFECAddrWr = DSL_TRUE;
      }
      nAddrRFEC = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_RP;
      nAddrLSYMB = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_LP;
      nAddrINTLVDEPTH = DSL_CMV_ADDRESS_CNFG_DATA_INTLVDEPTH_US;
   }
   else
   {
      if (nLp == DSL_LATENCY_IP_LP0)
      {
         nAddrNFEC = DSL_CMV_ADDRESS_RATE_DATA_NFEC_DS_LP0;
      }
      else if (nLp == DSL_LATENCY_FP_LP1)
      {
         nAddrNFEC = DSL_CMV_ADDRESS_RATE_DATA_NFEC_DS_LP1;
      }
      else
      {
         bNFECAddrWr = DSL_TRUE;
      }
      nAddrRFEC = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_RP;
      nAddrLSYMB = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_LP;
      nAddrINTLVDEPTH = DSL_CMV_ADDRESS_CNFG_DATA_INTLVDEPTH_DS;
   }

   if (bNFECAddrWr == DSL_FALSE)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_RATE,
         nAddrNFEC, nChannel, 1, &pData->nNFEC);
      if (nErrCode != DSL_SUCCESS)
      {
         return nErrCode;
      }
   }
   else
   {
      pData->nNFEC = 0;
   }

   nIdx = pData->nLPATH;

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrRFEC, nIdx, 1, &pData->nRFEC);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrLSYMB, nIdx, 1, &pData->nLSYMB);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrINTLVDEPTH, nIdx, 1, &pData->nINTLVDEPTH);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   pData->nINTLVBLOCK = pData->nNFEC;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_FramingParameterStatusGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
DSL_Error_t DSL_DRV_DEV_G997_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN const DSL_uint8_t nChannel,
   DSL_OUT DSL_G997_FramingParameterStatusData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_FramingParameterStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   /* Get current line state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   /* Only proceed if the line is in SHOWTIME state.*/
   if ( (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
        (nCurrentState != DSL_LINESTATE_SHOWTIME_NO_SYNC) )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Function is only available in the SHOWTIME!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   nErrCode = DSL_DRV_DANUBE_G997_FramingParameterStatusGet(
                 pContext, nDirection, nChannel, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_FramingParameterStatusGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/

#ifdef INCLUDE_DSL_DELT
/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltHlinScaleGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltHlinScaleData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_DeltHlinScaleGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (nDeltDataType == DSL_DELT_DATA_SHOWTIME)
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
   }
   else if (nDeltDataType == DSL_DELT_DATA_DIAGNOSTICS)
   {
      if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERROR;
      }

      if (pContext->DELT != DSL_NULL)
      {
         memcpy(pData,
            (nDirection == DSL_UPSTREAM) ?
            &pContext->DELT->hlinScaleDataUs :
            &pContext->DELT->hlinScaleDataDs,
            sizeof(DSL_G997_DeltHlinScaleInternalData_t));
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;
      }

      DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown DELT data type!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_DeltHlinScaleGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltHlinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltHlinData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_ComplexNumber_t *pIntData = DSL_NULL;
   DSL_uint16_t nNumData = 0, nMeasurementTime = 0;
   DSL_uint8_t nGroupSize;


   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_DeltHlinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (nDeltDataType == DSL_DELT_DATA_SHOWTIME)
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
   }
   else if (nDeltDataType == DSL_DELT_DATA_DIAGNOSTICS)
   {
      if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERROR;
      }

      if (pContext->DELT != DSL_NULL)
      {
         if (nDirection == DSL_DOWNSTREAM)
         {
            pIntData = pContext->DELT->hlinDataDs.deltHlin.nSCGComplexData;
            nGroupSize = pContext->DELT->hlinDataDs.nGroupSize;
            nNumData = pContext->DELT->hlinDataDs.deltHlin.nNumData;
            nMeasurementTime = pContext->DELT->hlinDataDs.nMeasurementTime;
         }
         else
         {
            pIntData = pContext->DELT->hlinDataUs.deltHlin.nSCGComplexData;
            nGroupSize = pContext->DELT->hlinDataUs.nGroupSize;
            nNumData = pContext->DELT->hlinDataUs.deltHlin.nNumData;
            nMeasurementTime = pContext->DELT->hlinDataUs.nMeasurementTime;
         }

         /* if actual group size != 1, values should be spread */
         if (nGroupSize != 1)
         {
            nErrCode = DSL_DRV_DANUBE_G997_DeltValuesSpread(
                          0x2, nGroupSize, nNumData,
                          pIntData, pData->deltHlin.nNSCComplexData);

            if (nErrCode == DSL_SUCCESS)
            {
               pData->deltHlin.nNumData = (DSL_uint16_t)(nNumData * nGroupSize);
               pData->nGroupSize        = 1;
               pData->nMeasurementTime  = nMeasurementTime;
            }
         }
         else
         {
            memcpy(
               pData,
               nDirection == DSL_DOWNSTREAM ?
               (DSL_void_t*)&(pContext->DELT->hlinDataDs) :
               (DSL_void_t*)&(pContext->DELT->hlinDataUs),
               nDirection == DSL_DOWNSTREAM ?
               sizeof(DSL_G997_DeltHlinInternalDsData_t):
               sizeof(DSL_G997_DeltHlinInternalUsData_t));
         }
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;
      }

      DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown DELT data type!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_DeltHlinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltHlogGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltHlogData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t *pIntData = DSL_NULL, nNumData = 0, nMeasurementTime = 0;
   DSL_uint8_t nGroupSize;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_DeltHlogGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (nDeltDataType == DSL_DELT_DATA_SHOWTIME)
   {
      /* Get current line state*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

      /* Only proceed if the specified line is in SHOWTIME state.*/
      if ( (nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC  ||
            nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         if (nDirection == DSL_DOWNSTREAM)
         {
            DSL_G997_DeltHlogData_t HlogData;

            memset(&HlogData, 0x0, sizeof(DSL_G997_DeltHlogData_t));

            /* Get SHOWTIME Hlog values*/
            nErrCode = DSL_DRV_DANUBE_G997_DeltHlogGet(
                          pContext, nDirection, &HlogData);
            if (nErrCode != DSL_SUCCESS)
            {
               DSL_DEBUG(DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Showtime Hlog get failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
               return nErrCode;
            }

            /* if actual group size != 1, values should be spread */
            if (HlogData.nGroupSize != 1)
            {
               nErrCode = DSL_DRV_DANUBE_G997_DeltValuesSpread(
                             0x1, HlogData.nGroupSize, HlogData.deltHlog.nNumData,
                             HlogData.deltHlog.nNSCData, pData->deltHlog.nNSCData);

               if (nErrCode == DSL_SUCCESS)
               {
                  pData->deltHlog.nNumData = 
                     (DSL_uint16_t)(HlogData.deltHlog.nNumData * HlogData.nGroupSize);
                  pData->nGroupSize        = 1;
                  pData->nMeasurementTime  = HlogData.nMeasurementTime;
               }
            }
            else
            {
               /* No spread needed, copy data*/
               memcpy(pData, &HlogData, sizeof(DSL_G997_DeltHlogData_t));
            }
         }
         else
         {
            /* Get HLOG US Showtime values*/
            nErrCode = DSL_DRV_DANUBE_G997_ShowtimeHlogUsGet(pContext,pData);
         }
      }
      else
      {
         nErrCode = DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;      
      }
   }
   else if (nDeltDataType == DSL_DELT_DATA_DIAGNOSTICS)
   {
      if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERROR;
      }

      if (pContext->DELT != DSL_NULL)
      {
         if (nDirection == DSL_DOWNSTREAM)
         {
            pIntData = pContext->DELT->hlogDataDs.deltHlog.nSCGData;
            nGroupSize = pContext->DELT->hlogDataDs.nGroupSize;
            nNumData = pContext->DELT->hlogDataDs.deltHlog.nNumData;
            nMeasurementTime = pContext->DELT->hlogDataDs.nMeasurementTime;
         }
         else
         {
            pIntData = pContext->DELT->hlogDataUs.deltHlog.nSCGData;
            nGroupSize = pContext->DELT->hlogDataUs.nGroupSize;
            nNumData = pContext->DELT->hlogDataUs.deltHlog.nNumData;
            nMeasurementTime = pContext->DELT->hlogDataUs.nMeasurementTime;
         }

         /* if actual group size != 1, values should be spread */
         if (nGroupSize != 1)
         {
            nErrCode = DSL_DRV_DANUBE_G997_DeltValuesSpread(
                          0x1, nGroupSize, nNumData,
                          pIntData, pData->deltHlog.nNSCData);

            if (nErrCode == DSL_SUCCESS)
            {
               pData->deltHlog.nNumData = (DSL_uint16_t)(nNumData * nGroupSize);
               pData->nGroupSize        = 1;
               pData->nMeasurementTime  = nMeasurementTime;
            }
         }
         else
         {
            memcpy(
               pData,
               nDirection == DSL_DOWNSTREAM ?
               (DSL_void_t*)&(pContext->DELT->hlogDataDs) :
               (DSL_void_t*)&(pContext->DELT->hlogDataUs),
               nDirection == DSL_DOWNSTREAM ?
               sizeof(DSL_G997_DeltHlogInternalDsData_t):
               sizeof(DSL_G997_DeltHlogInternalUsData_t));
         }
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;
      }

      DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown DELT data type!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_DeltHlogGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltQLNGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltQlnData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nNumData = 0, nMeasurementTime = 0;
   DSL_uint8_t *pIntData = DSL_NULL, nGroupSize = 0;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_DeltQLNGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (nDeltDataType == DSL_DELT_DATA_SHOWTIME)
   {
      /* Get current line state*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

      /* Only proceed if the specified line is in SHOWTIME state.*/
      if ( (nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC  ||
            nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         if (nDirection == DSL_DOWNSTREAM)
         {
            DSL_G997_DeltQlnData_t QlnData;
         
            /* Only proceed if the specified line is in SHOWTIME state.*/
            if ( (nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC  ||
                  nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
            {
               memset(&QlnData, 0x0, sizeof(DSL_G997_DeltQlnData_t));         

               /* Get SHOWTIME Qln values*/
               nErrCode = DSL_DRV_DANUBE_G997_DeltQLNGet(
                             pContext, nDirection, &QlnData);
               if (nErrCode != DSL_SUCCESS)
               {
                  DSL_DEBUG(DSL_DBG_ERR,
                     (pContext, "DSL[%02d]: ERROR - Showtime QLN get failed!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));
                  return nErrCode;
               }

               /* if actual group size != 1, values should be spread */
               if (QlnData.nGroupSize != 1)
               {
                  nErrCode = DSL_DRV_DANUBE_G997_DeltValuesSpread(
                                0x0, QlnData.nGroupSize, QlnData.deltQln.nNumData,
                                QlnData.deltQln.nNSCData, pData->deltQln.nNSCData);

                  if (nErrCode == DSL_SUCCESS)
                  {
                     pData->deltQln.nNumData = 
                        (DSL_uint16_t)(QlnData.deltQln.nNumData * QlnData.nGroupSize);
                     pData->nGroupSize        = 1;
                     pData->nMeasurementTime  = QlnData.nMeasurementTime;
                  }
               }
               else
               {
                  /* No spread needed, copy data*/
                  memcpy(pData, &QlnData, sizeof(DSL_G997_DeltQlnData_t));
               }
            }
         }
         else
         {
            nErrCode = DSL_DRV_DANUBE_G997_ShowtimeQlnUsGet(pContext, pData);
         }
      }
      else
      {
         nErrCode = DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
      }
   }
   else if (nDeltDataType == DSL_DELT_DATA_DIAGNOSTICS)
   {
      if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERROR;
      }

      if (pContext->DELT != DSL_NULL)
      {
         if (nDirection == DSL_DOWNSTREAM)
         {
            pIntData = pContext->DELT->qlnDataDs.deltQln.nSCGData;
            nGroupSize = pContext->DELT->qlnDataDs.nGroupSize;
            nNumData = pContext->DELT->qlnDataDs.deltQln.nNumData;
            nMeasurementTime = pContext->DELT->qlnDataDs.nMeasurementTime;
         }
         else
         {
            pIntData = pContext->DELT->qlnDataUs.deltQln.nSCGData;
            nGroupSize = pContext->DELT->qlnDataUs.nGroupSize;
            nNumData = pContext->DELT->qlnDataUs.deltQln.nNumData;
            nMeasurementTime = pContext->DELT->qlnDataUs.nMeasurementTime;
         }

         /* if actual group size != 1, values should be spread */
         if (nGroupSize != 1)
         {
            nErrCode = DSL_DRV_DANUBE_G997_DeltValuesSpread(
                          0x0, nGroupSize, nNumData,
                          pIntData, pData->deltQln.nNSCData);

            if (nErrCode == DSL_SUCCESS)
            {
               pData->deltQln.nNumData  = (DSL_uint16_t)(nNumData * nGroupSize);
               pData->nGroupSize        = 1;
               pData->nMeasurementTime  = nMeasurementTime;
            }
         }
         else
         {
            memcpy(
               pData,
               nDirection == DSL_DOWNSTREAM ?
               (DSL_void_t*)&(pContext->DELT->qlnDataDs) :
               (DSL_void_t*)&(pContext->DELT->qlnDataUs),
               nDirection == DSL_DOWNSTREAM ?
               sizeof(DSL_G997_DeltQlnInternalDsData_t):
               sizeof(DSL_G997_DeltQlnInternalUsData_t));
         }
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;
      }

      DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown DELT data type!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_DeltQLNGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltSNRGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltSnrData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   DSL_G997_SnrAllocationNsc_t *pShowtimeSnr = DSL_NULL;
   
   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_DeltSNRGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (nDeltDataType == DSL_DELT_DATA_SHOWTIME)
   {
      /* Get current line state*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

      /* Only proceed if the specified line is in SHOWTIME state.*/
      if ( (nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC  ||
            nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         pShowtimeSnr = (DSL_G997_SnrAllocationNsc_t*)DSL_DRV_Malloc(
                                          sizeof(DSL_G997_SnrAllocationNsc_t));

         if (pShowtimeSnr == DSL_NULL)
         {
            DSL_DEBUG(DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Showtime SNR memory allocation failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            return DSL_ERR_MEMORY;
         }

         pShowtimeSnr->nDirection = nDirection;
         
         /* Get SHOWTIME SNR values*/
         nErrCode = DSL_DRV_DEV_G997_SnrAllocationNscGet(pContext, pShowtimeSnr);

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Showtime SNR get failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
         else
         {
            /* Always 1 in ADSL*/
            pData->nGroupSize = 0x1;
            /* Not available*/
            pData->nMeasurementTime = 0;
            memcpy(&(pData->deltSnr), &(pShowtimeSnr->data.snrAllocationNsc),
               sizeof(DSL_G997_NSCData8_t));
         }
      }
      else
      {
         nErrCode = DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
      }
   }
   else if (nDeltDataType == DSL_DELT_DATA_DIAGNOSTICS)
   {
      if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERROR;
      }

      if (pContext->DELT != DSL_NULL)
      {
         if (nDirection == DSL_UPSTREAM)
         {
            DSL_DEBUG( DSL_DBG_MSG,
               (pContext, "DSL[%02d]: Snr data info - nMeasurementTime = %d,"
               "nNumData = %d (%p, %p)"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext),
               pContext->DELT->snrDataUs.nMeasurementTime,
               pContext->DELT->snrDataUs.deltSnr.nNumData,
               &pContext->DELT->snrDataUs.nMeasurementTime,
               &pContext->DELT->snrDataUs.deltSnr.nNumData));
         }
         else
         {
            DSL_DEBUG( DSL_DBG_MSG,
               (pContext, "DSL[%02d]: Snr data info - nMeasurementTime = %d,"
               "nNumData = %d (%p, %p)"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext),
               pContext->DELT->snrDataDs.nMeasurementTime,
               pContext->DELT->snrDataDs.deltSnr.nNumData,
               &pContext->DELT->snrDataDs.nMeasurementTime,
               &pContext->DELT->snrDataDs.deltSnr.nNumData));
         }

         memcpy(pData,
            (nDirection == DSL_UPSTREAM) ?
            (DSL_void_t*)&(pContext->DELT->snrDataUs) :
            (DSL_void_t*)&(pContext->DELT->snrDataDs),
            (nDirection == DSL_UPSTREAM) ?
            sizeof(DSL_G997_DeltSnrInternalUsData_t) :
            sizeof(DSL_G997_DeltSnrInternalDsData_t));
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;
      }

      DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown DELT data type!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   /* Free Showtime SNR data*/
   if (pShowtimeSnr != DSL_NULL)
   {
      DSL_DRV_MemFree(pShowtimeSnr);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_DeltSNRGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_DELT*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATE_FORCED_TRIGGER
*/
DSL_Error_t DSL_DRV_DEV_G997_PowerManagementStateForcedTrigger(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_G997_PowerManagementStateForcedTriggerData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_PowerManagementStatusData_t nPMStatus = {DSL_G997_PMS_NA};
   DSL_int_t nAttempt = 0, i;
   DSL_uint16_t nVal;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_G997_PowerManagementStateForcedTrigger"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get Actual Power Management Status from the DSL CPE context*/
   DSL_CTX_READ(pContext, nErrCode, powerMgmtStatus, nPMStatus);

   switch(pData->nPowerManagementState)
   {
   case DSL_G997_PMSF_L3_TO_L0:
      if(nPMStatus.nPowerManagementStatus == DSL_G997_PMS_L3)
      {
         /* Reset Flag*/
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bPowerManagementL3Forced, DSL_FALSE);
      }
      else
      {
         DSL_DEBUG( DSL_DBG_WRN,
            (pContext,
            "DSL[%02d]: PMSF L3-L0 requested but not feasible (actual mode: L%d)." DSL_DRV_CRLF ,
            DSL_DEV_NUM(pContext),
            (DSL_int_t)nPMStatus.nPowerManagementStatus));
         nErrCode = DSL_ERROR;
      }
      break;
   case DSL_G997_PMSF_L0_TO_L2:
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext,
         "DSL[%02d]: PMSF L0-L2 requested but not supported." DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      nErrCode = DSL_ERR_NOT_SUPPORTED_BY_DEVICE;
      break;
   case DSL_G997_PMSF_LX_TO_L3:
      if (nPMStatus.nPowerManagementStatus != DSL_G997_PMS_L3)
      {
         /* retry it DSL_LX_TO_L3_ATTEMPT_COUNT times */
         for (nAttempt = 0; nAttempt < DSL_LX_TO_L3_ATTEMPT_COUNT; nAttempt++)
         {
            /* read previous data in the L3 request CMV */
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNTL,
               DSL_CMV_ADDRESS_CNTL_L3_REQUEST, 0, 1, &nVal);
            if (nErrCode == DSL_SUCCESS)
            {
               nVal |= 1;
               nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_CNTL,
                  DSL_CMV_ADDRESS_CNTL_L3_REQUEST, 0, 1, &nVal);

               DSL_DEBUG(DSL_DBG_MSG,
                  (pContext, "DSL[%02d]: DEBUG - "
                  "DSL_DEV_G997_PowerManagementStateForcedTrigger: "
                  "CMV DSL_CMV_ADDRESS_CNTL_L3_REQUEST,0,1 write %hu"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), nVal));

               nErrCode = DSL_ERROR;

               for (i = 0; i < DSL_LX_TO_L3_TIMEOUT*10; i++)
               {
                  /*$$ ND: sleep for 100 msec, is it enough? */
                  DSL_WAIT(100);
                  /* read L3 request status */
                  nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                     DSL_CMV_ADDRESS_STAT_L3_REQUEST, 0, 1, &nVal);
                  DSL_DEBUG(DSL_DBG_MSG,
                     (pContext, "DSL[%02d]: DEBUG - "
                     "DSL_DEV_G997_PowerManagementStateForcedTrigger: "
                     "CMV DSL_CMV_ADDRESS_STAT_L3_REQUEST,0,1 read %hu"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext), nVal));

                  if (nErrCode != DSL_SUCCESS)
                  {
                     nErrCode = DSL_ERR_MSG_EXCHANGE;
                     break;
                  }
                  if ((nVal & 0x3) == 0)
                  {
                     continue;
                  }
                  else if ((nVal & 0x3) == 1)
                  {
                     nErrCode = DSL_ERR_L3_REJECTED;
                  }
                  else if ((nVal & 0x3) == 2)
                  {
                     nErrCode = DSL_SUCCESS;
                     break;
                  }
                  else
                  {
                     /* read L3 request failure reason */
                     nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                        DSL_CMV_ADDRESS_STAT_L3_FAILURE_REASON, 0, 1, &nVal);
                     DSL_DEBUG(DSL_DBG_MSG,
                        (pContext, "DSL[%02d]: DEBUG - "
                        "DSL_DEV_G997_PowerManagementStateForcedTrigger: "
                        "CMV DSL_CMV_ADDRESS_STAT_L3_FAILURE_REASON,0,1 read %hu"DSL_DRV_CRLF,
                        DSL_DEV_NUM(pContext), nVal));

                     if (nErrCode != DSL_SUCCESS)
                     {
                        nErrCode = DSL_ERR_MSG_EXCHANGE;
                        break;
                     }
                     if (((nVal >> 4) & 0x15) == 0x5)
                     {
                        nErrCode = DSL_ERR_L3_NOT_IN_L0;
                     }
                     else if (((nVal >> 4) & 0x15) == 0x9)
                     {
                        nErrCode = DSL_ERR_L3_TIMED_OUT;
                     }
                     else
                     {
                        nErrCode = DSL_ERR_L3_UNKNOWN_FAILURE;
                     }
                  }
               }

               if (nErrCode != DSL_ERROR)
               {
                  break;
               }
            }
         }

         if (nErrCode == DSL_ERROR)
         {
            nErrCode = DSL_ERR_L3_TIMED_OUT;
         }

         if (nErrCode == DSL_SUCCESS)
         {
            /*Set flag*/
            DSL_DEBUG(DSL_DBG_MSG,
               (pContext, "DSL[%02d]: DEBUG - "
               "DSL_DEV_G997_PowerManagementStateForcedTrigger: "
               "Set L3 power management status"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nVal));
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bPowerManagementL3Forced, DSL_TRUE);

            /* Trigger restart sequence*/
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootRestart, DSL_TRUE);
         }
      }
      else
      {
         DSL_DEBUG( DSL_DBG_WRN,
            (pContext,
            "DSL[%02d]: PMSF Lx-L3 requested but not feasible (actual mode: L%d)." DSL_DRV_CRLF ,
            DSL_DEV_NUM(pContext),
            (DSL_int_t)nPMStatus.nPowerManagementStatus));
         nErrCode = DSL_ERROR;
      }
      break;
   default:
      break;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_G997_PowerManagementStateForcedTrigger, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_DELT
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltValuesSpread(
   DSL_uint8_t dataType,
   DSL_uint8_t nGroupSize,
   DSL_uint16_t nOrigDataSize,
   DSL_void_t *pOrigData,
   DSL_void_t *pSpreadData)
{
   DSL_int_t nMsgIdx = 0, i = 0;
   
   if ( (pOrigData == DSL_NULL) || (pSpreadData == DSL_NULL))
   {
      return DSL_ERROR;
   }

   switch(dataType)
   {
   /* 8-bit*/
   case 0:
   /* 16-bit*/
   case 1:
   /* complex*/
   case 2:
      break;
   default:
      return DSL_ERROR;
   }

   /* check the sizes */
   if (nOrigDataSize * nGroupSize > DSL_MAX_NSC)
   {
      return DSL_ERROR;
   }
   else
   {
      if (dataType == 0x0)
      {
         ((DSL_uint8_t*)pSpreadData)[0] = ((DSL_uint8_t*)pOrigData)[0];
      }
      else if (dataType == 0x1)
      {
         ((DSL_uint16_t*)pSpreadData)[0] = ((DSL_uint16_t*)pOrigData)[0];
      }
      else
      {
         ((DSL_G997_ComplexNumber_t*)pSpreadData)[0] = ((DSL_G997_ComplexNumber_t*)pOrigData)[0];
      }
      
      for (nMsgIdx = 1; nMsgIdx < nOrigDataSize * nGroupSize; nMsgIdx += nGroupSize)
      {
         for (i = 0; i < nGroupSize; i++)
         {
            if (dataType == 0x0)
            {
               ((DSL_uint8_t*)pSpreadData)[nMsgIdx + i] = ((DSL_uint8_t*)pOrigData)[(nMsgIdx-1)/2];
            }
            else if (dataType == 0x1)
            {
               ((DSL_uint16_t*)pSpreadData)[nMsgIdx + i] = ((DSL_uint16_t*)pOrigData)[(nMsgIdx-1)/2];
            }
            else
            {
               ((DSL_G997_ComplexNumber_t*)pSpreadData)[nMsgIdx + i] =
                  ((DSL_G997_ComplexNumber_t*)pOrigData)[(nMsgIdx-1)/2];
            }
         }
      }
   }

   return DSL_SUCCESS;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltHlinScaleGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal;
   DSL_uint16_t nAddr;
   DSL_uint16_t *pData;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_DeltHlinScaleGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (pContext->bLoopDiagnosticsCompleted != DSL_TRUE)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - call is only possible "
         "after diagnostic is complete"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      nErrCode |= DSL_ERROR;
   }
   DSL_CHECK_ERR_CODE();

   if (nDirection == DSL_DOWNSTREAM)
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_PREV_STATS_DS;

      pData = &pContext->DELT->hlinScaleDataDs.nDeltHlinScale;
   }
   else
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_PREV_STATS_US;

      pData = &pContext->DELT->hlinScaleDataUs.nDeltHlinScale;
   }

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      nAddr, DSL_CMV_DELT_PREV_STATS_INDEX_HLIN_SC, 1, &nVal);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CMV "
         "DSL_CMV_ADDRESS_INFO_DELT_PREV_STATS_US(DS) read fail!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      *pData = nVal;
      DSL_DEBUG( DSL_DBG_MSG, (pContext, "DSL[%02d]: ERROR - CMV "
         "DSL_CMV_ADDRESS_INFO_DELT_PREV_STATS, nDirection=%d, "
         "Value=%d!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nDirection, nVal));
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_DeltHlinScaleGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltHlinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nIdx = 0, nSize = 6, nMsgIdx;
   DSL_uint16_t buf[12];
   DSL_uint16_t nSizeMax, nAddr;
   DSL_boolean_t bAdsl2p = DSL_FALSE;
   /* pointers data should be saved to */
   DSL_G997_ComplexNumber_t *pData;
   DSL_uint16_t *pnMeasurementTime, *pnNumData;
   DSL_uint8_t *pnGroupSize;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_DeltHlinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (pContext->bLoopDiagnosticsCompleted != DSL_TRUE)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - "
         "call is only possible after diagnostic is complete!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      nErrCode |= DSL_ERROR;
   }
   DSL_CHECK_ERR_CODE();

   bAdsl2p  = pContext->pDevCtx->data.bAdsl2p;

   if (nDirection == DSL_DOWNSTREAM)
   {
      pData = pContext->DELT->hlinDataDs.deltHlin.nSCGComplexData;
      pnMeasurementTime = &pContext->DELT->hlinDataDs.nMeasurementTime;
      pnGroupSize = &pContext->DELT->hlinDataDs.nGroupSize;
      pnNumData = &pContext->DELT->hlinDataDs.deltHlin.nNumData;

      nAddr = DSL_CMV_ADDRESS_INFO_DELT_HLIN_PS_DS;

      nSizeMax = 256;
   }
   else
   {
      pData = pContext->DELT->hlinDataUs.deltHlin.nSCGComplexData;
      pnMeasurementTime = &pContext->DELT->hlinDataUs.nMeasurementTime;
      pnGroupSize = &pContext->DELT->hlinDataUs.nGroupSize;
      pnNumData = &pContext->DELT->hlinDataUs.deltHlin.nNumData;

      nAddr = DSL_CMV_ADDRESS_INFO_DELT_HLIN_PS_US;

      nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, nDirection, &nSizeMax);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }
   }

   for (nIdx = 0; nIdx < nSizeMax; nIdx += nSize)
   {
      if (nIdx + nSize >= nSizeMax)
      {
         nSize = nSizeMax - nIdx;
      }

      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         nAddr, (DSL_uint16_t)(nIdx * 2), nSize * 2, buf);
         
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CMV "
            "DSL_CMV_ADDRESS_INFO_DELT_HLIN_PS_DS(US) read fail, nIdx=%d,"
            " nSize=%d!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nIdx * 2, nSize * 2));
      }
      else
      {
         memcpy (&pData[nIdx], &buf,
            ((DSL_uint32_t)(nSize * 2)) <= sizeof(buf)/sizeof(buf[0]) ? nSize * 2 * sizeof(DSL_uint16_t) :
                                                       sizeof(buf));
      }
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext, "DSL[%02d]: CMV "
      "DSL_CMV_ADDRESS_INFO_DELT_HLIN_PS, nDirection=%d,"
      " nSize=%d!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nDirection, nSizeMax));
   for (nMsgIdx = 0; nMsgIdx < nSizeMax; nMsgIdx++)
   {
      DSL_DEBUG( DSL_DBG_MSG, (pContext, " %02X%02X",
         pData[nMsgIdx].nReal, 
         pData[nMsgIdx].nImag ));
   }
   DSL_DEBUG( DSL_DBG_MSG, (pContext, DSL_DRV_CRLF));

   *pnNumData = nSizeMax;
   *pnMeasurementTime = 256;

   /* set real group size, it will be fixed in the API functions */
   if ((bAdsl2p) && (nDirection == DSL_DOWNSTREAM))
   {
      /* in case of ADSL 2+ the values will be doubled */
      *pnGroupSize = 2;
   }
   else
   {
      *pnGroupSize = 1;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_DeltHlinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltHlogGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_DeltHlogData_t *pHlogData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nIdx = 0, nSize = 12, nMsgIdx;
   DSL_uint16_t buf[12];
   DSL_uint16_t nSizeMax, nAddr, nAddrMT;
   DSL_boolean_t bAdsl2p = DSL_FALSE;
   /* pointers data should be saved to */
   DSL_uint16_t *pData;
   DSL_uint16_t *pnMeasurementTime, *pnNumData;
   DSL_uint8_t *pnGroupSize;

   DSL_CHECK_POINTER(pContext, pHlogData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_DeltHlogGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get ADSL2+ mode indication*/
   bAdsl2p  = pContext->pDevCtx->data.bAdsl2p;

   pData             = pHlogData->deltHlog.nNSCData;
   pnMeasurementTime = &(pHlogData->nMeasurementTime);
   pnGroupSize       = &(pHlogData->nGroupSize);
   pnNumData         = &(pHlogData->deltHlog.nNumData);

   if (nDirection == DSL_DOWNSTREAM)
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_HLOG_PS_DS;
      nAddrMT = DSL_CMV_ADDRESS_INFO_DELT_MT_DS;
      nSizeMax = 256;
   }
   else
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_HLOG_PS_US;
      nAddrMT = DSL_CMV_ADDRESS_INFO_DELT_MT_US;

      nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, nDirection, &nSizeMax);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }
   }

   for (nIdx = 0; nIdx < nSizeMax; nIdx += nSize)
   {
      if (nIdx + nSize >= nSizeMax)
      {
         nSize = nSizeMax - nIdx;
      }

      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         nAddr, (DSL_uint16_t)nIdx, nSize, buf);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CMV "
            "DSL_CMV_ADDRESS_INFO_DELT_HLOG_PS_DS(US) read fail, nIdx=%d,"
            " nSize=%d!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nIdx, nSize));
      }
      else
      {
         memcpy (&pData[nIdx], &buf,
            ((DSL_uint32_t)nSize) <= sizeof(buf)/sizeof(buf[0]) ? nSize * sizeof(DSL_uint16_t):
                                             sizeof(buf));
      }
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext, "DSL[%02d]: CMV "
      "DSL_CMV_ADDRESS_INFO_DELT_HLOG_PS, nDirection=%d, "
      "nSize=%d!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nDirection, nSizeMax));
   for (nMsgIdx = 0; nMsgIdx < nSizeMax; nMsgIdx++)
   {
      DSL_DEBUG( DSL_DBG_MSG, (pContext, " %02X", pData[nMsgIdx]));
   }
   DSL_DEBUG( DSL_DBG_MSG, (pContext, DSL_DRV_CRLF));

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      nAddrMT, DSL_CMV_DELT_INDEX_MT_HLOG, 1, buf);
   *pnMeasurementTime = buf[0];

   *pnNumData = nSizeMax;

   /* set real group size, it will be fixed in the API functions */
   if ((bAdsl2p) && (nDirection == DSL_DOWNSTREAM))
   {
      /* in case of ADSL 2+ the values will be doubled */
      *pnGroupSize = 2;
   }
   else
   {
      *pnGroupSize = 1;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_DeltHlogGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_G997_ShowtimeHlogUsGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_G997_DeltHlogData_t *pHlogData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t i = 0;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_G997_PowerManagement_t nPowerMode = DSL_G997_PMS_NA;
   DSL_uint16_t hdlcCmd[2];
   /* ack + Meaasurement time + NSC HLOG values*/
   DSL_uint16_t hdlcRxBuffer[1 + 1 + DSL_MAX_HLOG_US_SCG] = {0};
   DSL_int_t nHdlcRxLen = 0;
   
   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /* Check for the ADSL1 mode*/
   if (bAdsl1)
   {
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   /* In L2 power mode, do not read the OHC related parameters, instead give
      the indication to the calling IOCTL, that the readout fails (just
      return DSL_WRN_INCOMPLETE_RETURN_VALUES).  */
   nErrCode = DSL_DRV_DANUBE_L3StatusGet(pContext, &nPowerMode);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Could not get power mode!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   /* Only proceed if not in the L2 power mode*/
   if (nPowerMode == DSL_G997_PMS_L2)
   {
      return DSL_ERR_DEVICE_NO_DATA;
   }

   /* Fill HDLC command*/
   hdlcCmd[0] = 0x0181;
   hdlcCmd[1] = 0x01;

   /* Lock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

   /* Write HDLC*/
   nErrCode = DSL_DRV_DANUBE_HdlcWrite(
                 pContext, DSL_FALSE, (DSL_uint8_t *)hdlcCmd, 4);
   
   if (nErrCode == DSL_SUCCESS)
   {
      DSL_WAIT (1);

      /* Read HDLC*/
      nErrCode = DSL_DRV_DANUBE_HdlcRead(
                    pContext, DSL_FALSE, (DSL_uint8_t *)hdlcRxBuffer,
                   (DSL_int_t)(sizeof(hdlcRxBuffer)), &nHdlcRxLen);
      
      if (nErrCode == DSL_SUCCESS)
      {
         if (nHdlcRxLen <= 0)
         {
            DSL_DEBUG( DSL_DBG_WRN,
               (pContext, "DSL[%02d]: WARNING - HDLC read invalid RxLen!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            nErrCode = DSL_ERROR;
         }
      }
   }

   /* Unlock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get measurement time*/
   pHlogData->nMeasurementTime = DSL_Le2Cpu(hdlcRxBuffer[1]);

   /* Set Group size. According to the G997.1 (9.4.1.10 and 8.12.3.1)
      PMD test parameter Hlog(f) is reported per subcarrier.
      Set group size always to 1*/
   pHlogData->nGroupSize = 1;

   /* Get number of used tones according to the current ADSL mode*/
   nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(
      pContext, DSL_UPSTREAM, &(pHlogData->deltHlog.nNumData));
      
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   for (i = 0; (i < pHlogData->deltHlog.nNumData) &&
               (i < ((sizeof(hdlcRxBuffer)/sizeof(hdlcRxBuffer[0]) - 2))); i++)
   {
      pHlogData->deltHlog.nNSCData[i] = DSL_Le2Cpu(hdlcRxBuffer[i+2]);
   }

   return nErrCode;
}


/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltQLNGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_DeltQlnData_t *pQlnData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nIdx = 0, nSize = 12, nMsgIdx = 0;
   DSL_uint16_t buf[12];
   DSL_uint16_t nSizeMax, nAddr, nAddrMT;
   DSL_boolean_t bAdsl2p = DSL_FALSE;

   /* pointers data should be saved to */
   DSL_uint8_t *pData;
   DSL_uint16_t *pnMeasurementTime, *pnNumData;
   DSL_uint8_t *pnGroupSize;

   DSL_CHECK_POINTER(pContext, pQlnData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_DeltQLNGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get ADSL2+ mode indication*/
   bAdsl2p  = pContext->pDevCtx->data.bAdsl2p;

   pData             = pQlnData->deltQln.nNSCData;
   pnMeasurementTime = &(pQlnData->nMeasurementTime);
   pnGroupSize       = &(pQlnData->nGroupSize);
   pnNumData         = &(pQlnData->deltQln.nNumData);

   if (nDirection == DSL_DOWNSTREAM)
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_QLN_PS_DS;
      nAddrMT = DSL_CMV_ADDRESS_INFO_DELT_MT_DS;
      nSizeMax = 128;
   }
   else
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_QLN_PS_US;
      nAddrMT = DSL_CMV_ADDRESS_INFO_DELT_MT_US;

      nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, nDirection, &nSizeMax);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

      nSizeMax /=2;
   }

   for (nIdx = 0; nIdx < nSizeMax; nIdx += nSize)
   {
      if (nIdx + nSize >= nSizeMax)
      {
         nSize = nSizeMax - nIdx;
      }

      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         nAddr, (DSL_uint16_t)nIdx, nSize, buf);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CMV "
            "DSL_CMV_ADDRESS_INFO_DELT_QLN_PS_DS(US) read fail, nIdx=%d,"
            " nSize=%d!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nIdx, nSize));
      }
      else
      {
         for (nMsgIdx = 0; (nMsgIdx < nSize) && ((DSL_uint32_t)nSize <= sizeof(buf)/sizeof(buf[0]));
              nMsgIdx++)
         {
            pData[(nIdx + nMsgIdx) * 2]     = buf[nMsgIdx] & 0xFF;
            pData[(nIdx + nMsgIdx) * 2 + 1] = (buf[nMsgIdx] >> 8) & 0xFF;
         }
      }
   }

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      nAddrMT, DSL_CMV_DELT_INDEX_MT_QLN, 1, buf);
   *pnMeasurementTime = buf[0];

   *pnNumData = (DSL_uint16_t)(nSizeMax * 2);

   /* set real group size, it will be fixed in the API functions */
   if ((bAdsl2p) && (nDirection == DSL_DOWNSTREAM))
   {
      /* in case of ADSL 2+ the values will be doubled */
      *pnGroupSize = 2;
   }
   else
   {
      *pnGroupSize = 1;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_DeltQLNGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_G997_ShowtimeQlnUsGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_G997_DeltQlnData_t *pQlnData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t i = 0;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_G997_PowerManagement_t nPowerMode = DSL_G997_PMS_NA;
   DSL_uint16_t hdlcCmd[2];
   /* ack + Meaasurement time + NSC QLN values*/
   DSL_uint8_t hdlcRxBuffer[2 + 2 + DSL_MAX_QLN_US_SCG] = {0};
   DSL_int_t nHdlcRxLen = 0;
   DSL_uint16_t nWrapedData = 0;
   
   /* Get ADSL mode information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /* Check for the ADSL1 mode*/
   if (bAdsl1)
   {
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   /* In L2 power mode, do not read the OHC related parameters, instead give
      the indication to the calling IOCTL, that the readout fails (just
      return DSL_WRN_INCOMPLETE_RETURN_VALUES).  */
   nErrCode = DSL_DRV_DANUBE_L3StatusGet(pContext, &nPowerMode);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Could not get power mode!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   /* Only proceed if not in the L2 power mode*/
   if (nPowerMode == DSL_G997_PMS_L2)
   {
      return DSL_ERR_DEVICE_NO_DATA;
   }

   /* Fill HDLC command*/
   hdlcCmd[0] = 0x0181;
   hdlcCmd[1] = 0x03;

   /* Lock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

   /* Write HDLC*/
   nErrCode = DSL_DRV_DANUBE_HdlcWrite(
                 pContext, DSL_FALSE, (DSL_uint8_t *)hdlcCmd, 4);
   
   if (nErrCode == DSL_SUCCESS)
   {
      DSL_WAIT (1);

      /* Read HDLC*/
      nErrCode = DSL_DRV_DANUBE_HdlcRead(
                    pContext, DSL_FALSE, (DSL_uint8_t *)hdlcRxBuffer,
                   (DSL_int_t)sizeof(hdlcRxBuffer), &nHdlcRxLen);
      
      if (nErrCode == DSL_SUCCESS)
      {
         if (nHdlcRxLen <= 0)
         {
            DSL_DEBUG( DSL_DBG_WRN,
               (pContext, "DSL[%02d]: WARNING - HDLC read invalid RxLen!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            nErrCode = DSL_ERROR;
         }
      }
   }

   /* Unlock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get measurement time*/
   pQlnData->nMeasurementTime = DSL_Le2Cpu(*((DSL_uint16_t*)&hdlcRxBuffer[2]));

   /* Set Group size. According to the G997.1 
      PMD test parameter Qln(f) is reported per subcarrier.
      Set group size always to 1*/
   pQlnData->nGroupSize = 1;

   /* Get number of used tones according to the current ADSL mode*/
   nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(
      pContext, DSL_UPSTREAM, &(pQlnData->deltQln.nNumData));
      
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   for (i = 0; (i < pQlnData->deltQln.nNumData) && (i < (sizeof(hdlcRxBuffer) - 4)); i+=2)
   {
      nWrapedData = DSL_Le2Cpu(*((DSL_uint16_t*)&hdlcRxBuffer[i+4]));
      pQlnData->deltQln.nNSCData[i]   = (nWrapedData >> 8) & 0xFF;
      pQlnData->deltQln.nNSCData[i+1] =  nWrapedData & 0xFF;
   }

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
static DSL_Error_t DSL_DRV_DANUBE_G997_DeltSNRGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_DeltSnrData_t *pSnrData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nIdx = 0, nSize = 12, nMsgIdx;
   DSL_uint16_t buf[12];
   DSL_uint16_t nSizeMax, nAddr, nAddrMT;
   DSL_uint16_t nDsRangeD, nDsRangeU;
   DSL_uint16_t snrQ88 = 0, intPart = 0;
   DSL_boolean_t bAdsl2p = DSL_FALSE;

   /* pointers data should be saved to */
   DSL_uint8_t *pData;
   DSL_uint16_t *pnMeasurementTime, *pnNumData;
   DSL_uint8_t *pnGroupSize;

   DSL_CHECK_POINTER(pContext, pSnrData);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_G997_DeltSNRGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get ADSL2+ mode indication*/
   bAdsl2p = pContext->pDevCtx->data.bAdsl2p;

   pData             = pSnrData->deltSnr.nNSCData;
   pnMeasurementTime = &(pSnrData->nMeasurementTime);
   pnGroupSize       = &(pSnrData->nGroupSize);
   pnNumData         = &(pSnrData->deltSnr.nNumData);

   if (nDirection == DSL_DOWNSTREAM)
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_SNR_PS_DS;
      nAddrMT = DSL_CMV_ADDRESS_INFO_DELT_MT_DS;
      if (bAdsl2p)
      {
         nSizeMax = 512;
      }
      else
      {
         nSizeMax = 256;
      }
   }
   else
   {
      nAddr = DSL_CMV_ADDRESS_INFO_DELT_SNR_PS_US;
      nAddrMT = DSL_CMV_ADDRESS_INFO_DELT_MT_US;

      nErrCode = DSL_DRV_DANUBE_UsedTonesNumberGet(pContext, nDirection, &nSizeMax);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Used tones number get failed"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

      nSizeMax /=2;
   }

   for (nIdx = 0; nIdx < nSizeMax; nIdx += nSize)
   {
      if (nIdx + nSize >= nSizeMax)
      {
         nSize = nSizeMax - nIdx;
      }

      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         nAddr, (DSL_uint16_t)nIdx, nSize, buf);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CMV "
            "DSL_CMV_ADDRESS_INFO_DELT_SNR_PS_DS(US) read fail, nIdx=%d,"
            " nSize=%d!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nIdx, nSize));
      }
      else
      {
         for (nMsgIdx = 0; (nMsgIdx < nSize) && ((DSL_uint32_t)nSize <= sizeof(buf)/sizeof(buf[0]));
              nMsgIdx++)
         {
            if (nDirection == DSL_DOWNSTREAM)
            {
               snrQ88  = (DSL_uint16_t)(((buf[nMsgIdx]) + (32 << 8)) * 2);
               intPart = (snrQ88 >> 8) & 0xFF;

               pData[nIdx + nMsgIdx] =
                  (DSL_uint8_t)((snrQ88 & 0xFF) < 128 ? intPart : intPart + 1);
            }
            else
            {
               pData[(nIdx + nMsgIdx) * 2]     = buf[nMsgIdx] & 0xFF;;
               pData[(nIdx + nMsgIdx) * 2 + 1] = (buf[nMsgIdx] >> 8) & 0xFF;
            }
         }
      }
   }

   if (nDirection == DSL_UPSTREAM)
   {
      nSizeMax *= 2;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext, "DSL[%02d]: ERROR - CMV "
      "DSL_CMV_ADDRESS_INFO_DELT_SNR_PS, nDirection=%d, "
      "nSize=%d!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nDirection, nSizeMax));
   for (nMsgIdx = 0; nMsgIdx < nSizeMax; nMsgIdx++)
   {
      DSL_DEBUG( DSL_DBG_MSG, (pContext, " %02X",
         pData[nMsgIdx]));
   }
   DSL_DEBUG( DSL_DBG_MSG, (pContext, DSL_DRV_CRLF));

   /* convert values for DS */
   if (nDirection == DSL_DOWNSTREAM)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_DS_BIT_LOAD_FIRST, 0, 1, &nDsRangeD);
      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_DS_BIT_LOAD_LAST, 0, 1, &nDsRangeU);
         if (nErrCode == DSL_SUCCESS)
         {
            for (nIdx = 0; nIdx < nDsRangeD; nIdx++)
            {
               if (pData[nIdx] == 0x40)
               {
                  pData[nIdx] = 0xFF;
               }
            }
            for (nIdx = nDsRangeU + 1; nIdx < nSizeMax; nIdx++)
            {
               if (pData[nIdx] == 0x40)
               {
                  pData[nIdx] = 0xFF;
               }
            }
         }
      }
   }

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      nAddrMT, DSL_CMV_DELT_INDEX_MT_SNR, 1, buf);
   *pnMeasurementTime = buf[0];

   *pnNumData = nSizeMax;
   *pnGroupSize = 1;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Snr internal data info - nMeasurementTime = %d,"
      "nNumData = %d (%p, %p)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext),
      *pnMeasurementTime,
      *pnNumData,
      pnMeasurementTime,
      pnNumData));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_DeltSNRGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DANUBE_G997_LoopDiagnosticCompleted(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_AccessDir_t nDirection;
   DSL_uint8_t nAutoCount = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_G997_LoopDiagnosticCompleted"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_CTX_POINTER(pContext);

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLoopAutoCount, nAutoCount);
   if( nAutoCount > 0 )
   {
      nAutoCount -= 1;
      DSL_CTX_WRITE(pContext, nErrCode, nLoopAutoCount, nAutoCount);
   }

/* Do not include additional code */
#ifndef DSL_CPE_STATIC_DELT_DATA
   if (pContext->DELT == DSL_NULL)
   {
      pContext->DELT = DSL_DRV_Malloc(sizeof(DSL_G997_DeltData_t));
   }
#endif

   if (pContext->DELT == DSL_NULL)
   {
      nErrCode = DSL_ERR_MEMORY;
   }
   else
   {
      memset(pContext->DELT, 0, sizeof(DSL_G997_DeltData_t));
   }

   if (nErrCode == DSL_SUCCESS)
   {
      if (DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERROR;
      }

      for (nDirection = DSL_UPSTREAM; nDirection <= DSL_DOWNSTREAM; nDirection++)
      {
         nErrCode = DSL_DRV_DANUBE_G997_DeltHlinGet(pContext, nDirection);
         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_G997_DeltHlinScaleGet(pContext, nDirection);
            if (nErrCode == DSL_SUCCESS)
            {
               nErrCode = DSL_DRV_DANUBE_G997_DeltHlogGet(
                          pContext,
                          nDirection,
                          (nDirection == DSL_DOWNSTREAM) ?
                          (DSL_G997_DeltHlogData_t*)&(pContext->DELT->hlogDataDs) :
                          (DSL_G997_DeltHlogData_t*)&(pContext->DELT->hlogDataUs));
                          
               if (nErrCode == DSL_SUCCESS)
               {
                  nErrCode =  DSL_DRV_DANUBE_G997_DeltQLNGet(
                                 pContext, nDirection,
                                 (nDirection == DSL_DOWNSTREAM) ?
                                 (DSL_G997_DeltQlnData_t*)&(pContext->DELT->qlnDataDs) :
                                 (DSL_G997_DeltQlnData_t*)&(pContext->DELT->qlnDataUs));
                                 
                  if (nErrCode == DSL_SUCCESS)
                  {
                     nErrCode =  DSL_DRV_DANUBE_G997_DeltSNRGet(
                                    pContext, nDirection,
                                    (nDirection == DSL_DOWNSTREAM) ?
                                    (DSL_G997_DeltSnrData_t*)&(pContext->DELT->snrDataDs) :
                                    (DSL_G997_DeltSnrData_t*)&(pContext->DELT->snrDataUs));
                  }
               }
            }
         }
      }

      DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_G997_LoopDiagnosticCompleted, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_DELT*/

#ifdef __cplusplus
}
#endif
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/
