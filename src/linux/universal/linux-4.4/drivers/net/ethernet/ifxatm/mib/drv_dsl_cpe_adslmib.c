/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#ifdef INCLUDE_DSL_ADSL_MIB

#include "drv_dsl_cpe_api_ioctl.h"
#include "drv_dsl_cpe_api_adslmib.h"
#include "drv_dsl_cpe_api_adslmib_ioctl.h"

#ifdef __cplusplus
   extern "C" {
#endif

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_MIB

typedef struct
{
   DSL_uint8_t adslLineAlarmConfProfileName[32];
   DSL_boolean_t adslAtucInitFailureTrapEnable;
   DSL_int_t adslLineAlarmConfProfileRowStatus;
} DSL_adslLineAlarmConfProfilePrivEntry_t;

static DSL_adslLineAlarmConfProfilePrivEntry_t alarmConfProfile = 
   {"No name", DSL_TRUE, 1 };

typedef struct
{
   DSL_uint8_t adslLineAlarmExtConfProfileName[32];
   DSL_int_t adslLineAlarmExtConfProfileRowStatus;
} DSL_adslLineAlarmExtConfProfilePrivEntry_t;

#ifdef INCLUDE_DSL_PM
static DSL_adslLineAlarmExtConfProfilePrivEntry_t alarmExtConfProfile = 
   {"No name", 1 };
#endif

static DSL_Error_t DSL_DRV_MIB_ADSL_IoctlHandleHelperCall
(
   DSL_Context_t *pContext,
   DSL_boolean_t bIsInKernel,
   DSL_IoctlHandlerHelperType_t nType,
   DSL_DRV_IoctlHandlerHelperFunc_t pFunc,
   DSL_void_t *pArg,
   DSL_uint32_t nArgSz
);

/** \addtogroup DRV_DSL_CPE_DEBUG
 @{ */
DSL_char_t* DSL_DBG_ADSL_IoctlName(DSL_uint_t nIoctlCode)
{
   switch (nIoctlCode)
   {
   case DSL_FIO_MIB_ADSL_LINE_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_LINE_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_SET:
      return "DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_SET";
   case DSL_FIO_MIB_ADSL_TRAPS_GET:
      return "DSL_FIO_MIB_ADSL_TRAPS_GET";
#ifdef INCLUDE_ADSL_MIB_RFC3440
   case DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_SET:
      return "DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_SET";
   case DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET:
      return "DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET";
   case DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_SET:
      return "DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_SET";
   case DSL_FIO_MIB_ADSL_EXT_TRAPS_GET:
      return "DSL_FIO_MIB_ADSL_EXT_TRAPS_GET";
#endif /* INCLUDE_ADSL_MIB_RFC3440 */
   default:
      return "<unknown>";
   }
}

static DSL_Error_t DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
   DSL_uint32_t atuxFlagsAll,
   DSL_uint32_t origFlags,
   DSL_uint32_t handledFlags)
{
   if (handledFlags == origFlags)
   {
      return DSL_ERROR;
   }
   else if (handledFlags && (handledFlags != origFlags))
   {
      if (origFlags & ~atuxFlagsAll)
      {
         return DSL_WRN_INCONSISTENT_ADSL_MIB_FLAGS;
      }
      else
      {
         return DSL_WRN_INCOMPLETE_RETURN_VALUES;
      }
   }
   else
   {
      return DSL_SUCCESS;   
   }
}


/** @} DRV_DSL_CPE_DEBUG */

/** \addtogroup DRV_DSL_CPE_ADSL_MIB
 @{ */

DSL_Error_t DSL_DRV_MIB_ADSL_LineEntryGet(
   DSL_Context_t *pContext,
   adslLineTableEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_LineEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   if (IS_FLAG_SET(pData->flags, LINE_CODE_FLAG))
   {
      pData->adslLineCode = 2;
      CLR_FLAG(pData->flags, LINE_CODE_FLAG);
   }

   if (nErrCode == DSL_SUCCESS && pData->flags != 0)
   {
      nErrCode = DSL_ERR_NOT_SUPPORTED;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_LineEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_PhysEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   adslAtucPhysEntry_t *pAtucData = (adslAtucPhysEntry_t *)pData;
   adslAturPhysEntry_t *pAturData = (adslAturPhysEntry_t *)pData;
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   DSL_G997_LineInventory_t feInv;
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
#ifdef INCLUDE_DSL_G997_STATUS
   DSL_G997_LineStatus_t lineSts;
   DSL_G997_LineInitStatus_t lineInitSts;
   DSL_G997_LineFailures_t lineFailures;
#endif /* INCLUDE_DSL_G997_STATUS*/

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: IN - DSL_DRV_MIB_ADSL_PhysEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   memset(&feInv, 0, sizeof(DSL_G997_LineInventory_t));
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
   
#ifdef INCLUDE_DSL_G997_STATUS
   memset(&lineSts, 0, sizeof(DSL_G997_LineStatus_t));
   memset(&lineFailures, 0, sizeof(DSL_G997_LineFailures_t));
   memset(&lineInitSts, 0, sizeof(DSL_G997_LineInitStatus_t));
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   if ( ((origFlags & (ATUR_PHY_SER_NUM_FLAG | ATUR_PHY_VENDOR_ID_FLAG | ATUR_PHY_VER_NUM_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PHY_SER_NUM_FLAG | ATUC_PHY_VENDOR_ID_FLAG | ATUC_PHY_VER_NUM_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      feInv.nDirection = nDirection;
      /* Get Line Inventory information*/
      nErrCode = DSL_DRV_G997_LineInventoryGet(pContext, &feInv);

      if (nErrCode == DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            /* Serial number*/
            if (IS_FLAG_SET(pAturData->flags, ATUR_PHY_SER_NUM_FLAG))
            {
               memcpy(&pAturData->serial_no, &feInv.data.SerialNumber,
                  sizeof(pAturData->serial_no) > sizeof(feInv.data.SerialNumber) ?
                  sizeof(feInv.data.SerialNumber) : sizeof(pAturData->serial_no));

               CLR_FLAG(pAturData->flags, ATUR_PHY_SER_NUM_FLAG);
            }

            /* Vendor ID*/         
            if (IS_FLAG_SET(pAturData->flags, ATUR_PHY_VENDOR_ID_FLAG))
            {
               memcpy(&pAturData->vendor_id, &feInv.data.SystemVendorID,
                  sizeof(pAturData->vendor_id) > sizeof(feInv.data.SystemVendorID) ?
                  sizeof(feInv.data.SystemVendorID) : sizeof(pAturData->vendor_id));

               CLR_FLAG(pAturData->flags, ATUR_PHY_VENDOR_ID_FLAG);
            }

            /* Version number*/         
            if (IS_FLAG_SET(pAturData->flags, ATUR_PHY_VER_NUM_FLAG))
            {
               memcpy(&pAturData->version_no, &feInv.data.VersionNumber,
                  sizeof(pAturData->version_no) > sizeof(feInv.data.VersionNumber) ?
                  sizeof(feInv.data.VersionNumber) : sizeof(pAturData->version_no));

               CLR_FLAG(pAturData->flags, ATUR_PHY_VER_NUM_FLAG);
            }
         }
         else
         {
            /* Serial number*/
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PHY_SER_NUM_FLAG))
            {
               memcpy(&pAtucData->serial_no, &feInv.data.SerialNumber,
                  sizeof(pAtucData->serial_no) > sizeof(feInv.data.SerialNumber) ?
                  sizeof(feInv.data.SerialNumber) : sizeof(pAtucData->serial_no));

               CLR_FLAG(pAtucData->flags, ATUC_PHY_SER_NUM_FLAG);
            }

            /* Vendor ID*/
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PHY_VENDOR_ID_FLAG))
            {
               memcpy(&pAtucData->vendor_id, &feInv.data.G994VendorID,
                  sizeof(pAtucData->vendor_id) > sizeof(feInv.data.G994VendorID) ?
                     sizeof(feInv.data.G994VendorID) : sizeof(pAtucData->vendor_id));

               CLR_FLAG(pAtucData->flags, ATUC_PHY_VENDOR_ID_FLAG);
            }

            /* Version Number*/         
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PHY_VER_NUM_FLAG))
            {
               memcpy(&pAtucData->version_no, &feInv.data.VersionNumber,
                  sizeof(pAtucData->version_no) > sizeof(feInv.data.VersionNumber) ?
                  sizeof(feInv.data.VersionNumber) : sizeof(pAtucData->version_no));

               CLR_FLAG(pAtucData->flags, ATUC_PHY_VER_NUM_FLAG);
            }
         }
      }
   }
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

   if ( ((origFlags & (ATUR_CURR_OUT_PWR_FLAG | ATUR_CURR_ATTR_FLAG)) && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CURR_OUT_PWR_FLAG | ATUC_CURR_ATTR_FLAG)) && (nDirection == DSL_FAR_END)))
   {
      lineSts.nDirection    = nDirection == DSL_NEAR_END ? DSL_UPSTREAM : DSL_DOWNSTREAM;
      lineSts.nDeltDataType = DSL_DELT_DATA_SHOWTIME;
      /* Get Line Status*/
      nErrCode = DSL_DRV_G997_LineStatusGet(pContext, &lineSts);

      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CURR_OUT_PWR_FLAG))
            {
               pAturData->outputPwr = lineSts.data.ACTATP;
               CLR_FLAG(pAturData->flags, ATUR_CURR_OUT_PWR_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CURR_ATTR_FLAG))
            {
               pAturData->attainableRate = lineSts.data.ATTNDR;
               CLR_FLAG(pAturData->flags, ATUR_CURR_ATTR_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CURR_OUT_PWR_FLAG))
            {
               pAtucData->outputPwr = lineSts.data.ACTATP;
               CLR_FLAG(pAtucData->flags, ATUC_CURR_OUT_PWR_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CURR_ATTR_FLAG))
            {
               pAtucData->attainableRate = lineSts.data.ATTNDR;
               CLR_FLAG(pAtucData->flags, ATUC_CURR_ATTR_FLAG);
            }
         }
      }
   }

   if ( ((origFlags & (ATUR_CURR_ATN_FLAG | ATUR_CURR_SNR_FLAG)) && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CURR_ATN_FLAG | ATUC_CURR_SNR_FLAG)) && (nDirection == DSL_FAR_END)))
   {
      lineSts.nDirection    = nDirection == DSL_NEAR_END ? DSL_DOWNSTREAM : DSL_UPSTREAM;
      lineSts.nDeltDataType = DSL_DELT_DATA_SHOWTIME;
      /* Get Line Status*/
      nErrCode = DSL_DRV_G997_LineStatusGet(pContext, &lineSts);

      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CURR_ATN_FLAG))
            {
               pAturData->Attn = lineSts.data.LATN;
               CLR_FLAG(pAturData->flags, ATUR_CURR_ATN_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CURR_SNR_FLAG))
            {
               pAturData->SnrMgn = lineSts.data.SNR;
               CLR_FLAG(pAturData->flags, ATUR_CURR_SNR_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CURR_ATN_FLAG))
            {
               pAtucData->Attn = lineSts.data.LATN;
               CLR_FLAG(pAtucData->flags, ATUC_CURR_ATN_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CURR_SNR_FLAG))
            {
               pAtucData->SnrMgn = lineSts.data.SNR;
               CLR_FLAG(pAtucData->flags, ATUC_CURR_SNR_FLAG);
            }
         }
      }
   }

#ifdef INCLUDE_DSL_G997_STATUS
   if ( ((origFlags & (ATUR_CURR_STAT_FLAG)) && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CURR_STAT_FLAG)) && (nDirection == DSL_FAR_END)))
   {
      /* Get Line Init Status*/
      nErrCode = DSL_DRV_G997_LineInitStatusGet(pContext, &lineInitSts);   
      if (nErrCode == DSL_SUCCESS)
      {
         lineFailures.nDirection = nDirection;
         /* Get Line Failures Status*/
         nErrCode = DSL_DRV_G997_LineFailuresStatusGet(pContext, &lineFailures);
      }
   }

   if (nErrCode == DSL_SUCCESS)
   {
      if (nDirection == DSL_NEAR_END)
      {
         if (IS_FLAG_SET(pAturData->flags, ATUR_CURR_STAT_FLAG))
         {
            pAturData->status = 0;

            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOF)
            {
               pAturData->status |= 0x1;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOS)
            {
               pAturData->status |= 0x2;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LPR)
            {
               pAturData->status |= 0x4;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOM)
            {
               pAturData->status |= 0x8;
            }

            CLR_FLAG(pAturData->flags, ATUR_CURR_STAT_FLAG);
         }
      }
      else
      {
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CURR_STAT_FLAG))
         {
            pAtucData->status = 0;

            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOF)
            {
               pAtucData->status |= 0x1;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOS)
            {
               pAtucData->status |= 0x2;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LPR)
            {
               pAtucData->status |= 0x4;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOM)
            {
               pAtucData->status |= 0x8;
            }
            if (lineFailures.data.nLineFailures & DSL_G997_LINEFAILURE_LOL)
            {
               pAtucData->status |= 0x10;
            }
            if (lineInitSts.data.nLineInitStatus & LINIT_CONFIG_ERROR)
            {
               pAtucData->status |= 0x20;
            }
            if (lineInitSts.data.nLineInitStatus & LINIT_CONFIG_NOT_FEASIBLE)
            {
               pAtucData->status |= 0x40;
            }
            if (lineInitSts.data.nLineInitStatus & LINIT_COMMUNICATION_PROBLEM)
            {
               pAtucData->status |= 0x80;
            }
            if (lineInitSts.data.nLineInitStatus & LINIT_NO_PEER_XTU)
            {
               pAtucData->status |= 0x100;
            }

            CLR_FLAG(pAtucData->flags, ATUC_CURR_STAT_FLAG);
         }
      }
   }

