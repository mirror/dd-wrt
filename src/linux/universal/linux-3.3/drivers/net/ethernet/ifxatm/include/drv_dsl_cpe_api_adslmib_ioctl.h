/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_ADSLMIB_IOCTL_H
#define _DRV_DSL_CPE_API_ADSLMIB_IOCTL_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"

#if (INCLUDE_DSL_CPE_API_ADSL_SUPPORT == 1)

#ifdef INCLUDE_DSL_ADSL_MIB

/** \file
   This file defines the MIB ioctl interface for DSL CPE API in case of using
   RFC2662 and RFC3440
*/

/** \addtogroup DRV_DSL_CPE_ADSL_MIB
 @{ */


/* ************************************************************************** */
/* *** ioctl structures for ADSL SNMP MIB                                 *** */
/* ************************************************************************** */

/**
   encapsulate all ADSL SNMP MIB ioctl command arguments
*/
typedef union
{
   adslLineTableEntry_t               adslLineTableEntry_pt;
   adslAtucPhysEntry_t                adslAtucPhysEntry_pt;
   adslAturPhysEntry_t                adslAturPhysEntry_pt;
   adslAtucChanInfo_t                 adslAtucChanInfo_pt;
   adslAturChanInfo_t                 adslAturChanInfo_pt;
   atucPerfDataEntry_t                atucPerfDataEntry_pt;
   aturPerfDataEntry_t                aturPerfDataEntry_pt;
   adslAtucIntvlInfo_t                adslAtucIntvlInfo_pt;
   adslAturIntvlInfo_t                adslAturIntvlInfo_pt;
   atucChannelPerfDataEntry_t         atucChannelPerfDataEntry_pt;
   aturChannelPerfDataEntry_t         aturChannelPerfDataEntry_pt;
   adslAtucChanIntvlInfo_t            adslAtucChanIntvlInfo_pt;
   adslAturChanIntvlInfo_t            adslAturChanIntvlInfo_pt;
   adslLineAlarmConfProfileEntry_t    adslLineAlarmConfProfileEntry_pt;
   adslAturTrapsFlags_t               adslAturTrapsFlags_pt;

   #ifdef INCLUDE_ADSL_MIB_RFC3440
   adslLineExtTableEntry_t            adslLineExtTableEntry_pt;
   atucPerfDataExtEntry_t             atucPerfDataExtEntry_pt;
   adslAtucInvtlExtInfo_t             adslAtucInvtlExtInfo_pt;
   aturPerfDataExtEntry_t             aturPerfDataExtEntry_pt;
   adslAturInvtlExtInfo_t             adslAturInvtlExtInfo_pt;
   adslLineAlarmConfProfileExtEntry_t adslLineAlarmConfProfileExtEntry_pt;
   adslAturExtTrapsFlags_t            adslAturExtTrapsFlags_pt;
   #endif /* INCLUDE_ADSL_MIB_RFC3440 */
} DSL_IOCTL_MIB_arg_t;


/* ************************************************************************** */
/* * Ioctl interface definitions for ADSL SNMP MIB                          * */
/* ************************************************************************** */

/**
   This table includes common attributes describing both ends of the line.
   It is required for all ADSL physical interfaces. ADSL physical interfaces
   are those ifEntries where ifType is equal to adsl(94).

   CLI
   - long command: MIB_LineEntryGet
   - short command: mibleg

   \param adslLineTableEntry_t*
      The parameter points to a \ref adslLineTableEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslLineTableEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_LINE_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslLineTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.1\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslLineTable(1)\n
      Module:           ADSL-LINE-MIB (RFC2662)\n
      Parent:           adslMibObjects\n
      First child:      adslLineEntry\n
      Next sibling:     adslAtucPhysTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslLineEntry\n
      Composed syntax:  SEQUENCE OF AdslLineEntry\n
      Max access:       not-accessible\n
      Sequences:\n
        1: adslLineCoding - AdslLineCodingType(2 - integer (32 bit))\n
        2: adslLineType - INTEGER(2 - integer (32 bit))\n
        3: adslLineSpecific - VariablePointer(6 - object identifier)\n
        4: adslLineConfProfile - SnmpAdminString(4 - octets)\n
        5: adslLineAlarmConfProfile - SnmpAdminString(4 - octets)\n
*/
#define DSL_FIO_MIB_ADSL_LINE_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 1, adslLineTableEntry_t )


