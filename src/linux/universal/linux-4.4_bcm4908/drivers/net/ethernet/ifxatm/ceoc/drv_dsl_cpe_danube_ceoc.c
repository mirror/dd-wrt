/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#if defined(INCLUDE_DSL_CEOC)

#if defined (INCLUDE_DSL_CPE_API_DANUBE)

#include "drv_dsl_cpe_intern_ceoc.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_CEOC

DSL_Error_t DSL_CEOC_EventCB (
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if (pContext == NULL)
   {
      return DSL_ERROR;
   }

   /* Wakeup immediately the CEOC thread*/
   DSL_DRV_WAKEUP_EVENT((DSL_CEOC_CONTEXT(pContext)->ceocThread.waitEvent));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_ceoc.h'
*/
DSL_Error_t DSL_DRV_DANUBE_CEOC_FirmwareInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nData = 1;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_CEOC_FirmwareInit"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Enable Event Interrupts
      Enable additional ARC-to-MEI interrupts. */
   nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_OPTN,
      DSL_CMV_ADDRESS_OPTN_EVENT_INTS_CTRL, 0, 1, &nData);

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.bCeocRx, DSL_FALSE);
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.nCeocReadIndex, 0);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_CEOC_FirmwareInit, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_ceoc.h'
*/
DSL_Error_t DSL_CEOC_DEV_Start(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_DEV_Start"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_DEV_Start, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_CEOC_DEV_Stop(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_DEV_Stop"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_DEV_Stop, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_CEOC_DEV_Restart(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_DEV_Restart"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_DEV_Restart, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_CEOC_DEV_MessageSend(
   DSL_Context_t *pContext,
   DSL_uint16_t pProtIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nPktLen = 0, nOffset = 0;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_int_t nSwapIdx = 0;
   DSL_uint8_t *pPktData;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_DEV_MessageSend"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /* check the adsl mode, if adsl 2/2+ mode, it need to fill the clear eoc
      header. */
   if (bAdsl1) /* adsl mode */
   {
#if 0     
      nPktLen = pMsg->length;
      pPktData = DSL_DRV_PMalloc(nPktLen + 3);

      if (pPktData != DSL_NULL)
      {
         nOffset = 2;               /* snmp header */
         /* SNMP header */
         pPktData[0] = pProtIdent & 0xFF;
         pPktData[1] = (pProtIdent >> 8) & 0xFF;
         nPktLen += 2;
      }
#endif
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }
   else
   {
      nPktLen = pMsg->length + 4;       /* 4: eoc header and snmp header */
      pPktData = DSL_DRV_PMalloc (nPktLen + 1 + 2);
      if (pPktData != DSL_NULL)
      {
         memset (pPktData, 0, nPktLen + 1 + 2);
         /* fill clear eoc header */
         pPktData[0] = 0x1;
         pPktData[1] = 0x8;
         /* SNMP header */
         pPktData[2] = pProtIdent & 0xFF;
         pPktData[3] = (pProtIdent >> 8) & 0xFF;
         nOffset = 4;               /* skip eoc header + snmp header */
      }
   }

   if (pPktData == DSL_NULL)
   {
      nErrCode = DSL_ERR_MEMORY;
   }
   else
   {
      /* swap data */
      for (nSwapIdx = 0; nSwapIdx < (pMsg->length & ~0x1); nSwapIdx += 2)
      {
         /* printk("%02X %02X
            ",eoc_pkt->data[swap_idx],eoc_pkt->data[swap_idx+1]); */
         pPktData[nSwapIdx + nOffset]     = pMsg->data[nSwapIdx + 1];
         pPktData[nSwapIdx + 1 + nOffset] = pMsg->data[nSwapIdx];
      }

      if (pMsg->length % 2)
      {
         /* printk("%02X ",eoc_pkt->data[eoc_pkt->len-1]); */
         pPktData[(pMsg->length - 1) + nOffset] = pMsg->data[pMsg->length - 1];
         pPktData[pMsg->length + nOffset]       = pMsg->data[pMsg->length - 1];
      }

      /* Lock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

      /* send to CO */
      nErrCode = DSL_DRV_DANUBE_HdlcWrite(pContext, DSL_FALSE, pPktData, nPktLen);

      /* Unlock HDLC handling*/
      DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);

      DSL_DRV_PFree(pPktData);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_DEV_MessageSend, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/* It supposes that all pointers is already checked */
static DSL_Error_t DSL_CEOC_DANUBE_GetByte(
   DSL_uint32_t nVal32,
   DSL_uint8_t nOffset,
   DSL_uint8_t *pVal8)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if (nOffset > 3)
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "DSL: DSL_CEOC_DANUBE_GetByte - "
         "invalid offset (%u)!" DSL_DRV_CRLF, nOffset));
      return DSL_ERROR;   
   }

   *pVal8 = (DSL_uint8_t)((nVal32 & (0xFF << nOffset * 8)) >> (nOffset * 8));

   return nErrCode;
}