#endif /* INCLUDE_DSL_G997_STATUS*/

   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 nDirection == DSL_NEAR_END ?
                 ((((DSL_uint32_t)(ATUR_CURR_SNR_FLAG)) - 1) | ATUR_CURR_SNR_FLAG) :
                 ((((DSL_uint32_t)(ATUC_CURR_SNR_FLAG)) - 1) | ATUC_CURR_SNR_FLAG),
                 origFlags, handledFlags);
   
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_PhysEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AtucPhysEntryGet(
   DSL_Context_t *pContext,
   adslAtucPhysEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucPhysEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_PhysEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucPhysEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturPhysEntryGet(
   DSL_Context_t *pContext,
   adslAturPhysEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturPhysEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_PhysEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturPhysEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_ChanEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t nChannel = 0;
   DSL_uint8_t origFlags = 0, handledFlags = 0;
   adslAturChanInfo_t *pAturData = (adslAturChanInfo_t *)pData;
   adslAtucChanInfo_t *pAtucData = (adslAtucChanInfo_t *)pData;

   DSL_G997_ChannelStatus_t channelSts;
#ifdef INCLUDE_DSL_FRAMING_PARAMETERS
   DSL_FramingParameterStatus_t FramingParameterStatus;
#endif /* INCLUDE_DSL_FRAMING_PARAMETERS*/
#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
   DSL_G997_FramingParameterStatus_t g997FramingParameterStatus;
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_ChanEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;
   nChannel  = (DSL_uint8_t)((nDirection == DSL_NEAR_END) ? pAturData->ifIndex : pAtucData->ifIndex);

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }
                                     
   memset(&channelSts, 0, sizeof(DSL_G997_ChannelStatus_t));

   /* get channel number */
   /*$$ND: TBD*/
   channelSts.nChannel   = nChannel;
   channelSts.nDirection = (DSL_AccessDir_t)nDirection;
   nErrCode = DSL_DRV_G997_ChannelStatusGet(pContext, &channelSts);

   if (nErrCode == DSL_SUCCESS)
   {
      if (nDirection == DSL_NEAR_END)
      {
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_INTLV_DELAY_FLAG))
         {
            DSL_uint32_t remain = 0;
            
            remain = channelSts.data.ActualInterleaveDelay - 
                        (channelSts.data.ActualInterleaveDelay/100)*100;
            pAturData->interleaveDelay =
               (channelSts.data.ActualInterleaveDelay/100) + (remain >= 50 ? 1 : 0);
            CLR_FLAG(pAturData->flags, ATUR_CHAN_INTLV_DELAY_FLAG);
         }
         
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_CURR_TX_RATE_FLAG))
         {
            pAturData->currTxRate = channelSts.data.ActualDataRate;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_CURR_TX_RATE_FLAG);
         }
         
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PREV_TX_RATE_FLAG))
         {
            pAturData->prevTxRate = channelSts.data.PreviousDataRate;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_PREV_TX_RATE_FLAG);
         }
      }
      else
      {
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTLV_DELAY_FLAG))
         {
            DSL_uint32_t remain = 0;
            
            remain = channelSts.data.ActualInterleaveDelay - 
                        (channelSts.data.ActualInterleaveDelay/100)*100;
            pAtucData->interleaveDelay =
               (channelSts.data.ActualInterleaveDelay/100) + (remain >= 50 ? 1 : 0);
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_INTLV_DELAY_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_CURR_TX_RATE_FLAG))
         {
            pAtucData->currTxRate = channelSts.data.ActualDataRate;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_CURR_TX_RATE_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PREV_TX_RATE_FLAG))
         {
            pAtucData->prevTxRate = channelSts.data.PreviousDataRate;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_PREV_TX_RATE_FLAG);
         }
      }
   }

#ifdef INCLUDE_DSL_FRAMING_PARAMETERS
   if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_CRC_BLK_LEN_FLAG))
   {
      FramingParameterStatus.nChannel   = nChannel;
      FramingParameterStatus.nDirection = DSL_UPSTREAM;
   
      nErrCode = DSL_DRV_FramingParameterStatusGet(pContext, &FramingParameterStatus);

      if (nErrCode == DSL_SUCCESS)
      {
         /* Check for ADSL2/2+ modes*/
         if ((pContext->xtseCurr[2] & XTSE_3_03_A_3_NO) ||
             (pContext->xtseCurr[3] & XTSE_4_05_I_3_NO) ||
             (pContext->xtseCurr[4] & XTSE_5_03_L_3_NO) ||
             (pContext->xtseCurr[4] & XTSE_5_04_L_3_NO) ||
             (pContext->xtseCurr[4] & XTSE_5_07_M_3_NO) ||
             (pContext->xtseCurr[5] & XTSE_6_01_A_5_NO) ||
             (pContext->xtseCurr[5] & XTSE_6_07_I_5_NO) ||
             (pContext->xtseCurr[6] & XTSE_7_03_M_5_NO) ||
             (pContext->xtseCurr[2] & XTSE_3_05_B_3_NO) ||
             (pContext->xtseCurr[3] & XTSE_4_07_J_3_NO) ||
             (pContext->xtseCurr[5] & XTSE_6_03_B_5_NO) ||
             (pContext->xtseCurr[6] & XTSE_7_01_J_5_NO))
         {
            pAturData->crcBlkLen = FramingParameterStatus.data.nSEQ;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_CRC_BLK_LEN_FLAG);
         }
#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
         /* Check for ADSL1 mode*/
         else if ((pContext->xtseCurr[0] & XTSE_1_03_A_1_NO)   ||
                  (pContext->xtseCurr[0] & XTSE_1_01_A_T1_413) ||
                  (pContext->xtseCurr[1] & XTSE_2_01_A_2_NO)   ||
                  (pContext->xtseCurr[0] & XTSE_1_01_A_T1_413) || 
                  (pContext->xtseCurr[0] & XTSE_1_05_B_1_NO))
         {
            g997FramingParameterStatus.nChannel   = nChannel;
            g997FramingParameterStatus.nDirection = DSL_UPSTREAM;

            nErrCode = DSL_DRV_G997_FramingParameterStatusGet(
                          pContext, &g997FramingParameterStatus);

            if (nErrCode == DSL_SUCCESS)
            {
               pAturData->crcBlkLen = 
                  68 * (FramingParameterStatus.data.nBP + g997FramingParameterStatus.data.nRFEC + 1) - 1;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_CRC_BLK_LEN_FLAG);
            }
         }
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/
      }
   }