/**
   This table provides one row for each ATUC. Each row contains the Physical
   Layer Parameters table for that ATUC. ADSL physical interfaces are those
   ifEntries where ifType is equal to adsl(94).

   CLI
   - long command: MIB_ATUC_PhysEntryGet
   - short command: mibatucpeg

   \param adslAtucPhysEntry_t*
      The parameter points to a \ref adslAtucPhysEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAtucPhysEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucPhysTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.2\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAtucPhysTable(2)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAtucPhysEntry\n
      Prev sibling:     adslLineTable\n
      Next sibling:     adslAturPhysTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucPhysEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucPhysEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucInvSerialNumber - SnmpAdminString(4 - octets)\n
         2: adslAtucInvVendorID - SnmpAdminString(4 - octets)\n
         3: adslAtucInvVersionNumber - SnmpAdminString(4 - octets)\n
         4: adslAtucCurrSnrMgn - INTEGER(2 - integer (32 bit))\n
         5: adslAtucCurrAtn - Gauge32(66 - gauge (32 bit))\n
         6: adslAtucCurrStatus - BITS(4 - octets)\n
         7: adslAtucCurrOutputPwr - INTEGER(2 - integer (32 bit))\n
         8: adslAtucCurrAttainableRate - Gauge32(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 2, adslAtucPhysEntry_t )

/**
   This table provides one row for each ATUR. Each row contains the Physical
   Layer Parameters table for that ATUR. ADSL physical interfaces are those
   ifEntries where ifType is equal to adsl(94).

   CLI
   - long command: MIB_ATUR_PhysEntryGet
   - short command: mibaturpeg

   \param adslAturPhysEntry_t*
      The parameter points to a \ref adslAturPhysEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \remarks
      Name:             adslAturPhysTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.3\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAturPhysTable(3)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAturPhysEntry\n
      Prev sibling:     adslAtucPhysTable\n
      Next sibling:     adslAtucChanTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturPhysEntry\n
      Composed syntax:  SEQUENCE OF AdslAturPhysEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturInvSerialNumber - SnmpAdminString(4 - octets)\n
         2: adslAturInvVendorID - SnmpAdminString(4 - octets)\n
         3: adslAturInvVersionNumber - SnmpAdminString(4 - octets)\n
         4: adslAturCurrSnrMgn - INTEGER(2 - integer (32 bit))\n
         5: adslAturCurrAtn - Gauge32(66 - gauge (32 bit))\n
         6: adslAturCurrStatus - BITS(4 - octets)\n
         7: adslAturCurrOutputPwr - INTEGER(2 - integer (32 bit))\n
         8: adslAturCurrAttainableRate - Gauge32(66 - gauge (32 bit))\n

   \code
      adslAturPhysEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode */
#define DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 3, adslAturPhysEntry_t )

