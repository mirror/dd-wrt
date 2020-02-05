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

extern void CLG_Initialise(void);
extern void CLG_Finalise(void);
extern int CLG_GetClientIndex(IPAddr *client);
extern int CLG_LogNTPAccess(IPAddr *client, struct timespec *now);
extern int CLG_LogCommandAccess(IPAddr *client, struct timespec *now);
extern int CLG_LimitNTPResponseRate(int index);
extern int CLG_LimitCommandResponseRate(int index);
extern void CLG_GetNtpTimestamps(int index, NTP_int64 **rx_ts, NTP_int64 **tx_ts);
extern int CLG_GetNtpMinPoll(void);

/* And some reporting functions, for use by chronyc. */

extern int CLG_GetNumberOfIndices(void);
extern int CLG_GetClientAccessReportByIndex(int index, RPT_ClientAccessByIndex_Report *report, struct timespec *now);
extern void CLG_GetServerStatsReport(RPT_ServerStatsReport *report);

#endif /* GOT_CLIENTLOG_H */