#endif /* INCLUDE_DSL_FRAMING_PARAMETERS*/

   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 nDirection == DSL_NEAR_END ?
                 ((((DSL_uint32_t)(ATUR_CHAN_CRC_BLK_LEN_FLAG)) - 1) | ATUR_CHAN_CRC_BLK_LEN_FLAG) :
                 ((((DSL_uint32_t)(ATUC_CHAN_PREV_TX_RATE_FLAG)) - 1) | ATUC_CHAN_PREV_TX_RATE_FLAG),
                 (DSL_uint32_t)origFlags, (DSL_uint32_t)handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_ChanEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AtucChanEntryGet(
   DSL_Context_t *pContext,
   adslAtucChanInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucChanEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_ChanEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucChanEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturChanEntryGet(
   DSL_Context_t *pContext,
   adslAturChanInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturChanEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_ChanEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturChanEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
DSL_Error_t DSL_DRV_MIB_ADSL_PerfDataEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   aturPerfDataEntry_t *pAturData = (aturPerfDataEntry_t *)pData;
   atucPerfDataEntry_t *pAtucData = (atucPerfDataEntry_t *)pData;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_LineSecCounters_t lineSecData;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_PM_LineSecCountersTotal_t lineSecTotData;
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_PM_HistoryStatsDir_t histStats;
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_PerfDataEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   memset(&lineSecTotData, 0x0, sizeof(DSL_PM_LineSecCountersTotal_t));
   lineSecTotData.nDirection = nDirection;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   memset(&lineSecData, 0x0, sizeof(DSL_PM_LineSecCounters_t));
   lineSecData.nDirection = histStats.nDirection = nDirection;
   lineSecData.nHistoryInterval = 0;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   memset(&histStats, 0x0, sizeof(DSL_PM_HistoryStatsDir_t));
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS     
   if ( ((origFlags & (ATUR_PERF_LOFS_FLAG | ATUR_PERF_LOSS_FLAG | ATUR_PERF_ESS_FLAG | ATUR_PERF_LPR_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_LOFS_FLAG | ATUC_PERF_LOSS_FLAG | ATUC_PERF_ESS_FLAG | ATUC_PERF_INITS_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      /* Get Total Line Sec counters*/
      nErrCode = DSL_DRV_PM_LineSecCountersTotalGet(pContext, &lineSecTotData);
   
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_LOFS_FLAG))
            {
               pAturData->adslAturPerfLofs = lineSecTotData.data.nLOFS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_LOFS_FLAG);
            }
         
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_LOSS_FLAG))
            {
               pAturData->adslAturPerfLoss = lineSecTotData.data.nLOSS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_LOSS_FLAG);
            }
         
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_ESS_FLAG))
            {
               pAturData->adslAturPerfESs = lineSecTotData.data.nES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_LPR_FLAG))
            {
               /* Not supported*/
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_LOFS_FLAG))
            {
               pAtucData->adslAtucPerfLofs = lineSecTotData.data.nLOFS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_LOFS_FLAG);
            }
         
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_LOSS_FLAG))
            {
               pAtucData->adslAtucPerfLoss = lineSecTotData.data.nLOSS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_LOSS_FLAG);
            }
         
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_ESS_FLAG))
            {
               pAtucData->adslAtucPerfESs = lineSecTotData.data.nES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_INITS_FLAG))
            {
               /* Not supported*/
            }
         }
      }
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   if ( ((origFlags & (ATUR_PERF_VALID_INTVLS_FLAG | ATUR_PERF_INVALID_INTVLS_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_VALID_INTVLS_FLAG | ATUC_PERF_INVALID_INTVLS_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_LineSecHistoryStats15MinGet(pContext, &histStats);
   
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if(IS_FLAG_SET(pAturData->flags, ATUR_PERF_VALID_INTVLS_FLAG))
            {
               pAturData->adslAturPerfValidIntervals = histStats.data.nPrevIvs;
               CLR_FLAG(pAturData->flags, ATUR_PERF_VALID_INTVLS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_INVALID_INTVLS_FLAG))
            {
               pAturData->adslAturPerfInvalidIntervals = histStats.data.nPrevInvalidIvs;
               CLR_FLAG(pAturData->flags, ATUR_PERF_INVALID_INTVLS_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_VALID_INTVLS_FLAG))
            {
               pAtucData->adslAtucPerfValidIntervals = histStats.data.nPrevIvs;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_VALID_INTVLS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_INVALID_INTVLS_FLAG))
            {
               pAtucData->adslAtucPerfInvalidIntervals = histStats.data.nPrevInvalidIvs;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_INVALID_INTVLS_FLAG);
            }
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/   

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if ( ((origFlags & (ATUR_PERF_CURR_15MIN_TIME_ELAPSED_FLAG | ATUR_PERF_CURR_15MIN_LOFS_FLAG |
                       ATUR_PERF_CURR_15MIN_LOSS_FLAG | ATUR_PERF_CURR_15MIN_ESS_FLAG | 
                       ATUR_PERF_CURR_15MIN_LPR_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_CURR_15MIN_TIME_ELAPSED_FLAG | ATUC_PERF_CURR_15MIN_LOFS_FLAG |
                       ATUC_PERF_CURR_15MIN_LOSS_FLAG | ATUC_PERF_CURR_15MIN_ESS_FLAG |
                       ATUC_PERF_CURR_15MIN_INIT_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_LineSecCounters15MinGet(pContext, &lineSecData);

      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_TIME_ELAPSED_FLAG))
            {
               pAturData->adslAturPerfCurr15MinTimeElapsed = lineSecData.interval.nElapsedTime;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_15MIN_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_LOFS_FLAG))
            {
               pAturData->adslAturPerfCurr15MinLofs = lineSecData.data.nLOFS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_15MIN_LOFS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_LOSS_FLAG))
            {
               pAturData->adslAturPerfCurr15MinLoss = lineSecData.data.nLOSS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_15MIN_LOSS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_ESS_FLAG))
            {
               pAturData->adslAturPerfCurr15MinESs = lineSecData.data.nES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_15MIN_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_LPR_FLAG))
            {
               /* Not supported*/
            }
         }
         else
         {
            if(IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_TIME_ELAPSED_FLAG))
            {
               pAtucData->adslAtucPerfCurr15MinTimeElapsed = lineSecData.interval.nElapsedTime;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_15MIN_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_LOFS_FLAG))
            {
               pAtucData->adslAtucPerfCurr15MinLofs = lineSecData.data.nLOFS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_15MIN_LOFS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_LOSS_FLAG))
            {
               pAtucData->adslAtucPerfCurr15MinLoss = lineSecData.data.nLOSS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_15MIN_LOSS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_ESS_FLAG))
            {
               pAtucData->adslAtucPerfCurr15MinESs = lineSecData.data.nES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_15MIN_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_INIT_FLAG))
            {
               /* Not supported*/
            }
         }
      }
   }

   if ( ((origFlags & (ATUR_PERF_CURR_1DAY_TIME_ELAPSED_FLAG | ATUR_PERF_CURR_1DAY_LOFS_FLAG |
                       ATUR_PERF_CURR_1DAY_LOSS_FLAG | ATUR_PERF_CURR_1DAY_ESS_FLAG | 
                       ATUR_PERF_CURR_1DAY_LPR_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_CURR_1DAY_TIME_ELAPSED_FLAG | ATUC_PERF_CURR_1DAY_LOFS_FLAG |
                       ATUC_PERF_CURR_1DAY_LOSS_FLAG | ATUC_PERF_CURR_1DAY_ESS_FLAG |
                       ATUC_PERF_CURR_1DAY_INIT_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_LineSecCounters1DayGet(pContext, &lineSecData);

      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_TIME_ELAPSED_FLAG))
            {
               pAturData->adslAturPerfCurr1DayTimeElapsed = lineSecData.interval.nElapsedTime;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_1DAY_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_LOFS_FLAG))
            {
               pAturData->adslAturPerfCurr1DayLofs = lineSecData.data.nLOFS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_1DAY_LOFS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_LOSS_FLAG))
            {
               pAturData->adslAturPerfCurr1DayLoss = lineSecData.data.nLOSS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_1DAY_LOSS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_ESS_FLAG))
            {
               pAturData->adslAturPerfCurr1DayESs = lineSecData.data.nES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_1DAY_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_LPR_FLAG))
            {
               /* Not supported*/
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_TIME_ELAPSED_FLAG))
            {
               pAtucData->adslAtucPerfCurr1DayTimeElapsed = lineSecData.interval.nElapsedTime;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_1DAY_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_LOFS_FLAG))
            {
               pAtucData->adslAtucPerfCurr1DayLofs = lineSecData.data.nLOFS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_1DAY_LOFS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_LOSS_FLAG))
            {
               pAtucData->adslAtucPerfCurr1DayLoss = lineSecData.data.nLOSS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_1DAY_LOSS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_ESS_FLAG))
            {
               pAtucData->adslAtucPerfCurr1DayESs = lineSecData.data.nES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_1DAY_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_INIT_FLAG))
            {
               /* Not supported*/
            }
         }
      }
   }

   if ( ((origFlags & (ATUR_PERF_PREV_1DAY_MON_SEC_FLAG | ATUR_PERF_PREV_1DAY_LOFS_FLAG |
                       ATUR_PERF_PREV_1DAY_LOSS_FLAG | ATUR_PERF_PREV_1DAY_ESS_FLAG |
                       ATUR_PERF_PREV_1DAY_LPR_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_PREV_1DAY_MON_SEC_FLAG | ATUC_PERF_PREV_1DAY_LOFS_FLAG |
                       ATUC_PERF_PREV_1DAY_LOSS_FLAG | ATUC_PERF_PREV_1DAY_ESS_FLAG |
                       ATUC_PERF_PREV_1DAY_INITS_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      lineSecData.nHistoryInterval = 1;
      nErrCode = DSL_DRV_PM_LineSecCounters1DayGet(pContext, &lineSecData);
    
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_MON_SEC_FLAG))
            {
               pAturData->adslAturPerfPrev1DayMoniSecs = lineSecData.interval.nElapsedTime;
               CLR_FLAG(pAturData->flags, ATUR_PERF_PREV_1DAY_MON_SEC_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_LOFS_FLAG))
            {
               pAturData->adslAturPerfPrev1DayLofs = lineSecData.data.nLOFS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_PREV_1DAY_LOFS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_LOSS_FLAG))
            {
               pAturData->adslAturPerfPrev1DayLoss = lineSecData.data.nLOSS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_PREV_1DAY_LOSS_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_ESS_FLAG))
            {
               pAturData->adslAturPerfPrev1DayESs = lineSecData.data.nES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_PREV_1DAY_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_LPR_FLAG))
            {
               /* Not supported*/
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_MON_SEC_FLAG))
            {
               pAtucData->adslAtucPerfPrev1DayMoniSecs = lineSecData.interval.nElapsedTime;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_PREV_1DAY_MON_SEC_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_LOFS_FLAG))
            {
               pAtucData->adslAtucPerfPrev1DayLofs = lineSecData.data.nLOFS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_PREV_1DAY_LOFS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_LOSS_FLAG))
            {
               pAtucData->adslAtucPerfPrev1DayLoss = lineSecData.data.nLOSS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_PREV_1DAY_LOSS_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_ESS_FLAG))
            {
               pAtucData->adslAtucPerfPrev1DayESs = lineSecData.data.nES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_PREV_1DAY_ESS_FLAG);
            }

            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_INITS_FLAG))
            {
               /* Not supported*/
            }
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 nDirection == DSL_NEAR_END ?
                 ((((DSL_uint32_t)(ATUR_PERF_PREV_1DAY_ESS_FLAG)) - 1) | ATUR_PERF_PREV_1DAY_ESS_FLAG) :
                 ((((DSL_uint32_t)(ATUC_PERF_PREV_1DAY_INITS_FLAG)) - 1) | ATUC_PERF_PREV_1DAY_INITS_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_PerfDataEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AtucPerfDataEntryGet(
   DSL_Context_t *pContext,
   atucPerfDataEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucPerfDataEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_PerfDataEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucPerfDataEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturPerfDataEntryGet(
   DSL_Context_t *pContext,
   aturPerfDataEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturPerfDataEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_PerfDataEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturPerfDataEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_MIB_ADSL_IntervalEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   adslAturIntvlInfo_t *pAturData = (adslAturIntvlInfo_t *)pData;
   adslAtucIntvlInfo_t *pAtucData = (adslAtucIntvlInfo_t *)pData;
   DSL_PM_LineSecCounters_t lineSecData;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_IntervalEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   if (nDirection == DSL_FAR_END)
   {
      lineSecData.nHistoryInterval  = (DSL_uint32_t)pAturData->IntervalNumber;
   }
   else if (nDirection == DSL_NEAR_END)
   {
      lineSecData.nHistoryInterval  = (DSL_uint32_t)pAtucData->IntervalNumber;
   }
   lineSecData.nDirection = nDirection;

   nErrCode = DSL_DRV_PM_LineSecCounters15MinGet(pContext, &lineSecData);
   if (nErrCode >= DSL_SUCCESS)
   {
      if (nDirection == DSL_NEAR_END)
      {
         if (IS_FLAG_SET(pAturData->flags, ATUR_INTVL_LOF_FLAG))
         {
            pAturData->intervalLOF = lineSecData.data.nLOFS;
            CLR_FLAG(pAturData->flags, ATUR_INTVL_LOF_FLAG);
         }
      
         if (IS_FLAG_SET(pAturData->flags, ATUR_INTVL_LOS_FLAG))
         {
            pAturData->intervalLOS = lineSecData.data.nLOSS;
            CLR_FLAG(pAturData->flags, ATUR_INTVL_LOS_FLAG);
         }
      
         if (IS_FLAG_SET(pAturData->flags, ATUR_INTVL_ESS_FLAG))
         {
            pAturData->intervalES = lineSecData.data.nES;
            CLR_FLAG(pAturData->flags, ATUR_INTVL_ESS_FLAG);
         }
      
         if (IS_FLAG_SET(pAturData->flags, ATUR_INTVL_VALID_DATA_FLAG))
         {
            pAturData->intervalValidData = lineSecData.interval.bValid;
            CLR_FLAG(pAturData->flags, ATUR_INTVL_VALID_DATA_FLAG);
         }

         if (IS_FLAG_SET(pAturData->flags, ATUR_INTVL_LPR_FLAG))
         {
            /* Not supported*/
         }
      }
      else
      {
         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTVL_LOF_FLAG))
         {
            pAtucData->intervalLOF = lineSecData.data.nLOFS;
            CLR_FLAG(pAtucData->flags, ATUC_INTVL_LOF_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTVL_LOS_FLAG))
         {
            pAtucData->intervalLOS = lineSecData.data.nLOSS;
            CLR_FLAG(pAtucData->flags, ATUC_INTVL_LOS_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTVL_ESS_FLAG))
         {
            pAtucData->intervalES = lineSecData.data.nES;
            CLR_FLAG(pAtucData->flags, ATUC_INTVL_ESS_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTVL_VALID_DATA_FLAG))
         {
            pAtucData->intervalValidData = lineSecData.interval.bValid;
            CLR_FLAG(pAtucData->flags, ATUC_INTVL_VALID_DATA_FLAG);
         }

         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTVL_INIT_FLAG))
         {
            /* Not supported*/
         }
      }
   }


   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 nDirection == DSL_NEAR_END ?
                 ((((DSL_uint32_t)(ATUR_INTVL_VALID_DATA_FLAG)) - 1) | ATUR_INTVL_VALID_DATA_FLAG) :
                 ((((DSL_uint32_t)(ATUC_INTVL_VALID_DATA_FLAG)) - 1) | ATUC_INTVL_VALID_DATA_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_IntervalEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AtucIntervalEntryGet(
   DSL_Context_t *pContext,
   adslAtucIntvlInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucIntervalEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_IntervalEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucIntervalEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturIntervalEntryGet(
   DSL_Context_t *pContext,
   adslAturIntvlInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturIntervalEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_IntervalEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturIntervalEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
DSL_Error_t DSL_DRV_MIB_ADSL_ChanPerfDataEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   aturChannelPerfDataEntry_t *pAturData = (aturChannelPerfDataEntry_t *)pData;
   atucChannelPerfDataEntry_t *pAtucData = (atucChannelPerfDataEntry_t *)pData;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_PM_HistoryStatsChDir_t histStat;
   #endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   DSL_PM_ChannelCounters_t chanData;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   DSL_PM_ChannelCountersExt_t chanExtData;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_PM_ChannelCountersTotal_t chanTotData;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   DSL_PM_ChannelCountersExtTotal_t chanExtTotData;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_ChanPerfDataEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   memset(&chanData, 0x0, sizeof(DSL_PM_ChannelCounters_t));
   chanData.nDirection = nDirection;
   chanData.nHistoryInterval =0;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   memset(&chanExtData, 0x0, sizeof(DSL_PM_ChannelCountersExt_t));
   chanExtData.nDirection = DSL_NEAR_END;
   chanExtData.nHistoryInterval =0;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   memset(&histStat, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));
   histStat.nDirection = nDirection;
   /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   memset(&chanTotData, 0x0, sizeof(DSL_PM_ChannelCountersTotal_t));
   chanTotData.nDirection = nDirection;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   memset(&chanExtTotData, 0x0, sizeof(DSL_PM_ChannelCountersExtTotal_t));
   chanExtTotData.nDirection = DSL_NEAR_END;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

   /*$$ND: TBD */
   if (nDirection == DSL_NEAR_END)
   {
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      chanData.nChannel = (DSL_uint8_t)pAturData->ifIndex;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtData.nChannel = (DSL_uint8_t)pAturData->ifIndex;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      chanTotData.nChannel = (DSL_uint8_t)pAturData->ifIndex;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtTotData.nChannel = (DSL_uint8_t)pAturData->ifIndex;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif
   }
   else if (nDirection == DSL_FAR_END)
   {
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      chanData.nChannel = (DSL_uint8_t)pAtucData->ifIndex;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtData.nChannel = (DSL_uint8_t)pAtucData->ifIndex;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      chanTotData.nChannel = (DSL_uint8_t)pAtucData->ifIndex;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtTotData.nChannel = (DSL_uint8_t)pAtucData->ifIndex;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
   }

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   if ( ((origFlags & (ATUR_CHAN_RECV_BLK_FLAG | ATUR_CHAN_TX_BLK_FLAG |
                       ATUR_CHAN_CORR_BLK_FLAG | ATUR_CHAN_UNCORR_BLK_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CHAN_RECV_BLK_FLAG | ATUC_CHAN_TX_BLK_FLAG |
                       ATUC_CHAN_CORR_BLK_FLAG | ATUC_CHAN_UNCORR_BLK_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      nErrCode = DSL_DRV_PM_ChannelCountersExtTotalGet(pContext, &chanExtTotData);
      
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_RECV_BLK_FLAG))
            {
               pAturData->adslAturChanReceivedBlks = chanExtTotData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_RECV_BLK_FLAG);
            }
         
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_TX_BLK_FLAG))
            {
               pAturData->adslAturChanTransmittedBlks = chanExtTotData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_TX_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_RECV_BLK_FLAG))
            {
               pAtucData->adslAtucChanReceivedBlks = chanExtTotData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_RECV_BLK_FLAG);
            }
         
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_TX_BLK_FLAG))
            {
               pAtucData->adslAtucChanTransmittedBlks = chanExtTotData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_TX_BLK_FLAG);
            }
         }
      }
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

      nErrCode = DSL_DRV_PM_ChannelCountersTotalGet(pContext, &chanTotData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_CORR_BLK_FLAG))
            {
               pAturData->adslAturChanCorrectedBlks = chanTotData.data.nFEC;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_UNCORR_BLK_FLAG))
            {
               pAturData->adslAturChanUncorrectBlks = chanTotData.data.nCodeViolations;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_UNCORR_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_CORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanCorrectedBlks = chanTotData.data.nFEC;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_UNCORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanUncorrectBlks = chanTotData.data.nCodeViolations;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_UNCORR_BLK_FLAG);
            }
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/


