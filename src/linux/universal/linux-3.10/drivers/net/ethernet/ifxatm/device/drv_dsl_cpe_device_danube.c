/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN
/* lower level BSP driver stuff */
#include "drv_dsl_cpe_api.h"

#if defined (INCLUDE_DSL_CPE_API_DANUBE)

#include "drv_dsl_cpe_device_g997.h"
#include "drv_dsl_cpe_device_danube.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_DEVICE


#ifndef DSL_DEBUG_DISABLE
/* Blacklist for Danube message dump */
DSL_DEV_MsgIdRange_t g_DANUBE_MsgDumpBlacklist[]=
{
   /* GroupId              First Adr   Last Adr */
   { DSL_CMV_GROUP_CNTL,        2,          2     },
   { DSL_CMV_GROUP_STAT,        0,          1     },
   { DSL_CMV_GROUP_STAT,       14,         18     },
   { DSL_CMV_GROUP_INFO,       30,         33     },
   { DSL_CMV_GROUP_INFO,       81,         83     },
   { DSL_CMV_GROUP_RATE,        0,          1     },
   { DSL_CMV_GROUP_PLAM,        0,         19     },
   { DSL_CMV_GROUP_PLAM,       24,         28     },
   { DSL_CMV_GROUP_PLAM,       33,         37     },
   { DSL_CMV_GROUP_PLAM,       42,         45     },
   { DSL_CMV_GROUP_CNFG,       12,         28     },
   { -1,                   0xFFFF,     0xFFFF     }
};
#endif /* DSL_DEBUG_DISABLE*/

#ifdef INCLUDE_CALLBACK_SUPPORT
static DSL_int_t DSL_DRV_HandleEventCallback
(
   DSL_DEV_Device_t *pDev,
   DSL_BSP_CB_Type_t nCallbackType,
   DSL_BSP_CB_DATA_Union_t *pData
);
#endif /* #ifdef INCLUDE_CALLBACK_SUPPORT */

static DSL_Error_t DSL_DRV_DANUBE_CRATES2OptionGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint8_t *pOption);

static DSL_Error_t DSL_DRV_DANUBE_ChannelRateUpdate(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection);

static DSL_Error_t DSL_DRV_DANUBE_ModemReset(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_DANUBE_LineFeaturesUpdate(
   DSL_Context_t *pContext);

/* This routine sends ATSE configuration to the FW */
static DSL_Error_t DSL_DRV_DANUBE_XTUSystemEnablingConfigSend
(
   DSL_IN DSL_Context_t *pContext
);

static DSL_Error_t DSL_DRV_DANUBE_AttTotalDataRateGet
(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pAttTotalDataRate
);

/*
   This function checks the System Interface Configuration parameters
   against supported by firmware
*/
static DSL_Error_t DSL_DRV_DANUBE_SystemInterfaceConfigCheck
(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceConfigData_t *pData
);

/* This routine retreives current mode from the the FW and
    stores it in the DSL Context structure */
static DSL_Error_t DSL_DRV_DANUBE_XTUSystemEnablingStatusUpdate
(
   DSL_IN_OUT DSL_Context_t *pContext
);

/* This routine retreives current failure state and
    stores it in the DSL Context structure */
static DSL_Error_t DSL_DRV_DANUBE_LineFailuresUpdate
(
   DSL_IN_OUT DSL_Context_t *pContext
);

/* This function should be called when line is reached showtime state */
DSL_Error_t DSL_DRV_DANUBE_ShowtimeReached
(
   DSL_Context_t *pContext
);

/*
   This routine updates the version information for currently used
   - chip
   - low level MEI BSP driver
   and stores it within device specific context.

   \param pContext
      pointer to the DSL context
*/
DSL_Error_t DSL_DRV_DANUBE_LowLevelVersionsUpdate
(
   DSL_Context_t *pContext
);

/*
   This routine updates the version information for currently used firmware
   within device specific context.

   \param pContext
      pointer to the DSL context
*/
static DSL_Error_t DSL_DRV_DANUBE_FirmwareVersionUpdate
(
   DSL_Context_t *pContext
);

/*
   This routine writes common firmware configurations that has to be done
   between firmware download and link start.
   \attention All of this configurations will be written to the software
              independently of the autoboot thread state, means also if
              autoboot thread is disabled by using 'acs 0'.

   \param pContext
      pointer to the DSL context
*/
static DSL_Error_t DSL_DRV_DANUBE_FirmwareInit
(
   DSL_Context_t *pContext
);

/**
   Checks the currently used firmware version against the given one and returns
   the status.

   \param pContext
      pointer to the DSL context
   \param nMajVer
      Major version specifies the device type of the firmware to check.
      - 2: Danube
      - 3: ASE
      - 4: AR9
      In case of giving -1 this value will be not processed.
   \param nMinVer
      Minor version specifies the Feature Set of the firmware to check.
   \param nExtVer
      External version of firmware version to check. In case of giving -1 this
      value will be not processed.
   \param nIntVer
      Internal version of firmware version to check. In case of giving -1 this
      value will be not processed.
   \param nApp
      Application version of firmware version to check. In case of giving -1
      this value will be not processed.
   \param pVerCheck
      Returns the status of the version comparison

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
*/

static DSL_Error_t DSL_DRV_DANUBE_FirmwareVersionCheck
(
   DSL_Context_t *pContext,
   DSL_int_t nMajVer,
   DSL_int_t nMinVer,
   DSL_int_t nExtVer,
   DSL_int_t nIntVer,
   DSL_FirmwareAnnex_t nApp,
   DSL_DEV_VersionCheck_t *pVerCheck
);

/*
 * Get the hdlc status
 *
 * \return  HDLC status
 * \ingroup Internal
 */
static DSL_Error_t DSL_DRV_DANUBE_HdlcStatusGet (
   DSL_Context_t *pContext,
   DSL_uint16_t *pnStatus
);

/*
 * Check if the me is resolved.
 *
 * \param   status      the me status
 * \return  ME_HDLC_UNRESOLVED or ME_HDLC_RESOLVED
 * \ingroup Internal
 */
static DSL_int_t DSL_DRV_DANUBE_HdlcResolvedGet(
   DSL_Context_t *pContext,
   DSL_uint16_t nStatus
);

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
static DSL_Error_t DSL_DRV_DANUBE_OnLineInventoryFe
(
   DSL_Context_t *pContext
);
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)
static DSL_Error_t DSL_DRV_DANUBE_FailReasonGet
(
   DSL_Context_t *pContext
);
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)*/

#ifndef DSL_DEBUG_DISABLE
static const char *const gGroupName[] =
{
   "??", "CNTL", "STAT", "INFO", "TEST", "OPTN", "RATE", "PLAM", "CNFG"
};
#endif /* #ifndef DSL_DEBUG_DISABLE*/

static DSL_Error_t DSL_DRV_DANUBE_FirmwareInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_CALLBACK_SUPPORT
   DSL_DEV_VersionCheck_t nFwVer = DSL_VERSION_ERROR;
#endif
   DSL_ChipVersion_t chipVersion;
   DSL_FwVersion_t firmwareVersion;
   DSL_uint16_t nVal = 0;

#ifdef INCLUDE_CALLBACK_SUPPORT
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext, DSL_MIN_FW_VERSION_CB,
      DSL_FW_ANNEX_NA, &nFwVer);

   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

   if (nFwVer >= DSL_VERSION_EQUAL)
   {
      /* enable Event Interrupts*/
      nVal = 0x1;
      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_OPTN,
         DSL_CMV_ADDRESS_OPTN_EVENT_INTS_CTRL, 0, 1, &nVal) != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
           "DSL[%02d]: ERROR - intrrupts enable failed!" DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));

         return DSL_ERR_MSG_EXCHANGE;
      }

      /* configure reboot interrupt handler */
      /* Default value. Also please refer to the SMS00759304 issue*/
      nVal = 0x36;
      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_REBOOT_INT_CONFIG, 0, 1, &nVal) != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - interrupts configuration failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ERR_MSG_EXCHANGE;
      }
   }
#endif /* INCLUDE_CALLBACK_SUPPORT */

   /* Workaround for Amazon-SE firmware problem with device version 1.2 */
   nVal = 0;

   /* Get Chipset version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.chipVersion, chipVersion);
   /* Get FW version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, firmwareVersion);

   if (firmwareVersion.bValid == DSL_FALSE)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - No firmware version available" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_WRN_DEVICE_NO_DATA;
   }

   if (firmwareVersion.nMajorVersion == DSL_AMAZON_SE_FW_MAJOR_NUMBER)
   {
      if (chipVersion.bValid == DSL_FALSE)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: No chip version available to handle A12 workaround!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         return DSL_WRN_DEVICE_NO_DATA;
      }

      if (chipVersion.nMajorVersion != 1)
      {
         DSL_DEBUG(DSL_DBG_WRN, (pContext,
            "DSL[%02d]: A12 workaround not necessary." DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_SUCCESS;
      }

      if (chipVersion.nMinorVersion == 1)
      {
         /* Activate workaround*/
         nVal = 0x0;
      }
      else
      {
         /* Deactivate workaround*/
         nVal = 0x1;
      }

      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
             DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE, 5, 1, &nVal) != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Failed to send CMV to activate A12 workaround!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         return DSL_ERR_MSG_EXCHANGE;
      }
   }


   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_FirmwareVersionCheck(
   DSL_Context_t *pContext,
   DSL_int_t nMajVer,
   DSL_int_t nMinVer,
   DSL_int_t nExtVer,
   DSL_int_t nIntVer,
   DSL_FirmwareAnnex_t nApp,
   DSL_DEV_VersionCheck_t *pVerCheck)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t nActUsedVer = 0, nVer = 0;
   DSL_FwVersion_t firmwareVersion;

   DSL_CHECK_POINTER(pContext, pVerCheck);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DANUBE_FirmwareVersionCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   *pVerCheck = DSL_VERSION_ERROR;

   /* Get FW version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, firmwareVersion);

   if (firmwareVersion.bValid == DSL_FALSE)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Firmware version check not possible" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_DEVICE_NO_DATA;
   }

   /* First check if the Major Version (device) and application types (Annex)
      matches or not */
   if ((nApp != -1 && firmwareVersion.nApplicationType != nApp) ||
       (nMajVer != -1 && firmwareVersion.nMajorVersion != nMajVer))
   {
      *pVerCheck = DSL_VERSION_MISMATCH;
      return nErrCode;
   }

   /* Build 32-bit value for given firmware version (nVer) and
      for currently used firmware version (nActUsedVer) */
   nVer = ((DSL_uint32_t)nMinVer) << 16;
   nActUsedVer = ((DSL_uint32_t)(firmwareVersion.nMinorVersion)) << 16;
   if (nExtVer != -1)
   {
      nVer |= ((DSL_uint32_t)nExtVer) << 8;
      nActUsedVer |= ((DSL_uint32_t)firmwareVersion.nExternalVersion) << 8;
   }
   if (nIntVer != -1)
   {
      nVer |= (DSL_uint32_t)nIntVer;
      nActUsedVer |= (DSL_uint32_t)firmwareVersion.nInternalVersion;
   }

   /* Check both generated 32-bit values */
   if (nActUsedVer < nVer)
   {
      *pVerCheck = DSL_VERSION_SMALLER;
   }
   else if (nActUsedVer > nVer)
   {
      *pVerCheck = DSL_VERSION_BIGGER;
   }
   else
   {
      *pVerCheck = DSL_VERSION_EQUAL;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DANUBE_FirmwareVersionCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

DSL_DEV_Handle_t DSL_DRV_DEV_DriverHandleGet(DSL_int_t nMaj, DSL_int_t nNum)
{
#ifdef INCLUDE_CALLBACK_SUPPORT
   DSL_BSP_EventCallBack_t evtCB;
   DSL_int_t i = 0;
#endif

   DSL_DEV_Handle_t tmp = DSL_BSP_DriverHandleGet(nMaj, nNum);

#ifdef INCLUDE_CALLBACK_SUPPORT
   /* Temporarily because of neccessary clarification within low level driver */
   DSL_DRV_MemSet(&evtCB, 0, sizeof(DSL_BSP_EventCallBack_t));
   evtCB.function = DSL_DRV_HandleEventCallback;
   evtCB.pData = DSL_NULL;

   for (i = (DSL_BSP_CB_FIRST + 1); i < DSL_BSP_CB_LAST; i++)
   {
      evtCB.event = (DSL_BSP_CB_Type_t)i;
      if (DSL_BSP_EventCBRegister(&evtCB) != 0)
      {
         DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL,
            "DSL[%02d]: DSL_BSP_DriverHandleGet: Regiser Event Callback (Type=%d) failed!"
            DSL_DRV_CRLF, nNum,i));
      }
      else
      {
         DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "DSL[%02d]: DSL_BSP_DriverHandleGet: Regiser "
            "Event Callback (Type=%d) successful"DSL_DRV_CRLF, nNum, i));
      }
   }
#endif /* INCLUDE_CALLBACK_SUPPORT */

   return tmp;
}

DSL_Error_t DSL_DRV_DEV_DriverHandleDelete(
   DSL_DEV_Handle_t handle)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_CALLBACK_SUPPORT
   DSL_BSP_EventCallBack_t evtCB;
   DSL_int_t i = 0;

   /* Temporarily because of neccessary clarification within low level driver */
   DSL_DRV_MemSet(&evtCB, 0, sizeof(DSL_BSP_EventCallBack_t));
   evtCB.function = DSL_NULL;
   evtCB.pData = DSL_NULL;

   for (i = (DSL_BSP_CB_FIRST + 1); i < DSL_BSP_CB_LAST; i++)
   {
      evtCB.event = (DSL_BSP_CB_Type_t)i;
      if (DSL_BSP_EventCBUnregister(&evtCB) != 0)
      {
         DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL,
            "DSL_DEV_DriverHandleDelete: Unregiser Event Callback (Type=%d) Fail!"
            DSL_DRV_CRLF, i));

         nErrCode = DSL_ERROR;
      }
      else
      {
         DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "DSL_DEV_DriverHandleDelete: Event"
            "Callback (Type=%d) unregistered successfully." DSL_DRV_CRLF, i));
      }
   }
#endif /* INCLUDE_CALLBACK_SUPPORT */

   DSL_BSP_DriverHandleDelete(handle);
   return nErrCode;
}

#ifdef INCLUDE_CALLBACK_SUPPORT
/* handles events from the low level driver, it is called "new style" in order
   to split it from existing callback handling, later it should be renamed */
static DSL_int_t DSL_DRV_HandleEventCallback(
   DSL_DEV_Device_t *pDev,
   DSL_BSP_CB_Type_t nCallbackType,
   DSL_BSP_CB_DATA_Union_t *pData)
{
   DSL_int_t i = 0;
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if (pDev == DSL_NULL)
   {
      return -EIO;
   }

   if (ifxDevices[i].pContext != DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_MSG, (ifxDevices[i].pContext,
         "DSL[%02d]: DSL_DRV_HandleEventCallback: Callback event (type=%d) occured" DSL_DRV_CRLF,
         DSL_DEV_NUM(ifxDevices[i].pContext), nCallbackType));
   }

   switch (nCallbackType)
   {
   case DSL_BSP_CB_DYING_GASP:
      /* go through all devices we registered and determine for which of
         them an event has been occured */
      for (i = 0; i < DSL_DRV_MAX_DEVICE_NUMBER; i++)
      {
         if (ifxDevices[i].lowHandle == pDev)
         {
            DSL_DEBUG(DSL_DBG_ERR, (ifxDevices[i].pContext,
               "DSL[%02d]: DSL_DRV_HandleEventCallback: Got Dying Gasp interrupt"
               DSL_DRV_CRLF, DSL_DEV_NUM(ifxDevices[i].pContext)));
         }
      }
      break;
#if defined(INCLUDE_DSL_CEOC)
   case DSL_BSP_CB_CEOC_IRQ:
      /* go through all devices we registered and determine for which of
         them an event has been occured */
      for (i = 0; i < DSL_DRV_MAX_DEVICE_NUMBER; i++)
      {
         if (ifxDevices[i].lowHandle == pDev)
         {
            DSL_DEBUG(DSL_DBG_MSG, (ifxDevices[i].pContext,
               "DSL[%02d]: DSL_DRV_HandleEventCallback: Got CEOC interrupt" DSL_DRV_CRLF,
               DSL_DEV_NUM(ifxDevices[i].pContext)));

            nErrCode = DSL_CEOC_EventCB(ifxDevices[i].pContext);
         }
      }
      break;
#endif /* defined(INCLUDE_DSL_CEOC)*/
   case DSL_BSP_CB_FIRMWARE_REBOOT:
      /* go through all devices we registered and determine for which of
         them an event has been occured */
      for (i = 0; i < DSL_DRV_MAX_DEVICE_NUMBER; i++)
      {
         if (ifxDevices[i].lowHandle == pDev)
         {
            /* Set MEI Reboot indication flag*/
            ifxDevices[i].pContext->bMeiReboot = DSL_TRUE;

            /* ...and wakeup immediately the Autoboot thread*/
            DSL_DRV_WAKEUP_EVENT(ifxDevices[i].pContext->autobootEvent);
         }
      }
      break;
   default:
      /* skip an event */
      DSL_DEBUG(DSL_DBG_ERR, (ifxDevices[i].pContext,
         "DSL[%02d]: DSL_DRV_HandleEventCallback: Unknown/unhandled event type (%d)" DSL_DRV_CRLF,
         DSL_DEV_NUM(ifxDevices[i].pContext), nCallbackType));
      break;
   }

   return (nErrCode == DSL_SUCCESS ? 0 : -EFAULT);
}
#endif /* INCLUDE_CALLBACK_SUPPORT */

DSL_Error_t DSL_DRV_DANUBE_LowLevelVersionsUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_MeiSwVersion_t meiSwVersion;
   DSL_ChipVersion_t chipVersion;
   DSL_DEV_Version_t bspVersion = { 0 };
   DSL_DEV_HwVersion_t devVersion = { 0 };

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DANUBE_LowLevelVersionsUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_GET_VERSION,
      (DSL_uint32_t *)&bspVersion) < DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Failed to get BSP version!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_LOW_LEVEL_DRIVER_ACCESS;
   }

   meiSwVersion.nMajorVersion = (DSL_uint8_t)bspVersion.major;
   meiSwVersion.nMinorVersion = (DSL_uint8_t)bspVersion.minor;
   meiSwVersion.nBuild        = (DSL_uint8_t)bspVersion.revision;
   meiSwVersion.bValid        = DSL_TRUE;

   /* Update MEI version in the CPE API Context*/
   DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.version.meiSwVersion, meiSwVersion);

   nErrCode = DSL_DRV_DEV_BspIoctl(
                 pContext, DSL_FIO_BSP_GET_CHIP_INFO, (DSL_uint32_t *)&devVersion);

   if (nErrCode < DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Failed to get chip version!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_LOW_LEVEL_DRIVER_ACCESS;
   }
   else if ((devVersion.major == 0) && (devVersion.minor == 0))
   {
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: Got no valid chip version from LL driver" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_WRN_DEVICE_NO_DATA;
   }

   chipVersion.nMajorVersion = (DSL_uint8_t)devVersion.major;
   chipVersion.nMinorVersion = (DSL_uint8_t)devVersion.minor;
   chipVersion.bValid        = DSL_TRUE;

   /* Update Chipset version in the CPE API Context*/
   DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.version.chipVersion, chipVersion);

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DANUBE_LowLevelVersionsUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_FirmwareVersionUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_FwVersion_t fwVersion;
   DSL_uint16_t nValVer[2];

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DANUBE_FirmwareVersionUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Read the version */
   if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_VERSION, 0, 2, nValVer) != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Failed to get FW version!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_MSG_EXCHANGE;
   }

   /* Extract the version information from CMVs and store it within context */
   fwVersion.nMajorVersion    = (DSL_uint8_t)(nValVer[0] & 0xFF);
   fwVersion.nMinorVersion    = (DSL_uint8_t)((nValVer[0] >> 8) & 0xFF);
   fwVersion.nExternalVersion = (DSL_uint8_t)((nValVer[1] >> 4) & 0xF);
   fwVersion.nInternalVersion = (DSL_uint8_t)(nValVer[1] & 0xF);
   fwVersion.nReleaseState    = (DSL_uint8_t)((nValVer[1] >> 14) & 0x3);
   fwVersion.nApplicationType = (DSL_uint8_t)((nValVer[1] >> 8) & 0x3F);
   fwVersion.bValid           = DSL_TRUE;

   /* Update FW version in the CPE API Context*/
   DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.version.fwVersion, fwVersion);

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DANUBE_FirmwareVersionUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DANUBE_UsedTonesNumberGet(
   DSL_Context_t *pContext,
   const DSL_AccessDir_t nDirection,
   DSL_uint16_t *pTonesNum)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t *XTSE = DSL_NULL;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_UsedTonesNumberGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pTonesNum);
   DSL_CHECK_ERR_CODE();

   /*Get current xTSE octets*/
   XTSE = pContext->xtseCurr;

   /*
   ADSL   Annex   US/DS
   1      A       0...32/0...256
   1      B       0...64/0...256
   1      C       0...64/0...256
   2      A,I,L   0...32/0...256
   2      B,J,M   0...64/0...256
   2+     A,I,L   0...32/0...512
   2+     B,J,M   0...64/0...512
   */
   if (nDirection == DSL_DOWNSTREAM)
   {
      if ( (XTSE[6-1] & (XTSE_6_01_A_5_NO |  XTSE_6_03_B_5_NO | XTSE_6_07_I_5_NO)) ||
           (XTSE[7-1] & (XTSE_7_01_J_5_NO |  XTSE_7_03_M_5_NO)))
      {
         *pTonesNum = 512;
      }
      else if( (XTSE[1-1] & (XTSE_1_02_C_TS_101388)) ||
               (XTSE[1-1] & (XTSE_1_03_A_1_NO | XTSE_1_05_B_1_NO | XTSE_1_01_A_T1_413)) ||
               (XTSE[3-1] & (XTSE_3_03_A_3_NO |  XTSE_3_05_B_3_NO)) ||
               (XTSE[4-1] & (XTSE_4_05_I_3_NO |  XTSE_4_07_J_3_NO)) ||
               (XTSE[5-1] & (XTSE_5_03_L_3_NO | XTSE_5_04_L_3_NO | XTSE_5_07_M_3_NO)) )
      {
         *pTonesNum = 256;
      }
      else if ((XTSE[2-1] & XTSE_2_01_A_2_NO))
      {
         *pTonesNum = 128;
      }
      else
      {
         *pTonesNum = 0;

         nErrCode = DSL_ERROR;

         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - XTSE current settings not detected!"DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));
      }
   }
   else
   {
      if ( (XTSE[1-1] & (XTSE_1_02_C_TS_101388)) ||
           (XTSE[1-1] & (XTSE_1_05_B_1_NO)) ||
           (XTSE[3-1] & (XTSE_3_05_B_3_NO)) ||
           (XTSE[4-1] & (XTSE_4_07_J_3_NO)) ||
           (XTSE[5-1] & (XTSE_5_07_M_3_NO)) ||
           (XTSE[6-1] & (XTSE_6_03_B_5_NO)) ||
           (XTSE[7-1] & (XTSE_7_01_J_5_NO | XTSE_7_03_M_5_NO)) )
      {
         *pTonesNum = 64;
      }
      else if( (XTSE[1-1] & (XTSE_1_03_A_1_NO | XTSE_1_01_A_T1_413)) ||
               (XTSE[3-1] & (XTSE_3_03_A_3_NO)) ||
               (XTSE[2-1] & (XTSE_2_01_A_2_NO)) ||
               (XTSE[4-1] & (XTSE_4_05_I_3_NO )) ||
               (XTSE[5-1] & (XTSE_5_03_L_3_NO |  XTSE_5_04_L_3_NO)) ||
               (XTSE[6-1] & (XTSE_6_01_A_5_NO |  XTSE_6_07_I_5_NO)) )
      {
         *pTonesNum = 32;
      }
      else
      {
         *pTonesNum = 0;

         nErrCode = DSL_ERROR;

         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - XTSE current settings not detected!"DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_UsedTonesNumberGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return (nErrCode);
}

DSL_Error_t DSL_DRV_DEV_Reboot(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bModemReady = DSL_FALSE;
   DSL_int32_t nWaitCount = 0;
#if defined(INCLUDE_DSL_PM)
   DSL_uint32_t fwDwnldStartTime = 0, fwDwnldStopTime = 0, fwDwnldTime = 0;
#endif

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DEV_Reboot"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->bFirmwareReady, DSL_FALSE);

#if defined(INCLUDE_DSL_PM)
   fwDwnldStartTime = DSL_DRV_ElapsedTimeMSecGet(0);
#endif
   /* Reboot device*/
   if (DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_REBOOT, DSL_NULL) != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - device DSL_FIO_BSP_HALT failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   /* Start device*/
   nErrCode = DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_DSL_START, DSL_NULL);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - device DSL_FIO_BSP_START failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

#if defined(INCLUDE_DSL_PM)
   fwDwnldStopTime = DSL_DRV_ElapsedTimeMSecGet(0);
   fwDwnldTime     = fwDwnldStartTime > fwDwnldStopTime ?
                     fwDwnldStopTime :
                     ((fwDwnldStopTime - fwDwnldStartTime)/1000);

   /* Set FW unavailable time*/
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nFwUnavailableTime, fwDwnldTime);
#endif

   /* Poll for modem ready*/
   /* wait for Modem Ready state max 4 sec */
   for (nWaitCount = 0; nWaitCount < 40; nWaitCount++)
   {
      bModemReady = DSL_DRV_DEV_ModemIsReady(pContext);
      if (bModemReady)
      {
         break;
      }

      DSL_DRV_MSecSleep(100);
   }


   if (!bModemReady)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Modem not Ready!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }
   else
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->bFirmwareReady, DSL_TRUE);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DEV_Reboot"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DEV_FwDownload(
   DSL_Context_t *pContext,
   const DSL_char_t *pFw1,
   DSL_uint32_t nSize1,
   const DSL_char_t *pFw2,
   DSL_uint32_t nSize2,
   DSL_int32_t *pLoff,
   DSL_int32_t *pCurrentOff,
   DSL_boolean_t bLastChunk)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#if defined(INCLUDE_DSL_PM)
   DSL_uint32_t fwDwnldStartTime = 0, fwDwnldStopTime = 0, fwDwnldTime = 0;
#endif
   DSL_FwDownloadStatusData_t fwDwnlStatus;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DEV_FwDownload"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->bFirmwareReady, DSL_FALSE);
#if defined(INCLUDE_DSL_PM)
   fwDwnldStartTime = DSL_DRV_ElapsedTimeMSecGet(0);
#endif

   if (pFw1 == DSL_NULL || pFw2 != DSL_NULL
        || nSize1 == 0 || nSize2 != 0
        || pLoff == DSL_NULL || pCurrentOff == DSL_NULL )
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "DSL[%02d]: wrong parameters is passed "
         "to DSL_DRV_DEV_FwDownload(%p %p %p %p %d %d)" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext) ,pFw1, pFw2,
         pLoff, pCurrentOff, nSize1, nSize2));

      return DSL_ERR_INVALID_PARAMETER;
   }

   if (*pLoff == 0)
   {
      if ((nErrCode = DSL_DRV_DANUBE_ModemReset(pContext)) != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Could not reset a modem!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }
   }

   /* Lock driver access*/
   if (DSL_DRV_MUTEX_LOCK(pContext->bspMutex))
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, DSL_DRV_CRLF"DSL[%02d]: ERROR - getting mei driver semaphore failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   if (nErrCode == DSL_SUCCESS
       && DSL_BSP_FWDownload(pContext->pDevCtx->lowHandle, pFw1, nSize1,
      pLoff, pCurrentOff) != DSL_DEV_MEI_ERR_SUCCESS)
   {
      nErrCode = DSL_ERROR;
   }

   /* Unlock driver access*/
   DSL_DRV_MUTEX_UNLOCK(pContext->bspMutex);

   if (nErrCode == DSL_SUCCESS && bLastChunk != DSL_FALSE)
   {
      /* Start modem*/
      nErrCode = DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_DSL_START, DSL_NULL);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - couldn't start modem!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
      else
      {
         DSL_DRV_DANUBE_FirmwareVersionUpdate(pContext);

#if defined(INCLUDE_DSL_PM)
         fwDwnldStopTime = DSL_DRV_ElapsedTimeMSecGet(0);;
         fwDwnldTime     = fwDwnldStartTime > fwDwnldStopTime ?
                           fwDwnldStopTime :
                           ((fwDwnldStopTime - fwDwnldStartTime)/1000);

         /* Set FW unavailable time*/
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nFwUnavailableTime, fwDwnldTime);
#endif

         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->bFirmwareReady, DSL_TRUE);

         /* Fill DSL_EVENT_S_FIRMWARE_DOWNLOAD_STATUS event structure*/
         fwDwnlStatus.nError  = nErrCode == DSL_SUCCESS ? DSL_FW_LOAD_SUCCESS: DSL_FW_LOAD_ERROR;
         fwDwnlStatus.nFwType = DSL_FW_REQUEST_ADSL;

         if( DSL_DRV_EventGenerate( pContext, 0, DSL_ACCESSDIR_NA,
                DSL_XTUDIR_NA, DSL_EVENT_S_FIRMWARE_DOWNLOAD_STATUS,
               (DSL_EventData_Union_t*)&fwDwnlStatus, sizeof(DSL_FwDownloadStatusData_t))
             != DSL_SUCCESS )
         {
             DSL_DEBUG( DSL_DBG_ERR,
                (pContext, "DSL[%02d]: ERROR - Event(%d) generate failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_EVENT_S_FIRMWARE_DOWNLOAD_STATUS));
         }
      }
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DEV_FwDownload, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_boolean_t DSL_DRV_DEV_ModemIsReady(DSL_Context_t *pContext)
{
   DSL_uint32_t nModemReady = 0;
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   if (nErrCode != DSL_SUCCESS)
   {
      return DSL_FALSE;
   }

   if (DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_IS_MODEM_READY, &nModemReady) !=
      DSL_SUCCESS)
   {
      return DSL_FALSE;
   }

   return (nModemReady == 1) ? DSL_TRUE : DSL_FALSE;
}

