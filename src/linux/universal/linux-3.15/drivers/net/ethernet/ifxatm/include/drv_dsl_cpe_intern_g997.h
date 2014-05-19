/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_INTERN_G997_H
#define _DRV_DSL_CPE_INTERN_G997_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_g997.h"

#ifdef INCLUDE_DSL_CPE_API_VINAX
   #define DSL_MAX_HLIN_DS_SCG   512
   #define DSL_MAX_HLIN_US_SCG   512
   #define DSL_MAX_HLOG_DS_SCG   512
   #define DSL_MAX_HLOG_US_SCG   512
   #define DSL_MAX_SNR_DS_SCG    4096
   #define DSL_MAX_SNR_US_SCG    4096
   #define DSL_MAX_QLN_DS_SCG    512
   #define DSL_MAX_QLN_US_SCG    512
#else
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
      /* all data stored */
      #define DSL_MAX_HLIN_DS_SCG   256
      /* all data stored */
      #define DSL_MAX_HLIN_US_SCG   64
      /* group size = 2 */
      #define DSL_MAX_HLOG_DS_SCG   256
      /* all data stored */
      #define DSL_MAX_HLOG_US_SCG   64
      /* all data stored */
      #define DSL_MAX_SNR_DS_SCG    512
      /* all data stored */
      #define DSL_MAX_SNR_US_SCG    64
      /* group size = 2 */
      #define DSL_MAX_QLN_DS_SCG    256
      /* all data stored */
      #define DSL_MAX_QLN_US_SCG    64
   #else
      #error "Please define DELT internal arrays lenght!!!"
   #endif
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/

typedef struct
{
   /**
   Data valid or not*/
   DSL_boolean_t bValid;
   /**
   Downstream/Upstream Line Attenuation
   (see chapter 7.5.1.6/7.5.1.7 of G.997.1)
   This parameter is a integral value over the complete specturm.
   It is the measured difference in the total power transmitted by the xTU C
   and the total power received by the xTU R over all subcarriers during
   diagnostics mode and initialization. The downstream line attenuation ranges
   per band from 0 to +127 dB with 0.1 dB steps. A special value (1271) indicates
   the line attenuation per band is out of range to be represented,
   unused band is indicated by (-32768) value.
   \note Line Attenuation values are init values which means it will NOT change
         during showtime.
   \note For ADSL systems the value that will be reported if the line is
         disabled reflects the last init value. */
   DSL_OUT DSL_int16_t LATN;
   /**
   Downstream/Upstream Signal Attenuation
   (see chapter 7.5.1.8/7.5.1.9 of G.997.1)
   This parameter is a integral value over the complete specturm.
   It is the measured difference in the total power transmitted by the xTU C
   and the total power received by the xTU R over all subcarriers during
   showtime. The downstream line attenuation ranges from 0 to +127 dB
   with 0.1 dB steps. A special value (1271) indicates the line attenuation per
   band is out of range to be represented, unused band is indicated
   by (-32768) value.
   \note During showtime, only a subset of the subcarriers may be transmitted
         by the xTU-C, as compared to diagnostics mode and initialization.
         Therefore, the downstream Signal attenuation may be significantly
         lower than the downstream Line attenuation.
   \note For ADSL systems and normal initialization the value that will be
         returned depends on the line state as follows:
         \code
         t1 < t <  (t1 + 15s) : Init value reported
         t >= (t1 + 15s)      : Showtime value reported

         with:
         t1 = time where entering showtime
         t  = time where value is requested from DS API
         \endcode
         The value will set to 0 if leaving showtime whether automatically
         (e.g. exception) or manually (e.g. LineDeactivate).
         Also note that the showtime value will be directly requested from the
         firmware each time it will be requested from DSL API because this value
         might change in showtime.
         If operating in DELT mode according DELT values will be reported. */
   DSL_OUT DSL_int16_t SATN;
   /**
   Downstream/Ustream Signal-to-Noise Ratio Margin
   (see chapter 7.5.1.10/7.5.1.11 of G.997.1)
   This parameter is a integral value over the complete specturm.
   The downstream signal to noise ratio margin is the maximum increase in dB of
   the noise power received at the xTU R, such that the BER requirements are met
   for all downstream bearer channels. The downstream SNR margin ranges from
   -64 dB to +63 dB with 0.1 dB steps.
   A special value (-641) indicates the parameter is out of range to be
   represented.
   \note The downstream SNR margin measurement at the xTU R may take
         up to 15 s.
   \note For ADSL systems and normal initialization the value that will be
         returned depends on the line state as follows:
         \code
         t1 < t <  (t1 + 15s) : Init value reported
         t >= (t1 + 15s)      : Showtime value reported

         with:
         t1 = time where entering showtime
         t  = time where value is requested from DS API
         \endcode
         The value will set to 0 if leaving showtime whether automatically
         (e.g. exception) or manually (e.g. LineDeactivate).
         Also note that the showtime value will be directly requested from the
         firmware each time it will be requested from DSL API because this value
         might change in showtime.
         If operating in DELT mode according DELT values will be reported. */
   DSL_OUT DSL_int16_t SNR;
   /**
   Downstream/Upstream Maximum Attainable Data Rate
   (see chapter 7.5.1.12/7.5.1.13 of G.997.1)
   This parameter indicates the maximum downstream net data rate currently
   attainable by the xTU-C/xTU-R transmitter and the xTU-R/xTU-C receiver.
   The rate is coded in bit/s.
   \note For ADSL systems the value that will be reported if the line is
         disabled  reflects the last showtime/DELT value. */
   DSL_OUT DSL_uint32_t ATTNDR;
} DSL_G997_LineStatusBackupData_t;

