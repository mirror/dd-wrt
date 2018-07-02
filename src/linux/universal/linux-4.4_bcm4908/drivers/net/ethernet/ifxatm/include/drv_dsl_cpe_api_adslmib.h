/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_ADSLMIB_H
#define _DRV_DSL_CPE_API_ADSLMIB_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"

#if (INCLUDE_DSL_CPE_API_ADSL_SUPPORT == 1)

#ifdef INCLUDE_DSL_ADSL_MIB

/** \file
   This file defines the MIB interface definitions and structures for
   DSL CPE API in case of using RFC2662 and RFC3440
*/

/** \defgroup DRV_DSL_CPE_API Infineon DSL CPE API
    Lists the entire modules to the DSL CPE_API.
  @{ */

/**
   \defgroup DRV_DSL_CPE_ADSL_MIB ADSL SNMP MIB
   Includes all MIB related coding
*/

/** @} */


/** \addtogroup DRV_DSL_CPE_ADSL_MIB
 @{ */

/* Macro Definitions ? FLAG Setting & Testing */

/**
   This macro sets the flags with the flag_val */
#define SET_FLAG(flags, flag_val)   ((flags) = ((flags) | flag_val))

/*
   This macro verifies whether test_flag has been set in flags */
#define IS_FLAG_SET(flags, test_flag)  (((flags) & (test_flag)) == (test_flag)? test_flag:0)

/*
   This macro resets the specified flag_bit in the flags */
#define CLR_FLAG(flags, flag_bit)   ((flags) = (flags) & (~flag_bit))


/** Definitions for adslLineCode flags */
typedef enum
{
   LINE_CODE_FLAG = 0x1   /* BIT 0th position */
} adslLineTableFlags_t;

/** Definitions for adslAtucPhysTable flags */
typedef enum
{
   ATUC_PHY_SER_NUM_FLAG   = 0x1,   /* BIT 0th position */
   ATUC_PHY_VENDOR_ID_FLAG = 0x2,   /* BIT 1 */
   ATUC_PHY_VER_NUM_FLAG   = 0x4,   /* BIT 2 */
   ATUC_CURR_STAT_FLAG     = 0x8,   /* BIT 3 */
   ATUC_CURR_OUT_PWR_FLAG  = 0x10,  /* BIT 4 */
   ATUC_CURR_ATTR_FLAG     = 0x20,  /* BIT 5 */
   ATUC_CURR_ATN_FLAG      = 0x40,  /* BIT 6 */
   ATUC_CURR_SNR_FLAG      = 0x80   /* BIT 7 */
} adslAtucPhysTableFlags_t;

/** Definitions for adslAturPhysTable flags */
typedef enum
{
   ATUR_PHY_SER_NUM_FLAG   = 0x1,   /* BIT 0th position */
   ATUR_PHY_VENDOR_ID_FLAG = 0x2,   /* BIT 1 */
   ATUR_PHY_VER_NUM_FLAG   = 0x4,   /* BIT 2 */
   ATUR_CURR_STAT_FLAG     = 0x20,  /* BIT 3 */
   ATUR_CURR_OUT_PWR_FLAG  = 0x40,  /* BIT 4 */
   ATUR_CURR_ATTR_FLAG     = 0x80,   /* BIT 5 */
   ATUR_CURR_ATN_FLAG      = 0x100,  /* BIT 6 */
   ATUR_CURR_SNR_FLAG      = 0x200   /* BIT 7 */
} adslAturPhysTableFlags_t;

/** Definitions for adslAtucChanInfo_t flags */
typedef enum
{
   ATUC_CHAN_INTLV_DELAY_FLAG  = 0x1,   /* BIT 0th position */
   ATUC_CHAN_CURR_TX_RATE_FLAG = 0x2,   /* BIT 1 */
   ATUC_CHAN_PREV_TX_RATE_FLAG = 0x4    /* BIT 2 */
} adslAtucChanInfoFlags_t;

/** Definitions for adslAturChanInfo_t flags */
typedef enum
{
   ATUR_CHAN_INTLV_DELAY_FLAG  = 0x1,   /* BIT 0th position */
   ATUR_CHAN_CURR_TX_RATE_FLAG = 0x2,   /* BIT 1 */
   ATUR_CHAN_PREV_TX_RATE_FLAG = 0x4,   /* BIT 2 */
   ATUR_CHAN_CRC_BLK_LEN_FLAG  = 0x8    /* BIT 3 */
} adslAturChanInfoFlags_t;

/** Definitions for adslAtucPerfDataTable flags */
typedef enum
{
   ATUC_PERF_LOFS_FLAG               = 0x1,     /* BIT 0th position */
   ATUC_PERF_LOSS_FLAG               = 0x2,     /* BIT 1 */
   ATUC_PERF_ESS_FLAG                = 0x4,     /* BIT 2 */
   ATUC_PERF_INITS_FLAG              = 0x8,     /* BIT 3 */
   ATUC_PERF_VALID_INTVLS_FLAG       = 0x10,    /* BIT 4 */
   ATUC_PERF_INVALID_INTVLS_FLAG     = 0x20,    /* BIT 5 */
   ATUC_PERF_CURR_15MIN_TIME_ELAPSED_FLAG = 0x40, /* BIT 6 */
   ATUC_PERF_CURR_15MIN_LOFS_FLAG    = 0x80,     /* BIT 7 */
   ATUC_PERF_CURR_15MIN_LOSS_FLAG    = 0x100,    /* BIT 8 */
   ATUC_PERF_CURR_15MIN_ESS_FLAG     = 0x200,    /* BIT 9 */
   ATUC_PERF_CURR_15MIN_INIT_FLAG    = 0x400,    /* BIT 10 */
   ATUC_PERF_CURR_1DAY_TIME_ELAPSED_FLAG = 0x800, /* BIT 11 */
   ATUC_PERF_CURR_1DAY_LOFS_FLAG     = 0x1000,   /* BIT 12 */
   ATUC_PERF_CURR_1DAY_LOSS_FLAG     = 0x2000,   /* BIT 13 */
   ATUC_PERF_CURR_1DAY_ESS_FLAG      = 0x4000,   /* BIT 14 */
   ATUC_PERF_CURR_1DAY_INIT_FLAG     = 0x8000,   /* BIT 15 */
   ATUC_PERF_PREV_1DAY_MON_SEC_FLAG  = 0x10000,  /* BIT 16 */
   ATUC_PERF_PREV_1DAY_LOFS_FLAG     = 0x20000,  /* BIT 17 */
   ATUC_PERF_PREV_1DAY_LOSS_FLAG     = 0x40000,  /* BIT 18 */
   ATUC_PERF_PREV_1DAY_ESS_FLAG      = 0x80000,  /* BIT 19 */
   ATUC_PERF_PREV_1DAY_INITS_FLAG    = 0x100000  /* BIT 20 */
} adslAtucPerfDataTableFlags_t;

/** Definitions for adslAturPerfDataTable flags */
typedef enum
{
   ATUR_PERF_LOFS_FLAG               = 0x1,      /* BIT 0th position */
   ATUR_PERF_LOSS_FLAG               = 0x2,      /* BIT 1 */
   ATUR_PERF_LPR_FLAG                = 0x4,      /* BIT 2 */
   ATUR_PERF_ESS_FLAG                = 0x8,      /* BIT 3 */
   ATUR_PERF_VALID_INTVLS_FLAG       = 0x10,     /* BIT 4 */
   ATUR_PERF_INVALID_INTVLS_FLAG     = 0x20,     /* BIT 5 */
   ATUR_PERF_CURR_15MIN_TIME_ELAPSED_FLAG = 0x40, /* BIT 6 */
   ATUR_PERF_CURR_15MIN_LOFS_FLAG    = 0x80,     /* BIT 7 */
   ATUR_PERF_CURR_15MIN_LOSS_FLAG    = 0x100,    /* BIT 8 */
   ATUR_PERF_CURR_15MIN_LPR_FLAG     = 0x200,    /* BIT 9 */
   ATUR_PERF_CURR_15MIN_ESS_FLAG     = 0x400,    /* BIT 10 */
   ATUR_PERF_CURR_1DAY_TIME_ELAPSED_FLAG = 0x800, /* BIT 11 */
   ATUR_PERF_CURR_1DAY_LOFS_FLAG     = 0x1000,   /* BIT 12 */
   ATUR_PERF_CURR_1DAY_LOSS_FLAG     = 0x2000,   /* BIT 13 */
   ATUR_PERF_CURR_1DAY_LPR_FLAG      = 0x4000,   /* BIT 14 */
   ATUR_PERF_CURR_1DAY_ESS_FLAG      = 0x8000,   /* BIT 15 */
   ATUR_PERF_PREV_1DAY_MON_SEC_FLAG  = 0x10000,  /* BIT 16 */
   ATUR_PERF_PREV_1DAY_LOFS_FLAG     = 0x20000,  /* BIT 17 */
   ATUR_PERF_PREV_1DAY_LOSS_FLAG     = 0x40000,  /* BIT 18 */
   ATUR_PERF_PREV_1DAY_LPR_FLAG      = 0x80000,  /* BIT 19 */
   ATUR_PERF_PREV_1DAY_ESS_FLAG      = 0x100000  /* BIT 20 */
} adslAturPerfDataTableFlags_t;

