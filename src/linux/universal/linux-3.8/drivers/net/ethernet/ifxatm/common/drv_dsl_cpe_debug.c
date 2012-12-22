/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"
#include "drv_dsl_cpe_debug.h"

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef DSL_DEBUG_DISABLE

/** \file
   Debug support
*/

/** \addtogroup DRV_DSL_CPE_DEBUG
 @{ */

/*
 * Two global variables to store local and global debug levels.
 */

/* Default initialization for all debug blocks */
DSL_debugLevelEntry_t DSL_g_dbgLvl[DSL_DBG_MAX_ENTRIES] =
{
   { DSL_DBG_NONE,   "DSL_DBG_NO_BLOCK"            },   /* 00 */
   { DSL_DBG_ERR,    "DSL_DBG_CPE_API"             },   /* 01 */
   { DSL_DBG_ERR,    "DSL_DBG_G997"                },   /* 02 */
   { DSL_DBG_ERR,    "DSL_DBG_PM"                  },   /* 03 */
   { DSL_DBG_ERR,    "DSL_DBG_MIB"                 },   /* 04 */
   { DSL_DBG_ERR,    "DSL_DBG_CEOC"                },   /* 05 */
   { DSL_DBG_ERR,    "DSL_DBG_LED"                 },   /* 06 */
   { DSL_DBG_ERR,    "DSL_DBG_SAR"                 },   /* 07 */
   { DSL_DBG_ERR,    "DSL_DBG_DEVICE"              },   /* 08 */
   { DSL_DBG_ERR,    "DSL_DBG_AUTOBOOT_THREAD"     },   /* 09 */
   { DSL_DBG_ERR,    "DSL_DBG_OS"                  },   /* 10 */
   { DSL_DBG_ERR,    "DSL_DBG_CALLBACK"            },   /* 11 */
   { DSL_DBG_NONE,   "DSL_DBG_MESSAGE_DUMP"        },   /* 12 */
   { DSL_DBG_ERR,    "DSL_DBG_LOW_LEVEL_DRIVER"    },   /* 13 */
   { DSL_DBG_NONE,   "not used"                    },   /* 14 */
   { DSL_DBG_NONE,   "not used"                    },   /* 15 */
};

/* Initialization for all debug levels */
DSL_debugLevelEntry_t DSL_g_dbgLvlNames[] =
{
   { DSL_DBG_NONE,   "DSL_DBG_NONE"    },
   { DSL_DBG_PRN,    "DSL_DBG_PRN"     },
   { DSL_DBG_ERR,    "DSL_DBG_ERR"     },
   { DSL_DBG_WRN,    "DSL_DBG_WRN"     },
   { DSL_DBG_MSG,    "DSL_DBG_MSG"     },
   { DSL_DBG_LOCAL,  "DSL_DBG_LOCAL"   },
};

/**
   Definition of strings that reflects the direction.
   This array should be used with index +1 */
const DSL_char_t* DSL_DBG_PRN_DIR[] = { "NA", "US", "DS" };

/**
   Definition of strings that reflects the autoboot status.
   This array includes debug information that will be used to display the states
   that are defined within \ref DSL_AutobootStatGet_t enum as plain text */
const DSL_char_t* DSL_DBG_PRN_AUTOBOOT_STATUS[] =
{
   "Stopped",
   "Starting",
   "Running",
   "FwWait",
   "ConfigWriteWait",
   "LinkActivateWait",
   "RestartWait"
};

/**
   Definition of strings that reflects the autoboot state.
   This array includes debug information that will be used to display the states
   that are defined within \ref DSL_AutobootStatGet_t enum as plain text */
const DSL_char_t* DSL_DBG_PRN_AUTOBOOT_STATE[] =
{
   "Unknown",
   "FirmwareRequest",
   "FirmwareWait",
   "firmwareReady",
   "L3",
   "Init",
   "Train",
   "Showtime",
   "Exception",
   "Diagnostic",
   "Restart",
   "ConfigWriteWait",
   "LinkActivateWait",
   "RestartWait"
};


const DSL_uint8_t DSL_g_dbgLvlNumber = sizeof(DSL_g_dbgLvlNames) / sizeof(DSL_debugLevelEntry_t);

DSL_debugLevels_t DSL_g_globalDbgLvl = DSL_DBG_LOCAL;
DSL_uint16_t DSL_g_dbgStartLine = 0;
DSL_uint16_t DSL_g_dbgStopLine = DSL_MAX_LINE_NUMBER;

/* Array of line debug levels */
DSL_debugLevels_t DSL_g_dbgLineLvl[DSL_MAX_LINE_NUMBER];