#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if ( ((origFlags & (ATUR_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG | ATUR_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG |
                       ATUR_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG | ATUR_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG |
                       ATUR_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG | ATUC_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG |
                       ATUC_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG | ATUC_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG |
                       ATUC_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      nErrCode = DSL_DRV_PM_ChannelCountersExt15MinGet(pContext, &chanExtData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr15MinReceivedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr15MinTransmittedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr15MinReceivedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr15MinTransmittedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG);
            }
         }
      }
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

      nErrCode = DSL_DRV_PM_ChannelCounters15MinGet(pContext, &chanData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG))
            {
               pAturData->adslAturChanPerfCurr15MinTimeElapsed =
                  chanData.interval.nElapsedTime;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr15MinCorrectedBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr15MinUncorrectBlks =
                  chanData.data.nCodeViolations;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr15MinTimeElapsed =
                  chanData.interval.nElapsedTime;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr15MinCorrectedBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr15MinUncorrectBlks =
                  chanData.data.nCodeViolations;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG);
            }
         }
      }
   }


   if ( ((origFlags & (ATUR_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG | ATUR_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG |
                       ATUR_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG | ATUR_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG |
                       ATUR_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG | ATUC_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG |
                       ATUC_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG | ATUC_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG |
                       ATUC_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      nErrCode = DSL_DRV_PM_ChannelCountersExt1DayGet(pContext, &chanExtData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr1DayReceivedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr1DayTransmittedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr1DayReceivedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr1DayTransmittedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG);
            }
         }
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

      nErrCode = DSL_DRV_PM_ChannelCounters1DayGet(pContext, &chanData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG))
            {
               pAturData->adslAturChanPerfCurr1DayTimeElapsed =
                  chanData.interval.nElapsedTime;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr1DayCorrectedBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG))
            {
               pAturData->adslAturChanPerfCurr1DayUncorrectBlks =
                  chanData.data.nCodeViolations;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr1DayTimeElapsed =
                  chanData.interval.nElapsedTime;

               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr1DayCorrectedBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfCurr1DayUncorrectBlks =
                  chanData.data.nCodeViolations;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG);
            }
         }
      }
   }
   
   if ( ((origFlags & (ATUR_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG | ATUR_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG |
                    ATUR_CHAN_PERF_PREV_1DAY_TRANS_BLK_FLAG | ATUR_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG |
                    ATUR_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG))
      && (nDirection == DSL_NEAR_END)) ||
     ((origFlags & (ATUC_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG | ATUC_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG |
                    ATUC_CHAN_PERF_PREV_1DAY_TX_BLK_FLAG | ATUC_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG |
                    ATUC_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG))
      && (nDirection == DSL_FAR_END)))
   {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtData.nHistoryInterval = 1;
      nErrCode = DSL_DRV_PM_ChannelCountersExt1DayGet(pContext, &chanExtData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG))
            {
               pAturData->adslAturChanPerfPrev1DayReceivedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_TRANS_BLK_FLAG))
            {
               pAturData->adslAturChanPerfPrev1DayTransmittedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_TRANS_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfPrev1DayReceivedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_TX_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfPrev1DayTransmittedBlks = chanExtData.data.nSuperFrame;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_TX_BLK_FLAG);
            }
         }
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

      chanData.nHistoryInterval = 1;
      nErrCode = DSL_DRV_PM_ChannelCounters1DayGet(pContext, &chanData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG))
            {
               pAturData->adslAturChanPerfPrev1DayMoniSecs =
                  chanData.interval.nElapsedTime;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG))
            {
               pAturData->adslAturChanPerfPrev1DayCorrectedBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG))
            {
               pAturData->adslAturChanPerfPrev1DayUncorrectBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG))
            {
               pAtucData->adslAtucChanPerfPrev1DayMoniSecs =
                  chanData.interval.nElapsedTime;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG);            
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfPrev1DayCorrectedBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG))
            {
               pAtucData->adslAtucChanPerfPrev1DayUncorrectBlks =
                  chanData.data.nFEC;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG);
            }
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   if ( ((origFlags & (ATUR_CHAN_PERF_VALID_INTVL_FLAG | ATUR_CHAN_PERF_INVALID_INTVL_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_CHAN_PERF_VALID_INTVL_FLAG | ATUC_CHAN_PERF_INVALID_INTVL_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_ChannelHistoryStats15MinGet(pContext, &histStat);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_VALID_INTVL_FLAG))
            {
               pAturData->adslAturChanPerfValidIntervals = histStat.data.nPrevIvs;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_VALID_INTVL_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_PERF_INVALID_INTVL_FLAG))
            {
               pAturData->adslAturChanPerfInvalidIntervals = histStat.data.nPrevInvalidIvs;
               CLR_FLAG(pAturData->flags, ATUR_CHAN_PERF_INVALID_INTVL_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_VALID_INTVL_FLAG))
            {
               pAtucData->adslAtucChanPerfValidIntervals = histStat.data.nPrevIvs;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_VALID_INTVL_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_PERF_INVALID_INTVL_FLAG))
            {
               pAtucData->adslAtucChanPerfInvalidIntervals = histStat.data.nPrevInvalidIvs;
               CLR_FLAG(pAtucData->flags, ATUC_CHAN_PERF_INVALID_INTVL_FLAG);
            }
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/


   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 nDirection == DSL_NEAR_END ?
                 ((((DSL_uint32_t)(ATUR_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG)) - 1) |
                    ATUR_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG) :
                 ((((DSL_uint32_t)(ATUC_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG)) - 1) |
                    ATUC_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_ChanPerfDataEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
DSL_Error_t DSL_DRV_MIB_ADSL_AtucChanPerfDataEntryGet(
   DSL_Context_t *pContext,
   atucChannelPerfDataEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucChanPerfDataEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_ChanPerfDataEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucChanPerfDataEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturChanPerfDataEntryGet(
   DSL_Context_t *pContext,
   aturChannelPerfDataEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturChanPerfDataEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_ChanPerfDataEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturChanPerfDataEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_MIB_ADSL_ChanIntervalEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   adslAturChanIntvlInfo_t *pAturData = (adslAturChanIntvlInfo_t *)pData;
   adslAtucChanIntvlInfo_t *pAtucData = (adslAtucChanIntvlInfo_t *)pData;
   DSL_PM_ChannelCounters_t chanData;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   DSL_PM_ChannelCountersExt_t chanExtData;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_ChanIntervalEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   chanData.nDirection = nDirection;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   chanExtData.nDirection = DSL_NEAR_END;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

   /*$$ND: TBD */
   if (nDirection == DSL_NEAR_END)
   {
      chanData.nChannel = (DSL_uint8_t)pAtucData->ifIndex;
      chanData.nHistoryInterval = (DSL_uint32_t)pAtucData->IntervalNumber;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtData.nChannel = (DSL_uint8_t)pAtucData->ifIndex;
      chanExtData.nHistoryInterval = (DSL_uint32_t)pAtucData->IntervalNumber;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
   }
   else if (nDirection == DSL_FAR_END)
   {
      chanData.nChannel = (DSL_uint8_t)pAturData->ifIndex;
      chanData.nHistoryInterval = (DSL_uint32_t)pAturData->IntervalNumber;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
      chanExtData.nChannel =(DSL_uint8_t)pAturData->ifIndex;
      chanExtData.nHistoryInterval =(DSL_uint32_t)pAturData->IntervalNumber;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
   }

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
   nErrCode = DSL_DRV_PM_ChannelCountersExt15MinGet(pContext, &chanExtData);
   if (nErrCode >= DSL_SUCCESS)
   {
      if (nDirection == DSL_NEAR_END)
      {
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_INTVL_RECV_BLK_FLAG))
         {
            pAturData->chanIntervalRecvdBlks = chanExtData.data.nSuperFrame;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_INTVL_RECV_BLK_FLAG);
         }
         
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_INTVL_TX_BLK_FLAG))
         {
            pAturData->chanIntervalXmitBlks = chanExtData.data.nSuperFrame;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_INTVL_TX_BLK_FLAG);
         }
      }
      else
      {
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTVL_RECV_BLK_FLAG))
         {
            pAtucData->chanIntervalRecvdBlks = chanExtData.data.nSuperFrame;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_INTVL_RECV_BLK_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTVL_TX_BLK_FLAG))
         {
            pAtucData->chanIntervalXmitBlks = chanExtData.data.nSuperFrame;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_INTVL_TX_BLK_FLAG);
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/

   nErrCode = DSL_DRV_PM_ChannelCounters15MinGet(pContext, &chanData);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (nDirection == DSL_NEAR_END)
      {
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_INTVL_CORR_BLK_FLAG))
         {
            pAturData->chanIntervalCorrectedBlks =
               chanData.data.nFEC;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_INTVL_CORR_BLK_FLAG);
         }
         
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_INTVL_UNCORR_BLK_FLAG))
         {
            pAturData->chanIntervalUncorrectBlks =
               chanData.data.nCodeViolations;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_INTVL_UNCORR_BLK_FLAG);
         }
         
         if (IS_FLAG_SET(pAturData->flags, ATUR_CHAN_INTVL_VALID_DATA_FLAG))
         {
            pAturData->intervalValidData = chanData.interval.bValid;
            CLR_FLAG(pAturData->flags, ATUR_CHAN_INTVL_VALID_DATA_FLAG);
         }
      }
      else
      {
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTVL_NUM_FLAG))
         {
            CLR_FLAG(pAturData->flags, ATUC_CHAN_INTVL_NUM_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTVL_CORR_BLK_FLAG))
         {
            pAtucData->chanIntervalCorrectedBlks =
               chanData.data.nFEC;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_INTVL_CORR_BLK_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTVL_UNCORR_BLK_FLAG))
         {
            pAtucData->chanIntervalUncorrectBlks =
               chanData.data.nCodeViolations;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_INTVL_UNCORR_BLK_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_CHAN_INTVL_VALID_DATA_FLAG))
         {
            pAtucData->intervalValidData =
               chanData.interval.bValid;
            CLR_FLAG(pAtucData->flags, ATUC_CHAN_INTVL_VALID_DATA_FLAG);
         }
      }
   }


   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 nDirection == DSL_NEAR_END ?
                 ((((DSL_uint32_t)(ATUR_CHAN_INTVL_VALID_DATA_FLAG)) - 1) | ATUR_CHAN_INTVL_VALID_DATA_FLAG) :
                 ((((DSL_uint32_t)(ATUC_CHAN_INTVL_VALID_DATA_FLAG)) - 1) | ATUC_CHAN_INTVL_VALID_DATA_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_ChanIntervalEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AtucChanIntervalEntryGet(
   DSL_Context_t *pContext,
   adslAtucChanIntvlInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucChanIntervalEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_ChanIntervalEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucChanIntervalEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturChanIntervalEntryGet(
   DSL_Context_t *pContext,
   adslAturChanIntvlInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturChanIntervalEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_ChanIntervalEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturChanIntervalEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_PM*/

#ifdef INCLUDE_DSL_G997_ALARM
static DSL_Error_t DSL_DRV_MIB_ADSL_LineAlarmDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_G997_ChannelDataRateThresholdData_t *pDataRateInterleaveThresholds,
   DSL_G997_ChannelDataRateThresholdData_t *pDataRateFastThresholds)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_G997_ChannelDataRateThreshold_t dataRateThresholds;
   DSL_LatencyPath_t nLP;

   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   dataRateThresholds.nDirection = nDirection;
   dataRateThresholds.nChannel = 0;


   nErrCode = DSL_DRV_G997_ChannelDataRateThresholdConfigGet(
      pContext, &dataRateThresholds);
   if (nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][0], nLP);

      if (nLP == DSL_LATENCY_IP_LP0)
      {
         pDataRateInterleaveThresholds->nDataRateThresholdUpshift =
            dataRateThresholds.data.nDataRateThresholdUpshift;
         pDataRateInterleaveThresholds->nDataRateThresholdDownshift =
            dataRateThresholds.data.nDataRateThresholdDownshift;
      }
      else
      {
         pDataRateFastThresholds->nDataRateThresholdUpshift =
            dataRateThresholds.data.nDataRateThresholdUpshift;
         pDataRateFastThresholds->nDataRateThresholdDownshift =
            dataRateThresholds.data.nDataRateThresholdDownshift;
      }


#if (DSL_CHANNELS_PER_LINE > 1)
      dataRateThresholds.nChannel = 1;
      nErrCode = DSL_DRV_G997_ChannelDataRateThresholdConfigGet(
         pContext, &dataRateThresholds);
      if (nErrCode == DSL_SUCCESS)
      {
         DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][1], nLP);
      }

      if (nErrCode == DSL_SUCCESS)
      {
         if (nLP == DSL_LATENCY_IP_LP0)
         {
            pDataRateInterleaveThresholds->nDataRateThresholdUpshift =
               dataRateThresholds.data.nDataRateThresholdUpshift;
            pDataRateInterleaveThresholds->nDataRateThresholdDownshift =
               dataRateThresholds.data.nDataRateThresholdDownshift;
         }
         else
         {
            pDataRateFastThresholds->nDataRateThresholdUpshift =
               dataRateThresholds.data.nDataRateThresholdUpshift;
            pDataRateFastThresholds->nDataRateThresholdDownshift =
               dataRateThresholds.data.nDataRateThresholdDownshift;
         }
      }