/** Definitions for adslAtucIntvlInfo_t flags */
typedef enum
{
   ATUC_INTVL_LOF_FLAG      = 0x1,   /* BIT 0th position */
   ATUC_INTVL_LOS_FLAG      = 0x2,   /* BIT 1 */
   ATUC_INTVL_ESS_FLAG      = 0x4,   /* BIT 2 */
   ATUC_INTVL_INIT_FLAG     = 0x8,   /* BIT 3 */
   ATUC_INTVL_VALID_DATA_FLAG = 0x10 /* BIT 4 */
} adslAtucIntvlInfoFlags_t;

/** Definitions for adslAturIntvlInfo_t flags */
typedef enum
{
   ATUR_INTVL_LOF_FLAG      = 0x1,   /* BIT 0th position */
   ATUR_INTVL_LOS_FLAG      = 0x2,   /* BIT 1 */
   ATUR_INTVL_LPR_FLAG      = 0x4,   /* BIT 2 */
   ATUR_INTVL_ESS_FLAG      = 0x8,   /* BIT 3 */
   ATUR_INTVL_VALID_DATA_FLAG = 0x10 /* BIT 4 */
} adslAturIntvlInfoFlags_t;

/** Definitions for adslAtucChannelPerfDataTable flags */
typedef enum
{
   ATUC_CHAN_RECV_BLK_FLAG                     = 0x01,  /* BIT 0th position */
   ATUC_CHAN_TX_BLK_FLAG                       = 0x02,  /* BIT 1 */
   ATUC_CHAN_CORR_BLK_FLAG                     = 0x04,  /* BIT 2 */
   ATUC_CHAN_UNCORR_BLK_FLAG                   = 0x08,  /* BIT 3 */
   ATUC_CHAN_PERF_VALID_INTVL_FLAG             = 0x10,  /* BIT 4 */
   ATUC_CHAN_PERF_INVALID_INTVL_FLAG           = 0x20,  /* BIT 5 */
   ATUC_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG = 0x40,  /* BIT 6 */
   ATUC_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG     = 0x80,  /* BIT 7 */
   ATUC_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG       = 0x100, /* BIT 8 */
   ATUC_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG     = 0x200, /* BIT 9 */
   ATUC_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG   = 0x400, /* BIT 10 */
   ATUC_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG  = 0x800, /* BIT 11*/
   ATUC_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG      = 0x1000, /* BIT 12 */
   ATUC_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG        = 0x2000, /* BIT 13 */
   ATUC_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG      = 0x4000, /* BIT 14 */
   ATUC_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG    = 0x8000, /* BIT 15 */
   ATUC_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG      = 0x10000, /* BIT 16 */
   ATUC_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG      = 0x20000, /* BIT 17 */
   ATUC_CHAN_PERF_PREV_1DAY_TX_BLK_FLAG        = 0x40000, /* BIT 18 */
   ATUC_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG      = 0x80000, /* BIT 19 */
   ATUC_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG    = 0x100000 /* BIT 20 */
} adslAtucChannelPerfDataTableFlags_t;


/** Definitions for adslAturChannelPerfDataTable flags */
typedef enum
{
   ATUR_CHAN_RECV_BLK_FLAG                     = 0x01,     /* BIT 0th position */
   ATUR_CHAN_TX_BLK_FLAG                       = 0x02,     /* BIT 1 */
   ATUR_CHAN_CORR_BLK_FLAG                     = 0x04,     /* BIT 2 */
   ATUR_CHAN_UNCORR_BLK_FLAG                   = 0x08,     /* BIT 3 */
   ATUR_CHAN_PERF_VALID_INTVL_FLAG             = 0x10,     /* BIT 4 */
   ATUR_CHAN_PERF_INVALID_INTVL_FLAG           = 0x20,     /* BIT 5 */
   ATUR_CHAN_PERF_CURR_15MIN_TIME_ELAPSED_FLAG = 0x40,     /* BIT 6 */
   ATUR_CHAN_PERF_CURR_15MIN_RECV_BLK_FLAG     = 0x80,     /* BIT 7 */
   ATUR_CHAN_PERF_CURR_15MIN_TX_BLK_FLAG       = 0x100,    /* BIT 8 */
   ATUR_CHAN_PERF_CURR_15MIN_CORR_BLK_FLAG     = 0x200,    /* BIT 9 */
   ATUR_CHAN_PERF_CURR_15MIN_UNCORR_BLK_FLAG   = 0x400,    /* BIT 10 */
   ATUR_CHAN_PERF_CURR_1DAY_TIME_ELAPSED_FLAG  = 0x800,    /* BIT 11 */
   ATUR_CHAN_PERF_CURR_1DAY_RECV_BLK_FLAG      = 0x1000,   /* BIT 12 */
   ATUR_CHAN_PERF_CURR_1DAY_TX_BLK_FLAG        = 0x2000,   /* BIT 13 */
   ATUR_CHAN_PERF_CURR_1DAY_CORR_BLK_FLAG      = 0x4000,   /* BIT 14 */
   ATUR_CHAN_PERF_CURR_1DAY_UNCORR_BLK_FLAG    = 0x8000,   /* BIT 15 */
   ATUR_CHAN_PERF_PREV_1DAY_MONI_SEC_FLAG      = 0x10000,  /* BIT 16 */
   ATUR_CHAN_PERF_PREV_1DAY_RECV_BLK_FLAG      = 0x20000,  /* BIT 17 */
   ATUR_CHAN_PERF_PREV_1DAY_TRANS_BLK_FLAG     = 0x40000,  /* BIT 18 */
   ATUR_CHAN_PERF_PREV_1DAY_CORR_BLK_FLAG      = 0x80000,  /* BIT 19 */
   ATUR_CHAN_PERF_PREV_1DAY_UNCORR_BLK_FLAG    = 0x100000  /* BIT 20 */
} adslAturChannelPerfDataTableFlags_t;

/** Definitions for adslAtucChanIntvlInfo_t flags */
typedef enum
{
   ATUC_CHAN_INTVL_NUM_FLAG          = 0x1,   /* BIT 0th position */
   ATUC_CHAN_INTVL_RECV_BLK_FLAG     = 0x2,   /* BIT 1 */
   ATUC_CHAN_INTVL_TX_BLK_FLAG       = 0x4,   /* BIT 2 */
   ATUC_CHAN_INTVL_CORR_BLK_FLAG     = 0x8,   /* BIT 3 */
   ATUC_CHAN_INTVL_UNCORR_BLK_FLAG   = 0x10,  /* BIT 4 */
   ATUC_CHAN_INTVL_VALID_DATA_FLAG   = 0x20   /* BIT 5 */
} adslAtucChanIntvlInfoFlags_t;

/** Definitions for adslAturChanIntvlInfo_t flags */
typedef enum
{
   ATUR_CHAN_INTVL_NUM_FLAG          = 0x1,   /* BIT 0th Position */
   ATUR_CHAN_INTVL_RECV_BLK_FLAG     = 0x2,   /* BIT 1 */
   ATUR_CHAN_INTVL_TX_BLK_FLAG       = 0x4,   /* BIT 2 */
   ATUR_CHAN_INTVL_CORR_BLK_FLAG     = 0x8,   /* BIT 3 */
   ATUR_CHAN_INTVL_UNCORR_BLK_FLAG   = 0x10,  /* BIT 4 */
   ATUR_CHAN_INTVL_VALID_DATA_FLAG   = 0x20   /* BIT 5 */
} adslAturChanIntvlInfoFlags_t;

/** Definitions for adslAturLineAlarmConfProfileTable flags */
typedef enum
{
   ATUC_THRESH_15MIN_LOFS_FLAG            = 0x01,   /* BIT 0th position */
   ATUC_THRESH_15MIN_LOSS_FLAG            = 0x02,   /* BIT 1 */
   ATUC_THRESH_15MIN_ESS_FLAG             = 0x04,   /* BIT 2 */
   ATUC_THRESH_FAST_RATEUP_FLAG           = 0x08,   /* BIT 3 */
   ATUC_THRESH_INTERLEAVE_RATEUP_FLAG     = 0x10,   /* BIT 4 */
   ATUC_THRESH_FAST_RATEDOWN_FLAG         = 0x20,   /* BIT 5 */
   ATUC_THRESH_INTERLEAVE_RATEDOWN_FLAG   = 0x40,   /* BIT 6 */
   ATUC_INIT_FAILURE_TRAP_ENABLE_FLAG     = 0x80,   /* BIT 7 */
   ATUR_THRESH_15MIN_LOFS_FLAG            = 0x100,    /* BIT 8 */
   ATUR_THRESH_15MIN_LOSS_FLAG            = 0x200,    /* BIT 9 */
   ATUR_THRESH_15MIN_LPRS_FLAG            = 0x400,    /* BIT 10 */
   ATUR_THRESH_15MIN_ESS_FLAG             = 0x800,    /* BIT 11 */
   ATUR_THRESH_FAST_RATEUP_FLAG           = 0x1000,   /* BIT 12 */
   ATUR_THRESH_INTERLEAVE_RATEUP_FLAG     = 0x2000,   /* BIT 13 */
   ATUR_THRESH_FAST_RATEDOWN_FLAG         = 0x4000,   /* BIT 14 */
   ATUR_THRESH_INTERLEAVE_RATEDOWN_FLAG   = 0x8000,   /* BIT 15 */
   LINE_ALARM_CONF_PROFILE_ROWSTATUS_FLAG = 0x10000   /* BIT 16 */
} adslAturLineAlarmConfProfileTableFlags_t;