DSL_char_t* DSL_DBG_IoctlName(DSL_uint_t nIoctlCode)
{
   switch (nIoctlCode)
   {
   case DSL_FIO_AUTOBOOT_CONTROL_SET:
      return "DSL_FIO_AUTOBOOT_CONTROL_SET";
   case DSL_FIO_AUTOBOOT_LOAD_FIRMWARE:
      return "DSL_FIO_AUTOBOOT_LOAD_FIRMWARE";
   case DSL_FIO_LINE_STATE_GET:
      return "DSL_FIO_LINE_STATE_GET";
   case DSL_FIO_LINE_FEATURE_CONFIG_SET:
      return "DSL_FIO_LINE_FEATURE_CONFIG_SET";
   case DSL_FIO_LINE_FEATURE_CONFIG_GET:
      return "DSL_FIO_LINE_FEATURE_CONFIG_GET";
   case DSL_FIO_LINE_FEATURE_STATUS_GET:
      return "DSL_FIO_LINE_FEATURE_STATUS_GET";
   case DSL_FIO_DBG_MODULE_LEVEL_SET:
      return "DSL_FIO_DBG_MODULE_LEVEL_SET";
   case DSL_FIO_DBG_MODULE_LEVEL_GET:
      return "DSL_FIO_DBG_MODULE_LEVEL_GET";
   case DSL_FIO_VERSION_INFORMATION_GET:
      return "DSL_FIO_VERSION_INFORMATION_GET";
   case DSL_FIO_TEST_MODE_CONTROL_SET:
      return "DSL_FIO_TEST_MODE_CONTROL_SET";
#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   case DSL_FIO_SHOWTIME_LOGGING_DATA_GET:
      return "DSL_FIO_SHOWTIME_LOGGING_DATA_GET";
#endif
   case DSL_FIO_DBG_DEVICE_MESSAGE_SEND:
      return "DSL_FIO_DBG_DEVICE_MESSAGE_SEND";
   case DSL_FIO_EVENT_STATUS_GET:
      return "DSL_FIO_EVENT_STATUS_GET";

   case DSL_FIO_G997_LINE_FAILURES_STATUS_GET:
      return "DSL_FIO_G997_LINE_FAILURES_STATUS_GET";
   case DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET:
      return "DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET";
   case DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_GET:
      return "DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_GET";
   case DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET:
      return "DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET";
   case DSL_FIO_G997_BIT_ALLOCATION_NSC_GET:
      return "DSL_FIO_G997_BIT_ALLOCATION_NSC_GET";
   case DSL_FIO_G997_SNR_ALLOCATION_NSC_GET:
      return "DSL_FIO_G997_SNR_ALLOCATION_NSC_GET";
   case DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET:
      return "DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET";
   case DSL_FIO_G997_LINE_INVENTORY_GET:
      return "DSL_FIO_G997_LINE_INVENTORY_GET";
   case DSL_FIO_G997_LINE_INVENTORY_SET:
      return "DSL_FIO_G997_LINE_INVENTORY_SET";
   case DSL_FIO_G997_LINE_STATUS_GET:
      return "DSL_FIO_G997_LINE_STATUS_GET";
   case DSL_FIO_G997_LINE_ACTIVATE_CONFIG_SET:
      return "DSL_FIO_G997_LINE_ACTIVATE_CONFIG_SET";
   case DSL_FIO_G997_LINE_ACTIVATE_CONFIG_GET:
      return "DSL_FIO_G997_LINE_ACTIVATE_CONFIG_GET";
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   case DSL_FIO_G997_LINE_STATUS_PER_BAND_GET:
      return "DSL_FIO_G997_LINE_STATUS_PER_BAND_GET";
#endif
   case DSL_FIO_G997_CHANNEL_STATUS_GET:
      return "DSL_FIO_G997_CHANNEL_STATUS_GET";
   case DSL_FIO_G997_DATA_PATH_FAILURES_STATUS_GET:
      return "DSL_FIO_G997_DATA_PATH_FAILURES_STATUS_GET";

#if defined(INCLUDE_DSL_PM)
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   case DSL_FIO_PM_CHANNEL_COUNTERS_15MIN_GET:
      return "DSL_FIO_PM_CHANNEL_COUNTERS_15MIN_GET";
   case DSL_FIO_PM_CHANNEL_COUNTERS_1DAY_GET:
      return "DSL_FIO_PM_CHANNEL_COUNTERS_1DAY_GET";
   case DSL_FIO_PM_CHANNEL_COUNTERS_TOTAL_GET:
      return "DSL_FIO_PM_CHANNEL_COUNTERS_TOTAL_GET";
#endif
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   case DSL_FIO_PM_DATA_PATH_COUNTERS_TOTAL_GET:
      return "DSL_FIO_PM_DATA_PATH_COUNTERS_TOTAL_GET";
#endif
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   case DSL_FIO_PM_LINE_SEC_COUNTERS_15MIN_GET:
      return "DSL_FIO_PM_LINE_SEC_COUNTERS_15MIN_GET";
   case DSL_FIO_PM_LINE_SEC_COUNTERS_1DAY_GET:
      return "DSL_FIO_PM_LINE_SEC_COUNTERS_1DAY_GET";
   case DSL_FIO_PM_LINE_SEC_COUNTERS_TOTAL_GET:
      return "DSL_FIO_PM_LINE_SEC_COUNTERS_TOTAL_GET";
   case DSL_FIO_PM_LINE_INIT_COUNTERS_TOTAL_GET:
      return "DSL_FIO_PM_LINE_INIT_COUNTERS_TOTAL_GET";
   case DSL_FIO_PM_LINE_INIT_COUNTERS_15MIN_GET:
      return "DSL_FIO_PM_LINE_INIT_COUNTERS_15MIN_GET";
   case DSL_FIO_PM_LINE_INIT_COUNTERS_1DAY_GET:
      return "DSL_FIO_PM_LINE_INIT_COUNTERS_1DAY_GET";
#endif
#endif /* #if defined(INCLUDE_DSL_PM)*/
   default:
      return "<unknown>";
   }
}

/** @} DRV_DSL_CPE_DEBUG */

DSL_void_t DSL_DRV_ErrorSet(DSL_void_t *pContext, DSL_Error_t nCode)
{
   DSL_Context_t *pCtx = pContext;

   if (pContext == DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (DSL_NULL, "DSL DSL_DRV_ErrorSet: Context pointer is NULL!"DSL_DRV_CRLF));
         
      return;
   }

   pCtx->nErrNo = nCode;
}

#endif /* #ifndef DSL_DEBUG_DISABLE*/

#ifdef __cplusplus
}
#endif