#endif /* (DSL_CHANNELS_PER_LINE > 1)*/
   }
#endif /* INCLUDE_DSL_CONFIG_GET*/
   return nErrCode;
}   
#endif /* INCLUDE_DSL_G997_ALARM*/

DSL_Error_t DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntryGet(
   DSL_Context_t *pContext,
   adslLineAlarmConfProfileEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
#if defined (INCLUDE_DSL_PM) && defined (INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS) && \
       defined (INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)
   DSL_PM_LineSecThreshold_t lineSecThresholds;
#endif
#ifdef INCLUDE_DSL_G997_ALARM
   DSL_G997_ChannelDataRateThresholdData_t dataRateIntlvThresholds;
   DSL_G997_ChannelDataRateThresholdData_t dataRateFastThresholds;
#endif /* INCLUDE_DSL_G997_ALARM*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   origFlags = pData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   strncpy((DSL_char_t *)&pData->adslLineAlarmConfProfileName,
      (DSL_char_t *)&alarmConfProfile.adslLineAlarmConfProfileName, 32);

#if defined (INCLUDE_DSL_PM) && defined (INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS) && \
       defined (INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)
   memset(&lineSecThresholds, 0, sizeof(DSL_PM_LineSecThreshold_t));

   lineSecThresholds.nDirection = DSL_NEAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_LOFS_FLAG))
      {
         pData->adslAturThresh15MinLofs = lineSecThresholds.data.nLOFS;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_LOFS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_LOSS_FLAG))
      {
         pData->adslAturThresh15MinLoss = lineSecThresholds.data.nLOSS;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_LOSS_FLAG);
      }
 
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_ESS_FLAG))
      {
         pData->adslAturThresh15MinESs = lineSecThresholds.data.nES;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_LPRS_FLAG))
      {
         /* Not supported*/
      }
   }

   lineSecThresholds.nDirection = DSL_FAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_LOFS_FLAG))
      {
         pData->adslAtucThresh15MinLofs = lineSecThresholds.data.nLOFS;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_LOFS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_LOSS_FLAG))
      {
         pData->adslAtucThresh15MinLoss = lineSecThresholds.data.nLOSS;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_LOSS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG))
      {
         pData->adslAtucThresh15MinESs = lineSecThresholds.data.nES;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG);
      }
   }
#endif /* #ifdef defined (INCLUDE_DSL_PM) && defined (INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS) &&
                 defined (INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS) */

#ifdef INCLUDE_DSL_G997_ALARM
   nErrCode = DSL_DRV_MIB_ADSL_LineAlarmDataRateGet(pContext, DSL_DOWNSTREAM,
                 &dataRateIntlvThresholds, &dataRateFastThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_FAST_RATEUP_FLAG))
      {
         pData->adslAtucThreshFastRateUp =
            dataRateFastThresholds.nDataRateThresholdUpshift;
         CLR_FLAG(pData->flags, ATUC_THRESH_FAST_RATEUP_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_FAST_RATEDOWN_FLAG))
      {
         pData->adslAtucThreshFastRateDown =
            dataRateFastThresholds.nDataRateThresholdDownshift;
         CLR_FLAG(pData->flags, ATUC_THRESH_FAST_RATEDOWN_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_INTERLEAVE_RATEUP_FLAG))
      {
         pData->adslAtucThreshInterleaveRateUp =
            dataRateIntlvThresholds.nDataRateThresholdUpshift;
         CLR_FLAG(pData->flags, ATUC_THRESH_INTERLEAVE_RATEUP_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_INTERLEAVE_RATEDOWN_FLAG))
      {
         pData->adslAtucThreshInterleaveRateDown =
            dataRateIntlvThresholds.nDataRateThresholdDownshift;
         CLR_FLAG(pData->flags, ATUC_THRESH_INTERLEAVE_RATEDOWN_FLAG);
      }
   }

   if (IS_FLAG_SET(pData->flags, ATUC_INIT_FAILURE_TRAP_ENABLE_FLAG))
   {
      pData->adslAtucInitFailureTrapEnable =
         alarmConfProfile.adslAtucInitFailureTrapEnable;
      CLR_FLAG(pData->flags, ATUC_INIT_FAILURE_TRAP_ENABLE_FLAG);
   }

   nErrCode = DSL_DRV_MIB_ADSL_LineAlarmDataRateGet(pContext, DSL_UPSTREAM,
                 &dataRateIntlvThresholds, &dataRateFastThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_FAST_RATEUP_FLAG))
      {
         pData->adslAtucThreshFastRateUp =
            dataRateFastThresholds.nDataRateThresholdUpshift;
         CLR_FLAG(pData->flags, ATUR_THRESH_FAST_RATEUP_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_FAST_RATEDOWN_FLAG))
      {
         pData->adslAtucThreshFastRateDown =
            dataRateFastThresholds.nDataRateThresholdDownshift;
         CLR_FLAG(pData->flags, ATUR_THRESH_FAST_RATEDOWN_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_INTERLEAVE_RATEUP_FLAG))
      {
         pData->adslAtucThreshInterleaveRateUp =
            dataRateIntlvThresholds.nDataRateThresholdUpshift;
         CLR_FLAG(pData->flags, ATUR_THRESH_INTERLEAVE_RATEUP_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_INTERLEAVE_RATEDOWN_FLAG))
      {
         pData->adslAtucThreshInterleaveRateDown =
            dataRateIntlvThresholds.nDataRateThresholdDownshift;
         CLR_FLAG(pData->flags, ATUR_THRESH_INTERLEAVE_RATEDOWN_FLAG);
      }
   }

#endif /* INCLUDE_DSL_G997_ALARM*/

   if (IS_FLAG_SET(pData->flags, LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG))
   {
      pData->adslLineAlarmConfProfileRowStatus =
         alarmConfProfile.adslLineAlarmConfProfileRowStatus;
      CLR_FLAG(pData->flags, LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG);
   }

   handledFlags = pData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 ((((DSL_uint32_t)(LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG)) - 1) |
                    LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_G997_ALARM
static DSL_Error_t DSL_DRV_MIB_ADSL_LineAlarmDataRateSet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_G997_ChannelDataRateThresholdData_t *pDataRateInterleaveThresholds,
   DSL_G997_ChannelDataRateThresholdData_t *pDataRateFastThresholds)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_ChannelDataRateThreshold_t dataRateThresholds;
   DSL_LatencyPath_t nLP;

   dataRateThresholds.nDirection = nDirection;
   dataRateThresholds.nChannel = 0;

   DSL_CHECK_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][0], nLP);

   if (nLP == DSL_LATENCY_IP_LP0)
   {
      dataRateThresholds.data.nDataRateThresholdUpshift =
         pDataRateInterleaveThresholds->nDataRateThresholdUpshift;
      dataRateThresholds.data.nDataRateThresholdDownshift =
         pDataRateInterleaveThresholds->nDataRateThresholdDownshift;
   }
   else
   {
      dataRateThresholds.data.nDataRateThresholdUpshift =
         pDataRateFastThresholds->nDataRateThresholdUpshift;
      dataRateThresholds.data.nDataRateThresholdDownshift =
         pDataRateFastThresholds->nDataRateThresholdDownshift;
   }

   nErrCode = DSL_DRV_G997_ChannelDataRateThresholdConfigSet(
      pContext, &dataRateThresholds);

#if (DSL_CHANNELS_PER_LINE > 1)
   if (nErrCode == DSL_SUCCESS)
   {
      dataRateThresholds.nChannel = 1;
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLPath[nDirection][1], nLP);

      if (nLP == DSL_LATENCY_IP_LP0)
      {
         dataRateThresholds.data.nDataRateThresholdUpshift =
            pDataRateInterleaveThresholds->nDataRateThresholdUpshift;
         dataRateThresholds.data.nDataRateThresholdDownshift =
            pDataRateInterleaveThresholds->nDataRateThresholdDownshift;
      }
      else
      {
         dataRateThresholds.data.nDataRateThresholdUpshift =
            pDataRateFastThresholds->nDataRateThresholdUpshift;
         dataRateThresholds.data.nDataRateThresholdDownshift =
            pDataRateFastThresholds->nDataRateThresholdDownshift;
      }

      nErrCode = DSL_DRV_G997_ChannelDataRateThresholdConfigSet(
         pContext, &dataRateThresholds);
   }
#endif /* #if (DSL_CHANNELS_PER_LINE > 1)*/

   return nErrCode;
}   
#endif /* INCLUDE_DSL_G997_ALARM*/