/** Definitions for adslAturTraps flags */
typedef enum
{
   ATUC_PERF_LOFS_THRESH_FLAG =  0x1,   /* BIT 0th position */
   ATUC_PERF_LOSS_THRESH_FLAG =  0x2,   /* BIT 1 */
   ATUC_PERF_ESS_THRESH_FLAG  =  0x4,   /* BIT 2 */
   ATUC_RATE_CHANGE_FLAG      =  0x8,   /* BIT 3 */
   ATUR_PERF_LOFS_THRESH_FLAG =  0x10,  /* BIT 4 */
   ATUR_PERF_LOSS_THRESH_FLAG =  0x20,  /* BIT 5 */
   ATUR_PERF_LPRS_THRESH_FLAG =  0x40,  /* BIT 6 */
   ATUR_PERF_ESS_THRESH_FLAG  =  0x80,  /* BIT 7 */
   ATUR_RATE_CHANGE_FLAG      =  0x100  /* BIT 8 */
} adslAturTrapsFlags_t;


#ifdef INCLUDE_ADSL_MIB_RFC3440
/** Definitions for adslAtucLineExtTable flags */
typedef enum
{
   ATUC_LINE_TRANS_CAP_FLAG     = 0x1,      /* BIT 0th position */
   ATUC_LINE_TRANS_CONFIG_FLAG  = 0x2,      /* BIT 1 */
   ATUC_LINE_TRANS_ACTUAL_FLAG  = 0x4,      /* BIT 2 */
   LINE_GLITE_POWER_STATE_FLAG  = 0x8       /* BIT 3 */
} adslAtucLineExtTableFlags_t;

/** Definitions for adslAtucPerfDataExtTable flags */
typedef enum
{
   ATUC_PERF_STAT_FASTR_FLAG              = 0x1,    /* BIT 0th position */
   ATUC_PERF_STAT_FAILED_FASTR_FLAG       = 0x2,    /* BIT 1 */
   ATUC_PERF_STAT_SESL_FLAG               = 0x4,    /* BIT 2 */
   ATUC_PERF_STAT_UASL_FLAG               = 0x8,    /* BIT 3 */
   ATUC_PERF_CURR_15MIN_FASTR_FLAG        = 0x10,   /* BIT 4 */
   ATUC_PERF_CURR_15MIN_FAILED_FASTR_FLAG = 0x20,   /* BIT 5 */
   ATUC_PERF_CURR_15MIN_SESL_FLAG         = 0x40,   /* BIT 6 */
   ATUC_PERF_CURR_15MIN_UASL_FLAG         = 0x80,   /* BIT 7 */
   ATUC_PERF_CURR_1DAY_FASTR_FLAG         = 0x100,  /* BIT 8 */
   ATUC_PERF_CURR_1DAY_FAILED_FASTR_FLAG  = 0x200,  /* BIT 9 */
   ATUC_PERF_CURR_1DAY_SESL_FLAG          = 0x400,  /* BIT 10 */
   ATUC_PERF_CURR_1DAY_UASL_FLAG          = 0x800,  /* BIT 11 */
   ATUC_PERF_PREV_1DAY_FASTR_FLAG         = 0x1000, /* BIT 12 */
   ATUC_PERF_PREV_1DAY_FAILED_FASTR_FLAG  = 0x2000, /* BIT 13 */
   ATUC_PERF_PREV_1DAY_SESL_FLAG          = 0x4000, /* BIT 14 */
   ATUC_PERF_PREV_1DAY_UASL_FLAG          = 0x8000  /* BIT 15 */
} adslAtucPerfDataExtTableFlags_t;

/** Definitions for adslAturPerfDataExtTable flags */
typedef enum
{
   ATUR_PERF_STAT_SESL_FLAG       = 0x1,  /* BIT 0th position */
   ATUR_PERF_STAT_UASL_FLAG       = 0x2,  /* BIT 1 */
   ATUR_PERF_CURR_15MIN_SESL_FLAG = 0x4,  /* BIT 2 */
   ATUR_PERF_CURR_15MIN_UASL_FLAG = 0x8,  /* BIT 3 */
   ATUR_PERF_CURR_1DAY_SESL_FLAG  = 0x10, /* BIT 4 */
   ATUR_PERF_CURR_1DAY_UASL_FLAG  = 0x20, /* BIT 5 */
   ATUR_PERF_PREV_1DAY_SESL_FLAG  = 0x40, /* BIT 6 */
   ATUR_PERF_PREV_1DAY_UASL_FLAG  = 0x80  /* BIT 7 */
} adslAturPerfDataExtTableFlags_t;

/** Definitions for adslAtucIntvlExtInfo flags */
typedef enum
{
   ATUC_INTERVAL_FASTR_FLAG         = 0x1, /* Bit 0 */
   ATUC_INTERVAL_FAILED_FASTR_FLAG  = 0x2, /* Bit 1 */
   ATUC_INTERVAL_SESL_FLAG          = 0x4, /* Bit 2 */
   ATUC_INTERVAL_UASL_FLAG          = 0x8  /* Bit 3 */
} adslAtucIntvlExtInfoFlags_t;

/** Definitions for adslAturIntvExtTable flags */
typedef enum
{
   ATUR_INTERVAL_SESL_FLAG         = 0x1, /* BIT 0th position */
   ATUR_INTERVAL_UASL_FLAG         = 0x2  /* BIT 1 */
} adslAturIntvExtTableFlags_t;

/** Definitions for adslLineAlarmConfProfileExtTable flags */
typedef enum
{
   ATUC_THRESH_15MIN_FAILED_FASTR_FLAG = 0x1, /* BIT 0th position */
   ATUC_THRESH_15MIN_SESL_FLAG         = 0x2, /* BIT 1 */
   ATUC_THRESH_15MIN_UASL_FLAG         = 0x4, /* BIT 2 */
   ATUR_THRESH_15MIN_SESL_FLAG         = 0x8, /* BIT 3 */
   ATUR_THRESH_15MIN_UASL_FLAG         = 0x10 /* BIT 4 */
} adslLineAlarmConfProfileExtTableFlags_t;

/** Definitions for adslAturExtTraps flags */
typedef enum
{
   ATUC_15MIN_FAILED_FASTR_TRAP_FLAG = 0x1,  /* BIT 0th position */
   ATUC_15MIN_SESL_TRAP_FLAG         = 0x2,  /* BIT 1 */
   ATUC_15MIN_UASL_TRAP_FLAG         = 0x4,  /* BIT 2 */
   ATUR_15MIN_SESL_TRAP_FLAG         = 0x8,  /* BIT 3 */
   ATUR_15MIN_UASL_TRAP_FLAG         = 0x10  /* BIT 4 */
} adslAturExtTrapsFlags_t;

#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/** Helper definition for MIB table entry value */
typedef DSL_uint32_t AdslPerfTimeElapsed;
/** Helper definition for MIB table entry value */
typedef DSL_uint32_t AdslPerfPrevDayCount;
/** Helper definition for MIB table entry value */
typedef DSL_uint32_t PerfCurrentCount;
/** Helper definition for MIB table entry value */
typedef DSL_uint32_t PerfIntervalCount;
/** Helper definition for MIB table entry value */
typedef DSL_uint32_t AdslPerfCurrDayCount;


/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_LINE_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
      Specifies the ADSL coding type used on this line.
      - Numerical syntax: Integer (32 bit)
      - Base syntax: INTEGER */
   DSL_int_t adslLineCode;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslLineTableFlags_t */
   DSL_uint8_t flags;
} adslLineTableEntry_t;

#ifdef INCLUDE_ADSL_MIB_RFC3440
/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET and
   \ref DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_SET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
      The transmission modes, represented by a bitmask that the ATU-C is capable
      of supporting. The modes available are limited by the design of the
      equipment.
      - Numerical syntax: Octets
      - Base syntax: BITS
      - Reference: Section 7.3.2 ITU G.997.1 */
   DSL_uint16_t adslLineTransAtucCap;
   /**
      The transmission modes, represented by a bitmask, currently enabled by the
      ATU-C. The manager can only set those modes that are supported by the
      ATU-C. An ATU-C's supported modes are provided by AdslLineTransAtucCap.
      - Numerical syntax: Octets
      - Base syntax: BITS
      - Reference: Section 7.3.2 ITU G.997.1 */
   DSL_uint16_t adslLineTransAtucConfig;
   /**
      The actual transmission mode of the ATU-C. During ADSL line initialization,
      the ADSL Transceiver Unit - Remote terminal end (ATU-R) will determine the
      mode used for the link. This value will be limited a single transmission
      mode that is a subset of those modes enabled by the ATU-C and denoted by
      adslLineTransAtucConfig. After an initialization has occurred, its mode is
      saved as the 'Current' mode and is persistence should the link go down.
      This object returns 0 (i.e. BITS with no mode bit set) if the mode is not
      known.
      - Numerical syntax: Octets
      - Base syntax: BITS
      - Reference: Section 7.3.2 ITU G.997.1 */
   DSL_uint16_t adslLineTransAtucActual;
   /**
      The value of this object specifies the power state of this interface.
      L0 is power on, L1 is power on but reduced and L3 is power off. Power
      state cannot be configured by an operator but it can be viewed via the
      ifOperStatus object for the managed ADSL interface. The value of the
      object ifOperStatus is set to down(2) if the ADSL interface is in power
      state L3 and is set to up(1) if the ADSL line interface is in power state
      L0 or L1. If the object adslLineTransAtucActual is set to a G.992.2
      (G.Lite)-type transmission mode, the value of this object will be one of
      the valid power states: L0(2), L1(3), or L3(4).  Otherwise, its value
      will be none(1).
      - Numerical syntax: Integer (32 bit)
      - Base syntax: INTEGER
      - Value list:
         - 1: none(1)
         - 2: l0(2)
         - 3: l1(3)
         - 4: l3(4) */
   DSL_int_t adslLineGlitePowerState;
   /**
   Defines a bitmask to specify which parameters shall be accessed according
   to \ref adslAtucLineExtTableFlags_t */
   DSL_uint32_t flags;
} adslLineExtTableEntry_t;
#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/**
   Structure used within \ref adslAtucPhysEntry_t and \ref adslAturPhysEntry_t to
    store the vendor id information */
