/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_BND_H
#define _DRV_DSL_CPE_API_BND_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   Bonding interface.
*/

/** \addtogroup DRV_DSL_CPE_BND
 @{ */


#include "drv_dsl_cpe_api.h"


/**
   Data structure used to set the PAF handshake control.
*/
typedef struct
{
   /**
   Enable/disable bonding
   DSL_FALSE - bonding is disabled (Default)
   DSL_TRUE  -  bonding is enabled */
   DSL_IN DSL_boolean_t bPafEnable;
} DSL_BND_ConfigData_t;

/**
   Structure for configuring bonding handshake operation.
   This structure has to be used for ioctl
   - \ref DSL_FIO_BND_CONFIG_SET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains Bonding config data */
   DSL_IN DSL_BND_ConfigData_t data;
} DSL_BND_ConfigSet_t;

/**
   Structure for configuring bonding handshake operation.
   This structure has to be used for ioctl
   - \ref DSL_FIO_BND_CONFIG_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains Bonding config data */
   DSL_OUT DSL_BND_ConfigData_t data;
} DSL_BND_ConfigGet_t;

/**
   Defines possible values that signals the status whether the bonding feature
   is enabled on the far end side.
*/
typedef enum
{
   /**
   Default value for initialization (not updated/initialized so far) */
   DSL_BND_ENABLE_NOT_INITIALIZED = -1,
   /**
   Bonding is disabled at the far end side. */
   DSL_BND_ENABLE_OFF = 0,
   /**
   Bonding is enable at the far end side. */
   DSL_BND_ENABLE_ON = 1,
} DSL_BND_RemotePafSupported_t;

/**
   This type enumerates the Discovery or Aggregation activation modes.
*/
typedef enum
{
   /**
   No command */
   DSL_BND_NO_COMMAND = 0,
   /**
   Discovery set if clear */
   DSL_BND_DISCOVERY_SET_IF_CLEAR = 1,
   /**
   Discovery clear if same */
   DSL_BND_DISCOVERY_CLEAR_IF_SAME = 2,
   /**
   Aggregate set */
   DSL_BND_AGGREGATE_SET = 4,
   /**
   Aggregate clear */
   DSL_BND_AGGREGATE_CLR = 8,
} DSL_BND_ActivationMode_t;

/**
   Structure to retrieve bonding handshake status.
*/
typedef struct
{
   /**
   Indicates whether the bonding feature is enabled at the far end side. */
   DSL_OUT DSL_BND_RemotePafSupported_t nRemotePafSupported;
   /**
   Indicates the type of Aggregate or Discovery command received */
   DSL_OUT DSL_BND_ActivationMode_t nActivationMode;
   /**
   Discovery Code received in initial CL during "Set if Clear"
   or "Clear if Same" exchange. */
   DSL_OUT DSL_uint8_t nDiscoveryCode[6];
   /**
   Aggregate Data received in initial CL during "Set" command. */
   DSL_OUT DSL_uint32_t nAggregateData;
} DSL_BND_HsStatusData_t;

/**
   Structure to retrieve bonding handshake status.
   This structure has to be used for ioctl
   - \ref DSL_FIO_BND_HS_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains Bonding handshake status data */
   DSL_OUT DSL_BND_HsStatusData_t data;
} DSL_BND_HsStatusGet_t;

/**
   Structure to handle the handshake continue process.
*/
typedef struct
{
   /**
   Remote Discovery Register value to be used in CLR */
   DSL_IN DSL_uint8_t nDiscoveryCode[6];
   /**
   Aggregate Data value to be used in CLR */
   DSL_IN DSL_uint32_t nAggregateData;
} DSL_BND_HsContinueData_t;

/**
   Structure to configure the Remote Discovery Register and Aggregate Register
   for Bonding handshake.
   This structure has to be used for ioctl
   - \ref DSL_FIO_BND_HS_CONTINUE
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains handshake continue process data */
   DSL_IN DSL_BND_HsContinueData_t data;
} DSL_BND_HsContinue_t;

/**
   Structure to handle Bonding Ethernet Debug counters.

   \note These counters are non standardized.
*/
typedef struct
{
   /**
   Count of Rx good fragments.
   This value is calculated as follows
   - nRxGoodFragments = nRxFragmentsLink0 + nRxFragmentsLink1 -
                        nRxFragmentsDrop */
   DSL_OUT DSL_uint32_t nRxGoodFragments;
   /**
   Count of Rx Packets transmitted to Network Processor */
   DSL_OUT DSL_uint16_t nRxPackets;
   /**
   Count of Rx Fragments received from PHY0 */
   DSL_OUT DSL_uint16_t nRxFragmentsLink0;
   /**
   Count of Rx Fragments received from PHY1 */
   DSL_OUT DSL_uint16_t nRxFragmentsLink1;
   /**
   Count of Rx Fragments dropped */
   DSL_OUT DSL_uint16_t nRxFragmentsDrop;
   /**
   Count of Tx Packets received from Network Processor */
   DSL_OUT DSL_uint16_t nTxPackets;
   /**
   Count of Tx Fragments transmitted in PHY0 */
   DSL_OUT DSL_uint16_t nTxFragmentsLink0;
   /**
   Count of Tx Fragments transmitted in PHY1 */
   DSL_OUT DSL_uint16_t nTxFragmentsLink1;
   /**
   Control register includes common configuration settings.*/
   DSL_OUT DSL_uint16_t nControlRegister;
} DSL_BND_EthDbgCountersData_t;