DSL_Error_t DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntrySet(
   DSL_Context_t *pContext,
   adslLineAlarmConfProfileEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   DSL_boolean_t bSet = DSL_FALSE;
#if defined (INCLUDE_DSL_PM) && defined (INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS) && \
       defined (INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)
   DSL_PM_LineSecThreshold_t lineSecThresholds;
#endif
#ifdef INCLUDE_DSL_G997_ALARM
   DSL_G997_ChannelDataRateThresholdData_t dataRateIntlvThresholds;
   DSL_G997_ChannelDataRateThresholdData_t dataRateFastThresholds;
#endif /* INCLUDE_DSL_G997_ALARM*/

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntrySet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   origFlags = pData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   strncpy((DSL_char_t*)&alarmConfProfile.adslLineAlarmConfProfileName,
      (DSL_char_t*)&pData->adslLineAlarmConfProfileName, 32);

#if defined (INCLUDE_DSL_PM) && defined (INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS) && \
    defined (INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)
   memset(&lineSecThresholds, 0x0, sizeof(DSL_PM_LineSecThreshold_t));
   lineSecThresholds.nDirection = DSL_NEAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);
   
   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_LOFS_FLAG))
      {
         lineSecThresholds.data.nLOFS = (DSL_uint32_t)pData->adslAturThresh15MinLofs;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_LOFS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_LOSS_FLAG))
      {
         lineSecThresholds.data.nLOSS = (DSL_uint32_t)pData->adslAturThresh15MinLoss;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_LOSS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_ESS_FLAG))
      {
         lineSecThresholds.data.nES = (DSL_uint32_t)pData->adslAturThresh15MinESs;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_LPRS_FLAG))
      {
         /* Not supported*/
      }
   }

   if (bSet == DSL_TRUE)
   {
      nErrCode = DSL_DRV_PM_LineSecThresholds15MinSet(pContext, &lineSecThresholds);
      bSet = DSL_FALSE;
   }

   memset(&lineSecThresholds, 0x0, sizeof(DSL_PM_LineSecThreshold_t));
   lineSecThresholds.nDirection = DSL_FAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_LOFS_FLAG))
      {
         lineSecThresholds.data.nLOFS = (DSL_uint32_t)pData->adslAtucThresh15MinLofs;
         bSet = DSL_TRUE;
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_LOSS_FLAG))
      {
         lineSecThresholds.data.nLOSS = (DSL_uint32_t)pData->adslAtucThresh15MinLoss;
         bSet = DSL_TRUE;
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG))
      {
         lineSecThresholds.data.nES = (DSL_uint32_t)pData->adslAtucThresh15MinESs;
         bSet = DSL_TRUE;
      }
   }

   if (bSet == DSL_TRUE)
   {
      nErrCode = DSL_DRV_PM_LineSecThresholds15MinSet(pContext, &lineSecThresholds);
      bSet = DSL_FALSE;

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_LOFS_FLAG))
      {
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_LOFS_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_LOSS_FLAG))
      {
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_LOSS_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG))
      {
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_ESS_FLAG);
      }
   }
#endif /* defined (INCLUDE_DSL_PM) && defined (INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS) && \
          defined (INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)*/

#ifdef INCLUDE_DSL_G997_ALARM
   nErrCode = DSL_DRV_MIB_ADSL_LineAlarmDataRateGet(pContext, DSL_DOWNSTREAM,
                 &dataRateIntlvThresholds, &dataRateFastThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_FAST_RATEUP_FLAG))
      {
         dataRateFastThresholds.nDataRateThresholdUpshift =
            pData->adslAtucThreshFastRateUp;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_FAST_RATEUP_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_FAST_RATEDOWN_FLAG))
      {
         dataRateFastThresholds.nDataRateThresholdDownshift =
            pData->adslAtucThreshFastRateDown;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_FAST_RATEDOWN_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_INTERLEAVE_RATEUP_FLAG))
      {
         dataRateIntlvThresholds.nDataRateThresholdUpshift =
            pData->adslAtucThreshInterleaveRateUp;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_INTERLEAVE_RATEUP_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_INTERLEAVE_RATEDOWN_FLAG))
      {
         dataRateIntlvThresholds.nDataRateThresholdDownshift =
            pData->adslAtucThreshInterleaveRateDown;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_INTERLEAVE_RATEDOWN_FLAG);
      }

      if (bSet == DSL_TRUE)
      {
         nErrCode = DSL_DRV_MIB_ADSL_LineAlarmDataRateSet(pContext, DSL_DOWNSTREAM,
                       &dataRateIntlvThresholds, &dataRateFastThresholds);
         bSet = DSL_FALSE;
      }
   }

   if (IS_FLAG_SET(pData->flags, ATUC_INIT_FAILURE_TRAP_ENABLE_FLAG))
   {
      alarmConfProfile.adslAtucInitFailureTrapEnable =
         (DSL_boolean_t)pData->adslAtucInitFailureTrapEnable;
      CLR_FLAG(pData->flags, ATUC_INIT_FAILURE_TRAP_ENABLE_FLAG);
   }

   nErrCode = DSL_DRV_MIB_ADSL_LineAlarmDataRateGet(pContext, DSL_UPSTREAM,
                 &dataRateIntlvThresholds, &dataRateFastThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_FAST_RATEUP_FLAG))
      {
         dataRateFastThresholds.nDataRateThresholdUpshift =
            pData->adslAtucThreshFastRateUp;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_FAST_RATEUP_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_FAST_RATEDOWN_FLAG))
      {
         dataRateFastThresholds.nDataRateThresholdDownshift =
            pData->adslAtucThreshFastRateDown;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_FAST_RATEDOWN_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_INTERLEAVE_RATEUP_FLAG))
      {
         dataRateIntlvThresholds.nDataRateThresholdUpshift =
            pData->adslAtucThreshInterleaveRateUp;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_INTERLEAVE_RATEUP_FLAG);
      }

      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_INTERLEAVE_RATEDOWN_FLAG))
      {
         dataRateIntlvThresholds.nDataRateThresholdDownshift =
            pData->adslAtucThreshInterleaveRateDown;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_INTERLEAVE_RATEDOWN_FLAG);
      }

      if (bSet == DSL_TRUE)
      {
         nErrCode = DSL_DRV_MIB_ADSL_LineAlarmDataRateSet(pContext, DSL_UPSTREAM,
            &dataRateIntlvThresholds, &dataRateFastThresholds);
         bSet = DSL_FALSE;
      }
   }
#endif /* INCLUDE_DSL_G997_ALARM*/

   if (IS_FLAG_SET(pData->flags, LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG))
   {
      alarmConfProfile.adslLineAlarmConfProfileRowStatus =
         pData->adslLineAlarmConfProfileRowStatus;
      CLR_FLAG(pData->flags, LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG);
   }

   handledFlags = pData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 ((((DSL_uint32_t)(LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG)) - 1) |
                    LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntrySet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_TrapsGet(
   DSL_Context_t *pContext,
   adslAturTrapsFlags_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_MIB_ADSL_Thresholds_t nFlags;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_TrapsGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds, nFlags);

   *pData = (adslAturTrapsFlags_t)(nFlags & DSL_MIB_TRAPS_MASK_ADSL);
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
      nFlags & ~DSL_MIB_TRAPS_MASK_ADSL);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_TrapsGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_ADSL_MIB_RFC3440
DSL_Error_t DSL_DRV_MIB_ADSL_LineExtEntryGet(
   DSL_Context_t *pContext,
   adslLineExtTableEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   DSL_G997_LineInventory_t lineInventory;
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
   DSL_G997_XTUSystemEnabling_t xtseStatus;
   DSL_G997_PowerManagementStatus_t powerManagementStatus;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_LineExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   origFlags = pData->flags;
   
   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   if (origFlags & (ATUC_LINE_TRANS_CAP_FLAG))
   {
      memset(&lineInventory, 0x0, sizeof(DSL_G997_LineInventory_t));

      lineInventory.nDirection = DSL_FAR_END;

      /* Get Far-End line inventory*/
      nErrCode = DSL_DRV_G997_LineInventoryGet(pContext, &lineInventory);

      if (nErrCode >= DSL_SUCCESS)
      {
         if (IS_FLAG_SET(pData->flags, ATUC_LINE_TRANS_CAP_FLAG))
         {
            memcpy(&pData->adslLineTransAtucCap,
               &(lineInventory.data.XTSECapabilities[0]),
               sizeof(pData->adslLineTransAtucCap));
            CLR_FLAG(pData->flags, ATUC_LINE_TRANS_CAP_FLAG);
         }

         if (IS_FLAG_SET(pData->flags, ATUC_LINE_TRANS_CONFIG_FLAG))
         {
            /* Not Supported. It is not possible to get the ATU-C XTSE
               configuration at the ATU-R side*/
         }
      }
   }
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

   if (IS_FLAG_SET(pData->flags, ATUC_LINE_TRANS_ACTUAL_FLAG))
   {
      memset(&xtseStatus, 0x0, sizeof(DSL_G997_XTUSystemEnabling_t));
     
      nErrCode = DSL_DRV_G997_XTUSystemEnablingStatusGet(pContext, &xtseStatus);

      if (nErrCode >= DSL_SUCCESS)
      {
         memcpy(&pData->adslLineTransAtucActual,
            &(xtseStatus.data.XTSE[0]),
            sizeof(pData->adslLineTransAtucActual));
         CLR_FLAG(pData->flags, ATUC_LINE_TRANS_ACTUAL_FLAG);
      }
   }

   if (IS_FLAG_SET(pData->flags, LINE_GLITE_POWER_STATE_FLAG))
   {
      memset(&powerManagementStatus, 0x0, sizeof(DSL_G997_PowerManagementStatus_t));

      /* Get Power Management Status*/
      nErrCode = DSL_DRV_G997_PowerManagementStatusGet(pContext, &powerManagementStatus);

      if (nErrCode >= DSL_SUCCESS)
      {
         switch(powerManagementStatus.data.nPowerManagementStatus)
         {
            case DSL_G997_PMS_L0:
               pData->adslLineGlitePowerState = 2;
               break;
            case DSL_G997_PMS_L1:
               pData->adslLineGlitePowerState = 3;
               break;
            case DSL_G997_PMS_L3:
               pData->adslLineGlitePowerState = 4;
               break;
            default:
               pData->adslLineGlitePowerState = 1;
               break;
         }
         CLR_FLAG(pData->flags, LINE_GLITE_POWER_STATE_FLAG);
      }
   }

   handledFlags = pData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 ((((DSL_uint32_t)(LINE_GLITE_POWER_STATE_FLAG)) - 1) |
                    LINE_GLITE_POWER_STATE_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_LineExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_LineExtEntrySet(
   DSL_Context_t *pContext,
   adslLineExtTableEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_LineExtEntrySet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   origFlags = pData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   if (IS_FLAG_SET(pData->flags, ATUC_LINE_TRANS_CAP_FLAG))
   {
      /* Not supported*/
   }

   if (IS_FLAG_SET(pData->flags, ATUC_LINE_TRANS_CONFIG_FLAG))
   {
      /* Not supported*/
   }

   if (IS_FLAG_SET(pData->flags, ATUC_LINE_TRANS_ACTUAL_FLAG))
   {
      /* Not supported*/
   }

   if (IS_FLAG_SET(pData->flags, LINE_GLITE_POWER_STATE_FLAG))
   {
      /* Not supported*/
   }

   handledFlags = pData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 ((((DSL_uint32_t)(LINE_GLITE_POWER_STATE_FLAG)) - 1) | LINE_GLITE_POWER_STATE_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_LineExtEntrySet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_PM
#if defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
DSL_Error_t DSL_DRV_MIB_ADSL_PerfDataExtEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   aturPerfDataExtEntry_t *pAturData = (aturPerfDataExtEntry_t *)pData;
   atucPerfDataExtEntry_t *pAtucData = (atucPerfDataExtEntry_t *)pData;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_LineSecCounters_t lineSecData;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_PM_LineSecCountersTotal_t lineSecTotData;
#endif

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_PerfDataExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   memset(&lineSecData, 0x0, sizeof(DSL_PM_LineSecCounters_t));
      
   lineSecData.nHistoryInterval = 0;
   lineSecData.nDirection = nDirection;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   memset(&lineSecTotData, 0x0, sizeof(DSL_PM_LineSecCountersTotal_t));   
   lineSecTotData.nDirection = nDirection;
#endif

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   if ( ((origFlags & (ATUR_PERF_STAT_SESL_FLAG | ATUR_PERF_STAT_UASL_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_STAT_SESL_FLAG | ATUC_PERF_STAT_UASL_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_LineSecCountersTotalGet(pContext, &lineSecTotData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_STAT_SESL_FLAG))
            {
               pAturData->adslAturPerfStatSesL =
                  lineSecTotData.data.nSES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_STAT_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_STAT_UASL_FLAG))
            {
               pAturData->adslAturPerfStatUasL =
                  lineSecTotData.data.nUAS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_STAT_UASL_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_STAT_SESL_FLAG))
            {
               pAtucData->adslAtucPerfStatSesL =
                  lineSecTotData.data.nSES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_STAT_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_STAT_UASL_FLAG))
            {
               pAtucData->adslAtucPerfStatUasL =
                  lineSecTotData.data.nUAS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_STAT_UASL_FLAG);
            }
         }
      }
   }
   
   if ( ((origFlags & (ATUC_PERF_STAT_FASTR_FLAG | ATUC_PERF_STAT_FAILED_FASTR_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_STAT_FASTR_FLAG))
      {
         /* Not supported*/
      }
      
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_STAT_FAILED_FASTR_FLAG))
      {
         /* Not supported*/
      }
   }   
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/