typedef struct
{
   DSL_uint16_t   country_code;
   DSL_char_t   provider_id[4];  /* Ascii characters */
   DSL_char_t   revision_info[2];
} adslVendorId_t;

/**
   Structure (union) definition for storing vedor id in CMV and parameter
   format */
typedef union
{
   DSL_char_t vendor_id[16];
   adslVendorId_t vendor_info;
} vendor_id_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
      The vendor specific string that identifies the vendor equipment.
      - Numerical syntax: Octets
      - Base syntax: OCTET STRING
      - Size list: 1: 0..32 */
   DSL_char_t serial_no[32];
   /**
      The vendor ID code is a copy of the binary vendor identification field
      defined by the PHY[10] and expressed as readable characters.
      - Numerical syntax: Octets
      - Base syntax: OCTET STRING
      - Size list: 1: 0..16
      - Reference: ANSI T1.413[10] */
   vendor_id_t vendor_id;
   /**
      The vendor specific version number sent by this ATU as part of the
      initialization messages. It is a copy of the binary version number field
      defined by the PHY[10] and expressed as readable characters.
      - Numerical syntax: Octets
      - Base syntax: OCTET STRING
      - Size list: 1: 0..16
      - Reference: ANSI T1.413[10] */
   DSL_char_t version_no[16];
   /**
      Noise Margin as seen by this ATU with respect to its received signal in
      tenth dB.
      - Numerical syntax: Integer (32 bit)
      - Base syntax: INTEGER
      - Size list: 1: -640..640
      - Units: tenth dB */
   DSL_int_t SnrMgn;
   /**
      Measured difference in the total power transmitted by the peer ATU and
      the total power received by this ATU.
      - Numerical syntax: Gauge (32 bit)
      - Base syntax: Gauge32
      - Size list: 1: 0..630
      - Units: tenth dB */
   DSL_uint32_t Attn;
   /**
      Indicates current state of the ATUC line. This is a bit-map of possible
      conditions. The various bit positions are:
      - 0: noDefect            There no defects on the line
      - 1: lossOfFraming       ATUC failure due to not receiving valid frame.
      - 2: lossOfSignal        ATUC failure due to not receiving signal.
      - 3: lossOfPower         ATUC failure due to loss of power.
                               \note the Agent may still function.
      - 4: lossOfSignalQuality Loss of Signal Quality is declared when the Noise
                               Margin falls below the Minimum Noise Margin, or
                               the bit-error-rate exceeds 10^-7.
      - 5: lossOfLink          ATUC failure due to inability to link with ATUR.
      - 6: dataInitFailure     ATUC failure during initialization due to bit
                               errors corrupting startup exchange data.
      - 7: configInitFailure   ATUC failure during initialization due to peer
                               ATU not able to support
                               requested configuration
      - 8: protocolInitFailure ATUC failure during initialization due to
                               incompatible protocol used by the peer ATU.
      - 9: noPeerAtuPresent    ATUC failure during initialization due to no
                               activation sequence detected from peer ATU.
   This is intended to supplement ifOperStatus. */
   DSL_uint32_t status;
   /**
      Measured total output power transmitted by this ATU. This is the
      measurement that was reported during the last activation sequence.
      - Numerical syntax: Integer (32 bit)
      - Base syntax: INTEGER
      - Size list: 1: -310..310
      - Units: tenth dBm */
   DSL_int_t outputPwr;
   /**
      Indicates the maximum currently attainable data rate by the ATU. This
      value will be equal or greater than the current line rate.
      - Numerical syntax: Gauge (32 bit)
      - Base syntax: Gauge32
      - Units: bps */
   DSL_uint32_t attainableRate;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucPhysTableFlags_t */
   DSL_uint32_t flags;
} adslAtucPhysEntry_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
      The vendor specific string that identifies the vendor equipment.
      - Numerical syntax: Octets
      - Base syntax: OCTET STRING
      - Size list: 1: 0..32 */
   DSL_char_t serial_no[32];
   /**
      The vendor ID code is a copy of the binary vendor identification field
      defined by the PHY[10] and expressed as readable characters.
      - Numerical syntax: Octets
      - Base syntax: OCTET STRING
      - Size list: 1: 0..16
      - Reference: ANSI T1.413[10] */
   vendor_id_t vendor_id;
   /**
      The vendor specific version number sent by this ATU as part of the
      initialization messages. It is a copy of the binary version number field
      defined by the PHY[10] and expressed as readable characters.
      - Numerical syntax: Octets
      - Base syntax: OCTET STRING
      - Size list: 1: 0..16
      - Reference: ANSI T1.413[10] */
   DSL_char_t version_no[16];
   /**
      Noise Margin as seen by this ATU with respect to its received signal in
      tenth dB.
      - Numerical syntax: Integer (32 bit)
      - Base syntax: INTEGER
      - Size list: 1: -640..640
      - Units: tenth dB */
   DSL_int_t SnrMgn;
   /**
      Measured difference in the total power transmitted by the peer ATU and
      the total power received by this ATU.
      - Numerical syntax: Gauge (32 bit)
      - Base syntax: Gauge32
      - Size list: 1: 0..630
      - Units: tenth dB */
   DSL_uint32_t Attn;
   /**
      Indicates current state of the ATUR line.  This is a bit-map of possible
      conditions. Due to the isolation of the ATUR when line problems occur,
      many state conditions like loss of power, loss of quality signal, and
      initialization errors,  can not be determined. While trouble shooting
      ATUR, also use object, adslAtucCurrStatus. The various bit positions are:
      - 0: noDefect             There no defects on the line
      - 1: lossOfFraming        ATUR failure due to not receiving valid frame
      - 2: lossOfSignal         ATUR failure due to not receiving signal
      - 3: lossOfPower          ATUR failure due to loss of power
      - 4: lossOfSignalQuality  Loss of Signal Quality is declared when the
                                Noise Margin falls below the Minimum Noise
                                Margin, or the bit-error-rate exceeds 10^-7.
      This is intended to supplement ifOperStatus. */
   DSL_uint32_t status;
   /**
      Measured total output power transmitted by this ATU. This is the
      measurement that was reported during the last activation sequence.
      - Numerical syntax: Integer (32 bit)
      - Base syntax: INTEGER
      - Size list: 1: -310..310
      - Units: tenth dBm */
   DSL_int_t outputPwr;
   /**
      Indicates the maximum currently attainable data rate by the ATU. This
      value will be equal or greater than the current line rate.
      - Numerical syntax: Gauge (32 bit)
      - Base syntax: Gauge32
      - Units: bps */
   DSL_uint32_t attainableRate;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturPhysTableFlags_t */
   DSL_uint32_t flags;
} adslAturPhysEntry_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Interleave Delay for this channel.
   Interleave delay applies only to the interleave channel and defines the
   mapping (relative spacing) between subsequent input bytes at the interleaver
   input and their placement in the bit stream at the interleaver output.
   Larger numbers provide greater separation between consecutive input bytes in
   the output bit stream allowing for improved impulse noise immunity at the
   expense of payload latency.
   In the case where the ifType is Fast(125), use
   noSuchObject.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Units: milli-seconds */
   DSL_uint32_t interleaveDelay;
   /**
   Actual transmit rate on this channel.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax:   Gauge32
   - Units: bps */
   DSL_uint32_t currTxRate;
   /**
   The rate at the time of the last adslAtucRateChangeTrap event. It is also set
   at initialization to prevent a trap being sent.
   Rate changes less than adslAtucThresh(*)RateDown or less than
   adslAtucThresh(*)RateUp will not cause a trap or cause this object to change.
   (*) == Fast or Interleave.
   See AdslLineAlarmConfProfileEntry.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Max access: read-only
   - Units: bps */
   DSL_uint32_t prevTxRate;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucChanInfoFlags_t */
   DSL_uint8_t flags;
} adslAtucChanInfo_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Interleave Delay for this channel.
   Interleave delay applies only to the interleave channel and defines the
   mapping (relative spacing) between subsequent input bytes at the interleaver
   input and their placement in the bit stream at the interleaver output.
   Larger numbers provide greater separation between consecutive input bytes in
   the output bit stream allowing for improved impulse noise immunity at
   the expense of payload latency.
   In the case where the ifType is Fast(125), use noSuchObject.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: Gauge32
   - Max access: read-only
   - Units: milli-seconds */
   DSL_uint32_t interleaveDelay;
   /**
   Actual transmit rate on this channel.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Max access: read-only
   - Units: bps */
   DSL_uint32_t currTxRate;
   /**
   The rate at the time of the last adslAturRateChangeTrap event. It is also set
   at initialization to prevent a trap being sent. Rate changes less than
   adslAturThresh(*)RateDown or less than adslAturThresh(*)RateUp will not
   cause a trap or cause this object to change.
   (*) == Fast or Interleave.
   See AdslLineAlarmConfProfileEntry.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Units: bps */
   DSL_uint32_t prevTxRate;
   /**
   Indicates the length of the channel data-block on which the CRC operates.
   Refer to Line Code Specific MIBs, [11] and [12] for more information.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: Gauge32
   - Max access: read-only */
   DSL_uint32_t crcBlkLen;
   /**
   Defines a bitmask to specify which parameters shall be accessed according
   to \ref adslAturChanInfoFlags_t */
   DSL_uint8_t flags;
} adslAturChanInfo_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Count of the number of Loss of Framing failures since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucPerfLofs;
   /**
   Count of the number of Loss of Signal failures since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucPerfLoss;
   /**
   Count of the number of Errored Seconds since agent reset.
   The errored second parameter is a count of one-second intervals containing
   one or more crc anomalies, or one or more los or sef defects
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucPerfESs;
   /**
   Count of the line initialization attempts since agent reset. Includes both
   successful and failed attempts.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucPerfInits;
   /**
   The number of previous 15-minute intervals in the interval table for which
   data was collected. Given that [n] is the maximum # of intervals supported.
   The value will be [n] unless the measurement was (re-)started within the
   last ([n]*15) minutes, in which case the value will be the number of
   complete 15 minute intervals for which the agent has at least some data.
   In certain cases (e.g., in the case where the agent is a proxy) it is
   possible that some intervals are unavailable. In this case, this interval is
   the maximum interval number for which data is available.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAtucPerfValidIntervals;
   /**
   The number of intervals in the range from 0 to the value of
   adslAtucPerfValidIntervals for which no data is available. This object
   will typically be zero except in cases where the data for some intervals are
   not available (e.g., in proxy situations).
   - Numerical syntax: Integer (32 bit)
   - Base syntax:   INTEGER
   - Composed syntax:  INTEGER
   - Max access: read-only
   - Size list:  1: 0..96 */
   DSL_int_t adslAtucPerfInvalidIntervals;
   /**
   Total elapsed seconds in this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..899
   - Units: seconds */
   AdslPerfTimeElapsed adslAtucPerfCurr15MinTimeElapsed;
   /**
   Count of seconds in the current 15 minute interval when there was Loss of
   Framing.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAtucPerfCurr15MinLofs;
   /**
   Count of seconds in the current 15 minute interval when there was Loss of
   Signal.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAtucPerfCurr15MinLoss;
   /**
   Count of Errored Seconds in the current 15 minute interval.  The errored
   second parameter is a count of one-second intervals containing one or more
   crc anomalies, or one or more los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAtucPerfCurr15MinESs;
   /**
   Count of the line initialization attempts in the current 15 minute interval.
   Includes both successful and failed attempts.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAtucPerfCurr15MinInits;
   /**
   Number of seconds that have elapsed since the beginning of the current 1-day
   interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..86399
   - Units: seconds */
   AdslPerfTimeElapsed  adslAtucPerfCurr1DayTimeElapsed;
   /**
   Count of the number of seconds when there was Loss of Framing during the
   current day as measured by adslAtucPerfCurr1DayTimeElapsed.
   Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Status: current
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAtucPerfCurr1DayLofs;
   /**
   Count of the number of seconds when there was Loss of Signal during the
   current day as measured by adslAtucPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAtucPerfCurr1DayLoss;
   /**
   Count of Errored Seconds during the current day as measured by
   adslAtucPerfCurr1DayTimeElapsed. The errored second parameter is a count of
   one-second intervals containing one or more crc anomalies, or one or more
   los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAtucPerfCurr1DayESs;
   /**
   Count of the line initialization attempts in the day as measured by
   adslAtucPerfCurr1DayTimeElapsed. Includes both successful and failed attempts.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAtucPerfCurr1DayInits;
   /**
   The amount of time in the previous 1-day interval over which the performance
   monitoring information is actually counted. This value will be the same as
   the interval duration except in a situation where performance monitoring
   data could not be collected for any reason.
   Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..86400
   - Units: seconds */
   DSL_int_t adslAtucPerfPrev1DayMoniSecs;
   /**
   Count of seconds in the interval when there was Loss of Framing within the
   most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAtucPerfPrev1DayLofs;
   /**
   Count of seconds in the interval when there was Loss of Signal within the
   most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAtucPerfPrev1DayLoss;
   /**
   Count of Errored Seconds within the most recent previous 1-day period. The
   errored second parameter is a count of one-second intervals containing one
   or more crc anomalies, or one or more los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAtucPerfPrev1DayESs;
   /**
   Count of the line initialization attempts in the most
   recent previous 1-day period. Includes both successful
   and failed attempts.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAtucPerfPrev1DayInits;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucPerfDataTableFlags_t */
   DSL_uint32_t         flags;
} atucPerfDataEntry_t;