DSL_Error_t DSL_DRV_DANUBE_ActLatencyGet(
   DSL_Context_t *pContext,
   DSL_uint16_t nChannel,
   DSL_AccessDir_t nDirection,
   DSL_LatencyPath_t *nLPath)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if (nChannel >= DSL_CHANNELS_PER_LINE)
   {
      DSL_DEBUG (DSL_DBG_ERR, (pContext, "DSL[%02d]: DSL_DRV_DANUBE_ActLatencyGet - "
         "invalid channel (%d)!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nChannel));

      return DSL_ERROR;
   }

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][nChannel], *nLPath);

   /* Do not process disabled or invalid states */
   switch (*nLPath)
   {
   case DSL_LATENCY_DISABLED:
      nErrCode = DSL_ERROR;
      break;
   case DSL_LATENCY_IP_LP0:
   case DSL_LATENCY_FP_LP1:
      break;
   default:
      nErrCode = DSL_ERROR;
      break;
   }

   return nErrCode;
}

static DSL_boolean_t DSL_DRV_DANUBE_CmvAccessTypeGet(DSL_uint16_t cmv_word0)
{
   DSL_boolean_t bWrite = DSL_FALSE;

   switch ( (cmv_word0 >> 4) & 0xFF )
   {
   case DSL_CMV_OPCODE_H2D_CMV_WRITE:
   case DSL_CMV_OPCODE_D2H_CMV_WRITE_REPLY:
   case DSL_CMV_OPCODE_H2D_DEBUG_WRITE_DM:
   case DSL_CMV_OPCODE_D2H_DEBUG_WRITE_DM_REPLY:
   case DSL_CMV_OPCODE_H2D_DEBUG_WRITE_PM:
   case DSL_CMV_OPCODE_D2H_DEBUG_WRITE_FM_REPLY:
      bWrite = DSL_TRUE;
      break;
   default:
      break;
   }

   return bWrite;
}

static DSL_uint16_t DSL_DRV_DANUBE_CmvByteCountGet(DSL_uint16_t cmv_word0)
{
   DSL_uint16_t nSize = 0, nBitSize = 0, nCount = 0;

   nBitSize = cmv_word0 & 0xC000;
   nCount   = cmv_word0 & 0xF;

   switch (nBitSize)
   {
   case DSL_CPE_CMV_BYTE_SIZE:
      nSize = nCount + DSL_CMV_HEADER_LENGTH * 2;
      break;
   case DSL_CPE_CMV_DWORD_SIZE:
      nSize = (DSL_uint16_t)(nCount * 4 + DSL_CMV_HEADER_LENGTH * 2);
      break;
   case DSL_CPE_CMV_WORD_SIZE:
   default:
      nSize = (DSL_uint16_t)(nCount * 2 + DSL_CMV_HEADER_LENGTH * 2);
      break;
   }

   return nSize;
}

/*
   Reads a CMV message.

   \param pContext Pointer to DSL CPE API context structure, [I]
   \param nGroup   Message group number, [I]
   \param nAddress Message address, [I]
   \param nIndex   Message index, [I]
   \param nSize    Number of words to read, [I]
   \param pData    Pointer to the data in case of writing a CMV. In case of
                   reading a CMV is shall be DSL_NULL, [I]
   \param pMsg     Pointer to the CMV data that has been returned by the
                   firmware (excluding header info), [O]

   \ingroup Internal
*/
DSL_Error_t DSL_DRV_DANUBE_CmvRead (
   DSL_Context_t *pContext,
   DSL_uint8_t nGroup,
   DSL_uint16_t nAddress,
   DSL_uint16_t nIndex,
   DSL_int_t nSize,
   DSL_uint16_t *pData)
{
#ifndef WIN32
   DSL_uint16_t RxMessage[DSL_MAX_CMV_MSG_LENGTH]__attribute__ ((aligned(4)));
   DSL_uint16_t TxMessage[DSL_MAX_CMV_MSG_LENGTH]__attribute__ ((aligned(4)));
#else
   DSL_uint16_t RxMessage[DSL_MAX_CMV_MSG_LENGTH];
   DSL_uint16_t TxMessage[DSL_MAX_CMV_MSG_LENGTH];
#endif /* WIN32*/
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t i = 0;

   DSL_DRV_MemSet(&RxMessage, 0, sizeof(RxMessage));
   DSL_DRV_MemSet(&TxMessage, 0, sizeof(TxMessage));

   DSL_DRV_DANUBE_CmvPrepare(pContext, DSL_CMV_OPCODE_H2D_CMV_READ, nGroup,
      nAddress, nIndex, nSize, DSL_NULL, TxMessage);

   nErrCode = DSL_DRV_DANUBE_CmvSend(pContext, TxMessage, DSL_TRUE, RxMessage);
   if (nErrCode != DSL_SUCCESS)
   {
#ifndef DSL_DEBUG_DISABLE
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CMV send failed, read %s %d %d!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), gGroupName[nGroup], nAddress, nIndex));
#else
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CMV send failed, read %d %d %d!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nGroup, nAddress, nIndex));
#endif
   }
   else
   {
      for (i = 0; i < nSize; i++)
      {
         *(pData+i) = RxMessage[i+4];
      }
   }

   return nErrCode;
}

/*
   Writes a CMV message.

   \param pContext Pointer to DSL CPE API context structure, [I]
   \param nGroup   Message group number, [I]
   \param nAddress Message address, [I]
   \param nIndex   Message index, [I]
   \param nSize    Number of words to write, [I]
   \param pData    Pointer to the data in case of writing a CMV. In case of
                   reading a CMV is shall be DSL_NULL, [I]
   \param pMsg     Pointer to the CMV data that has been returned by the
                   firmware (excluding header info), [O]

   \ingroup Internal
*/
DSL_Error_t DSL_DRV_DANUBE_CmvWrite (
   DSL_Context_t *pContext,
   DSL_uint8_t nGroup,
   DSL_uint16_t nAddress,
   DSL_uint16_t nIndex,
   DSL_int_t nSize,
   DSL_uint16_t *pData)
{
#ifndef WIN32
   DSL_uint16_t RxMessage[DSL_MAX_CMV_MSG_LENGTH]__attribute__ ((aligned(4)));
   DSL_uint16_t TxMessage[DSL_MAX_CMV_MSG_LENGTH]__attribute__ ((aligned(4)));
#else
   DSL_uint16_t RxMessage[DSL_MAX_CMV_MSG_LENGTH];
   DSL_uint16_t TxMessage[DSL_MAX_CMV_MSG_LENGTH];
#endif /* WIN32*/
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DRV_MemSet(&RxMessage, 0, sizeof(RxMessage));
   DSL_DRV_MemSet(&TxMessage, 0, sizeof(TxMessage));

   DSL_DRV_DANUBE_CmvPrepare(pContext, DSL_CMV_OPCODE_H2D_CMV_WRITE, nGroup,
      nAddress, nIndex, nSize, pData, TxMessage);

   nErrCode = DSL_DRV_DANUBE_CmvSend(pContext, TxMessage, DSL_TRUE, RxMessage);
   if (nErrCode != DSL_SUCCESS)
   {
#ifndef DSL_DEBUG_DISABLE
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CMV send failed, write %s %d %d!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), gGroupName[nGroup], nAddress, nIndex));
#else
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CMV send failed, write %d %d %d!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nGroup, nAddress, nIndex));
#endif
   }

   return nErrCode;
}

#ifndef DSL_DEBUG_DISABLE
/*
   Checks if a firmware message has to be printed or not.
   This function takes into account
   + debug level of message dump module
   + defined message blacklist in case of 0x00 < DbgLvl < 0x80

   \param pContext Pointer to DSL CPE API context structure, [I]
   \param nGroup   Message group number, [I]
   \param nAddress Message address, [I]

   \ingroup Internal
 */
static DSL_boolean_t DSL_DRV_DANUBE_CmvCheckPrint(
   DSL_Context_t *pContext,
   const DSL_uint16_t *pMsg)
{
   DSL_DEV_MsgIdRange_t *pTable = &g_DANUBE_MsgDumpBlacklist[0];
   DSL_DBG_ModuleLevel_t dbgModuleLevel;
   DSL_boolean_t bPrint = DSL_TRUE, bWrite = DSL_FALSE;
   DSL_uint8_t nGroup = 0, nDir = 0;
   DSL_uint16_t nAddress = 0;

   /* Get module debug level*/
   DSL_DRV_MemSet(&dbgModuleLevel, 0, sizeof(dbgModuleLevel));
   dbgModuleLevel.data.nDbgModule = DSL_DBG_MESSAGE_DUMP;
   DSL_DRV_DBG_ModuleLevelGet(pContext, &dbgModuleLevel);

   /* Get message info*/
   nGroup   = (DSL_uint8_t)(*(pMsg + 1) & 0x7F);
   nAddress = *(pMsg + 2);
   /* nDir!=0 - Arc->ME, else ME->Arc*/
   nDir     = (*pMsg & 0x10);

   /* Check Rd/Wr access type*/
   bWrite = DSL_DRV_DANUBE_CmvAccessTypeGet(*pMsg);

   if (dbgModuleLevel.data.nDbgLevel > DSL_DBG_NONE)
   {
      if (dbgModuleLevel.data.nDbgLevel < DSL_DBG_MSG)
      {
         /* Check if the prepared message is listed within blacklist */
         for (bPrint = DSL_TRUE; pTable->nGroupId != -1; pTable++)
         {
            if ((DSL_int_t)nGroup != pTable->nGroupId)
            {
               continue;
            }

            if ((nAddress >= pTable->nAdrFirst) && (nAddress <= pTable->nAdrLast))
            {
               bPrint = DSL_FALSE;
               break;
            }
         }
      }
      else
      {
         bPrint = DSL_TRUE;
      }

      if (dbgModuleLevel.data.nDbgLevel != DSL_DBG_LOCAL && bPrint)
      {
         if ((bWrite && nDir) || (!bWrite && !nDir))
         {
            bPrint = DSL_FALSE;
         }
      }
   }
   else
   {
      bPrint = DSL_FALSE;
   }

   return bPrint;
}

static DSL_void_t DSL_DRV_DANUBE_CmvMessageDump(
   DSL_Context_t *pContext,
   const DSL_char_t *sName,
   const DSL_uint16_t *pMsg)
{
   DSL_boolean_t bPrint = DSL_TRUE;
   DSL_uint8_t nSize = 0, i = 0;

   bPrint = DSL_DRV_DANUBE_CmvCheckPrint(pContext, pMsg);

   if (bPrint)
   {
      /* Get CMV byte count*/
      nSize = (DSL_uint8_t)DSL_DRV_DANUBE_CmvByteCountGet(*pMsg);

      DSL_DRV_debug_printf(pContext, "DSL[%s]:",sName);

      /* Print message in the 16-bit format*/
      for (i = 0; i < (nSize/2 + nSize%2); i++)
      {
         DSL_DRV_debug_printf(pContext, " %04X", *(pMsg + i));
      }
      DSL_DRV_debug_printf(pContext, DSL_DRV_CRLF);
   }
}
#endif /* DSL_DEBUG_DISABLE*/

/*
   Compose a message.
   This function compose a message from opcode, group, address, index, size,
   and data

   \param pContext Pointer to DSL CPE API context structure, [I]
   \param nOpcode  Message opcode, [I]
   \param nGroup   Message group number, [I]
   \param nAddress Message address, [I]
   \param nIndex   Message index, [I]
   \param nSize    Number of words to read/write, [I]
   \param pData    Pointer to the data in case of writing a CMV. In case of
                   reading a CMV is shall be DSL_NULL, [I]
   \param pMsg     Pointer to composed message, [O]

   \ingroup Internal
 */
DSL_void_t DSL_DRV_DANUBE_CmvPrepare(
   DSL_Context_t *pContext,
   DSL_uint8_t nOpcode,
   DSL_uint8_t nGroup,
   DSL_uint16_t nAddress,
   DSL_uint16_t nIndex,
   DSL_int_t nSize,
   DSL_uint16_t *pData,
   DSL_uint16_t *pMsg)
{
   pMsg[0]= (nOpcode << 4) | (nSize & 0xf);
   pMsg[1]= (((nIndex==0) ? 0 : 1) << 7) | (nGroup & 0x7f);
   pMsg[2]= nAddress;
   pMsg[3]= nIndex;

   if ((nOpcode == DSL_CMV_OPCODE_H2D_CMV_WRITE) && (pData != NULL))
   {
      memcpy(pMsg+4, pData, nSize*2);
   }

   return;
}


/*
   Send a message to ARC and read the response
   This function sends a message to arc, waits the response, and reads the
   responses.

   \param pContext Pointer to DSL CPE API context structure, [I]
   \param pTxMsg   Pointer to CMV message that will be written, [I]
   \param reply    Specifies whether to wait on a firmware (DSL_TRUE) reply or
                   not (DSL_FALSE), [I]
   \param pRxMsg   Pointer to CMV message that has been returned by the firmware
                   as a reply to TxMsg, [O]

   \return  values from the DSL_Error_t enum

   \ingroup Internal
 */
DSL_Error_t DSL_DRV_DANUBE_CmvSend(
   DSL_Context_t *pContext,
   DSL_uint16_t *pTxMsg,
   DSL_boolean_t bReply,
   DSL_uint16_t *pRxMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if (pContext == DSL_NULL || pContext->pDevCtx == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifndef DSL_DEBUG_DISABLE
   /* Dump message*/
   DSL_DRV_DANUBE_CmvMessageDump(pContext,"tx",pTxMsg);
#endif /* DSL_DEBUG_DISABLE*/

   /* Lock driver access*/
   if (DSL_DRV_MUTEX_LOCK(pContext->bspMutex))
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, DSL_DRV_CRLF"DSL[%02d]: ERROR - getting mei driver semaphore failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   /* Send CMV*/
   if (DSL_BSP_SendCMV(pContext->pDevCtx->lowHandle, pTxMsg,
      (DSL_int_t)bReply, pRxMsg) != DSL_DEV_MEI_ERR_SUCCESS)
   {
      nErrCode = DSL_ERROR;
   }

   /* Unlock driver access*/
   DSL_DRV_MUTEX_UNLOCK(pContext->bspMutex);

#ifndef DSL_DEBUG_DISABLE
   /* Dump message*/
   DSL_DRV_DANUBE_CmvMessageDump(pContext,"rx",pRxMsg);
#endif /* DSL_DEBUG_DISABLE*/

   return nErrCode;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_LineFeaturesUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal = 0;
   DSL_FwVersion_t FwVer = {DSL_FALSE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
   DSL_boolean_t bVirtualNoiseSupport = DSL_FALSE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_LineFeaturesUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get DS Virtual Noise configuration data*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      lineFeatureDataCfg[DSL_DOWNSTREAM].bVirtualNoiseSupport,
      bVirtualNoiseSupport);
   /* Update DS Virtual Noise Status data*/
   DSL_CTX_WRITE_SCALAR(
      pContext, nErrCode, lineFeatureDataSts[DSL_DOWNSTREAM].bVirtualNoiseSupport,
      bVirtualNoiseSupport);

   /* Get US Virtual Noise configuration data*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      lineFeatureDataCfg[DSL_UPSTREAM].bVirtualNoiseSupport,
      bVirtualNoiseSupport);
   /* Update US Virtual Noise Status data*/
   DSL_CTX_WRITE_SCALAR(
      pContext, nErrCode, lineFeatureDataSts[DSL_UPSTREAM].bVirtualNoiseSupport,
      bVirtualNoiseSupport);

   /* Get FW  information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, FwVer);

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_OPTN,
                 DSL_CMV_ADDRESS_OPTN_ALG_CONTROL, 0, 1, &nVal);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
      lineFeatureDataSts[DSL_DOWNSTREAM].bBitswapEnable,
      (nVal & (1<<5)) ? DSL_FALSE : DSL_TRUE);

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                 DSL_CMV_ADDRESS_STAT_MISC, 0, 1, &nVal);

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
      lineFeatureDataSts[DSL_DOWNSTREAM].bTrellisEnable,
      (nVal & (1<<4)) ? DSL_TRUE : DSL_FALSE);

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
      lineFeatureDataSts[DSL_UPSTREAM].bTrellisEnable,
      (nVal & (1<<7)) ? DSL_TRUE : DSL_FALSE);

   /* Update DS Retransmission status*/
   if (FwVer.bValid == DSL_TRUE)
   {
      /* Check for the R4 FW*/
      if (FwVer.nMinorVersion >= 4)
      {
         nVal = 0;
         /* Get INFO 110 0 content*/
         if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
               DSL_CMV_ADDRESS_INFO_RETX_ERASURE_DECODING, 0, 1, &nVal)
               != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - INFO 110 read failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return DSL_ERROR;
         }

         DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
            lineFeatureDataSts[DSL_DOWNSTREAM].bReTxEnable,
            (nVal & (0x44)) ? DSL_TRUE : DSL_FALSE);
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: WARNING - No firmware version available. Retransmission status not updated"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_LineFeaturesUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   This function checks the System Interface Configuration parameters
   against supported by firmware
*/
static DSL_Error_t DSL_DRV_DANUBE_SystemInterfaceConfigCheck(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceConfigData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_SystemInterfaceConfigCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   /* System Interface selection check*/
   if (pData->nSystemIf != DSL_SYSTEMIF_MII)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unsupported SystemIf=%d selected!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pData->nSystemIf));

      nErrCode = DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
   }

   /* TC Layer selection check*/
   switch(pData->nTcLayer)
   {
   case DSL_TC_EFM:
   case DSL_TC_EFM_FORCED:
   case DSL_TC_AUTO:
      /* Upstream TC configuration selection check*/
      if ( pData->nEfmTcConfigUs &
           (~(DSL_EMF_TC_CLEANED | DSL_EMF_TC_NORMAL | DSL_EMF_TC_PRE_EMPTION)) )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Unsupported nEfmTcConfigUs=%d selected!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pData->nEfmTcConfigUs));

         nErrCode = DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
         break;
      }


      /* Downstream TC configuration selection check*/
      if ( pData->nEfmTcConfigDs &
           (~(DSL_EMF_TC_CLEANED | DSL_EMF_TC_NORMAL)) )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Unsupported nEfmTcConfigDs=%d selected!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pData->nEfmTcConfigDs));

         nErrCode = DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
         break;
      }

      /* Check if at least one of the possible TC Us/Ds configuration is selected*/
      if ((pData->nEfmTcConfigUs == DSL_EMF_TC_CLEANED) ||
          (pData->nEfmTcConfigDs == DSL_EMF_TC_CLEANED))
      {
         DSL_DEBUG( DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - At least one of the TC Us/Ds "
            "configuration has to be selected!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
      }

      break;

   case DSL_TC_ATM:
      DSL_DEBUG( DSL_DBG_WRN, (pContext,
         "DSL[%02d]: WARNING - All EFM configurations are ignored "
         "while the ATM is selected" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      break;

   default:
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unsupported nTcLayer=%d selected!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pData->nTcLayer));

      nErrCode = DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
      break;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_SystemInterfaceConfigCheck, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return (nErrCode);
}

DSL_Error_t DSL_DRV_DANUBE_RateAdaptationSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_AccessDir_t nDirection,
   DSL_OUT DSL_G997_RateAdaptationConfigData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nOptn15 = 0;

   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   if ( (pData->RA_MODE == DSL_G997_RA_MODE_AT_INIT) ||
        (pData->RA_MODE == DSL_G997_RA_MODE_DYNAMIC))
   {
      /* Get OLR control settings*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_OPTN,
                    DSL_CMV_ADDRESS_OPTN_OLR_CONTROL, 0, 1, &nOptn15);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - OPTN 15 read failed!" DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

      if (nDirection ==DSL_DOWNSTREAM)
      {
         nOptn15 = (DSL_uint16_t)(pData->RA_MODE == DSL_G997_RA_MODE_AT_INIT ?
                   nOptn15 & (~(1<<2)) : nOptn15 | (1<<2));
      }
      else
      {
         nOptn15 = (DSL_uint16_t)(pData->RA_MODE == DSL_G997_RA_MODE_AT_INIT ?
                   nOptn15 | (1<<8) : nOptn15 & (~(1<<8)));
      }

      /* Set OLR control settings*/
      nErrCode = DSL_DRV_DANUBE_CmvWrite(
                    pContext, DSL_CMV_GROUP_OPTN,
                    DSL_CMV_ADDRESS_OPTN_OLR_CONTROL, 0, 1, &nOptn15);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - OPTN 15 write failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

      /* Update internal Rate Adaptation Mode Settings*/
      DSL_CTX_WRITE(pContext, nErrCode, rateAdaptationMode[nDirection], pData->RA_MODE);
   }
   else
   {
      nErrCode = DSL_ERR_INVALID_PARAMETER;
   }

   return nErrCode;
}