/**
   This table provides one row for each ATUC channel. ADSL channel interfaces
   are those ifEntries where ifType is equal to adslInterleave(124) or
   adslFast(125).

   CLI
   - long command: MIB_ATUC_ChanEntryGet
   - short command: mibatucceg

   \param adslAtucChanInfo_t*
      The parameter points to a \ref adslAtucChanInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAtucChanInfo_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucChanTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.4\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAtucChanTable(4)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAtucChanEntry\n
      Prev sibling:     adslAturPhysTable\n
      Next sibling:     adslAturChanTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucChanEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucChanEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucChanInterleaveDelay - Gauge32(66 - gauge (32 bit))\n
         2: adslAtucChanCurrTxRate - Gauge32(66 - gauge (32 bit))\n
         3: adslAtucChanPrevTxRate - Gauge32(66 - gauge (32 bit))\n
         4: adslAtucChanCrcBlockLength - Gauge32(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 4, adslAtucChanInfo_t )

/**
   This table provides one row for each ATUR channel. ADSL channel interfaces
   are those ifEntries where ifType is equal to adslInterleave(124) or
   adslFast(125).

   CLI
   - long command: MIB_ATUR_ChanEntryGet
   - short command: mibaturceg

   \param adslAturChanInfo_t*
      The parameter points to a \ref adslAturChanInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAturChanInfo_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturChanTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.5\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAturChanTable(5)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAturChanEntry\n
      Prev sibling:     adslAtucChanTable\n
      Next sibling:     adslAtucPerfDataTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturChanEntry\n
      Composed syntax:  SEQUENCE OF AdslAturChanEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturChanInterleaveDelay - Gauge32(66 - gauge (32 bit))\n
         2: adslAturChanCurrTxRate - Gauge32(66 - gauge (32 bit))\n
         3: adslAturChanPrevTxRate - Gauge32(66 - gauge (32 bit))\n
         4: adslAturChanCrcBlockLength - Gauge32(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 5, adslAturChanInfo_t )

/**
   This table provides one row for each ATUC. ADSL physical interfaces are
   those ifEntries where ifType is equal to adsl(94).

   CLI
   - long command: MIB_ATUC_PerfDataEntryGet
   - short command: mibatucpdeg

   \param atucPerfDataEntry_t*
      The parameter points to a \ref atucPerfDataEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      atucPerfDataEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucPerfDataTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.6\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAtucPerfDataTable(6)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAtucPerfDataEntry\n
      Prev sibling:     adslAturChanTable\n
      Next sibling:     adslAturPerfDataTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucPerfDataEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucPerfDataEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucPerfLofs - Counter32(65 - counter (32 bit))\n
         2: adslAtucPerfLoss - Counter32(65 - counter (32 bit))\n
         3: adslAtucPerfLols - Counter32(65 - counter (32 bit))\n
         4: adslAtucPerfLprs - Counter32(65 - counter (32 bit))\n
         5: adslAtucPerfESs - Counter32(65 - counter (32 bit))\n
         6: adslAtucPerfInits - Counter32(65 - counter (32 bit))\n
         7: adslAtucPerfValidIntervals - INTEGER(2 - integer (32 bit))\n
         8: adslAtucPerfInvalidIntervals - INTEGER(2 - integer (32 bit))\n
         9: adslAtucPerfCurr15MinTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         10: adslAtucPerfCurr15MinLofs - PerfCurrentCount(66 - gauge (32 bit))\n
         11: adslAtucPerfCurr15MinLoss - PerfCurrentCount(66 - gauge (32 bit))\n
         12: adslAtucPerfCurr15MinLols - PerfCurrentCount(66 - gauge (32 bit))\n
         13: adslAtucPerfCurr15MinLprs - PerfCurrentCount(66 - gauge (32 bit))\n
         14: adslAtucPerfCurr15MinESs - PerfCurrentCount(66 - gauge (32 bit))\n
         15: adslAtucPerfCurr15MinInits - PerfCurrentCount(66 - gauge (32 bit))\n
         16: adslAtucPerfCurr1DayTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         17: adslAtucPerfCurr1DayLofs - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         18: adslAtucPerfCurr1DayLoss - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         19: adslAtucPerfCurr1DayLols - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         20: adslAtucPerfCurr1DayLprs - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         21: adslAtucPerfCurr1DayESs - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         22: adslAtucPerfCurr1DayInits - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         23: adslAtucPerfPrev1DayMoniSecs - INTEGER(2 - integer (32 bit))\n
         24: adslAtucPerfPrev1DayLofs - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         25: adslAtucPerfPrev1DayLoss - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         26: adslAtucPerfPrev1DayLols - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         27: adslAtucPerfPrev1DayLprs - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         28: adslAtucPerfPrev1DayESs - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         29: adslAtucPerfPrev1DayInits - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 6, atucPerfDataEntry_t )

/**
   This table provides one row for each ATUR. ADSL physical interfaces are
   those ifEntries where ifType is equal to adsl(94).

   CLI
   - long command: MIB_ATUC_PerfDataEntryGet
   - short command: mibatucpdeg

   \param aturPerfDataEntry_t*
      The parameter points to a \ref aturPerfDataEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      aturPerfDataEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturPerfDataTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.7\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAturPerfDataTable(7)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAturPerfDataEntry\n
      Prev sibling:     adslAtucPerfDataTable\n
      Next sibling:     adslAtucIntervalTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturPerfDataEntry\n
      Composed syntax:  SEQUENCE OF AdslAturPerfDataEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturPerfLofs - Counter32(65 - counter (32 bit))\n
         2: adslAturPerfLoss - Counter32(65 - counter (32 bit))\n
         3: adslAturPerfLprs - Counter32(65 - counter (32 bit))\n
         4: adslAturPerfESs - Counter32(65 - counter (32 bit))\n
         5: adslAturPerfValidIntervals - INTEGER(2 - integer (32 bit))\n
         6: adslAturPerfInvalidIntervals - INTEGER(2 - integer (32 bit))\n
         7: adslAturPerfCurr15MinTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         8: adslAturPerfCurr15MinLofs - PerfCurrentCount(66 - gauge (32 bit))\n
         9: adslAturPerfCurr15MinLoss - PerfCurrentCount(66 - gauge (32 bit))\n
         10: adslAturPerfCurr15MinLprs - PerfCurrentCount(66 - gauge (32 bit))\n
         11: adslAturPerfCurr15MinESs - PerfCurrentCount(66 - gauge (32 bit))\n
         12: adslAturPerfCurr1DayTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         13: adslAturPerfCurr1DayLofs - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         14: adslAturPerfCurr1DayLoss - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         15: adslAturPerfCurr1DayLprs - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         16: adslAturPerfCurr1DayESs - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         17: adslAturPerfPrev1DayMoniSecs - INTEGER(2 - integer (32 bit))\n
         18: adslAturPerfPrev1DayLofs - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         19: adslAturPerfPrev1DayLoss - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         20: adslAturPerfPrev1DayLprs - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         21: adslAturPerfPrev1DayESs - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 7, aturPerfDataEntry_t )

/**
   This table provides one row for each ATUC performance data collection
   interval. ADSL physical interfaces are those ifEntries where ifType is equal
   to adsl(94).

   CLI
   - long command: MIB_ATUC_IntervalEntryGet
   - short command: mibatucieg

   \param adslAtucIntvlInfo_t*
      The parameter points to a \ref adslAtucIntvlInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAtucIntvlInfo_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucIntervalTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.8\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAtucIntervalTable(8)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAtucIntervalEntry\n
      Prev sibling:     adslAturPerfDataTable\n
      Next sibling:     adslAturIntervalTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucIntervalEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucIntervalEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucIntervalNumber - INTEGER(2 - integer (32 bit))\n
         2: adslAtucIntervalLofs - PerfIntervalCount(66 - gauge (32 bit))\n
         3: adslAtucIntervalLoss - PerfIntervalCount(66 - gauge (32 bit))\n
         4: adslAtucIntervalLols - PerfIntervalCount(66 - gauge (32 bit))\n
         5: adslAtucIntervalLprs - PerfIntervalCount(66 - gauge (32 bit))\n
         6: adslAtucIntervalESs - PerfIntervalCount(66 - gauge (32 bit))\n
         7: adslAtucIntervalInits - PerfIntervalCount(66 - gauge (32 bit))\n
         8: adslAtucIntervalValidData - TruthValue(2 - integer (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 8, adslAtucIntvlInfo_t )

/**
   This table provides one row for each ATUC performance data collection
   interval. ADSL physical interfaces are those ifEntries where ifType is equal
   to adsl(94).

   CLI
   - long command: MIB_ATUR_IntervalEntryGet
   - short command: mibaturieg

   \param adslAturIntvlInfo_t*
      The parameter points to a \ref adslAturIntvlInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAturIntvlInfo_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturIntervalTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.9\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAturIntervalTable(9)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAturIntervalEntry\n
      Prev sibling:     adslAtucIntervalTable\n
      Next sibling:     adslAtucChanPerfDataTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturIntervalEntry\n
      Composed syntax:  SEQUENCE OF AdslAturIntervalEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturIntervalNumber - INTEGER(2 - integer (32 bit))\n
         2: adslAturIntervalLofs - PerfIntervalCount(66 - gauge (32 bit))\n
         3: adslAturIntervalLoss - PerfIntervalCount(66 - gauge (32 bit))\n
         4: adslAturIntervalLprs - PerfIntervalCount(66 - gauge (32 bit))\n
         5: adslAturIntervalESs - PerfIntervalCount(66 - gauge (32 bit))\n
         6: adslAturIntervalValidData - TruthValue(2 - integer (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 9, adslAturIntvlInfo_t )

/**
   This table provides one row for each ATUC channel. ADSL channel interfaces
   are those ifEntries where ifType is equal to adslInterleave(124) or
   adslFast(125).

   CLI
   - long command: MIB_ATUC_ChanPerfDataEntryGet
   - short command: mibatuccpdeg

   \param atucChannelPerfDataEntry_t*
      The parameter points to a \ref atucChannelPerfDataEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      atucChannelPerfDataEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucChanPerfDataTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.10\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAtucChanPerfDataTable(10)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAtucChanPerfDataEntry\n
      Prev sibling:     adslAturIntervalTable\n
      Next sibling:     adslAturChanPerfDataTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucChanPerfDataEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucChanPerfDataEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucChanReceivedBlks - Counter32(65 - counter (32 bit))\n
         2: adslAtucChanTransmittedBlks - Counter32(65 - counter (32 bit))\n
         3: adslAtucChanCorrectedBlks - Counter32(65 - counter (32 bit))\n
         4: adslAtucChanUncorrectBlks - Counter32(65 - counter (32 bit))\n
         5: adslAtucChanPerfValidIntervals - INTEGER(2 - integer (32 bit))\n
         6: adslAtucChanPerfInvalidIntervals - INTEGER(2 - integer (32 bit))\n
         7: adslAtucChanPerfCurr15MinTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         8: adslAtucChanPerfCurr15MinReceivedBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         9: adslAtucChanPerfCurr15MinTransmittedBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         10: adslAtucChanPerfCurr15MinCorrectedBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         11: adslAtucChanPerfCurr15MinUncorrectBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         12: adslAtucChanPerfCurr1DayTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         13: adslAtucChanPerfCurr1DayReceivedBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         14: adslAtucChanPerfCurr1DayTransmittedBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         15: adslAtucChanPerfCurr1DayCorrectedBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         16: adslAtucChanPerfCurr1DayUncorrectBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         17: adslAtucChanPerfPrev1DayMoniSecs - INTEGER(2 - integer (32 bit))\n
         18: adslAtucChanPerfPrev1DayReceivedBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         19: adslAtucChanPerfPrev1DayTransmittedBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         20: adslAtucChanPerfPrev1DayCorrectedBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         21: adslAtucChanPerfPrev1DayUncorrectBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 10, atucChannelPerfDataEntry_t )

/**
   This table provides one row for each ATUR channel. ADSL channel interfaces
   are those ifEntries where ifType is equal to adslInterleave(124) or
   adslFast(125).

   CLI
   - long command: MIB_ATUR_ChanPerfDataEntryGet
   - short command: mibaturcpdeg

   \param aturChannelPerfDataEntry_t*
      The parameter points to a \ref aturChannelPerfDataEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      aturChannelPerfDataEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturChanPerfDataTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.11\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAturChanPerfDataTable(11)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAturChanPerfDataEntry\n
      Prev sibling:     adslAtucChanPerfDataTable\n
      Next sibling:     adslAtucChanIntervalTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturChanPerfDataEntry\n
      Composed syntax:  SEQUENCE OF AdslAturChanPerfDataEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturChanReceivedBlks - Counter32(65 - counter (32 bit))\n
         2: adslAturChanTransmittedBlks - Counter32(65 - counter (32 bit))\n
         3: adslAturChanCorrectedBlks - Counter32(65 - counter (32 bit))\n
         4: adslAturChanUncorrectBlks - Counter32(65 - counter (32 bit))\n
         5: adslAturChanPerfValidIntervals - INTEGER(2 - integer (32 bit))\n
         6: adslAturChanPerfInvalidIntervals - INTEGER(2 - integer (32 bit))\n
         7: adslAturChanPerfCurr15MinTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         8: adslAturChanPerfCurr15MinReceivedBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         9: adslAturChanPerfCurr15MinTransmittedBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         10: adslAturChanPerfCurr15MinCorrectedBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         11: adslAturChanPerfCurr15MinUncorrectBlks - PerfCurrentCount(66 - gauge (32 bit))\n
         12: adslAturChanPerfCurr1DayTimeElapsed - AdslPerfTimeElapsed(66 - gauge (32 bit))\n
         13: adslAturChanPerfCurr1DayReceivedBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         14: adslAturChanPerfCurr1DayTransmittedBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         15: adslAturChanPerfCurr1DayCorrectedBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         16: adslAturChanPerfCurr1DayUncorrectBlks - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         17: adslAturChanPerfPrev1DayMoniSecs - INTEGER(2 - integer (32 bit))\n
         18: adslAturChanPerfPrev1DayReceivedBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         19: adslAturChanPerfPrev1DayTransmittedBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         20: adslAturChanPerfPrev1DayCorrectedBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         21: adslAturChanPerfPrev1DayUncorrectBlks - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 11, aturChannelPerfDataEntry_t )

/**
   This table provides one row for each ATUC channel's performance data
   collection interval. ADSL channel interfaces are those ifEntries where
   ifType is equal to adslInterleave(124) or adslFast(125).

   CLI
   - long command: MIB_ATUC_ChanIntervalEntryGet
   - short command: mibatuccieg

   \param adslAtucChanIntvlInfo_t*
      The parameter points to a \ref adslAtucChanIntvlInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAtucChanIntvlInfo_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucChanIntervalTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.12\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAtucChanIntervalTable(12)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAtucChanIntervalEntry\n
      Prev sibling:     adslAturChanPerfDataTable\n
      Next sibling:     adslAturChanIntervalTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucChanIntervalEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucChanIntervalEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucChanIntervalNumber - INTEGER(2 - integer (32 bit))\n
         2: adslAtucChanIntervalReceivedBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         3: adslAtucChanIntervalTransmittedBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         4: adslAtucChanIntervalCorrectedBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         5: adslAtucChanIntervalUncorrectBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         6: adslAtucChanIntervalValidData - TruthValue(2 - integer (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 12, adslAtucChanIntvlInfo_t )

/**
   This table provides one row for each ATUR channel's performance data
   collection interval. ADSL channel interfaces are those ifEntries where
   ifType is equal to adslInterleave(124) or adslFast(125).

   CLI
   - long command: MIB_ATUR_ChanIntervalEntryGet
   - short command: mibaturcieg

   \param adslAturChanIntvlInfo_t*
      The parameter points to a \ref adslAturChanIntvlInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAturChanIntvlInfo_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturChanIntervalTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.13\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslAturChanIntervalTable(13)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslAturChanIntervalEntry\n
      Prev sibling:     adslAtucChanIntervalTable\n
      Next sibling:     adslLineConfProfileTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturChanIntervalEntry\n
      Composed syntax:  SEQUENCE OF AdslAturChanIntervalEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturChanIntervalNumber - INTEGER(2 - integer (32 bit))\n
         2: adslAturChanIntervalReceivedBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         3: adslAturChanIntervalTransmittedBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         4: adslAturChanIntervalCorrectedBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         5: adslAturChanIntervalUncorrectBlks - PerfIntervalCount(66 - gauge (32 bit))\n
         6: adslAturChanIntervalValidData - TruthValue(2 - integer (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 13, adslAturChanIntvlInfo_t )

/**
   This table contains information on the ADSL line configuration. One entry in
   this table reflects a profile defined by a manager which can be used to
   configure the modem for a physical line

   CLI
   - long command: MIB_LineAlarmConfProfileEntryGet
   - short command: miblacpeg

   \param adslLineAlarmConfProfileEntry_t*
      The parameter points to a \ref adslLineAlarmConfProfileEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslLineAlarmConfProfileEntry_t nData = { 0 };
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslLineAlarmConfProfileTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.1.1.15\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslMibObjects(1).adslLineAlarmConfProfileTable(15)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslMibObjects\n
      First child:      adslLineAlarmConfProfileEntry\n
      Prev sibling:     adslLineConfProfileTable\n
      Next sibling:     adslLCSMib\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslLineAlarmConfProfileEntry\n
      Composed syntax:  SEQUENCE OF AdslLineAlarmConfProfileEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslLineAlarmConfProfileName - SnmpAdminString(4 - octets)\n
         2: adslAtucThresh15MinLofs - INTEGER(2 - integer (32 bit))\n
         3: adslAtucThresh15MinLoss - INTEGER(2 - integer (32 bit))\n
         4: adslAtucThresh15MinLols - INTEGER(2 - integer (32 bit))\n
         5: adslAtucThresh15MinLprs - INTEGER(2 - integer (32 bit))\n
         6: adslAtucThresh15MinESs - INTEGER(2 - integer (32 bit))\n
         7: adslAtucThreshFastRateUp - Unsigned32(66 - gauge (32 bit))\n
         8: adslAtucThreshInterleaveRateUp - Unsigned32(66 - gauge (32 bit))\n
         9: adslAtucThreshFastRateDown - Unsigned32(66 - gauge (32 bit))\n
         10: adslAtucThreshInterleaveRateDown - Unsigned32(66 - gauge (32 bit))\n
         11: adslAtucInitFailureTrapEnable - INTEGER(2 - integer (32 bit))\n
         12: adslAturThresh15MinLofs - INTEGER(2 - integer (32 bit))\n
         13: adslAturThresh15MinLoss - INTEGER(2 - integer (32 bit))\n
         14: adslAturThresh15MinLprs - INTEGER(2 - integer (32 bit))\n
         15: adslAturThresh15MinESs - INTEGER(2 - integer (32 bit))\n
         16: adslAturThreshFastRateUp - Unsigned32(66 - gauge (32 bit))\n
         17: adslAturThreshInterleaveRateUp - Unsigned32(66 - gauge (32 bit))\n
         18: adslAturThreshFastRateDown - Unsigned32(66 - gauge (32 bit))\n
         19: adslAturThreshInterleaveRateDown - Unsigned32(66 - gauge (32 bit))\n
         20: adslLineAlarmConfProfileRowStatus - RowStatus(2 - integer (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 14, adslLineAlarmConfProfileEntry_t )

/**
   For a description of this ioctl please refer to
   \ref DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET.
   The only difference is the access type get/set */