/**
   Structure to return Bonding Ethernet Debug counters
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains Bonding Ethernet Debug counters data */
   DSL_OUT DSL_BND_EthDbgCountersData_t data;
} DSL_BND_EthDbgCounters_t;

/**
   Structure to handle Bonding Ethernet counters.
*/
typedef struct
{
   /**
   Accumulated count of Rx Errored fragments PHY 0+1*/
   DSL_OUT DSL_uint32_t nRxErroredFragments;
   /**
   Accumulated count of Rx Small fragments PHY 0+1*/
   DSL_OUT DSL_uint32_t nRxSmallFragments;
   /**
   Accumulated count of Rx Large fragments PHY 0+1*/
   DSL_OUT DSL_uint32_t nRxLargeFragments;
   /**
   Count of Rx Lost Fragment Count */
   DSL_OUT DSL_uint16_t nRxLostFragments;
   /**
   Count of Rx Lost SOP Count*/
   DSL_OUT DSL_uint16_t nRxLostStarts;
   /**
   Count of Rx Lost EOP Count*/
   DSL_OUT DSL_uint16_t nRxLostEnds;
   /**
   Accumulated count of Rx Buffer Overflow PHY 0+1*/
   DSL_OUT DSL_uint32_t nRxOverlfows;
   /**
   Count of Rx Bad Fragment Count */
   DSL_OUT DSL_uint16_t nRxBadFragments;
} DSL_BND_EthCountersData_t;

/**
   Structure to return Bonding Ethernet counters.
   This structure has to be used for ioctl
   - \ref  DSL_FIO_BND_ETH_COUNTERS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains Bonding Ethernet Counter data */
   DSL_OUT DSL_BND_EthCountersData_t data;
} DSL_BND_EthCounters_t;

/**
   Structure to initialize the bonding hardware part.
   This structure has to be used for ioctl
   - \ref  DSL_FIO_BND_HW_INIT
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
} DSL_BND_HwInit_t;


/**
   Structure used for configuration of the port specific bonding logic.
*/
typedef struct
{
   /**
   PafEnable indicator.
   This configuration is done at showtime entry. It configures certain
   HW register settings to setup bonding logic.
   The value for bPafActivation is set to true if the following conditions are
   met:
   - bPafEnable is true (configuration)
   - bRemotePafSupported is true (status)
   - nActivationMode is 1 (set_if_clear)*/
   DSL_boolean_t bPafActivation;
   /**
   TxDataRateRatio
   The TxDataRateRatio is the ratio of PHY0 to PHY1 data rates.
   This value is coded as unsigned integer representing a scalar value within
   range of 1/256 (coded as 0x1) to (2^16-2)/256 (coded as 0xFFFE) in steps
   of 1/256.
   The following special values are defined
   - 0xFFFF specify that PHY0 is in showtime and PHY1 is not in showtime
   - 0x0000 specify that PHY0 is not in showtime and PHY1 is in showtime */
   DSL_uint16_t TxDataRateRatio;
} DSL_BND_SetupData_t;

/**
   Structure used for configuration of the port specific bonding logic.
   This structure has to be used for ioctl
   - \ref  DSL_FIO_BND_SETUP
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains Bonding Setup data */
   DSL_IN DSL_BND_SetupData_t data;
} DSL_BND_Setup_t;

/**
   Structure used to tear down a bonding port.
   This structure has to be used for ioctl
   - \ref  DSL_FIO_BND_TEAR_DOWN
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
} DSL_BND_TearDown_t;


/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_HW_INIT
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_HwInit(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_HwInit_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_SETUP
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_Setup(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_Setup_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_ConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_ConfigSet_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_ConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_ConfigGet_t *pData);
#endif
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_HS_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_HsStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_HsStatusGet_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_HS_CONTINUE
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_HsContinue(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_HsContinue_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_TEAR_DOWN
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_TearDown(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_TearDown_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_ETH_DBG_COUNTERS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_EthDbgCountersGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_EthDbgCounters_t *pData);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_ETH_COUNTERS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BND_EthCountersGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_EthCounters_t *pData);
#endif

/** @} DRV_DSL_CPE_BND */

#ifdef __cplusplus
}
#endif

#endif /** _DRV_DSL_CPE_API_BND_H*/