/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
DSL_Error_t DSL_DRV_DEV_SystemInterfaceConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceConfig_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_SystemInterfaceConfigSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_DANUBE_SystemInterfaceConfigCheck(pContext, &pData->data);
   if (nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.sysCIF, pData->data);
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DEV_SystemInterfaceConfigSet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return (nErrCode);
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
DSL_Error_t DSL_DRV_DEV_SystemInterfaceConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceConfig_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_SystemInterfaceConfigGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get System Configuration from the internal DANUBE device context*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.sysCIF, pData->data);

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DEV_SystemInterfaceConfigGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return (nErrCode);
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
DSL_Error_t DSL_DRV_DEV_SystemInterfaceStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_SystemInterfaceStatusGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pContext->pDevCtx->data.bSystemIfStatusValid == DSL_TRUE)
   {
      /* Get System Interface Status from the internal DANUBE device context*/
      DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.sysCIFSts, pData->data);
   }
   else
   {
      nErrCode = DSL_ERR_DEVICE_NO_DATA;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_SystemInterfaceStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return (nErrCode);
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_AutobootHandleStart(
   DSL_Context_t *pContext,
   DSL_boolean_t bLoopTest,
   DSL_boolean_t bShortInit)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_TestModeControlSet_t nTestMode = DSL_TESTMODE_DISABLE;
   DSL_DEV_VersionCheck_t nFwVerAnnexA_PTM = DSL_VERSION_ERROR,
                          nFwVerAnnexB_PTM = DSL_VERSION_ERROR,
                          nFwVerFTTXPOTS = DSL_VERSION_ERROR;
   DSL_uint16_t nPtmConfig = 0, nVal = 0;
   DSL_uint16_t nOptn9, nOptn2, nOptn16, nOptn15 = 0, nInfo = 0, nDsl = 0;
   DSL_uint8_t i;
   DSL_boolean_t bTrellisDS = DSL_FALSE;
   DSL_boolean_t bBitSwapUS = DSL_FALSE, bBitSwapDS = DSL_FALSE;
   DSL_boolean_t bReTxEnableDS = DSL_FALSE, bVirtualNoiseSupport = DSL_FALSE;
   DSL_InteropFeatureConfigData_t interopFeatureConfigData =
      {DSL_FALSE, {DSL_SNRM_REBOOT_AUTOMODE_API, 0}, DSL_FALSE, DSL_FALSE, DSL_FALSE};
   DSL_G997_RateAdaptationConfigData_t raCfgData = {DSL_G997_RA_MODE_DYNAMIC};
   DSL_FwVersion_t FwVer = {DSL_FALSE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
   DSL_SystemInterfaceConfigData_t sysCIF;
   DSL_int32_t lineOptionsConfig[DSL_OPT_LAST] = {0};

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DEV_AutobootHandleStart" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   /* Check for the Annex A firmware PTM support*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext, DSL_MIN_ANNEX_A_FW_VERSION_PTM,
      DSL_FW_ANNEX_A, &nFwVerAnnexA_PTM);

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Check for the Annex B firmware PTM support*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext, DSL_MIN_ANNEX_B_FW_VERSION_PTM,
      DSL_FW_ANNEX_B, &nFwVerAnnexB_PTM);

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get FW  information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, FwVer);

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nTestModeControl, nTestMode);

   nErrCode = DSL_DRV_DANUBE_FirmwareInit(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Initialization of the LED has to be done with ARC idle */
   nErrCode = DSL_DRV_DEV_LED_FirmwareInit(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

#if defined(INCLUDE_DSL_CEOC)
   nErrCode = DSL_DRV_DANUBE_CEOC_FirmwareInit(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }
#endif /* defined(INCLUDE_DSL_CEOC) */

   nErrCode = DSL_DRV_DANUBE_LineInventoryNeWrite(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   nErrCode = DSL_DRV_DANUBE_LineInventoryNeRead(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   memcpy(&pContext->lineInventoryNe.XTSECapabilities,
      &pContext->xtseCfg, DSL_G997_NUM_XTSE_OCTETS);

   nErrCode = DSL_DRV_DANUBE_XTUSystemEnablingConfigSend(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   if (nTestMode == DSL_TESTMODE_QUIET)
   {
      nVal = 0;

      if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
              DSL_CMV_ADDRESS_INFO_TX_POWER, 0, 1, &nVal)
              != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - INFO(write) "
            "TX power CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         return DSL_ERROR;
      }
   }

   /* Get interop configuration data*/
   DSL_CTX_READ(
      pContext, nErrCode, interopFeatureConfigData, interopFeatureConfigData);

   /* Get OLR control settings*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_OPTN,
                 DSL_CMV_ADDRESS_OPTN_OLR_CONTROL, 0, 1, &nOptn15);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - OPTN 15 read failed!" DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   nOptn15 = (DSL_uint16_t)(interopFeatureConfigData.bL2modeDisable ?
             (nOptn15 | (1<<9)) : (nOptn15 & (~(1<<9))));

   nOptn15 = (DSL_uint16_t)(interopFeatureConfigData.bL3modeDisable ?
             (nOptn15 | (1<<10)) : (nOptn15 & (~(1<<10))));

   /* Set OLR control settings*/
   nErrCode = DSL_DRV_DANUBE_CmvWrite(
                 pContext, DSL_CMV_GROUP_OPTN,
                 DSL_CMV_ADDRESS_OPTN_OLR_CONTROL, 0, 1, &nOptn15);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - OPTN 15 write failed!" DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* Set Rate Adaptation Configuration*/
   for (i = 0; i < 2; i++)
   {
      /* Get Rate Adaptation Mode Settings*/
      DSL_CTX_READ(pContext, nErrCode, rateAdaptationMode[i], raCfgData.RA_MODE);

      /* Check/set default dynamic mode if not configured yet*/
      raCfgData.RA_MODE = raCfgData.RA_MODE == DSL_G997_RA_MODE_AT_INIT ?
                             raCfgData.RA_MODE : DSL_G997_RA_MODE_DYNAMIC;

      /* Send Rate Adaptation Mode Settings*/
      nErrCode = DSL_DRV_DANUBE_RateAdaptationSet(
                 pContext, i == 0 ? DSL_UPSTREAM : DSL_DOWNSTREAM, &raCfgData);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - Rate Adaptation set failed, nDirection=%d!" DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext), i));
         return nErrCode;
      }
   }

   /* Check for DSL_MIN_FW_VERSION_FTTXPOTS*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext,
                 DSL_MIN_FW_VERSION_FTTXPOTS, DSL_FW_ANNEX_NA, &nFwVerFTTXPOTS);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - FW version check failed!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   if (nFwVerFTTXPOTS >= DSL_VERSION_EQUAL)
   {
      /* Alternative Tx POTS highpass filter configuration. This is not mentioned
         in the CMV spec. For more details please refer to the SMS00715335 issue*/
      if (interopFeatureConfigData.bFtTxPotsHp)
      {
         /* Read INFO 103 1*/
         if ( DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE, 1, 1, &nVal)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - INFO 103 1 read "
               "failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

            return nErrCode;
         }

         nVal |= 0x0080;

         /* Modify INFO 103 1*/
         if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE, 1, 1, &nVal)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - INFO(write) "
               "TX POTS filter CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

            return nErrCode;
         }
      }
   }

   /* Apply System Interface configuration options */
   if ((nFwVerAnnexA_PTM >= DSL_VERSION_EQUAL) || (nFwVerAnnexB_PTM >= DSL_VERSION_EQUAL))
   {
      /* Get System Interface Configuration*/
      DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.sysCIF, sysCIF);

      nErrCode = DSL_DRV_DANUBE_SystemInterfaceConfigCheck(pContext, &sysCIF);

      if (nErrCode != DSL_SUCCESS)
      {
          DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - System Interface configuration check failed!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

          return nErrCode;
      }

      nPtmConfig = DSL_CMV_SEL_TPS_TC;

      if (sysCIF.nTcLayer == DSL_TC_ATM || sysCIF.nTcLayer == DSL_TC_EFM_FORCED ||
          sysCIF.nTcLayer == DSL_TC_AUTO)
      {
         nPtmConfig |= DSL_CMV_ATM_SELECTED;
      }

      if (sysCIF.nTcLayer == DSL_TC_EFM || sysCIF.nTcLayer == DSL_TC_EFM_FORCED ||
          sysCIF.nTcLayer == DSL_TC_AUTO)
      {
         nPtmConfig |= DSL_CMV_PTM_SELECTED;

         nPtmConfig |= (sysCIF.nEfmTcConfigDs & DSL_EMF_TC_NORMAL ?
                           DSL_CMV_PTM_64_65_OCTET_ENCAP_SUPPORT_DS : 0x0);
         nPtmConfig |= (sysCIF.nEfmTcConfigUs & DSL_EMF_TC_NORMAL ?
                           DSL_CMV_PTM_64_65_OCTET_ENCAP_SUPPORT_US : 0x0);
         nPtmConfig |= (sysCIF.nEfmTcConfigUs & DSL_EMF_TC_PRE_EMPTION ?
                           DSL_CMV_PTM_64_65_OCTET_ENCAP_PRE_US : 0x0);
      }

      if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
              DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE,
              DSL_CMV_SYSTEM_INTERFACE_INDEX_CONFIG, 1, &nPtmConfig)
              != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - INFO(write) system interface "
            "configuration CMV send failed"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         return DSL_ERR_MSG_EXCHANGE;
      }

      if (sysCIF.nTcLayer == DSL_TC_EFM_FORCED)
      {
         if ( DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
                DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE,
                DSL_CMV_SYSTEM_INTERFACE_PTM_FORCE, 1, &nPtmConfig)
                != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - Error: INFO(write) system interface "
               "configuration CMV send failed"DSL_DRV_CRLF,
                DSL_DEV_NUM(pContext)));

            return DSL_ERR_MSG_EXCHANGE;
         }

         nPtmConfig |= DSL_CMV_PTM_FORCED;
         if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE,
                 DSL_CMV_SYSTEM_INTERFACE_PTM_FORCE, 1, &nPtmConfig)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - Error: INFO(write) system interface "
               "configuration CMV send failed"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return DSL_ERR_MSG_EXCHANGE;
         }

         if ( DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE,
                 DSL_CMV_SYSTEM_INTERFACE_PTM_FORCE, 1, &nPtmConfig)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - Error: INFO(write) system interface "
               "configuration CMV send failed"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return DSL_ERR_MSG_EXCHANGE;
         }
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: PTM mode not supported by the Firmware!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Apply G997 Line activation options */
   if (bLoopTest == DSL_TRUE)
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Enable diag or shortinit mode in the Firmware"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_OPTN,
             DSL_CMV_ADDRESS_OPTN_STATE_MACHINE_CTRL, 0, 1, &nOptn9)
              != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - OPTN(read) state "
            "machine control CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            return DSL_ERROR;
      }

      nOptn9 |= 1<<2;

      if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_OPTN,
               DSL_CMV_ADDRESS_OPTN_STATE_MACHINE_CTRL, 0, 1, &nOptn9)
              != DSL_SUCCESS)
       {
         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - Error: OPTN(write) "
            "state machine control CMV send failed"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ERROR;
      }
   }

   /* apply line features */
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      lineFeatureDataCfg[DSL_UPSTREAM].bBitswapEnable, bBitSwapUS);
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      lineFeatureDataCfg[DSL_DOWNSTREAM].bBitswapEnable, bBitSwapDS);
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      lineFeatureDataCfg[DSL_DOWNSTREAM].bTrellisEnable, bTrellisDS);
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      lineFeatureDataCfg[DSL_DOWNSTREAM].bReTxEnable, bReTxEnableDS);

   /* Check for the R4 FW*/
   if (FwVer.bValid == DSL_TRUE)
   {
      if (FwVer.nMinorVersion >= 4)
      {
         /* Get INFO 110 0 content*/
         if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
               DSL_CMV_ADDRESS_INFO_RETX_ERASURE_DECODING, 0, 1, &nInfo)
               != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - INFO 110 read failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return DSL_ERROR;
         }

         nInfo = bReTxEnableDS ? nInfo | 0x0030 : nInfo & (~(0x0030));

         if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_RETX_ERASURE_DECODING, 0, 1, &nInfo)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - INFO 110 write failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return DSL_ERROR;
         }
      }
      else
      {
         DSL_DEBUG(DSL_DBG_WRN, (pContext,
            "DSL[%02d]: WARNING - Retransmission mode only supported starting from R4 FW"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: WARNING - No firmware version available. Retransmission configuration ignored"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   }

   if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_OPTN,
         DSL_CMV_ADDRESS_OPTN_ALG_CONTROL, 0, 1, &nOptn2)
         != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - OPTN(read) state "
         "alg CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_OPTN,
          DSL_CMV_ADDRESS_OPTN_ALG_CONTROL2, 0, 1, &nOptn16)
          != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - OPTN(read) state "
         "alg2 CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   /* DS */
   if (bBitSwapDS == DSL_FALSE)
   {
      nOptn16 &= ~(1<<10);
      nOptn2 |= 1<<5;
   }
   else
   {
      nOptn16 &= ~(1<<10);
      nOptn2 &= ~(1<<5);
   }

   /* US */
   if (bBitSwapUS == DSL_FALSE)
   {
      nOptn2 |= 1<<15;
   }
   else
   {
      nOptn2 &= ~(1<<15);
   }

   if (bTrellisDS == DSL_TRUE)
   {
      nOptn2 &= ~(1<<13);
   }
   else
   {
      nOptn2 |= 1<<13;
   }

   if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_OPTN,
           DSL_CMV_ADDRESS_OPTN_ALG_CONTROL, 0, 1, &nOptn2)
           != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - OPTN(write) "
         "alg control CMV send failed"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_OPTN,
           DSL_CMV_ADDRESS_OPTN_ALG_CONTROL2, 0, 1, &nOptn16)
           != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - OPTN(write) "
        "alg control2 CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   /* Check for the R5 and R4 Annex B FW and set Line Options*/
   if (FwVer.bValid == DSL_TRUE)
   {
      if ((FwVer.nMinorVersion >= 5) ||
          ((FwVer.nMinorVersion == 4) && (FwVer.nApplicationType == 2)))
      {
         /* Get Line Options Configuration Data*/
         DSL_CTX_READ(pContext, nErrCode,
            lineOptionsConfig, lineOptionsConfig);

         /* Set CMV DSL 1 0 bit 1*/
         nDsl |= (lineOptionsConfig[DSL_ERASURE_DECODING_TYPE_DS] ? 0x2 : 0x0);
         /* Set CMV DSL 1 0 bit 2*/
         nDsl |= (lineOptionsConfig[DSL_TRUST_ME_BIT] ? 0x4 : 0x0);

         /* Write DSL1*/
         if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_DSL,
                 DSL_CMV_ADDRESS_DSL_ERASURE_DECODER_CONTROL, 0, 1, &nDsl)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - DSL1 write "
               "failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

            return DSL_ERROR;
         }

         nDsl = 0;
         /* Set CMV DSL 2 0 bit 0*/
         nDsl |= (lineOptionsConfig[DSL_INBAND_SPECTRAL_SHAPING_US] ? 0x1 : 0x0);

         /* Write DSL2*/
         if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_DSL,
                 DSL_CMV_ADDRESS_DSL_PSD_CONTROL, 0, 1, &nDsl)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - DSL2 write "
               "failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

            return DSL_ERROR;
         }

         nDsl = 0;

         /* Get DS Virtual Noise configuration*/
         DSL_CTX_READ_SCALAR(pContext, nErrCode,
            lineFeatureDataCfg[DSL_DOWNSTREAM].bVirtualNoiseSupport, bVirtualNoiseSupport);
         /* Set CMV DSL 0 0 bit 0*/
         nDsl |= (bVirtualNoiseSupport ? 0x1 : 0x0);

         /* Get US Virtual Noise configuration*/
         DSL_CTX_READ_SCALAR(pContext, nErrCode,
            lineFeatureDataCfg[DSL_UPSTREAM].bVirtualNoiseSupport, bVirtualNoiseSupport);
         /* Set CMV DSL 0 0 bit 1*/
         nDsl |= (bVirtualNoiseSupport ? 0x2 : 0x0);

         /* Set CMV DSL 0 0 bit 2*/
         nDsl |= (bShortInit ? 0x4 : 0x0);

         /* Write DSL0*/
         if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_DSL,
                 DSL_CMV_ADDRESS_DSL_FEATURE_CONTROL, 0, 1, &nDsl)
                 != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - DSL0 write "
               "failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

            return DSL_ERROR;
         }
      }
      else
      {
         DSL_DEBUG(DSL_DBG_WRN, (pContext,
            "DSL[%02d]: WARNING - Line Options are only supported starting from R4 Annex B or R5 FW"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: WARNING - No firmware version available. Line Options configuration ignored"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   }


#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   nVal = 0x800; /* enable showtime event logging */
   if ( (nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_DEBUGTRAIL_TRIGGER, 0, 1, &nVal))
            != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - INFO(write) showtime "
         "event logging control CMV send failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   /* set buffer length */
   nVal = DSL_DEV_SHOWTIME_EVENT_LOGGING_BUFFER_LENGTH;
   if ( (nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_DEBUGTRAIL_COLLECT, 1, 1, &nVal))
            != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - INFO(write) showtime "
         "event logging buffer length CMV send failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }
#endif

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_AutobootHandleStart, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_PowerStatusSet(
   DSL_Context_t *pContext,
   DSL_G997_PowerManagement_t nNewStatus)
{
   DSL_G997_PowerManagement_t nPreStatus = DSL_G997_PMS_NA;
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_PowerStatusSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* get current status and send event if changed */
   DSL_CTX_READ(pContext, nErrCode,
      powerMgmtStatus.nPowerManagementStatus, nPreStatus);

   if (nPreStatus != nNewStatus)
   {
      DSL_CTX_WRITE(pContext, nErrCode,
         powerMgmtStatus.nPowerManagementStatus, nNewStatus);

      nErrCode = DSL_DRV_EventGenerate(
                    pContext, 0, DSL_ACCESSDIR_NA, DSL_XTUDIR_NA,
                    DSL_EVENT_S_LINE_POWERMANAGEMENT_STATE,
                    (DSL_EventData_Union_t*)&pContext->powerMgmtStatus,
                    sizeof(DSL_G997_PowerManagementStatusData_t));
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Event(%d) generate failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), DSL_EVENT_S_LINE_POWERMANAGEMENT_STATE));
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_PowerStatusSet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_PowerStatusHandle(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_PowerManagement_t nNewStatus;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_PowerStatusHandle"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_DANUBE_L3StatusGet(pContext, &nNewStatus);
   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_DANUBE_PowerStatusSet(pContext, nNewStatus);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_PowerStatusHandle, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
static DSL_Error_t DSL_DRV_DANUBE_InitialSnrMarginDsUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nData = 0;
   DSL_int16_t  nInitSnrMarginDs = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_PLAM,
                    DSL_CMV_ADDRESS_PLAM_SNRM, 0, 1, &nData);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - PLAM 46 read failed!"DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));
      }
      else
      {
         /* FW reports SNRM with the 0.5dB precision, API expects it with the 0.1 dB*/
         nInitSnrMarginDs = ((DSL_int16_t)nData) * 5;
      }
   }
   else /* ADSL2/2+ mode*/
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_PLAM,
                    DSL_CMV_ADDRESS_PLAM_SNRM_0_1DB, 0, 1, &nData);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - PLAM 45 read failed!"DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));
      }

      nInitSnrMarginDs = (DSL_int16_t)nData;
   }

   if (nErrCode == DSL_SUCCESS)
   {
      /* Update Initial SNR margin in the device context*/
      DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.nInitSnrMarginDs, nInitSnrMarginDs);
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

static DSL_Error_t DSL_DRV_DANUBE_MinRequiredSnrMarginDsUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal;
   DSL_int16_t nsVal = 0, nMinSnrmDs = 0;
   DSL_DEV_VersionCheck_t nFwVerReboot = DSL_VERSION_ERROR;
   DSL_int16_t fwMinSnrMargin = 0;
   DSL_SnrMarginRebootCfg_t SnrMarginRebootCfg;
   DSL_boolean_t bAdsl_1_A = DSL_FALSE, bAdsl_1_B = DSL_FALSE,
                 bAdsl_2_2p_A = DSL_FALSE, bAdsl_2_2p_B = DSL_FALSE;
   DSL_uint8_t xtseCurr[DSL_G997_NUM_XTSE_OCTETS];

   /* Get SNR Reboot configuration*/
   DSL_CTX_READ(pContext, nErrCode,
      interopFeatureConfigData.nSnrMarginRebootCfg, SnrMarginRebootCfg);

   /* Get XTSE current*/
   DSL_CTX_READ(pContext, nErrCode, xtseCurr, xtseCurr);

   /* Get Min SNR Margin FW value only in appropriate DSL_SNRM_REBOOT mode*/
   if ((SnrMarginRebootCfg.nSnrMarginRebootMode != DSL_SNRM_REBOOT_MANUAL_OFF) &&
       (SnrMarginRebootCfg.nSnrMarginRebootMode != DSL_SNRM_REBOOT_MANUAL_USER))
   {
      /*Check for the ADSL1 Annex A*/
      if ((xtseCurr[0] & XTSE_1_03_A_1_NO)  || (xtseCurr[0] & XTSE_1_01_A_T1_413) ||
          (xtseCurr[1] & XTSE_2_01_A_2_NO))
      {
         bAdsl_1_A = DSL_TRUE;
      }
      /*Check for the ADSL1 Annex B*/
      else if ((xtseCurr[0] & XTSE_1_02_C_TS_101388) || (xtseCurr[0] & XTSE_1_05_B_1_NO))
      {
         bAdsl_1_B = DSL_TRUE;
      }
      /*Check for the ADSL2/2+ Annex A*/
      else if ((xtseCurr[2] & XTSE_3_03_A_3_NO) || (xtseCurr[3] & XTSE_4_05_I_3_NO) ||
               (xtseCurr[4] & XTSE_5_03_L_3_NO) || (xtseCurr[4] & XTSE_5_04_L_3_NO) ||
               (xtseCurr[4] & XTSE_5_07_M_3_NO) || (xtseCurr[5] & XTSE_6_01_A_5_NO) ||
               (xtseCurr[5] & XTSE_6_07_I_5_NO) || (xtseCurr[6] & XTSE_7_03_M_5_NO))
      {
         bAdsl_2_2p_A = DSL_TRUE;
      }
      /*Check for the ADSL2/2+ Annex B*/
      else if ((xtseCurr[2] & XTSE_3_05_B_3_NO) || (xtseCurr[3] & XTSE_4_07_J_3_NO) ||
               (xtseCurr[5] & XTSE_6_03_B_5_NO) || (xtseCurr[6] & XTSE_7_01_J_5_NO))
      {
         bAdsl_2_2p_B = DSL_TRUE;
      }

      /*ADSL1 mode*/
      if (bAdsl_1_A || bAdsl_1_B)
      {
         /* In case of DMT the values has to be read by using C-MSG-RA */
         if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_RCMSGRA, 1, 1, &nVal)) == DSL_SUCCESS)
         {
            /* C-MSG-RA returns returns a 6 bit signed value (2'complement)
               for min margin (G992.1 Table 10-12) */
            if (DSL_DRV_TwosComplement16_HexToInt(pContext, nVal, 6, &nsVal) == DSL_SUCCESS)
            {
               /* Global value for MINSNRM is stored in units of 0.1dB */
               nsVal *= 10;
            }

            fwMinSnrMargin = nsVal;
         }
         else
         {
            DSL_DEBUG(DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - C-MSG-RA read failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }
      /*ADSL2/2+ mode*/
      else if(bAdsl_2_2p_A || bAdsl_2_2p_B)
      {
         if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_CMsgs1, 1, 1, &nVal)) == DSL_SUCCESS)
         {
            /* In case of ADSL2/2+ the value is already defined as multiple
               of 0.1 dB within C-MSG1*/
            nsVal = (DSL_int16_t)nVal;

            fwMinSnrMargin = nsVal;
         }
         else
         {
            DSL_DEBUG(DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - C-MSG1 read failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }
      else
      {
         DSL_DEBUG(DSL_DBG_WRN,
            (pContext, "DSL[%02d]: WARNING - Unknown FW mode, Min Snr Margin DS was set to 0dB"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         fwMinSnrMargin = 0;
      }
   }

   /*
   If this mode is activated it depends on the firmware feature set which
   handling is used in the DSL CPE API as follows
   - For feature set 2 and lower firmware versions the handling of
     DSL_SNRM_REBOOT_AUTOMODE_API is used (DSL CPE API internal default
     handling is used)
   - For feature set 3 and higher firmware versions the firmware reboot
     functionality is used (NO DSL CPE API internal handling of reboot
     criteria for MinSnrMargin) */
   if (SnrMarginRebootCfg.nSnrMarginRebootMode == DSL_SNRM_REBOOT_AUTOMODE_FW)
   {

      /* Check for DSL_MIN_FW_VERSION_REBOOT*/
      nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext,
                          DSL_MIN_FW_VERSION_REBOOT, DSL_FW_ANNEX_NA, &nFwVerReboot);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - FW version check failed,"
            "DSL_SNRM_REBOOT_AUTOMODE_FW mode ignored, Min SNR margin unchanged!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      }
      else
      {
         if (nFwVerReboot >= DSL_VERSION_EQUAL)
         {
            /* Use DSL_SNRM_REBOOT_AUTOMODE_FE mode*/
            SnrMarginRebootCfg.nSnrMarginRebootMode = DSL_SNRM_REBOOT_AUTOMODE_FE;
         }
         else
         {
            /* Use DSL_SNRM_REBOOT_AUTOMODE_API mode*/
            SnrMarginRebootCfg.nSnrMarginRebootMode = DSL_SNRM_REBOOT_AUTOMODE_API;
         }
      }
   }

   switch (SnrMarginRebootCfg.nSnrMarginRebootMode)
   {
   case DSL_SNRM_REBOOT_AUTOMODE_FE:
      /*
      If this mode is activated the SnrMinMargin that is provided by the CO
      will be always used for ALL annexes (A/I/L/M/B/J) and modes (ADSL1/2/2+).*/
      nMinSnrmDs = fwMinSnrMargin;
      break;

   case DSL_SNRM_REBOOT_AUTOMODE_FW:
      /* Dummy*/
      break;

   case DSL_SNRM_REBOOT_MANUAL_OFF:
      /*
      If this mode is activated the SnrMinMargin that is provided by the CO
      will be ignored for ALL annexes (A/I/L/M/B/J) and modes (ADSL1/2/2+).
      Instead of a value of 0 dB will be assumed. */
      nMinSnrmDs = 0;
      break;

   case DSL_SNRM_REBOOT_MANUAL_USER:
      /*
      If this mode is activated it is possible to configure the value that should
      be used for SnrMinMargin reboot criteria. This
      value will be used instead of the CO provided one, for ALL annexes
      (A/I/L/M/B/J) and modes (ADSL1/2/2+). */
      nMinSnrmDs = SnrMarginRebootCfg.nUserMinSnrMargin;
      break;

   case DSL_SNRM_REBOOT_AUTOMODE_API:
   default:
      /*
      - Reboot criteria will be ALWAYS handled automatically by the API
        (independently from the FW feature set!)
        This is the default configuration at startup.
      - AnnexA(/I/L/M)
        SnrMinMargin that is provided by the CO will be ignored for ALL AnnexA
        modes (ADSL1/2/2+). Instead of a value of 0 dB will be assumed.
      - AnnexB(J/M)
        SnrMinMargin that is provided by the CO will be used for ALL AnnexB
        modes (ADSL1/2/2+). */
      /*Check for the ADSL Annex A*/
      if (bAdsl_1_A || bAdsl_2_2p_A)
      {
         /* Ignore FW value, use 0dB instead*/
         nMinSnrmDs = 0;
      }
      else
      {
         /* Use FW value*/
         nMinSnrmDs = fwMinSnrMargin;
      }
      break;
   }

   DSL_CTX_WRITE(pContext, nErrCode, nMinSnrmDs, nMinSnrmDs);

   return nErrCode;
}


DSL_Error_t DSL_DRV_DEV_ShowtimeStatusUpdate(
   DSL_Context_t *pContext,
   DSL_boolean_t bInit)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal= 0;
   DSL_int16_t nSnrmDs = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_ShowtimeStatusUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_DANUBE_XTUSystemEnablingStatusUpdate(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - An error occured while XTU System Enabling"
         " Status update!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* Update Min Required Steady State SNR Margin DS*/
   nErrCode = DSL_DRV_DANUBE_MinRequiredSnrMarginDsUpdate(pContext);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Min SNR Margin update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   if (bInit != DSL_FALSE)
   {
      nErrCode = DSL_DRV_DANUBE_LineFeaturesUpdate(pContext);
      if (nErrCode == DSL_SUCCESS)
      {
         nErrCode = DSL_DRV_DANUBE_ChannelStatusUpdate(pContext);
      }
      else
      {
         return nErrCode;
      }

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
      /* Update Initial SNR Margin DS*/
      nErrCode = DSL_DRV_DANUBE_InitialSnrMarginDsUpdate(pContext);

      if (nErrCode != DSL_SUCCESS)
      {
         return nErrCode;
      }
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/
   }

   /* Update US Channel Rate*/
   nErrCode = DSL_DRV_DANUBE_ChannelRateUpdate(pContext, DSL_UPSTREAM);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Update DS Channel Rate*/
   nErrCode = DSL_DRV_DANUBE_ChannelRateUpdate(pContext, DSL_DOWNSTREAM);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Update line failures*/
   nErrCode = DSL_DRV_DANUBE_LineFailuresUpdate(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Update DS SNR value*/
   if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
      DSL_CMV_ADDRESS_PLAM_SNRM_0_1DB, 0, 1, &nVal)) == DSL_SUCCESS)
   {
      nSnrmDs = (DSL_int16_t)nVal;
      DSL_CTX_WRITE(pContext, nErrCode, nSnrmDs, nSnrmDs);
   }

   nErrCode = DSL_DRV_DANUBE_PowerStatusHandle(pContext);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_ShowtimeStatusUpdate, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined(INCLUDE_DSL_PM)
#ifdef HAS_TO_BE_CLARIFIED
#define DSL_DRV_DANUBE_LINE_INIT_FAILURES_INC() \
   do { \
         DSL_CTX_READ_SCALAR(pContext, nErrCode, \
            bGotShortInitResponse, bResp); \
            if (bResp == DSL_TRUE) \
            { \
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, \
               pDevCtx->data.lineInitCounters.nFailedShortInits, \
               pContext->pDevCtx->data.lineInitCounters.nFailedShortInits + 1); \
         } \
      } \
   } while (0)
#endif
#endif

#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
static DSL_Error_t DSL_DRV_DANUBE_SystemInterfaceStatusUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_SystemInterfaceConfigData_t stsData =
      {DSL_TC_UNKNOWN, DSL_EMF_TC_CLEANED, DSL_EMF_TC_CLEANED, DSL_SYSTEMIF_UNKNOWN};
   DSL_DEV_VersionCheck_t nFwVerAnnexA_PTM = DSL_VERSION_ERROR,
                          nFwVerAnnexB_PTM = DSL_VERSION_ERROR;
   DSL_uint16_t nPtmSts = 0;

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DANUBE_SystemInterfaceStatusUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext, DSL_MIN_ANNEX_A_FW_VERSION_PTM,
      DSL_FW_ANNEX_A, &nFwVerAnnexA_PTM);

   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(pContext, DSL_MIN_ANNEX_B_FW_VERSION_PTM,
      DSL_FW_ANNEX_B, &nFwVerAnnexB_PTM);

   if ((nErrCode < DSL_SUCCESS) ||
       ((nFwVerAnnexA_PTM < DSL_VERSION_EQUAL) && (nFwVerAnnexB_PTM < DSL_VERSION_EQUAL)))
   {
      return nErrCode;
   }

   if ( DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
           DSL_CMV_ADDRESS_INFO_SYSTEM_INTERFACE,
           DSL_CMV_SYSTEM_INTERFACE_INDEX_STATUS, 1, &nPtmSts) != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Error: INFO(read) system interface configuration CMV "
         "send failed" DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_MSG_EXCHANGE;
   }

   /* The only system interface that is supported for Danube, Amazon-SE and
      AR9 is MII by now */
   stsData.nSystemIf = DSL_SYSTEMIF_MII;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: MSG - nPtmSts = %#x %s\n" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nPtmSts, (nPtmSts & DSL_CMV_ATM_SELECTED)?"DSL_TC_ATM":
     (nPtmSts & DSL_CMV_PTM_SELECTED)?"DSL_TC_EFM":"Unknown System Interface Status"));

   if (nPtmSts & DSL_CMV_ATM_SELECTED)
   {
      stsData.nTcLayer = DSL_TC_ATM;
   }
   else if (nPtmSts & DSL_CMV_PTM_SELECTED)
   {
      /* Reset bitmasks to uninitialized before updating with decoded
         firmware information. */
      stsData.nEfmTcConfigUs = DSL_EMF_TC_CLEANED;
      stsData.nEfmTcConfigDs = DSL_EMF_TC_CLEANED;

      stsData.nTcLayer = DSL_TC_EFM;

      stsData.nEfmTcConfigDs |= (nPtmSts & DSL_CMV_PTM_64_65_OCTET_ENCAP_SUPPORT_DS ?
                                    DSL_EMF_TC_NORMAL : 0x0);
      stsData.nEfmTcConfigUs |= (nPtmSts & DSL_CMV_PTM_64_65_OCTET_ENCAP_SUPPORT_US ?
                                    DSL_EMF_TC_NORMAL : 0x0);
      stsData.nEfmTcConfigUs |= (nPtmSts & DSL_CMV_PTM_64_65_OCTET_ENCAP_PRE_US ?
                                    DSL_EMF_TC_PRE_EMPTION : 0x0);
   }
   else
   {
      stsData.nTcLayer = DSL_TC_UNKNOWN;
   }

   /* Update System Interface status in the CPE API Context*/
   DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.sysCIFSts, stsData);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: MSG nSystemIf %#x, "
      "nTcLayer %#x, nEfmTcConfigUs %#x, nEfmTcConfigDs %#x\n" DSL_DRV_CRLF,
       DSL_DEV_NUM(pContext), stsData.nSystemIf, stsData.nTcLayer,
       stsData.nEfmTcConfigUs, stsData.nEfmTcConfigDs));

   if (stsData.nTcLayer == DSL_TC_UNKNOWN)
   {
      return nErrCode;
   }
   else
   {
      pContext->pDevCtx->data.bSystemIfStatusValid = DSL_TRUE;
   }

   nErrCode = DSL_DRV_EventGenerate(
                 pContext, 0, DSL_ACCESSDIR_NA, DSL_XTUDIR_NA,
                 DSL_EVENT_S_SYSTEM_INTERFACE_STATUS,
                 (DSL_EventData_Union_t*)&stsData,
                 sizeof(DSL_SystemInterfaceStatus_t));

   if( nErrCode < DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Event(%d) generate failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_EVENT_S_SYSTEM_INTERFACE_STATUS));
      return nErrCode;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DANUBE_SystemInterfaceStatusUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_AutobootHandleL3(
   DSL_Context_t *pContext,
   DSL_boolean_t bL3Forced)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_PowerManagement_t nActualPms = DSL_G997_PMS_NA;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_AutobootHandleL3"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (bL3Forced == DSL_FALSE)
   {
      nErrCode = DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_RESTART,
         DSL_AUTOBOOT_RESTART_POLL_TIME);
   }
   else
   {
      /* read actual power status */
      nErrCode = DSL_DRV_DANUBE_PowerStatusHandle(pContext);
      if (nErrCode == DSL_SUCCESS)
      {
         DSL_CTX_READ(pContext, nErrCode,
            powerMgmtStatus.nPowerManagementStatus, nActualPms);

         if (nActualPms != DSL_G997_PMS_L3)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext,
               "DSL[%02d]: ERROR - Unexpected power management state "
               "returned"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
         }
      }
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DEV_AutobootHandleL3, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_AutobootHandleTraining(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_TestModeControlSet_t nTestMode;
   DSL_LineStateValue_t nPrevLineState = DSL_LINESTATE_UNKNOWN;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_uint16_t nVal = 0;

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DEV_AutobootHandleTraining" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nTestModeControl, nTestMode);
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nPrevLineState);

   if ((nErrCode = DSL_DRV_LineStateUpdate(pContext)) == DSL_SUCCESS)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);
      /* Allow consecutive polling only in the DSL_LINESTATE_SHOWTIME_NO_SYNC state*/
      if ((nLineState != nPrevLineState) || (nPrevLineState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         switch (nLineState)
         {
            case DSL_LINESTATE_TEST:
               DSL_DEBUG(DSL_DBG_MSG,
                  (pContext, "DSL[%02d]: TEST state reached"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
               break;

            case DSL_LINESTATE_SILENT:
               DSL_DEBUG(DSL_DBG_MSG,
                  (pContext, "DSL[%02d]: SILENT state reached"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
               break;
            case DSL_LINESTATE_SHOWTIME_NO_SYNC:
               /* Clear bGotFullInit flag*/
               pContext->bGotFullInit = DSL_FALSE;
               /* Clear bGotShortInitResponse flag*/
               pContext->bGotShortInitResponse = DSL_FALSE;

               if (pContext->bGotShowtime == DSL_FALSE)
               {
                  pContext->bGotShowtime = DSL_TRUE;
                  /* Start new time counting*/
                  DSL_DRV_AutobootTimeoutSet(pContext, 10);
               }

               nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                  DSL_CMV_ADDRESS_STAT_MISC, 0, 1, &nVal);

               if (nErrCode == DSL_SUCCESS)
               {
                  if (nVal & 0x02)
                  {
#ifdef INCLUDE_DSL_FIRMWARE_MEMORY_FREE
                     /* Free Resources of low level driver */
                     if (DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_FREE_RESOURCE,
                        DSL_NULL) != DSL_SUCCESS)
                     {
                        DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: DSL_DRV_DEV_BspIoctl("
                           "DSL_FIO_BSP_FREE_RESOURCE) failed" DSL_DRV_CRLF,
                           DSL_DEV_NUM(pContext)));
                     }
#endif /* INCLUDE_DSL_FIRMWARE_MEMORY_FREE*/

                     /* Set Autoboot state*/
                     nErrCode = DSL_DRV_AutobootStateSet(
                                pContext, DSL_AUTOBOOTSTATE_SHOWTIME,
                                DSL_AUTOBOOT_SHOWTIME_POLL_TIME);

                     if (nErrCode == DSL_SUCCESS)
                     {
                        DSL_DEBUG(DSL_DBG_MSG, (pContext,
                           "DSL[%02d]: SHOWTIME state reached"DSL_DRV_CRLF,
                           DSL_DEV_NUM(pContext)));

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)
                        /* Set LINIT value to 'LINIT_SUCCESSFUL' here
                           because activation procedure
                           was finished without errors */
                        DSL_DRV_HandleLinitValue(pContext, LINIT_SUCCESSFUL, LINIT_SUB_NONE);
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)*/

                        DSL_DRV_ShowtimeStatusUpdate(pContext, DSL_TRUE);
                        DSL_DRV_DANUBE_ShowtimeReached(pContext);
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
                        DSL_DRV_Timeout_AddEvent( pContext,
                           DSL_TIMEOUTEVENT_FE_STATUS,
                           DSL_DEV_TIMEOUT_FE_STATUS );
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
                     }

#ifdef INCLUDE_DSL_PM
                     /* PM device specific handling for the Showtime entry point*/
                     nErrCode = DSL_DRV_PM_DEV_ShowtimeReachedHandle(pContext);
                     if (nErrCode != DSL_SUCCESS)
                     {
                        DSL_DEBUG(DSL_DBG_ERR,
                           (pContext, "DSL[%02d]: ERROR - PM showtime entry point handling failed!"
                           DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
                     }
#endif /* INCLUDE_DSL_PM*/
                     /* Update Line Status Training values*/
                     nErrCode = DSL_DRV_DANUBE_G997_LineTrainingStatusUpdate(pContext);
                     if (nErrCode != DSL_SUCCESS)
                     {
                        DSL_DEBUG(DSL_DBG_ERR,
                           (pContext, "DSL[%02d]: ERROR - G997 Line Status Training values update failed!"
                           DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
                     }
                  }
               }
               else
               {
                  DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - STAT(read) "
                     "code swaps complete CMV send failed!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));

                  nErrCode = DSL_ERR_MSG_EXCHANGE;

                  break;
               }
               break;

            case DSL_LINESTATE_EXCEPTION:
               DSL_DEBUG(DSL_DBG_MSG,
                  (pContext, "DSL[%02d]: EXCEPTION occurred"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));

               if (nTestMode == DSL_TESTMODE_TRAINING_LOCK)
               {
                  DSL_DEBUG(DSL_DBG_MSG,
                     (pContext, "DSL[%02d]: EXCEPTION - training lock is set, "
                     "exception state will not be set"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));
               }
               else
               {
                  DSL_DEBUG(DSL_DBG_MSG,
                     (pContext, "DSL[%02d]: EXCEPTION - set exception state"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));

                  nErrCode = DSL_DRV_AutobootStateSet(
                                pContext,
                                DSL_AUTOBOOTSTATE_EXCEPTION,
                                DSL_AUTOBOOT_EXCEPTION_POLL_TIME);
               }
               break;
            case DSL_LINESTATE_HANDSHAKE:
               /* Start new time counting*/
               DSL_DRV_AutobootTimeoutSet(pContext, 60);
               break;

            case DSL_LINESTATE_FULL_INIT:
               if (pContext->bGotFullInit == DSL_FALSE)
               {
                  pContext->bGotFullInit = DSL_TRUE;
#if defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
                  DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
                     pDevCtx->data.lineInitCounters.nFullInits,
                     pContext->pDevCtx->data.lineInitCounters.nFullInits + 1);
#endif /* defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/

#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
                  nErrCode = DSL_DRV_DANUBE_SystemInterfaceStatusUpdate(pContext);
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/
               }
               break;

            case DSL_LINESTATE_SHORT_INIT_ENTRY:
#if defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
               if (pContext->bGotShortInitResponse == DSL_FALSE)
               {
                  pContext->bGotShortInitResponse = DSL_TRUE;
                  DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
                     pDevCtx->data.lineInitCounters.nShortInits,
                     pContext->pDevCtx->data.lineInitCounters.nShortInits + 1);
               }
#endif /* defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/
               break;

            case DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE:
               nErrCode = DSL_DRV_AutobootStateSet(
                             pContext,
                             DSL_AUTOBOOTSTATE_DIAGNOSTIC,
                             DSL_AUTOBOOT_DIAGNOSTIC_POLL_TIME);

#ifdef INCLUDE_DSL_DELT
               DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bLoopDiagnosticsCompleted, DSL_FALSE);
#endif /* INCLUDE_DSL_DELT*/
               /* Start new time counting*/
               DSL_DRV_AutobootTimeoutSet(pContext, 120);

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)
               /* Set LINIT value to 'LINIT_SUCCESSFUL' here
                  because activation procedure
                  was finished without errors */
               DSL_DRV_HandleLinitValue(pContext, LINIT_SUCCESSFUL, LINIT_SUB_NONE);
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)*/

               /* In case of DIAGNOSTIC mode use this state to identify FULL_INIT*/
               if (pContext->bGotFullInit == DSL_FALSE)
               {
                  pContext->bGotFullInit = DSL_TRUE;
#if defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
                  DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
                     pDevCtx->data.lineInitCounters.nFullInits,
                     pContext->pDevCtx->data.lineInitCounters.nFullInits + 1);
#endif /* defined(INCLUDE_DSL_PM)*/
               }

               /* Update XTSE information*/
               nErrCode = DSL_DRV_DANUBE_XTUSystemEnablingStatusUpdate(pContext);
               if (nErrCode != DSL_SUCCESS)
               {
                  DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - Could not "
                     "read current mode!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
               }
               break;

            case DSL_LINESTATE_LOOPDIAGNOSTIC_COMPLETE:
