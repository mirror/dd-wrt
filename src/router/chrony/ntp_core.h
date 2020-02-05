/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  Header file for the main NTP protocol engine
  */

#ifndef GOT_NTP_CORE_H
#define GOT_NTP_CORE_H

#include "sysincl.h"

#include "addressing.h"
#include "srcparams.h"
#include "ntp.h"
#include "reports.h"

typedef enum {
  NTP_SERVER, NTP_PEER
} NTP_Source_Type;

typedef enum {
  NTP_TS_DAEMON = 0,
  NTP_TS_KERNEL,
  NTP_TS_HARDWARE
} NTP_Timestamp_Source;

typedef struct {
  struct timespec ts;
  double err;
  NTP_Timestamp_Source source;
} NTP_Local_Timestamp;

/* This is a private data type used for storing the instance record for
   each source that we are chiming with */
typedef struct NCR_Instance_Record *NCR_Instance;

/* Init and fini functions */
extern void NCR_Initialise(void);
extern void NCR_Finalise(void);

/* Get a new instance for a server or peer */
extern NCR_Instance NCR_GetInstance(NTP_Remote_Address *remote_addr, NTP_Source_Type type, SourceParameters *params);

/* Destroy an instance */
extern void NCR_DestroyInstance(NCR_Instance instance);

/* Start an instance */
extern void NCR_StartInstance(NCR_Instance instance);

/* Reset an instance */
extern void NCR_ResetInstance(NCR_Instance inst);

/* Reset polling interval of an instance */
extern void NCR_ResetPoll(NCR_Instance instance);

/* Change the remote address of an instance */
extern void NCR_ChangeRemoteAddress(NCR_Instance inst, NTP_Remote_Address *remote_addr);

/* This routine is called when a new packet arrives off the network,
   and it relates to a source we have an ongoing protocol exchange with */
extern int NCR_ProcessRxKnown(NCR_Instance inst, NTP_Local_Address *local_addr,
                              NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length);

/* This routine is called when a new packet arrives off the network,
   and we do not recognize its source */
extern void NCR_ProcessRxUnknown(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                                 NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length);

/* This routine is called when a packet is sent to a source we have
   an ongoing protocol exchange with */
extern void NCR_ProcessTxKnown(NCR_Instance inst, NTP_Local_Address *local_addr,
                               NTP_Local_Timestamp *tx_ts, NTP_Packet *message, int length);

/* This routine is called when a packet is sent to a destination we
   do not recognize */
extern void NCR_ProcessTxUnknown(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                                 NTP_Local_Timestamp *tx_ts, NTP_Packet *message, int length);

/* Slew receive and transmit times in instance records */
extern void NCR_SlewTimes(NCR_Instance inst, struct timespec *when, double dfreq, double doffset);

/* Take a particular source online (i.e. start sampling it) or offline
   (i.e. stop sampling it) */
extern void NCR_SetConnectivity(NCR_Instance inst, SRC_Connectivity connectivity);

extern void NCR_ModifyMinpoll(NCR_Instance inst, int new_minpoll);

extern void NCR_ModifyMaxpoll(NCR_Instance inst, int new_maxpoll);

extern void NCR_ModifyMaxdelay(NCR_Instance inst, double new_max_delay);

extern void NCR_ModifyMaxdelayratio(NCR_Instance inst, double new_max_delay_ratio);

extern void NCR_ModifyMaxdelaydevratio(NCR_Instance inst, double new_max_delay_dev_ratio);

extern void NCR_ModifyMinstratum(NCR_Instance inst, int new_min_stratum);

extern void NCR_ModifyPolltarget(NCR_Instance inst, int new_poll_target);

extern void NCR_InitiateSampleBurst(NCR_Instance inst, int n_good_samples, int n_total_samples);

extern void NCR_ReportSource(NCR_Instance inst, RPT_SourceReport *report, struct timespec *now);
extern void NCR_GetNTPReport(NCR_Instance inst, RPT_NTPReport *report);

extern int NCR_AddAccessRestriction(IPAddr *ip_addr, int subnet_bits, int allow, int all);
extern int NCR_CheckAccessRestriction(IPAddr *ip_addr);

extern void NCR_IncrementActivityCounters(NCR_Instance inst, int *online, int *offline, 
                                          int *burst_online, int *burst_offline);

extern NTP_Remote_Address *NCR_GetRemoteAddress(NCR_Instance instance);

extern uint32_t NCR_GetLocalRefid(NCR_Instance inst);

extern int NCR_IsSyncPeer(NCR_Instance instance);

extern void NCR_AddBroadcastDestination(IPAddr *addr, unsigned short port, int interval);

#endif /* GOT_NTP_CORE_H */