/**
   This structure is used to read the Hlin scale measured during diagnostic.
   Internal use only
*/
typedef struct
{
   /**
   Returns the HLIN scale value */
   DSL_OUT DSL_uint16_t nDeltHlinScale;
} DSL_G997_DeltHlinScaleInternalData_t;


typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_G997_ComplexNumber_t nSCGComplexData[DSL_MAX_HLIN_DS_SCG];
} DSL_G997_SCGComplexHlinDsData_t;

/**
   This structure is used to read the Hlin (downstream) per subcarrier group measured during
   diagnostic.
   Internal use only.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the HLIN values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGComplexHlinDsData_t deltHlin;
} DSL_G997_DeltHlinInternalDsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_G997_ComplexNumber_t nSCGComplexData[DSL_MAX_HLIN_US_SCG];
} DSL_G997_SCGComplexHlinUsData_t;

/**
   This structure is used to read the Hlin (upstream) per subcarrier group measured during
   diagnostic.
   Internal use only.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the HLIN values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGComplexHlinUsData_t deltHlin;
} DSL_G997_DeltHlinInternalUsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint16_t nSCGData[DSL_MAX_HLOG_DS_SCG];
} DSL_G997_SCGHlogDsData16_t;

/**
   This structure is used to read the Hlog (downstream) per subcarrier group during
   diagnostic.
   Internal use only.
*/
typedef struct
{
   /**
   Measurement time  */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the HLOG values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGHlogDsData16_t deltHlog;
} DSL_G997_DeltHlogInternalDsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint16_t nSCGData[DSL_MAX_HLOG_US_SCG];
} DSL_G997_SCGHlogUsData16_t;

/**
   This structure is used to read the Hlog (upstream) per subcarrier group during
   diagnostic.
   Internal use only.
*/
typedef struct
{
   /**
   Measurement time  */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the HLOG values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGHlogUsData16_t deltHlog;
} DSL_G997_DeltHlogInternalUsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint8_t nSCGData[DSL_MAX_SNR_DS_SCG];
} DSL_G997_SCGSnrDsData8_t;

/**
   This function is used to read the Signal to Noise Ratio (downstream) per subcarrier group
   measured during diagnostic.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the SNR values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGSnrDsData8_t deltSnr;
} DSL_G997_DeltSnrInternalDsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint8_t nSCGData[DSL_MAX_SNR_US_SCG];
} DSL_G997_SCGSnrUsData8_t;

/**
   This function is used to read the Signal to Noise Ratio (upstream) per subcarrier group
   measured during diagnostic.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the SNR values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGSnrUsData8_t deltSnr;
} DSL_G997_DeltSnrInternalUsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint8_t nSCGData[DSL_MAX_QLN_DS_SCG];
} DSL_G997_SCGQlnDsData8_t;

/**
   This function is used to read the Quiet Line Noise (downstream) per subcarrier group
   measured during diagnostic.
   Internal use only.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the QLN values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGQlnDsData8_t deltQln;
} DSL_G997_DeltQlnInternalDsData_t;

typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint8_t nSCGData[DSL_MAX_QLN_DS_SCG];
} DSL_G997_SCGQlnUsData8_t;

/**
   This function is used to read the Quiet Line Noise (upstream) per subcarrier group
   measured during diagnostic.
   Internal use only.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the QLN values for the number of subcarriers per group */
   DSL_OUT DSL_G997_SCGQlnUsData8_t deltQln;
} DSL_G997_DeltQlnInternalUsData_t;

/** This structure is used to store all internal DELT data */
typedef struct
{
   /** the room to store HLIN scale data */
   DSL_G997_DeltHlinScaleInternalData_t hlinScaleDataDs;
   DSL_G997_DeltHlinScaleInternalData_t hlinScaleDataUs;
   /** the room to store HLIN data */
   DSL_G997_DeltHlinInternalDsData_t hlinDataDs;
   DSL_G997_DeltHlinInternalUsData_t hlinDataUs;
   /** the room to store HLOG data */
   DSL_G997_DeltHlogInternalDsData_t hlogDataDs;
   DSL_G997_DeltHlogInternalUsData_t hlogDataUs;
   /** the room to store SNR data */
   DSL_G997_DeltSnrInternalDsData_t snrDataDs;
   DSL_G997_DeltSnrInternalUsData_t snrDataUs;
   /** the room to store QLN data */
   DSL_G997_DeltQlnInternalDsData_t qlnDataDs;
   DSL_G997_DeltQlnInternalUsData_t qlnDataUs;
#ifdef INCLUDE_DSL_CPE_API_VINAX
   /** the room to store Line Status data US & DS*/
   DSL_G997_LineStatusData_t lineStatus[DSL_ACCESSDIR_LAST];
#endif /** #ifdef INCLUDE_DSL_CPE_API_VINAX*/
} DSL_G997_DeltData_t;