#ifdef INCLUDE_DSL_DELT
               DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
                  bLoopDiagnosticsCompleted, DSL_TRUE);

               nErrCode = DSL_DRV_DANUBE_G997_LoopDiagnosticCompleted(pContext);
               if (nErrCode != DSL_SUCCESS)
               {
                  DSL_DEBUG(DSL_DBG_WRN, (pContext, "DSL[%02d]: WARNING - Could not "
                     "complete loop diagnostic mode!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));
               }
#endif /* INCLUDE_DSL_DELT*/
            /* Pass through */
            case DSL_LINESTATE_IDLE:
#if defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
               if ( (pContext->bGotFullInit == DSL_TRUE) &&
                    (nPrevLineState != DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE))
               {
                  DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
                     pDevCtx->data.lineInitCounters.nFailedFullInits,
                     pContext->pDevCtx->data.lineInitCounters.nFailedFullInits + 1);
               }
#endif /* defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)
               /* Get Modem Fail Reason*/
               nErrCode = DSL_DRV_DANUBE_FailReasonGet( pContext );
               if( nErrCode != DSL_SUCCESS )
               {
                  DSL_DEBUG(DSL_DBG_WRN, (pContext, "DSL[%02d]: WARNING - Can't get "
                     "modem Fail reason!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
               }
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)*/

               nErrCode = DSL_DRV_AutobootStateSet(
                             pContext,
                             DSL_AUTOBOOTSTATE_RESTART,
                             DSL_AUTOBOOT_TRAINING_POLL_TIME);
               break;
            default:
               DSL_DEBUG(DSL_DBG_ERR, (pContext,
                        "DSL[%02d]: ERROR - Unexpected line state (%X) while training!"DSL_DRV_CRLF,
                        DSL_DEV_NUM(pContext), nLineState));

               nErrCode = DSL_ERROR;

               break;
         }
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_AutobootHandleTraining, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_AutobootHandleException(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   DSL_uint32_t i;
#endif
#if defined(INCLUDE_DSL_CPE_TRACE_BUFFER) || defined(INCLUDE_DEVICE_EXCEPTION_CODES)
   DSL_uint16_t nVal;
#endif /* defined(INCLUDE_DSL_CPE_TRACE_BUFFER) || defined(INCLUDE_DEVICE_EXCEPTION_CODES)*/
#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
   DSL_DBG_LastExceptionCodesData_t LastExceptionCodes;
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_AutobootHandleException"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

#if defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
   /* Update nFailedFullInits counter*/
   if (pContext->bGotFullInit)
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
         pDevCtx->data.lineInitCounters.nFailedFullInits,
         pContext->pDevCtx->data.lineInitCounters.nFailedFullInits + 1);
   }

   /* Update nFailedShortInits counter*/
   if (pContext->bGotShortInitResponse)
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
         pDevCtx->data.lineInitCounters.nFailedShortInits,
         pContext->pDevCtx->data.lineInitCounters.nFailedShortInits + 1);
   }
#endif /* defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)
   /* Get Modem Fail Reason*/
   nErrCode = DSL_DRV_DANUBE_FailReasonGet( pContext );
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_WRN, (pContext, "DSL[%02d]: WARNING - Can't get "
         "modem Fail reason!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   }
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)*/

   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_DANUBE_LineFailuresUpdate(pContext);
   }

#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   for (i = 0; i < DSL_DEV_SHOWTIME_EVENT_LOGGING_BUFFER_LENGTH; i++)
   {
      if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_DATA_LOGGING_BUFFER, (DSL_uint16_t)i, 1, &nVal) != DSL_SUCCESS)
      {
         nErrCode = DSL_ERR_MSG_EXCHANGE;
         break;
      }

      pContext->loggingBuffer[i] = nVal;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: DEBUG:1:: showtime event logging data (after DSL_DRV_DANUBE_CmvRead() calls):",
      DSL_DEV_NUM(pContext)));
   for (i = 0; i < DSL_DEV_SHOWTIME_EVENT_LOGGING_BUFFER_LENGTH; i++)
   {
      if (i % 10 == 0) DSL_DEBUG(DSL_DBG_MSG, (pContext, DSL_DRV_CRLF));
      DSL_DEBUG(DSL_DBG_MSG, (pContext, "0x%04X ", pContext->loggingBuffer[i]));
   }
   DSL_DEBUG(DSL_DBG_MSG, (pContext, DSL_DRV_CRLF DSL_DRV_CRLF));

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   /* Inform upper software about a showtime event data is ready */
   if (nErrCode == DSL_SUCCESS)
   {
      nErrCode = DSL_DRV_EventGenerate(pContext, 0, DSL_ACCESSDIR_NA, DSL_XTUDIR_NA,
         DSL_EVENT_S_SHOWTIME_LOGGING, DSL_NULL, 0);
   }
#endif

   /* Update Power Management Status*/
   nErrCode = DSL_DRV_DANUBE_PowerStatusSet(pContext, DSL_G997_PMS_L3);
   if(nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Power Management State set failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
   /* Get Current link exception code*/
   nVal = 0;
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_STAT, DSL_CMV_ADDRESS_STAT_FAIL,
                 0, 1, &nVal);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - STAT 5 0 read failed!"DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   LastExceptionCodes.nError1 = (DSL_uint32_t)nVal;

   /* Get Previous link exception code*/
   nVal = 0;
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_STAT, DSL_CMV_ADDRESS_STAT_PREV_FAIL,
                 0, 1, &nVal);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - STAT 23 0 read failed!"DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   LastExceptionCodes.nError2 = (DSL_uint32_t)nVal;

   /* Update Last Exception Codes in the CPE API Context*/
   DSL_CTX_WRITE(pContext, nErrCode, LastExceptionCodes, LastExceptionCodes);
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/

#if defined(INCLUDE_ADSL_LED)
   /* OFF */
   /* use GPIO9 for TR68 data led off. */
   if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
      DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 5, 1, &off) != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "[%s %d]: CMV Write fail!" DSL_DRV_CRLF, __func__, __LINE__));
   }
#endif /* defined(INCLUDE_ADSL_LED)*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DEV_AutobootHandleException, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_DeviceInit(
   DSL_Context_t *pContext,
   DSL_Init_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_DeviceInit"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pData);

   /*
      System Interface Configuration check
   */
   nErrCode = DSL_DRV_SystemInterfaceConfigCheck(pContext, &(pData->data.nDeviceCfg.sysCIF));
   if( nErrCode < DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - System Interface Configuration check failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.sysCIF, pData->data.nDeviceCfg.sysCIF);

   DSL_DRV_DANUBE_LowLevelVersionsUpdate(pContext);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_DeviceInit, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_ModemReset(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_ModemReset"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_RESET, DSL_NULL);
   DSL_DRV_DEV_BspIoctl(pContext, DSL_FIO_BSP_HALT, DSL_NULL);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_ModemReset, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_LinkReset(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_LinkReset"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Update Power Management Status*/
   nErrCode = DSL_DRV_DANUBE_PowerStatusSet(pContext, DSL_G997_PMS_L3);
   if(nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Power Management State set failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_LinkReset, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DEV_LinkActivate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal = DSL_CMV_ADDRESS_ARG_CNTL_MODEM_START;
   DSL_TestModeControlSet_t nTestModeControl = DSL_TESTMODE_DISABLE;

   if ( DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_CNTL,
        DSL_CMV_ADDRESS_CNTL_MODEM_CONTROL, 0, 1, &nVal)
        != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CNTL(write) modem "
         "control CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   DSL_CTX_READ(pContext, nErrCode, nTestModeControl, nTestModeControl);

   if (nTestModeControl == DSL_TESTMODE_QUIET)
   {
      DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_TEST);
   }

   return nErrCode;
}

DSL_Error_t DSL_DRV_DEV_BspIoctl(
   DSL_Context_t *pContext,
   DSL_uint_t nCommand,
   DSL_uint32_t *nArg)
{
   DSL_DEV_Handle_t dev;
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_ERROR;
   dev = DSL_DRV_LowDeviceGet(pContext->pDevCtx);
   if (dev != NULL)
   {
      /* Lock driver access*/
      if(DSL_DRV_MUTEX_LOCK(pContext->bspMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock device mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ERROR;
      }

      /* Access driver IOCTL*/
      if ( DSL_BSP_KernelIoctls(dev, nCommand, (unsigned long)nArg) == 0 )
      {
         nErrCode = DSL_SUCCESS;
      }

      /* Unlock driver access*/
      DSL_DRV_MUTEX_UNLOCK(pContext->bspMutex);
   }

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_FirmwareVersionGet(
   DSL_Context_t *pContext,
   DSL_char_t    *pString)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_FwVersion_t fwVersion;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_FirmwareVersionGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_POINTER(pContext, pString);
   DSL_CHECK_ERR_CODE();

   /* Get FW version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, fwVersion);

   if (fwVersion.bValid == DSL_TRUE)
   {
      DSL_DRV_snprintf(pString, MAX_INFO_STRING_LEN, "%hu.%hu.%hu.%hu.%hu.%hu",
         fwVersion.nMajorVersion, fwVersion.nMinorVersion, fwVersion.nExternalVersion,
         fwVersion.nInternalVersion, fwVersion.nReleaseState,
         fwVersion.nApplicationType);

      DSL_DEBUG(DSL_DBG_MSG, (pContext,
         "DSL[%02d]: DSL_DEV_FirmwareVersionGet: %s" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pString));
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - No firmware version available" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      strcpy(pString, "n/a");

      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_FirmwareVersionGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_ChipHardwareVersionGet(
   DSL_Context_t *pContext,
   DSL_char_t    *pString)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_ChipVersion_t chipVersion;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: DSL_DEV_ChipVersionStringGet: "
      "This function is not applicable."DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pString);
   DSL_CHECK_ERR_CODE();

   /* Get Chipset version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.chipVersion, chipVersion);

   if (chipVersion.bValid == DSL_TRUE)
   {
      DSL_DRV_snprintf(pString, MAX_INFO_STRING_LEN, "%hu.%hu",
         chipVersion.nMajorVersion, chipVersion.nMinorVersion);

      DSL_DEBUG(DSL_DBG_MSG, (pContext,
         "DSL: DSL_DEV_ChipHardwareVersionGet: %s" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pString));
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - No chip version available" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      strcpy(pString, "n/a");

      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_ChipSetTypeGet(
   DSL_Context_t *pContext,
   DSL_char_t    *pString)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_FwVersion_t FwVer = {DSL_FALSE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

   DSL_CHECK_POINTER(pContext, pString);
   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   /* Get FW  information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, FwVer);

   if (FwVer.bValid == DSL_TRUE)
   {
      switch (FwVer.nMajorVersion)
      {
      case DSL_AR9_FW_MAJOR_NUMBER:
         strncpy(pString, "Ifx-AR9", MAX_INFO_STRING_LEN);
         break;

      case DSL_DANUBE_FW_MAJOR_NUMBER:
         strncpy(pString, "Ifx-Danube", MAX_INFO_STRING_LEN);
         break;

      case DSL_AMAZON_SE_FW_MAJOR_NUMBER:
         strncpy(pString, "Ifx-Amazon-SE", MAX_INFO_STRING_LEN);
         break;

      default:
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Unknown Chipset type (%d)" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), FwVer.nMajorVersion));

         strncpy(pString, "Ifx-Unknown", MAX_INFO_STRING_LEN);

         nErrCode = DSL_WRN_DEVICE_NO_DATA;

         break;
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - No firmware version available" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   return nErrCode;
}


/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_DriverVersionGet(
   DSL_Context_t *pContext,
   DSL_char_t    *pString)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_MeiSwVersion_t meiSwVersion;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_DriverVersionGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_POINTER(pContext, pString);
   DSL_CHECK_ERR_CODE();

   /* Get MEI version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.meiSwVersion, meiSwVersion);

   if (meiSwVersion.bValid == DSL_TRUE)
   {
      DSL_DRV_snprintf(pString, MAX_INFO_STRING_LEN, "%hu.%hu.%hu",
         meiSwVersion.nMajorVersion, meiSwVersion.nMinorVersion, meiSwVersion.nBuild);

      DSL_DEBUG(DSL_DBG_MSG, (pContext,
         "DSL[%02d]: DSL_DEV_DriverVersionGet: %s" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pString));
   }
   else
   {
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: ERROR - No low level driver version available!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      strcpy(pString, "n/a");

      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_LineStateGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LineStateValue_t *pnLineState)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nLineState;
   DSL_LineStateValue_t nCurrLineState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_LineStateGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_POINTER(pContext, pnLineState);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrLineState);

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                        DSL_CMV_ADDRESS_STAT_MACRO_STATE, 0, 1,
                        &nLineState);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: Got %d state"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nLineState));

   switch (nLineState)
   {
      case DSL_DEV_STAT_InitState:
         *pnLineState = DSL_LINESTATE_NOT_INITIALIZED;
         break;
      case DSL_DEV_STAT_FailState:
         *pnLineState = DSL_LINESTATE_EXCEPTION;
         break;
      case DSL_DEV_STAT_IdleState:
         if (nCurrLineState == DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE)
         {
            *pnLineState = DSL_LINESTATE_LOOPDIAGNOSTIC_COMPLETE;
         }
         else
         {
            *pnLineState = DSL_LINESTATE_IDLE;
         }
         break;
      case DSL_DEV_STAT_GhsState:
         *pnLineState = DSL_LINESTATE_HANDSHAKE;
         break;
      case DSL_DEV_STAT_FullInitState:
         *pnLineState = DSL_LINESTATE_FULL_INIT;
         break;
      case DSL_DEV_STAT_ShowTimeState:
         if (nCurrLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
         {
            *pnLineState = DSL_LINESTATE_SHOWTIME_NO_SYNC;
         }
         else
         {
            *pnLineState = nCurrLineState;
         }
         break;
      case DSL_DEV_STAT_FastRetrainState:
         *pnLineState = DSL_LINESTATE_FASTRETRAIN;
         break;
      case DSL_DEV_STAT_LoopDiagMode:
         *pnLineState = DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE;
         break;
      case DSL_DEV_STAT_ReadyState:
         *pnLineState = nCurrLineState != DSL_LINESTATE_TEST ?
                           DSL_LINESTATE_SILENT : nCurrLineState;
         break;
      case DSL_DEV_STAT_ShortInit:
         *pnLineState = DSL_LINESTATE_SHORT_INIT_ENTRY;
         break;
      /* [TD-20070118] Usage of fixed value (0xB) is a workaround for missing
         state of T.1413 (STAT 0 0 11). This state is an intermediate one which
         will be handled as handshake state to avoid restart of autoboot
         handling. */
      case 0x000B:
         *pnLineState = DSL_LINESTATE_HANDSHAKE;
         DSL_DEBUG( DSL_DBG_ERR, (pContext,
            "DSL[%02d]: T.1413 handled as handshake!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         break;
      case DSL_DEV_STAT_QuietState:
      default:
         DSL_DEBUG( DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - DSL_LINESTATE_UNKNOWN will be returned!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         *pnLineState = DSL_LINESTATE_UNKNOWN;

         break;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DEV_LineStateGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DEV_XtseSettingsCheck(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_uint8_t *pXTSE)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_XtseSettingsCheck"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   /* Check if G992.3 Annex A mode specified in case of Annex L*/
   if (((pXTSE[5-1] & XTSE_5_04_L_3_NO) || (pXTSE[5-1] & XTSE_5_03_L_3_NO)) &&
       (!(pXTSE[3-1] & XTSE_3_03_A_3_NO)))
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Annex L mode is only supported in conjunction with G992.3 Annex A"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      /* Set G992.3 Annex A mode*/
      pXTSE[3-1] |= XTSE_3_03_A_3_NO;

      nErrCode = DSL_WRN_INCONSISTENT_XTSE_CONFIGURATION;
   }

   /* Check if both narrow and wide Annex L modes are set*/
   if ((pXTSE[5-1] & (XTSE_5_04_L_3_NO | XTSE_5_03_L_3_NO)) &&
       ((pXTSE[5-1] & (XTSE_5_04_L_3_NO | XTSE_5_03_L_3_NO)) != (XTSE_5_04_L_3_NO | XTSE_5_03_L_3_NO)))
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Setting only both (narrow, wide) Annex L allowed"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      /* Set both Annex L modes*/
      pXTSE[5-1] |= (XTSE_5_04_L_3_NO | XTSE_5_03_L_3_NO);

      nErrCode = DSL_WRN_INCONSISTENT_XTSE_CONFIGURATION;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_XtseSettingsCheck"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_ModeControlSet(
   DSL_Context_t *pContext,
   DSL_FirmwareAnnex_t nAnnex)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nOptn0 = 0, nOptn7 = 0;
   DSL_uint8_t xtseCfg[DSL_G997_NUM_XTSE_OCTETS];

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_ModeControlSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get XTSE configuration*/
   DSL_CTX_READ(pContext, nErrCode, xtseCfg, xtseCfg);

   if ((nAnnex == DSL_FW_ANNEX_A) || (nAnnex == DSL_FW_ANNEX_UNKNOWN))
   {
      nOptn0  |= (xtseCfg[5] & XTSE_6_01_A_5_NO) ? 1 << 15 : 0;
      nOptn0  |= (xtseCfg[1] & XTSE_2_01_A_2_NO) ? 1 << 1 : 0;
      nOptn0  |= (xtseCfg[0] & XTSE_1_03_A_1_NO) ? 1 << 2 : 0;
      nOptn7  |= (xtseCfg[5] & XTSE_6_07_I_5_NO) ? 1 : 0;
      nOptn0  |= (xtseCfg[3] & XTSE_4_05_I_3_NO) ? 1 << 10 : 0;
      nOptn0  |= ((xtseCfg[2] & XTSE_3_03_A_3_NO) ||
                  (xtseCfg[4] & XTSE_5_03_L_3_NO) ||
                 (xtseCfg[4] & XTSE_5_04_L_3_NO)) ? 1 << 8 : 0;
      nOptn0  |= ((xtseCfg[4] & XTSE_5_03_L_3_NO) ||
                  (xtseCfg[4] & XTSE_5_04_L_3_NO)) ? 1 << 12 : 0;
      nOptn0  |=  xtseCfg[0] & XTSE_1_01_A_T1_413;
   }

   if ((nAnnex == DSL_FW_ANNEX_B) || (nAnnex == DSL_FW_ANNEX_UNKNOWN))
   {
      nOptn0  |= (xtseCfg[5] & XTSE_6_03_B_5_NO) ? 1 << 14 : 0;
      nOptn0  |= (xtseCfg[2] & XTSE_3_05_B_3_NO) ? 1 << 9 : 0;
      nOptn0  |= (xtseCfg[0] & XTSE_1_05_B_1_NO) ? 1 << 3 : 0;
      nOptn7  |= (xtseCfg[6] & XTSE_7_01_J_5_NO) ? 2 : 0;
      nOptn0  |= (xtseCfg[3] & XTSE_4_07_J_3_NO) ? 1 << 11 : 0;
      nOptn7  |= (xtseCfg[0] & XTSE_1_02_C_TS_101388) ? 1 << 3 : 0;
   }

   nOptn7  |= (xtseCfg[6] & XTSE_7_03_M_5_NO) ? 4 : 0;
   nOptn0  |= (xtseCfg[4] & XTSE_5_07_M_3_NO) ? 1 << 13 : 0;

   if (nOptn0 == 0 && nOptn7 == 0)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - There are no possible modes "
         "enabled in the configuration. Please check it!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_INVALID_PARAMETER;
   }

   if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_OPTN,
          DSL_CMV_ADDRESS_OPTN_MODECONTROL, 0, 1, &nOptn0 ) != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - OPTN mode control CMV "
         "send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   if (DSL_DRV_DANUBE_CmvWrite(pContext,DSL_CMV_GROUP_OPTN,
      DSL_CMV_ADDRESS_OPTN_MODECONTROL1, 0, 1, &nOptn7)
      != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - OPTN mode control "
         "CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_G997_ModeControlSet"
      ", retCode=0"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_DRV_DANUBE_XTUSystemEnablingConfigSend(
   DSL_IN DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal;
   DSL_FirmwareAnnex_t nAnnex = DSL_FW_ANNEX_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_DRV_DANUBE_XTUSystemEnablingConfigSend"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      DSL_CMV_ADDRESS_INFO_VERSION, 1, 1, &nVal);

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   nAnnex = (DSL_FirmwareAnnex_t)((nVal>>8) & 0x3f);
   switch (nAnnex)
   {
      case DSL_FW_ANNEX_A:
         DSL_DEBUG(DSL_DBG_MSG, (pContext,
            "DSL[%02d]: AnnexA FW is detected"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         break;

      case DSL_FW_ANNEX_B:
         DSL_DEBUG(DSL_DBG_MSG, (pContext,
            "DSL[%02d]: AnnexB FW is detected"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         break;

      default:
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Unknown Annex of "
            "the Firmware (0x%X)!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nAnnex));

         nErrCode = DSL_DRV_AutobootStateSet(
                       pContext,
                       DSL_AUTOBOOTSTATE_EXCEPTION,
                       DSL_AUTOBOOT_IDLE_POLL_TIME);

         return DSL_ERROR;
   }

   /* Set Mode Control*/
   nErrCode = DSL_DRV_DANUBE_ModeControlSet(pContext, nAnnex);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - operation mode set failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DANUBE_XTUSystemEnablingConfigSend, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_XTUSystemEnablingStatusUpdate(
   DSL_IN_OUT DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal, nRetry = 0;
   DSL_boolean_t bStatusUpdated = DSL_FALSE;
   DSL_uint8_t xtseCurr[DSL_G997_NUM_XTSE_OCTETS] = {0,0,0,0,0,0,0,0};
   DSL_uint16_t nAdslMode, nAdslMode1;
   DSL_xDslMode_t nXDslMode;
   DSL_AnnexType_t nAnnexType;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_XTUSystemEnablingStatusUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_CTX_WRITE(pContext, nErrCode, xtseCurr, xtseCurr);

   for (nRetry = 0; nRetry < 20; nRetry++)
   {
      /* Get STAT1 info*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                    DSL_CMV_ADDRESS_STAT_MODE, 0, 1, &nVal);

      if (nErrCode != DSL_SUCCESS)
      {
         break;
      }

      DSL_DEBUG(DSL_DBG_MSG, (pContext,
         "DSL[%02d]: STAT1 = %04X"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nVal));

      nAdslMode = nVal;

      if (nVal == 0)
      {
         if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
            DSL_CMV_ADDRESS_STAT_MODE1, 0, 1, &nVal)) == DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_MSG, (pContext,
               "DSL[%02d]: STAT17 = %04X"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nVal));

            /* If both STAT1 and STAT17 are "0" this means that they are not
               updated in the FW*/
            if (nVal == 0)
            {
               /* Sleep for a little bit*/
               DSL_WAIT(5);
               /* ... and try again to request STAT1 and STAT17*/
               continue;
            }

            nAdslMode1 = nVal;

            switch (nVal)
            {
               case DSL_CMV_STAT_MODE_G992_5_I:
                  xtseCurr[5] |= XTSE_6_07_I_5_NO;
                  nXDslMode = DSL_XDSLMODE_G_992_5;
                  nAnnexType = DSL_ANNEX_I;
                  break;
               case DSL_CMV_STAT_MODE_G992_5_J:
                  xtseCurr[6] |= XTSE_7_01_J_5_NO;
                  nXDslMode = DSL_XDSLMODE_G_992_5;
                  nAnnexType = DSL_ANNEX_J;
                  break;
               case DSL_CMV_STAT_MODE_G992_5_M:
                  xtseCurr[6] |= XTSE_7_03_M_5_NO;
                  nXDslMode = DSL_XDSLMODE_G_992_5;
                  nAnnexType = DSL_ANNEX_M;
                  break;
               case DSL_CMV_STAT_MODE_G992_1_C:
                  xtseCurr[0] |= XTSE_1_02_C_TS_101388;
                  nXDslMode = DSL_XDSLMODE_G_992_1;
                  nAnnexType = DSL_ANNEX_C;
                  break;

               default:
                  DSL_DEBUG(DSL_DBG_ERR,
                     (pContext, "DSL[%02d]: ERROR - STAT17 returned unknown XTSE status!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));

                  break;
            }
         }
         else
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - Read current mode "
               "CMV send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
         }
      }
      else
      {
         nAdslMode1 = 0;
         switch (nVal)
         {
            case DSL_CMV_STAT_MODE_T1_413:
               xtseCurr[0] |= XTSE_1_01_A_T1_413;
               nXDslMode = DSL_XDSLMODE_T1_413;
               nAnnexType = DSL_ANNEX_A;
               break;
            case DSL_CMV_STAT_MODE_G992_1_A:
               xtseCurr[0] |= XTSE_1_03_A_1_NO;
               nXDslMode = DSL_XDSLMODE_G_992_1;
               nAnnexType = DSL_ANNEX_A;
               break;
            case DSL_CMV_STAT_MODE_G992_1_B:
               xtseCurr[0] |= XTSE_1_05_B_1_NO;
               nXDslMode = DSL_XDSLMODE_G_992_1;
               nAnnexType = DSL_ANNEX_B;
               break;
            case DSL_CMV_STAT_MODE_G992_2_A:
               xtseCurr[1] |= XTSE_2_01_A_2_NO;
               nXDslMode = DSL_XDSLMODE_G_992_1;
               nAnnexType = DSL_ANNEX_B;
               break;
            case DSL_CMV_STAT_MODE_G992_3_A:
               xtseCurr[2] |= XTSE_3_03_A_3_NO;
               nXDslMode = DSL_XDSLMODE_G_992_3;
               nAnnexType = DSL_ANNEX_A;
               break;
            case DSL_CMV_STAT_MODE_G992_3_B:
               xtseCurr[2] |= XTSE_3_05_B_3_NO;
               nXDslMode = DSL_XDSLMODE_G_992_3;
               nAnnexType = DSL_ANNEX_B;
               break;
            case DSL_CMV_STAT_MODE_G992_3_I:
               xtseCurr[3] |= XTSE_4_05_I_3_NO;
               nXDslMode = DSL_XDSLMODE_G_992_3;
               nAnnexType = DSL_ANNEX_I;
               break;
            case DSL_CMV_STAT_MODE_G992_3_J:
               xtseCurr[3] |= XTSE_4_07_J_3_NO;
               nXDslMode = DSL_XDSLMODE_G_992_3;
               nAnnexType = DSL_ANNEX_J;
               break;
            case DSL_CMV_STAT_MODE_G992_3_L:
               if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
                  DSL_CMV_ADDRESS_STAT_MISC, 0, 1, &nVal)) == DSL_SUCCESS)
               {
                  DSL_DEBUG(DSL_DBG_MSG, (pContext,
                     "DSL[%02d]: STAT1 = %04X"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nVal));

                  if (nVal & DSL_CMV_STAT_MISC_ANNEX_L_MASK_1)
                  {
                     xtseCurr[4] |= XTSE_5_03_L_3_NO;
                  }
                  else if (nVal & DSL_CMV_STAT_MISC_ANNEX_L_MASK_2)
                  {
                     xtseCurr[4] |= XTSE_5_04_L_3_NO;
                  }
                  else
                  {
                     DSL_DEBUG(DSL_DBG_WRN, (pContext, "DSL[%02d]: WARNING - Unkhown Annex L "
                     "Mask received!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
                  }
               }
               else
               {
                  DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - STAT1 CMV "
                  "send failed!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
               }

               nXDslMode = DSL_XDSLMODE_G_992_3;
               nAnnexType = DSL_ANNEX_L;
               break;
            case DSL_CMV_STAT_MODE_G992_3_M:
               xtseCurr[4] |= XTSE_5_07_M_3_NO;
               nXDslMode = DSL_XDSLMODE_G_992_3;
               nAnnexType = DSL_ANNEX_M;
               break;
            case DSL_CMV_STAT_MODE_G992_5_B:
               xtseCurr[5] |= XTSE_6_03_B_5_NO;
               nXDslMode = DSL_XDSLMODE_G_992_5;
               nAnnexType = DSL_ANNEX_B;
               break;
            case DSL_CMV_STAT_MODE_G992_5_A:
               xtseCurr[5] |= XTSE_6_01_A_5_NO;
               nXDslMode = DSL_XDSLMODE_G_992_5;
               nAnnexType = DSL_ANNEX_A;
               break;

            default:
                  DSL_DEBUG(DSL_DBG_ERR,
                     (pContext, "DSL[%02d]: ERROR - STAT1 returned unknown XTSE status!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));

               DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - Unknown mode "
                  "is returned in the CMV!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
               break;
         }
      }

      bStatusUpdated = DSL_TRUE;
   }

   /* Check the XTSE info update status*/
   if (!bStatusUpdated)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - XTSE status update failed, "
         "FW STAT1 and STAT17 info is not valid!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
      nErrCode = DSL_ERR_DEVICE_NO_DATA;
   }
   else
   {
      DSL_CTX_WRITE(pContext, nErrCode, xtseCurr, xtseCurr);
      DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.nAdslMode, nAdslMode);
      DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.nAdslMode1, nAdslMode1);
      DSL_CTX_WRITE(pContext, nErrCode, nXDslMode, nXDslMode);
      DSL_CTX_WRITE(pContext, nErrCode, nAnnexType, nAnnexType);

      /* Check for the ADSL1 mode*/
      if (!((nAdslMode >= 0x100) || (nAdslMode1 != 0 && nAdslMode1 < 8)))
      {
         /* Set ADSL1 mode indication*/
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.bAdsl1, DSL_TRUE);
      }

      /* Check for the ADSL2+ mode*/
      if ((nAdslMode >= 0x4000) || (nAdslMode1 != 0 && nAdslMode1 < 8))
      {
         /* Set ADSL2+ mode indication*/
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.bAdsl2p, DSL_TRUE);
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_XTUSystemEnablingStatusUpdate, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DANUBE_ShowtimeReached(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LatencyPath_t nLPath = DSL_LATENCY_DISABLED;
   DSL_uint16_t nVal;
   DSL_uint32_t nRate = 0, nFast = 0, nIntl = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_ShowtimeReached"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nUAS, nVal);
   if ((nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_PLAM,
      DSL_CMV_ADDRESS_PLAM_NEAR_END_UASL_CNT, 0, 1, &nVal))
      != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CMV fail, Group 7 Address 10 Index 0!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_SHOWTIME_TC_SYNC);

   if ( (nErrCode = DSL_DRV_DANUBE_ChannelRateUpdate(pContext, DSL_UPSTREAM)) != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Could not update US data rates!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   if ( (nErrCode = DSL_DRV_DANUBE_ChannelRateUpdate(pContext, DSL_DOWNSTREAM)) != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Could not update DS data rates!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* \note Review implementation in case of using dual latency!
            By now the interface for low level driver is used
            to differentiate between fast and interleaved for ADSL1
            only. In case of ADSL2/2+ datarate will be always given
            by "nIntl" value. */
   if (DSL_DRV_DANUBE_ActLatencyGet(pContext, 0, DSL_UPSTREAM, &nLPath) >= DSL_SUCCESS)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nChannelActualDataRate[DSL_UPSTREAM][0],
         nRate);

      if (nLPath == DSL_LATENCY_IP_LP0)
      {
         nIntl = nRate;
      }
      else
      {
         nFast = nRate;
      }

      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: DSL_BSP_Showtime(nFast=%u, nIntl=%u)"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nFast, nIntl));

      /* Lock driver access*/
      if(DSL_DRV_MUTEX_LOCK(pContext->bspMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock device mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ERROR;
      }

      if ( DSL_BSP_Showtime (pContext->pDevCtx->lowHandle, nFast,
         nIntl) != DSL_DEV_MEI_ERR_SUCCESS )
      {
         nErrCode = DSL_ERROR;
      }

      /* Unlock driver access*/
      DSL_DRV_MUTEX_UNLOCK(pContext->bspMutex);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_ShowtimeReached, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DANUBE_ChannelStatusUpdate(
   DSL_IN DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LatencyPath_t nLPath;
   DSL_uint16_t nDataPath = 0;
   DSL_uint8_t nChannel = 0;
   DSL_AccessDir_t nDirection = DSL_ACCESSDIR_NA;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_ChannelStatusUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   for (nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++)
   {
      for (nDirection = DSL_UPSTREAM; nDirection <= DSL_DOWNSTREAM; nDirection++)
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_STAT,
                    (nDirection == DSL_UPSTREAM) ? DSL_CMV_ADDRESS_STAT_PATH_US :
                                                   DSL_CMV_ADDRESS_STAT_PATH_DS,
                    0, 1, &nDataPath);

         nLPath = DSL_LATENCY_UNKNOWN;

         if (nErrCode == DSL_SUCCESS)
         {
            if (nDataPath & 0x1)
            {
               nLPath = DSL_LATENCY_IP_LP0;
            }
            else if (nDataPath & 0x2)
            {
               nLPath = DSL_LATENCY_FP_LP1;
            }
            else
            {
               nLPath = DSL_LATENCY_DISABLED;
            }
         }

         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nLPath[nDirection][nChannel], nLPath);
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_ChannelStatusUpdate, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DANUBE_ChannelStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_uint16_t nChannel,
   DSL_IN DSL_AccessDir_t nDirection,
   DSL_IN_OUT DSL_G997_ChannelStatusData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nAddrProps = 0, nOffset = 0, nData = 0;
   /* $$TD: The handling for the latency path/bearer channel status update
      should be included in a central place after showtime is reached and
      stored within context (channel specific) to be used here for example.
      NOTE:
      Update for all bearer channels should be done together in this separate
      function (implementation below handles only bearer channel 0 which is the
      only possible at the moment).
   */
   DSL_LatencyPath_t nLPath = DSL_LATENCY_UNKNOWN;
   DSL_uint32_t nDelay = 0;
   DSL_G997_FramingParameterStatusData_t framingStatusData;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_ChannelStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   nAddrProps = nDirection == DSL_UPSTREAM ? DSL_CMV_ADDRESS_INFO_LATENCY_PATH_PROPS_US :
                                             DSL_CMV_ADDRESS_INFO_LATENCY_PATH_PROPS_DS;

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][nChannel], nLPath);

   /* Calculate offset according to current cofiguration */
   if (nLPath == DSL_LATENCY_IP_LP0)
   {
      nOffset = 0;
   }
   else if (nLPath == DSL_LATENCY_FP_LP1)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO, nAddrProps,
         0, 1, &nOffset);
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - Latency path (%d) is disabled."DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext), nChannel));

      return DSL_ERROR;
   }

   /* Calculate INP according to the framing parameters in all ADSL modes
      since the FW INFO 92/93 CMVs return incorrect results in some modes*/
   memset(&framingStatusData, 0x0, sizeof(DSL_G997_FramingParameterStatusData_t));

   /* Get G997 framing parameters*/
   nErrCode = DSL_DRV_DANUBE_G997_FramingParameterStatusGet(
                 pContext, nDirection, (DSL_uint8_t)nChannel, &framingStatusData);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - G997 framing parameters get failed!"DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext), nChannel));

      return nErrCode;
   }

   if (framingStatusData.nLSYMB != 0)
   {
      pData->ActualImpulseNoiseProtection =
         (DSL_uint8_t)(10 * 4 * framingStatusData.nRFEC * framingStatusData.nINTLVDEPTH /
                                                          framingStatusData.nLSYMB);
   }
   else
   {
      pData->ActualImpulseNoiseProtection = 255;
   }

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO, nAddrProps,
                 (DSL_uint16_t)(nOffset + 1), 1, &nData);

   if (nErrCode == DSL_SUCCESS)
   {
      /* Unit within firmware: (LATENCY * 4), unit within G.997.1: (LATENCY * 100)
         => (LATENCY(fw) * 100) / 4 = LATENCY (G.997.1) */
      nDelay = (nData * 100) / 4;
      pData->ActualInterleaveDelay = nDelay;
   }


   pData->ActualDataRate   = pContext->nChannelActualDataRate[nDirection][nChannel];
   pData->PreviousDataRate = pContext->nChannelPreviousDataRate[nDirection][nChannel];

   return nErrCode;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_ChannelRateUpdate(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_G997_ChannelStatusData_t chStsData;
   DSL_LatencyPath_t nLPath = DSL_LATENCY_DISABLED;
   DSL_uint32_t nActualDataRate = 0;
#ifdef INCLUDE_DSL_G997_ALARM
   DSL_G997_DataRateThresholdCrossingType_t nDataRateCrossingType;
   DSL_uint32_t nDataRateShiftThresh = 0;
   DSL_boolean_t bNotifyDataRateShift = {DSL_FALSE};
#ifdef INCLUDE_DSL_ADSL_MIB
   DSL_MIB_ADSL_Thresholds_t nMibThresholds = DSL_MIB_TRAPS_NOTHING;
#endif
#endif /* INCLUDE_DSL_G997_ALARM*/
   DSL_uint32_t nDataRate, nDataRate_remain, nOldDataRate, drDiv1, drDiv2;
   DSL_uint16_t nRate16[2] = {0};
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nCh;
   DSL_uint16_t nMp, nLp, nTp, nRp, nBpn, nKp;
   DSL_boolean_t bPrevDataRateValid = DSL_FALSE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_ChannelRateUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   /* Get the previous data rate valid flag */
   DSL_CTX_READ_SCALAR(pContext, nErrCode, bPrevDataRateValid, bPrevDataRateValid);

   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC &&
       nLineState != DSL_LINESTATE_SHOWTIME_NO_SYNC)
   {
      return DSL_ERROR;
   }

   for (nCh = 0; nCh < DSL_CHANNELS_PER_LINE; nCh++)
   {
      if (DSL_DRV_DANUBE_ActLatencyGet(pContext, nCh, nDirection, &nLPath) < DSL_SUCCESS)
      {
         continue;
      }

      DSL_CTX_READ_SCALAR(
         pContext, nErrCode, nChannelActualDataRatePrev[nDirection][nCh], nOldDataRate);

      /* In case of ADSL1 the values for the data rates can be directly
         read via CMVs */
      if (bAdsl1)
      {
         DSL_uint16_t idx = 0;

         if (nLPath == DSL_LATENCY_FP_LP1)
         {
            idx = 2;
         }

         nErrCode = DSL_DRV_DANUBE_CmvRead(
                       pContext, DSL_CMV_GROUP_RATE,
                       (DSL_uint16_t)((nDirection == DSL_UPSTREAM) ?
                       DSL_CMV_ADDRESS_RATE_UsRate : DSL_CMV_ADDRESS_RATE_DsRate),
                       idx, 2, nRate16);

         if (nErrCode != DSL_SUCCESS)
         {
            return nErrCode;
         }
         nActualDataRate = (nRate16[1] << 16) + nRate16[0];
      }
      else
      {
      /* In case of ADSL2/2+ the values for data rates has to be calculated
         using framing parameters */
         nMp = nLp = nTp = nRp = nKp = nBpn = nDataRate = nDataRate_remain = 0;

         nErrCode = DSL_DRV_DANUBE_CmvRead(
                       pContext, DSL_CMV_GROUP_CNFG,
                       (DSL_uint16_t)((nDirection == DSL_UPSTREAM) ?
                       DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_LP :
                       DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_LP),
                       nCh, 1, &nLp);

         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(
                          pContext, DSL_CMV_GROUP_CNFG,
                          (DSL_uint16_t)((nDirection == DSL_UPSTREAM) ?
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_RP :
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_RP),
                          nCh, 1, &nRp);
         }

         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(
                          pContext, DSL_CMV_GROUP_CNFG,
                          (DSL_uint16_t)((nDirection == DSL_UPSTREAM) ?
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_MP :
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_MP),
                          nCh, 1, &nMp);
         }

         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(
                          pContext, DSL_CMV_GROUP_CNFG,
                          (DSL_uint16_t)((nDirection == DSL_UPSTREAM) ?
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_TP :
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_TP),
                          nCh, 1, &nTp);
         }

         if (nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_DANUBE_CmvRead(
                          pContext, DSL_CMV_GROUP_CNFG,
                          (DSL_uint16_t)((nDirection == DSL_UPSTREAM) ?
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_BP :
                          DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_BP),
                          (DSL_uint16_t)(2* nCh), 2, nRate16);
         }

         if (nErrCode == DSL_SUCCESS)
         {
            nBpn = nRate16[0] + nRate16[1];
            nKp  = nBpn + 1;

            drDiv1 = (DSL_uint32_t)((nTp * (nBpn + 1) - 1) * nMp * nLp * 4);
            drDiv2 = (DSL_uint32_t)(nTp * (nKp * nMp + nRp));

            if (drDiv2 != 0)
            {
               nDataRate        = drDiv1 / drDiv2;
               nDataRate_remain = ((DSL_uint32_t)((drDiv1 % drDiv2) * 1000)) / drDiv2;
            }
            else
            {
               nDataRate = 0;
               nDataRate_remain = 0;

               DSL_DEBUG (DSL_DBG_WRN, (pContext,
                  "DSL[%02d]: WRN - %s nTp=%d, nKp=%d, nMp=%d, nRp=%d not acceptable!"
                  DSL_DRV_CRLF, DSL_DEV_NUM(pContext),
                  (nDirection == DSL_UPSTREAM) ? "US" : "DS",
                  nTp, nKp, nMp, nRp));
            }

            nActualDataRate = nDataRate * 1000 + nDataRate_remain;
         }
      }

      /* Update Actual Data Rate (prev)*/
      DSL_CTX_WRITE_SCALAR(
         pContext, nErrCode, nChannelActualDataRatePrev[nDirection][nCh], nActualDataRate);

      /* Update Actual Data Rate*/
      DSL_CTX_WRITE_SCALAR(
         pContext, nErrCode, nChannelActualDataRate[nDirection][nCh], nActualDataRate);

      if (nOldDataRate != nActualDataRate && nErrCode == DSL_SUCCESS)
      {
         /* Set previous data rate value */
         DSL_CTX_WRITE_SCALAR(
            pContext, nErrCode, nChannelPreviousDataRate[nDirection][nCh], nOldDataRate);

         /* Get channel status*/
         nErrCode = DSL_DRV_DANUBE_ChannelStatusGet(
                       pContext, nCh, nDirection, &chStsData);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Channel status get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
         else
         {
            chStsData.ActualDataRate   = nActualDataRate;
            chStsData.PreviousDataRate = nOldDataRate;

            nErrCode = DSL_DRV_EventGenerate(
                          pContext, (DSL_uint8_t)nCh,
                          nDirection, DSL_XTUDIR_NA,
                          DSL_EVENT_S_CHANNEL_DATARATE,
                         (DSL_EventData_Union_t*)&chStsData,
                          sizeof(DSL_G997_ChannelStatusData_t));

            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Event(%d) generate failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_EVENT_S_CHANNEL_DATARATE));
            }
         }
      }