#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if ( ((origFlags & (ATUR_PERF_CURR_15MIN_SESL_FLAG | ATUR_PERF_CURR_15MIN_UASL_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_CURR_15MIN_SESL_FLAG | ATUC_PERF_CURR_15MIN_UASL_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_LineSecCounters15MinGet(pContext, &lineSecData);   
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_SESL_FLAG))
            {
               pAturData->adslAturPerfCurr15MinSesL =
                  lineSecData.data.nSES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_15MIN_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_15MIN_UASL_FLAG))
            {
               pAturData->adslAturPerfCurr15MinUasL =
                  lineSecData.data.nUAS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_15MIN_UASL_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_SESL_FLAG))
            {
               pAtucData->adslAtucPerfCurr15MinSesL =
                  lineSecData.data.nSES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_15MIN_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_UASL_FLAG))
            {
               pAtucData->adslAtucPerfCurr15MinUasL =
                  lineSecData.data.nUAS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_15MIN_UASL_FLAG);
            }
         }
      }
   }

   if ( ((origFlags & (ATUC_PERF_CURR_15MIN_FASTR_FLAG | ATUC_PERF_CURR_15MIN_FAILED_FASTR_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_FASTR_FLAG))
      {
         /* Not supported*/
      }
      
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_15MIN_FAILED_FASTR_FLAG))
      {
         /* Not supported*/
      }
   }

   if ( ((origFlags & (ATUR_PERF_CURR_1DAY_SESL_FLAG | ATUR_PERF_CURR_1DAY_UASL_FLAG))
         && (nDirection == DSL_NEAR_END)) ||
        ((origFlags & (ATUC_PERF_CURR_1DAY_SESL_FLAG | ATUC_PERF_CURR_1DAY_UASL_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      nErrCode = DSL_DRV_PM_LineSecCounters1DayGet(pContext, &lineSecData);   
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_SESL_FLAG))
            {
               pAturData->adslAturPerfCurr1DaySesL =
                  lineSecData.data.nSES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_1DAY_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_CURR_1DAY_UASL_FLAG))
            {
               pAturData->adslAturPerfCurr1DayUasL =
                  lineSecData.data.nUAS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_CURR_1DAY_UASL_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_SESL_FLAG))
            {
               pAtucData->adslAtucPerfCurr1DaySesL =
                  lineSecData.data.nSES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_1DAY_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_UASL_FLAG))
            {
               pAtucData->adslAtucPerfCurr1DayUasL =
                  lineSecData.data.nUAS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_CURR_1DAY_UASL_FLAG);
            }
         }
      }

   }

   if ( ((origFlags & (ATUC_PERF_CURR_1DAY_FASTR_FLAG | ATUC_PERF_CURR_1DAY_FAILED_FASTR_FLAG))
         && (nDirection == DSL_FAR_END)))
   {
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_FASTR_FLAG))
      {
         /* Not supported*/
      }
      
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_CURR_1DAY_FAILED_FASTR_FLAG))
      {
         /* Not supported*/
      }
   }

   if ( ((origFlags & (ATUR_PERF_PREV_1DAY_SESL_FLAG | ATUR_PERF_PREV_1DAY_UASL_FLAG))
      && (nDirection == DSL_NEAR_END)) ||
     ((origFlags & (ATUC_PERF_PREV_1DAY_SESL_FLAG | ATUC_PERF_PREV_1DAY_UASL_FLAG))
      && (nDirection == DSL_FAR_END)))
   {
      lineSecData.nHistoryInterval = 1;
      nErrCode = DSL_DRV_PM_LineSecCounters1DayGet(pContext, &lineSecData);
      if (nErrCode >= DSL_SUCCESS)
      {
         if (nDirection == DSL_NEAR_END)
         {
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_SESL_FLAG))
            {
               pAturData->adslAturPerfPrev1DaySesL =
                  lineSecData.data.nSES;
               CLR_FLAG(pAturData->flags, ATUR_PERF_PREV_1DAY_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAturData->flags, ATUR_PERF_PREV_1DAY_UASL_FLAG))
            {
               pAturData->adslAturPerfPrev1DayUasL =
                  lineSecData.data.nUAS;
               CLR_FLAG(pAturData->flags, ATUR_PERF_PREV_1DAY_UASL_FLAG);
            }
         }
         else
         {
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_SESL_FLAG))
            {
               pAtucData->adslAtucPerfPrev1DaySesL =
                  lineSecData.data.nSES;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_PREV_1DAY_SESL_FLAG);
            }
            
            if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_UASL_FLAG))
            {
               pAtucData->adslAtucPerfPrev1DayUasL =
                  lineSecData.data.nUAS;
               CLR_FLAG(pAtucData->flags, ATUC_PERF_PREV_1DAY_UASL_FLAG);
            }
         }
      }
   }

   if ( ((origFlags & (ATUC_PERF_PREV_1DAY_FASTR_FLAG | ATUC_PERF_PREV_1DAY_FAILED_FASTR_FLAG))
      && (nDirection == DSL_FAR_END)))
   {
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_FASTR_FLAG))
      {
      }
      
      if (IS_FLAG_SET(pAtucData->flags, ATUC_PERF_PREV_1DAY_FAILED_FASTR_FLAG))
      {
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/


   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 (nDirection == DSL_NEAR_END) ?
                 ((((DSL_uint32_t)(ATUR_PERF_PREV_1DAY_UASL_FLAG)) - 1) | ATUR_PERF_PREV_1DAY_UASL_FLAG):
                 ((((DSL_uint32_t)(ATUC_PERF_PREV_1DAY_UASL_FLAG)) - 1) | ATUC_PERF_PREV_1DAY_UASL_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_PerfDataExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/

#if defined(INCLUDE_DSL_PM)
#if defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
DSL_Error_t DSL_DRV_MIB_ADSL_AtucPerfDataExtEntryGet(
   DSL_Context_t *pContext,
   atucPerfDataExtEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucPerfDataExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_PerfDataExtEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucPerfDataExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturPerfDataExtEntryGet(
   DSL_Context_t *pContext,
   aturPerfDataExtEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturPerfDataExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_PerfDataExtEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturPerfDataExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/
#endif /* defined(INCLUDE_DSL_PM)*/

#if defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_HISTORY) && defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
DSL_Error_t DSL_DRV_MIB_ADSL_IntervalExtEntryGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_void_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   adslAturInvtlExtInfo_t *pAturData = (adslAturInvtlExtInfo_t *)pData;
   adslAtucInvtlExtInfo_t *pAtucData = (adslAtucInvtlExtInfo_t *)pData;

   DSL_PM_LineSecCounters_t lineSecData;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_PerfDataExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Save original flags*/
   origFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   memset(&lineSecData, 0x0, sizeof(DSL_PM_LineSecCounters_t));

   if (nDirection == DSL_FAR_END)
   {
      lineSecData.nHistoryInterval = (DSL_uint32_t)pAturData->IntervalNumber;
   }
   else if (nDirection == DSL_NEAR_END)
   {
      lineSecData.nHistoryInterval = (DSL_uint32_t)pAtucData->IntervalNumber;
   }
   lineSecData.nDirection = nDirection;

   nErrCode = DSL_DRV_PM_LineSecCounters15MinGet(pContext, &lineSecData);
   if (nErrCode >= DSL_SUCCESS)
   {
      if (nDirection == DSL_NEAR_END)
      {
         if (IS_FLAG_SET(pAturData->flags, ATUR_INTERVAL_SESL_FLAG))
         {
            pAturData->adslAturIntervalSesL =
               lineSecData.data.nSES;
            CLR_FLAG(pAturData->flags, ATUR_INTERVAL_SESL_FLAG);
         }
         
         if (IS_FLAG_SET(pAturData->flags, ATUR_INTERVAL_UASL_FLAG))
         {
            pAturData->adslAturIntervalUasL =
               lineSecData.data.nUAS;
            CLR_FLAG(pAturData->flags, ATUR_INTERVAL_UASL_FLAG);
         }
      }
      else
      {
         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTERVAL_SESL_FLAG))
         {
            pAtucData->adslAtucIntervalSesL =
               lineSecData.data.nSES;
            CLR_FLAG(pAtucData->flags, ATUC_INTERVAL_SESL_FLAG);
         }
         
         if (IS_FLAG_SET(pAtucData->flags, ATUC_INTERVAL_UASL_FLAG))
         {
            pAtucData->adslAtucIntervalUasL =
               lineSecData.data.nUAS;
            CLR_FLAG(pAtucData->flags, ATUC_INTERVAL_UASL_FLAG);
         }
      }
   }
   
   if (nDirection == DSL_FAR_END)
   {
      if (IS_FLAG_SET(pAtucData->flags, ATUC_INTERVAL_FASTR_FLAG))
      {
         /* Not supported*/
      }
      
      if (IS_FLAG_SET(pAtucData->flags, ATUC_INTERVAL_FAILED_FASTR_FLAG))
      {
         /* Not supported*/
      }
   }


   handledFlags = (nDirection == DSL_NEAR_END) ? pAturData->flags : pAtucData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 (nDirection == DSL_NEAR_END) ?
                 ((((DSL_uint32_t)(ATUR_INTERVAL_UASL_FLAG)) - 1) | ATUR_INTERVAL_UASL_FLAG):
                 ((((DSL_uint32_t)(ATUC_INTERVAL_UASL_FLAG)) - 1) | ATUC_INTERVAL_UASL_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_IntervalExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AtucIntervalExtEntryGet(
   DSL_Context_t *pContext,
   adslAtucInvtlExtInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AtucIntervalExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_IntervalExtEntryGet(pContext, DSL_FAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AtucIntervalExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AturIntervalExtEntryGet(
   DSL_Context_t *pContext,
   adslAturInvtlExtInfo_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AturIntervalExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   nErrCode = DSL_DRV_MIB_ADSL_IntervalExtEntryGet(pContext, DSL_NEAR_END, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AturIntervalExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* defined(INCLUDE_DSL_PM) && defined(INCLUDE_DSL_CPE_PM_HISTORY) &&
          defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/

#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
DSL_Error_t DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntryGet(
   DSL_Context_t *pContext,
   adslLineAlarmConfProfileExtEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   DSL_PM_LineSecThreshold_t lineSecThresholds;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntryGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   origFlags = pData->flags;

   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   memset(&lineSecThresholds, 0x0, sizeof(DSL_PM_LineSecThreshold_t));

   strncpy((DSL_char_t *)&pData->adslLineAlarmConfProfileExtName,
      (DSL_char_t *)&alarmExtConfProfile.adslLineAlarmExtConfProfileName, 32);

   lineSecThresholds.nDirection = DSL_NEAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_SESL_FLAG))
      {
         pData->adslAturThreshold15MinSesL =
            lineSecThresholds.data.nSES;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_SESL_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_UASL_FLAG))
      {
         pData->adslAturThreshold15MinUasL =
            lineSecThresholds.data.nUAS;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_UASL_FLAG);
      }
   }

   if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_FAILED_FASTR_FLAG))
   {
      /* Not supported*/
   }

   lineSecThresholds.nDirection = DSL_FAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_SESL_FLAG))
      {
         pData->adslAtucThreshold15MinSesL =
            lineSecThresholds.data.nSES;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_SESL_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_UASL_FLAG))
      {
         pData->adslAtucThreshold15MinUasL =
            lineSecThresholds.data.nUAS;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_UASL_FLAG);
      }
   }

   handledFlags = pData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 ((((DSL_uint32_t)(ATUR_THRESH_15MIN_UASL_FLAG)) - 1) | ATUR_THRESH_15MIN_UASL_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntryGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntrySet(
   DSL_Context_t *pContext,
   adslLineAlarmConfProfileExtEntry_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t origFlags = 0, handledFlags = 0;
   DSL_PM_LineSecThreshold_t lineSecThresholds;
   DSL_boolean_t bSet = DSL_FALSE;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntrySet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   origFlags = pData->flags;
   
   if (origFlags == 0)
   {
      return DSL_ERR_NOT_SUPPORTED;
   }

   memset(&lineSecThresholds, 0x0, sizeof(DSL_PM_LineSecThreshold_t));
   lineSecThresholds.nDirection = DSL_FAR_END;

   strncpy((DSL_char_t*)&alarmExtConfProfile.adslLineAlarmExtConfProfileName,
      (DSL_char_t*)&pData->adslLineAlarmConfProfileExtName, 32);

   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_SESL_FLAG))
      {
         lineSecThresholds.data.nSES = pData->adslAtucThreshold15MinSesL;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_SESL_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_UASL_FLAG))
      {
         lineSecThresholds.data.nUAS = pData->adslAtucThreshold15MinUasL;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUC_THRESH_15MIN_UASL_FLAG);
      }
   }

   if (bSet == DSL_TRUE)
   {
      nErrCode = DSL_DRV_PM_LineSecThresholds15MinSet(pContext, &lineSecThresholds);
      bSet = DSL_FALSE;
   }

   if (IS_FLAG_SET(pData->flags, ATUC_THRESH_15MIN_FAILED_FASTR_FLAG))
   {
      /* Not supported*/
   }

   memset(&lineSecThresholds, 0x0, sizeof(DSL_PM_LineSecThreshold_t));
   lineSecThresholds.nDirection = DSL_NEAR_END;
   nErrCode = DSL_DRV_PM_LineSecThresholds15MinGet(pContext, &lineSecThresholds);

   if (nErrCode >= DSL_SUCCESS)
   {
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_SESL_FLAG))
      {
         lineSecThresholds.data.nSES = pData->adslAturThreshold15MinSesL;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_SESL_FLAG);
      }
      
      if (IS_FLAG_SET(pData->flags, ATUR_THRESH_15MIN_UASL_FLAG))
      {
         lineSecThresholds.data.nUAS = pData->adslAturThreshold15MinUasL;
         bSet |= DSL_TRUE;
         CLR_FLAG(pData->flags, ATUR_THRESH_15MIN_UASL_FLAG);
      }
   }

   if (bSet == DSL_TRUE)
   {
      nErrCode = DSL_DRV_PM_LineSecThresholds15MinSet(pContext, &lineSecThresholds);
   }

   handledFlags = pData->flags;

   nErrCode = DSL_DRV_MIB_ADSL_HandledFlagsCheckAndReturn(
                 ((((DSL_uint32_t)(ATUR_THRESH_15MIN_UASL_FLAG)) - 1) | ATUR_THRESH_15MIN_UASL_FLAG),
                 origFlags, handledFlags);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntrySet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_PM*/