#ifdef INCLUDE_DSL_CPE_API_VINAX
typedef struct
{
   /** the room to store HLOG data */
   DSL_G997_DeltHlogInternalUsData_t hlogDataUsVdsl;
   /** the room to store SNR data */
   DSL_G997_DeltSnrInternalUsData_t snrDataUsVdsl;
   /** the room to store QLN data */
   DSL_G997_DeltQlnInternalUsData_t qlnDataUsVdsl;
} DSL_G997_DeltShowtimeData_t;
#endif /* INCLUDE_DSL_CPE_API_VINAX*/

#if defined(INCLUDE_DSL_CEOC)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_SNMP_MESSAGE_SEND
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_SnmpMessageSend(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_G997_Snmp_t *pData);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_SNMP_MESSAGE_RECEIVE
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_SnmpMessageReceive(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN  DSL_Context_t *pContext,
   DSL_OUT DSL_G997_Snmp_t *pData);
#endif
#endif /** #if defined(INCLUDE_DSL_CEOC)*/

/** \file
   This file specifies the internal functions that are used for G.997.1 specific
   implementation of the ioctl interface.
   It is intendet to be used within the DSL CPE_API driver ONLY.
*/

/** \addtogroup DRV_DSL_CPE_G997
 @{ */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_ACTIVATE_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineActivateConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineActivate_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_ACTIVATE_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineActivateConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineActivate_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET

   \note CLI wrapper functions including arrays could be NOT generated
   automatically
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_XTUSystemEnablingConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_XTUSystemEnabling_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET

   \note CLI wrapper functions including arrays could be NOT generated
   automatically
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_XTUSystemEnablingConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_XTUSystemEnabling_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET

   \note CLI wrapper functions including arrays could be NOT generated
   automatically
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_XTUSystemEnablingStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_XTUSystemEnabling_t *pData
);
#endif

#ifdef INCLUDE_DSL_G997_ALARM
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_ChannelDataRateThresholdConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelDataRateThreshold_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_ChannelDataRateThresholdConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelDataRateThreshold_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_STATUS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_TRANSMISSION_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineTransmissionStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineTransmissionStatus_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_STATUS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_INIT_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineInitStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInitStatus_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_STATUS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatus_t *pData
);
#endif

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_STATUS_PER_BAND_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineStatusPerBandGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatusPerBand_t *pData
);
#endif
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_RATE_ADAPTATION_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_RateAdaptationConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_RateAdaptationConfig_t *pData
);
#endif

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_RATE_ADAPTATION_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_RateAdaptationConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_RateAdaptationConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_CHANNEL_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_ChannelStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelStatus_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATE_FORCED_TRIGGER
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_PowerManagementStateForcedTrigger(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_PowerManagementStateForcedTrigger_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_PowerManagementStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_PowerManagementStatus_t *pData
);
#endif

#ifdef INCLUDE_DSL_G997_STATUS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LAST_STATE_TRANSMITTED_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LastStateTransmittedGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LastStateTransmitted_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_PER_TONE
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_BIT_ALLOCATION_NSC_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_BitAllocationNSCGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_BitAllocationNsc_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_GainAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_GainAllocationNsc_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_SNR_ALLOCATION_NSC_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_SnrAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_SnrAllocationNsc_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_G997_ALARM
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_AlarmMaskLineFailuresConfigSet(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineFailures_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_AlarmMaskLineFailuresConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineFailures_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_STATUS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_FAILURES_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_LineFailuresStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineFailures_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_ALARM
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_AlarmMaskDataPathFailuresConfigSet(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DataPathFailures_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_AlarmMaskDataPathFailuresConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DataPathFailures_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_STATUS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DATA_PATH_FAILURES_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_DataPathFailuresStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DataPathFailures_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_STATUS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_FRAMING_PARAMETER_STATUS_GET
*/
#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_G997_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_FramingParameterStatus_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_INVENTORY_GET
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_LineInventoryGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventory_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_LINE_INVENTORY_SET
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_LineInventorySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventoryNe_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_DELT
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_HLIN_SCALE_GET
*/
DSL_Error_t DSL_DRV_G997_DeltHlinScaleGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltHlinScale_t *pData
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_HLIN_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_DeltHlinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltHlin_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_HLOG_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_DeltHlogGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltHlog_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_SNR_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_DeltSNRGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltSnr_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_QLN_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_DeltQLNGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltQln_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_G997_DELT_SNR_GET
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_G997_DeltFreeResources(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_DeltFreeResources_t *pData
);
#endif
#endif /* INCLUDE_DSL_DELT*/
/** @} DRV_DSL_CPE_G997 */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_G997_H */