#define DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_SET \
   _IOWR( DSL_IOC_MAGIC_MIB, 15, adslLineAlarmConfProfileEntry_t )

/**
   This ioctl returns a bitfield indicating which traps has been occured on
   ATU-R/C side.

   CLI
   - long command: MIB_TrapsGet
   - short command: mibtg

   \param adslAturTrapsFlags_t*
      The parameter points to a value that has to interpreted as a bitfield.
      The meaning of the single bits is defined by \ref adslAturTrapsFlags_t

   \return
      0 if successful and -1 in case of an error/warning

   \code
      unsigned log nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_TRAPS_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturTraps\n
      Type:             OBJECT-IDENTIFIER\n
      OID:              1.3.6.1.2.1.10.94.1.2.2\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslLineMib(1).\n
                        adslTraps(2).adslAturTraps(2)\n
      Module:           ADSL-LINE-MIB\n
      Parent:           adslTraps\n
      First child:      adslAturTrapsx0\n
      Prev sibling:     adslAtucTraps\n
*/
#define DSL_FIO_MIB_ADSL_TRAPS_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 16, adslAturTrapsFlags_t )


#ifdef INCLUDE_ADSL_MIB_RFC3440

/**
   This table is an extension of RFC 2662. It contains ADSL line configuration
   and monitoring information. This includes the ADSL line's capabilities and
   actual ADSL transmission system.

   CLI
   - long command: MIB_LineExtEntryGet
   - short command: mibleeg

   \param adslLineExtTableEntry_t*
      The parameter points to a \ref adslLineExtTableEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslLineExtTableEntry_t nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslLineExtTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.3.1.17\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslLineExtTable(17)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslLineExtEntry\n
      Next sibling:     adslAtucPerfDataExtTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslLineExtEntry\n
      Composed syntax:  SEQUENCE OF AdslLineExtEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslLineTransAtucCap - AdslTransmissionModeType(4 - octets)\n
         2: adslLineTransAtucConfig - AdslTransmissionModeType(4 - octets)\n
         3: adslLineTransAtucActual - AdslTransmissionModeType(4 - octets)\n
         4: adslLineGlitePowerState - INTEGER(2 - integer (32 bit))\n
         5: adslLineConfProfileDualLite - SnmpAdminString(4 - octets)\n
*/
#define DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 17, adslLineExtTableEntry_t )