#ifdef INCLUDE_DSL_G997_ALARM
      if (nOldDataRate != 0 && nErrCode == DSL_SUCCESS)
      {
         /* Check for the Data Rate Upshift */
         if( nActualDataRate > nOldDataRate )
         {
            DSL_CTX_READ_SCALAR(
               pContext, nErrCode, channelDataRateThreshold[nDirection].nDataRateThresholdUpshift,
               nDataRateShiftThresh);

            if( nActualDataRate - nOldDataRate > nDataRateShiftThresh)
            {
               bNotifyDataRateShift  = DSL_TRUE;
               nDataRateCrossingType = DSL_G997_DATARATE_THRESHOLD_UPSHIFT;
            }
         }

         /* Check for the Data Rate Downshift*/
         if( nActualDataRate < nOldDataRate && nErrCode == DSL_SUCCESS )
         {
            DSL_CTX_READ_SCALAR(
               pContext, nErrCode, channelDataRateThreshold[nDirection].nDataRateThresholdDownshift,
               nDataRateShiftThresh);

            if( nOldDataRate - nActualDataRate > nDataRateShiftThresh)
            {
               bNotifyDataRateShift  = DSL_TRUE;
               nDataRateCrossingType = DSL_G997_DATARATE_THRESHOLD_DOWNSHIFT;
            }
         }

         if( bNotifyDataRateShift == DSL_TRUE && nErrCode == DSL_SUCCESS)
         {
            nErrCode = DSL_DRV_EventGenerate(
                          pContext, (DSL_uint8_t)nCh, nDirection, DSL_XTUDIR_NA,
                          DSL_EVENT_I_CHANNEL_DATARATE_SHIFT_THRESHOLD_CROSSING,
                          (DSL_EventData_Union_t*)&nDataRateCrossingType,
                          sizeof(DSL_G997_DataRateThresholdCrossingType_t));

            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Event(%d) generate failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), DSL_EVENT_I_CHANNEL_DATARATE_SHIFT_THRESHOLD_CROSSING));
               return nErrCode;
            }
#ifdef INCLUDE_DSL_ADSL_MIB
            DSL_CTX_READ_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
               nMibThresholds);

            if (nDirection == DSL_UPSTREAM)
            {
               DSL_CTX_WRITE_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
                  nMibThresholds | DSL_MIB_THRESHOLD_ATUR_RATE_CHANGE_FLAG);
            }
            else
            {
               DSL_CTX_WRITE_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
                  nMibThresholds | DSL_MIB_THRESHOLD_ATUC_RATE_CHANGE_FLAG);
            }
#endif /* INCLUDE_DSL_ADSL_MIB*/
         }
      }
#endif /* INCLUDE_DSL_G997_ALARM*/
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_ChannelRateUpdate, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_LineFailuresUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nVal, nVal2, i;
   DSL_uint32_t nFailures = 0;
   DSL_uint32_t nFailures2[DSL_CHANNELS_PER_LINE] = {0};
#ifdef INCLUDE_DSL_G997_ALARM
   DSL_uint32_t nCfgDPFM_NE = 0;
   DSL_uint32_t nCfgDPFM_FE = 0;
   DSL_uint32_t nCfgLFM_NE = 0;
   DSL_uint32_t nCfgLFM_FE = 0;
#endif /* INCLUDE_DSL_G997_ALARM*/
#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
    (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
   DSL_uint32_t nActLineFailuresMask = 0;
   DSL_uint32_t nActDataPathFailuresMask[DSL_CHANNELS_PER_LINE] = {0};
   DSL_uint32_t nPreLineFailures = 0;
   DSL_uint32_t nPreDataPathFailures[DSL_CHANNELS_PER_LINE] = {0};
#endif /* defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) &&
          defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) ||
          defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))*/
#if defined(INCLUDE_DSL_PM)
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   DSL_pmLineEventShowtimeData_t pmLineEventShowtimeCounters;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   DSL_pmDataPathFailureData_t pmDataPathFailureCounters;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/
#endif /* defined(INCLUDE_DSL_PM)*/

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_LineFailuresUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
   (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
   /* Get Previous NE Failures information*/
   DSL_CTX_READ(pContext, nErrCode, nLineFailuresNe, nPreLineFailures);
   DSL_CTX_READ(pContext, nErrCode, nDataPathFailuresNe[0], nPreDataPathFailures[0]);
   if (DSL_CHANNELS_PER_LINE == 2)
   {
      DSL_CTX_READ(pContext, nErrCode, nDataPathFailuresNe[1], nPreDataPathFailures[1]);
   }
#endif

   if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
      DSL_CMV_ADDRESS_PLAM_NE_FAILURES, 0, 1, &nVal)) == DSL_SUCCESS)
   {
      if (nVal & 0x1)
      {
         nFailures |= DSL_G997_LINEFAILURE_LOS;

         /* Check for transition to L3 */
         if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
            DSL_CMV_ADDRESS_STAT_MISC, 0, 1, &nVal2)) == DSL_SUCCESS)
         {
            if (nVal2 & (1 << 14))
            {
               DSL_CTX_WRITE_SCALAR(pContext, nErrCode,
                  powerMgmtStatus.nPowerManagementStatus, DSL_G997_PMS_L3);
            }
         }
         else
         {
            DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: ERROR - Could not"
               " read CO L3 request status CMV"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }

      nFailures |= (nVal & 0x2 ? DSL_G997_LINEFAILURE_LOF : 0x0);
      nFailures |= (nVal & 0x4 ? DSL_G997_LINEFAILURE_LPR : 0x0);
      nFailures |= (nVal & 0x2000 ? DSL_G997_LINEFAILURE_LOM : 0x0);

      if (pContext->nXDslMode >= DSL_XDSLMODE_G_992_3)
      {
         if (nVal & (1 << 8))
         {
            nFailures2[0] |= DSL_G997_DATAPATHFAILURE_NCD;
         }
         if (DSL_CHANNELS_PER_LINE == 2 && (nVal & (1 << 9)))
         {
            nFailures2[1] |= DSL_G997_DATAPATHFAILURE_NCD;
         }
         if (nVal & (1 << 10))
         {
            nFailures2[0] |= DSL_G997_DATAPATHFAILURE_LCD;
         }
         if (DSL_CHANNELS_PER_LINE == 2 && nVal & (1 << 11))
         {
            nFailures2[1] |= DSL_G997_DATAPATHFAILURE_LCD;
         }
      }
      else
      {
         for (i = 0; i < DSL_CHANNELS_PER_LINE; i++)
         {
            if (pContext->nLPath[DSL_UPSTREAM][i] == DSL_LATENCY_IP_LP0)
            {
               if (nVal & (1 << 4))
               {
                  nFailures2[i] |= DSL_G997_DATAPATHFAILURE_NCD;
               }
               if (nVal & (1 << 6))
               {
                  nFailures2[i] |= DSL_G997_DATAPATHFAILURE_LCD;
               }
            }
            else if (pContext->nLPath[DSL_UPSTREAM][i] == DSL_LATENCY_FP_LP1)
            {
               if (nVal & (1 << 5))
               {
                  nFailures2[i] |= DSL_G997_DATAPATHFAILURE_NCD;
               }
               if (nVal & (1 << 7))
               {
                  nFailures2[i] |= DSL_G997_DATAPATHFAILURE_LCD;
               }
            }
         }
      }

#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
    (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
      /* Get Actual Failures Mask*/
      DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LOS, nActLineFailuresMask);
      DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LOF, nActLineFailuresMask);
      DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LPR, nActLineFailuresMask);
      DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LOM, nActLineFailuresMask);

      DSL_ACT_FAILURE_MASK_GET(nFailures2[0], nPreDataPathFailures[0], DSL_G997_DATAPATHFAILURE_NCD, nActDataPathFailuresMask[0]);
      DSL_ACT_FAILURE_MASK_GET(nFailures2[0], nPreDataPathFailures[0], DSL_G997_DATAPATHFAILURE_LCD, nActDataPathFailuresMask[0]);
      if (DSL_CHANNELS_PER_LINE == 2)
      {
         DSL_ACT_FAILURE_MASK_GET(nFailures2[1], nPreDataPathFailures[1], DSL_G997_DATAPATHFAILURE_NCD, nActDataPathFailuresMask[1]);
         DSL_ACT_FAILURE_MASK_GET(nFailures2[1], nPreDataPathFailures[1], DSL_G997_DATAPATHFAILURE_LCD, nActDataPathFailuresMask[1]);
      }
#endif

      /*
      Update PM Near-End Event Showtime counters. Increment appropriate counter only in case
      of corresponding failure switches from 0 to 1.
      */
#if defined(INCLUDE_DSL_PM)
#if defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS)
      DSL_CTX_READ(pContext, nErrCode, pmLineEventShowtimeCounters, pmLineEventShowtimeCounters);

      /* Update LOS counter*/
      if ((nFailures & DSL_G997_LINEFAILURE_LOS) &&
          (nActLineFailuresMask & DSL_G997_LINEFAILURE_LOS))
      {
         pmLineEventShowtimeCounters.data_ne.nLOS++;
      }

      /* Update LOF counter*/
      if ((nFailures & DSL_G997_LINEFAILURE_LOF) &&
          (nActLineFailuresMask & DSL_G997_LINEFAILURE_LOF))
      {
         pmLineEventShowtimeCounters.data_ne.nLOF++;
      }

      /* Update LPR counter*/
      if ((nFailures & DSL_G997_LINEFAILURE_LPR) &&
          (nActLineFailuresMask & DSL_G997_LINEFAILURE_LPR))
      {
         pmLineEventShowtimeCounters.data_ne.nLPR++;
      }

      /* Update LOM counter*/
      if ((nFailures & DSL_G997_LINEFAILURE_LOM) &&
          (nActLineFailuresMask & DSL_G997_LINEFAILURE_LOM))
      {
         pmLineEventShowtimeCounters.data_ne.nLOM++;
      }

      /* Update Line Event Showtime counters in the DSL CPE Context*/
      DSL_CTX_WRITE(pContext, nErrCode, pmLineEventShowtimeCounters, pmLineEventShowtimeCounters);
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)
      DSL_CTX_READ(pContext, nErrCode, pmDataPathFailureCounters, pmDataPathFailureCounters);

      /* Update NCD counter*/
      if ((nFailures2[0] & DSL_G997_DATAPATHFAILURE_NCD) &&
          (nActDataPathFailuresMask[0] & DSL_G997_DATAPATHFAILURE_NCD))
      {
         pmDataPathFailureCounters.data_ne[0].nNCD++;
      }

      /* Update LCD counter*/
      if ((nFailures2[0] & DSL_G997_DATAPATHFAILURE_LCD) &&
          (nActDataPathFailuresMask[0] & DSL_G997_DATAPATHFAILURE_LCD))
      {
         pmDataPathFailureCounters.data_ne[0].nLCD++;
      }

      if (DSL_CHANNELS_PER_LINE == 2)
      {
         /* Update NCD counter*/
         if ((nFailures2[1] & DSL_G997_DATAPATHFAILURE_NCD) &&
             (nActDataPathFailuresMask[1] & DSL_G997_DATAPATHFAILURE_NCD))
         {
            pmDataPathFailureCounters.data_ne[1].nNCD++;
         }

         /* Update LCD counter*/
         if ((nFailures2[1] & DSL_G997_DATAPATHFAILURE_LCD) &&
             (nActDataPathFailuresMask[1] & DSL_G997_DATAPATHFAILURE_LCD))
         {
            pmDataPathFailureCounters.data_ne[1].nLCD++;
         }
      }

      /* Update Failure counters in the DSL CPE Context*/
      DSL_CTX_WRITE(pContext, nErrCode, pmDataPathFailureCounters, pmDataPathFailureCounters);
#endif /* defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)*/
#endif /* defined(INCLUDE_DSL_PM)*/

      DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: DSL_DRV_DANUBE_LineFailuresUpdate: NE "
         "nVal = %d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nVal));

      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nLineFailuresNe, nFailures);

      DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: nLineFailuresNe = %d"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nFailures));

      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nDataPathFailuresNe[0], nFailures2[0]);

      DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: nDataPathFailuresNe[0] = %d"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nFailures2[0]));

      if (DSL_CHANNELS_PER_LINE == 2 && nErrCode == DSL_SUCCESS)
      {
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nDataPathFailuresNe[1], nFailures2[1]);

         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: nDataPathFailuresNe[1] = %d"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nFailures2[1]));
      }
   }