#ifdef INCLUDE_ADSL_MIB_RFC3440
/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   The value of this object reports the count of the number of fast line bs
   since last agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: line retrains
   - Reference: ITU G.997.1 Section 7.4.15.1 */
   DSL_uint32_t adslAtucPerfStatFastR;
   /**
   The value of this object reports the count of the number of failed fast line
   retrains since last agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: line retrains
   - Reference: ITU G.997.1 Section 7.4.15.2 */
   DSL_uint32_t adslAtucPerfStatFailedFastR;
   /**
   The value of this object reports the count of the number of severely errored
   seconds-line since last agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.7 */
   DSL_uint32_t adslAtucPerfStatSesL;
   /**
   The value of this object reports the count of the number of unavailable
   seconds-line since last agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.9 */
   DSL_uint32_t adslAtucPerfStatUasL;
   /**
   For the current 15-minute interval, adslAtucPerfCurr15MinFastR reports the
   current number of seconds during which there have been fast retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.4.15.1 */
   DSL_uint32_t adslAtucPerfCurr15MinFastR;
   /**
   For the current 15-minute interval, adslAtucPerfCurr15MinFailedFastR reports
   the current number of seconds during which there have been failed fast
   retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax:   Gauge32
   - Composed syntax:  PerfCurrentCount
   - Max access: read-only
   - Units:   seconds
   - Reference:  ITU G.997.1 Section 7.4.15.2 */
   DSL_uint32_t adslAtucPerfCurr15MinFailedFastR;
   /**
   For the current 15-minute interval, adslAtucPerfCurr15MinSesL reports the
   current number of seconds during which there have been severely errored
   seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax:   Gauge32
   - Composed syntax:  PerfCurrentCount
   - Max access: read-only
   - Units:   seconds
   - Reference:  ITU G.997.1 Section 7.2.1.1.7 */
   DSL_uint32_t adslAtucPerfCurr15MinSesL;
   /**
   For the current 15-minute interval, adslAtucPerfCurr15MinUasL reports the
   current number of seconds during which there have been unavailable
   seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.9 */
   DSL_uint32_t adslAtucPerfCurr15MinUasL;
   /**
   For the current day as measured by adslAtucPerfCurr1DayTimeElapsed [RFC2662],
   adslAtucPerfCurr1DayFastR reports the number of seconds during which there
   have been fast retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds
   - Reference:  ITU G.997.1 Section 7.4.15.1 */
   DSL_uint32_t adslAtucPerfCurr1DayFastR;
   /**
   For the current day as measured by adslAtucPerfCurr1DayTimeElapsed [RFC2662],
   adslAtucPerfCurr1DayFailedFastR reports the number of seconds during which
   there have been failed fast retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.4.15.2 */
   DSL_uint32_t adslAtucPerfCurr1DayFailedFastR;
   /**
   For the current day as measured by adslAtucPerfCurr1DayTimeElapsed [RFC2662],
   adslAtucPerfCurr1DaySesL reports the number of seconds during which there
   have been severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.7 */
   DSL_uint32_t adslAtucPerfCurr1DaySesL;
   /**
   For the current day as measured by adslAtucPerfCurr1DayTimeElapsed [RFC2662],
   adslAtucPerfCurr1DayUasL reports the number of seconds during which there
   have been unavailable seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.9 */
   DSL_uint32_t adslAtucPerfCurr1DayUasL;
   /**
   For the previous day, adslAtucPerfPrev1DayFastR reports the number of seconds
   during which there were fast retrains.
   Numerical syntax: Gauge (32 bit)
   Base syntax: Gauge32
   Composed syntax: AdslPerfPrevDayCount
   Max access: read-only
   Units: seconds
   Reference: ITU G.997.1 Section 7.4.15.1 */
   DSL_uint32_t adslAtucPerfPrev1DayFastR;
   /**
   For the previous day, adslAtucPerfPrev1DayFailedFastR reports the number
   of seconds during which there were failed fast retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.4.15.2 */
   DSL_uint32_t adslAtucPerfPrev1DayFailedFastR;
   /**
   For the previous day, adslAtucPerfPrev1DaySesL reports the number of seconds
   during which there were severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.7 */
   DSL_uint32_t adslAtucPerfPrev1DaySesL;
   /**
   For the previous day, adslAtucPerfPrev1DayUasL reports the number of seconds
   during which there were unavailable seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.9 */
   DSL_uint32_t adslAtucPerfPrev1DayUasL;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucPerfDataExtTableFlags_t */
   DSL_uint32_t flags;
} atucPerfDataExtEntry_t;