/**
   For a description of this ioctl please refer to
   \ref DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET.
   The only difference is the access type get/set */
#define DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_SET \
   _IOWR( DSL_IOC_MAGIC_MIB, 18, adslLineExtTableEntry_t )

/**
   This table extends adslAtucPerfDataTable [RFC2662] with additional ADSL
   physical line counter information such as unavailable seconds-line and
   severely errored seconds-line.

   CLI
   - long command: MIB_ATUC_PerfDataExtEntryGet
   - short command: mibatucpdeeg

   \param atucPerfDataExtEntry_t*
      The parameter points to a \ref atucPerfDataExtEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      atucPerfDataExtEntry_t nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslLineExtTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.3.1.17\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslLineExtTable(17)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslLineExtEntry\n
      Next sibling:     adslAtucPerfDataExtTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslLineExtEntry\n
      Composed syntax:  SEQUENCE OF AdslLineExtEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslLineTransAtucCap - AdslTransmissionModeType(4 - octets)\n
         2: adslLineTransAtucConfig - AdslTransmissionModeType(4 - octets)\n
         3: adslLineTransAtucActual - AdslTransmissionModeType(4 - octets)\n
         4: adslLineGlitePowerState - INTEGER(2 - integer (32 bit))\n
         5: adslLineConfProfileDualLite - SnmpAdminString(4 - octets)\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 19, atucPerfDataExtEntry_t )

/**
   This table contains ADSL physical line counters not defined in the
   adslAturPerfDataTable from the ADSL-LINE-MIB [RFC2662].

   CLI
   - long command: MIB_ATUR_PerfDataExtEntryGet
   - short command: mibaturpdeeg

   \param aturPerfDataExtEntry_t*
      The parameter points to a \ref aturPerfDataExtEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      aturPerfDataExtEntry_t nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturPerfDataExtTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.3.1.20\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslAturPerfDataExtTable(20)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslAturPerfDataExtEntry\n
      Prev sibling:     adslAtucIntervalExtTable\n
      Next sibling:     adslAturIntervalExtTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturPerfDataExtEntry\n
      Composed syntax:  SEQUENCE OF AdslAturPerfDataExtEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturPerfStatSesL - Counter32(65 - counter (32 bit))\n
         2: adslAturPerfStatUasL - Counter32(65 - counter (32 bit))\n
         3: adslAturPerfCurr15MinSesL - PerfCurrentCount(66 - gauge (32 bit))\n
         4: adslAturPerfCurr15MinUasL - PerfCurrentCount(66 - gauge (32 bit))\n
         5: adslAturPerfCurr1DaySesL - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         6: adslAturPerfCurr1DayUasL - AdslPerfCurrDayCount(66 - gauge (32 bit))\n
         7: adslAturPerfPrev1DaySesL - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
         8: adslAturPerfPrev1DayUasL - AdslPerfPrevDayCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 20, aturPerfDataExtEntry_t )

/**
   This table provides one row for each ATU-C performance data collection
   interval for ADSL physical interfaces whose IfEntries' ifType is equal to
   adsl(94).

   CLI
   - long command: MIB_ATUC_IntervalExtEntryGet
   - short command: mibatucieeg

   \param adslAtucInvtlExtInfo_t*
      The parameter points to a \ref adslAtucInvtlExtInfo_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslAtucInvtlExtInfo_t nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAtucIntervalExtTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.3.1.19\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslAtucIntervalExtTable(19)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslAtucIntervalExtEntry\n
      Prev sibling:     adslAtucPerfDataExtTable\n
      Next sibling:     adslAturPerfDataExtTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAtucIntervalExtEntry\n
      Composed syntax:  SEQUENCE OF AdslAtucIntervalExtEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAtucIntervalFastR - PerfIntervalCount(66 - gauge (32 bit))\n
         2: adslAtucIntervalFailedFastR - PerfIntervalCount(66 - gauge (32 bit))\n
         3: adslAtucIntervalSesL - PerfIntervalCount(66 - gauge (32 bit))\n
         4: adslAtucIntervalUasL - PerfIntervalCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 21, adslAtucInvtlExtInfo_t )

/**
   This table provides one row for each ATU-R performance data collection
   interval for ADSL physical interfaces whose IfEntries' ifType is equal to
   adsl(94).

   CLI
   - long command: MIB_ATUR_IntervalExtEntryGet
   - short command: mibaturieeg

   \param adslAturInvtlExtInfo_t*
      The parameter points to a \ref aturPerfDataExtEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      aturPerfDataExtEntry_t nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslAturIntervalExtTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.3.1.21\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslAturIntervalExtTable(21)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslAturIntervalExtEntry\n
      Prev sibling:     adslAturPerfDataExtTable\n
      Next sibling:     adslConfProfileExtTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslAturIntervalExtEntry\n
      Composed syntax:  SEQUENCE OF AdslAturIntervalExtEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslAturIntervalSesL - PerfIntervalCount(66 - gauge (32 bit))\n
         2: adslAturIntervalUasL - PerfIntervalCount(66 - gauge (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 22, adslAturInvtlExtInfo_t )

/**
   The adslConfProfileExtTable extends the ADSL line profile configuration
   information in the adslLineConfProfileTable from the ADSL-LINE-MIB [RFC2662]
   by adding the ability to configure the ADSL physical line mode.

   CLI
   - Not applicable for CLI interface

   \param adslLineAlarmConfProfileExtEntry_t*
      The parameter points to a \ref adslLineAlarmConfProfileExtEntry_t structure

   \return
      0 if successful and -1 in case of an error/warning

   \code
      adslLineAlarmConfProfileExtEntry_t nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslConfProfileExtTable\n
      Type:             OBJECT-TYPE\n
      OID:              1.3.6.1.2.1.10.94.3.1.22\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslConfProfileExtTable(22)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslConfProfileExtEntry\n
      Prev sibling:     adslAturIntervalExtTable\n
      Next sibling:     adslAlarmConfProfileExtTable\n
      Numerical syntax: Sequence\n
      Base syntax:      SEQUENCE OF AdslConfProfileExtEntry\n
      Composed syntax:  SEQUENCE OF AdslConfProfileExtEntry\n
      Max access:       not-accessible\n
      Sequences:\n
         1: adslConfProfileLineType - INTEGER(2 - integer (32 bit))\n
*/
#define DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 23, adslLineAlarmConfProfileExtEntry_t )