#ifdef INCLUDE_DSL_G997_ALARM
   if (nErrCode == DSL_SUCCESS)
   {
      /* Get (user) mask configuration for line failure event handling */
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nDataPathFailuresNeAlarmMask, nCfgDPFM_NE);
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineFailuresNeAlarmMask, nCfgLFM_NE);

      nActLineFailuresMask &= nCfgLFM_NE;

      if (nFailures & nActLineFailuresMask)
      {
         nErrCode = DSL_DRV_EventGenerate(
                        pContext, 0, DSL_ACCESSDIR_NA, DSL_NEAR_END,
                        DSL_EVENT_I_LINE_FAILURES,
                        (DSL_EventData_Union_t*)&nFailures,
                        sizeof(DSL_G997_LineFailuresData_t));
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Event(%d) generation failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_EVENT_I_LINE_FAILURES));

            return nErrCode;
         }
      }

      nActDataPathFailuresMask[0] &= nCfgDPFM_NE;

      if (nFailures2[0] & nActDataPathFailuresMask[0])
      {
         nErrCode = DSL_DRV_EventGenerate(
                        pContext, 0, DSL_ACCESSDIR_NA, DSL_NEAR_END,
                        DSL_EVENT_I_DATA_PATH_FAILURES,
                        (DSL_EventData_Union_t*)&nFailures2[0],
                        sizeof(DSL_G997_DataPathFailuresData_t));
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Event(%d) generation failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_EVENT_I_DATA_PATH_FAILURES));

            return nErrCode;
         }
      }

      if (DSL_CHANNELS_PER_LINE == 2)
      {
         nActDataPathFailuresMask[1] &= nCfgDPFM_NE;

         if (nFailures2[1] & nActDataPathFailuresMask[1])
         {
            nErrCode = DSL_DRV_EventGenerate(
                          pContext, 1, DSL_ACCESSDIR_NA, DSL_NEAR_END,
                          DSL_EVENT_I_DATA_PATH_FAILURES,
                          (DSL_EventData_Union_t*)&nFailures2[1],
                          sizeof(DSL_G997_DataPathFailuresData_t));
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Event(%d) generation failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), DSL_EVENT_I_DATA_PATH_FAILURES));

               return nErrCode;
            }
         }
      }
   }
#endif /* INCLUDE_DSL_G997_ALARM*/

   if (nErrCode == DSL_SUCCESS)
   {
      nFailures = 0;
      nFailures2[0] = 0;
#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
    (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
      nActLineFailuresMask = 0;
      nActDataPathFailuresMask[0] = 0;
#endif
      if (DSL_CHANNELS_PER_LINE == 2)
      {
         nFailures2[1] = 0;
#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
    (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
         nActDataPathFailuresMask[1] = 0;
#endif
      }

#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
    (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
      /* Get Previous FE Failures information*/
      DSL_CTX_READ(pContext, nErrCode, nLineFailuresFe, nPreLineFailures);
      DSL_CTX_READ(pContext, nErrCode, nDataPathFailuresFe[0], nPreDataPathFailures[0]);
      if (DSL_CHANNELS_PER_LINE == 2)
      {
         DSL_CTX_READ(pContext, nErrCode, nDataPathFailuresFe[1], nPreDataPathFailures[1]);
      }
#endif

      if ((nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_PLAM,
         DSL_CMV_ADDRESS_PLAM_FE_FAILURES, 0, 1, &nVal)) == DSL_SUCCESS)
      {
         nFailures |= nVal & 0x1 ? DSL_G997_LINEFAILURE_LOS : 0x0;
         nFailures |= nVal & 0x2 ? DSL_G997_LINEFAILURE_LOF : 0x0;
         nFailures |= nVal & 0x4 ? DSL_G997_LINEFAILURE_LPR : 0x0;

         nFailures2[0] |= nVal & 0x100 ? DSL_G997_DATAPATHFAILURE_NCD : 0x0;
         nFailures2[0] |= nVal & 0x400 ? DSL_G997_DATAPATHFAILURE_LCD : 0x0;

#if (DSL_CHANNELS_PER_LINE == 2)
         nFailures2[1] |= nVal & 0x200 ? DSL_G997_DATAPATHFAILURE_NCD : 0x0;
         nFailures2[1] |= nVal & 0x800 ? DSL_G997_DATAPATHFAILURE_LCD : 0x0;
#endif /* #if (DSL_CHANNELS_PER_LINE == 2)*/

#if defined(INCLUDE_DSL_G997_ALARM) || (defined(INCLUDE_DSL_PM) && \
    (defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) || \
    defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)))
         /* Get Actual Failures Mask*/
         DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LOS, nActLineFailuresMask);
         DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LOF, nActLineFailuresMask);
         DSL_ACT_FAILURE_MASK_GET(nFailures, nPreLineFailures, DSL_G997_LINEFAILURE_LPR, nActLineFailuresMask);

         DSL_ACT_FAILURE_MASK_GET(nFailures2[0], nPreDataPathFailures[0], DSL_G997_DATAPATHFAILURE_NCD, nActDataPathFailuresMask[0]);
         DSL_ACT_FAILURE_MASK_GET(nFailures2[0], nPreDataPathFailures[0], DSL_G997_DATAPATHFAILURE_LCD, nActDataPathFailuresMask[0]);
         if (DSL_CHANNELS_PER_LINE == 2)
         {
            DSL_ACT_FAILURE_MASK_GET(nFailures2[1], nPreDataPathFailures[1], DSL_G997_DATAPATHFAILURE_NCD, nActDataPathFailuresMask[1]);
            DSL_ACT_FAILURE_MASK_GET(nFailures2[1], nPreDataPathFailures[1], DSL_G997_DATAPATHFAILURE_LCD, nActDataPathFailuresMask[1]);
         }
#endif

   /*
      Update PM Far-End Event Showtime counters. Increment appropriate counter only in case
      of corresponding failure switches from 0 to 1.
   */
#if defined(INCLUDE_DSL_PM)
#if defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS)
         DSL_CTX_READ(pContext, nErrCode, pmLineEventShowtimeCounters, pmLineEventShowtimeCounters);

         /* Update LOS counter*/
         if ((nFailures & DSL_G997_LINEFAILURE_LOS) &&
             (nActLineFailuresMask & DSL_G997_LINEFAILURE_LOS))
         {
            pmLineEventShowtimeCounters.data_fe.nLOS++;
         }

         /* Update LOF counter*/
         if ((nFailures & DSL_G997_LINEFAILURE_LOF) &&
             (nActLineFailuresMask & DSL_G997_LINEFAILURE_LOF))
         {
            pmLineEventShowtimeCounters.data_fe.nLOF++;
         }

         /* Update LPR counter*/
         if ((nFailures & DSL_G997_LINEFAILURE_LPR) &&
             (nActLineFailuresMask & DSL_G997_LINEFAILURE_LPR))
         {
            pmLineEventShowtimeCounters.data_fe.nLPR++;
         }

         /* Update LOM counter. Not supported yet for the FE direction*/
         pmLineEventShowtimeCounters.data_fe.nLOM = 0;

         /* Update Event Showtime counters in the DSL CPE Context*/
         DSL_CTX_WRITE(pContext, nErrCode, pmLineEventShowtimeCounters, pmLineEventShowtimeCounters);
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)
         DSL_CTX_READ(pContext, nErrCode, pmDataPathFailureCounters, pmDataPathFailureCounters);

         /* Update NCD counter*/
         if ((nFailures2[0] & DSL_G997_DATAPATHFAILURE_NCD) &&
             (nActDataPathFailuresMask[0] & DSL_G997_DATAPATHFAILURE_NCD))
         {
            pmDataPathFailureCounters.data_fe[0].nNCD++;
         }

         /* Update LCD counter*/
         if ((nFailures2[0] & DSL_G997_DATAPATHFAILURE_LCD) &&
             (nActDataPathFailuresMask[0] & DSL_G997_DATAPATHFAILURE_LCD))
         {
            pmDataPathFailureCounters.data_fe[0].nLCD++;
         }

         if (DSL_CHANNELS_PER_LINE == 2)
         {
            /* Update NCD counter*/
            if ((nFailures2[1] & DSL_G997_DATAPATHFAILURE_NCD) &&
                (nActDataPathFailuresMask[1] & DSL_G997_DATAPATHFAILURE_NCD))
            {
               pmDataPathFailureCounters.data_fe[1].nNCD++;
            }

            /* Update LCD counter*/
            if ((nFailures2[1] & DSL_G997_DATAPATHFAILURE_LCD) &&
                (nActDataPathFailuresMask[1] & DSL_G997_DATAPATHFAILURE_LCD))
            {
               pmDataPathFailureCounters.data_fe[1].nLCD++;
            }
         }

         /* Update Failure counters in the DSL CPE Context*/
         DSL_CTX_WRITE(pContext, nErrCode, pmDataPathFailureCounters, pmDataPathFailureCounters);
#endif /* defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)*/
#endif /* defined(INCLUDE_DSL_PM)*/

         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: DSL_DRV_DANUBE_LineFailuresUpdate: FE "
            "nVal = %d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nVal));

         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nLineFailuresFe, nFailures);

         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: nLineFailuresFe = %d"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nFailures));

         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nDataPathFailuresFe[0], nFailures2[0]);

         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: nDataPathFailuresFe[0] = %d"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nFailures2[0]));

         if (DSL_CHANNELS_PER_LINE == 2 && nErrCode == DSL_SUCCESS)
         {
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nDataPathFailuresFe[1], nFailures2[1]);

            DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: nDataPathFailuresFe[1] = %d"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nFailures2[1]));
         }
      }
   }

#ifdef INCLUDE_DSL_G997_ALARM
   if (nErrCode == DSL_SUCCESS)
   {
      /* Get (user) mask configuration for line failure event handling */
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nDataPathFailuresFeAlarmMask, nCfgDPFM_FE);
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineFailuresFeAlarmMask, nCfgLFM_FE);

      nActLineFailuresMask &= nCfgLFM_FE;

      if (nFailures & nActLineFailuresMask)
      {
         nErrCode = DSL_DRV_EventGenerate(
                        pContext, 0, DSL_ACCESSDIR_NA, DSL_FAR_END,
                        DSL_EVENT_I_LINE_FAILURES,
                        (DSL_EventData_Union_t*)&nFailures,
                        sizeof(DSL_G997_LineFailuresData_t));
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Event(%d) generation failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_EVENT_I_LINE_FAILURES));

            return nErrCode;
         }
      }

      nActDataPathFailuresMask[0] &= nCfgDPFM_FE;

      if (nFailures2[0] & nActDataPathFailuresMask[0])
      {
         nErrCode = DSL_DRV_EventGenerate(
                        pContext, 0, DSL_ACCESSDIR_NA, DSL_FAR_END,
                        DSL_EVENT_I_DATA_PATH_FAILURES,
                        (DSL_EventData_Union_t*)&nFailures2[0],
                        sizeof(DSL_G997_DataPathFailuresData_t));
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Event(%d) generation failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), DSL_EVENT_I_DATA_PATH_FAILURES));

            return nErrCode;
         }
      }

      if (DSL_CHANNELS_PER_LINE == 2)
      {
         nActDataPathFailuresMask[1] &= nCfgDPFM_FE;

         if (nFailures2[1] & nActDataPathFailuresMask[1])
         {
            nErrCode = DSL_DRV_EventGenerate(
                          pContext, 1, DSL_ACCESSDIR_NA, DSL_FAR_END,
                          DSL_EVENT_I_DATA_PATH_FAILURES,
                          (DSL_EventData_Union_t*)&nFailures2[1],
                          sizeof(DSL_G997_DataPathFailuresData_t));
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Event(%d) generation failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), DSL_EVENT_I_DATA_PATH_FAILURES));

               return nErrCode;
            }
         }
      }
   }
#endif /* INCLUDE_DSL_G997_ALARM*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_LineFailuresUpdate, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined(INCLUDE_DSL_FRAMING_PARAMETERS) || defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)
DSL_Error_t DSL_DRV_DANUBE_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_FramingParameterStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nCh = pData->nChannel, nSEQ;
   DSL_uint16_t nAddrMp, nAddrTp, nAddrBp, nAddrMSGc, nAddrMSGLp, nIdxBp = 0;
   DSL_LatencyPath_t nLp = DSL_LATENCY_UNKNOWN;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint8_t nOption = 0;
   DSL_uint8_t nOffset = 0;
   DSL_uint16_t nS = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_FramingParameterStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_CHANNEL_RANGE(pData->nChannel);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_ERR_CODE();

   if (pData->nDirection == DSL_UPSTREAM)
   {
      nAddrMp = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_MP;
      nAddrTp = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_TP;
      nAddrBp = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_BP;
      nAddrMSGLp = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_MSGLp;
      nAddrMSGc = DSL_CMV_ADDRESS_CNFG_DATA_RATEUS_FLAG_ADSL2_MSGc;
   }
   else
   {
      nAddrMp = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_MP;
      nAddrTp = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_TP;
      nAddrBp = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_BP;
      nAddrMSGLp = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_MSGLp;
      nAddrMSGc = DSL_CMV_ADDRESS_CNFG_DATA_RATEDS_FLAG_ADSL2_MSGc;
   }

   /* Update channel status for the case if it was not updated yet */
   nErrCode = DSL_DRV_DANUBE_ChannelStatusUpdate(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get Actual Latency Path information*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[pData->nDirection][pData->nChannel], nLp);
   nIdxBp = nCh;
   if (nLp == DSL_LATENCY_FP_LP1)
   {
      nIdxBp += 2;
   }

   /* Get Tp value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrTp, nCh, 1, &pData->data.nTp);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get Bp value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrBp, nIdxBp, 1, &pData->data.nBP);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get MSGC value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrMSGc, nCh, 1, &pData->data.nMSGC);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get MSGLP value*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                 nAddrMSGLp, nCh, 1, &pData->data.nMSGLP);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /* Check for the ADSL1 mode indication*/
   if (bAdsl1)
   {
      /* Get C-RATES-2 direction specific option*/
      nErrCode = DSL_DRV_DANUBE_CRATES2OptionGet(pContext, pData->nDirection, &nOption);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - C-RATES2 get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      /*
      CMV specification is wrong for the C-RATES-RA format.
      Each 16-bit offset contains 1 byte from the C-RATES-RA.
      */

      /* Get Upstream RSf*/
      nOffset = pData->nDirection == DSL_UPSTREAM ? 27 : 22;
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_CRATESRA,
                    (DSL_uint16_t)((nOption - 1) * 30 + nOffset), 1, &nS);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - RSf get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      pData->data.nMp = nS & 0x3F;

      /* Not supported*/
      pData->data.nMSGC = 0x0;
      pData->data.nSEQ  = 0x0;

      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }
   else
   {
      /* Get Mp directly from the FW*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNFG,
                    nAddrMp, nCh, 1, &pData->data.nMp);
      if (nErrCode != DSL_SUCCESS)
      {
         return nErrCode;
      }

      /* Calculate the value for SEQ only in ADSL2/2+.
         For a description of the rules to set SEQ value please refer to
         ITU-T Rec. G.992.3, Table 7-7 */
      if (nCh == 0)
      {
         nSEQ = pData->data.nMSGLP == 0 ? pData->data.nMSGC + 6 : 6;
      }
      else
      {
         nSEQ = pData->data.nMSGLP == 0 ? 2 : pData->data.nMSGC + 2;
      }
      pData->data.nSEQ = nSEQ;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_FramingParameterStatusGet, "
      "retCode(%d)"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* defined(INCLUDE_DSL_FRAMING_PARAMETERS) || defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
#ifdef INCLUDE_DSL_FRAMING_PARAMETERS
DSL_Error_t DSL_DRV_DEV_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_FramingParameterStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_FramingParameterStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_DIRECTION(pData->nDirection);
   DSL_CHECK_CHANNEL_RANGE(pData->nChannel);
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

   nErrCode = DSL_DRV_DANUBE_FramingParameterStatusGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_FramingParameterStatusGet, "
      "retCode(%d)"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_FRAMING_PARAMETERS*/

/* HDLC block */

DSL_Error_t DSL_DRV_DANUBE_HdlcMutexControl(
   DSL_Context_t *pContext,
   DSL_boolean_t bLock)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( bLock )
   {
      /* Lock HDLC Mutex*/
      if( DSL_DRV_MUTEX_LOCK(pContext->hdlcMutex) )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock HDLC mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_SEMAPHORE_GET;
      }
   }
   else
   {
       /* Unlock HDLC Mutex*/
       DSL_DRV_MUTEX_UNLOCK(pContext->hdlcMutex);
   }

   return nErrCode;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_HdlcStatusGet (
   DSL_Context_t *pContext,
   DSL_uint16_t *pnStatus)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_HdlcStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get HDLC status */
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
      DSL_CMV_ADDRESS_STAT_ME_HDLC, 0, 1, pnStatus);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcStatusGet, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
 * Check if the me is resolved.
 *
 * \param   status      the me status
 * \return  ME_HDLC_UNRESOLVED or ME_HDLC_RESOLVED
 * \ingroup Internal
 */
/*
   This function does not check pContext pointer
*/
static DSL_int_t DSL_DRV_DANUBE_HdlcResolvedGet(
   DSL_Context_t *pContext,
   DSL_uint16_t nStatus)
{
   DSL_uint16_t nData = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_HdlcResolvedGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (nStatus == DSL_ME_HDLC_MSG_QUEUED || nStatus == DSL_ME_HDLC_MSG_SENT ||
       nStatus == DSL_ME_HDLC_MSG_NOT_SUPPORTED)
   {
      return DSL_ME_HDLC_UNRESOLVED;
   }

   if (nStatus == DSL_ME_HDLC_IDLE)
   {
      /* Get ME-HDLC Control */
      if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_CNTL,
             DSL_CMV_ADDRESS_CNTL_ME_HDLC, 0, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcResolvedGet"
            ", retCode=(DSL_ME_HDLC_URESOLVED)"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ME_HDLC_UNRESOLVED;
      }
      if (nData & (1 << 0))
      {
         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcResolvedGet"
            ", retCode=(DSL_ME_HDLC_URESOLVED)"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ME_HDLC_UNRESOLVED;
      }
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcResolvedGet"
      ", retCode=(DSL_ME_HDLC_RESOLVED)"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   return DSL_ME_HDLC_RESOLVED;
}

/*
   This function does not check pContext pointer
*/
static DSL_Error_t DSL_DRV_DANUBE_HdlcSend(
   DSL_Context_t *pContext,
   DSL_uint8_t *pHdlcPkt,
   DSL_int_t nPktLen,
   DSL_int_t nMaxLength)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nData = 0;
   DSL_uint16_t nLen = 0;
   DSL_uint16_t nRxLength = 0;
   DSL_int_t nWriteSize = 0;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_HdlcSend"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (nPktLen > nMaxLength)
   {
      /* Get ME-HDLC Control */
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 2, 1, &nRxLength);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - RX Length for HDLC get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }

      if (nRxLength + nMaxLength < nPktLen)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - Exceed maximum eoc "
            "rx(%d)+tx(%d) message length!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nRxLength, nMaxLength));

         return DSL_ERROR;
      }

      nData = 1;
      /* disable RX EOC */
      nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_RX_CLEAR_EOC, 6, 1, &nData);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Could not disable RX EOC"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }
   }

   while (nLen < nPktLen)
   {
      nWriteSize = nPktLen - nLen;
      if (nWriteSize > 24)
      {
         nWriteSize = 24;
      }

      nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_ME_HDLC_TX_BUFFER, (DSL_uint16_t)(nLen / 2),
         (nWriteSize + 1) / 2, (DSL_uint16_t*)(pHdlcPkt + nLen));
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - Could not write TX buffer!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }

      nLen += ((DSL_uint16_t)nWriteSize);
   }

   /* Update tx message length */
   nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
      DSL_CMV_ADDRESS_INFO_ME_HDLC_PARAMS, 2, 1, &nLen);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Could not write TX buffer length!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   /* Start to send */
   nData = (1 << 0);
   nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_CNTL,
      DSL_CMV_ADDRESS_CNTL_ME_HDLC, 0, 1, &nData);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - Could not write TX buffer length!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcSend"
      ", retCode=(DSL_SUCCESS)"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   return DSL_SUCCESS;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_danube.h'
*/
DSL_Error_t DSL_DRV_DANUBE_HdlcWrite (
   DSL_Context_t *pContext,
   DSL_boolean_t bDeviceLock,
   DSL_uint8_t *pHdlcPkt,
   DSL_int_t nHdlcPktLen)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nHdlcStatus = 0;
   DSL_uint16_t nMaxHdlcTxLength = 0;
   DSL_uint16_t nRetry = 0;
   DSL_int_t nSendBusyCounter = 0;
   DSL_boolean_t bSendRetry = DSL_FALSE;
   DSL_uint16_t nData = 0;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_HdlcWrite"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pHdlcPkt);
   DSL_CHECK_ERR_CODE();

   if (bDeviceLock == DSL_TRUE)
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Lock device!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Get ADSL1 mode information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /* Check for the ADSL1 mode*/
   if (bAdsl1)
   {
      return DSL_ERR_DEVICE_NO_DATA;
   }

   for (;;)
   {
      /* retry 1000 times (1 sec) */
      while (nRetry < 1000)
      {
         /* Get current Line State*/
         DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);
         if (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
         {
            return DSL_ERR_DEVICE_NO_DATA;
         }

         nErrCode = DSL_DRV_DANUBE_HdlcStatusGet(pContext, &nHdlcStatus);
         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Could not get HDLC status!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            break;
         }

         /* arc ready to send HDLC message */
         if (DSL_DRV_DANUBE_HdlcResolvedGet(pContext, nHdlcStatus)
                                                       == DSL_ME_HDLC_RESOLVED)
         {
            /* Get Maximum Allowed HDLC Tx Message Length */
            nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
               DSL_CMV_ADDRESS_INFO_ME_HDLC_PARAMS, 0, 1, &nMaxHdlcTxLength);

            if (nErrCode == DSL_SUCCESS)
            {
               nErrCode = DSL_DRV_DANUBE_HdlcSend (pContext, pHdlcPkt, nHdlcPktLen, nMaxHdlcTxLength);

               DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcWrite, "
                  "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

               return nErrCode;
            }
         }
         else
         {
            if ((nHdlcStatus == DSL_ME_HDLC_MSG_SENT) ||
                (nHdlcStatus == DSL_ME_HDLC_MSG_QUEUED))
            {
               nSendBusyCounter++;
            }
            else if (nHdlcStatus == DSL_ME_HDLC_MSG_NOT_SUPPORTED)
            {
               nErrCode = DSL_ERR_DEVICE_NO_DATA;
               break;
            }
            else
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Unhandled HDLC status (%d)!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), nHdlcStatus));

               nErrCode = DSL_ERR_DEVICE_NO_DATA;
               break;
            }
         }
         nRetry++;
         DSL_WAIT (1);
      }

      if (nErrCode != DSL_SUCCESS)
      {
         break;
      }

      /* wait 10 seconds and FW still report busy -> reset FW HDLC status */
      if (nSendBusyCounter > 950 && bSendRetry == DSL_FALSE)
      {
         bSendRetry = DSL_TRUE;
         nRetry = 0;

         DSL_DEBUG( DSL_DBG_WRN,
            (pContext, "DSL[%02d]: WARNING - Reset FW HDLC status!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nSendBusyCounter = 0;
         nData = 2;

         /* force reset to */
         nErrCode = DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_CNTL,
            DSL_CMV_ADDRESS_CNTL_ME_HDLC, 0, 1, &nData);

         if (nErrCode != DSL_SUCCESS)
         {
            break;
         }
         continue;
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;

         DSL_DEBUG( DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - Hdlc Write failed!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         break;
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcWrite, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device_danube.h'
*/
DSL_Error_t DSL_DRV_DANUBE_HdlcRead (
   DSL_Context_t *pContext,
   DSL_boolean_t bDeviceLock,
   DSL_uint8_t *pHdlcPkt,
   DSL_int_t nMaxHdlcPktLen,
   DSL_int_t *pnRead)
{
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_int_t nMsgReadLen, nRetry = 0, nCurrentSize = 0;
   DSL_uint16_t nHdlcStatus = 0, nPktLen = 0;
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t buf[DSL_MAX_CMV_MSG_LENGTH];
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_HdlcRead"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pHdlcPkt);
   DSL_CHECK_POINTER(pContext, pnRead);
   DSL_CHECK_ERR_CODE();

   *pnRead = 0;

   if (bDeviceLock == DSL_TRUE)
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Lock device"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Get ADSL mode information*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /* Check for the ADSL1 mode*/
   if (bAdsl1)
   {
      return DSL_ERR_DEVICE_NO_DATA;
   }

   /* Wait max 4 sec*/
   while (nRetry < 40)
   {
      /* Get current Line State*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);
      if (nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
      {
         return DSL_ERR_DEVICE_NO_DATA;
      }

      nErrCode = DSL_DRV_DANUBE_HdlcStatusGet(pContext, &nHdlcStatus);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Could not get HDLC status!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: DSL_DRV_DANUBE_HdlcRead: HDLC status = %hu"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nHdlcStatus));

      if (nHdlcStatus == DSL_ME_HDLC_RESP_RCVD)
      {
         /* Get EoC packet length */
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_ME_HDLC_PARAMS, 3, 1, &nPktLen);

         if (nErrCode == DSL_SUCCESS)
         {
            if (nPktLen > nMaxHdlcPktLen)
            {
               DSL_DEBUG(DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: DSL_DRV_DANUBE_HdlcRead: Buffer length too small "
                  "(received %hu against %d allocated)"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), nPktLen, nMaxHdlcPktLen));

               nErrCode = DSL_ERR_MEMORY;

               break;
            }
            else
            {
               while (nCurrentSize < nPktLen)
               {
                  if (nPktLen - nCurrentSize > (DSL_MAX_CMV_MSG_LENGTH * 2 - 8))
                  {
                     nMsgReadLen = DSL_MAX_CMV_MSG_LENGTH * 2 - 8;
                  }
                  else
                  {
                     nMsgReadLen = nPktLen - nCurrentSize;
                  }

                  /* Get EoC Rx packet data*/
                  nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
                     DSL_CMV_ADDRESS_INFO_ME_HDLC_RX_BUFFER,
                     (DSL_uint16_t)(0 + (nCurrentSize / 2)), (nMsgReadLen + 1) / 2, buf);

                  if (nErrCode == DSL_SUCCESS)
                  {
                     memcpy (pHdlcPkt + nCurrentSize, buf, nMsgReadLen);
                     nCurrentSize += nMsgReadLen;
                  }
                  else
                  {
                     break;
                  }
               }
               if (nErrCode == DSL_SUCCESS)
               {
                  *pnRead = nCurrentSize;
               }
               else
               {
                  break;
               }
            }
         }
         else
         {
            DSL_DEBUG(DSL_DBG_ERR,
              (pContext, "DSL[%02d]: DSL_DRV_DANUBE_HdlcRead: HDLC status = %hu"DSL_DRV_CRLF,
              DSL_DEV_NUM(pContext), nHdlcStatus));

            break;
         }
         break;
      }
      else if (nHdlcStatus == DSL_ME_HDLC_RESP_TIMEOUT)
      {
         DSL_DEBUG(DSL_DBG_WRN,
           (pContext, "DSL[%02d]: ERROR - HDLC read FW timeout (%d ms) with status (%d)"DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext), nRetry*100, nHdlcStatus));

         nErrCode = DSL_ERR_DEVICE_NO_DATA;
         break;
      }
      else
      {
         nErrCode = DSL_ERR_DEVICE_NO_DATA;
      }

      nRetry++;

      DSL_WAIT (100);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_HdlcRead, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/* Power management block */