DSL_Error_t DSL_DRV_MIB_ADSL_ExtTrapsGet(
   DSL_Context_t *pContext,
   adslAturExtTrapsFlags_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_MIB_ADSL_Thresholds_t nFlags;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL: IN - "
      "DSL_DRV_MIB_ADSL_ExtTrapsGet"DSL_DRV_CRLF));

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CTX_READ_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds, nFlags);

   *pData = (adslAturExtTrapsFlags_t)((nFlags & DSL_MIB_TRAPS_MASK_ADSL_EXT) >> 9);
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
      nFlags & ~DSL_MIB_TRAPS_MASK_ADSL_EXT);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL: OUT - DSL_DRV_MIB_ADSL_ExtTrapsGet, "
      "nReturn(%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_ADSL_MIB_RFC3440*/

static DSL_Error_t DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(
   DSL_Context_t *pContext,
   DSL_boolean_t bIsInKernel,
   DSL_IoctlHandlerHelperType_t nType,
   DSL_DRV_IoctlHandlerHelperFunc_t pFunc,
   DSL_void_t *pArg,
   DSL_uint32_t nArgSz)
{
   DSL_Error_t nErrCode = DSL_ERR_MEMORY;
   DSL_IOCTL_MIB_arg_t *pIOCTL_arg = DSL_NULL;

   if (nType != DSL_IOCTL_HELPER_GET && nType != DSL_IOCTL_HELPER_SET)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "Invalid parameter was passed "
         "to the IOCTL helper"DSL_DRV_CRLF));
      return DSL_ERR_INVALID_PARAMETER;
   }

   if (pArg == DSL_NULL || nArgSz == 0)
   {
      nErrCode = DSL_ERR_INVALID_PARAMETER;
   }
   else
   {
      pIOCTL_arg = DSL_DRV_VMalloc(nArgSz);
      if(pIOCTL_arg == DSL_NULL)
      {
         return nErrCode;
      }

      DSL_IoctlMemCpyFrom(bIsInKernel, pIOCTL_arg, pArg,
         nArgSz);

      nErrCode = pFunc(pContext, pIOCTL_arg);

      if (nErrCode >= DSL_SUCCESS)
      {
         if (nType == DSL_IOCTL_HELPER_GET)
         {
            DSL_IoctlMemCpyTo( bIsInKernel, pArg, pIOCTL_arg,
               nArgSz);
         }
      }

      DSL_DRV_VFree(pIOCTL_arg);
   }

   return nErrCode;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_mib.h'
*/
DSL_Error_t DSL_DRV_MIB_IoctlHandle(
   DSL_Context_t *pContext,
   DSL_boolean_t bIsInKernel,
   DSL_uint_t nCommand,
   DSL_uint32_t nArg)
{
   DSL_Error_t nErrCode = DSL_ERROR;
   DSL_IOCTL_MIB_arg_t *pIOCTL_arg = DSL_NULL;

   pIOCTL_arg = (DSL_IOCTL_MIB_arg_t*)DSL_DRV_VMalloc(sizeof(DSL_IOCTL_MIB_arg_t));
   if(pIOCTL_arg == DSL_NULL)
   {
      DSL_IoctlMemCpyTo( bIsInKernel, (DSL_void_t*)nArg, &nErrCode,
         sizeof(DSL_Error_t));
      return nErrCode;
   }

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "DSL: DSL_DRV_MIB_IoctlHandle: Call %d - "
      "(%s)"DSL_DRV_CRLF, nCommand, DSL_DBG_ADSL_IoctlName(nCommand)));

   switch (nCommand)
   {
   case DSL_FIO_MIB_ADSL_LINE_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_LineEntryGet,
         (DSL_void_t*) nArg, sizeof(adslLineTableEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucPhysEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAtucPhysEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturPhysEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAturPhysEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucChanEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAtucChanInfo_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturChanEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAturChanInfo_t));
      break;

#ifdef INCLUDE_DSL_PM
#if defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
   case DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucPerfDataEntryGet,
         (DSL_void_t*) nArg, sizeof(atucPerfDataEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturPerfDataEntryGet,
         (DSL_void_t*) nArg, sizeof(aturPerfDataEntry_t));
      break;
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS) && defined(INCLUDE_DSL_CPE_PM_HISTORY)
   case DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucIntervalEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAtucIntvlInfo_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturIntervalEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAturIntvlInfo_t));
      break;
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS) && defined(INCLUDE_DSL_CPE_PM_HISTORY)*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   case DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucChanPerfDataEntryGet,
         (DSL_void_t*) nArg, sizeof(atucChannelPerfDataEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturChanPerfDataEntryGet,
         (DSL_void_t*) nArg, sizeof(aturChannelPerfDataEntry_t));
      break;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   case DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucChanIntervalEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAtucChanIntvlInfo_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturChanIntervalEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAturChanIntvlInfo_t));
      break;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#endif /* INCLUDE_DSL_PM*/

   case DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntryGet,
         (DSL_void_t*) nArg, sizeof(adslLineAlarmConfProfileEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_SET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_SET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_LineAlarmConfProfileEntrySet,
         (DSL_void_t*) nArg, sizeof(adslLineAlarmConfProfileEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_TRAPS_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_TrapsGet,
         (DSL_void_t*) nArg, sizeof(adslAturTrapsFlags_t));
      break;

#ifdef INCLUDE_ADSL_MIB_RFC3440
   case DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_LineExtEntryGet,
         (DSL_void_t*) nArg, sizeof(adslLineExtTableEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_SET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_SET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_LineExtEntrySet,
         (DSL_void_t*) nArg, sizeof(adslLineExtTableEntry_t));
      break;

#ifdef INCLUDE_DSL_PM
#if defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
   case DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucPerfDataExtEntryGet,
         (DSL_void_t*) nArg, sizeof(atucPerfDataExtEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturPerfDataExtEntryGet,
         (DSL_void_t*) nArg, sizeof(aturPerfDataExtEntry_t));
      break;

#if defined(INCLUDE_DSL_CPE_PM_HISTORY)
   case DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AtucIntervalExtEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAtucInvtlExtInfo_t));
      break;

   case DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AturIntervalExtEntryGet,
         (DSL_void_t*) nArg, sizeof(adslAturInvtlExtInfo_t));
      break;
#endif /* defined(INCLUDE_DSL_CPE_PM_HISTORY)*/
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)*/ 

#if defined(INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS) && defined(INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS)
   case DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_GET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntryGet,
         (DSL_void_t*) nArg, sizeof(adslLineAlarmConfProfileExtEntry_t));
      break;

   case DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_SET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_SET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_AlarmConfProfileExtEntrySet,
         (DSL_void_t*) nArg, sizeof(adslLineAlarmConfProfileExtEntry_t));
      break;
#endif /* defined(INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS) && defined(INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS)*/
#endif /* #ifdef INCLUDE_DSL_PM*/
   case DSL_FIO_MIB_ADSL_EXT_TRAPS_GET:
      nErrCode = DSL_DRV_MIB_ADSL_IoctlHandleHelperCall(pContext, bIsInKernel,
         DSL_IOCTL_HELPER_SET,
         (DSL_DRV_IoctlHandlerHelperFunc_t)DSL_DRV_MIB_ADSL_ExtTrapsGet,
         (DSL_void_t*) nArg, sizeof(adslAturExtTrapsFlags_t));
      break;
#endif /* INCLUDE_ADSL_MIB_RFC3440 */

   default:
      nErrCode = DSL_ERR_NOT_SUPPORTED;
      break;
   }

   if (nErrCode >= DSL_SUCCESS)
   {
      /* ADSL MIB entries only valid in the ADSL1 mode*/
      if ((pContext->xtseCfg[0] & XTSE_1_03_A_1_NO)   ||
          (pContext->xtseCfg[0] & XTSE_1_01_A_T1_413) ||
          (pContext->xtseCfg[1] & XTSE_2_01_A_2_NO)   ||
          (pContext->xtseCfg[0] & XTSE_1_01_A_T1_413) || 
          (pContext->xtseCfg[0] & XTSE_1_05_B_1_NO))
      {
         if ((pContext->xtseCurr[2] & XTSE_3_03_A_3_NO) ||
             (pContext->xtseCurr[3] & XTSE_4_05_I_3_NO) ||
             (pContext->xtseCurr[4] & XTSE_5_03_L_3_NO) ||
             (pContext->xtseCurr[4] & XTSE_5_04_L_3_NO) ||
             (pContext->xtseCurr[4] & XTSE_5_07_M_3_NO) ||
             (pContext->xtseCurr[5] & XTSE_6_01_A_5_NO) ||
             (pContext->xtseCurr[5] & XTSE_6_07_I_5_NO) ||
             (pContext->xtseCurr[6] & XTSE_7_03_M_5_NO) ||
             (pContext->xtseCurr[2] & XTSE_3_05_B_3_NO) ||
             (pContext->xtseCurr[3] & XTSE_4_07_J_3_NO) ||
             (pContext->xtseCurr[5] & XTSE_6_03_B_5_NO) ||
             (pContext->xtseCurr[6] & XTSE_7_01_J_5_NO))
         {
            if (!(pContext->xtseCurr[0] & XTSE_1_03_A_1_NO)   &&
                !(pContext->xtseCurr[0] & XTSE_1_01_A_T1_413) &&
                !(pContext->xtseCurr[1] & XTSE_2_01_A_2_NO)   &&
                !(pContext->xtseCurr[0] & XTSE_1_01_A_T1_413) && 
                !(pContext->xtseCurr[0] & XTSE_1_05_B_1_NO))
            {
               nErrCode = DSL_WRN_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
            } 
         }
      }
      else
      {
         nErrCode = DSL_WRN_NOT_SUPPORTED_IN_CURRENT_ADSL_MODE_OR_ANNEX;
      }
   }

   DSL_DRV_VFree(pIOCTL_arg);

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "DSL_DRV_MIB_IoctlHandle - "
      "return(from %d - %s) %d"DSL_DRV_CRLF, nCommand, DSL_DBG_ADSL_IoctlName(nCommand),
      nErrCode));

   return nErrCode;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_mib.h'
*/
DSL_Error_t DSL_DRV_MIB_ModuleInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* Nothing to be done yet */
   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL_DRV_MIB_ModuleInit - "
      "Enter"DSL_DRV_CRLF));

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL_DRV_MIB_ModuleInit - "
      "Exit (%d)"DSL_DRV_CRLF, nErrCode));

   return nErrCode;
}

/** @} DRV_DSL_CPE_ADSL_MIB */

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DSL_ADSL_MIB*/
