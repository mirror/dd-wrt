/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  This module contains facilities for logging access by clients.

  */

#ifndef GOT_CLIENTLOG_H
#define GOT_CLIENTLOG_H

#include "sysincl.h"
#include "reports.h"

typedef enum {
  CLG_NTP = 0,
  CLG_NTSKE,
  CLG_CMDMON,
} CLG_Service;

typedef enum {
  CLG_PASS = 0,
  CLG_DROP,
  CLG_KOD,
} CLG_Limit;

extern void CLG_Initialise(void);
extern void CLG_Finalise(void);
extern int CLG_GetClientIndex(IPAddr *client);
extern int CLG_LogServiceAccess(CLG_Service service, IPAddr *client, struct timespec *now);
extern CLG_Limit CLG_LimitServiceRate(CLG_Service service, int index);
extern void CLG_UpdateNtpStats(int auth, NTP_Timestamp_Source rx_ts_src,
                               NTP_Timestamp_Source tx_ts_src);
extern int CLG_GetNtpMinPoll(void);

/* Functions to save and retrieve timestamps for server interleaved mode */
extern void CLG_SaveNtpTimestamps(NTP_int64 *rx_ts, struct timespec *tx_ts,
                                  NTP_Timestamp_Source tx_src);
extern void CLG_UndoNtpTxTimestampSlew(NTP_int64 *rx_ts, struct timespec *tx_ts);
extern void CLG_UpdateNtpTxTimestamp(NTP_int64 *rx_ts, struct timespec *tx_ts,
                                     NTP_Timestamp_Source tx_src);
extern int CLG_GetNtpTxTimestamp(NTP_int64 *rx_ts, struct timespec *tx_ts,
                                 NTP_Timestamp_Source *tx_src);
extern void CLG_DisableNtpTimestamps(NTP_int64 *rx_ts);

/* And some reporting functions, for use by chronyc. */

extern int CLG_GetNumberOfIndices(void);
extern int CLG_GetClientAccessReportByIndex(int index, int reset, uint32_t min_hits,
                                            RPT_ClientAccessByIndex_Report *report,
                                            struct timespec *now);
extern void CLG_GetServerStatsReport(RPT_ServerStatsReport *report);

#endif /* GOT_CLIENTLOG_H */