/*
   This function does not check pContext pointer
*/
DSL_Error_t DSL_DRV_DANUBE_L3StatusGet(
   DSL_Context_t *pContext,
   DSL_G997_PowerManagement_t *pnPowerMode)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   DSL_uint16_t nData;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_L3StatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Default mode is L3 (power off/no link) */
   *pnPowerMode = DSL_G997_PMS_L3;

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   if (DSL_DRV_DEV_ModemIsReady(pContext) == DSL_TRUE)
   {
      /* Only proceed if the specified line is in SHOWTIME state*/
      if ((nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC) ||
          (nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
            DSL_CMV_ADDRESS_STAT_POWER_STATE, 0, 1, &nData);
         if (nErrCode == DSL_SUCCESS)
         {
            *pnPowerMode = (DSL_G997_PowerManagement_t)nData;
         }
      }
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_DANUBE_L3StatusGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_DEV_DBG_DeviceMessageSend(
   DSL_Context_t *pContext,
   DSL_DeviceMessageData_t *pMsg )
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_uint16_t nSize = 0;
#ifndef WIN32
   DSL_uint16_t RxMessage[DSL_MAX_CMV_MSG_LENGTH]__attribute__ ((aligned(4)));
#else
   DSL_uint16_t RxMessage[DSL_MAX_CMV_MSG_LENGTH];
#endif /* WIN32*/
   DSL_boolean_t bWrite = DSL_FALSE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_DBG_DeviceMessageSend"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   if( ((*((DSL_uint16_t*)pMsg->pMsg)) & 0x1000) )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - not supported CMV!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED;
   }

   /* Check Rd/Wr access*/
   bWrite = DSL_DRV_DANUBE_CmvAccessTypeGet(*((DSL_uint16_t*)pMsg->pMsg));

   if (DSL_DRV_DANUBE_CmvSend(pContext, (DSL_uint16_t*)pMsg->pMsg, DSL_TRUE, RxMessage)
      != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - CMV send failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_MSG_EXCHANGE;
   }

   if (!bWrite)
   {
      /* Get CMV byte count*/
      nSize = DSL_DRV_DANUBE_CmvByteCountGet(RxMessage[0]);
   }

   if (nSize != 0)
   {
      memcpy(pMsg->pMsg, RxMessage,
         nSize <= (sizeof(RxMessage)) ? nSize : (sizeof(RxMessage)));
      pMsg->nSizeRx = nSize;
   }
   else
   {
      pMsg->nSizeRx = 0;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_DBG_DeviceMessageSend"
      ", retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)
/* All Reserved codes are not included and should be mapped to the
   LINIT_UNKNOWN by default*/
DSL_DANUBE_FailReasonTable_t g_FailReasonTable1[] = {
   /* No failure.  Does not by itself indicate a successful link.*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(0, 0, LINIT_SUCCESSFUL),
   /* Invalid Tx bit allocation table*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(1, 1, LINIT_CONFIG_ERROR),
   /* Invalid Rx bit allocation table*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(2, 2, LINIT_UNKNOWN),
   /* Message Fifo Overflow*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(4, 4, LINIT_UNKNOWN),
   /* Bitloading failure, ADSL1*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(6, 6, LINIT_CONFIG_NOT_FEASIBLE),
   /* Reserved*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(7, 11, LINIT_UNKNOWN),
   /* Error decoding C-B&G message
      Error decoding C-MSG2 or C-RATES2
      Timeout after RCMedleyRx waiting for C_SEGUE2
      Timeout after RReverbRATx waiting for C_SEGUE2
      Timeout after RReverb3Tx waiting for C_SEGUE1
      C-CRC2  CRC error
      C-CRC1 CRC error
      Timeout after RReverb5Tx waiting for C_SEGUE2
      Timeout after RReverb6Tx waiting for C_SEGUE3
      Timeout after RSegue5Tx waiting for C_SEGUE3
      Timeout after RCReverb5Rx waiting for C_SEGUE
      Timeout after RCReverbRARx waiting for C_SEGUE2
      C-CRC4  CRC error
      C-CRC5  CRC error
      C-CRC3  CRC error*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(12, 26, LINIT_COMMUNICATION_PROBLEM),
   /* DEC Path Delay timeout*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(27, 27, LINIT_UNKNOWN),
   /*
   DEC Training timeout
   Timeout after RCReverb3Rx waiting for C_SEGUE1
   Timeout waiting for the end of RCReverb2Rx signal
   Timeout waiting for the end of RQuiet2 signal
   Timeout waiting for the end of RCReverbFR1Rx signal
   Timeout waiting for the end of RCPilotFR1Rx signal
   Timeout after RCReverbFR2Rx waiting for C_SEGUE
   Timeout waiting for the end of RCReverbFR5Rx signal
   Timeout after RCReverbFR6Rx waiting for C_SEGUE
   Timeout after RCReverbFR8Rx waiting for C_SEGUE_FR4
   Timeout since no profile was selected
   Timeout waiting for the end of RCReverbFR8Rx signal
   C-CRCFR1  CRC error
   Timeout waiting for the end of RCRecovRx signal
   Timeout after RSegueFR5Tx waiting for C_SEGUE2
   Timeout waiting for the end of RRecovTx signal
   Timeout after RCMedleyFRRx waiting for C_SEGUE2 */
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(28, 44, LINIT_COMMUNICATION_PROBLEM),
   /* Timeout when transmitting FLAG in handshake cleardown*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(48, 48, LINIT_COMMUNICATION_PROBLEM),
   /* Background task fifo overflow*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(49, 49, LINIT_UNKNOWN),
   /*
   State machine timeout in R-Quiet5
   Timeout waiting for C-Tref2*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(50, 51, LINIT_COMMUNICATION_PROBLEM),
   /* DEC path delay timeout */
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(52, 52, LINIT_UNKNOWN),
   /*
   DEC Training timeout
   Timeout after RReverb5 waiting for C_SEGUE1
   Timeout in RCReverb4 waiting for C-Segue1
   C-MSG1 CRC Error
   Timeout in C-Reverb5 waiting for Segue
   C-MSG-FMT CRC Error
   Timeout in C-Quiet1,2,3 or 4
   Timeout in R-Quiet2 or 3
   C-MSG2 CRC Error
   R-Reverb6 timeout
   C-Reverb6 timeout waiting for C-Segue3
   C-Params CRC Error
   C-Reverb7 timeout waiting for Segue
   R-Reverb7 timeout
   Invalid C-MSG1 contents*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(53, 67, LINIT_COMMUNICATION_PROBLEM),
   /* R-Segue4 state machine error*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(69, 69, LINIT_COMMUNICATION_PROBLEM),
   /* CO bitload failure */
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(71, 71, LINIT_CONFIG_ERROR),
   /* CO bitload failure (infeasible parameters)*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(72, 72, LINIT_CONFIG_NOT_FEASIBLE),
   /* Unexpected received message index during Loop Diagnostics message exchange.*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(73, 73, LINIT_COMMUNICATION_PROBLEM),
   /* Insufficient DS cutback. It was detected during transceiver training that
     the cutback requested during Channel Discovery was insufficient.*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(74, 74, LINIT_UNKNOWN),
   /* Timeout when we cannot detect Pilot tone in RCQuiet2Rx*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(90, 90, LINIT_COMMUNICATION_PROBLEM),
   /* Overhead message fifo overflow*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(91, 91, LINIT_UNKNOWN),
   /* State machine error during R-Reverb2*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(94, 94, LINIT_COMMUNICATION_PROBLEM),
   /*
   Timeout during G.hs
   Rx Message buffer overflow during G.Hs due to long message from CO
   Tx Message buffer overflow during G.Hs due to long message.
   Near End Error Frame during G.Hs due to received message segment with non-matching CRC.
   Far end error frame reported by CO due to non-matching CRC
   Near end clear down in G.Hs due to non-understood or non-compliant received message
   Far end clear down in G.Hs, reported by CO due to non-understood or non-compliant message
   No common mode indicated in MS message */
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(150, 157, LINIT_COMMUNICATION_PROBLEM),
   /* Restart T1.413 after parsing CO vendor ID in C-MSG1*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(158, 158, LINIT_UNKNOWN),
   /*
   CO & CPE exchanged DMT and Plus mode Annex, which is invalid
   Far end not supported message from CO
   Far end no response from CO
   Near end no Annex bit found in CL/CLE exchange during G.Hs
   Disable Centillium work around for non centillium DSLAMs*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(159, 163, LINIT_COMMUNICATION_PROBLEM),
   /*
   ARC was reset with out a code download
   XDMA not functioning after reset
   Data swap encounters XDMA INT error
   Data swap encounters XDMA AHB error
   Data swap times out on software queue busy
   Data swap times out on XDMA engine busy
   Code swap encounters XDMA INT error
   Code swap encounters XDMA AHB error
   PPE never completes. Treating it as PPE Hung.*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(207, 215, LINIT_UNKNOWN)
};

DSL_DANUBE_FailReasonTable_t g_FailReasonTable2[] = {
   /*
   Invalid number of latency paths requested
   Invalid number of bearer channels requested*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(4, 5, LINIT_CONFIG_ERROR),
   /* Insufficient channel capacity*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(6, 6, LINIT_CONFIG_NOT_FEASIBLE),
   /* Unable to find valid framing configuration */
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(8, 10, LINIT_CONFIG_ERROR),
   /* Internal bitload error*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(11, 12, LINIT_UNKNOWN),
   /* Unsupported: Limited Rate configuration on 1st latency path for
      dual latency configuration*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(13, 13, LINIT_CONFIG_ERROR),
   /* Bitloading failure for 2nd latency path*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(14, 14, LINIT_UNKNOWN),
   /* Insufficient capacity to bitload 2nd latency path*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(15, 15, LINIT_CONFIG_NOT_FEASIBLE),
   /* Unsupported: latency > 1ms or INP > 0 for 2nd latency path*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(16, 16, LINIT_CONFIG_ERROR),
   /* Min reserved net data rate too high for 2nd latency path*/
   DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(18, 18, LINIT_CONFIG_ERROR)
};
static DSL_Error_t DSL_DRV_DANUBE_FailReasonGet(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_LineInit_t lineInitStatus = LINIT_UNKNOWN;
   DSL_DANUBE_FailReasonTable_t *pMapTable;
   DSL_uint16_t nVal = 0, i, tableSize = 0;
   DSL_uint8_t failureCode = 0;

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DANUBE_FailReasonGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
      DSL_CMV_ADDRESS_STAT_FAIL, 0, 1, &nVal);

   if (nErrCode == DSL_SUCCESS)
   {
      /* Check for the CPE bitloading failure, ADSL2 */
      if ((nVal & 0xFF) == 70)
      {
         /* Get failure code from the upper byte*/
         failureCode = (DSL_uint8_t)((nVal >> 8) & 0xFF);
         pMapTable = g_FailReasonTable2;
         tableSize = sizeof(g_FailReasonTable2)/sizeof(g_FailReasonTable2[0]);
      }
      else
      {
         failureCode = (DSL_uint8_t)(nVal & 0xFF);
         pMapTable = g_FailReasonTable1;
         tableSize = sizeof(g_FailReasonTable1)/sizeof(g_FailReasonTable1[0]);
      }

      for (i = 0; i < tableSize; i++)
      {
         if ((failureCode >= pMapTable->codeRange.firmwareCodeStart) &&
             (failureCode <= pMapTable->codeRange.firmwareCodeStop))
         {
            lineInitStatus = pMapTable->g997Code;
            break;
         }
         pMapTable++;
      }
   }

   DSL_DRV_HandleLinitValue(pContext, lineInitStatus, LINIT_SUB_NONE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_FailReasonGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return (nErrCode);
}
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_STATUS)*/

/*
   DANUBE Device specific implementation of the DSL_DRV_OnTimeoutEvent() function
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DEV_OnTimeoutEvent(
   DSL_Context_t *pContext,
   DSL_int_t nEventType,
   DSL_uint32_t nTimeoutID)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,(pContext, "DSL[%02d]: DSL_DRV_DEV_OnTimeoutEvent: "
      "nEventType=%d, nTimeoutID=0x%08X" DSL_DRV_CRLF ,
      DSL_DEV_NUM(pContext), nEventType, nTimeoutID));

   switch (nEventType)
   {
   case DSL_TIMEOUTEVENT_FE_STATUS:
      nErrCode = DSL_DRV_DANUBE_OnLineInventoryFe(pContext);
      if (nErrCode == DSL_SUCCESS)
      {
         /* Set FE line inventory flag*/
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFeLineInventoryValid, DSL_TRUE);
      }
      break;

   default:
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: Unexpected nEventType (%d)" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nEventType));
      break;
   }

   return (nErrCode);
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
static DSL_Error_t DSL_DRV_DANUBE_OnLineInventoryFe(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* Retreive all FE inventory information*/
   nErrCode = DSL_DRV_DANUBE_LineInventoryFeRead(pContext);
   if( nErrCode < DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - FE inventory get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   nErrCode = DSL_DRV_EventGenerate(
                 pContext, 0, DSL_ACCESSDIR_NA, DSL_FAR_END,
                 DSL_EVENT_S_FE_INVENTORY_AVAILABLE,
                 DSL_NULL, 0);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Event(%d) generate failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_EVENT_S_CHANNEL_DATARATE));

      return nErrCode;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

/*
   For a detailed description please refer to the drv_dsl_cpe_device_danube.h
*/
DSL_Error_t DSL_DRV_DANUBE_LineInventoryNeWrite(
   DSL_IN DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t i;
   DSL_uint16_t nData;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Send inventory info..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   for (i = 0; i < 4; i++)
   {
      nData = (pContext->lineInventoryNe.SystemVendorID[2*i+1] << 8) |
         pContext->lineInventoryNe.SystemVendorID[2*i];

      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
                           DSL_CMV_ADDRESS_INFO_VENDORID_NE,
                           (DSL_uint16_t)i, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't send vendor ID!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;

         break;
      }
   }

   for (i = 0; i < 8; i++)
   {
      nData = (pContext->lineInventoryNe.VersionNumber[2*i+1] << 8) |
         pContext->lineInventoryNe.VersionNumber[2*i];

      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
                           DSL_CMV_ADDRESS_INFO_VERSIONNUM_NE,
                           (DSL_uint16_t)i, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't send version num!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;

         break;
      }
   }

   for (i = 0; i < 16; i++)
   {
      nData = (pContext->lineInventoryNe.SerialNumber[2*i+1] << 8) |
         pContext->lineInventoryNe.SerialNumber[2*i];

      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
                           DSL_CMV_ADDRESS_INFO_SERIALNUM_NE,
                           (DSL_uint16_t)i, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't send serial number!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;

         break;
      }
   }

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_LineInventoryWrite, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_device_danube.h
*/
DSL_Error_t DSL_DRV_DANUBE_LineInventoryNeRead(
   DSL_IN DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t i;
   DSL_uint16_t nData;
   DSL_uint32_t SelfTestResult;
   DSL_uint8_t G994VendorID[DSL_G997_LI_MAXLEN_VENDOR_ID] = {0};

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Read inventory info..."DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   for (i = 0; i < DSL_G997_LI_MAXLEN_VENDOR_ID / 2; i++)
   {
      if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_VENDORID_G994_NE, (DSL_uint16_t)i, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get G994 vendor ID!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;

         break;
      }

      G994VendorID[2*i + 1] = (nData >> 8) & 0xFF;
      G994VendorID[2*i]     = nData & 0xFF;
   }

   DSL_CTX_WRITE(pContext, nErrCode, lineInventoryNe.G994VendorID, G994VendorID);

   if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      DSL_CMV_ADDRESS_INFO_SELFTEST, 2, 2,
      (DSL_uint16_t*)&SelfTestResult) != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get self test result!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }
   else
   {
      DSL_CTX_WRITE(pContext, nErrCode, lineInventoryNe.SelfTestResult, SelfTestResult);
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_LineInventoryNeRead, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the drv_dsl_cpe_device_danube.h
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DANUBE_LineInventoryFeRead(
   DSL_IN DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t i;
   DSL_uint16_t nData;
   DSL_uint16_t hdlcCmd[2];
   DSL_uint16_t hdlcRxBuffer[32];
   DSL_int_t nHdlcRxLen = 0;
   DSL_uint8_t hdlcOctet = 0;
   /* 5 SPAR octets, 6th octet is dummy and
      is needed due to the CMV 16-bit format*/
   DSL_uint8_t spar[5 + 1];
   DSL_uint8_t xtse[DSL_G997_NUM_XTSE_OCTETS] = {0};

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Read inventory info..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_MODEM_IS_READY();
   DSL_CHECK_ERR_CODE();

   /* Get G994 Vendor ID information*/
   for (i = 0; i < DSL_G997_LI_MAXLEN_VENDOR_ID / 2; i++)
   {
      if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_VENDORID_G994_FE, (DSL_uint16_t)i, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't get G994 vendor ID!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;

         break;
      }

      hdlcOctet = (nData >> 8) & 0xFF;
      DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.G994VendorID[2*i + 1], hdlcOctet);
      hdlcOctet = nData & 0xFF;
      DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.G994VendorID[2*i], hdlcOctet);
   }

   /* Fill HDLC command*/
   hdlcCmd[0] = 0x0143;
   hdlcCmd[1] = 0x0;

   /* Lock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_TRUE);

   if (DSL_DRV_DANUBE_HdlcWrite (pContext, DSL_FALSE, (DSL_uint8_t *) hdlcCmd, 4)
      == DSL_SUCCESS)
   {
      DSL_WAIT (1);

      if (DSL_DRV_DANUBE_HdlcRead (pContext, DSL_FALSE,
             (DSL_uint8_t *)&hdlcRxBuffer, 32 * 2, &nHdlcRxLen) == DSL_SUCCESS)
      {
         if (nHdlcRxLen <= 0)
         {
            DSL_DEBUG( DSL_DBG_WRN,
               (pContext, "DSL[%02d]: WARNING - HDLC read invalid RxLen!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
         }
         else
         {
            /* Get System Vendor ID information*/
            for (i = 0; i < DSL_G997_LI_MAXLEN_VENDOR_ID / 2; i++)
            {
               hdlcOctet = hdlcRxBuffer[i + 1] & 0xFF;
               DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.SystemVendorID[2*i], hdlcOctet);
               hdlcOctet = (hdlcRxBuffer[i + 1] >> 8) & 0xFF;
               DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.SystemVendorID[2*i + 1], hdlcOctet);
            }

            /* Get Version Number information*/
            for (i = 0; i < DSL_G997_LI_MAXLEN_VERSION / 2; i++)
            {
               hdlcOctet = hdlcRxBuffer[i + 5] & 0xFF;
               DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.VersionNumber[2*i], hdlcOctet);
               hdlcOctet = (hdlcRxBuffer[i + 5] >> 8) & 0xFF;
               DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.VersionNumber[2*i + 1], hdlcOctet);
            }

            /* Get Serial Number information*/
            for (i = 0; i < DSL_G997_LI_MAXLEN_SERIAL / 2; i++)
            {
               hdlcOctet = hdlcRxBuffer[i + 13] & 0xFF;
               DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.SerialNumber[2*i], hdlcOctet);
               hdlcOctet = (hdlcRxBuffer[i + 13] >> 8) & 0xFF;
               DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.SerialNumber[2*i + 1], hdlcOctet);
            }
         }
      }
      else
      {
         DSL_DEBUG( DSL_DBG_WRN,
            (pContext, "DSL[%02d]: WARNING - HDLC Read failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
      }
   }
   else
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - HDLC Send failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   /* Unlock HDLC handling*/
   DSL_DRV_DANUBE_HdlcMutexControl(pContext, DSL_FALSE);

   /* Check for incomplete return values*/
   if (nErrCode == DSL_WRN_INCOMPLETE_RETURN_VALUES)
   {
      /* Set FE line inventory incomplete flag*/
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFeLineInventoryIncomplete, DSL_TRUE);
   }

   /* Get 5 SPAR octets*/
   if (DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_XTSE_FE,
                 0, sizeof(spar)/sizeof(DSL_uint16_t), (DSL_uint16_t*)spar) != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get FE SPAR information!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }
   else
   {
      /*
         Map SPAR1 octet to the XTSE capabilities\
      */
      /* G992.1 Annex A*/
      xtse[1-1] |= (spar[1] & 0x1) ? XTSE_1_03_A_1_NO : 0x0;
      /* G992.1 Annex B*/
      xtse[1-1] |= (spar[1] & 0x2) ? XTSE_1_05_B_1_NO : 0x0;
      /* G992.1 Annex C*/
      xtse[1-1] |= (spar[1] & 0x4) ? XTSE_1_02_C_TS_101388 : 0x0;
      /* G992.2 Annex A*/
      xtse[2-1] |= (spar[1] & 0x8) ? XTSE_2_01_A_2_NO : 0x0;

      /*
         Map SPAR2 octet to the XTSE capabilities
      */
      /* No ADSL info*/

      /*
         Map SPAR3 octet to the XTSE capabilities
      */
      /* G992.3 Annex A*/
      xtse[3-1] |= (spar[3] & 0x1) ? XTSE_3_03_A_3_NO : 0x0;
      /* G992.3 Annex L*/
      xtse[5-1] |= (spar[3] & 0x1) ? (XTSE_5_03_L_3_NO | XTSE_5_04_L_3_NO) : 0x0;
      /* G992.3 Annex B*/
      xtse[3-1] |= (spar[3] & 0x2) ? XTSE_3_05_B_3_NO : 0x0;
      /* G992.3 Annex I*/
      xtse[4-1] |= (spar[3] & 0x4) ? XTSE_4_05_I_3_NO : 0x0;
      /* G992.3 Annex J*/
      xtse[4-1] |= (spar[3] & 0x8) ? XTSE_4_07_J_3_NO : 0x0;
      /* G992.4 Annex A*/
      xtse[4-1] |= (spar[3] & 0x10) ? XTSE_4_01_A_4_NO : 0x0;

      /*
         Map SPAR4 octet to the XTSE capabilities
      */
      /* G992.5 Annex A*/
      xtse[6-1] |= (spar[2] & 0x1) ? XTSE_6_01_A_5_NO : 0x0;
      /* G992.5 Annex B*/
      xtse[6-1] |= (spar[2] & 0x2) ? XTSE_6_03_B_5_NO : 0x0;
      /* G992.5 Annex I*/
      xtse[6-1] |= (spar[2] & 0x4) ? XTSE_6_07_I_5_NO : 0x0;
      /* G992.3 Annex M*/
      xtse[5-1] |= (spar[2] & 0x8) ? XTSE_5_07_M_3_NO : 0x0;
      /* G992.5 Annex J*/
      xtse[7-1] |= (spar[2] & 0x10) ? XTSE_7_01_J_5_NO : 0x0;

      /*
         Map SPAR5 octet to the XTSE capabilities
      */
      /* G992.5 Annex M*/
      xtse[7-1] |= (spar[5] & 0x1) ? XTSE_7_03_M_5_NO : 0x0;

      /* Set FE XTSE capabilities in the DSL CPE Context*/
      DSL_CTX_WRITE(pContext, nErrCode, lineInventoryFe.XTSECapabilities, xtse);
   }

   /* Get Self Test Result information*/
   if (DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_INFO,
      DSL_CMV_ADDRESS_INFO_SELFTEST, 0, 2,
      (DSL_uint16_t*)&pContext->lineInventoryFe.SelfTestResult) != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't get self test result!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_DANUBE_LineInventoryFeRead, "
      "retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

DSL_Error_t DSL_DRV_DEV_TestModeControlSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_TestModeControl_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_TestModeControlSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_WRITE(pContext, nErrCode, nTestModeControl, pData->data.nTestMode);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_TestModeControlSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return (nErrCode);
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_DRV_DEV_ResourceUsageStatisticsGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_ResourceUsageStatisticsData_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

#ifndef DSL_DEBUG_DISABLE
   pData->staticMemUsage  =  sizeof(g_DANUBE_MsgDumpBlacklist);
#else
   pData->staticMemUsage  =  0;
#endif /* DSL_DEBUG_DISABLE*/
   pData->dynamicMemUsage = 0;

   return nErrCode;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

DSL_Error_t DSL_DRV_DEV_Annex_M_J_UsBandBordersStatusGet(
   DSL_Context_t *pContext,
   DSL_Band_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_uint8_t XTSE[DSL_G997_NUM_XTSE_OCTETS] = {0};
   DSL_DEV_VersionCheck_t nFwVerAnnexA = DSL_VERSION_ERROR,
                          nFwVerAnnexB = DSL_VERSION_ERROR;
   DSL_uint16_t AnnexMode = 0;

   /*Get current xTSE octets*/
   DSL_CTX_READ(pContext, nErrCode, xtseCurr, XTSE);

   /* Get Line State*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   if ((XTSE[5-1] & XTSE_5_07_M_3_NO) || (XTSE[7-1] & XTSE_7_03_M_5_NO))
   {
      pData->nFirstToneIndex = 6;
      pData->nLastToneIndex  = 63;
   }
   else if ((XTSE[4-1] & XTSE_4_07_J_3_NO) || (XTSE[7-1] & XTSE_7_01_J_5_NO))
   {
      pData->nFirstToneIndex = 1;
      pData->nLastToneIndex  = 63;
   }
   else
   {
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   /* Only proceed if the specified line is in SHOWTIME state.*/
   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      /* Return Fixed Band Border values*/
      return nErrCode;
   }


   /* Annex A FW version check*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(
                 pContext, DSL_MIN_ANNEX_A_FW_VERSION_STAT24, DSL_FW_ANNEX_A, &nFwVerAnnexA);
   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Annex B FW version check*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(
                 pContext, DSL_MIN_ANNEX_B_FW_VERSION_STAT24, DSL_FW_ANNEX_B, &nFwVerAnnexB);
   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

   if ((nFwVerAnnexA < DSL_VERSION_EQUAL) && (nFwVerAnnexB < DSL_VERSION_EQUAL))
   {
      /* Return Fixed Band Border values*/
      return nErrCode;
   }

   /* Get Annex M/J mask information */
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
      DSL_CMV_ADDRESS_STAT_ANNEX_M_J_MODE, 0, 1, &AnnexMode);

   if (nErrCode != DSL_SUCCESS)
      return nErrCode;

   if (AnnexMode > 8)
      return nErrCode;

   /* Mask:
      M(n+1)=1(6)..(32+n*4); with n=0..7;
      M(n+1)=1(6)..(32+n*4-1); with n=8; */
   pData->nLastToneIndex  = 32 + AnnexMode * 4 - (AnnexMode == 8 ? 1 : 0);

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BAND_BORDER_STATUS_GET
*/
DSL_Error_t DSL_DRV_DEV_BandBorderStatusGet(
   DSL_Context_t *pContext,
   DSL_BandBorderStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_G997_BitAllocationNsc_t BAT;
   DSL_uint16_t nToneIdx = 0;
   DSL_AccessDir_t nDirection;
   DSL_Band_t bandLimits[DSL_ACCESSDIR_LAST];
   DSL_Band_t bandBorders[DSL_ACCESSDIR_LAST];

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_BandBorderStatusGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* For the ADSL we have only one Band*/

   /* Get Line State*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   /* Only proceed if the specified line is in SHOWTIME state.*/
   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      DSL_DEBUG(DSL_DBG_ERR,(pContext,
         "DSL[%02d]: WARNING - Function is only available if line is in showtime state!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   for (nDirection = DSL_UPSTREAM; nDirection < DSL_ACCESSDIR_LAST; nDirection++)
   {
      /* Get Band Limits */
      nErrCode =  DSL_DRV_AdslBandLimitsGet(pContext, nDirection, &bandLimits[nDirection]);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Band limits %s get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode;
      }

      /* Get Band Borders*/
      memset(&BAT, 0x0, sizeof(DSL_G997_BitAllocationNsc_t));
      BAT.nDirection = nDirection;

      /* Get Bit Allocation Table*/
      nErrCode = DSL_DRV_DEV_G997_BitAllocationNSCGet(pContext, &BAT);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - BAT %s get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode;
      }


      /* Find Band Border first tone index*/
      for (nToneIdx = 0; (nToneIdx < BAT.data.bitAllocationNsc.nNumData) &&
                         (nToneIdx < DSL_MAX_NSC); nToneIdx++)
      {
         if (BAT.data.bitAllocationNsc.nNSCData[nToneIdx] != 0)
         {
            bandBorders[nDirection].nFirstToneIndex = nToneIdx;
            break;
         }
      }

      /* Find Band Border last tone index. SC with index 0 is a DC component which is not
        used for the transmission*/
      for (nToneIdx = (BAT.data.bitAllocationNsc.nNumData - 1); (nToneIdx > 0) &&
          (nToneIdx < DSL_MAX_NSC); nToneIdx--)
      {
         if (BAT.data.bitAllocationNsc.nNSCData[nToneIdx] != 0)
         {
            bandBorders[nDirection].nLastToneIndex = nToneIdx;
            break;
         }
      }
   }


   /* Check if Borders from BAT fits to limits from standard*/
   if ((bandBorders[pData->nDirection].nFirstToneIndex < bandLimits[pData->nDirection].nFirstToneIndex) ||
       (bandBorders[pData->nDirection].nLastToneIndex  > bandLimits[pData->nDirection].nLastToneIndex))
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - out of limits Band Border detected!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nErrCode));
      nErrCode = DSL_WRN_FW_BB_STANDARD_VIOLATION;
   }

   /* Special check to cover issue issue with Split point configuration*/
   if (bandLimits[DSL_UPSTREAM].nLastToneIndex >= bandBorders[DSL_DOWNSTREAM].nFirstToneIndex)
   {
      bandLimits[DSL_UPSTREAM].nLastToneIndex = bandBorders[DSL_UPSTREAM].nLastToneIndex;
      bandLimits[DSL_DOWNSTREAM].nFirstToneIndex = bandBorders[DSL_DOWNSTREAM].nFirstToneIndex;

      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Band split point issue found!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nErrCode));

      nErrCode = DSL_WRN_FW_BB_STANDARD_VIOLATION;
   }

   /* Only one band*/
   pData->data.nNumData = 1;

   pData->data.nBandLimits[0].nFirstToneIndex = bandLimits[pData->nDirection].nFirstToneIndex;
   pData->data.nBandLimits[0].nLastToneIndex  = bandLimits[pData->nDirection].nLastToneIndex;

   pData->data.nBandBorder[0].nFirstToneIndex = bandBorders[pData->nDirection].nFirstToneIndex;
   pData->data.nBandBorder[0].nLastToneIndex  = bandBorders[pData->nDirection].nLastToneIndex;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_BandBorderStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return (nErrCode);
}

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   Get Minimum INP. ADSL2/2+ only.
*/
static DSL_Error_t DSL_DRV_DANUBE_MinimumInpGet(
   DSL_Context_t *pContext,
   DSL_LatencyPath_t lPath,
   DSL_int16_t *pMinInp)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nData = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   if (lPath == DSL_LATENCY_IP_LP0 || lPath == DSL_LATENCY_FP_LP1)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_GHS_PARAMS,
                    lPath == DSL_LATENCY_IP_LP0 ? 20 : 21, 1, &nData);
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Latency Path not specified for Minimum INP!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;
   }

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Minimum INP get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      *pMinInp = (DSL_int16_t)nData;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   Get US/DS Line Rate. ADSL1 only.
*/
static DSL_Error_t DSL_DRV_DANUBE_LineRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pLineRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_G997_BitAllocationNsc_t BAT;
   DSL_uint16_t nToneIdx = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      memset(&BAT, 0x0, sizeof(DSL_G997_BitAllocationNsc_t));
      BAT.nDirection = nDirection;

      /* Get Bit Allocation Table*/
      nErrCode = DSL_DRV_DEV_G997_BitAllocationNSCGet(pContext, &BAT);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - BAT get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

      /* Sum bit allocation table*/
      for (nToneIdx = 0; (nToneIdx < BAT.data.bitAllocationNsc.nNumData) &&
                         (nToneIdx < DSL_MAX_NSC); nToneIdx++)
      {
         *pLineRate += (DSL_uint32_t)BAT.data.bitAllocationNsc.nNSCData[nToneIdx];
      }
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