#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t         ifIndex;
   /**
   Count of the number of Loss of Framing failures since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAturPerfLofs;
   /**
   Count of the number of Loss of Signal failures since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAturPerfLoss;
   /**
   Count of the number of Loss of Power failures since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAturPerfLprs;
   /**
   Count of the number of Errored Seconds since agent reset. The errored
   second parameter is a count of one-second intervals containing one or more
   crc anomalies, or one or more los or sef defects.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAturPerfESs;
   /**
   The number of previous 15-minute intervals in the interval table for which
   data was collected. Given that [n] is the maximum # of intervals supported.
   The value will be [n] unless the measurement was (re-)started within the
   last ([n]*15) minutes, in which case the value will be the number of
   complete 15 minute intervals for which the agent has at least some data.
   In certain cases (e.g., in the case where the agent is a proxy) it is
   possible that some intervals are unavailable.  In this case, this interval
   is the maximum interval number for which data is available.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAturPerfValidIntervals;
   /**
   The number of intervals in the range from 0 to the value of
   adslAturPerfValidIntervals for which no data is available. This object
   will typically be zero except in cases where the data for some intervals are
   not available (e.g., in proxy situations).
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAturPerfInvalidIntervals;
   /**
   Total elapsed seconds in this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..899
   - Units: seconds */
   AdslPerfTimeElapsed  adslAturPerfCurr15MinTimeElapsed;
   /**
   Count of seconds in the current 15 minute interval when there was Loss of
   Framing.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAturPerfCurr15MinLofs;
   /**
   Count of seconds in the current 15 minute interval when there was Loss of
   Signal.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAturPerfCurr15MinLoss;
   /**
   Count of seconds in the current 15 minute interval when there was Loss of
   Power.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAturPerfCurr15MinLprs;
   /**
   Count of Errored Seconds in the current 15 minute interval. The errored
   second parameter is a count of one-second intervals containing one or more
   crc anomalies, or one or more los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds */
   PerfCurrentCount adslAturPerfCurr15MinESs;
   /**
   Number of seconds that have elapsed since the beginning of the current 1-day
   interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..86399
   - Units: seconds */
   AdslPerfTimeElapsed  adslAturPerfCurr1DayTimeElapsed;
   /**
   Count of the number of seconds when there was Loss of Framing during the
   current day as measured by adslAturPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAturPerfCurr1DayLofs;
   /**
   Count of the number of seconds when there was Loss of Signal during the
   current day as measured by adslAturPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAturPerfCurr1DayLoss;
   /**
   Count of the number of seconds when there was Loss of Power during the
   current day as measured by adslAturPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAturPerfCurr1DayLprs;
   /**
   Count of Errored Seconds during the current day as measured by
   adslAturPerfCurr1DayTimeElapsed. The errored second parameter is a count of
   one-second intervals containing one or more crc anomalies, or one or more
   los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfCurrDayCount adslAturPerfCurr1DayESs;
   /**
   The amount of time in the previous 1-day interval over which the performance
   monitoring information is actually counted. This value will be the same as
   the interval duration except in a situation where performance monitoring
   data could not be collected for any reason.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..86400
   - Units: seconds */
   DSL_int_t adslAturPerfPrev1DayMoniSecs;
   /**
   Count of seconds in the interval when there was Loss of Framing within the
   most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAturPerfPrev1DayLofs;
   /**
   Count of seconds in the interval when there was Loss of Signal within the
   most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAturPerfPrev1DayLoss;
   /**
   Count of seconds in the interval when there was Loss of Power within the most
   recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAturPerfPrev1DayLprs;
   /**
   Count of Errored Seconds within the most recent previous 1-day period. The
   errored second parameter is a count of one-second intervals containing one
   or more crc anomalies, or one or more los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds */
   AdslPerfPrevDayCount adslAturPerfPrev1DayESs;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturPerfDataTableFlags_t */
   DSL_uint32_t         flags;
} aturPerfDataEntry_t;

#ifdef INCLUDE_ADSL_MIB_RFC3440
/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   The value of this object reports the count of severely errored second-line
   since the last agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.1.7 */
   DSL_uint32_t adslAturPerfStatSesL;
   /**
   The value of this object reports the count of unavailable seconds-line since
   the last agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.9 */
   DSL_uint32_t adslAturPerfStatUasL;
   /**
   For the current 15-minute interval, adslAturPerfCurr15MinSesL reports the
   current number of seconds during which there have been severely errored
   seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.7 */
   DSL_uint32_t adslAturPerfCurr15MinSesL;
   /**
   For the current 15-minute interval, adslAturPerfCurr15MinUasL reports the
   current number of seconds during which there have been available seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.9 */
   DSL_uint32_t adslAturPerfCurr15MinUasL;
   /**
   For the current day as measured by adslAturPerfCurr1DayTimeElapsed [RFC2662],
   adslAturPerfCurr1DaySesL reports the number of seconds during which there
   have been severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.7 */
   DSL_uint32_t adslAturPerfCurr1DaySesL;
   /**
   For the current day as measured by adslAturPerfCurr1DayTimeElapsed [RFC2662],
   adslAturPerfCurr1DayUasL reports the number of seconds during which there
   have been unavailable seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.9 */
   DSL_uint32_t adslAturPerfCurr1DayUasL;
   /**
   For the previous day, adslAturPerfPrev1DaySesL reports the number of seconds
   during which there were severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.7 */
   DSL_uint32_t adslAturPerfPrev1DaySesL;
   /**
   For the previous day, adslAturPerfPrev1DayUasL reports the number of seconds
   during which there were severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only
   - Units: seconds
   - Reference: ITU G.997.1 Section 7.2.1.2.9 */
   DSL_uint32_t adslAturPerfPrev1DayUasL;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturPerfDataExtTableFlags_t */
   DSL_uint32_t flags;
} aturPerfDataExtEntry_t;
#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Performance Data Interval number 1 is the the most recent previous interval;
   interval 96 is 24 hours ago.  Intervals 2..96 are
   optional.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: not-accessible
   - Size list: 1: 1..96 */
   DSL_int_t IntervalNumber;
   /**
   Count of seconds in the interval when there was Loss of Framing.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalLOF;
   /**
   Count of seconds in the interval when there was Loss of Signal.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalLOS;
   /**
   Count of Errored Seconds in the interval. The errored second parameter is a
   count of one-second intervals containing one or more crc anomalies, or one
   or more los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalES;
   /**
   Count of the line initialization attempts during the interval. Includes both
   successful and failed attempts.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax:   Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount intervalInits;
   /**
   This variable indicates if the data for this interval is valid.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: TruthValue
   - Max access: read-only */
   DSL_int_t intervalValidData;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucIntvlInfoFlags_t */
   DSL_uint8_t flags;
} adslAtucIntvlInfo_t;

#ifdef INCLUDE_ADSL_MIB_RFC3440
/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   */
   DSL_int_t IntervalNumber;
   /**
   For the current interval, adslAtucIntervalFastR reports the current number
   of seconds during which there have been fast retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAtucIntervalFastR;
   /**
   For the each interval, adslAtucIntervalFailedFastR reports the number of
   seconds during which there have been failed fast retrains.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAtucIntervalFailedFastR;
   /**
   For the each interval, adslAtucIntervalSesL reports the number of seconds
   during which there have been severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAtucIntervalSesL;
   /**
   For the each interval, adslAtucIntervalUasL reports the number of seconds
   during which there have been unavailable seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAtucIntervalUasL;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucIntvlExtInfoFlags_t */
   DSL_uint32_t flags;
} adslAtucInvtlExtInfo_t;
#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Performance Data Interval number 1 is the the most recent previous interval;
   interval 96 is 24 hours ago.  Intervals 2..96 are optional.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: not-accessible
   - Size list: 1: 1..96 */
   DSL_int_t IntervalNumber;
   /**
   Count of seconds in the interval when there was Loss of Framing.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalLOF;
   /**
   Count of seconds in the interval when there was Loss of Signal.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalLOS;
   /**
   Count of seconds in the interval when there was Loss of Power.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalLPR;
   /**
   Count of Errored Seconds in the interval. The errored second parameter is a
   count of one-second intervals containing one or more crc anomalies, or one
   or more los or sef defects.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   PerfIntervalCount intervalES;
   /**
   This variable indicates if the data for this interval is valid.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: TruthValue
   - Max access: read-only */
   DSL_int_t intervalValidData;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturIntvlInfoFlags_t */
   DSL_uint8_t flags;
} adslAturIntvlInfo_t;

