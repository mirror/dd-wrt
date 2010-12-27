/******************************************************************************

                              Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "dsl_cpe_bnd.h"

#ifdef INCLUDE_DSL_BONDING

#include "drv_dsl_cpe_api_ioctl.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_APP

static DSL_Error_t DSL_CPE_BND_Setup(
                  DSL_int_t     fd,
                  DSL_boolean_t bPafEnable,
                  DSL_uint16_t  nTxDataRateRatio)
{
   DSL_int_t ret = 0;
   DSL_BND_Setup_t bndSetup;

   /* maximum message size includinf ID, Index and Length field */
   #define VNX_MSG_LEN  262
   /**
      This bit of the Message ID identifies 32 bit aligned TC messages */
   #define VDSL2_MBX_MSG_ID_IFX_MASK 0x0010

   DSL_DeviceMessage_t DevMsg;
   DSL_uint8_t msg[VNX_MSG_LEN];
   DSL_uint16_t *pMsg16 = (DSL_uint16_t*)&msg[0];
   DSL_uint32_t *pMsg32 = (DSL_uint32_t*)&msg[6];

   memset (&bndSetup, 0, sizeof(DSL_BND_Setup_t));

   bndSetup.data.bPafActivation  = bPafEnable;
   bndSetup.data.TxDataRateRatio = nTxDataRateRatio;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_SETUP, (int) &bndSetup);

   if ((ret < 0) || (bndSetup.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_SETUP ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   /*
   If Bonding is enabled, use the CMD_DBG_MEMMAPWRITE
   message via the DSL_FIO_DBG_DEVICE_MESSAGE_SEND
   IOCTL function to disable the MAC by clearing
   addresses 0xc0c50 and 0xc0c54
   */
   if (bPafEnable)
   {
       /* setup the message */
      pMsg16 = (DSL_uint16_t*)&msg[0];
      pMsg32 = (DSL_uint32_t*)&msg[6];

       #define CMD_DBG_MEMMAPWRITE 0xA173

      *pMsg16 = (DSL_uint16_t)CMD_DBG_MEMMAPWRITE;
      pMsg16++;
      *pMsg16 = (DSL_uint16_t)0;
      pMsg16++;
      *pMsg16 = (DSL_uint16_t)2;
      pMsg16++;

      /* set address to write to */
      *pMsg32 = (DSL_uint32_t)0xc0c50;
      pMsg32++;
      *pMsg32 = (DSL_uint32_t)0;

      DevMsg.data.pMsg    = (DSL_uint8_t*)&msg;
      DevMsg.data.nSizeTx = (DSL_uint16_t)14;
      DevMsg.data.nSizeRx = (DSL_uint16_t)VNX_MSG_LEN;

      ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int) &DevMsg);

      if ((ret < 0) || (DevMsg.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - DSL_FIO_DBG_DEVICE_MESSAGE_SEND ioctl call failed, msgId=0xA173!" DSL_CPE_CRLF));

         return DSL_ERROR;
      }

      pMsg32 = (DSL_uint32_t*)&msg[6];

      /* update the address to write to */
      *pMsg32 = (DSL_uint32_t)0xc0c54;

      ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int) &DevMsg);

      if ((ret < 0) || (DevMsg.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - DSL_FIO_DBG_DEVICE_MESSAGE_SEND ioctl call failed, msgId=0xA173!" DSL_CPE_CRLF));

         return DSL_ERROR;
      }
   } /* if (PafEnable == 1) */


   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_TearDown(
               DSL_int_t fd)
{
   DSL_int_t ret = 0;
   DSL_BND_TearDown_t bndTearDown;

   memset (&bndTearDown, 0, sizeof(DSL_BND_TearDown_t));

   /* call IOCTL to read driver  */
   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_TEAR_DOWN, (int) &bndTearDown);

   if ((ret < 0) || (bndTearDown.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_TEAR_DOWN ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_BondingCheck(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd,
               DSL_uint16_t nPort)
{
   DSL_int_t ret = 0, i;
   DSL_BND_HsStatusGet_t status;
   DSL_BND_HsContinue_t  HsContinue;

   /* read bonding status */
   memset (&status, 0, sizeof(DSL_BND_HsStatusGet_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_HS_STATUS_GET, (DSL_int_t)&status);

   if ((ret < 0) || (status.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HS_STATUS_GET ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }


   /* Set RemotePafAvailable */
   pBndCtx->lineMonitorStateMachine[nPort].RemotePafAvailable =
      (DSL_uint16_t)status.data.nRemotePafSupported;

   /* clear if same */
   if ((status.data.nActivationMode & DSL_BND_DISCOVERY_CLEAR_IF_SAME))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Clear If Same Command Received" DSL_CPE_CRLF));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         status.data.nDiscoveryCode[0], status.data.nDiscoveryCode[1],
         status.data.nDiscoveryCode[2], status.data.nDiscoveryCode[3],
         status.data.nDiscoveryCode[4], status.data.nDiscoveryCode[5]));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Remote Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         pBndCtx->remoteDiscoveryCode[0], pBndCtx->remoteDiscoveryCode[1],
         pBndCtx->remoteDiscoveryCode[2], pBndCtx->remoteDiscoveryCode[3],
         pBndCtx->remoteDiscoveryCode[4], pBndCtx->remoteDiscoveryCode[5]));

      /* if received discovery code = current Remote Discovery Register,
         then clear the Remote Discovery Register */
      for (i = 0; i < 6; i++)
      {
         if (status.data.nDiscoveryCode[i] != pBndCtx->remoteDiscoveryCode[i])
            break;
      }

      if (i == 6)
      {
         memset(pBndCtx->remoteDiscoveryCode, 0x0, 6);
      }
   }

   /* set if clear */
   if ((status.data.nActivationMode & DSL_BND_DISCOVERY_SET_IF_CLEAR))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Set If Clear Command Received" DSL_CPE_CRLF));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         status.data.nDiscoveryCode[0], status.data.nDiscoveryCode[1],
         status.data.nDiscoveryCode[2], status.data.nDiscoveryCode[3],
         status.data.nDiscoveryCode[4], status.data.nDiscoveryCode[5]));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Remote Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         pBndCtx->remoteDiscoveryCode[0], pBndCtx->remoteDiscoveryCode[1],
         pBndCtx->remoteDiscoveryCode[2], pBndCtx->remoteDiscoveryCode[3],
         pBndCtx->remoteDiscoveryCode[4], pBndCtx->remoteDiscoveryCode[5]));

      /*  if current Remote Discovery Register is 0,
          then set the Remote Discovery Register to received discovery code */
      for (i = 0; i < 6; i++)
      {
         if (pBndCtx->remoteDiscoveryCode[i] != 0)
            break;
      }

      if (i == 6)
      {
         memcpy(pBndCtx->remoteDiscoveryCode, status.data.nDiscoveryCode, 6);
      }
   }

   /* aggregate set */
   if ((status.data.nActivationMode & DSL_BND_AGGREGATE_SET))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - AggregateSet Command Received" DSL_CPE_CRLF));

      pBndCtx->aggregateReg |= (1 << nPort);
      pBndCtx->lineMonitorStateMachine[nPort].PafAggregate = 1;
   }

   /* aggregate clear */
   if ((status.data.nActivationMode & DSL_BND_AGGREGATE_CLR) != 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - AggregateClr Command Received" DSL_CPE_CRLF));

      pBndCtx->aggregateReg &= (~(1 << nPort));
      pBndCtx->lineMonitorStateMachine[nPort].PafAggregate = 0;
   }

   /* set continue flag after bonding registers processed */

   memset (&HsContinue, 0, sizeof(DSL_BND_HsContinue_t));

   memcpy(HsContinue.data.nDiscoveryCode, pBndCtx->remoteDiscoveryCode, 6);
   HsContinue.data.nAggregateData = pBndCtx->aggregateReg;

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_HS_CONTINUE, (DSL_int_t)&HsContinue);

   if ((ret < 0) || (HsContinue.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HS_CONTINUE ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_RemotePafAvailableCheck(
                     DSL_int_t fd,
                     DSL_uint16_t *pRemotePafAvailable)
{
   DSL_int_t ret = 0;
   DSL_BND_HsStatusGet_t status;

   /* read bonding status */
   memset (&status, 0, sizeof(DSL_BND_HsStatusGet_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_HS_STATUS_GET, (DSL_int_t)&status);

   if ((ret < 0) || (status.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HS_STATUS_GET ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   /* Set RemotePafAvailable */
   *pRemotePafAvailable = (DSL_uint16_t)status.data.nRemotePafSupported;

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_LocalPafAvailableCheck(
                     DSL_int_t fd,
                     DSL_uint16_t *pLocalPafAvailable)
{
   DSL_int_t ret = 0;
   DSL_BND_ConfigGet_t bndConfig;

   /* read bonding status */
   memset (&bndConfig, 0, sizeof(DSL_BND_ConfigGet_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_CONFIG_GET, (DSL_int_t)&bndConfig);

   if ((ret < 0) || (bndConfig.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_CONFIG_GET ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   *pLocalPafAvailable = (DSL_uint16_t)bndConfig.data.bPafEnable;

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_BND_AutobootStatusRestartWait(
               DSL_int_t fd,
               DSL_uint_t nDevice)
{
   /* Bonding Teardown*/
   return DSL_CPE_BND_TearDown(fd);
}

DSL_Error_t DSL_CPE_BND_LineStateHandle(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd, DSL_uint_t nDevice,
               DSL_LineStateValue_t nLineState,
               DSL_LineStateValue_t nPrevLineState)
{
   DSL_Error_t nErrorCode = DSL_SUCCESS;
   DSL_int_t ret = 0;
   DSL_G997_ChannelStatus_t pChannelData;
   DSL_uint32_t TxDataRateRatioTemp1;
   DSL_uint32_t TxDataRateRatioTemp2;
   DSL_uint16_t TxDataRateRatio;

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "BND - Device=%d nLineState=0x%x nPrevLineState=0x%x" DSL_CPE_CRLF,
      nDevice, nLineState,nPrevLineState));

   /* BONDING CLR state*/
   if (nLineState == DSL_LINESTATE_BONDING_CLR)
   {
      nErrorCode = DSL_CPE_BND_BondingCheck(pBndCtx, fd, nDevice);

      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Bonding Check failed, Device=%d" DSL_CPE_CRLF, nDevice));
         return nErrorCode;
      }
   }

   /* Entry to SHOWTIME */
   if ( (nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
        (nPrevLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC) )
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Device=%d entering SHOWTIME_TC_SYNC" DSL_CPE_CRLF, nDevice));

      /* Check CL information for RemotePafAvailable */
      nErrorCode = DSL_CPE_BND_RemotePafAvailableCheck(
                     fd,
                     &(pBndCtx->lineMonitorStateMachine[nDevice].RemotePafAvailable));

      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Remote Paf Check failed, Device=%d" DSL_CPE_CRLF, nDevice));
         return nErrorCode;
      }

      /* Check CL information for PafAvailable */
      nErrorCode = DSL_CPE_BND_LocalPafAvailableCheck(
                     fd,
                     &(pBndCtx->lineMonitorStateMachine[nDevice].PafAvailable));

      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Local Paf Check failed, Device=%d" DSL_CPE_CRLF, nDevice));
         return nErrorCode;
      }

      /* enable/disable bonding */
      if ((pBndCtx->lineMonitorStateMachine[nDevice].PafAvailable) &&
         (pBndCtx->lineMonitorStateMachine[nDevice].RemotePafAvailable == 1) &&
         (pBndCtx->lineMonitorStateMachine[nDevice].PafAggregate == 1) )
      {
         pBndCtx->lineMonitorStateMachine[nDevice].PafEnable = 1;
      }
      else
      {
         pBndCtx->lineMonitorStateMachine[nDevice].PafEnable = 0;
      }

      /* Call DSL_FIO_G997_CHANNEL_STATUS_GET ioctl function
         to get upstream ActualDataRate*/
      pChannelData.nChannel   = 0;
      pChannelData.nDirection = DSL_UPSTREAM;
      ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_CHANNEL_STATUS_GET, (int) &pChannelData);

      if ((ret < 0) || (pChannelData.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - DSL_FIO_G997_CHANNEL_STATUS_GET ioctl call failed!"
            DSL_CPE_CRLF, nDevice));

         return DSL_ERROR;
      }

      pBndCtx->lineMonitorStateMachine[nDevice].TxDataRate =
         pChannelData.data.ActualDataRate/1000;

      if (pBndCtx->lineMonitorStateMachine[1-nDevice].TxDataRate == 0)
      {
         if (nDevice == 0)
         {
            TxDataRateRatio = 0xFFFF;
         }
         else
         {
            TxDataRateRatio = 0x0000;
         }
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - TxDataRateRatio = 0x%x" DSL_CPE_CRLF, TxDataRateRatio));
      }
      else
      {
         TxDataRateRatioTemp1 =
            DSL_CPE_BND_TX_RATE_RATIO_UNITY * pBndCtx->lineMonitorStateMachine[0].TxDataRate;

         TxDataRateRatioTemp2 =
            TxDataRateRatioTemp1/pBndCtx->lineMonitorStateMachine[1].TxDataRate;

         TxDataRateRatio = (DSL_uint16_t)TxDataRateRatioTemp2;

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - TxDataRateRatio = 0x%x" DSL_CPE_CRLF, TxDataRateRatio));
      }

      if (DSL_CPE_BND_Setup(
             fd, pBndCtx->lineMonitorStateMachine[nDevice].PafEnable ? DSL_TRUE : DSL_FALSE,
             TxDataRateRatio) != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Bonding Setup failed, nDevice=%d!" DSL_CPE_CRLF, nDevice));

         return DSL_ERROR;
      }
   }
   else if ((nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
            (nPrevLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC) )
   {
      /* Exit from SHOWTIME */
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Device=%d leaving SHOWTIME_TC_SYNC" DSL_CPE_CRLF, nDevice));

      /* clear data rate */
      pBndCtx->lineMonitorStateMachine[nDevice].TxDataRate = 0;

      /* Bonding Teardown */
      if (DSL_CPE_BND_TearDown(fd) != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Bonding Tear Down failed, nDevice=%d!" DSL_CPE_CRLF, nDevice));

         return DSL_ERROR;
      }
   }

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_HwInit(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd)
{
   DSL_int_t ret = 0;
   DSL_BND_HwInit_t bndHwInit;

   if (pBndCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - HW init: expecting non-zero bonding context pointer!" DSL_CPE_CRLF));
      return DSL_ERROR;
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "dsl_cpe_control - VDSL BONDING version!" DSL_CPE_CRLF));

   memset(&bndHwInit, 0x0, sizeof(DSL_BND_HwInit_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_HW_INIT, (int) &bndHwInit);

   if ((ret < 0) || (bndHwInit.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HW_INIT ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_BND_Start(
   DSL_CPE_Control_Context_t *pCtrlCtx,
   DSL_int_t fd)
{
   DSL_Error_t ret = DSL_SUCCESS;
   DSL_int_t i;
   DSL_CPE_BND_Context_t *pBndCtx = DSL_NULL;

   if (pCtrlCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - missing CPE context pointer pCtrlCtx!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   pBndCtx = DSL_CPE_Malloc(sizeof(DSL_CPE_BND_Context_t));

   if (pBndCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - Bonding context memory allocation failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   pBndCtx->lineMonitorStateMachine[0].Port = 0;
   pBndCtx->lineMonitorStateMachine[1].Port = 1;

   for (i = 0; i < DSL_CPE_MAX_DEVICE_NUMBER; i++)
   {
      pBndCtx->lineMonitorStateMachine[i].PafAvailable       = -1;
      pBndCtx->lineMonitorStateMachine[i].RemotePafAvailable = -1;
      pBndCtx->lineMonitorStateMachine[i].PafAggregate       = -1;
      pBndCtx->lineMonitorStateMachine[i].PafEnable          = -1;
      pBndCtx->lineMonitorStateMachine[i].TxDataRate         =  0;
   }

   memset(&(pBndCtx->remoteDiscoveryCode), 0x0, 6);

   pBndCtx->aggregateReg = 0;

   /* Initialize Bonding HW*/
   ret = DSL_CPE_BND_HwInit(pBndCtx, fd);

   if (ret < DSL_SUCCESS)
   {
      DSL_CPE_Free(pBndCtx);
      pCtrlCtx->pBnd = DSL_NULL;
   }
   else
   {
      pCtrlCtx->pBnd = (DSL_void_t*)pBndCtx;
   }

   return ret;
}

DSL_void_t DSL_CPE_BND_Stop(
   DSL_CPE_BND_Context_t *pBndContext)
{
   if (pBndContext != DSL_NULL)
   {
      DSL_CPE_Free(pBndContext);
   }

   return;
}

#endif /* INCLUDE_DSL_BONDING*/