/**
   For a description of this ioctl please refer to
   \ref DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_GET.
   The only difference is the access type get/set */
#define DSL_FIO_MIB_ADSL_ALARM_CONF_PROFILE_EXT_ENTRY_SET \
   _IOWR( DSL_IOC_MAGIC_MIB, 24, adslLineAlarmConfProfileExtEntry_t )

/**
   This ioctl returns a bitfield indicating which traps has been occured on
   ATU-R/C side.

   CLI
   - long command: MIB_ExtTrapsGet
   - short command: mibetg

   \param adslAturExtTrapsFlags_t*
      The parameter points to a value that has to interpreted as a bitfield.
      The meaning of the single bits is defined by \ref adslAturExtTrapsFlags_t

   \return
      0 if successful and -1 in case of an error/warning

   \code
      unsigned log nData = 0;
      ret = ioctl(fd, DSL_FIO_MIB_ADSL_ATUR_EXT_TRAPS_GET, (DSL_int_t) &nData);
      // process received data
   \endcode

   \remarks
      Name:             adslExtTraps\n
      Type:             OBJECT-IDENTIFIER\n
      OID:              1.3.6.1.2.1.10.94.3.1.24\n
      Full path:        iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).\n
                        transmission(10).adslMIB(94).adslExtMIB(3).\n
                        adslExtMibObjects(1).adslExtTraps(24)\n
      Module:           ADSL-LINE-EXT-MIB\n
      Parent:           adslExtMibObjects\n
      First child:      adslExtAtucTraps\n
      Prev sibling:     adslAlarmConfProfileExtTable\n
*/
#define DSL_FIO_MIB_ADSL_EXT_TRAPS_GET \
   _IOWR( DSL_IOC_MAGIC_MIB, 25, adslAturExtTrapsFlags_t )

#endif /* INCLUDE_ADSL_MIB_RFC3440 */

/** @} DRV_DSL_CPE_ADSL_MIB */

#endif /* INCLUDE_DSL_ADSL_MIB */

#endif /* (INCLUDE_DSL_CPE_API_ADSL_SUPPORT == 1) */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_API_ADSLMIB_IOCTL_H */