#ifdef INCLUDE_ADSL_MIB_RFC3440
/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   */
   DSL_int_t IntervalNumber;
   /**
   For the each interval, adslAturIntervalSesL reports the number of seconds
   during which there have been severely errored seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAturIntervalSesL;
   /**
   For the each interval, adslAturIntervalUasL reports the number of seconds
   during which there have been unavailable seconds-line.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only
   - Units: seconds */
   DSL_uint32_t adslAturIntervalUasL;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturIntvExtTableFlags_t */
   DSL_uint32_t flags;
} adslAturInvtlExtInfo_t;
#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Count of all encoded blocks received on this channel since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucChanReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucChanTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected since agent
   reset. These blocks are passed on as good data.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucChanCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAtucChanUncorrectBlks;
   /**
   The number of previous 15-minute intervals in the interval table for which
   data was collected. Given that [n] is the maximum # of intervals supported.
   The value will be [n] unless the measurement was (re-)started within the
   last ([n]*15) minutes, in which case the value will be the number of
   complete 15 minute intervals for which the agent has at least some data.
   In certain cases (e.g., in the case where the agent is a proxy) it is
   possible that some intervals are unavailable. In this case, this interval is
   the maximum interval number for which data is available.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax:INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAtucChanPerfValidIntervals;
   /**
   The number of intervals in the range from 0 to the value of
   adslAtucChanPerfValidIntervals for which no data is available. This object
   will typically be zero except in cases where the data for some intervals are
   not available (e.g., in proxy situations).
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAtucChanPerfInvalidIntervals;
   /**
   Total elapsed seconds in this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..899
   - Units: seconds */
   AdslPerfTimeElapsed  adslAtucChanPerfCurr15MinTimeElapsed;
   /**
   Count of all encoded blocks received on this channel within the current
   15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax:PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAtucChanPerfCurr15MinReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel within the current
   15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAtucChanPerfCurr15MinTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   within the current 15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAtucChanPerfCurr15MinCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel within
   the current 15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only
   */
   PerfCurrentCount adslAtucChanPerfCurr15MinUncorrectBlks;
   /**
   Number of seconds that have elapsed since the beginning of the current 1-day
   interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..86399
   - Units: seconds */
   AdslPerfTimeElapsed  adslAtucChanPerfCurr1DayTimeElapsed;
   /**
   Count of all encoded blocks received on this channel during the current day
   as measured by adslAtucChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAtucChanPerfCurr1DayReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel during the current
   day as measured by adslAtucChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAtucChanPerfCurr1DayTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   during the current day as measured by adslAtucChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAtucChanPerfCurr1DayCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel during
   the current day as measured by adslAtucChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAtucChanPerfCurr1DayUncorrectBlks;
   /**
   The amount of time in the previous 1-day interval over which the performance
   monitoring information is actually counted. This value will be the same as
   the interval duration except in a situation where performance monitoring data
   could not be collected for any reason.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..86400
   - Units: seconds */
   DSL_int_t adslAtucChanPerfPrev1DayMoniSecs;
   /**
   Count of all encoded blocks received on this channel within the most recent
   previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAtucChanPerfPrev1DayReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel within the most
   recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAtucChanPerfPrev1DayTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   within the most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAtucChanPerfPrev1DayCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel within
   the most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAtucChanPerfPrev1DayUncorrectBlks;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucChannelPerfDataTableFlags_t */
   DSL_uint32_t flags;
} atucChannelPerfDataEntry_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Count of all encoded blocks received on this channel since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAturChanReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAturChanTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected since agent
   reset. These blocks are passed on as good data.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAturChanCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors since agent reset.
   - Numerical syntax: Counter (32 bit)
   - Base syntax: Counter32
   - Composed syntax: Counter32
   - Max access: read-only */
   DSL_uint32_t adslAturChanUncorrectBlks;
   /**
   The number of previous 15-minute intervals in the interval table for which
   data was collected. Given that [n] is the maximum # of intervals supported.
   The value will be [n] unless the measurement was (re-)started within the last
   ([n]*15) minutes, in which case the value will be the number of complete 15
   minute intervals for which the agent has at least some data. In certain cases
   (e.g., in the case where the agent is a proxy) it is possible that some
   intervals are unavailable.  In this case, this interval is the maximum
   interval number for which data is available.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAturChanPerfValidIntervals;
   /**
   The number of intervals in the range from 0 to the value of
   adslAturChanPerfValidIntervals for which no data is available. This object
   will typically be zero except in cases where the data for some intervals are
   not available (e.g., in proxy situations).
   - Numerical syntax:   Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..96 */
   DSL_int_t adslAturChanPerfInvalidIntervals;
   /**
   Total elapsed seconds in this interval. A full interval is 900 seconds.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..899
   - Units: seconds */
   AdslPerfTimeElapsed  adslAturChanPerfCurr15MinTimeElapsed;
   /**
   Count of all encoded blocks received on this channel within the current 15
   minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAturChanPerfCurr15MinReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel within the current
   15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   -Max access: read-only */
   PerfCurrentCount adslAturChanPerfCurr15MinTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   within the current 15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAturChanPerfCurr15MinCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel within
   the current 15 minute interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfCurrentCount
   - Max access: read-only */
   PerfCurrentCount adslAturChanPerfCurr15MinUncorrectBlks;
   /**
   Number of seconds that have elapsed since the beginning of the current 1-day
   interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfTimeElapsed
   - Max access: read-only
   - Size list: 1: 0..86399
   - Units: seconds */
   AdslPerfTimeElapsed  adslAturChanPerfCurr1DayTimeElapsed;
   /**
   Count of all encoded blocks received on this channel during the current day
   as measured by adslAturChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAturChanPerfCurr1DayReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel during the current
   day as measured by adslAturChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAturChanPerfCurr1DayTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   during the current day as measured by adslAturChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAturChanPerfCurr1DayCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel during
   the current day as measured by adslAturChanPerfCurr1DayTimeElapsed.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfCurrDayCount
   - Max access: read-only */
   AdslPerfCurrDayCount adslAturChanPerfCurr1DayUncorrectBlks;
   /**
   The amount of time in the previous 1-day interval over which the performance
   monitoring information is actually counted. This value will be the same as
   the interval duration except in a situation where performance monitoring data
   could not be collected for any reason.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-only
   - Size list: 1: 0..86400
   - Units: seconds */
   DSL_int_t adslAturChanPerfPrev1DayMoniSecs;
   /**
   Count of all encoded blocks received on this channel within the most recent
   previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAturChanPerfPrev1DayReceivedBlks;
   /**
   Count of all encoded blocks transmitted on this channel within the most
   recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAturChanPerfPrev1DayTransmittedBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   within the most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAturChanPerfPrev1DayCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel within
   the most recent previous 1-day period.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: AdslPerfPrevDayCount
   - Max access: read-only */
   AdslPerfPrevDayCount adslAturChanPerfPrev1DayUncorrectBlks;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturChannelPerfDataTableFlags_t */
   DSL_uint32_t flags;
} aturChannelPerfDataEntry_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Performance Data Interval number 1 is the the most recent previous interval;
   interval 96 is 24 hours ago. Intervals 2..96 are optional.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: not-accessible
   - Size list: 1: 1..96 */
   DSL_int_t IntervalNumber;
   /**
   Count of all encoded blocks received on this channel during this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalRecvdBlks;
   /**
   Count of all encoded blocks transmitted on this channel during this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalXmitBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   during this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel during
   this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalUncorrectBlks;
   /**
   This variable indicates if the data for this interval is valid.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: TruthValue
   - Max access: read-only */
   DSL_int_t intervalValidData;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAtucChanIntvlInfoFlags_t */
   DSL_uint8_t flags;
} adslAtucChanIntvlInfo_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET */
typedef struct
{
   /**
      Table index */
   DSL_int_t ifIndex;
   /**
   Performance Data Interval number 1 is the the most recent previous interval;
   interval 96 is 24 hours ago. Intervals 2..96 are optional.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: not-accessible
   - Size list: 1: 1..96 */
   DSL_int_t IntervalNumber;
   /**
   Count of all encoded blocks received on this channel during this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalRecvdBlks;
   /**
   Count of all encoded blocks transmitted on this channel during this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalXmitBlks;
   /**
   Count of all blocks received with errors that were corrected on this channel
   during this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalCorrectedBlks;
   /**
   Count of all blocks received with uncorrectable errors on this channel during
   this interval.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Gauge32
   - Composed syntax: PerfIntervalCount
   - Max access: read-only */
   PerfIntervalCount chanIntervalUncorrectBlks;
   /**
   This variable indicates if the data for this interval is valid.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: TruthValue
   - Max access: read-only */
   DSL_int_t intervalValidData;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturChanIntvlInfoFlags_t */
   DSL_uint8_t flags;
} adslAturChanIntvlInfo_t;

/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET and
   \ref DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_SET */