/*
   Get C-RATES2 Option according to selected direction.
   Return values: 1, 2, 3, 4
   ADSL1 mode only.

   Note: No internal check for the ADSL1 mode
*/
static DSL_Error_t DSL_DRV_DANUBE_CRATES2OptionGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint8_t *pOption)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nData = 0;
   DSL_uint8_t nOption = 0;

   DSL_CHECK_POINTER(pContext, pOption);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Get C-RATES2 content*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_CRATES2, 0, 1, &nData);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - C-RATES2 get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* Get C-RATES2 option according to the selected direction*/
   nData &= 0xFF; /* C-RATES2 is in the lower byte*/
   nOption = nDirection == DSL_DOWNSTREAM ? (nData >> 4) & 0xF : nData & 0xF;

   /* Decode bit pattern*/
   switch (nOption)
   {
   case 0x1:
   case 0x2:
      *pOption = nOption;
      break;
   case 0x4:
      *pOption = 3;
      break;
   case 0x8:
      *pOption = 4;
      break;
   default:
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext,
         "DSL[%02d]: ERROR - unknown bit pattern in the %s field of the C-RATES2!"
         DSL_DRV_CRLF,DSL_DEV_NUM(pContext),
         nDirection == DSL_DOWNSTREAM ? "DS" : "US"));
      nErrCode = DSL_ERROR;
      break;
   }

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   Get US/DS Net Rate. ADSL1 only.
*/
static DSL_Error_t DSL_DRV_DANUBE_NetRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pNetRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t i = 0;
   DSL_uint8_t nOption = 0;
   DSL_uint16_t nBi, nBf;
   DSL_uint32_t sumBiBf = 0;
   DSL_uint16_t startIdx = 0, stopIdx = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get C-RATES-2 direction specific option*/
      nErrCode = DSL_DRV_DANUBE_CRATES2OptionGet(pContext, nDirection, &nOption);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - C-RATES2 get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      startIdx = nDirection == DSL_UPSTREAM ? 7 : 0;
      stopIdx  = nDirection == DSL_UPSTREAM ? 9 : 6;

      /* Net Rate = sum(Bi + Bf)*8*/
      /*
      CMV specification is wrong for the C-RATES-RA format.
      Each 16-bit offset contains 1 byte from the C-RATES-RA.
      */
      for (i = startIdx; i <= stopIdx; i++)
      {
         /* Get Bf*/
         nErrCode = DSL_DRV_DANUBE_CmvRead(
                       pContext, DSL_CMV_GROUP_INFO,
                       DSL_CMV_ADDRESS_INFO_CRATESRA,
                       (DSL_uint16_t)((nOption - 1) * 30 + i), 1, &nBf);
         /* Get Bi*/
         nErrCode = DSL_DRV_DANUBE_CmvRead(
                       pContext, DSL_CMV_GROUP_INFO,
                       DSL_CMV_ADDRESS_INFO_CRATESRA,
                       (DSL_uint16_t)((nOption - 1) * 30 + 10 + i), 1, &nBi);

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext,"DSL[%02d]: ERROR - C-RATES-RA get failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            return nErrCode ;
         }

         sumBiBf += (DSL_uint16_t)((nBi & 0xFF) + (nBf & 0xFF));
      }

      *pNetRate = sumBiBf * 8;
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

/*
   Get Trellis Overhead.
*/
static DSL_Error_t DSL_DRV_DANUBE_TrellisOverheadGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pTrellisOverhead)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_BandBorderStatus_t nBandBorderStatus;
   DSL_G997_BitAllocationNsc_t BAT;
   DSL_uint16_t nToneIdx = 0;
   DSL_uint16_t nLoadedTonesNum = 0, nOneBitTonesNum = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   memset(&nBandBorderStatus, 0x0, sizeof(DSL_BandBorderStatus_t));
   nBandBorderStatus.nDirection = nDirection;
   /* Get Band Border status*/
   nErrCode = DSL_DRV_DEV_BandBorderStatusGet(pContext, &nBandBorderStatus);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext,"DSL[%02d]: ERROR - %s Band Border status get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
      return nErrCode ;
   }

   /* Get the number of loaded tones*/
   nLoadedTonesNum = (nBandBorderStatus.data.nBandBorder[0].nLastToneIndex -
                      nBandBorderStatus.data.nBandBorder[0].nFirstToneIndex) + 1;

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Just an approximation equation which fits with a high probability*/
      *pTrellisOverhead = ((nLoadedTonesNum + 1) / 2) + 4;
   }
   else
   {

      memset(&BAT, 0x0, sizeof(DSL_G997_BitAllocationNsc_t));
      BAT.nDirection = nDirection;

      /* Get Bit Allocation Table*/
      nErrCode = DSL_DRV_DEV_G997_BitAllocationNSCGet(pContext, &BAT);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - BAT get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

      /* Find Band Border first tone index*/
      for (nToneIdx = 0; (nToneIdx < BAT.data.bitAllocationNsc.nNumData) &&
                         (nToneIdx < DSL_MAX_NSC); nToneIdx++)
      {
         if (BAT.data.bitAllocationNsc.nNSCData[nToneIdx] == 0x1)
         {
            nOneBitTonesNum++;
         }
      }

      *pTrellisOverhead = ((((nLoadedTonesNum - (nOneBitTonesNum / 2)) + 1) / 2) + 4);
   }

   return nErrCode;
}

/*
   Get Atainable Line Data Rate.
*/
static DSL_Error_t DSL_DRV_DANUBE_AttLineDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pAttDataRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nData[2] = {0};
   DSL_uint32_t nAttTotalDataRate = 0, nTrellisOverhead = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    nDirection == DSL_UPSTREAM ?
                    DSL_CMV_ADDRESS_INFO_LINE_STATUS_US :
                    DSL_CMV_ADDRESS_INFO_LINE_STATUS_DS, 4, 2, nData);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - INFO 69 get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      *pAttDataRate = ((nData[1] << 16) | nData[0]) / 4000;
   }
   else
   {
      /* Get AttTotalDataRate*/
      nErrCode = DSL_DRV_DANUBE_AttTotalDataRateGet(
                    pContext, nDirection, &nAttTotalDataRate);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Atteinable Total Data Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      /* Get Trellis Overhead*/
      nErrCode = DSL_DRV_DANUBE_TrellisOverheadGet(
                    pContext, nDirection, &nTrellisOverhead);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Trellis Overhead get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      *pAttDataRate = nAttTotalDataRate + nTrellisOverhead;
   }

   return nErrCode;
}

/*
   Get US Atainable Total Data Rate.
*/
static DSL_Error_t DSL_DRV_DANUBE_AttTotalDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pAttTotalDataRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nData32 = 0;
   DSL_uint32_t nTrellisOverhead = 0;
   DSL_G997_FramingParameterStatusData_t g997framingParameters;
   DSL_uint16_t sumRp = 0;
   DSL_uint8_t nChannel = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get Attainable Line Data Rate*/
      nErrCode = DSL_DRV_DANUBE_AttLineDataRateGet(pContext, nDirection, &nData32);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Attainable Line Data Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      /* Get Trellis Overhead*/
      nErrCode = DSL_DRV_DANUBE_TrellisOverheadGet(pContext, nDirection, &nTrellisOverhead);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Trellis Overhead get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      /* Just an approximation equation which fits with a high probability*/
      *pAttTotalDataRate = nData32 - nTrellisOverhead;
   }
   else
   {
      /* Get AttAggregateDataRate*/
      nErrCode = DSL_DRV_DANUBE_AttAggregateDataRateGet(pContext, nDirection, &nData32);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Atteinable Aggregate Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      for (nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++)
      {
         nErrCode = DSL_DRV_DANUBE_G997_FramingParameterStatusGet(
                       pContext, nDirection, nChannel, &g997framingParameters);

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext,"DSL[%02d]: ERROR - %s G997 Framing Parameters Get Failed (nChannel=%d)!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US", nChannel));
            return nErrCode ;
         }

         sumRp += g997framingParameters.nRFEC;
      }

      /* AttTotalDataRate = AttAggregateDataRate + RS_overhead*/
      *pAttTotalDataRate = nData32 + (sumRp * 8);
   }

   return nErrCode;
}

/*
   Get RS Overhead Rate
*/
static DSL_Error_t DSL_DRV_DANUBE_RsOverheadRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pRsOverhead)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint8_t nOption = 0;
   DSL_uint8_t nOffset = 0;
   DSL_uint16_t RSf = 0, RSi = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get C-RATES-2 direction specific option*/
      nErrCode = DSL_DRV_DANUBE_CRATES2OptionGet(pContext, nDirection, &nOption);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - C-RATES2 get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      /*
      CMV specification is wrong for the C-RATES-RA format.
      Each 16-bit offset contains 1 byte from the C-RATES-RA.
      */

      /* Get Upstream RSf*/
      nOffset = nDirection == DSL_UPSTREAM ? 25 : 20;
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_CRATESRA,
                    (DSL_uint16_t)((nOption - 1) * 30 + nOffset), 1, &RSf);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - RSf get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      RSf &= 0x3F;

      /* Get Upstream RSi*/
      nOffset = nDirection == DSL_UPSTREAM ? 26 : 21;
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_CRATESRA,
                    (DSL_uint16_t)((nOption - 1) * 30 + nOffset), 1, &RSi);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - RSi get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode ;
      }

      RSi &= 0x3F;

      /* Get RS Overhead*/
      *pRsOverhead = (RSf + RSi)*8;
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;
}

/*
   Get US Atteinable Aggregate Data Rate.
*/
DSL_Error_t DSL_DRV_DANUBE_AttAggregateDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pAttAggregateDataRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nAttTotalDataRate = 0;
   DSL_uint32_t RS_Overhead = 0;
#if defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)
   DSL_G997_LineStatus_t lineSatusData;
   DSL_FramingParameterStatus_t framingParameters;
   DSL_G997_FramingParameterStatusData_t g997framingParameters;
   DSL_uint8_t nChannel = 0;
   DSL_uint16_t sumMp = 0, sumTp = 0, sumBp = 0, sumRp = 0, sumLp = 0;
   DSL_uint32_t nOverhead_rate = 0, nOverhead_den = 0;
#endif /* defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)*/

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {

      /* Get Atainable Total Data Rate.*/
      nErrCode = DSL_DRV_DANUBE_AttTotalDataRateGet(pContext, nDirection, &nAttTotalDataRate);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Atainable Total Data Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US"));
         return nErrCode ;
      }


      /* Get RS Overhead*/
      nErrCode = DSL_DRV_DANUBE_RsOverheadRateGet(pContext, nDirection, &RS_Overhead);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s RS Overhead Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US"));
         return nErrCode ;
      }

      /* AttAggregateDataRate = nAttTotalDataRate - RS_Overhead */
      *pAttAggregateDataRate = nAttTotalDataRate - RS_Overhead;
   }
   else
   {
#if defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)
      memset(&lineSatusData, 0x0, sizeof(DSL_G997_LineStatus_t));

      lineSatusData.nDirection = nDirection;
      lineSatusData.nDeltDataType = DSL_DELT_DATA_SHOWTIME;

      /* Get Line Status*/
      nErrCode = DSL_DRV_DANUBE_G997_LineStatusGet(pContext, &lineSatusData);

      if (nErrCode < DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s G997 Line Status Get Failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US"));
         return nErrCode ;
      }

      for (nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++)
      {
         framingParameters.nChannel = nChannel;
         framingParameters.nDirection = nDirection;

         nErrCode = DSL_DRV_DANUBE_FramingParameterStatusGet(
                       pContext, &framingParameters);

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext,"DSL[%02d]: ERROR - %s Framing Parameters Get Failed (nChannel=%d)!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US", nChannel));
            return nErrCode ;
         }

         sumMp += framingParameters.data.nMp;
         sumTp += framingParameters.data.nTp;
         sumBp += framingParameters.data.nBP;

         nErrCode = DSL_DRV_DANUBE_G997_FramingParameterStatusGet(
                       pContext, nDirection, nChannel, &g997framingParameters);

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext,"DSL[%02d]: ERROR - %s G997 Framing Parameters Get Failed (nChannel=%d)!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US", nChannel));
            return nErrCode ;
         }

         sumRp += g997framingParameters.nRFEC;
         sumLp += g997framingParameters.nLSYMB;
      }

      /* AttAggregateDataRate = nAttDrNet + Overhead_rate */
      nOverhead_den  = (DSL_uint32_t)(sumTp * ((sumBp + 1) * sumMp + sumRp));
      nOverhead_rate = (DSL_uint32_t)(nOverhead_den ? (sumMp * sumLp) / nOverhead_den : 0);

      *pAttAggregateDataRate = (DSL_uint32_t)((lineSatusData.data.ATTNDR/4000) + nOverhead_rate);
#else
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
#endif /* defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)*/
   }

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   Get US/DS Nominal PSD.
   The reported value is expressed in dBm/Hz and ranges from 0 dBm/Hz
   (coded as 0) to -97.5 dBm/Hz (coded as -975) within steps of 0.1 dBm/Hz.
   ADSL2/2+ only.
*/
static DSL_Error_t DSL_DRV_DANUBE_NominalPsdGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_int16_t *pNomPsd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nData = 0;

   DSL_CHECK_POINTER(pContext, pNomPsd);
   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      return DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   /* FW value seems to be in the (dBm/Hz) * 10 format even if this is
      not mentioned in the CMV description*/
   nErrCode = DSL_DRV_DANUBE_CmvRead(
                 pContext, DSL_CMV_GROUP_INFO,
                 DSL_CMV_ADDRESS_INFO_PMD,
                 nDirection == DSL_DOWNSTREAM ? 15 : 14, 1, &nData);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Nominal PSD %s get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US"));
   }
   else
   {
      *pNomPsd = (DSL_int16_t)nData;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

/*
   Get Dual Latency Status
*/
DSL_Error_t DSL_DRV_DANUBE_DualLatencyStatusGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_boolean_t *pDualLatencyActive)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint16_t nData[2] = {0}, nPath = 0;

   DSL_CHECK_POINTER(pContext, pDualLatencyActive);
   DSL_CHECK_ERR_CODE();

   for (nPath = 0; nPath < 2; nPath++)
   {
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_STAT,
                    nDirection == DSL_DOWNSTREAM ?
                    DSL_CMV_ADDRESS_STAT_PATH_DS : DSL_CMV_ADDRESS_STAT_PATH_US,
                    nPath, 1, &(nData[nPath]));

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Active %s Path staus get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_DOWNSTREAM ? "DS" : "US"));
         break;
      }
   }

   if (nErrCode == DSL_SUCCESS)
   {
      *pDualLatencyActive = (nData[0] && nData[1]) ? DSL_TRUE : DSL_FALSE;
   }

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   Get DS Attainable Data Rate Estimate coding gain that is assumed in bitloading.
*/
static DSL_Error_t DSL_DRV_DANUBE_AttndrCodingGainDsGet(
   DSL_Context_t *pContext,
   DSL_uint16_t *pCodingGain)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nData = 0;

   DSL_CHECK_POINTER(pContext, pCodingGain);
   DSL_CHECK_ERR_CODE();

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get Coding Gain*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_RMSGRA, 4, 1, &nData);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Coding Gain get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
      else
      {
         /* value in CMV is in units of 0.5dB => convert it to units of 0.1dB*/
         *pCodingGain = nData * 5;
      }
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   DS Attainable Data Rate Estimate Max number of bits per symbol including
   overhead.
*/
static DSL_Error_t DSL_DRV_DANUBE_AttndrMaxBpsDsGet(
   DSL_Context_t *pContext,
   DSL_uint16_t *pBps)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint16_t nData = 0;

   DSL_CHECK_POINTER(pContext, pBps);
   DSL_CHECK_ERR_CODE();

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get the number of bits per symbol*/
      nErrCode = DSL_DRV_DANUBE_CmvRead(
                    pContext, DSL_CMV_GROUP_INFO,
                    DSL_CMV_ADDRESS_INFO_RMSGRA, 7, 1, &nData);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Coding Gain get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
      else
      {
         *pBps = nData;
      }
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;

}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   Get Aggregate Data Rate. ADSL1 only.
*/
static DSL_Error_t DSL_DRV_DANUBE_AggregateDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pAggregateDataRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nNetRate = 0;
   DSL_boolean_t bDualLatencyActive = DSL_FALSE;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get Net Rate. Rate Net = sum(Bi+Bf)*8*/
      nErrCode = DSL_DRV_DANUBE_NetRateGet(pContext, nDirection, &nNetRate);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Net Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode;
      }

      /* Check if both fast and interleaved paths are active*/
      nErrCode = DSL_DRV_DANUBE_DualLatencyStatusGet(
                 pContext, nDirection, &bDualLatencyActive);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Dual Latency Status get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode;
      }

      /* Aggregate Rate = sum(Ki + Kf)*8; Ki = Bi + 1; Kf = Bf + 1;
         Net Rate = sum(Bi+Bf)*8;
         Aggregate Rate = Net Rate + Frame Overhead Rate*/
      *pAggregateDataRate = (DSL_uint32_t)(nNetRate + (bDualLatencyActive ? 2 : 1) * 8);
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;
}

/*
   Get Total Data Rate. ADSL1 only.
*/
static DSL_Error_t DSL_DRV_DANUBE_TotalDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_uint32_t *pTotalDataRate)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
   DSL_uint32_t nAggregateDataRate = 0, nRsOverhead = 0;

   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   /*Check for the ADSL1 modes*/
   if (bAdsl1)
   {
      /* Get Aggregate Rate*/
      nErrCode =  DSL_DRV_DANUBE_AggregateDataRateGet(pContext, nDirection, &nAggregateDataRate);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s Aggregate Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      /* Get RS Overhead Rate*/
      nErrCode = DSL_DRV_DANUBE_RsOverheadRateGet(pContext, nDirection, &nRsOverhead);

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - %s RS Overhead Rate get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_UPSTREAM ? "US" : "DS"));
         return nErrCode ;
      }

      *pTotalDataRate = (DSL_uint32_t)(nAggregateDataRate + nRsOverhead);
   }
   else
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
   }

   return nErrCode;
}

static DSL_Error_t DSL_DRV_DANUBE_Annex_M_J_UsMaskGet(
   DSL_Context_t *pContext,
   DSL_MLS_MaskAnnexMJ_Us_t *pMask)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t XTSE[DSL_G997_NUM_XTSE_OCTETS] = {0};
   DSL_DEV_VersionCheck_t nFwVerAnnexA = DSL_VERSION_ERROR,
                          nFwVerAnnexB = DSL_VERSION_ERROR;
   DSL_uint16_t AnnexMode = 0;

   *pMask = DSL_MASK_ANNEXMJ_US_NA;

   /*Get current xTSE octets*/
   DSL_CTX_READ(pContext, nErrCode, xtseCurr, XTSE);

   if ( !((XTSE[5-1] & XTSE_5_07_M_3_NO) || (XTSE[7-1] & XTSE_7_03_M_5_NO) ||
          (XTSE[4-1] & XTSE_4_07_J_3_NO) || (XTSE[7-1] & XTSE_7_01_J_5_NO)))
   {
      return nErrCode;
   }

   /* Annex A FW version check*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(
                 pContext, DSL_MIN_ANNEX_A_FW_VERSION_STAT24, DSL_FW_ANNEX_A, &nFwVerAnnexA);
   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

   /* Annex B FW version check*/
   nErrCode = DSL_DRV_DANUBE_FirmwareVersionCheck(
                 pContext, DSL_MIN_ANNEX_B_FW_VERSION_STAT24, DSL_FW_ANNEX_B, &nFwVerAnnexB);
   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

   if ((nFwVerAnnexA < DSL_VERSION_EQUAL) && (nFwVerAnnexB < DSL_VERSION_EQUAL))
   {
      return nErrCode;
   }

   /* Get Annex M/J mask information */
   nErrCode = DSL_DRV_DANUBE_CmvRead(pContext, DSL_CMV_GROUP_STAT,
      DSL_CMV_ADDRESS_STAT_ANNEX_M_J_MODE, 0, 1, &AnnexMode);

   if (nErrCode != DSL_SUCCESS)
      return nErrCode;

   if (AnnexMode > 8)
      return nErrCode;

   *pMask = (DSL_MLS_MaskAnnexMJ_Us_t)(AnnexMode + 1);

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_MISC_LINE_STATUS_GET
*/
DSL_Error_t DSL_DRV_DEV_MiscLineStatusGet(
   DSL_Context_t *pContext,
   DSL_MiscLineStatus_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_MiscLineStatusGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get Line State*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);

   /* Only proceed if the specified line is in SHOWTIME state.*/
   if (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      DSL_DEBUG(DSL_DBG_ERR,(pContext,
         "DSL[%02d]: WARNING - Function is only available if line is in showtime state!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
   }

   /* Request selected Misc Line Status parameter*/
   switch(pData->data.nStatusSelector)
   {
   case DSL_MLS_ATTNDR_CODING_GAIN_DS:
      {
         DSL_uint16_t nCodingGain = 0;

         /* Get Coding Gain*/
         nErrCode = DSL_DRV_DANUBE_AttndrCodingGainDsGet(pContext, &nCodingGain);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nCodingGain;
         }
      }
      break;

   case DSL_MLS_ATTNDR_MAX_BITS_PER_SYMBOL_DS:
      {
         DSL_uint16_t nBps = 0;

         /* Get BPS*/
         nErrCode = DSL_DRV_DANUBE_AttndrMaxBpsDsGet(pContext, &nBps);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nBps;
         }
      }
      break;

   case DSL_MLS_DUAL_LATENCY_ON_US:
   case DSL_MLS_DUAL_LATENCY_ON_DS:
      {
         DSL_boolean_t bDualLatencyActive = DSL_FALSE;

         /* Get Dual Latency Status*/
         nErrCode = DSL_DRV_DANUBE_DualLatencyStatusGet(
                       pContext,
                       pData->data.nStatusSelector == DSL_MLS_DUAL_LATENCY_ON_US ?
                       DSL_UPSTREAM : DSL_DOWNSTREAM, &bDualLatencyActive);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)bDualLatencyActive;
         }
      }
      break;

   case DSL_MLS_INIT_SNR_DS:
      {
         DSL_int16_t nInitSnrMarginDs = 0;

         /* Update Initial SNR margin in the device context*/
         DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.nInitSnrMarginDs, nInitSnrMarginDs);
         pData->data.nStatusValue = (DSL_int32_t)nInitSnrMarginDs;
      }
      break;

   case DSL_MLS_NOMPSD_US:
   case DSL_MLS_NOMPSD_DS:
      {
         DSL_int16_t nNomPsd = 0;

         /* Get Upstream Nominal PSD*/
         nErrCode = DSL_DRV_DANUBE_NominalPsdGet(
                    pContext, pData->data.nStatusSelector == DSL_MLS_NOMPSD_US ?
                    DSL_UPSTREAM : DSL_DOWNSTREAM, &nNomPsd);
         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nNomPsd;
         }
      }
      break;

   case DSL_MLS_MIN_INP_IP_LP0_DS:
   case DSL_MLS_MIN_INP_FP_LP1_DS:
      {
         DSL_int16_t nMinInp = 0;

         /* Get LP0 Minimum INP*/
         nErrCode = DSL_DRV_DANUBE_MinimumInpGet(
                       pContext, pData->data.nStatusSelector == DSL_MLS_MIN_INP_IP_LP0_DS ?
                       DSL_LATENCY_IP_LP0 : DSL_LATENCY_FP_LP1, &nMinInp);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nMinInp;
         }
      }
      break;

   case DSL_MLS_TOTAL_DATA_RATE_US:
   case DSL_MLS_TOTAL_DATA_RATE_DS:
      {
         DSL_uint32_t nTotalRate = 0;

         /* Get Total Rate*/
         nErrCode = DSL_DRV_DANUBE_TotalDataRateGet(
                       pContext, pData->data.nStatusSelector == DSL_MLS_TOTAL_DATA_RATE_US ?
                       DSL_UPSTREAM : DSL_DOWNSTREAM, &nTotalRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nTotalRate;
         }
      }
      break;

   case DSL_MLS_AGGREGATE_DATA_RATE_US:
   case DSL_MLS_AGGREGATE_DATA_RATE_DS:
      {
         DSL_uint32_t nAggregateRate = 0;

         /* Get Aggregate Rate*/
         nErrCode = DSL_DRV_DANUBE_AggregateDataRateGet(
                       pContext, pData->data.nStatusSelector == DSL_MLS_AGGREGATE_DATA_RATE_US ?
                       DSL_UPSTREAM : DSL_DOWNSTREAM, &nAggregateRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nAggregateRate;
         }
      }
      break;

   case DSL_MLS_LINE_RATE_US:
   case DSL_MLS_LINE_RATE_DS:
      {
         DSL_uint32_t nLineRate = 0;

         /* Get Line Rate*/
         nErrCode = DSL_DRV_DANUBE_LineRateGet(
                    pContext, pData->data.nStatusSelector == DSL_MLS_LINE_RATE_US ?
                    DSL_UPSTREAM : DSL_DOWNSTREAM, &nLineRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nLineRate;
         }
      }
      break;

   case DSL_MLS_NET_RATE_US:
   case DSL_MLS_NET_RATE_DS:
      {
         DSL_uint32_t nNetRate = 0;

         /* Get Net Rate*/
         nErrCode = DSL_DRV_DANUBE_NetRateGet(
                    pContext, pData->data.nStatusSelector == DSL_MLS_NET_RATE_US ?
                    DSL_UPSTREAM : DSL_DOWNSTREAM, &nNetRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nNetRate;
         }
      }
      break;

   case DSL_MLS_ATT_AGGREGATE_DATA_RATE_US:
   case DSL_MLS_ATT_AGGREGATE_DATA_RATE_DS:
      {
         DSL_uint32_t nAttAggregateDataRate = 0;

         /* Get US Atteinable Aggregate Data Rate*/
         nErrCode =  DSL_DRV_DANUBE_AttAggregateDataRateGet(
            pContext, pData->data.nStatusSelector == DSL_MLS_ATT_AGGREGATE_DATA_RATE_US ?
            DSL_UPSTREAM : DSL_DOWNSTREAM, &nAttAggregateDataRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nAttAggregateDataRate;
         }
      }
      break;

   case DSL_MLS_ATT_LINE_DATA_RATE_US:
   case DSL_MLS_ATT_LINE_DATA_RATE_DS:
      {
         DSL_uint32_t nAttLineDataRate = 0;

         /* Get Atteinable Line Data Rate*/
         nErrCode = DSL_DRV_DANUBE_AttLineDataRateGet(pContext,
            pData->data.nStatusSelector == DSL_MLS_ATT_LINE_DATA_RATE_US ?
            DSL_UPSTREAM : DSL_DOWNSTREAM, &nAttLineDataRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nAttLineDataRate;
         }
      }
      break;

   case DSL_MLS_ATT_TOTAL_DATA_RATE_US:
   case DSL_MLS_ATT_TOTAL_DATA_RATE_DS:
      {
         DSL_uint32_t nAttTotalDataRate = 0;

         /* Get Total Atteinable Data Rate*/
         nErrCode = DSL_DRV_DANUBE_AttTotalDataRateGet(pContext,
            pData->data.nStatusSelector == DSL_MLS_ATT_TOTAL_DATA_RATE_US ?
            DSL_UPSTREAM : DSL_DOWNSTREAM, &nAttTotalDataRate);

         if (nErrCode == DSL_SUCCESS)
         {
            pData->data.nStatusValue = (DSL_int32_t)nAttTotalDataRate;
         }
      }
      break;

   case DSL_MLS_MASK_ANNEX_M_J_US:
      nErrCode = DSL_DRV_DANUBE_Annex_M_J_UsMaskGet(pContext,
                  (DSL_MLS_MaskAnnexMJ_Us_t*)&(pData->data.nStatusValue));
      break;

   default:
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown Misc Line Status Selector (%d)!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pData->data.nStatusSelector));
      nErrCode = DSL_ERROR;
      break;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_MiscLineStatusGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return (nErrCode);
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

/* LED block */

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_device.h'
*/
DSL_Error_t DSL_DRV_DEV_LED_FirmwareInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_FwVersion_t FwVer = {DSL_FALSE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

   /* Configure GPIO[10] for OUTPUT */
   DSL_uint16_t nData=(1<<10);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_DEV_LED_FirmwareInit"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   /* Driver lock*/
   if(DSL_DRV_MUTEX_LOCK(pContext->bspMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock device mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   DSL_BSP_AdslLedInit(pContext->pDevCtx->lowHandle, DSL_LED_LINK_ID,
                            DSL_LED_LINK_TYPE, DSL_LED_HD_FW);

   /* Unlock driver*/
   DSL_DRV_MUTEX_UNLOCK(pContext->bspMutex);

   /* Get FW version*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.version.fwVersion, FwVer);

#if defined(INCLUDE_ADSL_LED)
   if ((FwVer.nMajorVersion == DSL_AMAZON_SE_FW_MAJOR_NUMBER) ||
       (FwVer.nMajorVersion == DSL_AR9_FW_MAJOR_NUMBER) ||
       (FwVer.nMajorVersion == DSL_DANUBE_FW_MAJOR_NUMBER))
   {
      /* Configure GPIO[9] for OUTPUT */
      nData |= (1<<9);
   }
#endif /* defined(INCLUDE_ADSL_LED)*/

   /*  Setup ADSL Link/Data LED */
   /*  setup ADSL GPIO as output mode */
   if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
      DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 0, 1, &nData) != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CMV send failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERR_MSG_EXCHANGE;
   }
   else
   {
      /*  setup ADSL GPIO mask */
      if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 2, 1, &nData) != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - CMV send failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_MSG_EXCHANGE;
      }
      else
      {
         /*  Let FW to handle ADSL Link LED */
         if (FwVer.nMajorVersion == DSL_DANUBE_FW_MAJOR_NUMBER)
         {
            nData=0x0A03;
         }
         else
         {
            nData=0x0901;
         }

         DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: MSG -  Link LED nData %#x"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nData));

         if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 4, 1, &nData) != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - CMV send failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            nErrCode = DSL_ERR_MSG_EXCHANGE;
         }
      }
   }

#if defined(INCLUDE_ADSL_LED)
   if ((FwVer.nMajorVersion == DSL_AMAZON_SE_FW_MAJOR_NUMBER) ||
       (FwVer.nMajorVersion == DSL_AR9_FW_MAJOR_NUMBER) ||
       (FwVer.nMajorVersion == DSL_DANUBE_FW_MAJOR_NUMBER))
   {
      if (nErrCode == DSL_SUCCESS)
      {
         /* OFF */
         /* use GPIO9 for TR68 data led off. */
         if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 5, 1, &off) != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "[%s %d]: CMV Write fail!" DSL_DRV_CRLF, __func__, __LINE__));
         }

         if (nErrCode == DSL_SUCCESS)
         {
            DSL_BSP_AdslLedInit(pContext->pDevCtx->lowHandle, DSL_LED_DATA_ID,
               DSL_LED_DATA_TYPE, DSL_LED_HD_FW);
         }
      }
   }
#endif /* defined(INCLUDE_ADSL_LED)*/

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_DEV_LED_FirmwareInit"
      ", retCode=%d"DSL_DRV_CRLF, nErrCode, DSL_DEV_NUM(pContext)));

   return nErrCode;
}
#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/