/* It supposes that all pointers is already checked */
static DSL_Error_t DSL_CEOC_DANUBE_MessageReceive(
   DSL_Context_t *pContext,
   DSL_uint16_t *pProtIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t nRxBufBaseAddr32 = 0, nRxBufEndAddr32 = 0, nVal32 = 0;
   DSL_uint16_t nAddrLsw = 0, nAddrMsw = 0, nIdx = 0;
   DSL_uint8_t nVal8 = 0;
   DSL_uint8_t *pBuf = DSL_NULL;
   /* Note:
      Numbers attached to following variable names indicates to which data type
      the offset is related, NOT the type of the variable itself! */
   DSL_uint16_t nReadIdx8 = 0, nWriteIdx8 = 0, nRxBufLen8 = 0, nPayloadLen8 = 0;
   DSL_uint16_t nReadAddrOffset32 = 0;
   DSL_uint8_t nOffset8 = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_DANUBE_MessageReceive"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   do
   {
      /* Get write index or byte offset from the start of the fifo.
         Points to the byte after the last byte that should be read by the ME. */
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 4, 1, &nWriteIdx8);
      if (nErrCode < DSL_SUCCESS) break;

      /* Get currently configured "Rx Clear EOC Fifo/Buffer" length in bytes */
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 2, 1, &nRxBufLen8);
      if (nErrCode < DSL_SUCCESS) break;

      /* Get base address of RX EOC buffer from within firmware */
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 0, 1, &nAddrLsw);
      if (nErrCode < DSL_SUCCESS) break;

      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 1, 1, &nAddrMsw);
      if (nErrCode < DSL_SUCCESS) break;

      nRxBufBaseAddr32 = nAddrMsw << 16 | nAddrLsw;

      /* Get read index or byte offset from the start of the fifo Points to the
         next byte that should be read by ME (use DSL CPE API internally stored
         reference value here). */
      DSL_CTX_READ_SCALAR(pContext, nErrCode, pDevCtx->data.nCeocReadIndex,
         nReadIdx8);
      if (nErrCode < DSL_SUCCESS) break;

      /* Calculate length of provided octets within firmware Rx buffer */
      if (nReadIdx8 <= nWriteIdx8)
      {
         nPayloadLen8 = nWriteIdx8 - nReadIdx8;
      }
      else
      {
         nPayloadLen8 = nRxBufLen8 - (nReadIdx8 - nWriteIdx8);
      }

      /* Do some sanity checks */
      if (nPayloadLen8 == 0)
      {
         DSL_DEBUG(DSL_DBG_WRN, (pContext, "DSL[%02d]: no data available in Rx buffer"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_WRN_SNMP_NO_DATA;
         
         break;
      }
      if ((sizeof(pMsg->data) + 2) < nPayloadLen8)
      {
         DSL_DEBUG(DSL_DBG_WRN, (pContext, "DSL[%02d]: SNMP FIFO element size to small!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERROR;
         
         break;
      }

      /* allocate memory */
      pBuf = DSL_DRV_PMalloc(nPayloadLen8 + 2);
      if (pBuf == NULL)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: DSL_CEOC_DANUBE_MessageReceive: "
            "Memory allocation failed!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERR_MEMORY;
         
         break;
      }

      /* Read CEOC data from firmware buffer by using 32-bit aligned access */
      nRxBufEndAddr32 = nRxBufBaseAddr32 + ((DSL_uint16_t)nRxBufLen8);
      nReadAddrOffset32 = nReadIdx8 & 0xFFFC;
      nOffset8 = nReadIdx8 % 4;
      for (nIdx = 0; nIdx < nPayloadLen8; nIdx++, nOffset8++)
      {
         if (nOffset8 >= 4)
         {
            nOffset8 = 0;
            nReadAddrOffset32 += 4;
         }

         /* Detect turnaround of RX buffer pointer */
         if ((nRxBufBaseAddr32 + nReadAddrOffset32) > (nRxBufEndAddr32 - 4))
         {
            nReadAddrOffset32 = 0;
         }

         /* Read new value from buffer if necessary */
         if ((nIdx == 0) || (nOffset8 == 0))
         {
            nErrCode = (DSL_Error_t)DSL_BSP_MemoryDebugAccess(pContext->pDevCtx->lowHandle,
               DSL_BSP_MEMORY_READ, (nRxBufBaseAddr32 + nReadAddrOffset32),
               &nVal32, 1);
         }
         if (nErrCode < DSL_SUCCESS)
         {
            nErrCode = DSL_ERROR;
            break;
         }

         nErrCode = DSL_CEOC_DANUBE_GetByte(nVal32, nOffset8, &nVal8);
         if (nErrCode < DSL_SUCCESS)
         {
            nErrCode = DSL_ERROR;
            break;
         }
         else
         {
            /* Set current octet within result buffer */
            pBuf[nIdx] = nVal8;
         }
      }

      /* Extract protocoll identifier and copy data to return buffer */
      if (nIdx > (sizeof(pMsg->data) + 2))
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: DSL_CEOC_DANUBE_MessageReceive: "
            "Local buffer underrun!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERR_MEMORY;
         
         break;
      }
      else if (nIdx <= 2)
      {
         *pProtIdent = 0;
         pMsg->length = nIdx;
         memcpy(pMsg->data, pBuf, nIdx);
      }
      else
      {
         *pProtIdent = ((DSL_uint16_t)(pBuf[1] & 0xFF)) | ((pBuf[0] << 8) & 0xFF00);
         pMsg->length = nIdx - 2;
         memmove(&pMsg->data[0], (pBuf + 2), nIdx - 2);
      }
   }
   while (0);

   if (nErrCode == DSL_SUCCESS)
   {
      nReadIdx8 = nReadAddrOffset32 + nOffset8;

      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 3, 1, &nReadIdx8);
      
      if (nErrCode == DSL_SUCCESS)
      {
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.nCeocReadIndex,
            nReadIdx8);
      }
   }

   if (pBuf != DSL_NULL)
   {
      DSL_DRV_PFree(pBuf);
      pBuf = DSL_NULL;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_CEOC_DANUBE_MessageReceive, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_CEOC_DEV_MessageReceive(
   DSL_Context_t *pContext,
   DSL_uint16_t *pProtIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_DEV_MessageReceive"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_POINTER(pContext, pContext->CEOC);
   DSL_CHECK_POINTER(pContext, pProtIdent);
   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   do
   {
      if (DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun == DSL_FALSE)
         break;

      nErrCode = DSL_CEOC_DANUBE_MessageReceive (pContext, pProtIdent, pMsg);
   }
   while (0);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_CEOC_DEV_MessageReceive, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/
#endif /* INCLUDE_DSL_CEOC */