typedef struct
{
   /**
   This object is used by the line alarm configuration table in order to
   identify a row of this table.
   When `dynamic' profiles are implemented, the profile name is user specified.
   Also, the system will always provide a default profile whose name is `DEFVAL'.
   When `static' profiles are implemented, there is an one-to-one relationship
   between each line and its profile.  In which case, the profile name will
   need to algorithmicly represent the Line's ifIndex. Therefore, the profile's
   name is a decimalized string of the ifIndex that is fixed-length (i.e., 10)
   with leading zero(s).  For example, the profile name for ifIndex which
   equals '15' will be '0000000015'.
   - Numerical syntax: Octets
   - Base syntax: OCTET STRING
   - Composed syntax: SnmpAdminString
   - Max access: not-accessible
   - Size list: 1: 1..32 */
   DSL_uint8_t adslLineAlarmConfProfileName[32];
   /**
   The number of Loss of Frame Seconds encountered by an ADSL interface within
   any given 15 minutes performance data collection period, which causes the
   SNMP agent to send an adslAtucPerfLofsThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will
   disable the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list: 1: 0..900
   - Units: seconds */
   DSL_int_t adslAtucThresh15MinLofs;
   /**
   The number of Loss of Signal Seconds encountered by an ADSL interface within
   any given 15 minutes performance data collection period, which causes the
   SNMP agent to send an adslAtucPerfLossThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will disable
   the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list: 1: 0..900
   - Units: seconds */
   DSL_int_t adslAtucThresh15MinLoss;
   /**
   The number of Errored Seconds encountered by an ADSL interface within any
   given 15 minutes performance data collection period, which causes the SNMP
   agent to send an adslAtucPerfESsThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will disable
   the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list: 1: 0..900
   - Units: seconds */
   DSL_int_t adslAtucThresh15MinESs;
   /**
   Applies to `Fast' channels only. Configured change in rate causing an
   adslAtucRateChangeTrap. A trap is produced when:
   ChanCurrTxRate >= ChanPrevTxRate plus the value of this object.
   A value of `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAtucThreshFastRateUp;
   /**
   Applies to `Interleave' channels only. Configured change in rate causing an
   adslAtucRateChangeTrap.  A trap is produced when:
   ChanCurrTxRate >= ChanPrevTxRate plus the value of this object.
   A value of `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAtucThreshInterleaveRateUp;
   /**
   Applies to `Fast' channels only. Configured change in rate causing an
   adslAtucRateChangeTrap.  A trap is produced when:
   ChanCurrTxRate <= ChanPrevTxRate minus the value of
   this object. A value of `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAtucThreshFastRateDown;
   /**
   Applies to `Interleave' channels only. Configured change in rate causing an
   adslAtucRateChangeTrap.  A trap is produced when:
   ChanCurrTxRate <= ChanPrevTxRate minus the value of this object.
   A value of `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAtucThreshInterleaveRateDown;
   /**
   Enables and disables the InitFailureTrap. This object is defaulted disable(2).
   - Numerical syntax: Integer (32 bit)
   - Base syntax:   INTEGER
   - Composed syntax:  INTEGER
   - Max access: read-create
   - Value list:
     - 1: enable(1)
     - 2: disable(2)
   - Default values: 1: disable (name) */
   DSL_int_t adslAtucInitFailureTrapEnable;
   /**
   The number of Loss of Frame Seconds encountered by an ADSL interface within
   any given 15 minutes performance data collection period, which causes the
   SNMP agent to send an adslAturPerfLofsThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will disable
   the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list:  1: 0..900
   - Units: seconds */
   DSL_int_t adslAturThresh15MinLofs;
   /**
   The number of Loss of Signal Seconds encountered by an ADSL interface within
   any given 15 minutes performance data collection period, which causes the
   SNMP agent to send an adslAturPerfLossThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will disable
   the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list: 1: 0..900
   - Units: seconds */
   DSL_int_t adslAturThresh15MinLoss;
   /**
   The number of Loss of Power Seconds encountered by an ADSL interface within
   any given 15 minutes performance data collection period, which causes the
   SNMP agent to send an adslAturPerfLprsThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will disable
   the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list: 1: 0..900
   - Units: seconds */
   DSL_int_t adslAturThresh15MinLprs;
   /**
   The number of Errored Seconds encountered by an ADSL interface within any
   given 15 minutes performance data collection period, which causes the SNMP
   agent to send an adslAturPerfESsThreshTrap.
   One trap will be sent per interval per interface. A value of `0' will disable
   the trap.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: INTEGER
   - Max access: read-create
   - Size list: 1: 0..900
   - Units: seconds */
   DSL_int_t adslAturThresh15MinESs;
   /**
   Applies to `Fast' channels only. Configured change in rate causing an
   adslAturRateChangeTrap. A trap is produced when:
   ChanCurrTxRate >= ChanPrevTxRate plus the value of this object. A value of
   `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAturThreshFastRateUp;
   /**
   Applies to `Interleave' channels only. configured change in rate causing an
   adslAturRateChangeTrap.  A trap is produced when:
   ChanCurrTxRate >= ChanPrevTxRate plus the value of this object. A value of
   `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAturThreshInterleaveRateUp;
   /**
   Applies to `Fast' channels only. Configured change in rate causing an
   adslAturRateChangeTrap.  A trap is produced when:
   ChanCurrTxRate <= ChanPrevTxRate minus the value of this object. A value of
   `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAturThreshFastRateDown;
   /**
   Applies to `Interleave' channels only. Configured change in rate causing an
   adslAturRateChangeTrap.  A trap is produced when:
   ChanCurrTxRate <= ChanPrevTxRate minus the value of this object. A value of
   `0' will disable the trap.
   - Numerical syntax: Gauge (32 bit)
   - Base syntax: Unsigned32
   - Composed syntax: Unsigned32
   - Max access: read-create
   - Units: bps */
   DSL_uint32_t adslAturThreshInterleaveRateDown;
   /**
   This object is used to create a new row or modify or delete an existing row
   in this table.
   A profile activated by setting this object to `active'.  When `active' is
   set, the system will validate the profile.
   Before a profile can be deleted or taken out of service, (by setting this
   object to `destroy' or `outOfService') it must be first unreferenced from
   all associated lines.
   If the implementator of this MIB has chosen not to implement
   `dynamic assignment' of profiles, this object's MIN-ACCESS is read-only and
   its value is always to be `active'.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: INTEGER
   - Composed syntax: RowStatus
   - Max access: read-create */
   DSL_int_t adslLineAlarmConfProfileRowStatus;
   /**
      Defines a bitmask to specify which parameters shall be accessed according
      to \ref adslAturLineAlarmConfProfileTableFlags_t */
   DSL_uint32_t flags;
} adslLineAlarmConfProfileEntry_t;

#ifdef INCLUDE_ADSL_MIB_RFC3440
/**
   Structure that defines all parameters that are used in ioctl
   \ref DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET and
   \ref DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_SET */
typedef struct
{
   /**
   */
   DSL_uint8_t adslLineAlarmConfProfileExtName[32];
   /**
   The first time the value of the corresponding instance of
   adslAtucPerfCurr15MinFailedFastR reaches or exceeds this value within a given
   15-minute performance data collection period, an adslAtucFailedFastRThreshTrap
   notification will be generated. The value '0' will disable the notification.
   The default value of this object is '0'.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: Integer32
   - Composed syntax: Integer32
   - Max access: read-create
   - Size list: 1: 0..900
   - Default values: 1: 0 (DSL_int_t)")
   - Units: seconds */
   DSL_uint32_t adslAtucThreshold15MinFailedFastR;
   /**
   The first time the value of the corresponding instance of adslAtucPerf15MinSesL
   reaches or exceeds this value within a given 15-minute performance data
   collection period, an adslAtucSesLThreshTrap notification will be generated.
   The value '0' will disable the notification.  The default value of this
   object is '0'.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: Integer32
   - Composed syntax: Integer32
   - Max access: read-create
   - Size list: 1: 0..900
   - Default values: 1: 0 (DSL_int_t)")
   - Units: seconds */
   DSL_uint32_t adslAtucThreshold15MinSesL;
   /**
   The first time the value of the corresponding instance of adslAtucPerf15MinUasL
   reaches or exceeds this value within a given 15-minute performance data
   collection period, an adslAtucUasLThreshTrap notification will be generated.
   The value '0' will disable the notification.  The default value of this
   object is '0'.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: Integer32
   - Composed syntax: Integer32
   - Max access: read-create
   - Size list: 1: 0..900
   - Default values: 1: 0 (DSL_int_t)")
   - Units: seconds */
   DSL_uint32_t adslAtucThreshold15MinUasL;
   /**
   The first time the value of the corresponding instance of adslAturPerf15MinSesL
   reaches or exceeds this value within a given 15-minute performance data
   collection period, an adslAturSesLThreshTrap notification will be generated.
   The value '0' will disable the notification.  The default value of this
   object is '0'.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: Integer32
   - Composed syntax: Integer32
   - Max access: read-create
   - Size list: 1: 0..900
   - Default values: 1: 0 (DSL_int_t)")
   - Units: seconds */
   DSL_uint32_t adslAturThreshold15MinSesL;
   /**
   The first time the value of the corresponding instance of adslAturPerf15MinUasL
   reaches or exceeds this value within a given 15-minute performance data
   collection period, an adslAturUasLThreshTrap notification will be generated.
   The value '0' will disable the notification.  The default value of this
   object is '0'.
   - Numerical syntax: Integer (32 bit)
   - Base syntax: Integer32
   - Composed syntax: Integer32
   - Max access: read-create
   - Size list: 1: 0..900
   - Default values: 1: 0 (DSL_int_t)")
   - Units: seconds */
   DSL_uint32_t adslAturThreshold15MinUasL;
   /**
   Defines a bitmask to specify which parameters shall be accessed according
   to \ref adslLineAlarmConfProfileExtTableFlags_t */
   DSL_uint32_t flags;
} adslLineAlarmConfProfileExtEntry_t;
#endif /* INCLUDE_ADSL_MIB_RFC3440 */
/* TRAPS */

/** @} DRV_DSL_CPE_ADSL_MIB */

#endif /* INCLUDE_DSL_ADSL_MIB */

#endif /* (INCLUDE_DSL_CPE_API_ADSL_SUPPORT == 1) */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_API_ADSLMIB_H */
